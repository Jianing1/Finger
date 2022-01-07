#pragma once

#include <list>
#include <fstream>
#include <iostream>
#include <string>
using namespace std;

#define F_LOGIN "./login.ini"

class InfoFile
{
public:
    InfoFile();
    ~InfoFile();

    void writeRegister(char* name, char* pwd); //将注册信息写入操作
    void ReadRegister(CString& name, CString& pwd); //从文件中读取用户名密码的操作
    int CountLines();  //读一共有多少行
    CString ReadLine(int line);
};

