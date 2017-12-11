#include "rules.h"

uint64_t get_thread_id_from_parsed_line( std::shared_ptr<std::vector<TokenWordPair>> &line ) {
    if( line->size() < 12 ) {
        return 0;
    }
    return atoi(line->at(11).word.c_str());
}

