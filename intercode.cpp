#include "intercode.h"


/*******************************************************************************
                                   ��Ԫʽ
*******************************************************************************/

/*
	��ʼ��
*/
void InterInst::init()
{
	op=OP_NOP;
	this->result=NULL;
	this->target=NULL;
	this->arg1=NULL;
	this->fun=NULL;
	this->arg2=NULL;
	first=false;
	//isDead=false;
}

/*
	һ������ָ��
*/
InterInst::InterInst (Operator op,Var *rs,Var *arg1,Var *arg2)
{
	init();
	this->op=op;
	this->result=rs;
	this->arg1=arg1;
	this->arg2=arg2;
}

/*
	��������ָ��
*/
InterInst::InterInst (Operator op,Fun *fun,Var *rs)
{
	init();
	this->op=op;
	this->result=rs;
	this->fun=fun;
	this->arg2=NULL;
}

/*
	������ջָ��
*/
InterInst::InterInst (Operator op,Var *arg1)
{
	init();
	this->op=op;
	this->result=NULL;
	this->arg1=arg1;
	this->arg2=NULL;
}

/*
	����Ψһ���
*/
InterInst::InterInst ()
{
	init();
	//label=GenIR::genLb();
}

/*
	������תָ��
*/
InterInst::InterInst (Operator op,InterInst *tar,Var *arg1,Var *arg2)
{
	init();
	this->op=op;
	this->target=tar;
	this->arg1=arg1;
	this->arg2=arg2;
}

/*
	�滻���ʽָ����Ϣ
*/
void InterInst::replace(Operator op,Var *rs,Var *arg1,Var *arg2)
{
	this->op=op;
	this->result=rs;
	this->arg1=arg1;
	this->arg2=arg2;
}

/*
	�滻��תָ����Ϣ��������ת�Ż�
*/
void InterInst::replace(Operator op,InterInst *tar,Var *arg1,Var *arg2)
{
	this->op=op;
	this->target=tar;
	this->arg1=arg1;
	this->arg2=arg2;
}

/*
	�滻�����������ڽ�CALLת��ΪPROC
*/
void InterInst::callToProc()
{
	this->result=NULL;//�������ֵ
	this->op=OP_PROC;
}


/*
	�������ڴ�
*/
InterInst::~InterInst()
{
	//if(arg1&&arg1->isLiteral())delete arg1;
	//if(arg2&&arg2->isLiteral())delete arg2;
}

/*
	���ָ����Ϣ
*/
void InterInst::toString()
{
	if(label!=""){
		printf("%s:\n",label.c_str());
		return;
	}
	switch(op)
	{
		//case OP_NOP:printf("nop");break;
		case OP_DEC:printf("dec ");arg1->value();break;
		case OP_ENTRY:printf("entry");break;
		case OP_EXIT:printf("exit");break;
		case OP_AS:result->value();printf(" = ");arg1->value();break;
		case OP_ADD:result->value();printf(" = ");arg1->value();printf(" + ");arg2->value();break;
		case OP_SUB:result->value();printf(" = ");arg1->value();printf(" - ");arg2->value();break;
		case OP_MUL:result->value();printf(" = ");arg1->value();printf(" * ");arg2->value();break;
		case OP_DIV:result->value();printf(" = ");arg1->value();printf(" / ");arg2->value();break;
		case OP_MOD:result->value();printf(" = ");arg1->value();printf(" %% ");arg2->value();break;
		case OP_NEG:result->value();printf(" = ");printf("-");arg1->value();break;
		case OP_GT:result->value();printf(" = ");arg1->value();printf(" > ");arg2->value();break;
		case OP_GE:result->value();printf(" = ");arg1->value();printf(" >= ");arg2->value();break;
		case OP_LT:result->value();printf(" = ");arg1->value();printf(" < ");arg2->value();break;
		case OP_LE:result->value();printf(" = ");arg1->value();printf(" <= ");arg2->value();break;
		case OP_EQU:result->value();printf(" = ");arg1->value();printf(" == ");arg2->value();break;
		case OP_NE:result->value();printf(" = ");arg1->value();printf(" != ");arg2->value();break;
		case OP_NOT:result->value();printf(" = ");printf("!");arg1->value();break;
		case OP_AND:result->value();printf(" = ");arg1->value();printf(" && ");arg2->value();break;
		case OP_OR:result->value();printf(" = ");arg1->value();printf(" || ");arg2->value();break;
		case OP_JMP:printf("goto %s",target->label.c_str());break;
		case OP_JT:printf("if( ");arg1->value();printf(" )goto %s",target->label.c_str());break;
		case OP_JF:printf("if( !");arg1->value();printf(" )goto %s",target->label.c_str());break;
		// case OP_JG:printf("if( ");arg1->value();printf(" > ");arg2->value();printf(" )goto %s",
		// 	target->label.c_str());break;
		// case OP_JGE:printf("if( ");arg1->value();printf(" >= ");arg2->value();printf(" )goto %s",
		// 	target->label.c_str());break;
		// case OP_JL:printf("if( ");arg1->value();printf(" < ");arg2->value();printf(" )goto %s",
		// 	target->label.c_str());break;
		// case OP_JLE:printf("if( ");arg1->value();printf(" <= ");arg2->value();printf(" )goto %s",
		// 	target->label.c_str());break;
		// case OP_JE:printf("if( ");arg1->value();printf(" == ");arg2->value();printf(" )goto %s",
		// 	target->label.c_str());break;
		case OP_JNE:printf("if( ");arg1->value();printf(" != ");arg2->value();printf(" )goto %s",
			target->label.c_str());break;
		case OP_ARG:printf("arg ");arg1->value();break;
		case OP_PROC:printf("%s()",fun->getName().c_str());break;
		case OP_CALL:result->value();printf(" = %s()",fun->getName().c_str());break;
		case OP_RET:printf("return goto %s",target->label.c_str());break;
		case OP_RETV:printf("return ");arg1->value();printf(" goto %s",target->label.c_str());break;
		case OP_LEA:result->value();printf(" = ");printf("&");arg1->value();break;
		case OP_SET:printf("*");arg1->value();printf(" = ");result->value();break;
		case OP_GET:result->value();printf(" = ");printf("*");arg1->value();break;
	}
	printf("\n");	
}

/*
	�Ƿ�����ת��ָ��JT,JF,Jcond
*/
bool InterInst::isJcond()
{
	return op>=OP_JT&&op<=OP_JNE;
}

/*
	�Ƿ�ֱ��ת��ָ��JMP,return
*/
bool InterInst::isJmp()
{
	return op==OP_JMP||op==OP_RET||op==OP_RETV;
}

/*
	�����ָ��
*/
void InterInst::setFirst()
{
	first=true;
}

/*
	����ָ��
*/
bool InterInst::isFirst()
{
	return first;
}

/*
	�Ƿ��Ǳ�ǩ
*/
bool InterInst::isLb()
{
	return label!="";
}

/*
	�ǻ������ͱ��ʽ����,���Զ�ָ��ȡֵ
*/
bool InterInst::isExpr()
{
	return (op>=OP_AS&&op<=OP_OR||op==OP_GET);//&&result->isBase();
}

/*
	��ȷ��������Ӱ�������(ָ�븳ֵ����������)
*/
bool InterInst::unknown()
{
	return op==OP_SET||op==OP_PROC||op==OP_CALL;
}

/*
	��ȡ������
*/
Operator InterInst::getOp()
{
	return op;
}


/*
	��ȡ��תָ���Ŀ��ָ��
*/
InterInst* InterInst::getTarget()
{
	return target;
}

/*
	��ȡ����ֵ
*/
Var*InterInst::getResult()
{
	return result;
}

/*
	���õ�һ������
*/
void InterInst::setArg1(Var*arg1)
{
	this->arg1=arg1;
}



/*
	�Ƿ�������
*/
bool InterInst::isDec()
{
	return op==OP_DEC;
}

/*
	��ȡ��һ������
*/
Var* InterInst::getArg1()
{
	return arg1;
}

/*
	��ȡ�ڶ�������
*/
Var* InterInst::getArg2()
{
	return arg2;
}

/*
	��ȡ��ǩ
*/
string InterInst::getLabel()
{
	return label;
}

/*
	��ȡ��������
*/
Fun* InterInst::getFun()
{
	return fun;
}



/*******************************************************************************
                                   �м����
*******************************************************************************/

/*
	����м����
*/
void InterCode::addInst(InterInst*inst)
{
	code.push_back(inst);
}

/*
	���ָ����Ϣ
*/
void InterCode::toString()
{
	for(int i=0;i<code.size();i++)
	{
		code[i]->toString();
	}
}

/*
	����ڴ�
*/
InterCode::~InterCode()
{
	for(int i=0;i<code.size();i++)
	{
		delete code[i];
	}
}

/*
	��ʶ����ָ�
*/
void InterCode::markFirst()
{
	unsigned int len=code.size();//ָ�����������Ϊ2
	//��ʶEntry��Exit
	code[0]->setFirst();
	code[len-1]->setFirst();
	//��ʶ��һ��ʵ��ָ�����еĻ�
	if(len>2)code[1]->setFirst();
	//��ʶ��1��ʵ��ָ�������2��ָ��
	for(unsigned int i=1;i<len-1;++i){
		if(code[i]->isJmp()||code[i]->isJcond()){//��ֱ��/��������תָ��Ŀ��ͽ���ָ�����ָ��
			code[i]->getTarget()->setFirst();
			code[i+1]->setFirst();
		}
	}
}

/*
	��ȡ�м��������
*/
vector<InterInst*>& InterCode::getCode()
{
	return code;
}


