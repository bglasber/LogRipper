#include "binner.h"
#include <cassert>
#include <thread>
#include <iostream>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

void LineTransitions::addTransition( std::vector<TokenWordPair> *other_line ) {
    LineKey lk( *other_line );
    auto search = transitions.find( lk );
    if( search != transitions.end() ) {
        std::pair<std::vector<TokenWordPair>, uint64_t> &entry = search->second;
        entry.second++;
    } else {
        auto inner_pair = std::make_pair( *other_line, 1 );
        auto outer_pair = std::make_pair( std::move( lk ), std::move( inner_pair ) );
        transitions.emplace( std::move( outer_pair ) );
    }
}

void LineWithTransitions::addTransition( std::vector<TokenWordPair> *other_line ) {
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

//Destroying bins like this breaks deserialization if you put it in the default destructor
//Bypass it by making a separate function
void Bin::destroyBinEntries( ) {
    for( unsigned int i = 0; i < unique_entries_in_bin.size(); i++ ) {
        delete unique_entries_in_bin.at(i);
    }
}

void Bin::insertIntoBin( std::vector<TokenWordPair> *line ) {
    bool found_match = false;
    for( std::vector<TokenWordPair> *line_in_bucket : unique_entries_in_bin ) {
        found_match = vectorMatch( line_in_bucket, line );
        if( found_match ) {
            break;
        }
    }
    if( !found_match ) {
        unique_entries_in_bin.push_back( line );
    } else {
        //There's already one of these, destroy the string
        delete line;
    }
}

bool Bin::vectorMatch( std::vector<TokenWordPair> *line1, std::vector<TokenWordPair> *line2 ) {
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

void Binner::binEntriesInBuffer( ParseBuffer *buffer ) {
    for( unsigned int i = 0; i < buffer->ind; i++ ) {
        std::vector<TokenWordPair> *line = buffer->parsed_lines[i];
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

        auto found = bin_map.find( bk );
        if( found != bin_map.end() ) {
            Bin &bin = found->second;
            bin.insertIntoBin( line );
        } else {
            //Moved from objects get deconstructed, so we put the line in
            //on the "moved" object.
            Bin bin;
            bin_map.emplace( std::make_pair<BinKey, Bin>( std::move( bk ), std::move( bin ) ) );
            auto search = bin_map.find( bk );
            assert( search != bin_map.end() );
            search->second.insertIntoBin( line );
        }
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
    for( auto it = bin_map.begin(); it != bin_map.end(); it++ ) {
        it->second.destroyBinEntries();
    }
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
