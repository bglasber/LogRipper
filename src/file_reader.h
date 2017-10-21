#include <sys/stat.h>
struct BufferPool {
    unsigned    num_buffs;
    unsigned    buff_sizes;
    char        **buff_ptrs;
public:
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
    END,
    NUM_WORDS
};

class FileReader {

    /* High Level Data --- used to manage reads from disk */
    int             fd; //the file
    unsigned        num_buffers; //number of buffers
    unsigned        half_iovec_cnt; //half the number of buffers
    blksize_t       fs_blksize; //Filesystem block size
    off_t           f_size; //file size
    BufferPool      *buffer_pool; //Pool of buffers to read into;
    int             load_half; //Which half of the buffers to load
    struct iovec    *preconstructed_iovecs[2]; //preconstructed iovec halves for loading

    /* State Data --- used for lexing/parsing the logfile */
public:
    FileReader( int fd, unsigned num_buffers );
    void loadBuffers();
    void processFile();
    static Token getTokenForChar( char c );
    void processNextToken( char c );
    ~FileReader();
};
