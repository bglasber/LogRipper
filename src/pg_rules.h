#ifndef __PG_RULES_H__
#define __PG_RULES_H__
#include "token.h"
#include <vector>

void anonymize_pg_preamble( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines );
void strip_location_line( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines );
void strip_detail_line( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines );
void strip_hint_line( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines );
void strip_context_line( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines );
void fold_stmt_rollback_line( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines );
void fold_stmt_commit_line( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines );



uint64_t get_pg_thread_id_from_parsed_line( std::vector<TokenWordPair> *line );
void dump_line( std::vector<TokenWordPair> *line );
uint64_t get_tid_offset_in_line( std::vector<TokenWordPair> *line );
uint64_t get_pg_thread_id_from_tid_offset( std::vector<TokenWordPair> *line, uint64_t tid_offset );

#endif
