#include "token.h"
#include "file_reader.h"
#include "rule_applier.h"
#include "binner.h"
#include "rules.h"
#include "pg_rules.h"
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>

int main() {

    int fd = open( "ssServer.log.short", O_RDONLY ); 
    assert( fd > 0 );
    ParseBufferEngine pbe_file_to_rule;
    ParseBufferEngine pbe_rule_to_binner;
    FileReader reader( fd, 128u, &pbe_file_to_rule );

    std::list<RuleFunction> rule_funcs;
    rule_funcs.push_back( strip_location_line );
    rule_funcs.push_back( strip_detail_line );
    rule_funcs.push_back( strip_hint_line );
    //TODO: it could be the case that we actually care about the context,
    //but lets assume we don't until proven otherwise
    rule_funcs.push_back( strip_context_line );
    rule_funcs.push_back( fold_stmt_rollback_line );
    rule_funcs.push_back( fold_stmt_commit_line );
    rule_funcs.push_back( anonymize_pg_preamble );


    //For GLOG
    /*
    rule_funcs.push_back( anonymize_glog_preamble );
    rule_funcs.push_back( word_colon_number_anonymize );
    rule_funcs.push_back( word_colon_space_number_anonymize );
    rule_funcs.push_back( anonymize_array_indexes );
    rule_funcs.push_back( anonymize_equal_signs );
    rule_funcs.push_back( anonymize_client_ids );
    rule_funcs.push_back( anonymize_location_ids );
    rule_funcs.push_back( anonymize_write_list );
    rule_funcs.push_back( anonymize_decimal );
    rule_funcs.push_back( abstract_from_for_number );
    rule_funcs.push_back( abstract_hostname );
    rule_funcs.push_back( abstract_millis );
    rule_funcs.push_back( abstract_bucket_line1 );
    rule_funcs.push_back( abstract_client_locks_number );
    rule_funcs.push_back( abstract_destination_site );
    */
    RuleApplier rule_applier( std::move( rule_funcs ), &pbe_file_to_rule, &pbe_rule_to_binner );

    Binner binner( &pbe_rule_to_binner );

    //Start the pipeline...
    binner.startProcessingBuffers();
    rule_applier.startProcessingBuffers();
    reader.processFile();
    std::cout << "Done processing file." << std::endl;

    rule_applier.terminateWhenDoneProcessing();
    std::cout << "Done waiting for rule applier..." << std::endl;
    binner.terminateWhenDoneProcessing();
    std::cout << "Done waiting for binner..." << std::endl;

    std::unordered_map<BinKey, Bin, BinKeyHasher> &map = binner.getUnderlyingMap();
    std::cout << "Found " << map.size() << " buckets in the map." << std::endl;

    unsigned num_unique_entries = 0;
    for( auto it = map.begin(); it != map.end(); it++ ) {
        std::vector<LineWithTransitions> &vec = it->second.getBinVector();
        std::cout << "Bucket Entries: " << std::endl;
        for( auto it2 = vec.begin(); it2 != vec.end(); it2++ ) {
            std::vector<TokenWordPair> &l = *(it2->getLine());
            for( auto it3 = l.begin(); it3 != l.end(); it3++ ) {
                std::cout << it3->word << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
        num_unique_entries += vec.size();
    }
    std::cout << "Found " << num_unique_entries << " unique entries in the map." << std::endl;

    std::ofstream os;
    os.open( "deserialized_map", std::ofstream::out );
    binner.serialize( os );
    return 0;
}
