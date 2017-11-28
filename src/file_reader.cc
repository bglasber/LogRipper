#include "file_reader.h"
#include <assert.h>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/sysinfo.h>
#include <thread>
#include <cstring>

static Token token_transition_map[NUM_WORDS][NUM_WORDS] = {
                 /* START, WORD,  NUMBER,  WHITE_SPACE, PUNCTUATION, NEW_LINE, ERROR, EOF_TOK, END */
/* START */       { ERROR, WORD,  NUMBER,  WHITE_SPACE, PUNCTUATION, NEW_LINE, ERROR, EOF_TOK, END },
/* WORD */        { ERROR, WORD,  END,     END,         END,         END,      ERROR, END,     ERROR},
/* NUMBER */      { ERROR, END,   NUMBER,  END,         END,         END,      ERROR, END,     ERROR},
/* WHITE_SPACE */ { ERROR, END,   END,     WHITE_SPACE, END,         END,      ERROR, END,     ERROR},
/* PUNCTUATION */ { ERROR, END,   END,     END,         END,         END,      ERROR, END,     ERROR},
/* NEW_LINE */    { ERROR, END,   END,     END,         END,         NEW_LINE, ERROR, END,     ERROR},
/* ERROR    */    { ERROR, ERROR, ERROR,   ERROR,       ERROR,       ERROR,    ERROR, ERROR,   ERROR},
/* EOF_TOK  */    { ERROR, ERROR, ERROR,   ERROR,       ERROR,       ERROR,    ERROR, ERROR,   ERROR},
/* END      */    { ERROR, ERROR, ERROR,   ERROR,       ERROR,       ERROR,    ERROR, ERROR,   ERROR},
};

//We use a lookup table for token types rather than if-blocks
//A branch mispredict is ~5 ns
//A L1 cache lookup is 0.5 ns.
//My L1 cache is 32K on 5 year old machine
//This map is 128 * 4 bytes -> 512 bytes
//Therefore, it is better to keep this map in L1 cache and do lookups from it
static Token char_to_token_type_map[128] = {
    EOF_TOK, //\0
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

FileReader::FileReader( int fd, unsigned num_buffers, ParseBufferEngine *pbe ) : fd( fd ), num_buffers( num_buffers ) {
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
    expected_to_read = fs_blksize * half_iovec_cnt;
    found_last_buff = false;

    parse_buffer_engine = pbe;
}

bool FileReader::loadBuffers( ) {
    ssize_t bytes_read = readv( fd, preconstructed_iovecs[load_half], half_iovec_cnt );
    std::cout << "fd " << fd << std::endl;;
    std::cout << "half_iovec_cnt: " << half_iovec_cnt << std::endl;
    std::cout << "preconstructed iovecs: " << &(preconstructed_iovecs[load_half][0]) << std::endl;
    std::cout << "Loaded " << bytes_read << " bytes." << std::endl;
    if( expected_to_read > bytes_read ) {
        std::cout << "SYNC: expected to load " << expected_to_read << " bytes, but only loaded " << bytes_read << "bytes. Likely EOF!" << std::endl;
        found_last_buff = true;
        last_buff_id = (bytes_read / fs_blksize) + (load_half * half_iovec_cnt);
        bytes_in_last_buff = bytes_read % fs_blksize;
    }
    load_half = (load_half + 1) % 2;
    return found_last_buff;
}

void FileReader::asyncReloadBuffers( ) {
    for( ;; ) {
        std::unique_lock<std::mutex> lk( mut );
        cv.wait( lk, [this]{ return async_reload; } );
        ssize_t bytes_read = readv( fd, preconstructed_iovecs[load_half], half_iovec_cnt );
        if( expected_to_read > bytes_read ) {
            std::cout << "ASYNC: expected to load " << expected_to_read << " bytes, but only loaded " << bytes_read << "bytes. Likely EOF!" << std::endl;
            std::cout << "ASYNC: Done file." << std::endl;
            found_last_buff = true;
            last_buff_id = (bytes_read / fs_blksize) + (load_half * half_iovec_cnt);
            bytes_in_last_buff = bytes_read % fs_blksize;
            break;
        }
        //std::cout << "Async reloaded buffer half: " << load_half << std::endl;
        load_half = (load_half + 1) % 2;
        async_reload = false;
    }
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
    bool last_buff = loadBuffers();
    //Haven't hit EOF yet
    if( !last_buff ) {
        last_buff = loadBuffers();

        if( !last_buff ) {
            //Start async reload thread
            async_reload = false;
            std::thread t( &FileReader::asyncReloadBuffers, this );
            t.detach();
        }
    }
    unsigned buffer_id_to_process = 0;
    char *buff = buffer_pool->buff_ptrs[buffer_id_to_process];
    unsigned buff_idx = 0;
    Token cur_state = START;
    Token last_state = START;
    Token next_token;
    std::string token_buff;
    std::unique_ptr<ParseBuffer> buffer = std::make_unique<ParseBuffer>();
    std::unique_ptr<std::vector<TokenWordPair>> tokens_in_line = std::make_unique<std::vector<TokenWordPair>>();
    for( ;; ) {
        if( buff_idx >= fs_blksize ) {
            //std::cout << "Wrapping around on buffers..." << std::endl;
            buff_idx = 0;
            buffer_id_to_process++;
            buff = buffer_pool->buff_ptrs[buffer_id_to_process == num_buffers ? 0 : buffer_id_to_process ];

            //If we've crossed into the second half of the buffers, async reload the
            //first half
            if( buffer_id_to_process == half_iovec_cnt ) {
                if( found_last_buff && last_buff_id == buffer_id_to_process &&
                    bytes_in_last_buff == 0 ) {
                    //We are out of buffers, push the state and return, we ended on a boundary

                    TokenWordPair twp;
                    twp.tok = cur_state;
                    twp.word = token_buff;
                    tokens_in_line->push_back( twp );
                    buffer->addLine( std::move( tokens_in_line ) );
                    //No need to get a new buff, because we are done parsing
                    //Just signal this buffer as ready
                    parse_buffer_engine->putNextBuffer( std::move( buffer ) );
#if !defined( NDEBUG )
                    buffer = NULL;
                    tokens_in_line = nullptr;
#endif
                    break;
                }
                if( found_last_buff ) {
                    //No need to block on async reloads, that thread is shut down
                    goto PARSER_MAIN;
                }
                //Wait for async reload
                bool done_flag = false;
                while( async_reload ) {
                    if( found_last_buff && last_buff_id == buffer_id_to_process &&
                        bytes_in_last_buff == 0 ) {
                        //We are out of buffers, push the state and return, we ended on a boundary
                        TokenWordPair twp;
                        twp.tok = cur_state;
                        twp.word = token_buff;
                        tokens_in_line->push_back( twp );
                        buffer->addLine( std::move( tokens_in_line ) );
                        //No need to get a new buff, because we are done parsing
                        //Just signal this buffer as ready
                        parse_buffer_engine->putNextBuffer( std::move( buffer ) );
#if !defined( NDEBUG )
                        buffer = NULL;
                        tokens_in_line = nullptr;
#endif
                        done_flag = true;
                        break;
                    }
                    if( found_last_buff ) {
                        //No need to block on async reloads, that thread is shut down
                        goto PARSER_MAIN;
                    }
                    std::this_thread::yield();
                        //std::cout << "Warning... waiting for buffers!" << std::endl;
                }
                if( done_flag ) {
                    break;
                }
                async_reload = true;
                cv.notify_one();
            //If we're the max buffer, hop back to the first buffer and async reload
            //the second half
            } else if( buffer_id_to_process == num_buffers ) {
                if( found_last_buff && last_buff_id == buffer_id_to_process &&
                    bytes_in_last_buff == 0 ) {
                    //We are out of buffers, push the state and return, we ended on a boundary
                    TokenWordPair twp;
                    twp.tok = cur_state;
                    twp.word = token_buff;
                    tokens_in_line->push_back( twp );
                    buffer->addLine( std::move( tokens_in_line ) );
                    //No need to get a new buff, because we are done parsing
                    //Just signal this buffer as ready
                    parse_buffer_engine->putNextBuffer( std::move( buffer ) );
#if !defined( NDEBUG )
                    buffer = NULL;
                    tokens_in_line = nullptr;
#endif
                    break;
                }
                if( found_last_buff ) {
                    //No need to block on async reloads, that thread is shut down
                    buffer_id_to_process = 0;
                    goto PARSER_MAIN;
                }
                //Not sure how many buffers remain, but we are waiting on them to be reloaded
                bool done_flag = false;
                while( async_reload ) {
                    if( found_last_buff && last_buff_id == buffer_id_to_process &&
                        bytes_in_last_buff == 0 ) {
                        //We are out of buffers, push the state and return, we ended on a boundary

                        //TODO: This is a bunch of copies, need to fix
                        TokenWordPair twp;
                        twp.tok = cur_state;
                        twp.word = token_buff;
                        tokens_in_line->push_back( twp );
                        buffer->addLine( std::move( tokens_in_line ) );
                        //No need to get a new buff, because we are done parsing
                        //Just signal this buffer as ready
                        parse_buffer_engine->putNextBuffer( std::move( buffer ) );
#if !defined( NDEBUG )
                        buffer = NULL;
                        tokens_in_line = nullptr;
#endif
                        done_flag = true;
                        break;
                    }
                    if( found_last_buff ) {
                        //No need to block on async reloads, that thread is shut down
                        buffer_id_to_process = 0;
                        goto PARSER_MAIN;
                    }
                    std::this_thread::yield();
                }
                if( done_flag ) {
                    break;
                }
                async_reload = true;
                cv.notify_one();
                buffer_id_to_process = 0;
            }
        }
PARSER_MAIN:
        bool countdown_bytes = found_last_buff && (last_buff_id == buffer_id_to_process);
        if( countdown_bytes && bytes_in_last_buff == 0 ) {
            //Alright, we are out of bytes, push what we have and return
            TokenWordPair twp;
            twp.tok = cur_state;
            twp.word = token_buff;
            tokens_in_line->push_back( twp );
            buffer->addLine( std::move( tokens_in_line ) );
            //No need to get a new buff, because we are done parsing
            //Just signal this buffer as ready
            parse_buffer_engine->putNextBuffer( std::move( buffer ) );
#if !defined( NDEBUG )
            buffer = NULL;
            tokens_in_line = nullptr;
#endif
            break;
        }

        char next_char = buff[buff_idx++];
        next_token = FileReader::getTokenForChar( next_char );
        //std::cout << "Got next token: " << next_token << std::endl;
        last_state = cur_state;
        cur_state = token_transition_map[ cur_state ][ next_token ];
        assert( cur_state != ERROR );
        assert( !(cur_state == END && last_state == START) );

        // Finished processing a token ?
        if( cur_state == END ) {
            if( last_state != NEW_LINE ) {
                //Push back the old token
                //std::cout << "Pushing back " << last_state << std::endl;
                TokenWordPair twp;
                twp.tok = last_state;
                twp.word = token_buff;
                tokens_in_line->push_back( twp );

                //Make a new buffer
                token_buff.clear();

                //Get the state after processing THIS token
                last_state = START;
                cur_state = token_transition_map[ START ][ next_token ];
            } else {
                //Push line into buffer
                TokenWordPair twp;
                twp.tok = NEW_LINE;
                twp.word = token_buff;
                tokens_in_line->push_back( twp );

                bool buff_done = buffer->addLine( std::move( tokens_in_line ) );
                //Buffer full, make a new one
                if( buff_done ) {
                    parse_buffer_engine->putNextBuffer( std::move( buffer ) );
                    struct sysinfo info;
RECHECK_MEM_AVAIL:
                    int rc = sysinfo( &info );
                    assert( rc == 0 );
                    if( info.freeram < 1024*1000*100 /* <100 MB of space remaining */ ) {
                        //throttle so we can clear up some space
                        std::this_thread::sleep_for( std::chrono::milliseconds(300) );
                        std::cout << "FR: waiting for memory to be available..." << std::endl;
                        goto RECHECK_MEM_AVAIL;
                    }
                    buffer = std::make_unique<ParseBuffer>();
                }
                tokens_in_line = std::make_unique<std::vector<TokenWordPair>>();

                //If we hit EOF as the next char, then just finish
                if( next_token == EOF_TOK ) {
                    std::cout << "Done processing file!" << std::endl;
                    break;
                }

                //Set the next buff
                token_buff.clear();

                //Else, figure out what the token should be (bypass peek step) --- Reset state
                last_state = START;
                cur_state = token_transition_map[ START ][ next_token ];
            }
        } else if( cur_state == EOF_TOK ) {
            std::cout << "Done processing file!" << std::endl;
            break;
        }

        token_buff.append( 1, next_char );

        if( countdown_bytes ) {
            bytes_in_last_buff--;
        }
    }
}

FileReader::~FileReader() {
    free( preconstructed_iovecs[0] );
    free( preconstructed_iovecs[1] );
    delete buffer_pool;
    close( fd );
}
