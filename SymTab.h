#ifndef SYMTAB_H_INCLUDED
#define SYMTAB_H_INCLUDED
#include <bits/stdc++.h>
#include "common.h"
#include "genir.h"
#include "intercode.h"
using namespace std;
class Var;
class Fun;
class Var
{
    bool isConst;//是否是常量
    Tag type;//变量类型
    string name;//变量名称
    bool isArray;//是否是数组
    int arraySize;//数组大小
    bool inited;//是否初始化
    //变量的值
    int intVal;
    double floatVal;
    char charVal;
    string strVal;
    int size;//变量大小

public:
    Var();
    Var(Tag t,bool ptr,string name,Var*init=NULL);//变量
    Var(Tag t,string name,int len);//数组
    Var(int val);//整数变量
    Var(vector<int>&sp,Tag t,bool ptr);//临时变量
    Var(vector<int>&sp,Var*v);//拷贝变量
    Var();//void变量
    void value();
    string getName();
    void setType(Tag t);//设置类型
    void setName(string n);//设置名字
    void setArray(int len);//设定数组
    //寄存器分配信息
    bool unInit();//是否初始化
    bool notConst();//是否是常量
    int getVal();//获取常量值
    bool istemp;//是否是临时变量
    bool live;//记录变量的活跃性
    bool isLiteral();//是基本类型常量（字符串除外），没有存储在符号表，需要单独内存管理

};
class Fun
{
    Tag type;//返回类型
    string name;//函数名字
    vector<Var*> paralist;//形参表
public:
    string getName();

};
class SymTab//符号表
{
    unordered_map<string,Var*> varTab;
    unordered_map<string,Var*> strTab;//字符串常量
    unordered_map<string,Fun*> funTab;//函数表
    GenIR* ir;//四元式生成器
    InterCode* intercode;//中间代码
public:
    SymTab();
    ~SymTab();
    //变量管理
    void addVar(Var* v);//添加一个变量
    void addStr(Var* v);//添加一个字符串常量
    Var* getVar(string name);//获取一个变量
    //函数管理
    void decFun(Fun*fun);//声明一个函数
    void defFun(Fun*fun);//定义一个函数
    void endDefFun();//结束定义一个函数
    Fun* getFun(string name,vector<Var*>& args);//根据调用类型，获取一个函数
    void addInst(InterInst*inst);//添加一条中间代码
    //外部调用接口
    void setIr(GenIR*ir);//设置中间代码生成器
    void toString();//输出信息
    void printInterCode();//输出中间指令
    void optimize();//执行优化操作
    void printOptCode();//输出优化后的中间指令
    void genData(FILE*file);//输出数据
    void genAsm(char*fileName);//输出汇编文件
};


#endif // SYMTAB_H_INCLUDED