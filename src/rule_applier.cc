#include "rule_applier.h"
#include "pg_rules.h"
#include <chrono>
#include <thread>
#include <iostream>

void RuleApplier::applyRules( std::unique_ptr<ParseBuffer> &buff ) {
    for( unsigned i = 0; i < buff->ind; i++ ) {
        std::unique_ptr<std::vector<TokenWordPair>> &line = buff->parsed_lines[ i ];
        //Apply rules to the chunk
        for( RuleFunction rf: abstraction_rules ) {
            rf( line );
        }
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
            return;
        }
        applyRules( buffer );
        pbe_out->putNextBuffer( std::move( buffer ) );
    }
}


void RuleApplier::terminateWhenDoneProcessing() {
    pbe_in->termWhenOutOfBuffers();
    while( !done ) {
        std::this_thread::sleep_for( std::chrono::milliseconds(300) );
    }
}

