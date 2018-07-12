#ifndef __PG_RULES_H__
#define __PG_RULES_H__
#include "token.h"
#include <vector>

void anonymize_pg_preamble( std::vector<std::vector<TokenWordPair> *> &tokens_in_line );
uint64_t get_pg_thread_id_from_parsed_line( std::vector<TokenWordPair> *line );

#endif
