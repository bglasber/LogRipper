#include "file_reader.h"
#include <assert.h>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <vector>

static Token token_transition_map[NUM_WORDS][NUM_WORDS] = {
                 /* START, WORD,  NUMBER,  WHITE_SPACE, PUNCTUATION, NEW_LINE, ERROR, END */  
/* START */       { ERROR, WORD,  NUMBER,  WHITE_SPACE, PUNCTUATION, NEW_LINE, ERROR, END },
/* WORD */        { ERROR, WORD,  END,     END,         END,         END,      ERROR, ERROR},
/* NUMBER */      { ERROR, END,   NUMBER,  END,         END,         END,      ERROR, ERROR},
/* WHITE_SPACE */ { ERROR, END,   END,     WHITE_SPACE, END,         END,      ERROR, ERROR},
/* PUNCTUATION */ { ERROR, END,   END,     END,         END,         END,      ERROR, ERROR},
/* NEW_LINE */    { ERROR, END,   END,     END,         END,         NEW_LINE, ERROR, ERROR},
/* ERROR    */    { ERROR, ERROR, ERROR,   ERROR,       ERROR,       ERROR,    ERROR, ERROR},
/* END      */    { ERROR, ERROR, ERROR,   ERROR,       ERROR,       ERROR,    ERROR, ERROR},
};

//We use a lookup table for token types rather than if-blocks
//A branch mispredict is ~5 ns
//A L1 cache lookup is 0.5 ns.
//My L1 cache is 32K on 5 year old machine
//This map is 128 * 4 bytes -> 512 bytes
//Therefore, it is better to keep this map in L1 cache and do lookups from it
static Token char_to_token_type_map[128] = {
    ERROR, //0
    ERROR, //1
    ERROR, //2
    ERROR, //3
    ERROR,
    ERROR,
    ERROR,
    ERROR,
    ERROR,
    WHITE_SPACE, //9 = \t
    NEW_LINE, //10 = \n
    WHITE_SPACE, //11, bunch of special chars
    WHITE_SPACE, //12
    WHITE_SPACE, //13
    ERROR,
    ERROR,
    ERROR,
    ERROR,
    ERROR,
    ERROR,
    ERROR,
    ERROR,
    ERROR,
    ERROR,
    ERROR,
    ERROR,
    ERROR,
    ERROR,
    ERROR,
    ERROR,
    ERROR,
    ERROR,
    WHITE_SPACE, //32, space
    PUNCTUATION, //!
    PUNCTUATION,
    PUNCTUATION,
    PUNCTUATION,
    PUNCTUATION,
    PUNCTUATION,
    PUNCTUATION,
    PUNCTUATION,
    PUNCTUATION,
    PUNCTUATION,
    PUNCTUATION,
    PUNCTUATION,
    PUNCTUATION,
    PUNCTUATION,
    PUNCTUATION,
    NUMBER, //NUMBER, 0
    NUMBER, //NUMBER, 1
    NUMBER, //NUMBER, 2
    NUMBER, //NUMBER, 3
    NUMBER, //NUMBER, 4
    NUMBER, //NUMBER, 5
    NUMBER, //NUMBER, 6
    NUMBER, //NUMBER, 7
    NUMBER, //NUMBER, 8
    NUMBER, //NUMBER, 9
    PUNCTUATION, //:
    PUNCTUATION, //;
    PUNCTUATION,
    PUNCTUATION,
    PUNCTUATION,
    PUNCTUATION,
    PUNCTUATION,
    WORD, //A
    WORD,
    WORD,
    WORD,
    WORD,
    WORD,
    WORD,
    WORD,
    WORD,
    WORD,
    WORD,
    WORD,
    WORD,
    WORD,
    WORD,
    WORD,
    WORD,
    WORD,
    WORD,
    WORD,
    WORD,
    WORD,
    WORD,
    WORD,
    WORD,
    WORD, //Z
    PUNCTUATION, // [
    PUNCTUATION,
    PUNCTUATION,
    PUNCTUATION,
    PUNCTUATION,
    PUNCTUATION,
    WORD, //a
    WORD,
    WORD,
    WORD,
    WORD,
    WORD,
    WORD,
    WORD,
    WORD,
    WORD,
    WORD,
    WORD,
    WORD,
    WORD,
    WORD,
    WORD,
    WORD,
    WORD,
    WORD,
    WORD,
    WORD,
    WORD,
    WORD,
    WORD,
    WORD,
    WORD,
    PUNCTUATION,
    PUNCTUATION,
    PUNCTUATION,
    PUNCTUATION,
    ERROR,
};



BufferPool::BufferPool( unsigned num_buffs, unsigned buff_sizes ) : num_buffs( num_buffs ), buff_sizes( buff_sizes ) {
    buff_ptrs = (char **) malloc( sizeof( char * ) * num_buffs );
    for( unsigned i = 0; i < num_buffs; i++ ) {
        buff_ptrs[i] = (char *) malloc( sizeof( char ) * buff_sizes );
    }
}

BufferPool::~BufferPool() {
    for( unsigned i = 0; i < num_buffs; i++ ) {
        free( buff_ptrs[i] );
    }
    free( buff_ptrs );
}

FileReader::FileReader( int fd, unsigned num_buffers ) : fd( fd ), num_buffers( num_buffers ) {
    struct stat stat_buf;
    int rc = fstat( fd, &stat_buf );
    assert( rc == 0 );
    f_size = stat_buf.st_size;
    fs_blksize = stat_buf.st_blksize;
    buffer_pool = new BufferPool( num_buffers, fs_blksize );

    //It's a log file --- Tell the kernel we are going to read it sequentially
    rc = posix_fadvise( fd, 0, 0, POSIX_FADV_SEQUENTIAL );
    assert( rc == 0 );

    load_half = 0;

    //Preconstruct the iovecs
    half_iovec_cnt = num_buffers / 2;
    struct iovec *iovec_half = (struct iovec *) malloc( sizeof(struct iovec) * half_iovec_cnt );
    for( unsigned i = 0; i < half_iovec_cnt; i++ ) {
        iovec_half[i].iov_base = buffer_pool->buff_ptrs[i];
        iovec_half[i].iov_len = fs_blksize;
    }

    struct iovec *iovec_second_half = (struct iovec *) malloc( sizeof(struct iovec) * half_iovec_cnt );
    for( unsigned i = 0; i < half_iovec_cnt; i++ ) {
        assert( half_iovec_cnt + i < num_buffers );
        iovec_second_half[i].iov_base = buffer_pool->buff_ptrs[half_iovec_cnt + i];
        iovec_second_half[i].iov_len = fs_blksize;
    }
    preconstructed_iovecs[0] = iovec_half;
    preconstructed_iovecs[1] = iovec_second_half;

}

void FileReader::loadBuffers( ) {
    ssize_t bytes_read = readv( fd, preconstructed_iovecs[load_half], half_iovec_cnt );
    std::cout << "fd " << fd << std::endl;;
    std::cout << "half_iovec_cnt: " << half_iovec_cnt << std::endl;
    std::cout << "preconstructed iovecs: " << &(preconstructed_iovecs[load_half][0]) << std::endl;
    load_half = (load_half + 1) % 2;
    std::cout << "Loaded " << bytes_read << " bytes." << std::endl;
}

Token FileReader::getTokenForChar( char c ) {
    return char_to_token_type_map[ (int) c ];
}

void FileReader::processNextToken( char c ) {
    Token tok = START;
    Token tok_type = getTokenForChar( c );
    Token next_tok = token_transition_map[ tok ][ tok_type ];
    (void) next_tok;
}

void FileReader::processFile( ) {
    //Load both buffer halves so we have a backup ready while we reload the others
    loadBuffers();
    loadBuffers();

    int buffer_id_to_process = 0;
    char *buff = buffer_pool->buff_ptrs[buffer_id_to_process];
    int buff_idx = 0;
    Token cur_state = START;
    Token last_state = START;
    std::vector<Token> tokens_in_line;
    for( ;; ) {
        char next_char = buff[buff_idx++];
        std::cout << "Processing char: " << next_char << std::endl;
        Token next_token = FileReader::getTokenForChar( next_char );
        last_state = cur_state;
        std::cout << "State was: " << cur_state << std::endl;
        cur_state = token_transition_map[ cur_state ][ next_token ];
        std::cout << "State is now: " << cur_state << std::endl;
        assert( cur_state != ERROR );
        assert( !(cur_state == END && last_state == START) );

        // Finished processing a token
        if( cur_state == END ) {
            if( last_state != NEW_LINE ) {
                //Push back the old token
                tokens_in_line.push_back( last_state );

                //Get the state after processing THIS token
                last_state = START;
                cur_state = token_transition_map[ START ][ next_token ];
            } else {
                //TODO: Push back the vector into another, keep going
                std:: cout << "Got First line of tokens: [ ";
                for( const auto &t : tokens_in_line ) {
                    std::cout << t << ", ";
                }
                std::cout << " ]" << std::endl;
                break;
            }
        }
    }
}


FileReader::~FileReader() {
    free( preconstructed_iovecs[0] );
    free( preconstructed_iovecs[1] );
    delete buffer_pool;
    close( fd );
}
