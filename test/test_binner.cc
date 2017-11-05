#define GTEST_HAS_TR1_TUPLE 0
#include "../src/binner.h"
#include "../src/parse_buffer.h"
#include "../src/token.h"
#include <gmock/gmock.h>
#include <vector>

using namespace ::testing;

class test_binner : public ::testing::Test {};

TEST( test_binner, test_binner_keys_single_word_entry_correctly ) {
    ParseBufferEngine pbe_in;
    Binner binner( &pbe_in );

    ParseBuffer *buffer = new ParseBuffer();
    std::vector<TokenWordPair> *line = new std::vector<TokenWordPair>();
    TokenWordPair twp;
    twp.tok = WORD;
    twp.word = "TEST";
    line->push_back( twp );
    twp.tok = NEW_LINE;
    twp.word = "\n";
    line->push_back( twp );

    buffer->addLine( line );

    binner.binEntriesInBuffer( buffer );
    std::unordered_map<BinKey, Bin, BinKeyHasher> &map = binner.getUnderlyingMap();

    BinKey bk;
    bk.num_words = 1;
    bk.num_params = 0;
    auto search = map.find( bk );

    EXPECT_NE( search, map.end() );
    std::vector<std::vector<TokenWordPair> *> &vec = search->second.getBinVector();
    EXPECT_EQ( vec.size(), 1 );
    EXPECT_EQ( vec.at(0), line );

    //The lines will be destroyed when the binner shuts down
    delete buffer;
}

TEST( test_binner, test_binner_keys_different_word_size_different_buckets ) {
    ParseBufferEngine pbe_in;
    Binner binner( &pbe_in );

    ParseBuffer *buffer = new ParseBuffer();

    //One word
    std::vector<TokenWordPair> *line = new std::vector<TokenWordPair>();
    TokenWordPair twp;
    twp.tok = WORD;
    twp.word = "TEST";
    line->push_back( twp );
    twp.tok = NEW_LINE;
    twp.word = "\n";
    line->push_back( twp );
    buffer->addLine( line );

    //Two words
    std::vector<TokenWordPair> *line2 = new std::vector<TokenWordPair>();
    twp.tok = WORD;
    twp.word = "TEST";
    line2->push_back( twp );
    line2->push_back( twp );
    twp.tok = NEW_LINE;
    twp.word = "\n";
    line->push_back( twp );
    buffer->addLine( line2 );

    binner.binEntriesInBuffer( buffer );
    std::unordered_map<BinKey, Bin, BinKeyHasher> &map = binner.getUnderlyingMap();

    EXPECT_EQ( map.size(), 2 );

    BinKey bk;
    bk.num_words = 1;
    bk.num_params = 0;
    auto search = map.find( bk );

    EXPECT_NE( search, map.end() );
    std::vector<std::vector<TokenWordPair> *> &vec = search->second.getBinVector();
    EXPECT_EQ( vec.size(), 1 );
    EXPECT_EQ( vec.at(0), line );

    bk.num_words = 2;
    search = map.find( bk );
    EXPECT_NE( search, map.end() );
    std::vector<std::vector<TokenWordPair> *> &vec2 = search->second.getBinVector();
    EXPECT_EQ( vec2.size(), 1 );
    EXPECT_EQ( vec2.at(0), line2 );

    delete buffer;
}

TEST( test_binner, same_entry_word_match_removed ) {
    ParseBufferEngine pbe_in;
    Binner binner( &pbe_in );

    ParseBuffer *buffer = new ParseBuffer();

    //One word
    std::vector<TokenWordPair> *line = new std::vector<TokenWordPair>();
    TokenWordPair twp;
    twp.tok = WORD;
    twp.word = "TEST";
    line->push_back( twp );
    twp.tok = NEW_LINE;
    twp.word = "\n";
    line->push_back( twp );
    buffer->addLine( line );

    //Different line, but same content
    std::vector<TokenWordPair> *line2 = new std::vector<TokenWordPair>();
    twp.tok = WORD;
    twp.word = "TEST";
    line2->push_back( twp );
    twp.tok = NEW_LINE;
    twp.word = "\n";
    line2->push_back( twp );
    buffer->addLine( line2 );

    binner.binEntriesInBuffer( buffer );
    std::unordered_map<BinKey, Bin, BinKeyHasher> &map = binner.getUnderlyingMap();

    EXPECT_EQ( map.size(), 1 );

    BinKey bk;
    bk.num_words = 1;
    bk.num_params = 0;
    auto search = map.find( bk );

    EXPECT_NE( search, map.end() );
    std::vector<std::vector<TokenWordPair> *> &vec = search->second.getBinVector();
    EXPECT_EQ( vec.size(), 1 );
    EXPECT_EQ( vec.at(0), line );

    delete buffer;
}

TEST( test_binner, same_entry_word_mismatch_kept ) {
    ParseBufferEngine pbe_in;
    Binner binner( &pbe_in );

    ParseBuffer *buffer = new ParseBuffer();

    //One word
    std::vector<TokenWordPair> *line = new std::vector<TokenWordPair>();
    TokenWordPair twp;
    twp.tok = WORD;
    twp.word = "TEST";
    line->push_back( twp );
    twp.tok = NEW_LINE;
    twp.word = "\n";
    line->push_back( twp );
    buffer->addLine( line );

    //Different line, but same content
    std::vector<TokenWordPair> *line2 = new std::vector<TokenWordPair>();
    twp.tok = WORD;
    twp.word = "TEST2";
    line2->push_back( twp );
    twp.tok = NEW_LINE;
    twp.word = "\n";
    line2->push_back( twp );
    buffer->addLine( line2 );

    binner.binEntriesInBuffer( buffer );
    std::unordered_map<BinKey, Bin, BinKeyHasher> &map = binner.getUnderlyingMap();

    EXPECT_EQ( map.size(), 1 );

    BinKey bk;
    bk.num_words = 1;
    bk.num_params = 0;
    auto search = map.find( bk );

    EXPECT_NE( search, map.end() );
    std::vector<std::vector<TokenWordPair> *> &vec = search->second.getBinVector();
    EXPECT_EQ( vec.size(), 2 );
    EXPECT_EQ( vec.at(0), line );
    EXPECT_EQ( vec.at(1), line2 );

    delete buffer;
}

TEST( test_binner, same_word_different_params ) {
    ParseBufferEngine pbe_in;
    Binner binner( &pbe_in );

    ParseBuffer *buffer = new ParseBuffer();

    //One word
    std::vector<TokenWordPair> *line = new std::vector<TokenWordPair>();
    TokenWordPair twp;
    twp.tok = WORD;
    twp.word = "TEST";
    line->push_back( twp );
    twp.tok = NEW_LINE;
    twp.word = "\n";
    line->push_back( twp );
    buffer->addLine( line );

    //Different line, but same content
    std::vector<TokenWordPair> *line2 = new std::vector<TokenWordPair>();
    twp.tok = WORD;
    twp.word = "TEST";
    line2->push_back( twp );
    twp.tok = ABSTRACTED_VALUE;
    line2->push_back( twp );
    twp.tok = NEW_LINE;
    twp.word = "\n";
    line2->push_back( twp );
    buffer->addLine( line2 );

    binner.binEntriesInBuffer( buffer );
    std::unordered_map<BinKey, Bin, BinKeyHasher> &map = binner.getUnderlyingMap();

    EXPECT_EQ( map.size(), 2 );

    BinKey bk;
    bk.num_words = 1;
    bk.num_params = 0;
    auto search = map.find( bk );

    EXPECT_NE( search, map.end() );
    std::vector<std::vector<TokenWordPair> *> &vec = search->second.getBinVector();
    EXPECT_EQ( vec.size(), 1 );
    EXPECT_EQ( vec.at(0), line );

    bk.num_params = 1;
    search = map.find( bk );
    EXPECT_NE( search, map.end() );
    std::vector<std::vector<TokenWordPair> *> &vec2 = search->second.getBinVector();
    EXPECT_EQ( vec2.size(), 1 );
    EXPECT_EQ( vec2.at(0), line2 );

    delete buffer;
}

TEST( test_binner, no_match_abstracted_vals ) {
    ParseBufferEngine pbe_in;
    Binner binner( &pbe_in );

    ParseBuffer *buffer = new ParseBuffer();

    //One word
    std::vector<TokenWordPair> *line = new std::vector<TokenWordPair>();
    TokenWordPair twp;
    twp.tok = WORD;
    twp.word = "TEST";
    line->push_back( twp );
    twp.tok = ABSTRACTED_VALUE;
    twp.word = "BLAH"; //doesn't match
    line->push_back( twp );
    twp.tok = NEW_LINE;
    twp.word = "\n";
    line->push_back( twp );
    buffer->addLine( line );

    //Different line, but same content
    std::vector<TokenWordPair> *line2 = new std::vector<TokenWordPair>();
    twp.tok = WORD;
    twp.word = "TEST";
    line2->push_back( twp );
    twp.tok = ABSTRACTED_VALUE;
    line2->push_back( twp );
    twp.tok = NEW_LINE;
    twp.word = "\n";
    line2->push_back( twp );
    buffer->addLine( line2 );

    binner.binEntriesInBuffer( buffer );
    std::unordered_map<BinKey, Bin, BinKeyHasher> &map = binner.getUnderlyingMap();

    EXPECT_EQ( map.size(), 1 );

    BinKey bk;
    bk.num_words = 1;
    bk.num_params = 1;
    auto search = map.find( bk );

    EXPECT_NE( search, map.end() );
    std::vector<std::vector<TokenWordPair> *> &vec = search->second.getBinVector();
    EXPECT_EQ( vec.size(), 2 );
    EXPECT_EQ( vec.at(0), line );
    EXPECT_EQ( vec.at(1), line2 );

    delete buffer;
}
