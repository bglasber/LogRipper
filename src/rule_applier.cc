#include "rule_applier.h"
#include <chrono>
#include <thread>
#include <iostream>

void RuleApplier::applyRules( std::unique_ptr<ParseBuffer> &buff ) {
    for( ;; ) {
        std::vector<std::vector<TokenWordPair> *> next_chunk = generateNextChunk( buff );
        if( next_chunk.empty() ) {
            break;
        }

        //Apply rules to the chunk
        for( RuleFunction rf: abstraction_rules ) {
            rf( next_chunk );
        }
    }
}

void RuleApplier::startProcessingBuffers() {
    std::thread t( &RuleApplier::processLoop, this );
    t.detach();
}

void RuleApplier::processLoop() {
    for( ;; ) {
        std::unique_ptr<ParseBuffer> buffer = std::move( pbe_in->getNextBuffer() );
        if( !buffer ) {
            done = true;
            std::cout << "Rule Applier got null buffer, terminating..." << std::endl;
            return;
        }
        applyRules( buffer );
        pbe_out->putNextBuffer( std::move( buffer ) );
    }
}


std::vector<std::vector<TokenWordPair> *> RuleApplier::generateNextChunk( std::unique_ptr<ParseBuffer> &buffer ) {
    if( last_line < buffer->ind ) {
        std::vector<TokenWordPair> *tokens_in_line = (buffer->parsed_lines[last_line]).get();
        last_line++;

        bool is_location_line = false;
        if( tokens_in_line->size() < 25 ) {
            is_location_line = true;
        } else {
            if( tokens_in_line->at(23).tok == WORD ) {
                if( tokens_in_line->at(23).word == "LOCATION" ) {
                    is_location_line = true;
                }
            } else {
                if( tokens_in_line->at(24).tok == WORD ) {
                    if( tokens_in_line->at(24).word == "LOCATION" ) {
                        is_location_line = true;
                    }
                }
            }
        }
        if( !is_location_line ) {
            internal_buffer.push_back( tokens_in_line );
            std::vector<std::vector<TokenWordPair> *> empty;
            return empty;

        }

        //Merge last few lines into a big one and then return
        //Since the first chunk is owned by the buffer, it should be gc'd when the buffer is destroyed
        //So just ram more stuff in it
        std::vector<std::vector<TokenWordPair> *> chunk;
        chunk.reserve( internal_buffer.size() + 1 );
        while( !internal_buffer.empty() ) {
            std::vector<TokenWordPair> *next_line = internal_buffer.front();
            internal_buffer.pop_front();
            chunk.push_back( next_line );
        }
        chunk.push_back( tokens_in_line );
        return chunk;
    } 
    std::vector<std::vector<TokenWordPair> *> empty;
    return empty;
}

void RuleApplier::terminateWhenDoneProcessing() {
    pbe_in->termWhenOutOfBuffers();
    while( !done ) {
        std::this_thread::sleep_for( std::chrono::milliseconds(300) );
    }
}
