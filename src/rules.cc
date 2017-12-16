#include "rules.h"
#include <iostream>

void anonymize_log_preamble( std::unique_ptr<std::vector<TokenWordPair>> &line ) {

    //The port we connected from
    line->at(0).tok = ABSTRACTED_VALUE; //NUMBER 127
    //PUNC .
    line->at(2).tok = ABSTRACTED_VALUE; //NUMBER 0
    //PUNC .
    line->at(4).tok = ABSTRACTED_VALUE; //NUMBER 0
    //PUNC .
    line->at(6).tok = ABSTRACTED_VALUE; //NUMBER 1
    //WHITE_SPACE 7
    //PUNC "-" 8 //This is the thread Id
    auto iter = line->begin() + 7; //Space Before
    auto delete_start = iter+1;
    auto delete_end = delete_start;
    TokenWordPair twp;
    twp.tok = ABSTRACTED_VALUE;
    twp.word = "";
    while( delete_end->tok != WHITE_SPACE ) {
        twp.word += delete_end->word;
        delete_end++;
    }
    line->erase( delete_start, delete_end );
    iter++;
    line->insert( iter, std::move( twp ) );
    
    //WHITE_SPACE 9
    //PUNC "-" 10
    //WHITE_SPACE //11
    //PUNC 12
    line->at(13).tok = ABSTRACTED_VALUE; //NUMBER
    assert( line->at(14).word == "/" ); //PUNC
    line->at(15).tok = ABSTRACTED_VALUE; //WORD, month
    assert( line->at(16).word == "/" ); //PUNC
    line->at(17).tok = ABSTRACTED_VALUE; //NUMBER, year
    // PUNC, :18
    line->at(19).tok = ABSTRACTED_VALUE; //NUMBER, hour
    // PUNC, :20
    line->at(21).tok = ABSTRACTED_VALUE; //NUMBER, minute
    // PUNC, :22
    line->at(23).tok = ABSTRACTED_VALUE; //NUMBER, second
    // white space, 24
    // - symbol, 25
    line->at(26).tok = ABSTRACTED_VALUE; //GMT -5
}

void anonymize_size_of_page( std::unique_ptr<std::vector<TokenWordPair>> &line ) {
    size_t num_toks = line->size();
    line->at( num_toks-2 ).tok = ABSTRACTED_VALUE;
}

void anonymize_query_string( std::unique_ptr<std::vector<TokenWordPair>> &line ) {
    auto insert_loc = line->begin() + 35;
    auto pos = insert_loc;
    pos++; //at ?
    auto end_delete_loc = pos;
    TokenWordPair twp;
    twp.tok = ABSTRACTED_VALUE;

    while( end_delete_loc->tok != WHITE_SPACE ) {
        //We need to extract the username/customerid to determine the thread
        if( end_delete_loc->tok == WORD && end_delete_loc->word == "username" ) {
            twp.word = (end_delete_loc+3)->word;
        } else if( end_delete_loc->tok == WORD && end_delete_loc->word == "customerid" ) {
            twp.word = (end_delete_loc+2)->word;
        }
        end_delete_loc++;
    }
    insert_loc++;
    line->erase( pos, end_delete_loc );
    line->insert( insert_loc, std::move( twp ) );
}

uint64_t get_thread_id_from_parsed_line( std::shared_ptr<std::vector<TokenWordPair>> &line ) {
    if( line->size() < 37 ) {
        return 0;
    }
    std::string &str = line->at(36).word;
    return atoi(str.c_str());
}
