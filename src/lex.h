#ifndef __LEX_H__
#define __LEX_H__
#include <unistd.h>

// Lexer for log files. Tokenizes them into the appropriate characters.
// Goes line by line.
class Lexer {
    char buff[512];
    char disk_sector[4096];
    int fd;

    public:
    Lexer( int fd ) : fd( fd ) {}
    ~Lexer( ) {
        close( fd );
    }

    void nextChunk();

};
#endif
