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
    ReadFile.open(F_LOGIN, ios::in);//ios::in ��ʾ��ֻ���ķ�ʽ��ȡ�ļ�
    if (ReadFile.fail())//�ļ���ʧ��:����0
    {
        return 0;
    }
    else//�ļ�����
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

    ofs << name << endl;  //д���û�����
    ofs << pwd << endl;   //д������
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
