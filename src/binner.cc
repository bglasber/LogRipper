#include "binner.h"
#include "rules.h"
#include <cassert>
#include <thread>
#include <iostream>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

void LineTransitions::addTransition( std::shared_ptr<std::vector<TokenWordPair>> other_line ) {
    LineKey lk( other_line );
    auto search = transitions.find( lk );
    if( search != transitions.end() ) {
        std::pair<std::shared_ptr<std::vector<TokenWordPair>>, uint64_t> &entry = search->second;
        entry.second++;
    } else {
        auto inner_pair = std::make_pair( other_line, 1 );
        auto outer_pair = std::make_pair( std::move( lk ), std::move( inner_pair ) );
        transitions.emplace( std::move( outer_pair ) );
    }
}

uint64_t LineTransitions::getTransitionCount( std::shared_ptr<std::vector<TokenWordPair>> &line ) {
    LineKey lk( line );
    auto search = transitions.find( lk );
    if( search != transitions.end() ) {
        std::pair<std::shared_ptr<std::vector<TokenWordPair>>, uint64_t> &entry = search->second;
        return entry.second;
    }
    return 0;
}

static void print_line( std::shared_ptr<std::vector<TokenWordPair>> &line ) {
    for( const auto &twp : *line ) {
        std::cout << twp.word << " ";
    }
    std::cout << std::endl;
}

std::pair<bool,double> LineTransitions::isOutlier( std::shared_ptr<std::vector<TokenWordPair>> &transition_line, uint64_t total_transitions ) {
    //Find entry with max count
    uint64_t max_count = 0;
    std::shared_ptr<std::vector<TokenWordPair>> dominant_follower;
    for( auto iter = transitions.begin(); iter != transitions.end(); iter++ ) {
        uint64_t count = iter->second.second;
        if( count > max_count ) {
            max_count = count;
            dominant_follower = iter->second.first;
        }
    }
    //std::cout << "Got max count: " << max_count;
    //std::cout << "Got total transitions: " << total_transitions;

    uint64_t this_line_count = getTransitionCount( transition_line );
    if( this_line_count == max_count ) {
        //We're the dominant behaviour, no need to report
        return std::make_pair( false, 0.0 );
    }

    //If the z-stat is > 0, then the dominant behaviour happens more than 90% of the time
    double z_stat_top = ((double) max_count / total_transitions) - 0.9;

    if( z_stat_top < 0 ) {
        //std::cout << "not the dominant behaviour, but got z-stat-top of: " << z_stat_top << std::endl;
        return std::make_pair( false, 0.0 );
    }

    double z_stat = z_stat_top / sqrt((0.9 * (1-0.9))/total_transitions);
    std::cout << "Found outlier, got z-stat of: " << z_stat << std::endl;
    std::cout << "Count: " << this_line_count << ", Total: " << total_transitions << std::endl;
    std::cout << "Dominant Count: " << max_count << ", Total: " << total_transitions << std::endl;
    std::cout << "Next Line: " << std::endl;
    print_line( transition_line );
    std::cout << "Expected to be: " << std::endl;
    print_line( dominant_follower );

    return std::make_pair( true, z_stat );
}

void LineWithTransitions::addTransition( std::shared_ptr<std::vector<TokenWordPair>> other_line ) {
    times_seen++;
    return lt.addTransition( other_line );
}

double LineWithTransitions::getTransitionProbability( std::shared_ptr<std::vector<TokenWordPair>> &line ) {
    if( times_seen != 0 ) {
        return (double) lt.getTransitionCount( line ) / times_seen;
    }
    return 0;
}

std::shared_ptr<std::vector<TokenWordPair>> LastLineForEachThread::getLastLine( uint64_t thread_id ) {
    auto search = last_lines.find( thread_id );
    if( search != last_lines.end() ) {
        return search->second;
    }
    return nullptr;
}

void LastLineForEachThread::addNewLine( uint64_t thread_id, std::shared_ptr<std::vector<TokenWordPair>> line ) {
    last_lines.insert_or_assign( std::move( thread_id ), line );
}

void Bin::insertIntoBin( std::shared_ptr<std::vector<TokenWordPair>> line, std::shared_ptr<std::vector<TokenWordPair>> &last_line ) {
    bool found_match = false;

    //Build the transition structure if it doesn't exist
    for( LineWithTransitions &line_in_bucket : unique_entries_in_bin ) {
        found_match = abstracted_line_match( line_in_bucket.getLine(), line );
        if( found_match ) {
            break;
        }
    }
    if( !found_match ) {
        //Copy line in
        LineWithTransitions lwt( std::move( line ) );
        unique_entries_in_bin.emplace_back( std::move( lwt ) );
    }
}

LineWithTransitions *Bin::findEntryInBin( std::shared_ptr<std::vector<TokenWordPair>> &line ) {
    for( LineWithTransitions &line_in_bucket : unique_entries_in_bin ) {
        if( abstracted_line_match( line_in_bucket.getLine(), line ) ) {
            return &line_in_bucket;
        }
    }
    return nullptr;
}

bool abstracted_line_match( const std::shared_ptr<std::vector<TokenWordPair>> &line1, const std::shared_ptr<std::vector<TokenWordPair>> &line2 ) {
    if( line1->size() != line2->size() ) {
        //Hash collision?
        return false;
    }
    for( unsigned int i = 0; i < line1->size(); i++ ) {
        //Mismatch tokens, or mismatch a word entry
        if( line1->at(i).tok != line2->at(i).tok ||
            ( (line1->at(i).tok == WORD || line1->at(i).tok == NUMBER) && (line1->at(i).word != line2->at(i).word) ) ) {
            return false;
        }
    } return true;
}

BinKey Bin::makeBinKeyForLine( std::vector<TokenWordPair> &line ) {
    BinKey bk;
    unsigned tot_words = 0;
    unsigned tot_params = 0;
    for( unsigned int line_ind = 0; line_ind < line.size(); line_ind++ ) {
        if( line.at(line_ind).tok == WORD ) {
            tot_words++;
        } else if( line.at(line_ind).tok == ABSTRACTED_VALUE ) {
            tot_params++;
        }
    }
    bk.num_words = tot_words;
    bk.num_params = tot_params;
    return bk;
}

BinKey Bin::makeBinKeyForLine( std::unique_ptr<std::vector<TokenWordPair>> &line ) {
    return makeBinKeyForLine( *line );
}

BinKey Bin::makeBinKeyForLine( std::shared_ptr<std::vector<TokenWordPair>> &line ) {
    return makeBinKeyForLine( *line );
}

void Binner::binEntriesInBuffer( std::unique_ptr<ParseBuffer> buffer ) {
    for( unsigned int i = 0; i < buffer->ind; i++ ) {
        std::unique_ptr<std::vector<TokenWordPair>> line = std::move( buffer->parsed_lines[i] );

        //At this point, we need to put this line into the last_line position and into the
        //bin (if unique)
        //If unique, then last_line gets destroyed first
        //Else immediately destroyed then last_line needs to live for a bit
        //Easiest to solve this problem with a shared_ptr

        std::shared_ptr<std::vector<TokenWordPair>> shared_line_ptr( std::move( line ) );

        uint64_t thread_id = get_thread_id_from_parsed_line( shared_line_ptr );
        BinKey bk = Bin::makeBinKeyForLine( shared_line_ptr );

        std::shared_ptr<std::vector<TokenWordPair>> last_line = last_lines.getLastLine( thread_id );
        auto found_bin_entry = bin_map.find( bk );

        if( found_bin_entry != bin_map.end() ) {
            Bin &bin = found_bin_entry->second;
            bin.insertIntoBin( shared_line_ptr, last_line );
        } else {
            //Moved from objects get deconstructed, so we put the line in
            //on the "moved" object.
            Bin bin;
            bin_map.emplace( std::make_pair<BinKey, Bin>( std::move( bk ), std::move( bin ) ) );
            auto search = bin_map.find( bk );
            assert( search != bin_map.end() );
            search->second.insertIntoBin( shared_line_ptr, last_line );
        }

        //Find old entry, update it
        if( last_line != nullptr ) {
            BinKey last_line_bk = Bin::makeBinKeyForLine( last_line );
            found_bin_entry = bin_map.find( last_line_bk );
            //Can't miss on this, should already be inserted
            assert( found_bin_entry != bin_map.end() );
            Bin &bin = found_bin_entry->second;
            //This double lookup is painful, but there aren't many records so
            //its probably faster than a second hashtable
            LineWithTransitions *lwt = bin.findEntryInBin( last_line );
            assert( lwt != nullptr );
            lwt->addTransition( shared_line_ptr );
        }

        //Update last found line
        last_lines.addNewLine( thread_id, shared_line_ptr );
    }
}

void Binner::serialize( std::ofstream &os ) {
    boost::archive::text_oarchive oarch( os );
    oarch << bin_map;
}

void Binner::deserialize( std::ifstream &is ) {
    boost::archive::text_iarchive iarch( is );
    iarch >> bin_map;
}

Binner::~Binner() {
}

void Binner::processLoop() {
    for( ;; ) {
        std::unique_ptr<ParseBuffer> buffer = std::move( pbe_in->getNextBuffer() );
        if( !buffer ) {
            done = true;
            std::cout << "Binner got null buffer, terminating..." << std::endl;
            return;
        }
        binEntriesInBuffer( std::move( buffer ) );
    }
}

void Binner::startProcessingBuffers() {
    std::thread t( &Binner::processLoop, this );
    t.detach();
}

void Binner::terminateWhenDoneProcessing() {
    pbe_in->termWhenOutOfBuffers();
    while( !done ) {
        std::this_thread::sleep_for( std::chrono::milliseconds(300) );
    }
}
