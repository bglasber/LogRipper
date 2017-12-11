#include "rules.h"

void anonymize_log_preamble( std::unique_ptr<std::vector<TokenWordPair>> &line ) {
    if( line->size() < 35 ) {
        return;
    }
    //The port we connected from
    line->at(0).tok = ABSTRACTED_VALUE; //NUMBER
    //PUNC
    line->at(2).tok = ABSTRACTED_VALUE; //NUMBER
    //PUNC
    line->at(4).tok = ABSTRACTED_VALUE; //NUMBER
    //PUNC
    line->at(6).tok = ABSTRACTED_VALUE; //NUMBER
    //PUNC
    line->at(8).tok = ABSTRACTED_VALUE; //NUMBER
    //PUNC
    line->at(10).tok = ABSTRACTED_VALUE; //NUMBER
    //PUNC
    line->at(12).tok = ABSTRACTED_VALUE; //NUMBER
    //PUNC
    line->at(14).tok = ABSTRACTED_VALUE; //NUMBER
    //WHITE_SPACE
    //PUNC "-"
    //WHITE_SPACE
    //PUNC "-"
    //WHITE_SPACE //19
    //PUNC 20
    line->at(21).tok = ABSTRACTED_VALUE; //NUMBER
    assert( line->at(22).word == "/" ); //PUNC
    line->at(23).tok = ABSTRACTED_VALUE; //WORD, month
    assert( line->at(24).word == "/" ); //PUNC
    line->at(25).tok = ABSTRACTED_VALUE; //NUMBER, year
    // PUNC, :
    line->at(27).tok = ABSTRACTED_VALUE; //NUMBER, hour
    // PUNC, :
    line->at(29).tok = ABSTRACTED_VALUE; //NUMBER, minute
    // PUNC, :
    line->at(31).tok = ABSTRACTED_VALUE; //NUMBER, second
    // white space, 32
    // - symbol, 33
    line->at(34).tok = ABSTRACTED_VALUE; //GMT -5
}

void anonymize_size_of_page( std::unique_ptr<std::vector<TokenWordPair>> &line ) {
    size_t num_toks = line->size();
    line->at( num_toks-2 ).tok = ABSTRACTED_VALUE;
}
void anonymize_id( std::unique_ptr<std::vector<TokenWordPair>> &line, std::string id ) {
    int found_loc = -1;
    for( unsigned ind = 39; ind < line->size(); ind++ ) {
        if( line->at( ind ).tok == WORD && line->at( ind ).word == id ) {
            found_loc = (int) ind;
        }
    }
    if( found_loc == -1 ) {
        return;
    }

    auto insert_loc = line->begin() + found_loc;
    auto pos = insert_loc;
    pos++; //at equals sign
    pos++; //at actual thing now
    auto end_delete_loc = pos;
    while( end_delete_loc->tok != WHITE_SPACE ) {
        end_delete_loc++;
    }
    line->erase( pos, end_delete_loc );
    
    insert_loc++; //at equals sign
    insert_loc++; //after deletion point

    TokenWordPair twp;
    twp.tok = ABSTRACTED_VALUE;
    twp.word = "";

    line->insert( insert_loc, std::move( twp ) );
}

void anonymize_product_id( std::unique_ptr<std::vector<TokenWordPair>> &line ) {
    anonymize_id( line, "productId" );
}

void anonymize_item_id( std::unique_ptr<std::vector<TokenWordPair>> &line ) {
    anonymize_id( line, "itemId" );
}

void anonymize_category_id( std::unique_ptr<std::vector<TokenWordPair>> &line ) {
    anonymize_id( line, "categoryId" );
   
}
void anonymize_working_item_id( std::unique_ptr<std::vector<TokenWordPair>> &line ) {
    anonymize_id( line, "workingItemId" );
}

uint64_t get_thread_id_from_parsed_line( std::shared_ptr<std::vector<TokenWordPair>> &line ) {
    if( line->size() < 12 ) {
        return 0;
    }
    return 0;
}

