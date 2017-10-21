#define GTEST_HAS_TR1_TUPLE 0
#include "../src/file_reader.h"
#include <gmock/gmock.h>
#include <ctype.h>

using namespace ::testing;

class test_lex : public ::testing::Test {};

/* Assert that we pull the right values for tokens out of the map */
TEST( test_lex, test_char_token_mappings ) {
    for( int i = 0; i < 128; i++ ) {
        char c = (char) i;
        if( isalpha( c ) ) {
            EXPECT_EQ( FileReader::getTokenForChar( c ), WORD ) << "Char " << c << ", " << i << ", is not a word!";
        } else if( isdigit( c ) ) {
            EXPECT_EQ( FileReader::getTokenForChar( c ), NUMBER ) << "Char " << c << ", " << i << ", is not a number!";
        } else if( isspace( c ) && c != '\n' ) {
            EXPECT_EQ( FileReader::getTokenForChar( c ), WHITE_SPACE ) << "Char " << c << ", " << i << ", is not a space!";
        } else if( c == '\n' ) {
            EXPECT_EQ( FileReader::getTokenForChar( c ), NEW_LINE ) << "Char " << c << ", " << i << ", is not a new_line!";
        } else if( ispunct( c ) ) {
            EXPECT_EQ( FileReader::getTokenForChar( c ), PUNCTUATION ) << "Char " << c << ", " << i << ", is not a punctuation!";
        } else {
            EXPECT_EQ(FileReader::getTokenForChar( c ), ERROR ) << "Char " << c << ", " << i << ", is not an error!";

        }
    }
};
