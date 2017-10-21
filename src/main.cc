#include "lex.h"
#include "file_reader.h"
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main() {

    int fd = open( "ssServer.log", O_RDONLY ); 
    assert( fd > 0 );
    FileReader reader( fd, 64u );
    reader.processFile();
    return 0;
}
