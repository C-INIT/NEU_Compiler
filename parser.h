#pragma once
#include"token.h"
#include "SymTab.h"
class Parser
{
	//文法开始
	void program();
	//void segment();
	void def();
	Tag type();
	
	//声明与定义
	void var_data();
	void def_list();
	void def_info();
	// Var* defdata(bool ext,Tag t);
	// void deflist(bool ext,Tag t);
	// Var* varrdef(bool ext,Tag t,bool ptr,string name);
	// Var* init(bool ext,Tag t,bool ptr,string name);
	// void def(bool ext,Tag t);
	// void id_tail(bool ext,Tag t,bool ptr,string name);
	
	//函数
	Var* paradata_tail(Tag t,string name);
	Var* paradata(Tag t);
	void para(vector<Var*>&list);
	void paralist(vector<Var*>&list);
	void fun_tail(Fun*f);
	void block();
	void subprogram();
	void localdef();
	
	//语句
	void statement();
	void whilestat();
	void dowhilestat();
	void forstat();
	void forinit();
	void ifstat();
	void elsestat();
	void switchstat();
	void casestat(Var*cond);
	Var* caselabel();
	
	//表达式
	Var* alt_exp();
	Var* exp();
	// Var* ass_exp();
	// Var* ass_tail(Var*lval);
	Var* assign();
	Var* or_exp();
	Var* or_tail(Var*lval);
	Var* and_exp();
	Var* and_tail(Var*lval);
	Var* cmp_exp();
	Var* cmp_tail(Var*lval);
	Tag cmps();
	Var* alo_exp();
	Var* alo_tail(Var*lval);
	Tag adds();
	Var* item();
	Var* item_tail(Var*lval);
	Tag muls();
	Var* factor();
	Tag lop();
	Var* val();
	Tag rop();
	Var* elem();
	Var* literal();
	Var* id_exp(string name);
	void realarg(vector<Var*> &args);
	void arglist(vector<Var*> &args);
	Var* arg();
	
	//词法分析
	Lexer &lexer;//词法分析器
	Token look;//超前查看的字符
	
	//符号表
	SymTab &symtab;
	
	//中间代码生成器
	GenIR &ir;
	
	//语法分析与错误修复
	void nextw(){
        
    }
	bool match(Tag t){
        return look.tag == t;
    }
	void recovery(bool cond,SynError lost,SynError wrong);//错误修复

public:
	
	//构造与初始化
	Parser(Lexer&lex,SymTab&tab,GenIR&inter);
	
	//外部调用接口
	void analyse();//语法分析主程序
};