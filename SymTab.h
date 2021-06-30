#ifndef SYMTAB_H_INCLUDED
#define SYMTAB_H_INCLUDED
#include <bits/stdc++.h>
#include "common.h"
using namespace std;
class Var;
class Fun;
class Var
{
    int level;//变量所在的层数。
    bool isConst;//是否是常量
    Tag type;//变量类型
    string name;//变量名称
    bool isArray;//是否是数组
    int ArraySize;//数组大小
    bool inited;//是否初始化
    //变量的值
    int intVal;
    char charVal;
    string strVal;
    int Size;//变量大小
};
class Fun
{
    Tag type;//返回类型
    string name;//函数名字
    vector<Var> paralist;//形参表

};
class SymTab//符号表
{
    unordered_map<string,Var> varTab;
    unordered_map<string,Var> strTab;
};
class SymTab_Overall:public SymTab//全局符号表
{
    unordered_map<string,Fun> funTab;
};

#endif // SYMTAB_H_INCLUDED
