#ifndef __RULES_H__
#define __RULES_H__
#include "token.h"
#include <vector>
#include <memory>

void anonymize_glog_preamble( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines );
void word_colon_number_anonymize( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines );
void word_colon_space_number_anonymize( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines );
void anonymize_array_indexes( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines );
void anonymize_equal_signs( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines );
void anonymize_client_ids( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines );
void anonymize_location_ids( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines );
void anonymize_write_list( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines );
void anonymize_decimal( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines );
void abstract_from_for_number( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines );
void abstract_destination_site( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines );
void abstract_hostname( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines );
void abstract_millis( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines );
void abstract_bucket_line1( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines );
void abstract_client_locks_number( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines );

uint64_t get_thread_id_from_parsed_line( std::vector<TokenWordPair> *line );
#endif
