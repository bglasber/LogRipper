#include "binner.h"
#include <cassert>
#include <thread>
#include <iostream>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

void LineTransitions::addTransition( std::vector<TokenWordPair> &other_line ) {
    LineKey lk( other_line );
    auto search = transitions.find( lk );
    if( search != transitions.end() ) {
        std::pair<std::vector<TokenWordPair>, uint64_t> &entry = search->second;
        entry.second++;
    } else {
        auto inner_pair = std::make_pair( other_line, 1 );
        auto outer_pair = std::make_pair( std::move( lk ), std::move( inner_pair ) );
        transitions.emplace( std::move( outer_pair ) );
    }
}

void LineWithTransitions::addTransition( std::vector<TokenWordPair> &other_line ) {
    times_seen++;
    return lt.addTransition( other_line );
}

uint64_t LineTransitions::getTransitionCount( std::vector<TokenWordPair> &line ) {
    LineKey lk( line );
    auto search = transitions.find( lk );
    if( search != transitions.end() ) {
        std::pair<std::vector<TokenWordPair>, uint64_t> &entry = search->second;
        return entry.second;
    }
    return 0;
}

double LineWithTransitions::getTransitionProbability( std::vector<TokenWordPair> &line ) {
    if( times_seen != 0 ) {
        return (double) lt.getTransitionCount( line ) / times_seen;
    }
    return 0;
}

std::vector<TokenWordPair> *LastLineForEachThread::getLastLine( uint64_t thread_id ) {
    auto search = last_lines.find( thread_id );
    if( search != last_lines.end() ) {
        return &(search->second);
    }
    return nullptr;
}

void LastLineForEachThread::addNewLine( uint64_t thread_id, std::vector<TokenWordPair> &line ) {
    std::vector<TokenWordPair> line_copy( line );
    last_lines.insert_or_assign( std::move( thread_id ), std::move( line_copy ) );
}

void Bin::insertIntoBin( std::vector<TokenWordPair> *line, std::vector<TokenWordPair> *last_line ) {
    bool found_match = false;

    //Build the transition structure if it doesn't exist
    for( LineWithTransitions &line_in_bucket : unique_entries_in_bin ) {
        found_match = abstracted_line_match( &line_in_bucket.getLine(), line );
        if( found_match ) {
            break;
        }
    }
    if( !found_match ) {
        //Copy line in
        LineWithTransitions lwt( *line );
        unique_entries_in_bin.emplace_back( std::move( lwt ) );
    }
}

LineWithTransitions *Bin::findEntryInBin( std::vector<TokenWordPair> *line ) {
    for( LineWithTransitions &line_in_bucket : unique_entries_in_bin ) {
        if( abstracted_line_match( &line_in_bucket.getLine(), line ) ) {
            return &line_in_bucket;
        }
    }
    return nullptr;
}

bool abstracted_line_match( const std::vector<TokenWordPair> *line1, const std::vector<TokenWordPair> *line2 ) {
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

static uint64_t get_thread_id_from_parsed_line( std::vector<TokenWordPair> *line ) {
    if( line->size() < 12 ) {
        return 0;
    }
    return atoi(line->at(11).word.c_str());
}

BinKey Bin::makeBinKeyForLine( std::vector<TokenWordPair> *line ) {
    BinKey bk;
    unsigned tot_words = 0;
    unsigned tot_params = 0;
    for( unsigned int line_ind = 0; line_ind < line->size(); line_ind++ ) {
        if( line->at(line_ind).tok == WORD ) {
            tot_words++;
        } else if( line->at(line_ind).tok == ABSTRACTED_VALUE ) {
            tot_params++;
        }
    }
    bk.num_words = tot_words;
    bk.num_params = tot_params;
    return bk;
}

void Binner::binEntriesInBuffer( ParseBuffer *buffer ) {
    for( unsigned int i = 0; i < buffer->ind; i++ ) {
        std::vector<TokenWordPair> *line = buffer->parsed_lines[i];
        BinKey bk = Bin::makeBinKeyForLine( line );
        uint64_t thread_id = get_thread_id_from_parsed_line( line );
        std::vector<TokenWordPair> *last_line = last_lines.getLastLine( thread_id );
        auto found_bin_entry = bin_map.find( bk );

        if( found_bin_entry != bin_map.end() ) {
            Bin &bin = found_bin_entry->second;
            bin.insertIntoBin( line, last_line );
        } else {
            //Moved from objects get deconstructed, so we put the line in
            //on the "moved" object.
            Bin bin;
            bin_map.emplace( std::make_pair<BinKey, Bin>( std::move( bk ), std::move( bin ) ) );
            auto search = bin_map.find( bk );
            assert( search != bin_map.end() );
            search->second.insertIntoBin( line, last_line );
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
            lwt->addTransition( *line );
        }

        //Update last found line
        last_lines.addNewLine( get_thread_id_from_parsed_line( line ), *line );

        //Destroy line
        delete line;
    }
    delete buffer;
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
        ParseBuffer *buffer = pbe_in->getNextBuffer();
        if( !buffer ) {
            done = true;
            std::cout << "Binner got null buffer, terminating..." << std::endl;
            return;
        }
        binEntriesInBuffer( buffer );
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
