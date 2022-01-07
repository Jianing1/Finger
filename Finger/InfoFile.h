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

    void writeRegister(char* name, char* pwd); //��ע����Ϣд�����
    void ReadRegister(CString& name, CString& pwd); //���ļ��ж�ȡ�û�������Ĳ���
    int CountLines();  //��һ���ж�����
    CString ReadLine(int line);
};

