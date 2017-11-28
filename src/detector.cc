#include "detector.h"
#include "rules.h"
#include <iostream>
#include <thread>
#include <cmath>

void Detector::startProcessingBuffers() {
    std::thread t( &Detector::processLoop, this );
    t.detach();
}

void Detector::terminateWhenDoneProcessing() {
    pbe_in->termWhenOutOfBuffers();
    while( !done ) {
        std::this_thread::sleep_for( std::chrono::milliseconds(300) );
    }
}

void Detector::processLoop() {
    for( ;; ) {
        std::unique_ptr<ParseBuffer> buffer = std::move( pbe_in->getNextBuffer() );
        if( !buffer ) {
            done = true;
            std::cout << "Detector got null buffer, terminating..." << std::endl;
            return;
        }
        processLinesInBuffer( std::move( buffer ) );
    }
}

static void print_line( std::shared_ptr<std::vector<TokenWordPair>> &line ) {
    for( const auto &twp : *line ) {
        std::cout << twp.word << " ";
    }
    std::cout << std::endl;
}

void Detector::processLinesInBuffer( std::unique_ptr<ParseBuffer> buffer ) {
    for( unsigned int i = 0; i < buffer->ind; i++ ) {
        std::shared_ptr<std::vector<TokenWordPair>> line( std::move( buffer->parsed_lines[i] ) );
        uint64_t thread_id = get_thread_id_from_parsed_line( line );
        std::shared_ptr<std::vector<TokenWordPair>> last_line = last_lines.getLastLine( thread_id );

        //std::cout << "Processing Line: " << line_counter << std::endl;
        //std::cout << "Perceived Thread: " << thread_id << std::endl;
        //print_line( line );
        //if( last_line != nullptr ) {
            //std::cout << "Perceived Last Line: ";
            //print_line( last_line );
        //} else {
            //std::cout << "No previous line." << std::endl;
        //}


        //Check transition to see if unlikely
        if( last_line != nullptr ) {
            BinKey last_line_bk = Bin::makeBinKeyForLine( last_line );
            auto found_bin_entry = bin_map.find( last_line_bk );
            //Can't miss on this, should already be inserted
            assert( found_bin_entry != bin_map.end() );
            Bin &bin = found_bin_entry->second;
            LineWithTransitions *lwt = bin.findEntryInBin( last_line );
            assert( lwt != nullptr );

            std::pair<bool,double> is_outlier = lwt->isOutlier( line );
            if( is_outlier.first ) {
                std::cout << is_outlier.second << ": Found outlier line: " << line_counter << std::endl;
                print_line( line );
            }
        }
        last_lines.addNewLine( thread_id, line );
        line_counter++;

    }
    //Buffer is destroyed
}
