#define GTEST_HAS_TR1_TUPLE 0
#include "../src/rule_applier.h"
#include "../src/token.h"
#include <gmock/gmock.h>
#include <memory>

using namespace ::testing;

class test_rules : public ::testing::Test {};

void pass_through_rule( std::unique_ptr<std::vector<TokenWordPair>> &line ) {
    (void) line;
}

void abstract_all_rule( std::unique_ptr<std::vector<TokenWordPair>> &line ) {
    for( unsigned int i = 0; i < line->size(); i++ ) {
        line->at(i).tok = ABSTRACTED_VALUE;
    }
}

void abstract_all_word( std::unique_ptr<std::vector<TokenWordPair>> &line ) {
    for( unsigned int i = 0; i < line->size(); i++ ) {
        if( line->at(i).tok == WORD ) {
            line->at(i).tok = ABSTRACTED_VALUE;
        }
    }
}

void abstract_all_number( std::unique_ptr<std::vector<TokenWordPair>> &line ) {
    for( unsigned int i = 0; i < line->size(); i++ ) {
        if( line->at(i).tok == NUMBER ) {
            line->at(i).tok = ABSTRACTED_VALUE;
        }
    }
}


TEST( test_rules, single_entry_pass_through ) {
    ParseBufferEngine pbe_in;
    ParseBufferEngine pbe_out;
    std::list<RuleFunction> rule_funcs;
    rule_funcs.push_back( pass_through_rule );
    RuleApplier ra( std::move( rule_funcs ), &pbe_in, &pbe_out );

    std::unique_ptr<ParseBuffer> buffer = std::make_unique<ParseBuffer>();

    std::unique_ptr<std::vector<TokenWordPair>> tokens_in_line = std::make_unique<std::vector<TokenWordPair>>();
    TokenWordPair twp;
    twp.tok = WORD;
    twp.word = "TESTWORD";
    TokenWordPair twp2;
    twp2.tok = NEW_LINE;
    twp2.word = "\n";

    tokens_in_line->push_back( twp );
    tokens_in_line->push_back( twp2 );
    EXPECT_EQ( twp.tok, WORD );
    EXPECT_EQ( tokens_in_line->at(0).tok, WORD );
    EXPECT_EQ( tokens_in_line->at(1).tok, NEW_LINE );

    bool done_buffer = buffer->addLine( std::move( tokens_in_line ) );
    EXPECT_FALSE( done_buffer );
    EXPECT_EQ( buffer->ind, 1 );
    const std::unique_ptr<std::vector<TokenWordPair>> &buffer_line_ptr = buffer->parsed_lines[0];

    ra.applyRules( buffer );
    EXPECT_EQ( buffer_line_ptr->at(0).tok, WORD );
    EXPECT_EQ( buffer_line_ptr->at(1).tok, NEW_LINE );
}

TEST( test_rules, single_entry_abstract_all ) {
    ParseBufferEngine pbe_in;
    ParseBufferEngine pbe_out;
    std::list<RuleFunction> rule_funcs;
    rule_funcs.push_back( abstract_all_rule );
    RuleApplier ra( std::move( rule_funcs ), &pbe_in, &pbe_out );

    std::unique_ptr<ParseBuffer> buffer = std::make_unique<ParseBuffer>();

    std::unique_ptr<std::vector<TokenWordPair>> tokens_in_line = std::make_unique<std::vector<TokenWordPair>>();
    (void) tokens_in_line;
    TokenWordPair twp;
    twp.tok = WORD;
    twp.word = "TESTWORD";
    TokenWordPair twp2;
    twp2.tok = NEW_LINE;
    twp2.word = "\n";

    tokens_in_line->push_back( twp );
    tokens_in_line->push_back( twp2 );
    EXPECT_EQ( twp.tok, WORD );
    EXPECT_EQ( tokens_in_line->at(0).tok, WORD );
    EXPECT_EQ( tokens_in_line->at(1).tok, NEW_LINE );

    bool done_buffer = buffer->addLine( std::move( tokens_in_line ) );
    EXPECT_FALSE( done_buffer );
    EXPECT_EQ( buffer->ind, 1 );
    const std::unique_ptr<std::vector<TokenWordPair>> &buffer_line_ptr = buffer->parsed_lines[0];

    ra.applyRules( buffer );
    EXPECT_EQ( buffer_line_ptr->at(0).tok, ABSTRACTED_VALUE );
    EXPECT_STREQ( buffer_line_ptr->at(0).word.c_str(), "TESTWORD" );
    EXPECT_EQ( buffer_line_ptr->at(1).tok, ABSTRACTED_VALUE );
    EXPECT_STREQ( buffer_line_ptr->at(1).word.c_str(), "\n" );
}

TEST( test_rules, apply_rule_across_whole_buffer ) {
    ParseBufferEngine pbe_in;
    ParseBufferEngine pbe_out;
    std::list<RuleFunction> rule_funcs;
    rule_funcs.push_back( abstract_all_rule );
    RuleApplier ra( std::move( rule_funcs ), &pbe_in, &pbe_out );

    std::unique_ptr<ParseBuffer> buffer = std::make_unique<ParseBuffer>();

    bool done = false;
    while( !done ) {
        std::unique_ptr<std::vector<TokenWordPair>> tokens_in_line = std::make_unique<std::vector<TokenWordPair>>();
        TokenWordPair twp;
        twp.tok = WORD;
        twp.word = "TESTWORD";
        TokenWordPair twp2;
        twp2.tok = NEW_LINE;
        twp2.word = "\n";

        tokens_in_line->push_back( twp );
        tokens_in_line->push_back( twp2 );
        done = buffer->addLine( std::move( tokens_in_line ) );
    }

    ra.applyRules( buffer );

    EXPECT_EQ( buffer->ind, LINES_IN_BUFFER );
    for( unsigned int i = 0; i < buffer->ind; i++ ) {
        const std::unique_ptr<std::vector<TokenWordPair>> &tokens_in_line = buffer->parsed_lines[i];
        EXPECT_EQ( tokens_in_line->at(0).tok, ABSTRACTED_VALUE );
        EXPECT_STREQ( tokens_in_line->at(0).word.c_str(), "TESTWORD" );
        EXPECT_EQ( tokens_in_line->at(1).tok, ABSTRACTED_VALUE );
        EXPECT_STREQ( tokens_in_line->at(1).word.c_str(), "\n" );
    }
}

TEST( test_rules, apply_multiple_rules_across_whole_buffer ) {
    ParseBufferEngine pbe_in;
    ParseBufferEngine pbe_out;
    std::list<RuleFunction> rule_funcs;
    rule_funcs.push_back( abstract_all_word );
    rule_funcs.push_back( abstract_all_number );
    RuleApplier ra( std::move( rule_funcs ), &pbe_in, &pbe_out );

    std::unique_ptr<ParseBuffer> buffer = std::make_unique<ParseBuffer>();

    bool done = false;
    while( !done ) {
        std::unique_ptr<std::vector<TokenWordPair>> tokens_in_line = std::make_unique<std::vector<TokenWordPair>>();
        TokenWordPair twp;
        twp.tok = WORD;
        twp.word = "TEST";
        tokens_in_line->push_back( twp );
        tokens_in_line->push_back( twp );
        tokens_in_line->push_back( twp );
        twp.tok = PUNCTUATION;
        twp.word = ";";
        tokens_in_line->push_back( twp );
        twp.tok = NUMBER;
        twp.word = "1";
        tokens_in_line->push_back( twp );
        twp.tok = PUNCTUATION;
        twp.word = ".";
        tokens_in_line->push_back( twp );
        TokenWordPair twpn;
        twpn.tok = NEW_LINE;
        twpn.word = "\n";
        tokens_in_line->push_back( twpn );
        done = buffer->addLine( std::move( tokens_in_line ) );
    }

    ra.applyRules( buffer );

    EXPECT_EQ( buffer->ind, LINES_IN_BUFFER );
    for( unsigned int i = 0; i < buffer->ind; i++ ) {
        const std::unique_ptr<std::vector<TokenWordPair>> &tokens_in_line = buffer->parsed_lines[i];
        EXPECT_EQ( tokens_in_line->at(0).tok, ABSTRACTED_VALUE );
        EXPECT_STREQ( tokens_in_line->at(0).word.c_str(), "TEST" );
        EXPECT_EQ( tokens_in_line->at(1).tok, ABSTRACTED_VALUE );
        EXPECT_STREQ( tokens_in_line->at(1).word.c_str(), "TEST" );
        EXPECT_EQ( tokens_in_line->at(2).tok, ABSTRACTED_VALUE );
        EXPECT_STREQ( tokens_in_line->at(2).word.c_str(), "TEST" );
        EXPECT_EQ( tokens_in_line->at(3).tok, PUNCTUATION );
        EXPECT_STREQ( tokens_in_line->at(3).word.c_str(), ";" );
        EXPECT_EQ( tokens_in_line->at(4).tok, ABSTRACTED_VALUE );
        EXPECT_STREQ( tokens_in_line->at(4).word.c_str(), "1" );
        EXPECT_EQ( tokens_in_line->at(5).tok, PUNCTUATION );
        EXPECT_STREQ( tokens_in_line->at(5).word.c_str(), "." );
        EXPECT_EQ( tokens_in_line->at(6).tok, NEW_LINE );
        EXPECT_STREQ( tokens_in_line->at(6).word.c_str(), "\n" );
    }
}
