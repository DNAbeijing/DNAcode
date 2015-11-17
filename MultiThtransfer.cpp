// MultiThtransfer.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include<io.h>
#include <stdio.h>
#include<iostream>
#include<vector>
#include<fstream>
#include<string>
#include<iostream>
#include<time.h>
#include<stdlib.h>
#include <strstream>
#include <direct.h>
#include<windows.h>
#include <thread>

using namespace std;

#define COUNT 3
#define NUM_THREADS 5

void getFiles(string path, vector<string>& files)
{
	//文件句柄  
	long   hFile = 0;
	//文件信息  
	struct _finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			//如果是目录,迭代之  
			//如果不是,加入列表  
			if ((fileinfo.attrib &  _A_SUBDIR))
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
					getFiles(p.assign(path).append("\\").append(fileinfo.name), files);
			}
			else
			{
				files.push_back(p.assign(path).append("\\").append(fileinfo.name));
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}

vector< string> split(string str, string pattern)
{
	vector<string> ret;
	if (pattern.empty()) return ret;
	size_t start = 0, index = str.find_first_of(pattern, 0);
	while (index != str.npos)
	{
		if (start != index)
			ret.push_back(str.substr(start, index - start));
		start = index + 1;
		index = str.find_first_of(pattern, start);
	}
	if (!str.substr(start).empty())
		ret.push_back(str.substr(start));
	return ret;
}

string toSeq(string s, string lastS)
{
	string seq = "";
	//char tmp[100];
	char *tmp;
	tmp = (char *)malloc(sizeof(char) * 200);
	int sum = 0;
	if (lastS != "")
	{
		sum = atoi(lastS.c_str());
	}
	string tmp_seq;
	switch (s[0]){
	case 'A':
		sum += 2;
		_itoa_s(sum, tmp, 10, 10);
		tmp_seq = tmp;
		seq = tmp_seq;
		break;
	case 'G':
		sum += 1;
		_itoa_s(sum, tmp, 10, 10);
		tmp_seq = tmp;
		seq = tmp_seq;
		break;
	case 'C':
		sum += -1;
		_itoa_s(sum, tmp, 10, 10);
		tmp_seq = tmp;
		seq = tmp_seq;
		break;
	case 'T':
		sum += -2;
		_itoa_s(sum, tmp, 10, 10);
		tmp_seq = tmp;
		seq = tmp_seq;
		break;
	default:
		_itoa_s(sum, tmp, 10, 10);
		tmp_seq = tmp;
		seq = tmp_seq;
		break;
	}
	for (int i = 1; i < s.size(); i++){
		switch (s[i]){
		case 'A':
			sum += 2;
			_itoa_s(sum, tmp, 10, 10);
			tmp_seq = tmp;
			seq = seq + "," + tmp_seq;
			break;
		case 'G':
			sum += 1;
			_itoa_s(sum, tmp, 10, 10);
			tmp_seq = tmp;
			seq = seq + "," + tmp_seq;
			break;
		case 'C':
			sum += -1;
			_itoa_s(sum, tmp, 10, 10);
			tmp_seq = tmp;
			seq = seq + "," + tmp_seq;
			break;
		case 'T':
			sum += -2;
			_itoa_s(sum, tmp, 10, 10);
			tmp_seq = tmp;
			seq = seq + "," + tmp_seq;
			break;
		default:
			_itoa_s(sum, tmp, 10, 10);
			tmp_seq = tmp;
			seq = seq + "," + tmp_seq;
			break;
		}
	}
	free(tmp);
	return seq;
}


//递归创建目录
void CreateDir(char* strDirPath)
{
	if (strlen(strDirPath)>MAX_PATH)
	{
		return;
	}
	int ipathLength = strlen(strDirPath);
	int ileaveLength = 0;
	int iCreatedLength = 0;
	char szPathTemp[MAX_PATH] = { 0 };
	for (int i = 0; (NULL != strchr(strDirPath + iCreatedLength, '\\')); i++)
	{
		ileaveLength = strlen(strchr(strDirPath + iCreatedLength, '\\')) - 1;
		iCreatedLength = ipathLength - ileaveLength;
		strncpy_s(szPathTemp, strDirPath, iCreatedLength);
		_mkdir(szPathTemp);
	}
	if (iCreatedLength<ipathLength)
	{
		_mkdir(strDirPath);
	}
}


void myThread(int seg)
{
	char * filePath = "D:\\study files\\Data Mining\\DNA\\Bac_fna";
	//char * filePath = "D:\\study files\\Data Mining\\DNA\\RAW_DATA\\test\\fq";
	vector<string> files;
	string pattern = "\\";

	////获取该路径下的所有文件路径和文件名  
	getFiles(filePath, files);

	//char str[30];
	int size = files.size();
	cout << size << endl;
	vector<string> tmp;
	string name, path;
	ofstream out;
	//ofstream outlen;
	//string* s;
	vector<string> tmpTS;
	int i = seg;
	while (i < size)
	{
		long start, finish;
		double totaltime;
		start = clock();
		//cout << files[i].c_str() << endl;

		tmp = split(files[i], pattern);

		/*
		for (int j = 1; j < tmp.size() - 1; j++){
		path = path + "\\" + tmp[j];
		}
		*/
		path = "F:\\tmp\\Bac_fna_txt";

		//创建不存在的文件夹
		char *mk_dir;
		mk_dir = (char *)malloc(sizeof(char) * 100);
		mk_dir = const_cast<char*>(path.c_str());
		CreateDir(mk_dir);
		//free(mk_dir);

		name = tmp[tmp.size() - 1];
		tmp = split(name, ".");
		name = tmp[0];


		out.open(path + "\\" + name + ".txt", ios::trunc);
		//outlen.open(path + "\\" + name + "_len.txt", ios::trunc);

		ifstream in(files[i].c_str());

		string str;
		string lastS = "";
		while (!in.eof())
		{
			int k = 0;
			bool flag = false;
			//s = (string *)malloc(sizeof(string)*COUNT);
			//tmpTS = (char*)malloc(sizeof(char) * 200);

			getline(in, str);
			if (str[0] >= 'A' && str[0] <= 'Z'){
				if (in.eof()){
					//outlen << str.size();
					str = toSeq(str, lastS);
					tmpTS = split(str, ",");
					for (int m = 0; m < tmpTS.size() - 1; m++){
						out << tmpTS[m] << endl;
					}
					out << tmpTS[tmpTS.size() - 1] << endl;
					lastS = tmpTS[tmpTS.size() - 1];
					break;
				}
				else{
					//outlen << str.size() << endl;
					str = toSeq(str, lastS);
					tmpTS = split(str, ",");
					for (int m = 0; m < tmpTS.size() - 1; m++){
						out << tmpTS[m] << endl;
					}
					out << tmpTS[tmpTS.size() - 1] << endl;
					lastS = tmpTS[tmpTS.size() - 1];
				}
			}
		}
		finish = clock();
		totaltime = (double)(finish - start) / CLOCKS_PER_SEC;
		out.close();
		//outlen.close();
		cout << "The running time is: " << totaltime / 60.0 << endl;
		i = i + 5;
	}
	out.close();
}

int _tmain(int argc, _TCHAR* argv[])
{
	//char * filePath = "D:\\study files\\Data Mining\\DNA\\RAW_DATA\\test";
	//char * filePath = "D:\\study files\\Data Mining\\DNA\\fna";
	
	//char * filePath = "D:\\study files\\Data Mining\\DNA\\Bac_fna";
	//vector<string> files;
	//string pattern = "\\";

	//////获取该路径下的所有文件路径和文件名  
	//getFiles(filePath, files);

	////char str[30];
	//int size = files.size();
	//cout << size << endl;
	//vector<string> tmp;
	//string name, path;
	//ofstream out;
	////ofstream outlen;
	////string* s;
	//vector<string> tmpTS;

	//for (int i = 0; i < size; i++)
	//{
	//	long start, finish;
	//	double totaltime;
	//	start = clock();
	//	//cout << files[i].c_str() << endl;

	//	tmp = split(files[i], pattern);

	//	/*
	//	for (int j = 1; j < tmp.size() - 1; j++){
	//	path = path + "\\" + tmp[j];
	//	}
	//	*/
	//	path = "F:\\tmp\\Bac_txt";

	//	//创建不存在的文件夹
	//	char *mk_dir;
	//	mk_dir = (char *)malloc(sizeof(char) * 100);
	//	mk_dir = const_cast<char*>(path.c_str());
	//	CreateDir(mk_dir);
	//	//free(mk_dir);

	//	name = tmp[tmp.size() - 1];
	//	tmp = split(name, ".");
	//	name = tmp[0];


	//	out.open(path + "\\" + name + ".txt", ios::trunc);
	//	//outlen.open(path + "\\" + name + "_len.txt", ios::trunc);

	//	ifstream in(files[i].c_str());

	//	string str;
	//	string lastS = "";
	//	while (!in.eof())
	//	{
	//		int k = 0;
	//		bool flag = false;
	//		//s = (string *)malloc(sizeof(string)*COUNT);
	//		//tmpTS = (char*)malloc(sizeof(char) * 200);

	//		getline(in, str);
	//		if (str[0] >= 'A' && str[0] <= 'Z'){
	//			if (in.eof()){
	//				//outlen << str.size();
	//				str = toSeq(str, lastS);
	//				tmpTS = split(str, ",");
	//				for (int m = 0; m < tmpTS.size() - 1; m++){
	//					out << tmpTS[m] << endl;
	//				}
	//				out << tmpTS[tmpTS.size() - 1] << endl;
	//				lastS = tmpTS[tmpTS.size() - 1];
	//				break;
	//			}
	//			else{
	//				//outlen << str.size() << endl;
	//				str = toSeq(str, lastS);
	//				tmpTS = split(str, ",");
	//				for (int m = 0; m < tmpTS.size() - 1; m++){
	//					out << tmpTS[m] << endl;
	//				}
	//				out << tmpTS[tmpTS.size() - 1] << endl;
	//				lastS = tmpTS[tmpTS.size() - 1];
	//			}
	//		}
	//	}
	//	finish = clock();
	//	totaltime = (double)(finish - start) / CLOCKS_PER_SEC;
	//	out.close();
	//	//outlen.close();
	//	cout << "The running time is: " << totaltime / 60.0 << endl;
	//}
		long start, finish;
		double totaltime;
		start = clock();
	thread a0(myThread, 0);
	thread a1(myThread, 1);
	thread a2(myThread, 2);
	thread a3(myThread, 3);
	thread a4(myThread, 4);
	/*thread a5(myThread, 5);
	thread a6(myThread, 6);
	thread a7(myThread, 7);
	thread a8(myThread, 8);
	thread a9(myThread, 9);*/
	a0.join();
	a1.join();
	a2.join();
	a3.join();
	a4.join();
	/*a5.join();
	a6.join();
	a7.join();
	a8.join();
	a9.join();*/
		finish = clock();
		totaltime = (double)(finish - start) / CLOCKS_PER_SEC;

		//outlen.close();
		cout << "The running time is: " << totaltime / 60.0 << endl;
	return 0;
}


