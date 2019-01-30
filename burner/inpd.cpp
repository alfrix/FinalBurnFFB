// Burner Input Dialog module
#include "app.h"

HWND hInpdDlg=NULL; // Handle to the Input Dialog
static HWND hInpdList=NULL;
static unsigned char *LastVal=NULL; int bLastValDefined=0; // Last input values/defined

static HWND hInpdGi=NULL,hInpdPci=NULL; // Combo boxes

// Update which input is using which PC input
static int InpdUseUpdate()
{
  unsigned int i=0; struct GameInp *pgi=NULL;
  if (hInpdList==NULL) return 1;

  // Update the values of all the inputs
  for (i=0,pgi=GameInp; i<nGameInpCount; i++,pgi++)
  {
    LVITEM LvItem;
    char *pszVal=NULL;

    pszVal=InpToDesc(pgi);

    memset(&LvItem,0,sizeof(LvItem));
    LvItem.mask=LVIF_TEXT;
    LvItem.iItem=i;
    LvItem.iSubItem=1;
    LvItem.pszText=pszVal;

    SendMessage(hInpdList,LVM_SETITEM,0,(LPARAM)&LvItem);
  }
  return 0;
}

int InpdUpdate()
{
  unsigned int i=0; struct GameInp *pgi=NULL; unsigned char *plv=NULL;
  if (hInpdList==NULL) return 1;
  if (LastVal  ==NULL) return 1;

  // Update the values of all the inputs
  for (i=0,pgi=GameInp,plv=LastVal; i<nGameInpCount; i++,pgi++,plv++)
  {
    LVITEM LvItem;
    char szVal[16];
    unsigned char nThisVal=0;

    nThisVal=*(pgi->pVal);

    if (bLastValDefined && nThisVal==*plv) continue; // hasn't changed

    // Has changed (or not defined yet)
    *plv=nThisVal; // Update LastVal

    if (pgi->nType==0)
    {
      if (nThisVal==0) szVal[0]=0;
      else if (nThisVal==1) strcpy(szVal,"ON");
      else sprintf(szVal,"%d",nThisVal);
    }
    else
    {
      sprintf(szVal,"0x%.2x",nThisVal);
    }

    memset(&LvItem,0,sizeof(LvItem));
    LvItem.mask=LVIF_TEXT;
    LvItem.iItem=i;
    LvItem.iSubItem=2;
    LvItem.pszText=szVal;

    SendMessage(hInpdList,LVM_SETITEM,0,(LPARAM)&LvItem);
  }

  bLastValDefined=1; // LastVal is now defined

  return 0;
}

static int InpdListBegin()
{
  LVCOLUMN LvCol;
  if (hInpdList==NULL) return 1;
  // Full row select style:
  SendMessage(hInpdList,LVM_SETEXTENDEDLISTVIEWSTYLE,0,LVS_EX_FULLROWSELECT);

  // Make column headers
  memset(&LvCol,0,sizeof(LvCol));
  LvCol.mask=LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;
  LvCol.cx=0x60;
  LvCol.pszText="Game input";
  SendMessage(hInpdList,LVM_INSERTCOLUMN,0,(LPARAM)&LvCol);

  LvCol.cx=0x60;
  LvCol.pszText="Key to use";
  SendMessage(hInpdList,LVM_INSERTCOLUMN,1,(LPARAM)&LvCol);

  LvCol.cx=0x30;
  LvCol.pszText="";
  SendMessage(hInpdList,LVM_INSERTCOLUMN,2,(LPARAM)&LvCol);
  return 0;
}

// Make a list view of the game inputs
int InpdListMake(int bBuild)
{
  unsigned int i=0;
  if (hInpdList==NULL) return 1;

  bLastValDefined=0;
  if (bBuild) SendMessage(hInpdList,LVM_DELETEALLITEMS,0,0);
  // Add all the input names to the list
  for (i=0;i<nGameInpCount;i++)
  {
    struct BurnInputInfo bii;
    LVITEM LvItem;

    // Get the name of the input
    bii.szName=NULL; BurnDrvGetInputInfo(&bii,i); if (bii.szName==NULL) bii.szName="";
    
    memset(&LvItem,0,sizeof(LvItem));
    LvItem.mask=LVIF_TEXT;
    LvItem.iItem=i;
    LvItem.iSubItem=0;
    LvItem.pszText=bii.szName;

    SendMessage(hInpdList,bBuild?LVM_INSERTITEM:LVM_SETITEM,0,(LPARAM)&LvItem);
  }

  InpdUseUpdate();
  return 0;
}

static int InpdInit()
{
  int nMemLen=0;
  hInpdList=GetDlgItem(hInpdDlg,IDC_INPD_LIST);

  // Allocate a last val array for the last input values
  nMemLen=nGameInpCount*sizeof(unsigned char);
  LastVal=(unsigned char *)malloc(nMemLen);
  if (LastVal==NULL) return 1;
  memset(LastVal,0,nMemLen);

  InpdListBegin();
  InpdListMake(1);

  // Init the Combo boxes
  hInpdGi= GetDlgItem(hInpdDlg,IDC_INPD_GI);
  SendMessage(hInpdGi,CB_ADDSTRING,0,(LPARAM)"Player 1");
  SendMessage(hInpdGi,CB_ADDSTRING,0,(LPARAM)"Player 2");
  SendMessage(hInpdGi,CB_ADDSTRING,0,(LPARAM)"Player 3");
  SendMessage(hInpdGi,CB_ADDSTRING,0,(LPARAM)"Player 4");

  hInpdPci= GetDlgItem(hInpdDlg,IDC_INPD_PCI);
  SendMessage(hInpdPci,CB_ADDSTRING,0,(LPARAM)"Keyboard");
  SendMessage(hInpdPci,CB_ADDSTRING,0,(LPARAM)"Joystick 1");
  SendMessage(hInpdPci,CB_ADDSTRING,0,(LPARAM)"Joystick 2");
  SendMessage(hInpdPci,CB_ADDSTRING,0,(LPARAM)"Joystick 3");
  SendMessage(hInpdPci,CB_ADDSTRING,0,(LPARAM)"---------------");
  SendMessage(hInpdPci,CB_ADDSTRING,0,(LPARAM)"ANALOG:");
  SendMessage(hInpdPci,CB_ADDSTRING,0,(LPARAM)"J1 Absolute");
  SendMessage(hInpdPci,CB_ADDSTRING,0,(LPARAM)"J1 Centering Slider");
  SendMessage(hInpdPci,CB_ADDSTRING,0,(LPARAM)"J1 Normal Slider");
  SendMessage(hInpdPci,CB_ADDSTRING,0,(LPARAM)"Kbd Centering Slider");
  SendMessage(hInpdPci,CB_ADDSTRING,0,(LPARAM)"Kbd Normal Slider");
  return 0;
}

static int InpdExit()
{
  // Exit the Combo boxes
  hInpdPci=NULL;
  hInpdGi=NULL;

  if (LastVal!=NULL) free(LastVal);  LastVal=NULL;
  hInpdList=NULL;
  hInpdDlg=NULL;
  return 0;
}

// Configure some of the game input
static int GameInpConfig(int nPlayer,int nPcDev)
{
  unsigned int i=0; struct GameInp *pgi=NULL;
  for (i=0,pgi=GameInp; i<nGameInpCount; i++,pgi++)
  {
    struct BurnInputInfo bii; char *szi=NULL;
    // Get the extra info about the input
    bii.szInfo=NULL; BurnDrvGetInputInfo(&bii,i);
    if (bii.szInfo==NULL) bii.szInfo="";
    szi=bii.szInfo;

    switch (nPcDev)
    {
      // DIGITAL CONTROLS
      case  0: GamcPlayer(pgi,bii.szInfo,nPlayer,-1); break; // Keyboard
      case  1: GamcPlayer(pgi,bii.szInfo,nPlayer, 0); break; // Joystick 1
      case  2: GamcPlayer(pgi,bii.szInfo,nPlayer, 1); break; // Joystick 2
      case  3: GamcPlayer(pgi,bii.szInfo,nPlayer, 2); break; // Joystick 3

      // ANALOG CONTROLS
      case  6: GamcAnalogJoy(pgi,szi,nPlayer,0,0); break; // J1 Absolute
      case  7: GamcAnalogJoy(pgi,szi,nPlayer,0,1); break; // J1 Centering Slider
      case  8: GamcAnalogJoy(pgi,szi,nPlayer,0,2); break; // J1 Normal Slider
      case  9: GamcAnalogKey(pgi,szi,nPlayer,  1); break; // Kbd Centering Slider
      case 10: GamcAnalogKey(pgi,szi,nPlayer,  2); break; // Kbd Normal Slider
    }

    switch (nPcDev)
    {
      case 6: case 7: case 8: // J1...
        GamcPlayer(pgi,bii.szInfo,nPlayer, 0); // Also set digital controls (for fire buttons)
      break;
      case 9: case 10: // Kbd...
        GamcPlayer(pgi,bii.szInfo,nPlayer,-1);
      break;
    }
  
  }
  return 0;
}

static int ListItemActivate()
{
  struct BurnInputInfo bii; int nSel=0;
  // List item activated
  // Find out which one was activated
  nSel=SendMessage(hInpdList,LVM_GETNEXTITEM,(unsigned)-1,LVNI_SELECTED);
  if (nSel<0) return 1;
  bii.nType=0; BurnDrvGetInputInfo(&bii,nSel);

  DestroyWindow(hInpsDlg); DestroyWindow(hInpcDlg); // Make sure exited
  if (bii.nType==2)
  {
    // Dip switch is a constant - change it
    nInpcInput=nSel;
    InpcCreate();
  }
  else
  {
    // Assign to a key
    nInpsInput=nSel;
    InpsCreate();
  }
  return 0;  
}

static int DeleteInput(unsigned int i)
{
  struct BurnInputInfo bii;
  if (i>=nGameInpCount) return 1; // out of range
  bii.nType=0; BurnDrvGetInputInfo(&bii,i);
  if (bii.nType==2) return 1; // Don't delete dip switches
  GameInp[i].nInput=1; GameInp[i].nConst=0;
  return 0;
}

static int ListItemDelete()
{
  unsigned int nStart=(unsigned)-1; int nRet=0;
  // List item deleted
  // Find out which ones were selected
  
  for (;;)
  {
    nRet=SendMessage(hInpdList,LVM_GETNEXTITEM,nStart,LVNI_SELECTED);
    if (nRet<0) break;
    DeleteInput(nRet); nStart=nRet;
  }

  InpdListMake(0); // refresh view
  return 0;  
}

static BOOL CALLBACK DialogProc(HWND hDlg,UINT Msg,WPARAM wParam,LPARAM lParam)
{
  (void)lParam; (void)wParam; (void)hDlg;
  if (Msg==WM_INITDIALOG) { hInpdDlg=hDlg; InpdInit(); return 1; } // 1 = we didn't set focus?
  if (Msg==WM_CLOSE) { DestroyWindow(hInpdDlg); return 0; }
  if (Msg==WM_DESTROY) { InpdExit(); return 0; }

  if (Msg==WM_COMMAND)
  {
    int Id=LOWORD(wParam); int Notify=HIWORD(wParam);
    
    if (Id==IDOK && Notify==BN_CLICKED) { ListItemActivate(); return 0;  }
    if (Id==IDCANCEL && Notify==BN_CLICKED) { SendMessage(hDlg,WM_CLOSE,0,0); return 0; }  // cancel=close

    if (Id==IDC_INPD_DEFAULT && Notify==BN_CLICKED)
    {
      int nRet=0;
      DSoundStop(); // stop while we display message box
      nRet=MessageBox(
        hDlg,"Reset all controls for this game to defaults?","Reset to Defaults",
        MB_YESNO | MB_DEFBUTTON2 | MB_ICONWARNING);
      DSoundPlay(); // restart sound
      if (nRet!=IDYES) return 0; // don't do it
      GameInpBlank(0);
      GameInpDefault();
      InpdListMake(0); // refresh view
      return 0;
    }

    if (Id==IDC_INPD_PCI && Notify==CBN_SELCHANGE)
    {
      int nGi=0,nPci=0;
      nGi =SendMessage(hInpdGi, CB_GETCURSEL,0,0); if (nGi ==CB_ERR) goto Err;
      nPci=SendMessage(hInpdPci,CB_GETCURSEL,0,0); if (nPci==CB_ERR) goto Err;
      // Configure inputs:
      GameInpConfig(nGi,nPci);

      InpdListMake(0); // refresh view
    Err:
      return 0;
    }
  }

  if (Msg==WM_NOTIFY && lParam!=NULL)
  {
    int Id=LOWORD(wParam); NMHDR *pnm=(NMHDR *)lParam;
    
    if (Id==IDC_INPD_LIST && pnm->code==LVN_ITEMACTIVATE)
    {
      ListItemActivate();
    }
    if (Id==IDC_INPD_LIST && pnm->code==LVN_KEYDOWN)
    {
      NMLVKEYDOWN *pnmkd=(NMLVKEYDOWN *)lParam;
      if (pnmkd->wVKey==VK_DELETE) ListItemDelete();
    }
return 0;
  }

  return 0;
}

int InpdCreate()
{
  if (bDrvOkay==0) return 1; // No game is loaded

  DestroyWindow(hInpdDlg); // Make sure exited

  hInpdDlg=CreateDialog(hAppInst,MAKEINTRESOURCE(IDD_INPD),hScrnWnd,DialogProc);
  if (hInpdDlg==NULL) return 1;

  WndInMid(hInpdDlg,hScrnWnd);
  ShowWindow(hInpdDlg,SW_NORMAL);
  return 0;
}
