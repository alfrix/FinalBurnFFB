// Screen Window
#include "app.h"

static char *szClass="Scrn"; // Window class name
HWND hScrnWnd=NULL; // Handle to the screen window

// The Screen window proc
static LRESULT CALLBACK ScrnProc(HWND hWnd,UINT Msg,WPARAM wParam,LPARAM lParam)
{
  if (Msg==WM_CREATE)
  {
    return 0;
  }

  if (Msg==WM_PAINT)
  {
    VidPaint(1);  // paint and validate
  }

  if (Msg==WM_SETFOCUS)
  {
    if (nVidFullscreen==0)
    {
      if (bPanelDlgIsShown) SetFocus(hPanelDlg); // Focus on the panel instead
    }
    return 0;
  }

  if (Msg==WM_RBUTTONDOWN)
  {
    if (nVidFullscreen==0) PanelShow(bPanelDlgIsShown==0);
    return 0;
  }

  if (Msg==WM_KEYDOWN && wParam==27)
  {
    if (nVidFullscreen==0) PanelShow(bPanelDlgIsShown==0);
  }

  if (Msg==WM_SIZE)
  {
    int nWidth,nHeight;
    nWidth=LOWORD(lParam);
    nHeight=HIWORD(lParam); // Get the size of our window
    MoveWindow(hPanelDlg,0,0,nWidth,nPanelHeight,1); // Size the panel
    return 0;
  }

  if (Msg==WM_CLOSE)
  {
    PostQuitMessage(0);  // Quit the program if the window is closed
    return 0;
  }

  if (Msg==WM_DESTROY)
  {
    // Panel is destroyed automatically
    VidExit(); // Stop using video with the Window
    hScrnWnd=NULL; // Make sure handle is not used again
    return 0;
  }

  if (Msg==WM_ENTERIDLE)
  {
    MSG Message;
    // Modeless dialog is idle
    while (!PeekMessage(&Message,NULL,0,0,PM_NOREMOVE)) RunIdle();
    return 0;
  }

  if (Msg==WM_SYSCOMMAND)
  {
    if (wParam==0x100) write_datfile(0);
    if (wParam==0x101) write_datfile(1);
  }

  return DefWindowProc(hWnd,Msg,wParam,lParam);
}

static HBRUSH hScrnBrush=NULL;

static int ScrnRegister()
{
  ATOM Atom=0;
  WNDCLASSEX WndClassEx;
  // Register the window class
  memset(&WndClassEx,0,sizeof(WndClassEx)); // Init structure to all zeros
  WndClassEx.cbSize=sizeof(WndClassEx);
  WndClassEx.style=CS_HREDRAW|CS_VREDRAW;
  WndClassEx.lpfnWndProc=ScrnProc;
  WndClassEx.hInstance=hAppInst;
  WndClassEx.hIcon=LoadIcon(hAppInst,MAKEINTRESOURCE(IDI_APP));
  WndClassEx.hIconSm=WndClassEx.hIcon;
  WndClassEx.hCursor=LoadCursor(NULL,IDC_ARROW);

  hScrnBrush=CreateSolidBrush(0);
  WndClassEx.hbrBackground=hScrnBrush;
  WndClassEx.lpszClassName=szClass;
  // Register the window class with the above information:
  Atom=RegisterClassEx(&WndClassEx); if (Atom==0) return 1;
  return 0;
}

int ScrnSize()
{
  int x,y,w,h,ew,eh,a,ah;
  int nBmapWidth=320,nBmapHeight=224;
  static RECT WorkArea={0,0,640,480}; // Work area on the desktop
  if (hScrnWnd==NULL) return 1;

  if (bDrvOkay) BurnDrvGetScreen(&nBmapWidth,&nBmapHeight);

  SystemParametersInfo(SPI_GETWORKAREA,0,&WorkArea,0); // Find the size of the visible WorkArea
  if (nVidFullscreen) return 1;

  if (nBmapWidth <=0) return 1;
  if (nBmapHeight<=0) return 1;

  // Find the midpoint for the window
  x=WorkArea.left  +WorkArea.right; x/=2;
  y=WorkArea.bottom+WorkArea.top  ; y/=2;
  // Find the width and height
  w=WorkArea.right -WorkArea.left;
  h=WorkArea.bottom-WorkArea.top;

  // Find out how much space is taken up by the borders
  ew =GetSystemMetrics(SM_CXSIZEFRAME)<<1;
  eh =GetSystemMetrics(SM_CYSIZEFRAME)<<1;
  eh+=GetSystemMetrics(SM_CYCAPTION);
  eh+=nPanelHeight; // for panel

  w-=ew; h-=eh; // Subtract the border space
  a=w/nBmapWidth; ah=h/nBmapHeight; // Find out how much we can zoom along each axis
  if (a>ah) a=ah; // a=minimum of the two axes
  if (a>2) a=2; // limit size
  if (a>0) { w=a*nBmapWidth; h=a*nBmapHeight; } // Scale to an integer size
  w+=ew; h+=eh;

  x-=w/2; y-=h/2; // Re-add the border space
  MoveWindow(hScrnWnd,x,y,w,h,1);
  return 0;
}

int ScrnTitle()
{
  char szWinName[128]="";
  if (bDrvOkay)
  {
    // Create window title
    sprintf (szWinName,"%.70s - " APP_TITLE " v%.20s",BurnDrvText(1),szAppBurnVer);
  }
  else
  {
    sprintf (szWinName, APP_TITLE " v%.20s",szAppBurnVer); // Create window title
  }

  SetWindowText(hScrnWnd,szWinName);
  return 0;
}

static int DatInMenu()
{
  HMENU s=NULL;
  s=GetSystemMenu(hScrnWnd,0);
  // Add functions to write out dats to the System Menu
  InsertMenu(s,5,MF_BYPOSITION|MF_SEPARATOR,0,NULL);
  InsertMenu(s,6,MF_BYPOSITION|MF_STRING,0x100,"Export CMPro &Dat");
  InsertMenu(s,7,MF_BYPOSITION|MF_STRING,0x101,"Export &RomCenter Dat");
  return 0;
}

// Init the screen window (create it)
int ScrnInit()
{
  if (ScrnRegister()!=0) return 1;

  hScrnWnd=CreateWindow(
    szClass,APP_TITLE, nVidFullscreen?WS_POPUP:WS_OVERLAPPEDWINDOW, // style of window
    0,0,0,0, // size of window
    NULL,NULL,hAppInst,NULL);
  if (hScrnWnd==NULL) { ScrnExit(); return 1; }

  ScrnSize();
  ScrnTitle();

  if (nVidFullscreen==0)
  {
    PanelCreate(hScrnWnd); // Create the control panel, hPanelDlg
    PanelShow(1);
    TrainCreate(); // Creat trainer dialog
    DatInMenu();
  }

  return 0;
}

// Exit the screen window (destroy it)
int ScrnExit()
{
  DestroyWindow(hTrainDlg); hTrainDlg=NULL;

  if (hScrnWnd!=NULL) DestroyWindow(hScrnWnd);  hScrnWnd=NULL; // Ensure the window is destroyed
  UnregisterClass(szClass,hAppInst); // Unregister the scrn class
  DeleteObject(hScrnBrush); hScrnBrush=NULL;
  return 0;
}
