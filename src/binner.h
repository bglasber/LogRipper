#include "token.h"
#include "parse_buffer.h"
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <cstdint>
#include <unordered_map>
#include <fstream>

struct BinKey {
    friend class boost::serialization::access;
    template<class Archive>
        void serialize( Archive &ar, const unsigned int version ) {
            ar & num_words;
            ar & num_params;
        }

    BinKey(){}
    /* To reconstruct via deserialization */
    BinKey( const unsigned int &num_words, const unsigned &num_params ) :
        num_words( num_words ),
        num_params( num_params ){ }
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
    friend class boost::serialization::access;
    template <class Archive>
        void serialize( Archive &ar, const unsigned int version ) {
            ar & unique_entries_in_bin;
        }
    std::vector<std::vector<TokenWordPair> *> unique_entries_in_bin;
public:
    Bin() {}
    Bin( std::vector<std::vector<TokenWordPair> *> &unique_entries_in_bin ) : unique_entries_in_bin( unique_entries_in_bin ) {}
    void insertIntoBin( std::vector<TokenWordPair> *line );
    void destroyBinEntries();
    bool vectorMatch( std::vector<TokenWordPair> *line1, std::vector<TokenWordPair> *line2 );
    std::vector<std::vector<TokenWordPair> *> &getBinVector() {
        return unique_entries_in_bin;
    }
};

class Binner {
    std::unordered_map<BinKey, Bin, BinKeyHasher> bin_map;
    ParseBufferEngine *pbe_in;
    volatile bool done;

    void processLoop();

public:
    Binner( ParseBufferEngine *pbe_in ) : pbe_in( pbe_in ), done( false ) {}
    ~Binner();
    void binEntriesInBuffer( ParseBuffer *buffer );

    void serialize( std::ofstream &os );
    void deserialize( std::ifstream &is );
    void startProcessingBuffers();
    void terminateWhenDoneProcessing();

    std::unordered_map<BinKey, Bin, BinKeyHasher> &getUnderlyingMap() {
        return bin_map;
    }
};
