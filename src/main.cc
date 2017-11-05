#include "lex.h"
#include "file_reader.h"
#include "rule_applier.h"
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main() {

    int fd = open( "ssServer.log", O_RDONLY ); 
    assert( fd > 0 );
    ParseBufferEngine pbe_file_to_rule;
    ParseBufferEngine pbe_rule_to_out;
    FileReader reader( fd, 128u, &pbe_file_to_rule );

    std::list<RuleFunction> rule_funcs;
    RuleApplier rule_applier( std::move( rule_funcs ), &pbe_file_to_rule, &pbe_rule_to_out );
    reader.processFile();
    return 0;
}
