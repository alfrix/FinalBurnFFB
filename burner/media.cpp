// Media module
#include "app.h"

HINSTANCE DDrawDll=NULL;

int MediaInit()
{
  int nRet=0; static char szWinName[512]="";

  // Init the Scrn Window
  nRet=ScrnInit();
  if (nRet!=0) { AppError("ScrnInit Failed",0); return 1; }
  // Show the screen window
  ShowWindow(hScrnWnd,SW_NORMAL);

  // Init DirectInput
  DirInputInit(hAppInst,hScrnWnd); // But can still use keyboard if it fails

  nRet=InputInit(); // Init Input

  // Init Dsound
  nDSoundFps=nAppVirtualFps;
  DSoundInit(hScrnWnd); // (not critical if it fails)

  // ------- Set up Burn library sound ------------------------
  nBurnSoundRate=0; pBurnSoundOut=NULL; // Assume no sound
  if (bDSoundOkay)
  {
    nBurnSoundRate=nDSoundSamRate;
    nBurnSoundLen=nDSoundSegLen;
  }

  // Dynamically load ddraw.lib
  DDrawDll=LoadLibrary("ddraw.dll");

  hVidWnd=hScrnWnd; // Use Screen window for video
  // Video is init later
  return 0;
}

int MediaExit()
{
  hVidWnd=NULL;
  if (DDrawDll!=NULL) FreeLibrary(DDrawDll);  DDrawDll=NULL;

  nBurnSoundRate=0; pBurnSoundOut=NULL; // Blank sound
  // ------- End of Set up Burn library ---------------------------

  DestroyWindow(hInpsDlg); // Make sure the Input Set dialog is exited
  DestroyWindow(hInpdDlg); // Make sure the Input Dialog is exited

  DSoundExit(); // Exit Dsound
  InputExit();
  DirInputExit();  // Exit DirectInput

  ScrnExit();  // Exit the Scrn Window
  return 0;
}
