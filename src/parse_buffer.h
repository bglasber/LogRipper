#ifndef __PARSE_BUFFER_H__
#define __PARSE_BUFFER_H__
#include "token.h"
#include "config.h"
#include <mutex>
#include <condition_variable>
#include <list>
#include <vector>
#include <memory>

//A buffer of pointers to parsed lines
//Used to pass "units of work" between the components
struct ParseBuffer {
    unsigned ind;
    std::unique_ptr<std::vector<TokenWordPair>> parsed_lines[ LINES_IN_BUFFER ];
    ParseBuffer();
    void destroyBufferLines();
    bool addLine( std::unique_ptr<std::vector<TokenWordPair>> line );
};


//Manages passing buffers back and forth using mutex/cv
//Destroys held buffers on destruction
class ParseBufferEngine {
    std::mutex mut;
    std::condition_variable cv;
    std::list<std::unique_ptr<ParseBuffer>> ready_buffers;
    bool term_when_out_of_buffers;
public:
    ParseBufferEngine();
    ~ParseBufferEngine();
    std::unique_ptr<ParseBuffer> getNextBuffer();
    void termWhenOutOfBuffers();
    void putNextBuffer( std::unique_ptr<ParseBuffer> buff );

    //For unit tests
    std::list<std::unique_ptr<ParseBuffer>> &getReadyBuffers();
};
#endif
