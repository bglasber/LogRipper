#include "rule_applier.h"
#include <chrono>
#include <thread>
#include <iostream>

void RuleApplier::applyRules( ParseBuffer *buff ) {
    for( unsigned int i = 0; i < buff->ind; i++ ) {
        std::vector<TokenWordPair> *tokens_in_line = buff->parsed_lines[i];
        for( RuleFunction rf : abstraction_rules ) {
            rf( tokens_in_line );
        }
    }
}

void RuleApplier::startProcessingBuffers() {
    std::thread t( &RuleApplier::processLoop, this );
    t.detach();
}

void RuleApplier::processLoop() {
    for( ;; ) {
        ParseBuffer *buffer = pbe_in->getNextBuffer();
        if( !buffer ) {
            done = true;
            std::cout << "Rule Applier got null buffer, terminating..." << std::endl;
            return;
        }
        std::cout << "RAH got another buffer..." << std::endl;
        applyRules( buffer );

        pbe_out->putNextBuffer( buffer );
    }
}

void RuleApplier::terminateWhenDoneProcessing() {
    pbe_in->termWhenOutOfBuffers();
    while( !done ) {
        std::this_thread::sleep_for( std::chrono::milliseconds(300) );
    }
}
