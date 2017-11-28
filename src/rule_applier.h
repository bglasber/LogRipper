#include "parse_buffer.h"
#include <memory>
#include <list>

typedef void (*RuleFunction)( std::vector<TokenWordPair> *tokens_in_line );

class RuleApplier {
    std::list<RuleFunction> abstraction_rules;
    ParseBufferEngine *pbe_in;
    ParseBufferEngine *pbe_out;
    volatile bool done;
    void processLoop();
public:
    RuleApplier( std::list<RuleFunction> &&abstraction_rules, ParseBufferEngine *pbe_in, ParseBufferEngine *pbe_out ) :
        abstraction_rules( abstraction_rules ),
        pbe_in( pbe_in ),
        pbe_out( pbe_out ),
        done( false ) {}
    void applyRules( std::unique_ptr<ParseBuffer> &buffer );
    void startProcessingBuffers();
    void terminateWhenDoneProcessing();
};
