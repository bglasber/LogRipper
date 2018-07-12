#include "pg_rules.h"
#include <iostream>

void anonymize_pg_preamble( std::vector<std::vector<TokenWordPair> *> &tokens_in_line ) {
    std::cout << tokens_in_line.size() << std::endl;
    (void) tokens_in_line;
}


uint64_t get_pg_thread_id_from_parsed_line( std::vector<TokenWordPair> *line ) {
    for( unsigned int i = 0; i < line->size()-2; i++ ) {
        if( (line->at(i).tok == WORD and line->at(i).word == "p" ) and
            (line->at(i+1).tok == PUNCTUATION and line->at(i+1).word == "=" ) and
            (line->at(i+2).tok == NUMBER ) ) {
            return atoi(line->at(i+2).word.c_str());
        }
    }
    return 0;
}
