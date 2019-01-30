// Driver Selector module
#include "app.h"

static HWND hSelDlg=NULL;
static HWND hSelList=NULL;
static int nDialogSelect=0; // The driver which this dialog selected

// Make a list view of the available drivers
static int SelListMake()
{
  LVCOLUMN LvCol;
  unsigned int i;
  int nOldSelect=0;

  if (hSelList==NULL) return 1;
  // Full row select style:
  SendMessage(hSelList,LVM_SETEXTENDEDLISTVIEWSTYLE,0,LVS_EX_FULLROWSELECT);

  // Make columns
  memset(&LvCol,0,sizeof(LvCol));
  LvCol.mask=LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;
  LvCol.cx=0xd0;
  LvCol.pszText="Game";
  SendMessage(hSelList,LVM_INSERTCOLUMN,0,(LPARAM)&LvCol);

  LvCol.cx=0xb0;
  LvCol.pszText="Notes";
  SendMessage(hSelList,LVM_INSERTCOLUMN,1,(LPARAM)&LvCol);

  nOldSelect=nBurnDrvSelect; // preserve the currently selected driver
  // Add all the driver names to the list
  for (i=0;i<nBurnDrvCount;i++)
  {
    LVITEM LvItem;
    nBurnDrvSelect=i; // Switch to driver i
    
    memset(&LvItem,0,sizeof(LvItem));
    LvItem.mask=LVIF_TEXT;
    LvItem.iItem=i;
    LvItem.iSubItem=0;
    LvItem.pszText=BurnDrvText(1);

    SendMessage(hSelList,LVM_INSERTITEM,0,(LPARAM)&LvItem);

    LvItem.iSubItem=1;
    LvItem.pszText=BurnDrvText(2);
    SendMessage(hSelList,LVM_SETITEM,0,(LPARAM)&LvItem);

  }
  nBurnDrvSelect=nOldSelect;

  return 0;
}

// Find which number is selected
static unsigned int SelListNum()
{
  return SendMessage(hSelList,LVM_GETNEXTITEM,~0U,LVNI_SELECTED);
}

static void SelOkay()
{
  // User clicked ok for a driver in the list
  unsigned int nSelect=0;
  nSelect=SelListNum();
  // Clip to available drivers
  if (nSelect>=nBurnDrvCount) nSelect=0;
  nDialogSelect=nSelect;
  EndDialog(hSelDlg,0);
}

static BOOL CALLBACK DialogProc(HWND hDlg,UINT Msg,WPARAM wParam,LPARAM lParam)
{
  if (Msg==WM_INITDIALOG)
  {
    hSelDlg=hDlg;
    SetWindowText(hSelDlg,APP_TITLE " - Select Game");
    hSelList=GetDlgItem(hSelDlg,IDC_SEL_LIST);
    SelListMake();
    return 1;
  }

  if (Msg==WM_COMMAND && HIWORD(wParam)==BN_CLICKED)
  {
    int wID=LOWORD(wParam);
    if (wID==IDOK) SelOkay();
    if (wID==IDCANCEL) SendMessage(hDlg,WM_CLOSE,0,0); // cancel=close
  }

  if (Msg==WM_CLOSE) { nDialogSelect=-1; EndDialog(hDlg,0); }

  if (Msg==WM_NOTIFY)
  {
    NMHDR *pNmHdr; pNmHdr=(NMHDR *)lParam;
    if (pNmHdr->code==LVN_ITEMACTIVATE) SelOkay();
  }
  return 0;
}

int SelDialog() // Choose a Burn driver
{
  int nRet=0;
  nRet=DialogBox(hAppInst,MAKEINTRESOURCE(IDD_SEL),hScrnWnd,DialogProc);
  hSelDlg=NULL; hSelList=NULL;
  return nDialogSelect;
}

