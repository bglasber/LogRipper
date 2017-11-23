#include "token.h"
#include "parse_buffer.h"
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/functional/hash.hpp>
#include <cstdint>
#include <unordered_map>
#include <fstream>
#include <vector>
#include <list>

struct LineKey {
    std::vector<TokenWordPair> line;
    LineKey() {}
    LineKey( const std::vector<TokenWordPair> &line ) : line( line ) { }

    bool operator==( const LineKey &other ) const {
        return line == other.line;
    }
};

struct LineKeyHasher {
    std::size_t operator()( const LineKey &lk ) const {
        std::size_t seed = 0;
        for( const auto &twp : lk.line ) {
            boost::hash_combine( seed, twp.tok );
        }
        return seed;
    }
};

class LineTransitions {
    std::unordered_map<LineKey, std::pair<std::vector<TokenWordPair>, uint64_t>, LineKeyHasher> transitions;
public:
    void addTransition( std::vector<TokenWordPair> *other_line );
    uint64_t getTransitionCount( std::vector<TokenWordPair> &line );
};

class LineWithTransitions {
    std::vector<TokenWordPair>  line;
    LineTransitions             lt;
    uint64_t                    times_seen;
public:
    LineWithTransitions( std::vector<TokenWordPair> &line ) : line( line ), times_seen( 0 ) {}
    void addTransition( std::vector<TokenWordPair> *other_line );
    double getTransitionProbability( std::vector<TokenWordPair> &line );
};

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
