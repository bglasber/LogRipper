#ifndef __RULES_H__
#define __RULES_H__
#include "token.h"
#include <vector>
#include <memory>

void anonymize_log_preamble( std::unique_ptr<std::vector<TokenWordPair>> &tokens_in_line );

uint64_t get_thread_id_from_parsed_line( std::shared_ptr<std::vector<TokenWordPair>> &line );
#endif
