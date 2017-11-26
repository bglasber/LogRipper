#ifndef __RULES_H__
#define __RULES_H__
#include "token.h"
#include <vector>


void anonymize_glog_preamble( std::vector<TokenWordPair> *tokens_in_line );
void word_colon_number_anonymize( std::vector<TokenWordPair> *tokens_in_line );
void word_colon_space_number_anonymize( std::vector<TokenWordPair> *tokens_in_line );
void anonymize_array_indexes( std::vector<TokenWordPair> *tokens_in_line );
void anonymize_equal_signs( std::vector<TokenWordPair> *tokens_in_line );
void anonymize_client_ids( std::vector<TokenWordPair> *tokens_in_line );
void anonymize_location_ids( std::vector<TokenWordPair> *tokens_in_line );
void anonymize_write_list( std::vector<TokenWordPair> *tokens_in_line );
void anonymize_decimal( std::vector<TokenWordPair> *tokens_in_line );
void abstract_from_for_number( std::vector<TokenWordPair> *tokens_in_line );
void abstract_destination_site( std::vector<TokenWordPair> *tokens_in_line );
void abstract_hostname( std::vector<TokenWordPair> *tokens_in_line );
void abstract_millis( std::vector<TokenWordPair> *tokens_in_line );
void abstract_bucket_line1( std::vector<TokenWordPair> *tokens_in_line );
void abstract_client_locks_number( std::vector<TokenWordPair> *tokens_in_line );

uint64_t get_thread_id_from_parsed_line( std::vector<TokenWordPair> *line );
#endif
