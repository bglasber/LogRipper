#include "file_reader.h"
#include "assert.h"
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/uio.h>

BufferPool::BufferPool( int num_buffs, int buff_sizes ) : num_buffs( num_buffs ), buff_sizes( buff_sizes ) {
    buff_ptrs = (char **) malloc( sizeof( char * ) * num_buffs );
    for( int i = 0; i < num_buffs; i++ ) {
        buff_ptrs[i] = (char *) malloc( sizeof( char ) * buff_sizes );
    }
}

BufferPool::~BufferPool() {
    for( int i = 0; i < num_buffs; i++ ) {
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
    preconstructed_iovecs[load_half];
    ssize_t bytes_read = readv( fd, preconstructed_iovecs[load_half], half_iovec_cnt );
    std::cout << "fd " << fd << std::endl;;
    std::cout << "half_iovec_cnt: " << half_iovec_cnt << std::endl;
    std::cout << "preconstructed iovecs: " << &(preconstructed_iovecs[load_half][0]) << std::endl;
    load_half = (load_half + 1) % 2;
    std::cout << "Loaded " << bytes_read << " bytes." << std::endl;
}

void FileReader::processFile( ) {
    //Load both buffer halves so we have a backup ready while we reload the others
    loadBuffers();
    loadBuffers();

    int buffer_id_to_process = 0;
    char *buff = buffer_pool->buff_ptrs[buffer_id_to_process];
    int buff_idx = 0;
    for( ;; ) {
        std::cout << buff[buff_idx++];
        std::cout << buff[buff_idx++];
        std::cout << buff[buff_idx++];
        std::cout << buff[buff_idx++];
        std::cout << buff[buff_idx++];
        std::cout << buff[buff_idx++];
        std::cout << buff[buff_idx++];
        std::cout << buff[buff_idx++];
        std::cout << buff[buff_idx++];
        std::cout << buff[buff_idx++];
        std::cout << buff[buff_idx++];
        std::cout << buff[buff_idx++];
        std::cout << buff[buff_idx++];
        std::cout << buff[buff_idx++] << std::endl;
        break;
    }
}

void FileReader::processToken( ) {
    Token tok = START;
}

FileReader::~FileReader() {
    free( preconstructed_iovecs[0] );
    free( preconstructed_iovecs[1] );
    delete buffer_pool;
    close( fd );
}
