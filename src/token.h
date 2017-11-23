#ifndef __TOKEN_H__
#define __TOKEN_H__
#include <string>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/string.hpp>
#include <boost/functional/hash.hpp>

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

    bool operator==( const TokenWordPair &other ) const {
        return tok == other.tok && word == other.word;
    }

    template<class Archive>
        void serialize( Archive &ar, const unsigned int version ) {
            ar & tok;
            ar & word;
        }
};

/*
inline std::size_t hash_value( const TokenWordPair &twp ) {
    std::size_t seed = 0;
    boost::hash_combine( seed, twp.tok );
    boost::hash_combine( seed, twp.word );
    return seed;
}
*/

#endif
