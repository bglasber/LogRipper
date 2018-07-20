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

DEFINE_string( in_file_name, "deserialized_map", "Input map file name" );

int main( int argc, char **argv ) {

    gflags::ParseCommandLineFlags( &argc, &argv, true );

    //Read the deserialized map
    std::ifstream is;
    is.open( FLAGS_in_file_name.c_str(), std::ifstream::in );
    boost::archive::text_iarchive iarch( is );
    std::unordered_map<BinKey, Bin, BinKeyHasher> bin_map;
    iarch >> bin_map;

    uint64_t tot_times_seen = 0;
    for( auto &ent : bin_map ) {
        Bin &bin = ent.second;
        std::vector<LineWithTransitions> &vec = bin.getBinVector();
        for( auto &lwt : vec ) {
            tot_times_seen += lwt.getTimesSeen();
        }
    }

    
    LineKeyHasher lkh;
    for( auto &ent : bin_map ) {
        Bin &bin = ent.second;
        std::vector<LineWithTransitions> &vec = bin.getBinVector();
        for( auto &lwt : vec ) {
            const std::shared_ptr<std::vector<TokenWordPair>> &line = lwt.getLine();
            uint64_t times_seen = lwt.getTimesSeen();
            double prob = (double) times_seen / tot_times_seen;
            LineKey lk( line );
            std::cout << lkh( lk ) << ": " << prob << ": ";

            for( const auto &twp : *line ) {
                std::cout << twp.word << " ";
            }
            std::cout << std::endl;
        }
    }
    std::cout << std::endl;
    for( auto &ent : bin_map ) {
        Bin &bin = ent.second;
        std::vector<LineWithTransitions> &vec = bin.getBinVector();
        for( auto &lwt : vec ) {
            std::shared_ptr<std::vector<TokenWordPair>> &line = lwt.getLine();
            LineKey lk( line );
            uint64_t key = lkh( lk );
            LineTransitions &lt = lwt.getLineTransitions();
            uint64_t times_seen = lwt.getTimesSeen();
            for( auto &transition : lt.getTransitions() ) {
                uint64_t transition_key = lkh( transition.first );
                uint64_t count = transition.second.second;
                std::cout << key << " -> " << transition_key << ": " << (double) count / times_seen << std::endl;
            }
        }
    }

}

