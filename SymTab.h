#ifndef SYMTAB_H_INCLUDED
#define SYMTAB_H_INCLUDED
#include <bits/stdc++.h>
#include "common.h"
using namespace std;
class Var;
class Fun;
class Var
{
    int level;//�������ڵĲ�����
    bool isConst;//�Ƿ��ǳ���
    Tag type;//��������
    string name;//��������
    bool isArray;//�Ƿ�������
    int ArraySize;//�����С
    bool inited;//�Ƿ��ʼ��
    //������ֵ
    int intVal;
    char charVal;
    string strVal;
    int Size;//������С
};
class Fun
{
    Tag type;//��������
    string name;//��������
    vector<Var> paralist;//�βα�

};
class SymTab//���ű�
{
    unordered_map<string,Var> varTab;
    unordered_map<string,Var> strTab;
};
class SymTab_Overall:public SymTab//ȫ�ַ��ű�
{
    unordered_map<string,Fun> funTab;
};

#endif // SYMTAB_H_INCLUDED
