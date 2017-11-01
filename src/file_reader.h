#include <sys/stat.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
struct BufferPool {
    unsigned    num_buffs;
    unsigned    buff_sizes;
    char        **buff_ptrs; public:
    BufferPool( unsigned num_buffs, unsigned buff_sizes );
    ~BufferPool();
};

enum Token {
    START = 0,
    WORD,
    NUMBER,
    WHITE_SPACE,
    PUNCTUATION,
    NEW_LINE,
    ERROR,
    EOF_TOK,
    END,
    NUM_WORDS
};

class FileReader {

    /* High Level Data --- used to manage reads from disk */
    int                             fd; //the file
    unsigned                        num_buffers; //number of buffers
    unsigned                        half_iovec_cnt; //half the number of buffers
    blksize_t                       fs_blksize; //Filesystem block size
    off_t                           f_size; //file size
    BufferPool                      *buffer_pool; //Pool of buffers to read into;
    int                             load_half; //Which half of the buffers to load
    struct iovec                    *preconstructed_iovecs[2]; //preconstructed iovec halves for loading

    bool                            async_reload; //Avoid spurious wakeup
    bool                            done_file; //mark if we're done
    std::mutex                      mut; //For CV
    std::condition_variable         cv; //Block background reload thread
    ssize_t                         expected_to_read; //# of bytes we expect to load
    std::vector<std::vector<Token>> parsed_lines; //Track what we've parsed

    /* State Data --- used for lexing/parsing the logfile */
public:
    FileReader( int fd, unsigned num_buffers );
    bool loadBuffers();
    void asyncReloadBuffers();
    void processFile();
    static Token getTokenForChar( char c );
    void processNextToken( char c );
    std::vector<std::vector<Token>> *getParsedData();
    ~FileReader();
};
