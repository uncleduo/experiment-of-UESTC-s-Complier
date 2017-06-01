#include<iostream>
#include<Windows.h>
#include<string>
#include<fstream>
#include<vector>
#include<stdlib.h>

#define WordLen 16 //字符串长度

using namespace std;

struct code_val {
	char val[WordLen + 1] = "";	//字符串
	int code;				//编码
};
struct var_table
{
	string word;
	string now_procedure;
	bool is_formal;
	string type = "integer";
	int procedure_level;
	int var_pos;
};
struct procedure_table
{
	string now_procedure;
	string type = "integer";
	int procedure_level;
	int begin_var = -1;
	int end_var = -1;
	int procedure_pos;
};
//保留字
int begin_code = 1;
int end_code = 2;
int integer_code = 3;
int if_code = 4;
int then_code = 5;
int else_code = 6;
int function_code = 7;
int read_code = 8;
int write_code = 9;
int identifier_code = 10;
int constant_code = 11;
int FEOLN = 24;
int FEOF = 25;
//int n = atoi(str);
void main_procedure();
void branch_procedure();
void declare_statement_table();
void declare_statement_table_Plus();
void declare_statement();
void var_declare();
void var(bool define_or_use, bool is_formal);
string identifier();
void func_declare();
void parameter();
void func_body();
void exec_statement_table();
void exec_statement_table_Plus();
void exec_statement();
void read_statement();
void write_statement();
void assign_statement();
void math_expression();
void math_expression_Plus();
void item();
void item_Plus();
void factor();
void func_call();
void constant();
void unsigned_integer();
void condition_statement();
void condition_expression();
void relation_operator();

vector <code_val> WordStream;
vector <var_table> var_Table;
int var_count;
vector <procedure_table> procedure_Table;
int procedure_count;
code_val token;
code_val forward_token;
int sp = 0;
int lineNum = 1;
fstream dysFile("test.dys", ios::out);
fstream errFile("test.err",ios_base::out);//在词法错误日志的基础上写


bool var_define = 0;//变量是否定义
bool var_use = 1;//变量是否使用
bool formal_parameter = 1;//是否是形参
int procedure_level = 0;//过程深度
string now_Procedure;//当前过程名

void Read()
{
	ifstream inf;
	inf.open("test.dyd");
	while (!inf.eof())
	{
		code_val temp;
		inf >> temp.val >> temp.code;
		WordStream.push_back(temp);
	}
}
void Write()
{
	fstream varFile("test.var", ios::out);
	fstream proFile("test.pro", ios::out);
	for (int i = 0; i < var_Table.size(); i++)
	{
		varFile << var_Table[i].word <<"	" << var_Table[i].now_procedure << "	" << var_Table[i].is_formal << "	" << var_Table[i].type;
		varFile << "	" << var_Table[i].procedure_level << "	" << var_Table[i].var_pos << endl;
	}
	for (int j = 0; j < procedure_Table.size(); j++)
	{
		proFile << procedure_Table[j].now_procedure << "	" << procedure_Table[j].type << "	" << procedure_Table[j].procedure_level;
		proFile << "	" << procedure_Table[j].begin_var << "	" << procedure_Table[j].end_var << endl;
	}
	varFile.close();
	proFile.close();
}


void err_print(char* err_info)
{
	cout << err_info << " 行数: " << lineNum << endl;;
	errFile << err_info << " 行数: " << lineNum << endl;
}
void now()
{
	while (WordStream[sp].code == FEOLN)
	{
		lineNum++;
		sp++;
	}
	strcpy_s(token.val, WordStream[sp].val);
	token.code = WordStream[sp].code;
}
void now_more_one()
{
	int temp = 1;
	while (WordStream[sp+temp].code == FEOLN)
	{
		temp++;
	}
	strcpy_s(forward_token.val, WordStream[sp+temp].val);
	forward_token.code = WordStream[sp+temp].code;
}
void advanced()
{
	sp++;
	now();
}
void main_procedure()
{
//<程序>→<分程序>
	now();
	branch_procedure();
	return;
}

void branch_procedure()
{
// <分程序>→begin <说明语句表>；<执行语句表> end
	if (token.code == begin_code)
	{
		advanced();
		declare_statement_table();
		if (token.code == 23)
		{
			advanced();
			exec_statement_table();
			if (token.code == end_code)
				advanced();
			else
				err_print("分程序错误，是否缺少 end");
		}
		else
			err_print("分程序错误，是否缺少 ;");
	}
	else
		err_print("分程序错误，是否缺少 begin");
	return;
}

void declare_statement_table()
// <说明语句表>→<说明语句>│<说明语句表> ；<说明语句>
	// 需要改写为
// <说明语句表>→<说明语句><说明语句表A>
// <说明语句表A>→;<说明语句><说明语句表A>│空
{
	declare_statement();
	declare_statement_table_Plus();
	return;
}
void declare_statement_table_Plus()
//<说明语句表A>→;<说明语句><说明语句表A>│空
{
	now_more_one();
	if (token.code == 23 && forward_token.code == integer_code)
	{
		advanced();
		declare_statement();
		declare_statement_table_Plus();
	}
	else
		return;
}
void declare_statement()
// <说明语句>→<变量说明>│<函数说明>
{
	if (token.code == integer_code)
	{
		now_more_one();
		if (forward_token.code == 7)
			func_declare();
		else
			var_declare();
	}
	else
		err_print("说明语句出错,是否缺少integer");
}
void var_declare()
// <变量说明>→integer <变量>
{
	if (token.code == 3)
	{
		advanced();
		var(var_define,false);
	}
	else
		err_print("变量说明出错，是否缺少integer");
}
void var(bool define_or_use, bool is_formal)
{
	bool flag;
	string word;
	bool use_word;
	//<变量>→<标识符>
	if (define_or_use == var_define)
	{
		word = identifier();
		flag = true;
		for (int i = 0; i < var_Table.size(); i++)
			if (word == var_Table[i].word)
				flag = false;
		if (flag)
		{
			var_table temp;
			temp.word = word;
			temp.now_procedure = now_Procedure;
			temp.is_formal = is_formal;
			temp.procedure_level = procedure_level;
			temp.var_pos = var_count;
			var_Table.push_back(temp);
			var_count += 1;
		}
	}
	else
		if (define_or_use == var_use)
		{
			word = identifier();
			use_word = false;
				for (int i = 0; i < var_Table.size();i++)
				{
					if (var_Table[i].word == word)
						use_word = true;
				}
				for (int i = 0; i < procedure_Table.size();i++)
				{
					if (procedure_Table[i].now_procedure == word)
						use_word = true;
				}
				if (!use_word)
					err_print("符号无定义");
		}
	for (int i = 0; i < procedure_Table.size(); i++)//修改
	{
		if (procedure_Table[i].now_procedure == now_Procedure)
		{
			if (procedure_Table[i].begin_var == -1)
				procedure_Table[i].begin_var = lineNum;
			if (procedure_Table[i].end_var < lineNum)
				procedure_Table[i].end_var = lineNum;
		}
	}
}
string identifier()
// <标识符>→<字母>│<标识符><字母>│ <标识符><数字>
{
	if (token.code == identifier_code)
	{
		string temp_word = token.val;
		advanced();
		return temp_word;
	}
	else
	{
		string result = "NUL";//有问题
		err_print("标识符出错");
		return result;
	}
		
}
void func_declare()
{
	//<函数说明>→integer function <标识符>（<参数>）；<函数体>
	procedure_level += 1;
	bool flag;
	string  last_Procedure = now_Procedure;//保存上一个过程名
	if (token.code == integer_code)
	{
		advanced();
		if (token.code == function_code)
		{
			advanced();
			now_Procedure = identifier();
			flag = true;
			for (int i = 0; i < procedure_Table.size(); i++)
			{
				if (now_Procedure == procedure_Table[i].now_procedure)
					flag = false;
			}
			if (flag)
			{
				procedure_table temp;
				temp.now_procedure = now_Procedure;
				temp.procedure_level = procedure_level;
				temp.procedure_pos = procedure_count;
				//proFile << now_Procedure << "integer" << procedure_level << -1 << -1 << endl;
				procedure_Table.push_back(temp);
			}
			if (token.code == 21)
			{
				advanced();
				parameter();
				if (token.code == 22)
				{
					advanced();
					if (token.code == 23)
					{
						advanced();
						func_body();
					}
					else
						err_print("函数说明出错，缺少;");
				}
				else
					err_print("函数说明出错，缺少)");
			}
			else
				err_print("函数说明出错，缺少(");
		}
		else
			err_print("函数说明出错，缺少function");
	}
	else
	{
		err_print("函数说明出错，缺少integer");
		now_Procedure = last_Procedure;
		procedure_level -= 1;
	}
}

void parameter()
// <参数>→<变量>
{
	var(var_define, formal_parameter);
	return;
}
void func_body()
// <函数体>→begin <说明语句表>；<执行语句表> end
{
	if (token.code == begin_code)
	{
		advanced();
		declare_statement_table();
		if (token.code == 23)
		{
			advanced();
			exec_statement_table();
			if (token.code == end_code)
				advanced();
			else
				err_print("函数体出错，缺少end");
		}
		else
			err_print("函数体出错，缺少;");
	}
	else
		err_print("函数体错误，缺少begin");
}

void exec_statement_table()
// 左递归：<执行语句表>→<执行语句>│<执行语句表>；<执行语句>
// 需要改写为
// <执行语句表>→<执行语句><执行语句表A>
// <执行语句表A>→;<执行语句><执行语句表A>│空
{
	exec_statement();
	exec_statement_table_Plus();
	return;
}
void exec_statement_table_Plus()
// <执行语句表A>→;<执行语句><执行语句表A>│空
{
	int now_code = token.code;
	if ( now_code == 23)
	{
		now_more_one();
		if (forward_token.code == read_code || forward_token.code == write_code || forward_token.code == if_code || forward_token.code == identifier_code)
		{
			advanced();
			exec_statement();
			exec_statement_table_Plus();
		}
	}
	else
		return;
}
void exec_statement()
// <执行语句>→<读语句>│<写语句>│<赋值语句>│<条件语句>
{
	int exec_state = token.code;
	if (exec_state == read_code)
		read_statement();
	else 
		if (exec_state == write_code)
		write_statement();
	else 
		if (exec_state == if_code)
		condition_statement();
			else 
				if (exec_state == identifier_code)
			assign_statement();
	else
		err_print("执行语句出错，不知道该走哪里");
}
void read_statement()
// <读语句>→read(<变量>)
{
	if (token.code == 8)
	{
		advanced();
		if (token.code == 21)
		{
			advanced();
			var(var_use,false);
			if (token.code == 22)
				advanced();
			else
				err_print("读语句出错");
		}
		else
			err_print("读语句出错");
	}
	else
		err_print("读语句出错");
}

void write_statement()
//<写语句>→write(<变量>)
{
	if (token.code == 9)
	{
		advanced();
		if (token.code == 21)
		{
			advanced();
			var(var_use,false);
			if (token.code == 22)
				advanced();
			else
				err_print("写语句出错");
		}
		else
			err_print("写语句出错");
	}
	else
		err_print("写语句出错");
}
void assign_statement()
// <赋值语句>→<变量>:=<算术表达式>
{
	var(var_use,false);
	if (token.code == 20)
	{
		advanced();
		math_expression();
	}
	else
		err_print("赋值语句出错");
}
void math_expression()
// 左递归：<算术表达式>→<算术表达式>-<项>│<项>
// 改写：<算术表达式>→<项><算术表达式A>
//<算术表达式A>→-<项><算术表达式A>|空
{
	item();
	math_expression_Plus();
}
void math_expression_Plus()
//<算术表达式A>→-<项><算术表达式A>|空
{
	if (token.code == 18)
	{
		advanced();
		item();
		math_expression_Plus();
	}
	else
		return;
}
void item()
// 左递归：<项>→<项>*<因子>│<因子>
// 改写<项>→<因子><项A>
// <项A>→*<因子><项A>│空
{
	factor();
	item_Plus();
}
void item_Plus()
// <项A>→*<因子><项A>│空
{
	if (token.code == 19)
	{
		advanced();
		factor();
		item_Plus();
	}
	else
		return;
}
void factor()
// <因子>→<变量>│<常数>│<函数调用>
{
	now_more_one();
	if (token.code == 11)
		constant();
	else
		if (forward_token.code == 21)
			func_call();
		else
			var(var_use,false);
}
void func_call()
// <函数调用>→<标识符>(<算数表达式>)
{
	identifier();
	if (token.code == 21)
	{
		advanced();
		math_expression();
		if (token.code == 22)
			advanced();
		else
			err_print("函数调用出错");
	}
	else
		err_print("函数调用出错");
}
void constant()
// <常数>→<无符号整数>
{
	unsigned_integer();
}
void unsigned_integer()
{
	if (token.code == 11)
		advanced();
	else
		err_print("常数调用出错，遇到非数字");
}
void condition_statement()
// <条件语句>→if<条件表达式>then<执行语句>else <执行语句>
{
	if (token.code == 4)
	{
		advanced();
		condition_expression();
		if (token.code == 5)
		{
			advanced();
			exec_statement();
			if (token.code == 6)
			{
				advanced();
				exec_statement();
			}
			else
				err_print("条件语句出错");
		}
		else
			err_print("条件语句出错");
	}
	else
		err_print("条件语句出错");
}
void condition_expression()
// <条件表达式>→<算术表达式><关系运算符><算术表达式>
{
	math_expression();
	relation_operator();
	math_expression();
}
void relation_operator()
// <关系运算符> →<│<=│>│>=│=│<>
{
	int now_code = token.code;
	if ( now_code == 15 || now_code == 14 || now_code == 17 || now_code == 16 || now_code == 12 || now_code == 13)
		advanced();
	else
		err_print("关系运算符出错");
}
void main()
{
	Read();
	main_procedure();
	Write();
	system("pause");
}
