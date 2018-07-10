#include "rule_applier.h"
#include <chrono>
#include <thread>
#include <iostream>

void RuleApplier::applyRules( std::unique_ptr<ParseBuffer> &buff ) {
    for( unsigned int i = 0; i < buff->ind; i++ ) {
        std::unique_ptr<std::vector<TokenWordPair>> &tokens_in_line = buff->parsed_lines[i];
        for( RuleFunction rf : abstraction_rules ) {
            rf( tokens_in_line.get() );
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
        for( ;; ) {
            std::vector<TokenWordPair> *next_line= generateNextLine( buffer );
            if( !next_line ) {
                break;
            }
            for( RuleFunction rf: abstraction_rules ) {
                rf( next_line );
            }
        }
        
        pbe_out->putNextBuffer( std::move( buffer ) );
    }
}


std::vector<TokenWordPair> *RuleApplier::generateNextLine( std::unique_ptr<ParseBuffer> &buffer ) {
    if( last_line < buffer->ind ) {
        std::vector<TokenWordPair> *tokens_in_line = (buffer->parsed_lines[last_line]).get();
        last_line++;
        return tokens_in_line;
    } 
    return nullptr;
}

void RuleApplier::terminateWhenDoneProcessing() {
    pbe_in->termWhenOutOfBuffers();
    while( !done ) {
        std::this_thread::sleep_for( std::chrono::milliseconds(300) );
    }
}
