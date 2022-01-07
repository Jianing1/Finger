#include "stdafx.h"
#include "InfoFile.h"
#include <sstream>
#include <vector>

InfoFile::InfoFile()
{
}

InfoFile::~InfoFile()
{
}

int InfoFile::CountLines()
{
    ifstream ReadFile;
    int n = 0;
    char line[512];
    ReadFile.open(F_LOGIN, ios::in);//ios::in 表示以只读的方式读取文件
    if (ReadFile.fail())//文件打开失败:返回0
    {
        return 0;
    }
    else//文件存在
    {
        while (!ReadFile.eof())
        {
            ReadFile.getline(line, 512, '\n');
            n++;
        }
        ReadFile.close();
        return n;
    }
}

CString InfoFile::ReadLine(int line)
{
    int i = -1;
    char temp[256] = {0};
    ifstream ifs;
    ifs.open(F_LOGIN, ios::in);
   
    while (i < line)
    {
        ifs.getline(temp, sizeof(temp), '\n');
        i++;
    }
    ifs.close();
    return CString(temp);
}

void InfoFile::writeRegister(char* name, char* pwd)
{
    ofstream ofs;
    ofs.open(F_LOGIN, ios::app);

    ofs << name << endl;  //写入用户姓名
    ofs << pwd << endl;   //写入密码
    ofs.close();
}

void InfoFile::ReadRegister(CString& name, CString& pwd)
{
    char buf[256] = { 0 };
    ifstream ifs;
    ifs.open(F_LOGIN, ios::in);

    ifs.getline(buf, sizeof(buf), '\n');
    name = CString(buf);

    ifs.getline(buf, sizeof(buf), '\n');
    pwd = CString(buf);

    ifs.close();
}
