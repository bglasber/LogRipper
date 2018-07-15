#ifndef __BINNER_H__
#define __BINNER_H__
#include "token.h"
#include "parse_buffer.h"
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/functional/hash.hpp>
#include <cstdint>
#include <unordered_map>
#include <fstream>
#include <vector>
#include <list>
#include <iostream>
#include <memory>

bool abstracted_line_match( const std::shared_ptr<std::vector<TokenWordPair>> &line1, const std::shared_ptr<std::vector<TokenWordPair>> &line2 );

struct LineKey {
    friend class boost::serialization::access;
    template <class Archive>
        void serialize( Archive &ar, const unsigned int version ) {
            ar & line;
        }
    std::shared_ptr<std::vector<TokenWordPair>> line;
    LineKey() {}
    LineKey( std::shared_ptr<std::vector<TokenWordPair>> line ) : line( line ) { }

    bool operator==( const LineKey &other ) const {
        return abstracted_line_match( line, other.line );
    }
};

struct LineKeyHasher {
    std::size_t operator()( const LineKey &lk ) const {
        std::size_t seed = 0;
        for( const auto &twp : *(lk.line) ) {
            boost::hash_combine( seed, twp.tok );
        }
        return seed;
    }
};

class LineTransitions {
    friend class boost::serialization::access;
    template <class Archive>
        void serialize( Archive &ar, const unsigned int version ) {
            ar & transitions;
        }
    std::unordered_map<LineKey, std::pair<std::shared_ptr<std::vector<TokenWordPair>>, uint64_t>, LineKeyHasher> transitions;
public:
    LineTransitions() {}
    LineTransitions( const std::unordered_map<LineKey, std::pair<std::shared_ptr<std::vector<TokenWordPair>>, uint64_t>, LineKeyHasher> &transitions ) : transitions( transitions ) {}
    void addTransition( std::shared_ptr<std::vector<TokenWordPair>> other_line );
    uint64_t getTransitionCount( std::shared_ptr<std::vector<TokenWordPair>> &line );

    std::pair<bool,double> isOutlier( std::shared_ptr<std::vector<TokenWordPair>> &transition_line, uint64_t total_transitions );

    std::unordered_map<LineKey, std::pair<std::shared_ptr<std::vector<TokenWordPair>>, uint64_t>, LineKeyHasher> &getTransitions() {
        return transitions;
    }
};

class LineWithTransitions {
    friend class boost::serialization::access;
    template <class Archive>
        void serialize( Archive &ar, const unsigned int version ) {
            ar & line;
            ar & lt;
            ar & times_seen;
        }
    std::shared_ptr<std::vector<TokenWordPair>>  line;
    LineTransitions             lt;
    uint64_t                    times_seen;
public:
    LineWithTransitions(): times_seen( 0 ) { }
    LineWithTransitions( std::shared_ptr<std::vector<TokenWordPair>> line ) : line( line ), times_seen( 0 ) {}
    LineWithTransitions( std::shared_ptr<std::vector<TokenWordPair>> line, LineTransitions &lt, uint64_t &times_seen ) : line( line ), lt( lt ), times_seen( times_seen ) {}
    void addTransition( std::shared_ptr<std::vector<TokenWordPair>> other_line );
    double getTransitionProbability( std::shared_ptr<std::vector<TokenWordPair>> &line );

    std::pair<bool,double> isOutlier( std::shared_ptr<std::vector<TokenWordPair>> &transition_line ) {
        std::pair<bool, double> loc = lt.isOutlier( transition_line, times_seen );
        if( loc.first ) {
            std::cout << "Given Prior String: " << std::endl;
            for( const auto &twp : *line ) {
                std::cout << twp.word << " ";
            }
            std::cout << std::endl;
        }
        return loc;
    }
    std::shared_ptr<std::vector<TokenWordPair>> &getLine() {
        return line;
    }

    LineTransitions &getLineTransitions() {
        return lt;
    }

    uint64_t getTimesSeen() {
        return times_seen;
    }
};

class LastLineForEachThread {
    std::unordered_map<uint64_t, std::shared_ptr<std::vector<TokenWordPair>>> last_lines;
public:
    LastLineForEachThread() {}
    std::shared_ptr<std::vector<TokenWordPair>> getLastLine( uint64_t thread_id );
    void addNewLine( uint64_t thread_id, std::shared_ptr<std::vector<TokenWordPair>> line );
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
    std::vector<LineWithTransitions> unique_entries_in_bin;

    static BinKey makeBinKeyForLine( std::vector<TokenWordPair> &line );
public:
    Bin() {}
    Bin( const std::vector<LineWithTransitions> &unique_entries_in_bin ) : unique_entries_in_bin( unique_entries_in_bin ) {}
    void insertIntoBin( std::shared_ptr<std::vector<TokenWordPair>> line, std::shared_ptr<std::vector<TokenWordPair>> &last_line );
    LineWithTransitions *findEntryInBin( std::shared_ptr<std::vector<TokenWordPair>> &line );

    std::vector<LineWithTransitions> &getBinVector() {
        return unique_entries_in_bin;
    }

    static BinKey makeBinKeyForLine( std::unique_ptr<std::vector<TokenWordPair>> &line );
    static BinKey makeBinKeyForLine( std::shared_ptr<std::vector<TokenWordPair>> &line );
};

class Binner {
    std::unordered_map<BinKey, Bin, BinKeyHasher> bin_map;
    LastLineForEachThread last_lines;
    ParseBufferEngine *pbe_in;
    volatile bool done;

    void processLoop();

public:
    Binner( ParseBufferEngine *pbe_in ) : pbe_in( pbe_in ), done( false ) {}
    ~Binner();
    void binEntriesInBuffer( std::unique_ptr<ParseBuffer> buffer );

    void serialize( std::ofstream &os );
    void deserialize( std::ifstream &is );
    void startProcessingBuffers();
    void terminateWhenDoneProcessing();

    std::unordered_map<BinKey, Bin, BinKeyHasher> &getUnderlyingMap() {
        return bin_map;
    }
};
#endif
