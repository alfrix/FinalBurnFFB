// Trainer dialog
#include "app.h"

HWND hTrainDlg=NULL; // Handle to the Dialog
static int ScanLen=0; // Length we are scanning
static unsigned char *ScanAlive=NULL; // True if the address is still 'in'
static int ScanValue=0;
static int ScanPos=0; // Position we have got to in scan
static int CouldBeCount=0;
static char *CouldText=NULL;

// Get the length of the scan areas
static int TacbGetLen(struct BurnArea *pba)
{
  ScanLen+=pba->nLen;
  return 0;
}

// Add text to the 'could be' list
static int CouldAdd(char *Add)
{
  int TotLen=0,AddLen=0; void *NewMem=NULL;
  if (CouldText!=NULL) { TotLen=strlen(CouldText); }
  AddLen=strlen(Add);
  NewMem=realloc(CouldText,TotLen+AddLen+1);
  if (NewMem==NULL) return 1;
  CouldText=(char *)NewMem;
  strcpy(CouldText+TotLen,Add);
  return 0;
}

// Called when a memory location could be the one we are looking for
static int CouldBe(struct BurnArea *pba,int a,int d)
{
  char Text[128];
  if (CouldBeCount>=256) return 1; // Too many could bes
  sprintf (Text,"%s[0x%.4x]=0x%.2x;\r\n",pba->szName,a,d);
  CouldAdd(Text);
  SetDlgItemText(hTrainDlg,IDC_TRAIN_EDIT,CouldText);
  CouldBeCount++;
  return 0;
}

// Area call back to scan part of the emulator memory for ScanValue
static int TacbScan(struct BurnArea *pba)
{
  int a=0; unsigned char *pd;

  for (a=0,pd=(unsigned char *)pba->Data; a<pba->nLen; a++,pd++,ScanPos++)
  {
    if (ScanPos>=ScanLen) return 1; // Quit on buffer overflow

    if (*pd!=ScanValue)
    {
      ScanAlive[ScanPos]=0; // No match
    }

    if (ScanAlive[ScanPos]) CouldBe(pba,a,*pd);
  }
  return 0;
}

static int TrainReset()
{
  memset(ScanAlive,1,ScanLen); // Could be any location
  return 0;
}

static int DoFind()
{
  int Min=0;
  char Text[32];
  if (ScanAlive==NULL) return 1;

  memset(Text,0,sizeof(Text));
  GetWindowText(GetDlgItem(hTrainDlg,IDC_TRAIN_VAL),Text,sizeof(Text)-1);

  ScanValue=strtol(Text,NULL,0);

  CouldBeCount=0;
  ScanPos=0;
  CouldText=NULL;
  SetDlgItemText(hTrainDlg,IDC_TRAIN_EDIT,"(None)");

  BurnAcb=TacbScan;
  BurnAreaScan(4|1,&Min); // scan volatile ram (read)
  BurnAreaScan(8|1,&Min); // scan nvram (read)

  free(CouldText); CouldText=NULL;
  return 0;
}


static BOOL CALLBACK DialogProc(HWND hDlg,UINT Msg,WPARAM wParam,LPARAM lParam)
{
  int Id=0;
  (void)lParam;

  switch (Msg)
  {
    case WM_INITDIALOG:
    SendMessage(GetDlgItem(hDlg,IDC_TRAIN_EDIT),WM_SETFONT,(WPARAM)GetStockObject(ANSI_FIXED_FONT),0);
    return 1;
    case WM_COMMAND:
    Id=wParam&0xffff;

    switch (Id)
    {
      case IDOK: DoFind(); return 0;
      case IDC_TRAIN_RESET: TrainReset(); return 0;
    }
    return 0;
    case WM_CLOSE: ShowWindow(hDlg,SW_HIDE); return 0;
    case WM_NCDESTROY: hTrainDlg=NULL; return 0; // Blank handle
  }
  return 0;
}

int TrainCreate()
{
  hTrainDlg=CreateDialog(hAppInst,MAKEINTRESOURCE(IDD_TRAIN),hScrnWnd,DialogProc);
  if (hTrainDlg==NULL) return 1;
  SetActiveWindow(hScrnWnd);
  return 0;
}

// Called to allocate memory copies
int TrainStart()
{
  int Min=0;

  ScanLen=0;
  BurnAcb=TacbGetLen;
  BurnAreaScan(4,&Min); // scan volatile ram
  BurnAreaScan(8,&Min); // scan nvram

  ScanAlive=(unsigned char *)malloc(ScanLen);
  if (ScanAlive==NULL) return 1;
  TrainReset();

  return 0;
}

// Called to deallocate memory copies
int TrainStop()
{
  free(ScanAlive); ScanAlive=NULL;
  ScanLen=0;
  return 0;
}
