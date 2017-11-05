#ifndef __PARSE_BUFFER_H__
#define __PARSE_BUFFER_H__
#include "token.h"
#include "config.h"
#include <mutex>
#include <condition_variable>
#include <list>
#include <vector>

//A buffer of pointers to parsed lines
//Used to pass "units of work" between the components
struct ParseBuffer {
    unsigned ind;
    std::vector<TokenWordPair> *parsed_lines[ LINES_IN_BUFFER ];
    ParseBuffer();
    ~ParseBuffer();
    bool addLine( std::vector<TokenWordPair> *line );
};


//Manages passing buffers back and forth using mutex/cv
//Destroys held buffers on destruction
class ParseBufferEngine {
    std::mutex mut;
    std::condition_variable cv;
    std::list<ParseBuffer *> ready_buffers;
public:
    ParseBufferEngine();
    ~ParseBufferEngine();
    ParseBuffer *getNextBuffer();
    void putNextBuffer( ParseBuffer *buff );

    //For unit tests
    std::list<ParseBuffer *> &getReadyBuffers();
};
#endif
