// Driver Init module
#include "app.h"

int bDrvOkay=0; // 1 if the Driver has been initted okay, and it's okay to use the BurnDrv functions

char szAppRomPath[256]="roms\\";

static int DoLibInit() // Do Init of Burn library driver
{
  int i=0; int nRet=0;
  static char *szRomName[BZIP_MAX];

  memset(&szRomName,0,sizeof(szRomName));
  for (i=0;i<BZIP_MAX;i++)
  {
    char *szName=NULL; int nRet=0; int nStrLen=0;

    nRet=BurnDrvGetZipName(&szName,i);
    if (nRet!=0) break;

    // Allocate space for each full pathname
    nStrLen=strlen(szName)+strlen(szAppRomPath);
    szRomName[i]=(char *)malloc(nStrLen+1); if (szRomName[i]==NULL) continue;
    sprintf (szRomName[i],"%s%s",szAppRomPath,szName); // Use path plus default zip file name
    // Point bzip to it
    szBzipName[i]=szRomName[i];
  }
  szBzipName[i]=NULL; // Mark end of zip name list

  // Open the zip file(s) and check if they are okay
  BzipOpen();

  // If there is an error with the romset, report it
  if (nBzipError!=0)
  {
    char *szTitle; int nIcon=0,nButton=0; int nRet=0;
    // Make the correct title and icon
    if (nBzipError&1) { nIcon=MB_ICONERROR;   szTitle=APP_TITLE " Error";   }
    else              { nIcon=MB_ICONWARNING; szTitle=APP_TITLE " Warning"; }

    if (nBzipError&0x08) nButton=MB_OK; // no data at all - pretty basic!
    else
    {
      BzipText.Add("\nWould you like more detailed information?\n(For experts only!)\n");
      nButton=MB_DEFBUTTON2|MB_YESNO;
    }

    nRet=MessageBox(hScrnWnd,BzipText.szText,  szTitle,nButton|nIcon);
    if (nRet==IDYES)
    {
      // Give the more detailed information string
      MessageBox(hScrnWnd,BzipDetail.szText,szTitle,MB_OK|nIcon);
    }
  }

  nRet=BurnDrvInit(); // Init the driver

  BzipClose();

  for (i=0;i<BZIP_MAX;i++)
  {
    // Free the names of each pathnames
    if (szRomName[i]!=NULL) free(szRomName[i]); // Free the strings
    szRomName[i]=NULL; szBzipName[i]=NULL; // Blank BZip list
  }
  szBzipName[i]=NULL; // Mark end of zip name list

  if (nRet!=0) return 1;

  return 0;
}

int DrvInit(int nDrvNum)
{
  int nRet=0;
  DrvExit(); // Make sure exited
  
  nBurnDrvSelect=nDrvNum; // set the driver number
  nRet=DoLibInit(); // Init the Burn library's driver

  if (nRet!=0)
  {
    char szTemp[512];

    BurnDrvExit(); // Exit the driver

    ScrnTitle();
    sprintf (szTemp,"There was an error starting '%s'.\n",BurnDrvText(1));
    MessageBox(hScrnWnd,szTemp,APP_TITLE " Error",MB_OK|MB_ICONERROR);
    return 1;
  }

  TrainStart(); // Trainer allocate memory copies

  GameInpInit(); // Init game input
  // Load config for the game
  nRet=ConfigGameLoad(); if (nRet!=0) ConfigGameSave(); // create initial game config file

  // Dip switch values in driver will have been set now:
  GameInpDefault(); // Auto-config any input which is still undefined to defaults
  // (also sets/gets dip switch valeus from driver)

  bDrvOkay=1; // Okay to use the BurnDrv functions

  StatedAuto(0); // Load nv (or full) ram

  ScrnTitle();
  ScrnSize();

  nBurnLayer=0xff; // show all layers

  // Init the video plugin
  nRet=VidInit();
  if (nRet!=0) { AppError("VidInit Failed",0); return 1; }

  // Reset the speed throttling code, so we don't 'jump' after the load
  RunReset();

  return 0;
}

int DrvExit()
{
  VidExit();

  InvalidateRect(hScrnWnd,NULL,1); UpdateWindow(hScrnWnd); // blank scrn window

  DestroyWindow(hInpdDlg); // Make sure the Input Dialog is exited

  TrainStop(); // Trainer deallocate memory copies
  if (nBurnDrvSelect<nBurnDrvCount)
  {
    StatedAuto(1); // Save nv (or full) ram

    // Save config for the game
    if (bDrvOkay) ConfigGameSave(); // Only save config if we initted okay
    GameInpExit(); // Exit game input
    BurnDrvExit(); // Exit the driver
  }

  bDrvOkay=0; // Stop using the BurnDrv functions

  bRunPause=0; // Don't pause when exited

  if (bDSoundOkay)
  {
    // Write silence into the sound buffer on exit, and for drivers which don't use pBurnSoundOut
    memset(DSoundNextSound,0,nDSoundSegLen<<2);
  }

  nBurnDrvSelect=~0U; // no driver selected
  return 0;
}
