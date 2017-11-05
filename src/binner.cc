#include "binner.h"
#include <cassert>
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
    }
    //There's already one of these, destroy the string
    delete line;
}

bool Bin::vectorMatch( std::vector<TokenWordPair> *line1, std::vector<TokenWordPair> *line2 ) {
    assert( line1->size() == line2->size() );
    for( unsigned int i = 0; i < line1->size(); i++ ) {
        if( line1->at(i).tok != line2->at(i).tok ||
            line1->at(i).word != line2->at(i).word ) {
            return false;
        }
    }
    return true;
}

void Binner::binEntriesInBuffer( ParseBuffer *buffer ) {
    for( unsigned int i = 0; i < buffer->ind; i++ ) {
        std::vector<TokenWordPair> *line = buffer->parsed_lines[i];
        BinKey bk;
        unsigned tot_words;
        unsigned tot_params;
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
        }
    }
}
