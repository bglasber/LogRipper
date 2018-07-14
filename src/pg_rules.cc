#include "pg_rules.h"
#include <iostream>

void anonymize_pg_preamble( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines ) {
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
