/* ******************************************************************** **
** @@ TGI
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  : Empty FileInfo Creator for TG (Directory HTML table generator)
** ******************************************************************** */

/* ******************************************************************** **
**                uses pre-compiled headers
** ******************************************************************** */

#include <stdafx.h>

#include <stdio.h>
#include <time.h>

#include "..\shared\file_find.h"
#include "..\shared\vector.h"
#include "..\shared\vector_sorted.h"

/* ******************************************************************** **
** @@                   internal defines
** ******************************************************************** */

struct ListEntry
{
   char           _pszShortName[MAX_PATH + 1];
};

#define  INITIAL_SIZE         (256)
#define  INITIAL_DELTA        (32)

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef NDEBUG
#pragma optimize("gsy",on)
#pragma comment(linker,"/FILEALIGN:512 /MERGE:.rdata=.text /MERGE:.data=.text /SECTION:.text,EWR /IGNORE:4078")
#endif

/* ******************************************************************** **
** @@                   internal prototypes
** ******************************************************************** */

/* ******************************************************************** **
** @@                   external global variables
** ******************************************************************** */

extern DWORD   dwKeepError = 0;

/* ******************************************************************** **
** @@                   static global variables
** ******************************************************************** */

static SortedVector*    _pList = NULL;

/* ******************************************************************** **
** @@                   real code
** ******************************************************************** */

/* ******************************************************************** **
** @@ CompareNamesShort()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

static int CompareNamesShort(const void** const pKey1,const void** const pKey2)
{
   ListEntry**     p1 = (ListEntry**)pKey1;
   ListEntry**     p2 = (ListEntry**)pKey2;

   return stricmp((*p1)->_pszShortName,(*p2)->_pszShortName);
}

/* ******************************************************************** **
** @@ ForEach()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

static void ForEach(const char* const pszPrefix,const WIN32_FIND_DATA& w32fd)
{
   char*    pDot = strrchr(w32fd.cFileName,'.');

   if (!pDot)
   {
      // Error !
      // No Extension
      return;
   }

   if (!stricmp(pDot + 1,"i"))
   {
      // Info file
      return;
   }

   ListEntry*     pEntry = new ListEntry;

   if (!pEntry)
   {
      return;
   }

   memset(pEntry,0,sizeof(ListEntry));

   if (strlen(pszPrefix))
   {
      strcat(pEntry->_pszShortName,pszPrefix);
      strcat(pEntry->_pszShortName,"\\");
   }

   strcat(pEntry->_pszShortName,w32fd.cFileName);

   if (_pList->Insert(pEntry) == -1)
   {
      delete pEntry;
      pEntry = NULL;
   }
}

/* ******************************************************************** **
** @@ ShowHelp()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

static void ShowHelp()
{
   const char  pszCopyright[] = "-*-   TGI 1.0   *   (c) gazlan 2011   -*-";
   const char  pszDescript [] = "Empty FileInfo Creator for TG";
   const char  pszE_Mail   [] = "complains_n_suggestions direct to gazlan@yandex.ru";

   printf("%s\n\n",pszCopyright);
   printf("%s\n\n",pszDescript);
   printf("Usage: tgi.exe [FullPath]\n\n");
   printf("%s\n",pszE_Mail);
}

/* ******************************************************************** **
** @@ main()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

int main(int argc,char** argv)
{
   if (argc > 2)
   {
      ShowHelp();
      return 0;
   }

   FindFile*   pFF = new FindFile;

   if (!pFF)
   {
      // Error !
      return -1;
   }

   if (argc == 2)
   {
      if ((!strcmp(argv[1],"?")) || (!strcmp(argv[1],"/?")) || (!strcmp(argv[1],"-?")) || (!stricmp(argv[1],"/h")) || (!stricmp(argv[1],"-h")))
      {
         ShowHelp();
         return 0;
      }

      pFF->SetPrefix(argv[1]);
   }

   pFF->SetMask("*.*");

   _pList = new SortedVector;

   if (!_pList)
   {
      // Error !
      delete pFF;
      pFF = NULL;
      return -1;
   }

   _pList->Resize(INITIAL_SIZE);
   _pList->Delta(INITIAL_DELTA);
   _pList->SetSorter(CompareNamesShort);

   while (pFF->Fetch())
   {
      if ((pFF->_w32fd.dwFileAttributes | FILE_ATTRIBUTE_NORMAL) && !(pFF->_w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
      {
         ForEach(pFF->_pszPrefix,pFF->_w32fd);
      }
   }

   delete pFF;
   pFF = NULL;

   _pList->QSort();

   int iCnt = _pList->Count();

   for (int ii = iCnt - 1; ii >= 0; --ii)
   {
      ListEntry*     pEntry = (ListEntry*)_pList->At(ii);

      char     pszInfoName[MAX_PATH + 1];

      strcpy(pszInfoName,pEntry->_pszShortName);
      strcat(pszInfoName,".i");

      // Check exists
      HANDLE   hFile = CreateFile(pszInfoName,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

      if (hFile != INVALID_HANDLE_VALUE)
      {
         // File exist
         CloseHandle(hFile);
         hFile = INVALID_HANDLE_VALUE;

         delete pEntry;
         pEntry = NULL;

         _pList->RemoveAt(ii);

         continue;
      }

      FILE*    pOut = fopen(pszInfoName,"wt");

      time_t         LongTime;

      time(&LongTime);

      struct tm*     pNow = localtime(&LongTime);

      fprintf(pOut,"; --- Made by TGI FileInfo Creator 1.0  *  %04d-%02d-%02d  %02d:%02d:%02d\n",pNow->tm_year + 1900,pNow->tm_mon + 1,pNow->tm_mday,pNow->tm_hour,pNow->tm_min,pNow->tm_sec);
      fprintf(pOut,"%s\n","; Predefined Icon types are: DEFAULT, DOS16, DOS32, WIN16, WIN32, MAC, X. May be blank.");
      fprintf(pOut,"%s\n","; If DEFAULT type chosen, Icon reference will be generated related to the File Extension.");
      fprintf(pOut,"%s\n","; So, for *.html -> html.png; *.rar -> rar.png; *.chm -> chm.png; etc.");
      fprintf(pOut,"%s\n","; For Custom IconType place Icon filename into brackets. Ex: ICON=[custom.png]");
      fprintf(pOut,"%s\n","ICON=DEFAULT");
      fprintf(pOut,"%s\n","; Name is FullName for the File. Be shown in the BOLD.");
      fprintf(pOut,"%s\n","; Should be up to 256 chars max (without tabs and EOL signs). May be blank.");
      fprintf(pOut,"%s\n","NAME=");
      fprintf(pOut,"%s\n","; Vers is the Version of this file (if any). May be blank.");
      fprintf(pOut,"%s\n","; Up to 256 chars max (without tabs and EOL signs).");
      fprintf(pOut,"%s\n","VERS=");
      fprintf(pOut,"%s\n","; Text is Verbouse Description to the File.");
      fprintf(pOut,"%s\n","; Up to 2 Kb text max (without tabs and EOL signs).  May be blank (Not recommemnded).");
      fprintf(pOut,"%s\n","INFO=");

      fclose(pOut);
      pOut = NULL;

      delete pEntry;
      pEntry = NULL;

      _pList->RemoveAt(ii);
   }

   delete _pList;
   _pList = NULL;

   return 0;
}
