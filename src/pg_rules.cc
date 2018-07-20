#include "pg_rules.h"
#include <iostream>

void anonymize_pg_preamble( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines ) {
    if( tokens_in_lines.size() != 1 ) {
        dump_line( tokens_in_lines.at(0) );
        dump_line( tokens_in_lines.at(1) );
    }
    assert( tokens_in_lines.size() == 1 );
    std::vector<TokenWordPair> *line = tokens_in_lines.at(0);
    line->at(0).tok = ABSTRACTED_VALUE; //year
    line->at(2).tok = ABSTRACTED_VALUE; //month
    line->at(4).tok = ABSTRACTED_VALUE; //day
    line->at(6).tok = ABSTRACTED_VALUE; //hour
    line->at(8).tok = ABSTRACTED_VALUE; //minute
    line->at(10).tok = ABSTRACTED_VALUE; //second
    line->at(12).tok = ABSTRACTED_VALUE; //ms
    //line->at(14).tok = ABSTRACTED_VALUE; //timezone
    //line->at(16).tok = ABSTRACTED_VALUE; //d
    uint64_t offset = get_tid_offset_in_line( line );
    line->at( offset+2 ).tok = ABSTRACTED_VALUE; //tid
    line->erase( line->begin() + 16, line->begin() + offset );
}

void dump_line( std::vector<TokenWordPair> *line ) {
    for( const auto &twp : *line ) {
        std::cout << twp.tok << ", " << twp.word << std::endl;
    }
}

void strip_location_line( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines ) {
    //Last line in location line, git outta here
    //std::cout << "Num tokens in line: " << tokens_in_lines.size() << std::endl;
    delete tokens_in_lines.at( tokens_in_lines.size() - 1);
    tokens_in_lines.erase( tokens_in_lines.end()-1 );
}

void strip_detail_line( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines ) {
    for( unsigned int i = 0; i < tokens_in_lines.size(); i++ ) {
        uint64_t offset = get_tid_offset_in_line( tokens_in_lines.at(i) );
        if( tokens_in_lines.at(i)->at( offset + 4 ).word == "DETAIL" ) {
            delete tokens_in_lines.at( i );
            tokens_in_lines.erase( tokens_in_lines.begin() + i );
            break;
        }
    }
}

void strip_hint_line( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines ) {
    for( unsigned int i = 0; i < tokens_in_lines.size(); i++ ) {
        uint64_t offset = get_tid_offset_in_line( tokens_in_lines.at(i) );
        if( tokens_in_lines.at(i)->at( offset + 4 ).word == "HINT" ) {
            delete tokens_in_lines.at( i );
            tokens_in_lines.erase( tokens_in_lines.begin() + i );
            break;
        }
    }
}

void strip_context_line( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines ) {
    for( unsigned int i = 0; i < tokens_in_lines.size(); i++ ) {
        uint64_t offset = get_tid_offset_in_line( tokens_in_lines.at(i) );
        if( tokens_in_lines.at(i)->at( offset + 4 ).word == "CONTEXT" ) {
            delete tokens_in_lines.at( i );
            tokens_in_lines.erase( tokens_in_lines.begin() + i );
            break;
        }
    }
}

void fold_stmt_rollback_line( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines ) {
    //First line is statement
    uint64_t offset1 = get_tid_offset_in_line( tokens_in_lines.at(0) );

    if( tokens_in_lines.at(0)->at( offset1 + 4 ).word == "STATEMENT" and
        tokens_in_lines.at(1)->at( tokens_in_lines.at(1)->size()-2 ).word == "ROLLBACK" ) {
        TokenWordPair twp;
        twp.tok = WORD;
        twp.word = "ROLLBACK";
        TokenWordPair twp2;
        twp2.tok = WHITE_SPACE;
        twp2.word = " ";
        tokens_in_lines.at(0)->insert( tokens_in_lines.at(0)->begin() + offset1 + 4, twp2 );
        tokens_in_lines.at(0)->insert( tokens_in_lines.at(0)->begin() + offset1 + 4, twp );
        delete tokens_in_lines.at( 1 );
        tokens_in_lines.erase( tokens_in_lines.begin() + 1 );
    }
}

void abstract_equal_number( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines ) {
    for( uint64_t i = 0; i < tokens_in_lines.size(); i++ ) { 
        std::vector<TokenWordPair> *line = tokens_in_lines.at(i);
        for( uint64_t j = 0; j < line->size()-2; j++ ) {
            if( line->at( j ).tok == PUNCTUATION and line->at( j ).word == "=" and
                line->at(j+1).tok == WHITE_SPACE and
                line->at(j+2).tok == NUMBER )
            {
                line->at(j+2).tok = ABSTRACTED_VALUE;
            }
        }
    }
}

void fold_debug_line( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines ) {
    //First line is DEBUG
    if( tokens_in_lines.size() > 1 ) {
        uint64_t offset1 = get_tid_offset_in_line( tokens_in_lines.at(1) );
        if( tokens_in_lines.at(1)->at( offset1 + 4 ).word == "DEBUG" ) {
            tokens_in_lines.at(0)->insert( tokens_in_lines.at(0)->end(), tokens_in_lines.at(1)->begin() + offset1 + 10, tokens_in_lines.at(1)->end() );

            delete tokens_in_lines.at( 1 );
            tokens_in_lines.erase( tokens_in_lines.begin() + 1 );
            //dump_line( tokens_in_lines.at(0) );
        }
    }
}

void fold_error_line( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines ) {
    //First line is DEBUG
    if( tokens_in_lines.size() > 1 ) {
        uint64_t offset1 = get_tid_offset_in_line( tokens_in_lines.at(1) );
        if( tokens_in_lines.at(1)->at( offset1 + 4 ).word == "ERROR" ) {
            tokens_in_lines.at(0)->insert( tokens_in_lines.at(0)->end(), tokens_in_lines.at(1)->begin() + offset1 + 10, tokens_in_lines.at(1)->end() );

            delete tokens_in_lines.at( 1 );
            tokens_in_lines.erase( tokens_in_lines.begin() + 1 );
            //dump_line( tokens_in_lines.at(0) );
        }
    }
}

void strip_redundant_secondary_line( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines ) {
    if( tokens_in_lines.size() > 1 ) {
        uint64_t offset1 = get_tid_offset_in_line( tokens_in_lines.at(1) );
        if( tokens_in_lines.at(1)->at( offset1 + 4 ).word == "LOG" ) {
            delete tokens_in_lines.at( 1 );
            tokens_in_lines.erase( tokens_in_lines.begin() + 1 );
        }
    }
}

void abstract_statement_names( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines ) {
    assert( tokens_in_lines.size() == 1 );
    uint64_t offset1 = get_tid_offset_in_line( tokens_in_lines.at(0) );
    std::vector<TokenWordPair> *line = tokens_in_lines.at(0);
    for( uint64_t i = offset1; i <= line->size()-2; i++ ) {

        if( line->at( i ).tok == WORD and line->at( i ).word == "S" and
            line->at( i+1 ).tok == PUNCTUATION and line->at( i+1 ).word == "_" and
            line->at( i+2 ).tok == NUMBER )  {
            TokenWordPair twp;
            twp.tok = ABSTRACTED_VALUE;
            twp.word = line->at(i).word + line->at(i+1).word + line->at(i+2).word;
            line->erase( line->begin() + i, line->begin() + i+3 );
            line->insert( line->begin() + i, twp );
        }
    }
}

void abstract_unnamed( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines ) {
    assert( tokens_in_lines.size() == 1 );
    uint64_t offset1 = get_tid_offset_in_line( tokens_in_lines.at(0) );
    std::vector<TokenWordPair> *line = tokens_in_lines.at(0);
    for( uint64_t i = offset1; i <= line->size()-2; i++ ) {

        if( line->at( i ).tok == PUNCTUATION and line->at( i ).word == "<" and
            line->at( i+1 ).tok == WORD and line->at( i+1 ).word == "unnamed" and
            line->at( i+2 ).tok == PUNCTUATION and line->at( i+1 ).word == ">" )  {
            TokenWordPair twp;
            twp.tok = ABSTRACTED_VALUE;
            twp.word = line->at(i).word + line->at(i+1).word + line->at(i+2).word;
            line->erase( line->begin() + i, line->begin() + i+3 );
            line->insert( line->begin() + i, twp );
        }
    }
}

void abstract_threshold( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines ) {
    assert( tokens_in_lines.size() == 1 );
    uint64_t offset1 = get_tid_offset_in_line( tokens_in_lines.at(0) );
    std::vector<TokenWordPair> *line = tokens_in_lines.at(0);

    for( uint64_t i = offset1; i <= line->size()-3; i++ ) {
        if( line->at( i ).tok == PUNCTUATION and line->at( i ).word == "(" and
            line->at( i+1 ).tok == WORD and line->at( i+1 ).word == "threshold" and
            line->at( i+2 ).tok == WHITE_SPACE and 
            line->at( i+3 ).tok == NUMBER ) {
            line->at( i+3 ).tok = ABSTRACTED_VALUE;
        }
    }
}

void abstract_vac_anl( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines ) {
    assert( tokens_in_lines.size() == 1 );
    uint64_t offset1 = get_tid_offset_in_line( tokens_in_lines.at(0) );
    std::vector<TokenWordPair> *line = tokens_in_lines.at(0);

    for( uint64_t i = offset1; i <= line->size()-3; i++ ) {
        if( line->at( i ).tok == WORD and (line->at( i ).word == "vac" or line->at( i ).word == "anl" ) and
            line->at( i+1 ).tok == PUNCTUATION and line->at( i+1 ).word == ":" and
            line->at( i+2 ).tok == WHITE_SPACE and 
            line->at( i+3 ).tok == NUMBER ) {
            line->at( i+3 ).tok = ABSTRACTED_VALUE;
        }
    }
}

void abstract_log_file_identifier(  std::vector<std::vector<TokenWordPair> *> &tokens_in_lines ) {
    assert( tokens_in_lines.size() == 1 );
    uint64_t offset1 = get_tid_offset_in_line( tokens_in_lines.at(0) );
    std::vector<TokenWordPair> *line = tokens_in_lines.at(0);
    for( uint64_t i = offset1; i <= line->size()-2; i++ ) {
        if( line->at( i ).tok == WORD and ( 
                line->at( i ).word == "log" or
                line->at( i ).word == "stats" ) and
            line->at( i+1 ).tok == WHITE_SPACE and
            line->at( i+2 ).tok == WORD and line->at( i+2 ).word == "file" ) {

            TokenWordPair twp;
            twp.tok = ABSTRACTED_VALUE;
            std::string word;
            for( unsigned int j = i; j < line->size(); j++ ) {
                word += line->at( j ).word;
            }
            twp.word = word;
            line->erase( line->begin() + i, line->end() );
            line->insert( line->begin() + i, twp );
            break;
        }
    }
}

void abstract_post_int(  std::vector<std::vector<TokenWordPair> *> &tokens_in_lines ) {
    assert( tokens_in_lines.size() == 1 );
    uint64_t offset1 = get_tid_offset_in_line( tokens_in_lines.at(0) );
    std::vector<TokenWordPair> *line = tokens_in_lines.at(0);
    for( uint64_t i = offset1; i <= line->size()-2; i++ ) {
        if( line->at( i ).tok == WORD and (
                line->at( i ).word == "in" or
                line->at( i ).word == "of" or
                line->at( i ).word == "is" or
                line->at( i ).word == "found" or
                line->at( i ).word == "remove" or
                line->at( i ).word == "contains" or
                line->at( i ).word == "removed" or
                line->at( i ).word == "scanned" or
                line->at( i ).word == "PID" ) and 
            line->at( i+1 ).tok == WHITE_SPACE and
            line->at( i+2 ).tok == NUMBER ) {
            line->at( i+2 ).tok = ABSTRACTED_VALUE;
        }
    }
}

void abstract_pre_int(  std::vector<std::vector<TokenWordPair> *> &tokens_in_lines ) {
    assert( tokens_in_lines.size() == 1 );
    uint64_t offset1 = get_tid_offset_in_line( tokens_in_lines.at(0) );
    std::vector<TokenWordPair> *line = tokens_in_lines.at(0);
    for( uint64_t i = offset1; i <= line->size()-2; i++ ) {
        if( line->at(i).tok == NUMBER and 
            line->at( i+1 ).tok == WHITE_SPACE and
            line->at( i+2 ).tok == WORD and (
                line->at( i+2 ).word == "on" or
                line->at( i+2 ).word == "callbacks" or
                line->at( i+2 ).word == "before" or
                line->at( i+2 ).word == "dead" or
                line->at( i+2 ).word == "rows" or
                line->at( i+2 ).word == "nonremovable" or
                line->at( i+2 ).word == "estimated" ) ) {
            line->at( i ).tok = ABSTRACTED_VALUE;
        }
    }
}

void abstract_slash_numbers( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines ) {
    assert( tokens_in_lines.size() == 1 );
    uint64_t offset1 = get_tid_offset_in_line( tokens_in_lines.at(0) );
    std::vector<TokenWordPair> *line = tokens_in_lines.at(0);
    for( uint64_t i = offset1; i <= line->size()-4; i++ ) {
        if( line->at(i).tok == NUMBER and
            line->at(i+1).tok == PUNCTUATION and line->at(i+1).word == "/" and
            line->at(i+2).tok == NUMBER and
            line->at(i+3).tok == PUNCTUATION and line->at(i+3).word == "/" and
            line->at(i+4).tok == NUMBER ) {
            line->at(i).tok = ABSTRACTED_VALUE;
            line->at(i+2).tok = ABSTRACTED_VALUE;
            line->at(i+4).tok = ABSTRACTED_VALUE;
        }
    }
}

void strip_intermediate_newline( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines ) {
    assert( tokens_in_lines.size() == 1 );
    uint64_t offset1 = get_tid_offset_in_line( tokens_in_lines.at(0) );
    std::vector<TokenWordPair> *line = tokens_in_lines.at(0);
    for( uint64_t i = offset1; i <= line->size()-1; i++ ) {
        if( line->at(i).tok == NEW_LINE ) { 
            //i+1 not included
            line->erase( line->begin() + i, line->begin() + i + 1 );
            break;
        }
    }
}


void abstract_int_index( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines ) {
    assert( tokens_in_lines.size() == 1 );
    uint64_t offset1 = get_tid_offset_in_line( tokens_in_lines.at(0) );
    std::vector<TokenWordPair> *line = tokens_in_lines.at(0);
    for( uint64_t i = offset1; i <= line->size()-2; i++ ) {
        if( line->at(i).tok == PUNCTUATION and ( 
                line->at(i).word == "(" or line->at(i).word == "[" ) and
            line->at( i+1 ).tok == NUMBER and
            line->at( i+2 ).tok == PUNCTUATION and (
                line->at(i+2).word == ")" or line->at(i+2).word == "]" ) ) {
            line->at( i+1 ).tok = ABSTRACTED_VALUE;
        }
    }
}

void abstract_int_equal(  std::vector<std::vector<TokenWordPair> *> &tokens_in_lines ) {
    assert( tokens_in_lines.size() == 1 );
    uint64_t offset1 = get_tid_offset_in_line( tokens_in_lines.at(0) );
    std::vector<TokenWordPair> *line = tokens_in_lines.at(0);
    for( uint64_t i = offset1; i <= line->size()-2; i++ ) {
        if( line->at(i).tok == WORD and 
            line->at( i+1 ).tok == PUNCTUATION and line->at( i+1 ).word == "=" and
            line->at( i+2 ).tok == NUMBER ) {
            line->at( i+2 ).tok = ABSTRACTED_VALUE;
        }
    }
}

void fold_stmt_commit_line( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines ) {
    uint64_t offset1 = get_tid_offset_in_line( tokens_in_lines.at(0) );
    if( tokens_in_lines.at(0)->at( offset1 + 4 ).word == "STATEMENT" and
        tokens_in_lines.at(0)->at( tokens_in_lines.at(0)->size() - 2 ).word == "COMMIT" ) {
        delete tokens_in_lines.at( 1 );
        tokens_in_lines.erase( tokens_in_lines.begin() + 1 );
    }
}

void strip_exec_name( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines ) {
    std::vector<TokenWordPair> *line = tokens_in_lines.at(0);
    uint64_t offset1 = get_tid_offset_in_line( line );
    if( offset1 + 9 < line->size() ) {
        if( line->at( offset1 + 10 ).word == "execute" ) {
            uint64_t prune_offset = offset1 + 12; //Start of whatever the name is of the thing we are executing
            uint64_t end_offset;
            // FIXME Breaks horribly if there is no : in this line
            for( end_offset = prune_offset; end_offset < line->size(); end_offset++ ) {
                if( line->at( end_offset ).word == ":" ) {
                    break;
                }
            }
            end_offset++;
            line->erase( line->begin() + prune_offset, line->begin() + end_offset );
        }
    }

}

uint64_t get_pg_thread_id_from_tid_offset( std::vector<TokenWordPair> *line, uint64_t tid_offset ) {
    return atoi(line->at(tid_offset+2).word.c_str());
}

uint64_t get_pg_thread_id_from_parsed_line( std::vector<TokenWordPair> *line ) {
    int i = get_tid_offset_in_line( line );
    return get_pg_thread_id_from_tid_offset( line, i );
}

uint64_t get_tid_offset_in_line ( std::vector<TokenWordPair> *line ) {
    for( unsigned int i = 0; i < line->size()-2; i++ ) {
        if( (line->at(i).tok == WORD and line->at(i).word == "p" ) and
            (line->at(i+1).tok == PUNCTUATION and line->at(i+1).word == "=" ) and
            (line->at(i+2).tok == NUMBER or line->at(i+2).tok == ABSTRACTED_VALUE ) ) {
            return i;
        }
    }
    //cause array out of bounds error
    return UINT64_MAX;
}
