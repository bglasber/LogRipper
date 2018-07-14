#include "rule_applier.h"
#include "pg_rules.h"
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

        //push the chunk into the parse buffer
        //parse buffers only accept a single line, so we need to merge down
        //Copy so we can throw away the old buffer
        assert( next_chunk.size() == 1 );

        //We own this ptr now
        std::unique_ptr<std::vector<TokenWordPair>> new_chunk( next_chunk.at(0) );
        if( output_buffer->ind == LINES_IN_BUFFER ) {
            //Out of space, push this, make new chunk
            //TODO push
            pbe_out->putNextBuffer( std::move( output_buffer ) );
            output_buffer = std::make_unique<ParseBuffer>();
        }
        output_buffer->addLine( std::move( new_chunk ) );
    }

}

void RuleApplier::startProcessingBuffers() {
    std::thread t( &RuleApplier::processLoop, this );
    t.detach();
}

void RuleApplier::processLoop() {
    for( ;; ) {
        //We own this ptr now.
        std::unique_ptr<ParseBuffer> buffer = std::move( pbe_in->getNextBuffer() );

        if( !buffer ) {
            done = true;
            std::cout << "Rule Applier got null buffer, putting last buffer..." << std::endl;
            pbe_out->putNextBuffer( std::move( output_buffer ) );
            std::cout << "Put last buffer, terminating..." << std::endl;
            return;
        }
        applyRules( buffer );
        //Buffer is destroyed

    }
}


std::vector<std::vector<TokenWordPair> *> RuleApplier::generateNextChunk( std::unique_ptr<ParseBuffer> &buffer ) {
    while( last_line < buffer->ind ) {
        std::vector<TokenWordPair> *tokens_in_line = (buffer->parsed_lines[last_line]).get();
        last_line++;

        uint64_t tid_offset = get_tid_offset_in_line( tokens_in_line );
        bool is_location_line = false;

        is_location_line = tokens_in_line->at( tid_offset + 4 ).word == "LOCATION";

        uint64_t tid = get_pg_thread_id_from_tid_offset( tokens_in_line, tid_offset );
        if( !is_location_line ) {
            auto search = internal_buffers.find( tid );
            if( search == internal_buffers.end() ){
                std::pair<uint64_t, std::list<std::vector<TokenWordPair> *>> entry = {};
                entry.first = tid;
                internal_buffers.insert( std::move( entry ) );
                search = internal_buffers.find( tid );
            }
            auto &internal_buffer = search->second;
            internal_buffer.push_back( new std::vector<TokenWordPair>( *tokens_in_line ) );
            continue;
        }

        //Merge last few lines into a big one and then return
        //Since the first chunk is owned by the buffer, it should be gc'd when the buffer is destroyed
        //So just ram more stuff in it
        std::vector<std::vector<TokenWordPair> *> chunk;

        auto search = internal_buffers.find( tid );
        if( search != internal_buffers.end() ) {
            auto& internal_buffer = search->second;
            chunk.reserve( internal_buffer.size() + 1 );
            while( !internal_buffer.empty() ) {
                std::vector<TokenWordPair> *next_line = internal_buffer.front();
                uint64_t read_tid = get_pg_thread_id_from_parsed_line( next_line );
                if( read_tid != tid ) {
                    dump_line( next_line );
                    assert( false );
                }
                internal_buffer.pop_front();
                chunk.push_back( next_line );
            }
        }
        chunk.push_back( new std::vector<TokenWordPair>( *tokens_in_line ) );
        return chunk;
    } 
    last_line = 0;
    std::vector<std::vector<TokenWordPair> *> empty;
    return empty;
}

void RuleApplier::terminateWhenDoneProcessing() {
    pbe_in->termWhenOutOfBuffers();
    while( !done ) {
        std::this_thread::sleep_for( std::chrono::milliseconds(300) );
    }
}

