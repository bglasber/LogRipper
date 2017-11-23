#define GTEST_HAS_TR1_TUPLE 0
#include "../src/binner.h"
#include "../src/parse_buffer.h"
#include "../src/token.h"
#include <gmock/gmock.h>
#include <vector>
#include <fstream>
#include <unistd.h>
#include <iostream>

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

    //Create copy locally
    std::vector<TokenWordPair> line2( *line );

    binner.binEntriesInBuffer( buffer );
    std::unordered_map<BinKey, Bin, BinKeyHasher> &map = binner.getUnderlyingMap();

    BinKey bk;
    bk.num_words = 1;
    bk.num_params = 0;
    auto search = map.find( bk );

    EXPECT_NE( search, map.end() );
    std::vector<LineWithTransitions> &vec = search->second.getBinVector();
    EXPECT_EQ( vec.size(), 1 );
    EXPECT_EQ( vec.at(0).getLine(), line2 );

    //The lines will be destroyed when the binner shuts down
    //delete buffer;
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

    std::vector<TokenWordPair> loc_line1( *line );

    //Two words
    std::vector<TokenWordPair> *line2 = new std::vector<TokenWordPair>();
    twp.tok = WORD;
    twp.word = "TEST";
    line2->push_back( twp );
    line2->push_back( twp );
    twp.tok = NEW_LINE;
    twp.word = "\n";
    line2->push_back( twp );
    buffer->addLine( line2 );


    std::vector<TokenWordPair> loc_line2( *line2 );

    binner.binEntriesInBuffer( buffer );
    std::unordered_map<BinKey, Bin, BinKeyHasher> &map = binner.getUnderlyingMap();

    EXPECT_EQ( map.size(), 2 );

    BinKey bk;
    bk.num_words = 1;
    bk.num_params = 0;
    auto search = map.find( bk );

    EXPECT_NE( search, map.end() );
    std::vector<LineWithTransitions> &vec = search->second.getBinVector();
    EXPECT_EQ( vec.size(), 1 );
    EXPECT_EQ( vec.at(0).getLine(), loc_line1 );

    bk.num_words = 2;
    search = map.find( bk );
    EXPECT_NE( search, map.end() );
    std::vector<LineWithTransitions> &vec2 = search->second.getBinVector();
    EXPECT_EQ( vec2.size(), 1 );
    EXPECT_EQ( vec2.at(0).getLine(), loc_line2 );

    //delete buffer;
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
    std::vector<TokenWordPair> loc_line1( *line );

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
    std::vector<LineWithTransitions> &vec = search->second.getBinVector();
    EXPECT_EQ( vec.size(), 1 );
    EXPECT_EQ( vec.at(0).getLine(), loc_line1 );

    //delete buffer;
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
    std::vector<TokenWordPair> loc_line1( *line );

    //Different line, but same content
    std::vector<TokenWordPair> *line2 = new std::vector<TokenWordPair>();
    twp.tok = WORD;
    twp.word = "TEST2";
    line2->push_back( twp );
    twp.tok = NEW_LINE;
    twp.word = "\n";
    line2->push_back( twp );
    buffer->addLine( line2 );
    std::vector<TokenWordPair> loc_line2( *line2 );

    binner.binEntriesInBuffer( buffer );
    std::unordered_map<BinKey, Bin, BinKeyHasher> &map = binner.getUnderlyingMap();

    EXPECT_EQ( map.size(), 1 );

    BinKey bk;
    bk.num_words = 1;
    bk.num_params = 0;
    auto search = map.find( bk );

    EXPECT_NE( search, map.end() );
    std::vector<LineWithTransitions> &vec = search->second.getBinVector();
    EXPECT_EQ( vec.size(), 2 );
    EXPECT_EQ( vec.at(0).getLine(), loc_line1 );
    EXPECT_EQ( vec.at(1).getLine(), loc_line2 );

    //delete buffer;
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
    std::vector<TokenWordPair> loc_line1( *line );

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
    std::vector<TokenWordPair> loc_line2( *line2 );

    binner.binEntriesInBuffer( buffer );
    std::unordered_map<BinKey, Bin, BinKeyHasher> &map = binner.getUnderlyingMap();

    EXPECT_EQ( map.size(), 2 );

    BinKey bk;
    bk.num_words = 1;
    bk.num_params = 0;
    auto search = map.find( bk );

    EXPECT_NE( search, map.end() );
    std::vector<LineWithTransitions> &vec = search->second.getBinVector();
    EXPECT_EQ( vec.size(), 1 );
    EXPECT_EQ( vec.at(0).getLine(), loc_line1 );

    bk.num_params = 1;
    search = map.find( bk );
    EXPECT_NE( search, map.end() );
    std::vector<LineWithTransitions> &vec2 = search->second.getBinVector();
    EXPECT_EQ( vec2.size(), 1 );
    EXPECT_EQ( vec2.at(0).getLine(), loc_line2 );

    //delete buffer;
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
    std::vector<TokenWordPair> loc_line1( *line );

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
    std::vector<LineWithTransitions> &vec = search->second.getBinVector();
    EXPECT_EQ( vec.size(), 1 );
    EXPECT_EQ( vec.at(0).getLine(), loc_line1 );

    //delete buffer;
}

TEST( test_binner, test_serialize ) {
    std::ofstream os;
    unlink( "test_serialize.txt" );
    os.open( "test_serialize.txt", std::ofstream::out );

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
    binner.serialize( os );
    os.close();

    std::ifstream is;
    is.open( "test_serialize.txt", std::ifstream::in );

    Binner binner2( &pbe_in );
    std::cout << "Going to deserialize" << std::endl;
    binner2.deserialize( is );
    std::cout << "Done deserializing..." << std::endl;

    std::unordered_map<BinKey, Bin, BinKeyHasher> &map = binner2.getUnderlyingMap();
    std::unordered_map<BinKey, Bin, BinKeyHasher> &map2 = binner.getUnderlyingMap();
    EXPECT_EQ( map.size(), map2.size() );
    BinKey bk;
    bk.num_words = 1;
    bk.num_params = 1;
    auto search = map.find( bk );
    EXPECT_NE( search, map.end() );
    auto search2 = map2.find( bk );
    EXPECT_NE( search2, map.end() );

    std::vector<LineWithTransitions> &vec = search->second.getBinVector();
    std::vector<LineWithTransitions> &vec2 = search2->second.getBinVector();
    EXPECT_EQ( vec.size(), vec2.size() );
    EXPECT_EQ( vec.size(), 1 );
    std::vector<TokenWordPair> &recon_line1 = vec.at(0).getLine();
    std::vector<TokenWordPair> &recon_line2 = vec2.at(0).getLine();
    EXPECT_NE( &recon_line1, &recon_line2 );

    EXPECT_EQ( recon_line1.at(0).tok, recon_line2.at(0).tok );
    EXPECT_STREQ( recon_line1.at(0).word.c_str(), recon_line2.at(0).word.c_str() );
    EXPECT_EQ( recon_line1.at(1).tok, recon_line2.at(1).tok );
    EXPECT_STREQ( recon_line1.at(1).word.c_str(), recon_line2.at(1).word.c_str() );
    EXPECT_EQ( recon_line1.at(2).tok, recon_line2.at(2).tok );
    EXPECT_STREQ( recon_line1.at(2).word.c_str(), recon_line2.at(2).word.c_str() );
}

TEST( test_transition_counter, create_simple_counter ) {
    std::vector<TokenWordPair> line;
    TokenWordPair twp;
    twp.tok = WORD;
    twp.word = "TEST";
    line.push_back( twp );
    twp.tok = ABSTRACTED_VALUE;
    line.push_back( twp );
    twp.tok = NEW_LINE;
    twp.word = "\n";
    line.push_back( twp );

    LineWithTransitions lwt( line );
    EXPECT_EQ( lwt.getTransitionProbability( line ), 0.0 );
    lwt.addTransition( line );
    EXPECT_EQ( lwt.getTransitionProbability( line ), 1.0 );
}

TEST( test_transition_counter, multiple_transitions ) {
    std::vector<TokenWordPair> line;
    TokenWordPair twp;
    twp.tok = WORD;
    twp.word = "TEST";
    line.push_back( twp );
    twp.tok = ABSTRACTED_VALUE;
    line.push_back( twp );
    twp.tok = NEW_LINE;
    twp.word = "\n";
    line.push_back( twp );

    std::vector<TokenWordPair> line2;
    twp.tok = WORD;
    twp.word = "TEST";
    line2.push_back( twp );
    twp.tok = ABSTRACTED_VALUE;
    twp.word = "BLAH"; //doesn't match
    line2.push_back( twp );
    twp.tok = NEW_LINE;
    twp.word = "\n";
    line2.push_back( twp );

    LineWithTransitions lwt( line );
    EXPECT_EQ( lwt.getTransitionProbability( line ), 0.0 );
    lwt.addTransition( line );
    EXPECT_EQ( lwt.getTransitionProbability( line ), 1.0 );
    lwt.addTransition( line2 );
    EXPECT_EQ( lwt.getTransitionProbability( line ), 0.5 );
    EXPECT_EQ( lwt.getTransitionProbability( line2 ), 0.5 );
}

TEST( test_transition_counter, multiple_lines_and_transitions ) {
    std::vector<TokenWordPair> line;
    TokenWordPair twp;
    twp.tok = WORD;
    twp.word = "TEST";
    line.push_back( twp );
    twp.tok = ABSTRACTED_VALUE;
    line.push_back( twp );
    twp.tok = NEW_LINE;
    twp.word = "\n";
    line.push_back( twp );

    std::vector<TokenWordPair> line2;
    twp.tok = WORD;
    twp.word = "TEST";
    line2.push_back( twp );
    twp.tok = ABSTRACTED_VALUE;
    twp.word = "BLAH"; //doesn't match
    line2.push_back( twp );
    twp.tok = NEW_LINE;
    twp.word = "\n";
    line2.push_back( twp );

    LineWithTransitions lwt( line );
    LineWithTransitions lwt2( line2 );
    EXPECT_EQ( lwt.getTransitionProbability( line ), 0.0 );
    lwt.addTransition( line );
    EXPECT_EQ( lwt.getTransitionProbability( line ), 1.0 );
    lwt.addTransition( line2 );
    EXPECT_EQ( lwt.getTransitionProbability( line ), 0.5 );
    EXPECT_EQ( lwt.getTransitionProbability( line2 ), 0.5 );

    EXPECT_EQ( lwt2.getTransitionProbability( line ), 0.0 );
    lwt2.addTransition( line );
    EXPECT_EQ( lwt2.getTransitionProbability( line ), 1.0 );
    lwt2.addTransition( line2 );
    EXPECT_EQ( lwt2.getTransitionProbability( line ), 0.5 );
    EXPECT_EQ( lwt2.getTransitionProbability( line2 ), 0.5 );
}

TEST( test_binner, computes_transitions_among_single_thread ) {
    std::vector<TokenWordPair> line;
    TokenWordPair twp;
    twp.tok = WORD;
    twp.word = "I";
    line.push_back( twp );
    twp.tok = NUMBER;
    twp.word = "1019";
    line.push_back( twp );
    twp.tok = WHITE_SPACE;
    twp.word = " ";
    line.push_back( twp );
    twp.tok = NUMBER;
    twp.word = "12";
    line.push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = ":";
    line.push_back( twp );
    twp.tok = NUMBER;
    twp.word = "12";
    line.push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = ":";
    line.push_back( twp );
    twp.tok = NUMBER;
    twp.word = "41";
    line.push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = ".";
    line.push_back( twp );
    twp.tok = NUMBER;
    twp.word = "428384";
    line.push_back( twp );
    twp.tok = WHITE_SPACE;
    twp.word = " ";
    line.push_back( twp );
    twp.tok = NUMBER;
    twp.word = "12811";
    line.push_back( twp );
    twp.tok = WHITE_SPACE;
    twp.word = " ";
    line.push_back( twp );
    twp.tok = WORD;
    twp.word = "siteSelectorServer";
    line.push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = ".";
    line.push_back( twp );
    twp.tok = WORD;
    twp.word = "cc";
    line.push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = ":";
    line.push_back( twp );
    twp.tok = NUMBER;
    twp.word = "109";
    line.push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = "]";
    line.push_back( twp );
    twp.tok = WHITE_SPACE;
    twp.word = " ";
    line.push_back( twp );
    twp.tok = WORD;
    twp.word = "adding";
    line.push_back( twp );
    twp.tok = WORD;
    twp.word = "connection";
    line.push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = ":";
    line.push_back( twp );
    twp.tok = ABSTRACTED_VALUE;
    twp.word = "0";
    line.push_back( twp );

    std::vector<TokenWordPair> *line_copy1 = new std::vector<TokenWordPair>( line );
    std::vector<TokenWordPair> *line_copy2 = new std::vector<TokenWordPair>( line );

    ParseBufferEngine pbe_in;
    Binner binner( &pbe_in );

    ParseBuffer *buffer = new ParseBuffer();
    buffer->addLine( line_copy1 );
    buffer->addLine( line_copy2 );
    binner.binEntriesInBuffer( buffer );

    std::unordered_map<BinKey, Bin, BinKeyHasher> &map = binner.getUnderlyingMap();

    BinKey bk;
    bk.num_words = 5;
    bk.num_params = 1;
    auto search = map.find( bk );

    EXPECT_NE( search, map.end() );
    std::vector<LineWithTransitions> &vec = search->second.getBinVector();
    EXPECT_EQ( vec.size(), 1 );

    LineWithTransitions &lwt = vec.at(0);
    EXPECT_EQ( lwt.getLine(), line );
    EXPECT_EQ( lwt.getTransitionProbability( line ), 1.0 );
}
