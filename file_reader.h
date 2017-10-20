#include <sys/stat.h>

struct BufferPool {
    unsigned    num_buffs;
    int         buff_sizes;
    char        **buff_ptrs;
public:
    BufferPool( int num_buffs, int buff_sizes );
    ~BufferPool();
};

class FileReader {
    unsigned        num_buffers; //number of buffers
    int             fd; //the file
    blksize_t       fs_blksize; //Filesystem block size
    off_t           f_size; //file size
    BufferPool      *buffer_pool; //Pool of buffers to read into;
    int             load_half; //Which half of the buffers to load
    struct iovec    *preconstructed_iovecs[2]; //preconstructed iovec halves for loading
public:
    FileReader( int fd, unsigned num_buffers );
    void loadBuffers();
    ~FileReader();
};
