#include "token.h"
#include "parse_buffer.h"
#include <cstdint>
#include <unordered_map>

struct BinKey {
    unsigned num_words;
    unsigned num_params;
    bool operator==( const BinKey &other ) const {
        return num_words == other.num_words &&
               num_params == other.num_params;
    }
};

struct BinKeyHasher {
    std::size_t operator()( const BinKey &bk ) const {
        return std::hash<unsigned>()( bk.num_words ) ^ (
               std::hash<unsigned>()( bk.num_params ) << 7 );
    }
};


class Bin {
    std::vector<std::vector<TokenWordPair> *> unique_entries_in_bin;
public:
    void insertIntoBin( std::vector<TokenWordPair> *line );
    bool vectorMatch( std::vector<TokenWordPair> *line1, std::vector<TokenWordPair> *line2 );
};

class Binner {
    std::unordered_map<BinKey, Bin, BinKeyHasher> bin_map;
    ParseBufferEngine *pbe_in;

public:
    Binner( ParseBufferEngine *pbe_in ) : pbe_in( pbe_in ) {}
    void binEntriesInBuffer( ParseBuffer *buffer );
};
