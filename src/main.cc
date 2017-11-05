#include "lex.h"
#include "file_reader.h"
#include "rule_applier.h"
#include "binner.h"
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>

//Different word matches will entail different log entries
//We don't want the file names in the GLOG preamble to create different events
void anonymize_glog_preamble( std::vector<TokenWordPair> *tokens_in_line ) {
    if( tokens_in_line->size() <= 18 ) {
        for( unsigned int i = 0; i < tokens_in_line->size(); i++ ) {
            std::cout <<  tokens_in_line->at(i).word << " ";
        }
        std::cout << std::endl;
        return;
    }
    tokens_in_line->at(0).tok = ABSTRACTED_VALUE; //0 is log level
    tokens_in_line->at(1).tok = ABSTRACTED_VALUE; //RUN_ID
    //2 is white_space
    tokens_in_line->at(3).tok = ABSTRACTED_VALUE; //3 is HOUR
    //4 is :
    tokens_in_line->at(5).tok = ABSTRACTED_VALUE; //5 is MINUTE
    //6 is :
    tokens_in_line->at(7).tok = ABSTRACTED_VALUE; //7 is SECOND
    //8 is .
    tokens_in_line->at(9).tok = ABSTRACTED_VALUE; //9 is NANOSECOND
    //10 is whitespace
    tokens_in_line->at(11).tok = ABSTRACTED_VALUE; //11 is thread_id
    //13 is path of C++ file
    tokens_in_line->at(13).tok = ABSTRACTED_VALUE; //C++ FILE_NAME
    //14 is .
    tokens_in_line->at(15).tok = ABSTRACTED_VALUE; //C++ FILE EXT
    //16 is :
    tokens_in_line->at(17).tok = ABSTRACTED_VALUE; //C++ LINE NUMBER
}

int main() {

    int fd = open( "ssServer.log.short", O_RDONLY ); 
    assert( fd > 0 );
    ParseBufferEngine pbe_file_to_rule;
    ParseBufferEngine pbe_rule_to_binner;
    FileReader reader( fd, 128u, &pbe_file_to_rule );

    std::list<RuleFunction> rule_funcs;
    rule_funcs.push_back( anonymize_glog_preamble );
    RuleApplier rule_applier( std::move( rule_funcs ), &pbe_file_to_rule, &pbe_rule_to_binner );

    Binner binner( &pbe_rule_to_binner );

    //Start the pipeline...
    binner.startProcessingBuffers();
    rule_applier.startProcessingBuffers();
    reader.processFile();
    std::cout << "Done processing file." << std::endl;

    rule_applier.terminateWhenDoneProcessing();
    std::cout << "Done waiting for rule applier..." << std::endl;
    binner.terminateWhenDoneProcessing();
    std::cout << "Done waiting for binner..." << std::endl;

    std::ofstream os;
    os.open( "deserialized_map", std::ofstream::out );
    binner.serialize( os );
    return 0;
}
