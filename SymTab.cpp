//
// Created by 64803 on 2021/7/6.
//
#include "SymTab.h"
Var::Var() {}
void Var::value()
{
    switch(type)
    {
        case INT_VAL:{printf("%d",intVal);break;}
        case FLOAT_VAL:{printf("%f",floatVal);break;}
        case BOOL_VAL:{printf("%d",intVal);break;}
        case CH_VAL:{printf("%c",charVal);break;}
        case STRING_VAL:{printf("%s",strVal.c_str());break;}
        default:{}

    }
}
string Var::getName(){return name;}
string Fun::getName() {return name;}
SymTab::SymTab()
{
    ir=NULL;
    intercode=new InterCode;
}
void SymTab::setIr(GenIR*ir)
{
    this->ir=ir;
}
void SymTab::addVar(Var* var)
{
    if(varTab.find(var->getName())==varTab.end()){ //没有该名字的变量
        varTab[var->getName()]=var;//添加变量
    }
    else{
        return ;//存在同名变量，返回。
    }
    if(ir){
        int flag=ir->genVarInit(var);//产生变量初始化语句,常量返回0
    }
    return ;
}
//添加字符串常量
void SymTab::addStr(Var* v)
{
    strTab[v->getName()]=v;
}
//获取一个变量
Var* SymTab::getVar(string name)
{
    Var*select=NULL;//最佳选择
    if(varTab.find(name)!=varTab.end()){
        select=varTab[name];
    }
    return select;
}

	//添加一条中间代码
void SymTab::addInst(InterInst*inst)
{
    intercode->addInst(inst);
    return ;
}
void SymTab::optimize()
{
    //for(int i=0;i<funList.size();i++){
    //    funTab[funList[i]]->optimize(this);
    //}
}

/*
	输出符号表信息
*/
/*
void SymTab::toString()
{
    printf("----------变量表----------\n");
    for(int i=0;i<varList.size();i++){
        string varName=varList[i];
        vector<Var*>&list=*varTab[varName];
        printf("%s:\n",varName.c_str());
        for(int j=0;j<list.size();j++){
            printf("\t");
            list[j]->toString();
            printf("\n");
        }
    }
    printf("----------串表-----------\n");
    hash_map<string, Var*, string_hash>::iterator strIt,strEnd=strTab.end();
    for(strIt=strTab.begin();strIt!=strEnd;++strIt)
        printf("%s=%s\n",strIt->second->getName().c_str(),strIt->second->getStrVal().c_str());
    printf("----------函数表----------\n");
    for(int i=0;i<funList.size();i++){
        funTab[funList[i]]->toString();
    }
}
*/
/*
	输出中间代码
*/
/*
void SymTab::printInterCode()
{
    for(int i=0;i<funList.size();i++){
        funTab[funList[i]]->printInterCode();
    }
}
*/
/*
	输出优化的中间代码
*/
/*
void SymTab::printOptCode()
{
    for(int i=0;i<funList.size();i++){
        funTab[funList[i]]->printOptCode();
    }
}*/