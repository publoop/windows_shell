#include <stdio.h>
#include <windows.h>
#include <tchar.h>
#include <vector>
using std::vector;

/************************************************************************/
/* 遍历指定目录及其子目录                                                */
/************************************************************************/

//目录信息结构体
typedef struct _DIR
{
	char nPath[MAX_PATH]; //目录路径信息
	int nDeep; //深度信息
	unsigned long long nLineinfo; //连接线信息
}DIR, *PDIR;

/*
	打印节点信息
	arg:
		filename	文件名
		deep 深度
		lineinfo 连接线信息
*/
static void printNode(const TCHAR * filename, int deep, unsigned long long lineinfo)
{
	
	for (int i = 0; i < deep; i++)
	{
		if ((lineinfo >> i) & 1) 
			printf("|        ");
		else
			printf("         ");
	}
	_tprintf(_T("|_ _ _ %s\n"), filename);
}

void Travel(const TCHAR* pszPath)
{
	vector<PDIR> dirlist;
	PDIR pdir;

	PDIR tmp;
	HANDLE hFind = NULL; //当前文件句柄
	WIN32_FIND_DATA findData = { 0 };//当前文件信息

	unsigned long long current_lineinfo; //当前目录要打印的连线
	int current_deep;//当前目录的深度


	//初始化根目录
	pdir = (PDIR)malloc(sizeof DIR);
	//合并成一个可用的目录信息
	_stprintf_s(pdir->nPath, MAX_PATH, _T("%s\\*"), pszPath); 
	pdir->nDeep = 0;
	pdir->nLineinfo = 0xffffffffffffffff;
	dirlist.push_back(pdir);
	do{
		
		pdir = dirlist.back();
		dirlist.pop_back();

		hFind = FindFirstFile(pdir->nPath, &findData);
		pdir->nPath[_tcslen(pdir->nPath) - 2] = '\0';

		printNode(pdir->nPath, pdir->nDeep, pdir->nLineinfo); //打印父文件夹
		if (hFind == INVALID_HANDLE_VALUE) //如果该目录是空文件夹,遍历下一个文件夹
		{
			free(pdir);
			break;
		}

		current_deep = pdir->nDeep + 1; //当前目录的深度为父文件夹的深度+1
		//判断父文件夹是否是上级目录下最后一个文件夹
		if (!(dirlist.empty() || dirlist.back()->nDeep != pdir->nDeep)) //计算当前目录要打印的线
		{
			//父文件夹不是父级目录的最后一个文件夹还需要为父级目录的其他文件夹提供连线
			current_lineinfo = pdir->nLineinfo;
		}
		else{//父文件夹是父级目录的最后一个将父文件夹不用为父级目录其他文件夹提供连线,将该位置0
			current_lineinfo = pdir->nLineinfo & ~(1 << pdir->nDeep);
		}

		do
		{
			if (!_tccmp(findData.cFileName, ".") || !_tccmp(findData.cFileName, ".."))
			{
				continue; 
			}	

			// 判断当前文件是文件还是文件夹
			if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))//如果是文件直接打印
			{
				printNode(findData.cFileName, current_deep, current_lineinfo);
			}
			else //如果是文件夹封装为目录信息保存在list中
			{
				tmp = (PDIR)malloc(sizeof DIR);
				// 将当前文件夹路径和目录名合成一个新的文件夹路径
				_stprintf_s(tmp->nPath, MAX_PATH, _T("%s\\%s\\*"), pdir->nPath, findData.cFileName);
				tmp->nDeep = current_deep;
				tmp->nLineinfo = current_lineinfo;
				dirlist.push_back(tmp);
			}

		} while (FindNextFile(hFind, &findData));
		FindClose(hFind);

		free(pdir);
	} while ((!dirlist.empty()));

	
}


int main(int argc, char*argv[])
{
	TCHAR *dir;
	if (argc < 2)
		dir = _T(".");
	else
		dir = _T(argv[1]);
	Travel(dir);

	return 0;
}