#ifndef __TOKEN_H__
#define __TOKEN_H__
#include <string>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/string.hpp>

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
    friend class boost::serialization::access;

    Token       tok;
    std::string word;
    TokenWordPair() { }
    TokenWordPair( const TokenWordPair &other ) {
        tok = other.tok;
        word = other.word;
    }
    TokenWordPair( Token &tok, std::string &word ) : tok( tok ), word( word ) { }

    template<class Archive>
        void serialize( Archive &ar, const unsigned int version ) {
            ar & tok;
            ar & word;
        }
};
#endif
