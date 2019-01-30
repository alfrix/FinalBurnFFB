// State dialog module
#include "app.h"
#include <commdlg.h>

int bDrvSaveAll=0;

static char szChoice[0x400]=""; // File chosen by the user
static OPENFILENAME ofn;

static int MakeOfn()
{
  memset(&ofn,0,sizeof(ofn));
  ofn.lStructSize=sizeof(ofn);
  ofn.hwndOwner=hScrnWnd;
  ofn.lpstrFilter="Final Burn States\0*.fs\0\0";
  ofn.lpstrFile=szChoice;
  ofn.nMaxFile=sizeof(szChoice);
  ofn.lpstrInitialDir=".\\state";
  ofn.Flags=OFN_NOCHANGEDIR | OFN_HIDEREADONLY;
  ofn.lpstrDefExt="fs";
  return 0;
}

// The automatic save
int StatedAuto(int bSave)
{
  int nRet=0; static char szName[32]="";
  sprintf (szName,"cfg\\%.8s.fs",BurnDrvText(0));
  if (bSave==0)
  {
    nRet=StateLoad(szName,bDrvSaveAll); // Load ram
    if (nRet!=0 && bDrvSaveAll) nRet=StateLoad(szName,0); // Couldn't get all - okay just try the nvram
  }
  else
  {
    nRet=StateSave(szName,bDrvSaveAll); // Save ram
  }
  return nRet;
}

int StatedLoad()
{
  int nRet=0; int bOldPause=0;
  if (bDrvOkay==0) return 1;

  sprintf (szChoice,"%.8s*.fs",BurnDrvText(0));
  MakeOfn();
  ofn.lpstrTitle="Load State";

  bOldPause=bRunPause; bRunPause=1;
  nRet=GetOpenFileName(&ofn);
  bRunPause=bOldPause;

  if (nRet==0) return 1; // Error
  nRet=StateLoad(szChoice,1);

  // Describe the error:
       if (nRet==3) { AppError("The save state is for the wrong game.",0); }
  else if (nRet==4) { AppError("This state is too old and cannot be loaded.",0); }
  else if (nRet==5) { AppError("Emulator is too old to load this state.",0); }
  else if (nRet!=0) { AppError("Error loading state",0); }

  return 0;
}

int StatedSave()
{
  int nRet=0; int bOldPause=0;
  if (bDrvOkay==0) return 1;

  sprintf (szChoice,"%.8s",BurnDrvText(0));
  MakeOfn();
  ofn.lpstrTitle="Save State";
  ofn.Flags|=OFN_OVERWRITEPROMPT;

  bOldPause=bRunPause; bRunPause=1;
  nRet=GetSaveFileName(&ofn);
  bRunPause=bOldPause;

  if (nRet==0) return 1; // Error

  nRet=StateSave(szChoice,1);
  if (nRet!=0)
  {
    AppError("Error saving state",1);
  }

  return 0;
}
