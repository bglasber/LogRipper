#include "rules.h"
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

int main() {

    int fd = open( "ssServer.log.short", O_RDONLY );
    assert( fd > 0 );

    //Read the deserialized map
    std::ifstream is;
    is.open( "deserialized_map", std::ifstream::in );
    boost::archive::text_iarchive iarch( is );
    std::unordered_map<BinKey, Bin, BinKeyHasher> bin_map;
    iarch >> bin_map;

    ParseBufferEngine pbe_file_to_rule;
    ParseBufferEngine pbe_rule_to_detector;
    FileReader reader( fd, 128u, &pbe_file_to_rule );

    std::list<RuleFunction> rule_funcs;
    rule_funcs.push_back( anonymize_log_preamble );
    rule_funcs.push_back( anonymize_size_of_page );
    rule_funcs.push_back( anonymize_category_id );
    rule_funcs.push_back( anonymize_item_id );
    rule_funcs.push_back( anonymize_product_id );
    rule_funcs.push_back( anonymize_working_item_id );
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
