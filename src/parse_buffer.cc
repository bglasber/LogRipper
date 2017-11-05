#include "parse_buffer.h"
#include <cassert>

ParseBuffer::ParseBuffer() {
    ind = 0;
}

ParseBuffer::~ParseBuffer() {
    for( unsigned i = 0; i < ind; i++ ) {
        delete( parsed_lines[i] );
    }
}

bool ParseBuffer::addLine( std::vector<TokenWordPair> *line ) {
    assert( ind < LINES_IN_BUFFER );
    parsed_lines[ ind ] = line;
    ind++;
    return ind >= LINES_IN_BUFFER;
}

ParseBufferEngine::ParseBufferEngine() {
}

ParseBufferEngine::~ParseBufferEngine() {
    std::unique_lock<std::mutex> lk( mut );
    for( ParseBuffer *buff : ready_buffers ) {
        delete buff;
    }
    ready_buffers.clear();
}

ParseBuffer *ParseBufferEngine::getNextBuffer() {
    std::unique_lock<std::mutex> lk( mut );
    //Sleep until there are some ready buffers
    cv.wait( lk, [ this ] { return !ready_buffers.empty(); } );
    ParseBuffer *buff = ready_buffers.front();
    ready_buffers.pop_front();
    return buff;
}

void ParseBufferEngine::putNextBuffer( ParseBuffer *buff ) {
    std::unique_lock<std::mutex> lk( mut );
    ready_buffers.push_back( buff );
    cv.notify_one();
}


std::list<ParseBuffer *> &ParseBufferEngine::getReadyBuffers() {
    return ready_buffers;
}
