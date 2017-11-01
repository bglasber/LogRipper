#define GTEST_HAS_TR1_TUPLE 0
#include "../src/file_reader.h"
#include <gmock/gmock.h>
#include <cerrno>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>


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
        } else if( c == '\0' ) {
            EXPECT_EQ( FileReader::getTokenForChar( c ), EOF_TOK ) << "Char " << c << ", " << i << ", is not an EOF!";
        } else {
            EXPECT_EQ(FileReader::getTokenForChar( c ), ERROR ) << "Char " << c << ", " << i << ", is not an error!";

        }
    }
};

TEST( test_lex, test_process_line ) {
    unlink( "test_lex.txt" );
    int fd = open( "test_lex.txt", O_CREAT | O_EXCL | O_WRONLY );
    ASSERT_GT( fd, 2 );
    std::string msg = "I12:12:12 [file_name.cc] This is a test message\n";
    int rc = write( fd, msg.data(), msg.size() );
    ASSERT_EQ( rc, msg.size());
    close( fd );
    fd = open( "test_lex.txt", O_RDONLY );
    // Other fds are still in use, since I didn't close them...
    ASSERT_GT( fd, 2 );

    //Only need one buffer
    FileReader reader( fd, 2 );
    reader.processFile();

    //Check what we read
    std::vector<std::vector<Token>> *parsed_lines = reader.getParsedData();
    EXPECT_EQ( parsed_lines->size(), 1 );
    std::vector<Token> &line = parsed_lines->at(0);
    int line_ind = 0;

    EXPECT_EQ( line[line_ind++], WORD );
    EXPECT_EQ( line[line_ind++], NUMBER );
    EXPECT_EQ( line[line_ind++], PUNCTUATION );
    EXPECT_EQ( line[line_ind++], NUMBER );
    EXPECT_EQ( line[line_ind++], PUNCTUATION );
    EXPECT_EQ( line[line_ind++], NUMBER );
    EXPECT_EQ( line[line_ind++], WHITE_SPACE );

    EXPECT_EQ( line[line_ind++], PUNCTUATION );
    EXPECT_EQ( line[line_ind++], WORD );
    EXPECT_EQ( line[line_ind++], PUNCTUATION );
    EXPECT_EQ( line[line_ind++], WORD );
    EXPECT_EQ( line[line_ind++], PUNCTUATION );
    EXPECT_EQ( line[line_ind++], WORD );
    EXPECT_EQ( line[line_ind++], PUNCTUATION );
    EXPECT_EQ( line[line_ind++], WHITE_SPACE );

    EXPECT_EQ( line[line_ind++], WORD );
    EXPECT_EQ( line[line_ind++], WHITE_SPACE );
    EXPECT_EQ( line[line_ind++], WORD );
    EXPECT_EQ( line[line_ind++], WHITE_SPACE );
    EXPECT_EQ( line[line_ind++], WORD );
    EXPECT_EQ( line[line_ind++], WHITE_SPACE );
    EXPECT_EQ( line[line_ind++], WORD );
    EXPECT_EQ( line[line_ind++], WHITE_SPACE );
    EXPECT_EQ( line[line_ind++], WORD );
    EXPECT_EQ( line[line_ind++], NEW_LINE );

    EXPECT_EQ( line.size(), line_ind ); //size
}

TEST( test_lex, one_full_buffer ) {
    unlink( "test_lex.txt" );
    int fd = open( "test_lex.txt", O_CREAT | O_EXCL | O_WRONLY );
    ASSERT_GT( fd, 2 );
    std::string msg = "AAAAAAAAAAAAAAA\n"; //16 bytes so it aligns evenly on buffer boundary

    struct stat stat_buf;
    int rc = fstat( fd, &stat_buf );
    ASSERT_EQ( rc, 0 );
    size_t fs_blksize = stat_buf.st_blksize;
    //Test that messages don't cross buffers
    EXPECT_EQ( fs_blksize % msg.size(), 0 );
    size_t num_messages_to_write = fs_blksize / msg.size();

    for( unsigned i = 0; i < num_messages_to_write; i++ ) {
        rc = write( fd, msg.data(), msg.size() );
        ASSERT_EQ( rc, msg.size() );
    }

    close( fd );
    fd = open( "test_lex.txt", O_RDONLY );
    // Other fds are still in use, since I didn't close them...
    ASSERT_GT( fd, 2 );

    //Need both buffers this time
    FileReader reader( fd, 2 );
    reader.processFile();

    //Check what we read
    std::vector<std::vector<Token>> *parsed_lines = reader.getParsedData();
    EXPECT_EQ( parsed_lines->size(), num_messages_to_write );
    for( unsigned int i = 0; i < num_messages_to_write; i++ ) {
        std::vector<Token> &line = parsed_lines->at(i);
        int line_ind = 0;

        EXPECT_EQ( line[line_ind++], WORD );
        EXPECT_EQ( line[line_ind++], NEW_LINE );
        EXPECT_EQ( line.size(), line_ind );
    }
}
