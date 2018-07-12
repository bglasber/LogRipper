#include "parse_buffer.h"
#include <memory>
#include <list>
#include <utility>
#include <unordered_map>

typedef void (*RuleFunction)( std::vector<std::vector<TokenWordPair> *> &tokens_in_lines );

class RuleApplier {
    std::list<RuleFunction> abstraction_rules;
    ParseBufferEngine *pbe_in;
    ParseBufferEngine *pbe_out;
    std::unordered_map<uint64_t, std::list<std::vector<TokenWordPair> *>> internal_buffers;
    std::unique_ptr<ParseBuffer> output_buffer;
    
    unsigned int last_line;
    volatile bool done;
    void processLoop();
public:
    RuleApplier( std::list<RuleFunction> &&abstraction_rules, ParseBufferEngine *pbe_in, ParseBufferEngine *pbe_out ) :
        abstraction_rules( abstraction_rules ),
        pbe_in( pbe_in ),
        pbe_out( pbe_out ),
        output_buffer( std::make_unique<ParseBuffer>() ),
        last_line( 0 ),
        done( false ) {}
    void applyRules( std::unique_ptr<ParseBuffer> &buffer );
    void startProcessingBuffers();
    void terminateWhenDoneProcessing();
    std::vector<std::vector<TokenWordPair> *> generateNextChunk( std::unique_ptr<ParseBuffer> &buffer );
};
