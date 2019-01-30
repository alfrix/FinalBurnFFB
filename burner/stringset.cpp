// StringSet C++ class
#include "app.h"

int __cdecl StringSet::Add(char *szFormat,...)
{
  char szAdd[256]; int nAddLen=0; char *NewMem=NULL;
  va_list Arg; va_start(Arg,szFormat);
  vsprintf(szAdd,szFormat,Arg);
  nAddLen=strlen(szAdd); // find out the length of the new text
  NewMem=(char *)realloc(szText,nLen+nAddLen+1);
  if (NewMem!=NULL)
  {
    szText=NewMem;
    // copy the new text to the end
    memcpy(szText+nLen,szAdd,nAddLen);
    nLen+=nAddLen;
    szText[nLen]=0; // zero-terminate
  }
  va_end(Arg);
  return 0;
}

int StringSet::Reset()
{
  // Reset the text
  nLen=0;
  szText=(char *)realloc(szText,1); if (szText==NULL) return 1;
  szText[0]=0;
  return 0;
}

StringSet::StringSet()
{
  szText=NULL; nLen=0;
  Reset(); // reset string to nothing
}

StringSet::~StringSet()
{
  realloc(szText,0); // Free BZip text
}
