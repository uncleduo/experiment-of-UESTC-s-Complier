#include<iostream>
#include<fstream>
#include<Windows.h>
#include<string>
#include<stdlib.h>
#include<vector>
#include<algorithm>

#define BufSize 10000
#define WordLen 16
#define table_len 11

using namespace std;
struct code_val
{
	char code[3];
	char val[WordLen + 1];
};
char *table[table_len] = {"NUL","begin","end","integer","if","then","else","function","read","write","EOLN"};
char *table_code[table_len] = { "0","1","2","3","4","5","6","7","8","9","24" };
vector<string> symbol_table;
int LineNum = 0;
//enum table { begin,end,integer,if,then,else,function,read,write };
void pretreatment(char filename[],char Buf[])
{
	ifstream cinf(filename, ios::in);
	char c0 = '$', c1;//c0
	bool in_comment = false;
	cout << "Source Program" << endl;
	int i = 0;//Buf下标
	while (cinf.read(&c1, sizeof(char)))
	{
		cout << c1;
		switch (in_comment)
		{
		case false:
			if (c0 == '/' && c1 == '*')
				in_comment = true, i++;
			else
			{
				/*if (c0 == '\\' && c1 == '\n')
					i--;*/
				if (c1 == '\t')
				{
					c1 = ' ';
				}
				/*if (c1 == '\n')
				{
					Buf[i] = 'E';
					Buf[i + 1] = 'O';
					Buf[i + 2] = 'L';
					Buf[i + 3] = 'N';
					i += 4;
				}*/
				else
				{
					Buf[i] = c1;
					i++;
				}
			}
			break;
		case true:
			if (c0 == '*' && c1 == '/')
				in_comment = false;
		}
		c0 = c1;
	}
	Buf[i] = '#';
	cinf.close();
}

void concat(char token[],char c)//将新字符连接进token
{
	int i = 0;
	for (i = 0; token[i]; i++);//找到单词尾
	token[i] = c;
	token[i + 1] = '\0';
}

char* reserve(char token[],char **table,char** table_code)
{
		for (int i = 0; i < table_len; i++)//遍历保留字表
		{
			if (strcmp(token, (table[i])) == 0)
			{
				return table_code[i];
			}
		}
		return "10";//不在保留字表则返回标识符类型 "10"
}

struct code_val scanner(char Buf[], int &i,ofstream &er)
{
	struct code_val temp = { "0","NUL" };
	char token[WordLen + 1] = "";
	/*if (Buf[i] == 'k')
	{
		int a = 0;
	}*/
	if (Buf[i] == '\n')
	{
		LineNum++;
		strcpy_s(temp.val , "EOLN");
		i++;
		strcpy_s(temp.code, "24");//" EOLN "
		return temp;
	}
	if (Buf[i] == '#')
	{
		strcpy_s(temp.val,"EOF");
		strcpy_s(temp.code, "25");//" EOF "
		return temp;
	}
	if ((Buf[i] >= 'a' && Buf[i] <= 'z') || (Buf[i] >= 'A' && Buf[i] <= 'Z'))
	{
		while ((Buf[i] >= 'a' && Buf[i] <= 'z') || (Buf[i] >= 'A' && Buf[i] <= 'Z') || (Buf[i] >= '0' && Buf[i] <= '9'))
		{
			concat(token, Buf[i]);
			i++;
		}
		strcpy_s(temp.code,reserve(token,table,table_code));//填写单词类别
		strcpy_s(temp.val, token);
		if (strcmp(temp.code,"10") == 0) //判断是标识符
		{
			if (find(symbol_table.begin(), symbol_table.end(), temp.val) == symbol_table.end())//在标识符表里找不到
			{
				symbol_table.push_back(temp.val);
			}
		}
		return temp;
	}
	if (Buf[i] >= '0' && Buf[i] <= '9')
	{
		while (Buf[i] >= '0' && Buf[i] <= '9')
		{
			concat(token, Buf[i]);
			i++;
		}
		strcpy_s(temp.code, "11");// 是常数
		strcpy_s(temp.val, token);
		return temp;
	}
	switch (Buf[i])
	{
	case '=':
		concat(token, Buf[i]);
		i++;
		strcpy_s(temp.code, "12");// " = "
		break;
	case '<':
		concat(token, Buf[i]);
		i++;
		if (Buf[i] == '>')
		{
			concat(token, Buf[i]);
			i++;
			strcpy_s(temp.code, "13");//" <> "
			break;
		}
		if (Buf[i] == '=')
		{
			concat(token, Buf[i]);
			i++;
			strcpy_s(temp.code, "14");//" <= "
			break;
		}
		else
		{
			strcpy_s(temp.code, "15");//" < "
			break;
		}
	case '>':
		concat(token, Buf[i]);
		i++;
		if (Buf[i] == '=')
		{
			concat(token, Buf[i]);
			i++;
			strcpy_s(temp.code, "16");//" >= "
			break;
		}
		else
		{
			strcpy_s(temp.code, "17");//" > "
			break;
		}
	case '-':
		concat(token, Buf[i]);
		i++;
		strcpy_s(temp.code, "18");// " - "
		break;
	case '*':
		concat(token, Buf[i]);
		i++;
		strcpy_s(temp.code, "19");// " * "
		break;
	case ':':
		if (Buf[i + 1] == '=') //一个" : "不在类别表中
		{
			concat(token, Buf[i]);
			concat(token, Buf[i+1]);
			i += 2;
			strcpy_s(temp.code, "20");// " := "
			break;
		}
		else
		{
			cout << "Error char-> " << Buf[i] << endl;
			er << "未知运算符 " << Buf[i - 1] << Buf[i] << endl;
		}
	case '(':
		concat(token, Buf[i]);
		i++;
		strcpy_s(temp.code, "21");// " ( "
		break;
	case ')':
		concat(token, Buf[i]);
		i++;
		strcpy_s(temp.code, "22");// " ) "
		break;
	case ';':
		concat(token, Buf[i]);
		i++;
		strcpy_s(temp.code, "23");// " ; "
		break;
	default:
		cout << "Error operation symbol -> " << " Line: " << LineNum<< " symbol: "<<Buf[i] << endl;
		er << " Line: " << LineNum << " | ErrorType: " << "Error operation symbol" << endl;
	}
	strcpy_s(temp.val, token);
	return temp;
}

void main()
{
	char Buf[BufSize] = { '\0' };
	pretreatment("Source.pas", Buf);
	LineNum = 1;
	//pretreatment("Source.pas", Buf);
	ofstream coutf("Source.dyd", ios::out);
	//ofstream coutf("LexOut.dyd", ios::out);
	ofstream er;
	er.open("Source.err");
	code_val t;
	int i = 0;
	do 
	{
		while (Buf[i] == ' ')
			i++;
		t = scanner(Buf, i,er);
		coutf << t.val << " " << t.code << endl;
		cout << "( " << t.val << " , " << t.code << " )" << endl;
	} while (strcmp(t.val, "EOF") != 0);
	for (int i = 0; i < symbol_table.size(); i++)
	{
		cout << symbol_table[i] << endl;
	}
	cout << endl;
	coutf.close();
	er.close();
	system("pause");
}
