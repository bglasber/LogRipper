#include <sys/stat.h>

struct BufferPool {
    unsigned    num_buffs;
    int         buff_sizes;
    char        **buff_ptrs;
public:
    BufferPool( int num_buffs, int buff_sizes );
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

static Token char_to_token_type_map[128] = {
    ERROR, //0
    ERROR,
    ERROR,
    ERROR,
    ERROR,
    ERROR,
    ERROR,
    ERROR,
    ERROR,
    WHITE_SPACE, //9 = \t 
    NEW_LINE, //10 = \n
    ERROR, //11, bunch of special chars
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
    WORD,
    WORD, //Z
    PUNCTUATION, // [
    PUNCTUATION, 
    PUNCTUATION, 
    PUNCTUATION, 
    PUNCTUATION, 
    PUNCTUATION, // [
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
    PUNCTUATION,
    PUNCTUATION,
    PUNCTUATION,
    PUNCTUATION,
    PUNCTUATION,
    ERROR,
};

class FileReader {

    /* High Level Data --- used to manage reads from disk */
    unsigned        num_buffers; //number of buffers
    unsigned        half_iovec_cnt; //half the number of buffers
    int             fd; //the file
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
    void processToken();
    ~FileReader();
};
