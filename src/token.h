#ifndef __TOKEN_H__
#define __TOKEN_H__
#include <string>
enum Token {
    START = 0,
    WORD,
    NUMBER,
    WHITE_SPACE,
    PUNCTUATION,
    NEW_LINE,
    ERROR,
    EOF_TOK,
    END,
    ABSTRACTED_VALUE,
    NUM_WORDS
};

struct TokenWordPair {
    Token       tok;
    std::string word;
    TokenWordPair() { }
    TokenWordPair( const TokenWordPair &other ) {
        tok = other.tok;
        word = other.word;
    }
};
#endif
