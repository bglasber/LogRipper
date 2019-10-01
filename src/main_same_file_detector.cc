#include "rules.h"
#include "pg_rules.h"
#include "binner.h"
#include "detector.h"
#include "rule_applier.h"
#include "file_reader.h"
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <gflags/gflags.h>

DEFINE_string( in_file_name, "ssServer.log.short", "Input file name" );
DEFINE_string( map_file, "deserialized_map", "Deserialized Map file" );

int main( int argc, char **argv ) {

    gflags::ParseCommandLineFlags( &argc, &argv, true );

    int fd = open( FLAGS_in_file_name.c_str(), O_RDONLY );
    assert( fd > 0 );

    //Read the deserialized map
    std::ifstream is;
    is.open( FLAGS_map_file.c_str(), std::ifstream::in );
    boost::archive::text_iarchive iarch( is );
    std::unordered_map<BinKey, Bin, BinKeyHasher> bin_map;
    iarch >> bin_map;

    ParseBufferEngine pbe_file_to_rule;
    ParseBufferEngine pbe_rule_to_detector;
    FileReader reader( fd, 128u, &pbe_file_to_rule );

    std::list<RuleFunction> rule_funcs;
    rule_funcs.push_back( anonymize_pg_preamble );

    /* For GLOG
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
    RuleApplier rule_applier( std::move( rule_funcs ), &pbe_file_to_rule, &pbe_rule_to_detector );

    //Make a detector from the old map
    Detector detector( &pbe_rule_to_detector, std::move( bin_map ) );

    //Start the pipeline...
    detector.startProcessingBuffers();
    rule_applier.startProcessingBuffers();
    reader.processFile();
    std::cout << "Done processing file." << std::endl;

    rule_applier.terminateWhenDoneProcessing();
    std::cout << "Done waiting for rule applier..." << std::endl;
    detector.terminateWhenDoneProcessing();
    std::cout << "Done waiting for binner..." << std::endl;

    close( fd );
}
