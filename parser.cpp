#include "parser.h"
#include "token.h"
void Parser::analyse(){
    nextw();
    program();
}

void Parser::program(){
    if(match(Tag::END)){
        return;
    }else{
        segment();
        program();
    }
}

void Parser::segment(){
    if(match(Tag::KW))
    nextw();
    
}