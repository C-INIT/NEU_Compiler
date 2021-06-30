#include "parser.h"
#include "token.h"

/*******************************************************************************
                                   语法分析器
*******************************************************************************/

Parser::Parser(Lexer &lex, SymTab &tab, GenIR &inter)
    : lexer(lex), symtab(tab), ir(inter) {}

/*
    语法分析主程序
*/
void Parser::analyse() {
    move(); //预先读入
    program();
}

/*
    移进
*/
void Parser::move() {
    look = lexer.tokenize();
    if (Args::showToken)
        printf("%s\n", look->toString().c_str()); //输出词法记号——测试
}

/*
    匹配,查看并移动
*/
bool Parser::match(Tag need) {
    if (look->tag == need) {
        move();
        return true;
    } else
        return false;
}

//打印语法错误
#define Tag::SYNERROR(code, t) Error::synError(code, t)

/*
    错误修复
*/
#define _(T) || look->tag == T
#define F(C) look->tag == C
void Parser::recovery(bool cond, SynError lost, SynError wrong) {
    if (cond) /*在给定的Follow集合内*/
        Tag::SYNERROR(lost, look);
    else {
        Tag::SYNERROR(wrong, look);
        move();
    }
}

//类型
#define Tag::TYPE_FIRST F(Tag::KW_INT) _(Tag::KW_CHAR) _(Tag::KW_VOID)
//表达式
#define Tag::EXPR_FIRST                                                             \
    F(Tag::LPAREN)                                                                  \
    _(Tag::NUM) _(Tag::CH) _(Tag::STR) _(Tag::ID) _(Tag::NOT) _(Tag::SUB) _(Tag::LEA) _(Tag::MUL) _(Tag::INC) _(Tag::DEC)
//左值运算
#define Tag::LVAL_OPR                                                               \
    F(Tag::ASSIGN)                                                                  \
    _(Tag::OR) _(Tag::AND) _(Tag::GT) _(Tag::GE) _(Tag::LT) _(Tag::LE) _(Tag::EQU) _(Tag::NEQU) _(Tag::ADD) _(Tag::SUB) _(Tag::MUL)   \
        _(Tag::DIV) _(Tag::MOD) _(Tag::INC) _(Tag::DEC)
//右值运算
#define Tag::RVAL_OPR                                                               \
    F(Tag::OR)                                                                      \
    _(Tag::AND) _(Tag::GT) _(Tag::GE) _(Tag::LT) _(Tag::LE) _(Tag::EQU) _(Tag::NEQU) _(Tag::ADD) _(Tag::SUB) _(Tag::MUL) _(Tag::DIV)  \
        _(Tag::MOD)
//语句
#define Tag::STATEMENT_FIRST                                                        \
    (Tag::EXPR_FIRST) _(Tag::SEMICON) _(Tag::KW_WHILE) _(Tag::KW_FOR) _(Tag::KW_DO) _(Tag::KW_IF)            \
        _(Tag::KW_SWITCH) _(Tag::KW_RETURN) _(Tag::KW_BREAK) _(Tag::KW_CONTINUE)

/*
    <program>			->	<segment><program>|^
*/
void Parser::program() {
    if (F(Tag::END)) { //分析结束

        return;
    } else {
        segment();
        program();
    }
}

/*
    <segment>			->	rsv_extern <type><def>|<type><def>
*/
void Parser::segment() {
    bool ext = match(Tag::KW_EXTERN); //记录声明属性
    Tag t = type();
    def(ext, t);
}

/*
    <type>				->	rsv_int|rsv_char|rsv_bool|rsv_void
*/
Tag Parser::type() {
    Tag tmp = Tag::KW_INT; //默认类型
    if (Tag::TYPE_FIRST) {
        tmp = look->tag; //记录类型
        move();          //移进
    } else               //报错
        recovery(F(Tag::ID) _(Tag::MUL), Tag::TYPE_LOST, Tag::TYPE_WRONG);
    return tmp; //记录类型
}

/*
    <defdata>			->	ident <varrdef>|mul ident <init>
*/
Var *Parser::defdata(bool ext, Tag t) {
    string name = "";
    if (F(Tag::ID)) {
        name = (((Id *)look)->name);
        move();
        return varrdef(ext, t, false, name);
    } else if (match(Tag::MUL)) {
        if (F(Tag::ID)) {
            name = (((Id *)look)->name);
            move();
        } else
            recovery(F(Tag::SEMICON) _(Tag::COMMA) _(Tag::ASSIGN), Tag::ID_LOST, Tag::ID_WRONG);
        return init(ext, t, true, name);
    } else {
        recovery(F(Tag::SEMICON) _(Tag::COMMA) _(Tag::ASSIGN) _(Tag::LBRACK), Tag::ID_LOST, Tag::ID_WRONG);
        return varrdef(ext, t, false, name);
    }
}

/*
    <deflist>			->	comma <defdata> <deflist>|semicon
*/
void Parser::deflist(bool ext, Tag t) {
    if (match(Tag::COMMA)) { //下一个声明
        symtab.addVar(defdata(ext, t));
        deflist(ext, t);
    } else if (match(Tag::SEMICON)) //最后一个声明
        return;
    else {                  //出错了
        if (F(Tag::ID) _(Tag::MUL)) { //逗号
            recovery(1, Tag::COMMA_LOST, Tag::COMMA_WRONG);
            symtab.addVar(defdata(ext, t));
            deflist(ext, t);
        } else
            recovery(Tag::TYPE_FIRST || Tag::STATEMENT_FIRST || F(Tag::KW_EXTERN) _(Tag::RBRACE),
                     Tag::SEMICON_LOST, Tag::SEMICON_WRONG);
    }
}

/*
    <varrdef>			->	lbrack num rbrack | <init>
*/
Var *Parser::varrdef(bool ext, Tag t, bool ptr, string name) {
    if (match(Tag::LBRACK)) {
        int len = 0;
        if (F(Tag::NUM)) {
            len = ((Num *)look)->val;
            move();
        } else
            recovery(F(Tag::RBRACK), Tag::NUM_LOST, Tag::NUM_WRONG);
        if (!match(Tag::RBRACK))
            recovery(F(Tag::COMMA) _(Tag::SEMICON), Tag::RBRACK_LOST, Tag::RBRACK_WRONG);
        return new Var(symtab.getScopePath(), ext, t, name, len); //新的数组
    } else
        return init(ext, t, ptr, name);
}

/*
    <init>				->	assign <expr>|^
*/
Var *Parser::init(bool ext, Tag t, bool ptr, string name) {
    Var *initVal = Tag::NULL;
    if (match(Tag::ASSIGN)) {
        initVal = expr();
    }
    return new Var(symtab.getScopePath(), ext, t, ptr, name,
                   initVal); //新的变量或者指针
}

/*
    <def>					->	mul id <init><deflist>|ident <idtail>
*/
void Parser::def(bool ext, Tag t) {
    string name = "";
    if (match(Tag::MUL)) { //指针
        if (F(Tag::ID)) {
            name = (((Id *)look)->name);
            move();
        } else
            recovery(F(Tag::SEMICON) _(Tag::COMMA) _(Tag::ASSIGN), Tag::ID_LOST, Tag::ID_WRONG);
        symtab.addVar(init(ext, t, true, name)); //新建一个指针变量
        deflist(ext, t);
    } else {
        if (F(Tag::ID)) { //变量 数组 函数
            name = (((Id *)look)->name);
            move();
        } else
            recovery(F(Tag::SEMICON) _(Tag::COMMA) _(Tag::ASSIGN) _(Tag::LPAREN) _(Tag::LBRACK), Tag::ID_LOST,
                     Tag::ID_WRONG);
        idtail(ext, t, false, name);
    }
}

/*
    <idtail>			->	<varrdef><deflist>|lparen <para> rparen <funtail>
*/
void Parser::idtail(bool ext, Tag t, bool ptr, string name) {
    if (match(Tag::LPAREN)) { //函数
        symtab.enter();
        vector<Var *> paraList; //参数列表
        para(paraList);
        if (!match(Tag::RPAREN))
            recovery(F(Tag::LBRACK) _(Tag::SEMICON), Tag::RPAREN_LOST, Tag::RPAREN_WRONG);
        Fun *fun = new Fun(ext, t, name, paraList);
        funtail(fun);
        symtab.leave();
    } else {
        symtab.addVar(varrdef(ext, t, false, name));
        deflist(ext, t);
    }
}

/*
    <paradatatail>->	lbrack rbrack|lbrack num rbrack|^
*/
Var *Parser::paradatatail(Tag t, string name) {
    if (match(Tag::LBRACK)) {
        int len = 1; //参数数组忽略长度
        if (F(Tag::NUM)) {
            len = ((Num *)look)->val;
            move();
        } //可以没有指定长度
        if (!match(Tag::RBRACK))
            recovery(F(Tag::COMMA) _(Tag::RPAREN), Tag::RBRACK_LOST, Tag::RBRACK_WRONG);
        return new Var(symtab.getScopePath(), false, t, name, len);
    }
    return new Var(symtab.getScopePath(), false, t, false, name);
}

/*
    <paradata>		->	mul ident|ident <paradatatail>
*/
Var *Parser::paradata(Tag t) {
    string name = "";
    if (match(Tag::MUL)) {
        if (F(Tag::ID)) {
            name = ((Id *)look)->name;
            move();
        } else
            recovery(F(Tag::COMMA) _(Tag::RPAREN), Tag::ID_LOST, Tag::ID_WRONG);
        return new Var(symtab.getScopePath(), false, t, true, name);
    } else if (F(Tag::ID)) {
        name = ((Id *)look)->name;
        move();
        return paradatatail(t, name);
    } else {
        recovery(F(Tag::COMMA) _(Tag::RPAREN) _(Tag::LBRACK), Tag::ID_LOST, Tag::ID_WRONG);
        return new Var(symtab.getScopePath(), false, t, false, name);
    }
}

/*
    <para>				->	<type><paradata><paralist>|^
*/
void Parser::para(vector<Var *> &list) {
    if (F(Tag::RPAREN))
        return;
    Tag t = type();
    Var *v = paradata(t);
    symtab.addVar(v); //保存参数到符号表
    list.push_back(v);
    paralist(list);
}

/*
    <paralist>		->	comma<type><paradata><paralist>|^
*/
void Parser::paralist(vector<Var *> &list) {
    if (match(Tag::COMMA)) { //下一个参数
        Tag t = type();
        Var *v = paradata(t);
        symtab.addVar(v);
        list.push_back(v);
        paralist(list);
    }
}

/*
    <funtail>			->	<block>|semicon
*/
void Parser::funtail(Fun *f) {
    if (match(Tag::SEMICON)) { //函数声明
        symtab.decFun(f);
    } else { //函数定义
        symtab.defFun(f);
        block();
        symtab.endDefFun(); //结束函数定义
    }
}

/*
    <block>				->	lbrac<subprogram>rbrac
*/

void Parser::block() {
    if (!match(Tag::LBRACE))
        recovery(Tag::TYPE_FIRST || Tag::STATEMENT_FIRST || F(Tag::RBRACE), Tag::LBRACE_LOST,
                 Tag::LBRACE_WRONG);
    subprogram();
    if (!match(Tag::RBRACE))
        recovery(Tag::TYPE_FIRST || Tag::STATEMENT_FIRST ||
                     F(Tag::KW_EXTERN) _(Tag::KW_ELSE) _(Tag::KW_CASE) _(Tag::KW_DEFAULT),
                 Tag::RBRACE_LOST, Tag::RBRACE_WRONG);
}

/*
    <subprogram>	->	<localdef><subprogram>|<statements><subprogram>|^
*/
void Parser::subprogram() {
    if (Tag::TYPE_FIRST) { //局部变量
        localdef();
        subprogram();
    } else if (Tag::STATEMENT_FIRST) { //语句
        statement();
        subprogram();
    }
}

/*
    <localdef>		->	<type><defdata><deflist>
*/
void Parser::localdef() {
    Tag t = type();
    symtab.addVar(defdata(false, t));
    deflist(false, t);
}

/*
    <statement>		->	<altexpr>semicon
                                        |<whilestat>|<forstat>|<dowhilestat>
                                        |<ifstat>|<switchstat>
                                        |rsv_break semicon
                                        |rsv_continue semicon
                                        |rsv_return<altexpr>semicon
*/
void Parser::statement() {
    switch (look->tag) {
    case Tag::KW_WHILE:
        whilestat();
        break;
    case Tag::KW_FOR:
        forstat();
        break;
    case Tag::KW_DO:
        dowhilestat();
        break;
    case Tag::KW_IF:
        ifstat();
        break;
    case Tag::KW_SWITCH:
        switchstat();
        break;
    case Tag::KW_BREAK:
        ir.genBreak(); //产生break语句
        move();
        if (!match(Tag::SEMICON))
            recovery(Tag::TYPE_FIRST || Tag::STATEMENT_FIRST || F(Tag::RBRACE), Tag::SEMICON_LOST,
                     Tag::SEMICON_WRONG);
        break;
    case Tag::KW_CONTINUE:
        ir.genContinue(); //产生continue语句
        move();
        if (!match(Tag::SEMICON))
            recovery(Tag::TYPE_FIRST || Tag::STATEMENT_FIRST || F(Tag::RBRACE), Tag::SEMICON_LOST,
                     Tag::SEMICON_WRONG);
        break;
    case Tag::KW_RETURN:
        move();
        ir.genReturn(altexpr()); //产生return语句
        if (!match(Tag::SEMICON))
            recovery(Tag::TYPE_FIRST || Tag::STATEMENT_FIRST || F(Tag::RBRACE), Tag::SEMICON_LOST,
                     Tag::SEMICON_WRONG);
        break;
    default:
        altexpr();
        if (!match(Tag::SEMICON))
            recovery(Tag::TYPE_FIRST || Tag::STATEMENT_FIRST || F(Tag::RBRACE), Tag::SEMICON_LOST,
                     Tag::SEMICON_WRONG);
    }
}

/*
    <whilestat>		->	rsv_while lparen<altexpr>rparen<block>
    <block>				->	<block>|<statement>
*/
void Parser::whilestat() {
    symtab.enter();
    InterInst *_while, *_exit;      //标签
    ir.genWhileHead(_while, _exit); // while循环头部
    match(Tag::KW_WHILE);
    if (!match(Tag::LPAREN))
        recovery(Tag::EXPR_FIRST || F(Tag::RPAREN), Tag::LPAREN_LOST, Tag::LPAREN_WRONG);
    Var *cond = altexpr();
    ir.genWhileCond(cond, _exit); // while条件
    if (!match(Tag::RPAREN))
        recovery(F(Tag::LBRACE), Tag::RPAREN_LOST, Tag::RPAREN_WRONG);
    if (F(Tag::LBRACE))
        block();
    else
        statement();
    ir.genWhileTail(_while, _exit); // while尾部
    symtab.leave();
}

/*
    <dowhilestat> -> 	rsv_do <block> rsv_while lparen<altexpr>rparen semicon
    <block>				->	<block>|<statement>
*/
void Parser::dowhilestat() {
    symtab.enter();
    InterInst *_do, *_exit;        //标签
    ir.genDoWhileHead(_do, _exit); // do-while头部
    match(Tag::KW_DO);
    if (F(Tag::LBRACE))
        block();
    else
        statement();
    if (!match(Tag::KW_WHILE))
        recovery(F(Tag::LPAREN), Tag::WHILE_LOST, Tag::WHILE_WRONG);
    if (!match(Tag::LPAREN))
        recovery(Tag::EXPR_FIRST || F(Tag::RPAREN), Tag::LPAREN_LOST, Tag::LPAREN_WRONG);
    symtab.leave();
    Var *cond = altexpr();
    if (!match(Tag::RPAREN))
        recovery(F(Tag::SEMICON), Tag::RPAREN_LOST, Tag::RPAREN_WRONG);
    if (!match(Tag::SEMICON))
        recovery(Tag::TYPE_FIRST || Tag::STATEMENT_FIRST || F(Tag::RBRACE), Tag::SEMICON_LOST,
                 Tag::SEMICON_WRONG);
    ir.genDoWhileTail(cond, _do, _exit); // do-while尾部
}

/*
    <forstat> 		-> 	rsv_for lparen <forinit> semicon <altexpr> semicon
   <altexpr> rparen <block> <block>				->	<block>|<statement>
*/
void Parser::forstat() {
    symtab.enter();
    InterInst *_for, *_exit, *_step, *_block; //标签
    match(Tag::KW_FOR);
    if (!match(Tag::LPAREN))
        recovery(Tag::TYPE_FIRST || Tag::EXPR_FIRST || F(Tag::SEMICON), Tag::LPAREN_LOST,
                 Tag::LPAREN_WRONG);
    forinit();                                      //初始语句
    ir.genForHead(_for, _exit);                     // for循环头部
    Var *cond = altexpr();                          //循环条件
    ir.genForCondBegin(cond, _step, _block, _exit); // for循环条件开始部分
    if (!match(Tag::SEMICON))
        recovery(Tag::EXPR_FIRST, Tag::SEMICON_LOST, Tag::SEMICON_WRONG);
    altexpr();
    if (!match(Tag::RPAREN))
        recovery(F(Tag::LBRACE), Tag::RPAREN_LOST, Tag::RPAREN_WRONG);
    ir.genForCondEnd(_for, _block); // for循环条件结束部分
    if (F(Tag::LBRACE))
        block();
    else
        statement();
    ir.genForTail(_step, _exit); // for循环尾部
    symtab.leave();
}

/*
    <forinit> 		->  <localdef> | <altexpr>
*/
void Parser::forinit() {
    if (Tag::TYPE_FIRST)
        localdef();
    else {
        altexpr();
        if (!match(Tag::SEMICON))
            recovery(Tag::EXPR_FIRST, Tag::SEMICON_LOST, Tag::SEMICON_WRONG);
    }
}

/*
    <ifstat>			->	rsv_if lparen<expr>rparen<block><elsestat>
*/
void Parser::ifstat() {
    symtab.enter();
    InterInst *_else, *_exit; //标签
    match(Tag::KW_IF);
    if (!match(Tag::LPAREN))
        recovery(Tag::EXPR_FIRST, Tag::LPAREN_LOST, Tag::LPAREN_WRONG);
    Var *cond = expr();
    ir.genIfHead(cond, _else); // if头部
    if (!match(Tag::RPAREN))
        recovery(F(Tag::LBRACE), Tag::RPAREN_LOST, Tag::RPAREN_WRONG);
    if (F(Tag::LBRACE))
        block();
    else
        statement();
    symtab.leave();

    ir.genElseHead(_else, _exit); // else头部
    if (F(Tag::KW_ELSE)) {             //有else
        elsestat();
    }
    ir.genElseTail(_exit); // else尾部
    //不对if-else的else部分优化，妨碍冗余删除算法的效果
    // if(F(Tag::KW_ELSE)){//有else
    // 	ir.genElseHead(_else,_exit);//else头部
    // 	elsestat();
    // 	ir.genElseTail(_exit);//else尾部
    // }
    // else{//无else
    // 	ir.genIfTail(_else);
    // }
}

/*
    <elsestat>		-> 	rsv_else<block>|^
*/
void Parser::elsestat() {
    if (match(Tag::KW_ELSE)) {
        symtab.enter();
        if (F(Tag::LBRACE))
            block();
        else
            statement();
        symtab.leave();
    }
}

/*
    <switchstat>	-> 	rsv_switch lparen <expr> rparen lbrac <casestat> rbrac
*/
void Parser::switchstat() {
    symtab.enter();
    InterInst *_exit;        //标签
    ir.genSwitchHead(_exit); // switch头部
    match(Tag::KW_SWITCH);
    if (!match(Tag::LPAREN))
        recovery(Tag::EXPR_FIRST, Tag::LPAREN_LOST, Tag::LPAREN_WRONG);
    Var *cond = expr();
    if (cond->isRef())
        cond = ir.genAssign(cond); // switch(*p),switch(a[0])
    if (!match(Tag::RPAREN))
        recovery(F(Tag::LBRACE), Tag::RPAREN_LOST, Tag::RPAREN_WRONG);
    if (!match(Tag::LBRACE))
        recovery(F(Tag::KW_CASE) _(Tag::KW_DEFAULT), Tag::LBRACE_LOST, Tag::LBRACE_WRONG);
    casestat(cond);
    if (!match(Tag::RBRACE))
        recovery(Tag::TYPE_FIRST || Tag::STATEMENT_FIRST, Tag::RBRACE_LOST, Tag::RBRACE_WRONG);
    ir.genSwitchTail(_exit); // switch尾部
    symtab.leave();
}

/*
    <casestat> 		-> 	rsv_case <caselabel> colon <subprogram><casestat>
                                        |rsv_default colon <subprogram>
*/
void Parser::casestat(Var *cond) {
    if (match(Tag::KW_CASE)) {
        InterInst *_case_exit; //标签
        Var *lb = caselabel();
        ir.genCaseHead(cond, lb, _case_exit); // case头部
        if (!match(Tag::COLON))
            recovery(Tag::TYPE_FIRST || Tag::STATEMENT_FIRST, Tag::COLON_LOST, Tag::COLON_WRONG);
        symtab.enter();
        subprogram();
        symtab.leave();
        ir.genCaseTail(_case_exit); // case尾部
        casestat(cond);
    } else if (match(Tag::KW_DEFAULT)) { // default默认执行
        if (!match(Tag::COLON))
            recovery(Tag::TYPE_FIRST || Tag::STATEMENT_FIRST, Tag::COLON_LOST, Tag::COLON_WRONG);
        symtab.enter();
        subprogram();
        symtab.leave();
    }
}

/*
    <caselabel>		->	<literal>
*/
Var *Parser::caselabel() { return literal(); }

/*
    <altexpr>			->	<expr>|^
*/
Var *Parser::altexpr() {
    if (Tag::EXPR_FIRST)
        return expr();
    return Var::getVoid(); //返回特殊Tag::VOID变量
}

/*
    <expr> 				-> 	<assexpr>
*/
Var *Parser::expr() { return assexpr(); }

/*
    <assexpr>			->	<orexpr><asstail>
*/
Var *Parser::assexpr() {
    Var *lval = orexpr();
    return asstail(lval);
}

/*
    <asstail>			->	assign<assexpr>|^
*/
Var *Parser::asstail(Var *lval) {
    if (match(Tag::ASSIGN)) {
        Var *rval = assexpr();
        Var *result = ir.genTwoOp(lval, Tag::ASSIGN, rval);
        return asstail(result);
    }
    return lval;
}

/*
    <orexpr> 			-> 	<andexpr><ortail>
*/
Var *Parser::orexpr() {
    Var *lval = andexpr();
    return ortail(lval);
}

/*
    <ortail> 			-> 	or <andexpr> <ortail>|^
*/
Var *Parser::ortail(Var *lval) {
    if (match(Tag::OR)) {
        Var *rval = andexpr();
        Var *result = ir.genTwoOp(lval, Tag::OR, rval);
        return ortail(result);
    }
    return lval;
}

/*
    <andexpr> 		-> 	<cmpexpr><andtail>
*/
Var *Parser::andexpr() {
    Var *lval = cmpexpr();
    return andtail(lval);
}

/*
    <andtail> 		-> 	and <cmpexpr> <andtail>|^
*/
Var *Parser::andtail(Var *lval) {
    if (match(Tag::AND)) {
        Var *rval = cmpexpr();
        Var *result = ir.genTwoOp(lval, Tag::AND, rval);
        return andtail(result);
    }
    return lval;
}

/*
    <cmpexpr>			->	<aloexpr><cmptail>
*/
Var *Parser::cmpexpr() {
    Var *lval = aloexpr();
    return cmptail(lval);
}

/*
    <cmptail>			->	<cmps><aloexpr><cmptail>|^
*/
Var *Parser::cmptail(Var *lval) {
    if (F(Tag::GT) _(Tag::GE) _(Tag::LT) _(Tag::LE) _(Tag::EQU) _(Tag::NEQU)) {
        Tag opt = cmps();
        Var *rval = aloexpr();
        Var *result = ir.genTwoOp(lval, opt, rval);
        return cmptail(result);
    }
    return lval;
}

/*
    <cmps>				->	gt|ge|ls|le|equ|nequ
*/
Tag Parser::cmps() {
    Tag opt = look->tag;
    move();
    return opt;
}

/*
    <aloexpr>			->	<item><alotail>
*/
Var *Parser::aloexpr() {
    Var *lval = item();
    return alotail(lval);
}

/*
    <alotail>			->	<adds><item><alotail>|^
*/
Var *Parser::alotail(Var *lval) {
    if (F(Tag::ADD) _(Tag::SUB)) {
        Tag opt = adds();
        Var *rval = item();
        Var *result = ir.genTwoOp(lval, opt, rval); //双目运算
        return alotail(result);
    }
    return lval;
}

/*
    <adds>				->	add|sub
*/
Tag Parser::adds() {
    Tag opt = look->tag;
    move();
    return opt;
}

/*
    <item>				->	<factor><itemtail>
*/
Var *Parser::item() {
    Var *lval = factor();
    return itemtail(lval);
}

/*
    <itemtail>		->	<muls><factor><itemtail>|^
*/
Var *Parser::itemtail(Var *lval) {
    if (F(Tag::MUL) _(Tag::DIV) _(Tag::MOD)) {
        Tag opt = muls();
        Var *rval = factor();
        Var *result = ir.genTwoOp(lval, opt, rval); //双目运算
        return itemtail(result);
    }
    return lval;
}

/*
    <muls>				->	mul|div|mod
*/
Tag Parser::muls() {
    Tag opt = look->tag;
    move();
    return opt;
}

/*
    <factor> 			-> 	<lop><factor>|<val>
*/
Var *Parser::factor() {
    if (F(Tag::NOT) _(Tag::SUB) _(Tag::LEA) _(Tag::MUL) _(Tag::INC) _(Tag::DEC)) {
        Tag opt = lop();
        Var *v = factor();
        return ir.genOneOpLeft(opt, v); //单目左操作
    } else
        return val();
}

/*
    <lop> 				-> 	not|sub|lea|mul|incr|decr
*/
Tag Parser::lop() {
    Tag opt = look->tag;
    move();
    return opt;
}

/*
    <val>					->	<elem><rop>
*/
Var *Parser::val() {
    Var *v = elem();
    if (F(Tag::INC) _(Tag::DEC)) {
        Tag opt = rop();
        v = ir.genOneOpRight(v, opt);
    }
    return v;
}

/*
    <rop>					->	incr|decr|^
*/
Tag Parser::rop() {
    Tag opt = look->tag;
    move();
    return opt;
}

/*
    <elem>				->	ident<idexpr>|lparen<expr>rparen|<literal>
*/
Var *Parser::elem() {
    Var *v = Tag::NULL;
    if (F(Tag::ID)) { //变量，数组，函数调用
        string name = ((Id *)look)->name;
        move();
        v = idexpr(name);
    } else if (match(Tag::LPAREN)) { //括号表达式
        v = expr();
        if (!match(Tag::RPAREN)) {
            recovery(Tag::LVAL_OPR, Tag::RPAREN_LOST, Tag::RPAREN_WRONG);
        }
    } else //常量
        v = literal();
    return v;
}

/*
    <literal>			->	number|string|chara
*/
Var *Parser::literal() {
    Var *v = Tag::NULL;
    if (F(Tag::NUM) _(Tag::STR) _(Tag::CH)) {
        v = new Var(look);
        if (F(Tag::STR))
            symtab.addStr(v); //字符串常量记录
        else
            symtab.addVar(v); //其他常量也记录到符号表
        move();
    } else
        recovery(Tag::RVAL_OPR, Tag::LITERAL_LOST, Tag::LITERAL_WRONG);
    return v;
}

/*
    <idexpr>			->	lbrack <expr> rbrack|lparen<realarg>rparen|^
*/
Var *Parser::idexpr(string name) {
    Var *v = Tag::NULL;
    if (match(Tag::LBRACK)) {
        Var *index = expr();
        if (!match(Tag::RBRACK))
            recovery(Tag::LVAL_OPR, Tag::LBRACK_LOST, Tag::LBRACK_WRONG);
        Var *array = symtab.getVar(name); //获取数组
        v = ir.genArray(array, index);    //产生数组运算表达式
    } else if (match(Tag::LPAREN)) {
        vector<Var *> args;
        realarg(args);
        if (!match(Tag::RPAREN))
            recovery(Tag::RVAL_OPR, Tag::RPAREN_LOST, Tag::RPAREN_WRONG);
        Fun *function = symtab.getFun(name, args); //获取函数
        v = ir.genCall(function, args);            //产生函数调用代码
    } else
        v = symtab.getVar(name); //获取变量
    return v;
}

/*
    <realarg>			->	<arg><arglist>|^
*/
void Parser::realarg(vector<Var *> &args) {
    if (Tag::EXPR_FIRST) {
        args.push_back(arg()); //压入参数
        arglist(args);
    }
}

/*
    <arglist>			->	comma<arg><arglist>|^
*/
void Parser::arglist(vector<Var *> &args) {
    if (match(Tag::COMMA)) {
        args.push_back(arg());
        arglist(args);
    }
}

/*
    <arg> 				-> 	<expr>
*/
Var *Parser::arg() {
    //添加一个实际参数
    return expr();
}
