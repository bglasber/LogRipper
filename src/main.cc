#include "lex.h"
#include "file_reader.h"
#include "rule_applier.h"
#include "binner.h"
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>

//Different word matches will entail different log entries
//We don't want the file names in the GLOG preamble to create different events
void anonymize_glog_preamble( std::vector<TokenWordPair> *tokens_in_line ) {
    if( tokens_in_line->size() <= 18 ) {
        for( unsigned int i = 0; i < tokens_in_line->size(); i++ ) {
            std::cout <<  tokens_in_line->at(i).word << " ";
        }
        std::cout << std::endl;
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

void word_colon_number_anonymize( std::vector<TokenWordPair> *tokens_in_line ) {
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

void word_colon_space_number_anonymize( std::vector<TokenWordPair> *tokens_in_line ) {
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

void anonymize_array_indexes( std::vector<TokenWordPair> *tokens_in_line ) {
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

void anonymize_equal_signs( std::vector<TokenWordPair> *tokens_in_line ) {
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

void anonymize_client_ids( std::vector<TokenWordPair> *tokens_in_line ) {
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

void anonymize_location_ids( std::vector<TokenWordPair> *tokens_in_line ) {
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

void anonymize_write_list( std::vector<TokenWordPair> *tokens_in_line ) {
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

void anonymize_decimal( std::vector<TokenWordPair> *tokens_in_line ) {

    auto iter = tokens_in_line->begin();
    while( iter != tokens_in_line->end()-2 ) {
        auto iter_next = std::next( iter );
        auto iter_next_next = std::next( iter_next );
        if( (iter->tok == NUMBER || iter->tok == ABSTRACTED_VALUE ) &&
            iter_next->tok == PUNCTUATION &&
            iter_next->word == "." &&
            iter_next_next->tok == NUMBER ) {
            //match!
            /*
            std::cout << "Found decimal match..." << std::endl;
            for( auto it = tokens_in_line->begin(); it != tokens_in_line->end(); it++ ) {
                std::cout << it->word;
            }
            std::cout << std::endl;
            */
     
            break;
        }
        iter++;
    }

    if( iter != tokens_in_line->end()-2 ) {
        std::cout << "Found decimal match..." << std::endl;
        for( auto it = tokens_in_line->begin(); it != tokens_in_line->end(); it++ ) {
            std::cout << it->word;
        }
        std::cout << std::endl;
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

void abstract_from_for_number( std::vector<TokenWordPair> *tokens_in_line ) {
    auto iter = tokens_in_line->begin();
    while( iter != tokens_in_line->end() - 1 ) {
        auto iter_next = std::next( iter );
        auto iter_next_next = std::next( iter_next );
        if( iter->tok == WORD && ( iter->word == "from" || iter->word == "for" ) &&
            iter_next->tok == WHITE_SPACE && iter_next_next->tok == NUMBER ) {
            iter_next_next->tok = ABSTRACTED_VALUE;
        }
        iter++;
    }
}

void abstract_hostname( std::vector<TokenWordPair> *tokens_in_line ) {
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

void abstract_millis( std::vector<TokenWordPair> *tokens_in_line ) {
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

int main() {

    int fd = open( "ssServer.log.short", O_RDONLY ); 
    assert( fd > 0 );
    ParseBufferEngine pbe_file_to_rule;
    ParseBufferEngine pbe_rule_to_binner;
    FileReader reader( fd, 128u, &pbe_file_to_rule );

    std::list<RuleFunction> rule_funcs;
    rule_funcs.push_back( anonymize_glog_preamble );
    rule_funcs.push_back( word_colon_number_anonymize );
    rule_funcs.push_back( word_colon_space_number_anonymize );
    rule_funcs.push_back( anonymize_array_indexes );
    rule_funcs.push_back( anonymize_equal_signs );
    rule_funcs.push_back( anonymize_client_ids );
    rule_funcs.push_back( anonymize_location_ids );
    rule_funcs.push_back( anonymize_write_list );
    rule_funcs.push_back( anonymize_decimal );
    rule_funcs.push_back( abstract_from_for_number );
    rule_funcs.push_back( abstract_hostname );
    rule_funcs.push_back( abstract_millis );
    RuleApplier rule_applier( std::move( rule_funcs ), &pbe_file_to_rule, &pbe_rule_to_binner );

    Binner binner( &pbe_rule_to_binner );

    //Start the pipeline...
    binner.startProcessingBuffers();
    rule_applier.startProcessingBuffers();
    reader.processFile();
    std::cout << "Done processing file." << std::endl;

    rule_applier.terminateWhenDoneProcessing();
    std::cout << "Done waiting for rule applier..." << std::endl;
    binner.terminateWhenDoneProcessing();
    std::cout << "Done waiting for binner..." << std::endl;

    std::unordered_map<BinKey, Bin, BinKeyHasher> &map = binner.getUnderlyingMap();
    std::cout << "Found " << map.size() << " buckets in the map." << std::endl;

    unsigned num_unique_entries = 0;
    for( auto it = map.begin(); it != map.end(); it++ ) {
        std::vector<std::vector<TokenWordPair> *> &vec = it->second.getBinVector();
        std::cout << "Bucket Entries: " << std::endl;
        for( auto it2 = vec.begin(); it2 != vec.end(); it2++ ) {
            for( auto it3 = (*it2)->begin(); it3 != (*it2)->end(); it3++ ) {
                std::cout << it3->word << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
        num_unique_entries += vec.size();
    }
    std::cout << "Found " << num_unique_entries << " unique entries in the map." << std::endl;

    std::ofstream os;
    os.open( "deserialized_map", std::ofstream::out );
    binner.serialize( os );
    return 0;
}
