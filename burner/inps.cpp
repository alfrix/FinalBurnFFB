// Burner Input Set dialog module
#include "app.h"

HWND hInpsDlg=NULL; // Handle to the Input Set Dialog
static HWND hPush=NULL; // Handle to the Push button
static HWND hKey=NULL; // Handle to the text
unsigned int nInpsInput=0; // The input number we are redefining
static struct BurnInputInfo bii; // Info about the input
static int nDlgState=0;
static int nInputCode=-1; // If in state 3, code N was nInputCode 
static int nCounter=0; // Counter of frames since the dialog was made
static struct GameInp *pgi=NULL; // Current GameInp
static struct GameInp OldInp; // Old GameInp
static int bOldPush=0; // 1 if the push button was pressed last time

static int InpsInit()
{
  char szTitle[128];
  memset(&OldInp,0,sizeof(OldInp));

  hPush=GetDlgItem(hInpsDlg,IDC_INPS_PUSH);
  hKey =GetDlgItem(hInpsDlg,IDC_INPS_KEY);

  // Get the extra info about the input
  memset(&bii,0,sizeof(bii));
  BurnDrvGetInputInfo(&bii,nInpsInput);
  if (bii.szName==NULL) bii.szName="";

  // Set the dialog title
  sprintf(szTitle,"Press a key for %.100s",bii.szName);
  SetWindowText(hInpsDlg,szTitle);

  nDlgState=1; nInputCode=-1; nCounter=0;

  pgi=NULL;
  if (nInpsInput>=nGameInpCount) return 1; // input out of range
  pgi=GameInp+nInpsInput;

  OldInp=*pgi;
  bOldPush=0;
  return 0;
}

static int InpsExit()
{
  bOldPush=0;
  if (pgi!=NULL) *pgi=OldInp;
  memset(&OldInp,0,sizeof(OldInp));

  nDlgState=0;
  hKey=NULL;
  hPush=NULL;
  hInpsDlg=NULL;
  return 0;
}

static int SetInput(int nCode)
{
  if (bii.nType==2) { DestroyWindow(hInpsDlg); return 0; } // Don't change dip switches!

  pgi->nInput=2;
  pgi->nCode=(unsigned short)nCode;
  OldInp=*pgi;

  InpdListMake(0); // Update list with new input type
  return 0;
}

static int InpsPushUpdate()
{
  int nPushState=0;
  if (pgi==NULL) return 1;
  // See if the push button is pressed
  nPushState=SendMessage(hPush,BM_GETSTATE,0,0);
  if (nPushState&BST_PUSHED) nPushState=1; else nPushState=0;

  if (nPushState)
  {
    // Change to constant 1
    pgi->nInput=1;
    pgi->nConst=1;
  }
  else
  {
    // Change back
    *pgi=OldInp;
  }
  if (nPushState!=bOldPush) InpdListMake(0); // refresh view
  bOldPush=nPushState;
  return 0;
}

int InpsUpdate()
{
  char szTemp[128]=""; int nFind=-1;

  if (hInpsDlg==NULL) return 1; // Don't do anything if the dialog isn't created
  if (GetActiveWindow()!=hInpsDlg) return 1; // Dialog not in foreground

  if (nCounter<0x100000) nCounter++; // advance frames since dialog was created
  DirInputStart(); // Poll joysticks etc

  InpsPushUpdate();

  if (nDlgState==1) // 1=Waiting for all keys to be released
  {
    nFind=DirInputFind();
    if (nFind>=0)
    {
      // All keys haven't been released yet
      if (nCounter>=60 && (nCounter&15)==0 )
      {
        // Alert the user that a key is stuck
        sprintf (szTemp,"Waiting for %.80s to be released...",InputCodeDesc(nFind));
        SetWindowText(hKey,szTemp);
      }
      return 0;
    }
    // All keys released
    SetWindowText(hKey,"");
    nDlgState=2;
    return 0;
  }

  if (nDlgState==2) // 2=Waiting for key N for the control
  {
    nFind=DirInputFind();
    if (nFind<0) return 0; // Still waiting

    nInputCode=nFind;
    sprintf (szTemp,"%s",InputCodeDesc(nInputCode));
    SetWindowText(hKey,szTemp);
    nDlgState=3;
    return 0;
  }

  if (nDlgState==3)  // 3=waiting for key N to be released
  {
    int s=0;
    s=DirInputState(nInputCode);
    if (s) return 0; // Still waiting for release
    
    // Key released
    SetInput(nInputCode);
    nDlgState=0;
    DestroyWindow(hInpsDlg); // Quit dialog
    return 0;
  }

  return 0;
}

static BOOL CALLBACK DialogProc(HWND hDlg,UINT Msg,WPARAM wParam,LPARAM lParam)
{
  (void)lParam; (void)wParam; (void)hDlg;
  if (Msg==WM_INITDIALOG) { hInpsDlg=hDlg; InpsInit(); return 1; } // 1 = we didn't set focus?
  if (Msg==WM_CLOSE) { DestroyWindow(hInpsDlg); return 0; }
  if (Msg==WM_DESTROY) { InpsExit(); return 0; }

  if (Msg==WM_COMMAND)
  {
    int Id=LOWORD(wParam); int Notify=HIWORD(wParam);
    if (Id==IDCANCEL && Notify==BN_CLICKED)
    {
      SendMessage(hDlg,WM_CLOSE,0,0); // cancel=close
      return 0;
    }
  }
  return 0;
}

int InpsCreate()
{
  DestroyWindow(hInpsDlg); hInpsDlg=NULL; // Make sure exited

  hInpsDlg=CreateDialog(hAppInst,MAKEINTRESOURCE(IDD_INPS),hInpdDlg,DialogProc);
  if (hInpsDlg==NULL) return 1;

  WndInMid(hInpsDlg,hInpdDlg);
  ShowWindow(hInpsDlg,SW_NORMAL);
  return 0;
}
