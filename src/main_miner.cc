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
#include <gflags/gflags.h>

DEFINE_string( in_file_name, "ssServer.log.short", "Input file name" );
DEFINE_string( out_file_name, "deserialized_map", "Output map file" );

int main( int argc, char **argv ) {

    gflags::ParseCommandLineFlags(&argc, &argv, true);

    int fd = open( FLAGS_in_file_name.c_str(), O_RDONLY ); 
    assert( fd > 0 );
    ParseBufferEngine pbe_file_to_rule;
    ParseBufferEngine pbe_rule_to_binner;
    FileReader reader( fd, 128u, &pbe_file_to_rule );

    std::list<RuleFunction> rule_funcs;
    rule_funcs.push_back( anonymize_pg_preamble );
    rule_funcs.push_back( abstract_parse_line );
    rule_funcs.push_back( abstract_name_line );
    rule_funcs.push_back( delete_at_character );
    rule_funcs.push_back( delete_after_stats_file );
    rule_funcs.push_back( abstract_equal_number );
    rule_funcs.push_back( abstract_brackets );
    rule_funcs.push_back( abstract_callbacks );
    rule_funcs.push_back( abstract_vac_anl );
    rule_funcs.push_back( abstract_vacuum_analyze_lines );
    rule_funcs.push_back( abstract_removable_lines );
    rule_funcs.push_back( abstract_live_dead_lines );
    rule_funcs.push_back( abstract_index_count_rows );
    rule_funcs.push_back( abstract_writing_block );
    rule_funcs.push_back( abstract_removed_page );
    rule_funcs.push_back( abstract_pid );

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
    os.open( FLAGS_out_file_name.c_str(), std::ofstream::out );
    binner.serialize( os );
    return 0;
}
