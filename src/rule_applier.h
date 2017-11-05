#include "parse_buffer.h"

typedef void (*RuleFunction)( std::vector<TokenWordPair> *tokens_in_line );

class RuleApplier {
    std::list<RuleFunction> abstraction_rules;
    ParseBufferEngine *pbe_in;
    ParseBufferEngine *pbe_out;
public:
    RuleApplier( std::list<RuleFunction> &&abstraction_rules, ParseBufferEngine *pbe_in, ParseBufferEngine *pbe_out ) :
        abstraction_rules( abstraction_rules ),
        pbe_in( pbe_in ),
        pbe_out( pbe_out ) {}
    void applyRules( ParseBuffer *buffer );
    void startProcessingBuffers();
};
