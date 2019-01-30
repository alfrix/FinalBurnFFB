//---------------------------------------------------------------------------
// Burner - Arcade emulator interface
// Copyright (c) 2002 Dave (www.finalburn.com), all rights reserved.

// This refers to all code except where stated otherwise
// (e.g. unzip and zlib code)

// You can use, modify and redistribute this code freely as long as you
// don't do so commercially. This copyright notice must remain with the code.
// If your program uses this code, you must either distribute or link to the
// source code. If you modify or improve this code, you must distribute
// the source code improvements.

// Dave
// Homepage: www.finalburn.com
// E-mail:  dave@finalburn.com
//---------------------------------------------------------------------------

// Main module

#include "app.h"
#ifdef _DEBUG
#include <crtdbg.h>
#endif

//#define APP_DEBUG_LOG // log debug messages to zzd.txt

HINSTANCE hAppInst=NULL; // Application Instance
static char *szCmdLine="";
int nAppVirtualFps=600; // App fps * 10
int nUseSysMemory=-1;
int bAppBassFilter=0;
char szAppBurnVer[16]="";

// Show a message box with an error message
int AppError(char *szText,int bWarning)
{
  DSoundStop();
  MessageBox(hScrnWnd,szText,
    bWarning ? APP_TITLE " Warning" :  APP_TITLE " Error",
    MB_OK | (bWarning?MB_ICONWARNING:MB_ICONERROR)
  );
  DSoundPlay();
  return 0;
}

static int AppInit()
{
  int nRet=0;

  // Bump up the thread priority
  SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_ABOVE_NORMAL);
  // Load config for the application
  nRet=ConfigAppLoad(); if (nRet!=0) ConfigAppSave(); // create initial config file

  // Start any wave logging here if needed

  BurnLibInit(); // Init the Burn library

  MediaInit();

  return 0;
}

static int AppExit()
{
  DrvExit(); // Make sure any game driver is exited
  MediaExit();
  BurnLibExit(); // Exit the Burn library
  WaveLogStop(); // Stop any wave logging
  ConfigAppSave(); // Save config for the application

  return 0;
}

int AppMessage(MSG *pMsg)
{
  // Process dialog messages
  if (IsDialogMessage(hPanelDlg,pMsg)) return 0;
  if (IsDialogMessage(hInpdDlg,pMsg)) return 0;
  if (IsDialogMessage(hInpsDlg,pMsg)) return 0;
  if (IsDialogMessage(hInpcDlg,pMsg)) return 0;
  if (IsDialogMessage(hTrainDlg,pMsg)) return 0;
  return 1; // Didn't process this message
}

#ifdef APP_DEBUG_LOG
static FILE *DebugFile=NULL;
// Debug printf to a file
static int __cdecl AppDebugPrintf(char *szFormat,...)
{
  va_list Arg; va_start(Arg,szFormat);

  if (DebugFile!=NULL) { vfprintf(DebugFile,szFormat,Arg); fflush(DebugFile); }
  va_end(Arg); return 0;
}
#endif

// Main program entry point
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE,LPSTR lpCmdLine,int)
{
  int nRet=0;

  hAppInst=hInstance;

  AppDirectory(); // Set current directory to be the applications directory

  // Make version string
  if (nBurnVer&0xff)
  {
    // private version
    sprintf (szAppBurnVer,"%x.%.5x",nBurnVer>>20,nBurnVer&0xfffff);
  }
  else
  {
    // public version
    sprintf (szAppBurnVer,"%x.%.3x",nBurnVer>>20,(nBurnVer>>8)&0xfff);
  }

  // Make sure there are roms and cfg subdirectories
  CreateDirectory("roms",NULL);
  CreateDirectory("cfg" ,NULL);
  CreateDirectory("state",NULL);

#ifdef APP_DEBUG_LOG
  DebugFile=fopen("zzd.txt","wt"); // Log debug to a file
  dprintf=AppDebugPrintf; // Redirect Burn library debug to our function
#endif

  InitCommonControls();

  if (lpCmdLine!=NULL) szCmdLine=lpCmdLine;

  nRet=AppInit(); // Init the application
  if (nRet!=0) goto Error;

  RunMessageLoop(); // Run the application message loop

Error:
  AppExit(); // Exit the application

#ifdef APP_DEBUG_LOG
  if (DebugFile!=NULL) fclose(DebugFile);  DebugFile=NULL;
#endif

#ifdef _DEBUG
  _CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF ); // Check for memory leaks
#endif
  return 0;
}
