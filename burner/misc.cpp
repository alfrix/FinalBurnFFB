// Misc functions module
#include "app.h"

// Set the current directory to be the application's directory
int AppDirectory()
{
  char *szCmd=NULL,*pc=NULL; int nPathLen=0; char *szPath=NULL;
  szCmd=GetCommandLine();

  // Find the end of the "c:\directory\program.exe" bit
  if (szCmd[0]!='\"') return 1; // Error: it didn't start with a "

  for (pc=szCmd+1; *pc; pc++)
    if (*pc=='\"') break; // Find the last "

  // Find the last \ or /
  for (          ; pc>=szCmd+1; pc--)
  {
    if (*pc=='\\') break;
    if (*pc=='/') break;
  }

  nPathLen=pc-(szCmd+1);
  if (nPathLen<=0) return 1; // No path

  // Now copy the path into a new buffer
  szPath=(char *)malloc(nPathLen+1); if (szPath==NULL) return 1;
  memcpy(szPath,szCmd+1,nPathLen); szPath[nPathLen]=0;
  SetCurrentDirectory(szPath); // Finally set the current directory to be the application's directory
  free(szPath);
  return 0;
}

// This function takes a rectangle and scales it to a multiple of the
// screen resolution, so that the video hardware doesn't have to do a fractional scale
// (which looks bad in DirectDraw).
int IntegerScale(RECT *pRect,int ScrnWidth,int ScrnHeight)
{
  int xm=0,ym=0; // The multiple of ScrnWidth and ScrnHeight we can fit in
  int m=0; // The multiple of both we can fit in
  int Width=0,Height;
  int FitWidth=0,FitHeight=0;
  Width =pRect->right-pRect->left;
  Height=pRect->bottom-pRect->top;

  xm=Width/ScrnWidth; ym=Height/ScrnHeight;
  m=xm; if (m>ym) m=ym;

  if (m>=1) { FitWidth=ScrnWidth*xm; FitHeight=ScrnHeight*ym; }
  else { FitWidth=Width; FitHeight=Height; }
  
  if (Width== 640 && Height==480 && ScrnWidth==384) FitWidth=640; // 597 for exact 4/3
  if (Width== 512 && Height==384 && ScrnWidth==224) FitWidth=288; // exact 4/3
  if (Width==1024 && Height==768 && ScrnWidth==224) FitWidth=576; // exact 4/3

  pRect->left=(pRect->right+pRect->left)/2;
  pRect->left-=FitWidth>>1;
  pRect->right=pRect->left+FitWidth;

  pRect->top=(pRect->top+pRect->bottom)/2;
  pRect->top-=FitHeight>>1;
  pRect->bottom=pRect->top+FitHeight;
  return 0;
}


// Get the position of the client area of a window on the screen
int GetClientScreenRect(HWND hWnd,RECT *pRect)
{
  POINT Corner={0,0};
  GetClientRect(hWnd,pRect);
  Corner.x=0; Corner.y=0;
  ClientToScreen(hWnd,&Corner);
  pRect->left+=Corner.x; pRect->right+=Corner.x;
  pRect->top+=Corner.y; pRect->bottom+=Corner.y;
  return 0;
}

// Standard callbacks for 8/16/24/32 bit color:
static unsigned int HighCol8 (int r,int g,int b,int i)
{
  unsigned int t; (void)i;
  t=(r+g+b)/3;
  return t;
}

static unsigned int HighCol15(int r,int g,int b,int i)
{
  unsigned int t; (void)i;
  t =(r<<7)&0x7c00; // 0rrr rr00 0000 0000
  t|=(g<<2)&0x03e0; // 0000 00gg ggg0 0000
  t|=(b>>3)&0x001f; // 0000 0000 000b bbbb
  return t;
}

static unsigned int HighCol16(int r,int g,int b,int i)
{
  unsigned int t; (void)i;
  t =(r<<8)&0xf800; // rrrr r000 0000 0000
  t|=(g<<3)&0x07e0; // 0000 0ggg ggg0 0000
  t|=(b>>3)&0x001f; // 0000 0000 000b bbbb
  return t;
}

// 24-bit/32-bit
static unsigned int HighCol24(int r,int g,int b,int i)
{
  unsigned int t; (void)i;
  t =(r<<16)&0xff0000;
  t|=(g<<8 )&0x00ff00;
  t|=(b    )&0x0000ff;

  return t;
}

int SetBurnHighCol(int Depth)
{
  if (Depth<= 8) { BurnHighCol=HighCol8;  return 0; }
  if (Depth<=15) { BurnHighCol=HighCol15; return 0; }
  if (Depth<=16) { BurnHighCol=HighCol16; return 0; }
  BurnHighCol=HighCol24;
  return 0;
}

// Put a window in the middle of another window
int WndInMid(HWND hMid,HWND hBase)
{
  RECT MidRect={0,0,0,0};  int mw=0,mh=0;
  RECT BaseRect={0,0,0,0}; int bx=0,by=0;
  // Find the height and width of the Mid window
  GetWindowRect(hMid,&MidRect);
  mw=MidRect.right-MidRect.left;
  mh=MidRect.bottom-MidRect.top;

  // Find the center of the Base window
  GetWindowRect(hBase,&BaseRect);
  bx=BaseRect.left+BaseRect.right; bx>>=1;
  by=BaseRect.top+BaseRect.bottom; by>>=1;

  // Center the window
  SetWindowPos(hMid,NULL,bx-(mw>>1),by-(mh>>1),0,0,SWP_NOSIZE | SWP_NOZORDER);
  return 0;
}

static char *KeyName[]=
{
  // 0x00:
  "","Left Mouse","Right Mouse","Cancel", "Middle Mouse","","","",
  "Backspace","Tab","","", "Clear","Enter", "","",
  // 0x10
  "Shift","Control","Menu","Pause", "Caps Lock","","","",
  "","","","Escape", "","","","",
  // 0x20
  "Space","Page Up","Page Down","End", "Home","Left","Up","Right",
  "Down","","","", "Print Scrn","Insert","Delete","",
  // 0x30
  "0","1","2","3", "4","5","6","7",
  "8","9","","",  "","","","",
  // 0x40
  "", "A","B","C", "D","E","F","G",
  "H","I","J","K", "L","M","N","O",
  // 0x50
  "P","Q","R","S", "T","U","V","W",
  "X","Y","Z","Left Win", "Right Win","App Menu","","",
  // 0x60
  "NumPad 0","NumPad 1","NumPad 2","NumPad 3", "NumPad 4","NumPad 5","NumPad 6","NumPad 7",
  "NumPad 8","NumPad 9","NumPad *","NumPad +", "","NumPad -","NumPad .","NumPad /",
  // 0x70
  "F1","F2","F3","F4", "F5","F6","F7","F8",
  "F9","F10","F11","F12", "","","","",
  // 0x80
  "","","","","","","","", "","","","","","","","",
  // 0x90
  "Num Lock","Scroll Lock"
};

// Print a nice name for input code c
char *InputCodeDesc(int c)
{
  static char String[64];
  char *Name="";
  if (c>=0x4000)
  {
    int Joy=0,Code=0;
    Joy=(c>>8)&0x3f;
    Code=c&0xff;
    if (Code>=0x10)
    {
      sprintf (String,"Joy-%d Button %d",Joy+1,Code-0x0f);
      return String;
    }
    if (Code>=1 && Code<=4)
    {
      static char Dir[4][8]={"Up","Down","Left","Right"};
      sprintf (String,"Joy-%d %s",Joy+1,Dir[Code-1]);
      return String;
    }
  }

  // Try and get a name for the key code
  if (c < sizeof(KeyName)/sizeof(KeyName[0])) Name=KeyName[c]; // Get from the table of first few keys

  if (Name[0]) sprintf (String,"%s",Name);
  else sprintf (String,"Key 0x%.2x",c);
  return String;
}
