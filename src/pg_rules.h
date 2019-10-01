#ifndef __PG_RULES_H__
#define __PG_RULES_H__
#include "token.h"
#include <vector>

void anonymize_pg_preamble( std::unique_ptr<std::vector<TokenWordPair>> &line );
/*
void strip_location_line( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines );
void strip_detail_line( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines );
void strip_hint_line( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines );
void strip_context_line( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines );
void fold_stmt_rollback_line( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines );
void fold_stmt_commit_line( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines );
void strip_exec_name( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines );
void abstract_equal_number( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines );
void fold_debug_line( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines );
void fold_error_line( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines );
void strip_redundant_secondary_line( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines );
void abstract_statement_names( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines );
void abstract_unnamed( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines );
void abstract_threshold( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines );
void abstract_vac_anl( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines );
void abstract_log_file_identifier(  std::vector<std::vector<TokenWordPair> *> &tokens_in_lines );
void abstract_post_int( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines );
void abstract_pre_int(  std::vector<std::vector<TokenWordPair> *> &tokens_in_lines );
void abstract_int_index(  std::vector<std::vector<TokenWordPair> *> &tokens_in_lines );
void abstract_int_equal(  std::vector<std::vector<TokenWordPair> *> &tokens_in_lines );
void abstract_slash_numbers( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines );
void strip_intermediate_newline( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines );


*/
uint64_t get_pg_thread_id_from_parsed_line( std::unique_ptr<std::vector<TokenWordPair>> &line );
void dump_line( std::unique_ptr<std::vector<TokenWordPair>> &line );
uint64_t get_tid_offset_in_line( std::unique_ptr<std::vector<TokenWordPair>> &line );
uint64_t get_pg_thread_id_from_tid_offset( std::unique_ptr<std::vector<TokenWordPair>> &line, uint64_t tid_offset );

#endif
