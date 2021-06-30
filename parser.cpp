#include "parser.h"
#include "token.h"
#define S(A) look.tag==A
#define _(A) ||look.tag==A
bool isType(Tag t) {
    int i = static_cast<int>(t);
    return i >= static_cast<int>(Tag::KW_INT) &&
           i <= static_cast<int>(Tag::KW_VOID);
}

bool isCmp(Tag t) {
    int i = static_cast<int>(t);
    return i >= static_cast<int>(Tag::GT) &&
           i <= static_cast<int>(Tag::NEQU);
}

void Parser::analyse() {
    nextw();
    program();
}

void Parser::program() {
    if (S(Tag::END)) {
        return;
    } else {
        def();
        program();
    }
}

void Parser::def() {
    if (isType(look.tag)){
        nextw();
        if(S(Tag::ID)){
            nextw();
        }
        def_info();
    }else if (S(Tag::ID)){
        //不应存在
        assign();
    }else{
        exp();
        if(S(Tag::SEMICON)){
            nextw();
        }
    }
}

void Parser::def_info(){
    var_data();
    def_list();
}

void Parser::def_list(){
    if(S(Tag::SEMICON)){
        nextw();
        return;
    }
}

void Parser::var_data(){
    if(S(Tag::EQU)){
        nextw();
    }
    exp();
}

Var* Parser::exp(){
    return or_exp();
}

Var* Parser::or_exp(){
    or_tail(and_exp());
}

Var* Parser::or_tail(Var* lval){
    if(S(Tag::OR)){
        nextw();
        return or_tail(and_exp());
    }
    return lval;
}

Var* Parser::and_exp(){
    and_tail(cmp_exp());
}

Var* Parser::and_tail(Var* lval){
    if(S(Tag::AND)){
        nextw();
        return and_tail(cmp_exp());
    }
    return lval;
}

Var* Parser::cmp_exp(){
    cmp_tail(alo_exp());
}

Var* Parser::cmp_tail(Var* lval){
    if(isCmp(look.tag)){
        nextw();
        return cmp_tail(alo_exp());
    }
    return lval;
}

Var* Parser::alo_exp(){
    alo_tail(item());
}

Var* Parser::alo_tail(Var* lval){
    if(S(Tag::ADD)_(Tag::OR)){
        nextw();
        return alo_tail(item());
    }
    return lval;
}

Var* Parser::item(){
    item_tail(factor());
}

Var* Parser::item_tail(Var* lval){
    if(S(Tag::MUL)_(Tag::DIV)_(Tag::MOD)){
        nextw();
        return item_tail(factor());
    }
    return lval;
}

Var* Parser::factor(){
    return literal();
}

Var* Parser::literal(){
    Var* v;
    //Var* v = new Var(look);
    return v;
}