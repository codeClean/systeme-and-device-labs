/*
 mylab  9 ex 3 a
 */

#define UNICODE
#define _UNICODE

#define _CRT_DEFINE_NO_WARNINGS

#include <windows.h>
#include <tchar.h>
#include <process.h>
#include <stdio.h>

// Debug Printing (y/n)
#define DEBUG    1
// Debug: Waiting Time (dt milliseconds)
#define dt    1000

#define BUF_SIZE 16384
#define L 500

#define DIRNAME_LEN MAX_PATH + 2

#define TYPE_FILE 1
#define TYPE_DIR 2
#define TYPE_DOT 3

typedef struct threads {
  LPTSTR fileName;
  LPTSTR *directories; 
  LPTSTR outfile;
  INT thid;
} threads_t;
INT WINAPI threadFunction(LPVOID);
static void TraverseAndPrint (LPTSTR, INT, HANDLE);
static DWORD FileType(LPWIN32_FIND_DATA);



INT _tmain(
  INT argc, 
  LPTSTR argv[]
  )
{
  INT i, j,fileN,n; 
  DWORD *threadId;
  HANDLE *threadHandle; 
  threads_t *threadData;
 

  fileN = argc - 1;

  threadData = (threads_t *)malloc(fileN * sizeof(threads_t));
  threadHandle = (HANDLE *)malloc(fileN * sizeof(HANDLE));
  threadId = (DWORD *)malloc(fileN * sizeof(DWORD));

  LPTSTR newFile;

  // Run Thread
  for (i=0; i<fileN-1; i++) {
      threadData[i].fileName = argv[i+1];
      threadData[i].thid = i;
      _tprintf(_T("dirctive %d  = %s \n"),i,threadData[i].fileName);
      //_stprintf( newFile, _T("%s%d\n"),argv[i+1],i); 
      threadData[i].outfile = (LPTSTR)malloc(MAX_PATH * sizeof(TCHAR));


      _stprintf(threadData[i].outfile, _T("%d%s.txt"),i,argv[fileN]); 
      _tprintf(_T("out file %d  = %s "),i,threadData[i].outfile);

      threadHandle[i] = CreateThread(NULL, 0,
      (LPTHREAD_START_ROUTINE)threadFunction, &threadData[i],
      0, &threadId[i]);
      if (threadHandle[i] == NULL) {
      ExitProcess(0);
      }
      Sleep(2000);
  }

  // Wait until all threads have terminated.
  WaitForMultipleObjects(fileN, threadHandle, TRUE, INFINITE);
  for (i=0; i<fileN; i++) {
    CloseHandle(threadHandle[i]);
  }
LARGE_INTEGER fileSize;
for (int i = 0; i < fileN-1; ++i)
{ 
  HANDLE hIn;
  TCHAR buffer[BUF_SIZE];
  DWORD nIn;
  LPTSTR content;
  hIn = CreateFile(threadData[i].outfile,GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,  FILE_ATTRIBUTE_NORMAL, NULL);
  if (hIn == INVALID_HANDLE_VALUE) {
    _tprintf(_T("Open file error: %x\n"), GetLastError());
    return (1);
  }
  if (!GetFileSizeEx(hIn, &fileSize)) {
    _tprintf(_T("Impossible to get file size for %s\n"), threadData[i].outfile); 
    return (1);
  }
  fileSize.HighPart=0;
   content = (LPTSTR)malloc(fileSize.LowPart + sizeof(TCHAR));
  _tprintf(_T("reading file %s with sizeof = %d \n"),threadData[i].outfile,fileSize.LowPart);  

 while (ReadFile(hIn, content,fileSize.LowPart, &nIn, NULL) && nIn == fileSize.LowPart) {
    content[fileSize.LowPart / sizeof(TCHAR)] = _T('\0');

   _tprintf(_T("%s//"),content); 
    }
  _tprintf(_T(" \\reading is done \n \n"));
  free(threadData[i].outfile);
  free(content);
  CloseHandle(hIn);

}

  /*for (i = 0; i < fileN; i++) {
    free(threadData[i]);
  }*/
  free(threadData);
  free(threadHandle);
  free(threadId);

  _tprintf(_T("End Now: "));
  _tscanf_s(_T("%d"), &n);

  return 0;
}

INT WINAPI threadFunction(  LPVOID lpParam  )
{
      threads_t *data;

      data = (threads_t *)lpParam;
      HANDLE hOut;
      _tprintf(_T(" the file to be created is %s \n "),data->outfile);
      hOut = CreateFile (data->outfile, GENERIC_WRITE, 0, NULL,
      CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
      if (hOut == INVALID_HANDLE_VALUE) {
        _tprintf(_T(" error  file %s \n "),data->outfile);

        fprintf (stderr, "Cannot open output file   Error: %x\n",  GetLastError ());
        CloseHandle(hOut);
      return 3;
      }


 
      TraverseAndPrint(data->fileName,data->thid,hOut);

      _tprintf(_T(" exiting from thread %d \n"),data->thid);

      CloseHandle(hOut);

      ExitThread(0);
}

static void TraverseAndPrint( LPTSTR SourcePathName ,INT thid, HANDLE hOut  )
{
        HANDLE SearchHandle;
        WIN32_FIND_DATA FindData;
        DWORD FType, l;
        TCHAR newline[MAX_PATH + 20],s[MAX_PATH + 20];
        DWORD n,strLen;


        // SetCurrentDirectory(SourcePathName);

        _stprintf(s, _T("%s\\*"), SourcePathName);


        SearchHandle = FindFirstFile(s, &FindData);

        _tprintf(_T("\n--> current directory FILE: %s \n"),s);

        do {
                    FType = FileType(&FindData);
                    l = _tcslen(SourcePathName);
                    if (SourcePathName[l-1] == '\\') {
                      _stprintf (s, _T("%s%s"), SourcePathName, FindData.cFileName); 
                    } else {
                      _stprintf (s, _T("%s\\%s"), SourcePathName, FindData.cFileName); 
                    }

                    if (FType == TYPE_FILE) 
                    {
                             
                      _stprintf(newline,_T("%u-%s\n"),GetCurrentThreadId(),FindData.cFileName);
                              strLen = (DWORD)_tcscnlen(newline, MAX_PATH + 20);

                               if (!WriteFile(hOut, newline, strLen * sizeof(TCHAR), &n, NULL) || n != strLen * sizeof(TCHAR)) {
                                 _tprintf(_T("%u - Error writing file"), GetCurrentThreadId());
                              } 
                                _tprintf(_T("In file  %d number of byte is writen, file name %s\n "),n,FindData.cFileName);
 
                    }
                    if (FType == TYPE_DIR) {
                               
                              _stprintf(newline,_T("%u-%s\n"),GetCurrentThreadId(),FindData.cFileName);
                              strLen = (DWORD)_tcscnlen(newline, MAX_PATH + 20);

                               if (!WriteFile(hOut, newline, strLen * sizeof(TCHAR), &n, NULL) || n != strLen * sizeof(TCHAR)) {
                                 _tprintf(_T("%u - Error writing file"), GetCurrentThreadId());
                              }

                                _tprintf(_T("IN dir  %d number of byte is writen, file name %s\n "),n,FindData.cFileName);

                                TraverseAndPrint(s, thid,hOut);
                                // SetCurrentDirectory(_T(".."));
                     }

            } while (FindNextFile(SearchHandle, &FindData));

        FindClose(SearchHandle);

 return;
}


static DWORD FileType( LPWIN32_FIND_DATA pFileData )
{
  BOOL IsDir;
  DWORD FType;
  FType = TYPE_FILE;
  IsDir = (pFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
  if (IsDir)
    if (lstrcmp(pFileData->cFileName, _T(".")) == 0
      || lstrcmp(pFileData->cFileName, _T("..")) == 0)
      FType = TYPE_DOT;
    else
      FType = TYPE_DIR;

  return FType;
}





