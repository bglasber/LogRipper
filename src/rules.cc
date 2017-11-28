#include "rules.h"

//Different word matches will entail different log entries
//We don't want the file names in the GLOG preamble to create different events
void anonymize_glog_preamble( std::unique_ptr<std::vector<TokenWordPair>> &tokens_in_line ) {
    if( tokens_in_line->size() <= 18 ) {
        return;
    }
    tokens_in_line->at(0).tok = ABSTRACTED_VALUE; //0 is log level
    tokens_in_line->at(1).tok = ABSTRACTED_VALUE; //RUN_ID
    //2 is white_space
    tokens_in_line->at(3).tok = ABSTRACTED_VALUE; //3 is HOUR
    //4 is :
    tokens_in_line->at(5).tok = ABSTRACTED_VALUE; //5 is MINUTE
    //6 is :
    tokens_in_line->at(7).tok = ABSTRACTED_VALUE; //7 is SECOND
    //8 is .
    tokens_in_line->at(9).tok = ABSTRACTED_VALUE; //9 is NANOSECOND
    //10 is whitespace
    tokens_in_line->at(11).tok = ABSTRACTED_VALUE; //11 is thread_id
    //13 is path of C++ file
    tokens_in_line->at(13).tok = ABSTRACTED_VALUE; //C++ FILE_NAME
    //14 is .
    tokens_in_line->at(15).tok = ABSTRACTED_VALUE; //C++ FILE EXT
    //16 is :
    tokens_in_line->at(17).tok = ABSTRACTED_VALUE; //C++ LINE NUMBER
}

void word_colon_number_anonymize( std::unique_ptr<std::vector<TokenWordPair>> &tokens_in_line ) {
    unsigned int i = 0;
    while( i < tokens_in_line->size() - 2 ) {
        if( tokens_in_line->at(i).tok == WORD &&
            tokens_in_line->at(i+1).tok == PUNCTUATION &&
            tokens_in_line->at(i+1).word == ":" &&
            tokens_in_line->at(i+2).tok == NUMBER ){
            tokens_in_line->at(i+2).tok = ABSTRACTED_VALUE;
        }
        i++;
    }
}

void word_colon_space_number_anonymize( std::unique_ptr<std::vector<TokenWordPair>> &tokens_in_line ) {
    unsigned int i = 0;
    while( i < tokens_in_line->size() - 3 ) {
        if( tokens_in_line->at(i).tok == WORD &&
            tokens_in_line->at(i+1).tok == PUNCTUATION &&
            tokens_in_line->at(i+1).word == ":" &&
            tokens_in_line->at(i+2).tok == WHITE_SPACE &&
            tokens_in_line->at(i+3).tok == NUMBER ){
            tokens_in_line->at(i+3).tok = ABSTRACTED_VALUE;
        }
        i++;
    }
}

void anonymize_array_indexes( std::unique_ptr<std::vector<TokenWordPair>> &tokens_in_line ) {
    unsigned int i = 0;
    while( i < tokens_in_line->size() - 2 ) {
        if( tokens_in_line->at(i).tok == PUNCTUATION &&
            tokens_in_line->at(i).word == "[" &&
            tokens_in_line->at(i+1).tok == NUMBER &&
            tokens_in_line->at(i+2).tok == PUNCTUATION &&
            tokens_in_line->at(i+2).word == "]" ) {
            tokens_in_line->at(i+1).tok = ABSTRACTED_VALUE;
        }
        i++;
    }
}

void anonymize_equal_signs( std::unique_ptr<std::vector<TokenWordPair>> &tokens_in_line ) {
    unsigned int i = 0;
    while( i < tokens_in_line->size() - 2 ) {
        if( tokens_in_line->at(i+1).tok == PUNCTUATION &&
            tokens_in_line->at(i+1).word == "=" ) {
            if( tokens_in_line->at(i).tok == NUMBER ) {
                tokens_in_line->at(i).tok = ABSTRACTED_VALUE;
            }
            if( tokens_in_line->at(i+2).tok == NUMBER ) {
                tokens_in_line->at(i+2).tok = ABSTRACTED_VALUE;
            }
        }
        i++;
    }
}

void anonymize_client_ids( std::unique_ptr<std::vector<TokenWordPair>> &tokens_in_line ) {
    unsigned int i = 0;
    while( i < tokens_in_line->size() - 2 ) {
        if( tokens_in_line->at(i).tok == WORD &&
            ( tokens_in_line->at(i).word == "client" ||
              tokens_in_line->at(i).word == "Client" ) ) {
            if( tokens_in_line->at(i+1).tok == NUMBER ) {
                tokens_in_line->at(i+1).tok = ABSTRACTED_VALUE;
            } else if( tokens_in_line->at(i+1).tok == WHITE_SPACE &&
                       tokens_in_line->at(i+2).tok == NUMBER ) {
                tokens_in_line->at(i+2).tok = ABSTRACTED_VALUE;
            }
        }
        i++;
    }
}

void anonymize_location_ids( std::unique_ptr<std::vector<TokenWordPair>> &tokens_in_line ) {
    unsigned int i = 0;
    while( i < tokens_in_line->size() - 2 ) {
        if( tokens_in_line->at(i).tok == WORD &&
            tokens_in_line->at(i).word == "at" ) {
            if( tokens_in_line->at(i+1).tok == NUMBER ) {
                tokens_in_line->at(i+1).tok = ABSTRACTED_VALUE;
            } else if( tokens_in_line->at(i+1).tok == WHITE_SPACE &&
                       tokens_in_line->at(i+2).tok == NUMBER ) {
                tokens_in_line->at(i+2).tok = ABSTRACTED_VALUE;
            }
        }
        i++;
    }
}

void anonymize_write_list( std::unique_ptr<std::vector<TokenWordPair>> &tokens_in_line ) {
    bool anonymized = false;
    auto iter = tokens_in_line->begin();
    while( iter != tokens_in_line->end()-3 ) {
        auto iter_next = std::next( iter );
        auto iter_next_next = std::next( iter_next );
        auto iter_next_next_next = std::next( iter_next_next );
        if( (iter)->tok == PUNCTUATION &&
            (iter)->word == "[" &&
                (
                    ( (iter_next)->tok == PUNCTUATION &&
                      (iter_next)->word == "{" ) ||
                    ( (iter_next)->tok == WHITE_SPACE &&
                      (iter_next_next)->tok == NUMBER &&
                      (iter_next_next_next)->tok == PUNCTUATION &&
                      (iter_next_next_next)->word == "," )
                )
        ) {
            break;
        }
        iter++;
    }
    // We found the array...
    if( iter != tokens_in_line->end()-3 ) {
        auto insert_loc = iter;
        ++iter; //Now the elements of the array

        TokenWordPair twp;
        twp.tok = ABSTRACTED_VALUE;
        twp.word = "";

        auto end_delete_loc = iter;

        while( !(end_delete_loc->tok == PUNCTUATION &&
               end_delete_loc->word == "]") ) {
            assert( end_delete_loc != tokens_in_line->end() );
            twp.word += " " + end_delete_loc->word;
            end_delete_loc++;
        }

        if( iter != end_delete_loc ) {
            tokens_in_line->erase( iter, end_delete_loc );
        }

        insert_loc++;
        tokens_in_line->insert( insert_loc, std::move( twp ) );
        anonymized = true;
    }
    //Might be more, recurse
    if( anonymized ) {
        anonymize_write_list( tokens_in_line );
    }
}

void anonymize_decimal( std::unique_ptr<std::vector<TokenWordPair>> &tokens_in_line ) {

    auto iter = tokens_in_line->begin();
    while( iter != tokens_in_line->end()-2 ) {
        auto iter_next = std::next( iter );
        auto iter_next_next = std::next( iter_next );
        if( (iter->tok == NUMBER || iter->tok == ABSTRACTED_VALUE ) &&
            iter_next->tok == PUNCTUATION &&
            iter_next->word == "." &&
            iter_next_next->tok == NUMBER ) {
            //match!
            break;
        }
        iter++;
    }

    if( iter != tokens_in_line->end()-2 ) {
        TokenWordPair twp;
        twp.tok = ABSTRACTED_VALUE;
        twp.word = "";

        auto end_delete_loc = iter;
        auto insert_loc = iter;
        insert_loc--;

        for( ;; ) {
            if( !(end_delete_loc->tok == NUMBER || end_delete_loc->tok == ABSTRACTED_VALUE ) ) {
                break;
            }
            twp.word += " " + end_delete_loc->word;
            end_delete_loc++;

            if( !(end_delete_loc->tok == PUNCTUATION  &&
                end_delete_loc->word== ".") ) {
                break;
            }
            twp.word += " " + end_delete_loc->word;
            end_delete_loc++;

            if( end_delete_loc->tok != NUMBER ) {
                break;
            }
            twp.word += " " + end_delete_loc->word;
            end_delete_loc++;

             if( !(end_delete_loc->tok == WORD &&
                   end_delete_loc->word == "e") ) {
                break;
            }
            twp.word += " " + end_delete_loc->word;
            end_delete_loc++;

            if( !(end_delete_loc->tok == PUNCTUATION &&
                 ( end_delete_loc->word == "+" ||
                   end_delete_loc->word == "-") ) ) {
                break;
            }
            twp.word += " " + end_delete_loc->word;
            end_delete_loc++;

            if( end_delete_loc->tok != NUMBER ) {
                break;
            }
            twp.word += " " + end_delete_loc->word;
            end_delete_loc++;
        }
        tokens_in_line->erase( iter, end_delete_loc );
        insert_loc++;
        tokens_in_line->insert( insert_loc, std::move( twp ) );
    }
}

void abstract_from_for_number( std::unique_ptr<std::vector<TokenWordPair>> &tokens_in_line ) {
    auto iter = tokens_in_line->begin();
    while( iter != tokens_in_line->end() - 2 ) {
        auto iter_next = std::next( iter );
        auto iter_next_next = std::next( iter_next );
        if( iter->tok == WORD && ( iter->word == "from" || iter->word == "for" || iter->word == "on" ) &&
            iter_next->tok == WHITE_SPACE && iter_next_next->tok == NUMBER ) {
            iter_next_next->tok = ABSTRACTED_VALUE;
        }
        iter++;
    }
}

void abstract_destination_site( std::unique_ptr<std::vector<TokenWordPair>> &tokens_in_line ) {
    auto iter = tokens_in_line->begin();
    while( iter != tokens_in_line->end() - 1 ) {
        auto iter_next = std::next( iter );
        if( iter->tok == WORD && iter->word == "destination" &&
            iter_next->tok == NUMBER ) {
            iter_next->tok = ABSTRACTED_VALUE;
        }
        iter++;
    }

}

void abstract_hostname( std::unique_ptr<std::vector<TokenWordPair>> &tokens_in_line ) {
    auto iter = tokens_in_line->begin();
    while( iter != tokens_in_line->end() -1 ) {
        auto iter_next = std::next( iter );
        if( iter->tok == WORD && iter->word == "tem" &&
            iter_next->tok == NUMBER ) {
            iter_next->tok = ABSTRACTED_VALUE;
        }
        iter++;
    }
}

void abstract_millis( std::unique_ptr<std::vector<TokenWordPair>> &tokens_in_line ) {
    auto iter = tokens_in_line->begin();
    while( iter != tokens_in_line->end() - 2 ) {
        auto iter_next = std::next( iter );
        auto iter_next_next = std::next( iter_next );
        if( iter->tok == NUMBER &&
            iter_next->tok == WHITE_SPACE &&
            iter_next_next->tok == WORD &&
            iter_next_next->word == "millis" ) {
            iter->tok = ABSTRACTED_VALUE;
        }
        iter++;
    }
}

void abstract_bucket_line1( std::unique_ptr<std::vector<TokenWordPair>> &tokens_in_line ) {
    auto iter = tokens_in_line->begin();
    while( iter != tokens_in_line->end()-1 ) {
        auto iter_next = std::next( iter );
        if( iter->tok == WORD &&
            iter->word == "Bucket" &&
            iter_next->tok == PUNCTUATION &&
            iter_next->word == "[" ) {            
            break;
        }
        iter++;
    }

    if( iter != tokens_in_line->end()-1 ) {
        iter++; //After: [
        auto insert_loc = iter;
        (void) insert_loc;
        iter++; //After: Stuff in bracket

        TokenWordPair twp;
        twp.tok = ABSTRACTED_VALUE;
        twp.word = "";
        auto end_delete_loc = iter;
        while( !(end_delete_loc->tok == PUNCTUATION &&
               end_delete_loc->word == "]") ) {
            assert( end_delete_loc != tokens_in_line->end() );
            twp.word += " " + end_delete_loc->word;
            end_delete_loc++;
        }



        tokens_in_line->erase( iter, end_delete_loc );
        insert_loc++;
        tokens_in_line->insert( insert_loc, std::move( twp ) );

        iter = insert_loc; //The abstracted token
        iter++; //The ] bracket
        assert( iter != tokens_in_line->end() );
        iter++; //whitespace
        assert( iter != tokens_in_line->end() );
        insert_loc = iter;
        iter++; //Start of stuff to abstract
        assert( iter != tokens_in_line->end() );
        
        TokenWordPair twp2;
        twp2.tok = ABSTRACTED_VALUE;
        twp2.word = "";
        end_delete_loc = iter;
        for( ;; ) {
            assert( end_delete_loc != tokens_in_line->end() );
            if( end_delete_loc->tok == WORD && end_delete_loc->word == "is" ) {
                end_delete_loc--; //WHITESPACE, thing before it should be entry
                break;
            }
            twp2.word += " " + end_delete_loc->word;
            end_delete_loc++;
        }


        tokens_in_line->erase( iter, end_delete_loc );
        insert_loc++;
        tokens_in_line->insert( insert_loc, std::move( twp2 ) );


        auto iter = insert_loc;
        while( !(iter->tok == WORD && iter->word == "site") ) {
            iter++;
        }
        iter++; //hop to whitespace
        iter++; //hop to number
        assert( iter->tok == NUMBER );
        iter->tok = ABSTRACTED_VALUE;
    }
}

void abstract_client_locks_number( std::unique_ptr<std::vector<TokenWordPair>> &tokens_in_line ) {
    auto iter = tokens_in_line->begin();
    while( iter != tokens_in_line->end()-1 ) {
        if( iter->tok == WORD && iter->word == "locks" ) {
            break;
        }
        iter++;
    }

    if( iter != tokens_in_line->end()-1 ) {
        iter++;
        if( iter->tok == NUMBER ) {
            iter->tok = ABSTRACTED_VALUE;
        }
    }
}

uint64_t get_thread_id_from_parsed_line( std::shared_ptr<std::vector<TokenWordPair>> &line ) {
    if( line->size() < 12 ) {
        return 0;
    }
    return atoi(line->at(11).word.c_str());
}

