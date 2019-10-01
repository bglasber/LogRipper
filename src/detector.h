#ifndef __DETECTOR_H__
#define __DETECTOR_H__
#include "binner.h"
#include <memory>
#include <map>

class Detector {
    ParseBufferEngine *pbe_in;
    std::unordered_map<BinKey, Bin, BinKeyHasher> bin_map;
    volatile bool done;
    LastLineForEachThread last_lines;
    uint64_t line_counter;

    void processLoop();
public:
    Detector( ParseBufferEngine *pbe_in, std::unordered_map<BinKey, Bin, BinKeyHasher> &&bin_map ) : pbe_in( pbe_in ), bin_map( bin_map ), done( false ) {
        line_counter = 1;
    }

    void startProcessingBuffers();
    void terminateWhenDoneProcessing();
    void processLinesInBuffer( std::unique_ptr<ParseBuffer> buffer );
};
#endif
