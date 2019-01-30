// Burner Panel dialog module
#include "app.h"

HWND hPanelDlg=NULL; // Handle to the Input Dialog
int bPanelDlgIsShown=0;
int nPanelHeight=0;
static HWND hParent=NULL; 

int PanelShow(int bShow)
{
  // Toggle panel dialog on and off
  if (bShow) { ShowWindow(hPanelDlg,SW_NORMAL); SetFocus(hPanelDlg); }
  else       { ShowWindow(hPanelDlg,SW_HIDE); }
  return 0;
}
static int PanelUpdate()
{
  CheckDlgButton(hPanelDlg,IDC_PANEL_STRETCH,bDtoStretch?BST_CHECKED:BST_UNCHECKED);
  CheckDlgButton(hPanelDlg,IDC_PANEL_TRIPLE,bDtoTripleBuffer?BST_CHECKED:BST_UNCHECKED);
  CheckDlgButton(hPanelDlg,IDC_PANEL_SCAN,  bVidScanlines?BST_CHECKED:BST_UNCHECKED);
  CheckDlgButton(hPanelDlg,IDC_PANEL_SOUND, (nBurnSoundChoice&1)?BST_CHECKED:BST_UNCHECKED);
  CheckDlgButton(hPanelDlg,IDC_PANEL_MUSIC, (nBurnSoundChoice&2)?BST_CHECKED:BST_UNCHECKED);
  return 0;
}

static void ActualShow(int bShow) { ShowCursor(bShow); }

static BOOL CALLBACK DialogProc(HWND hDlg,UINT Msg,WPARAM wParam,LPARAM lParam)
{
  (void)lParam; (void)wParam; (void)hDlg;

  if (Msg==WM_INITDIALOG)
  {
    RECT Panel;
    hParent=GetParent(hDlg); // find our parent
    memset(&Panel,0,sizeof(Panel));
    GetWindowRect(hDlg,&Panel);
    nPanelHeight=Panel.bottom-Panel.top; // calculate our height
    bPanelDlgIsShown=0; // not shown yet
    return 1; // 1 = we didn't set focus?
  }

  // Update the bPanelDlgIsShown flag
  if (Msg==WM_SHOWWINDOW)
  {
    bPanelDlgIsShown=wParam;
    if (bPanelDlgIsShown==0) SetFocus(hParent); // If we are hidden, focus back on our parent
    InvalidateRect(hParent,NULL,1);
    UpdateWindow(hParent);

    ActualShow(bPanelDlgIsShown);
    return 0;
  }

  if (Msg==WM_COMMAND && HIWORD(wParam)==BN_CLICKED)
  {
    int wID=LOWORD(wParam);
    // Hide the window if escape/cancel is pressed
    if (wID==IDCANCEL)
    {
      ShowWindow(hPanelDlg,SW_HIDE);
      return 0;
    }
    if (wID==IDC_PANEL_LOAD)
    {
      int n=0; static int bReEntry=0;
      if (bReEntry) return 0;  bReEntry++;

      n=SelDialog(); // Bring up select dialog to pick a driver

      if (n>=0)
      {
        DSoundStop(); // stop while we load roms
        DrvInit(n);    // Init the game driver
        DSoundPlay(); // restart sound
      }

      bReEntry--;
      return 0;
    }
    if (wID==IDC_PANEL_INPUT)
    {
      // Toggle Input dialog
      if (hInpdDlg==NULL) InpdCreate();
      else DestroyWindow(hInpdDlg);
      return 0;
    }
    if (wID==IDC_PANEL_FULLSCREEN)
    { nVidFullscreen=!nVidFullscreen; PostMessage(NULL,WM_APP+0,0,0); return 0; } // restart video

    if (wID==IDC_PANEL_STRETCH) { bDtoStretch=!bDtoStretch; PanelUpdate(); return 0; }
    if (wID==IDC_PANEL_TRIPLE)  { bDtoTripleBuffer=!bDtoTripleBuffer; PanelUpdate(); return 0; }
    if (wID==IDC_PANEL_SCAN)
    {
      bVidScanlines=!bVidScanlines; PanelUpdate();
      PostMessage(NULL,WM_APP+0,0,0); // restart video
      return 0;
    }

    if (wID==IDC_PANEL_STATE_LOAD) { StatedLoad(); return 0; }
    if (wID==IDC_PANEL_STATE_SAVE) { StatedSave(); return 0; }

    if (wID==IDC_PANEL_SOUND)  { nBurnSoundChoice^=1; PanelUpdate(); return 0; }
    if (wID==IDC_PANEL_MUSIC)  { nBurnSoundChoice^=2; PanelUpdate(); return 0; }

    if (wID==IDC_PANEL_TRAINER)  { ShowWindow(hTrainDlg,SW_NORMAL); return 0; }
  }

  if (Msg==WM_DESTROY)
  {
    if (bPanelDlgIsShown) ActualShow(0);
    bPanelDlgIsShown=0; hPanelDlg=NULL; hParent=NULL;
    return 0;
  }
  return 0;
}

int PanelCreate(HWND hParent)
{
  HWND hWasActive=NULL;

  hWasActive=GetActiveWindow();
  hPanelDlg=CreateDialog(hAppInst,MAKEINTRESOURCE(IDD_PANEL),hParent,DialogProc);
  if (hPanelDlg==NULL) return 1;
  PanelUpdate();

  SetActiveWindow(hWasActive);
  return 0;
}
