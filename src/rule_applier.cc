#include "rule_applier.h"

void RuleApplier::applyRules( ParseBuffer *buff ) {
    for( unsigned int i = 0; i < buff->ind; i++ ) {
        std::vector<TokenWordPair> *tokens_in_line = buff->parsed_lines[i];
        for( RuleFunction rf : abstraction_rules ) {
            rf( tokens_in_line );
        }
    }
}

void RuleApplier::startProcessingBuffers() {
    ParseBuffer *buffer = pbe_in->getNextBuffer();
    applyRules( buffer );
    pbe_out->putNextBuffer( buffer );
}


