#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#pragma warning (push)
#pragma warning (disable:4201)
#include <mmsystem.h>
#pragma warning (pop) 
#define DIRECTINPUT_VERSION 0x0500 // Only need version from DirectX 5
#include <dinput.h>

#include "../burn/burn.h"
#include "resource.h"

// Make the INLINE macro
#undef INLINE
#define INLINE inline

// Macro for releasing a COM object
#define RELEASE(x) { if ((x)!=NULL) (x)->Release();  (x)=NULL; }

#define APP_TITLE "Final Burn"
#define APP_TAKE_INPUT(hActive) ( hActive!=NULL && (hActive==hScrnWnd || hActive==hInpdDlg ))

#define KEY_DOWN(Code) ((GetAsyncKeyState(Code)&0x8000)?1:0)
#define BZIP_MAX (4) // Maximum zip files to search through


// -----------------------------------------------

// dat.cpp
int write_datfile(int nDatType);

// dsp.cpp
int DspInit();
int DspExit();
int DspDo(short *Wave,int nCount);

// main.cpp
extern HINSTANCE hAppInst; // Application Instance
extern int nAppVirtualFps; // virtual fps
extern int nUseSysMemory;
extern int bAppBassFilter;
extern char szAppBurnVer[16];
int AppError(char *szText,int bWarning);
int AppMessage(MSG *pMsg);

// media.cpp
extern HINSTANCE DDrawDll;
int MediaInit();
int MediaExit();

// train.cpp
extern HWND hTrainDlg; // Handle to the Dialog
int TrainCreate();
int TrainStart();
int TrainStop();

// vid.cpp
// video output plugin:
struct VidOut
{
  int (*Init) ();
  int (*Exit) ();
  int (*Frame) ();
  int (*Paint) (int bValidate);
};
extern HWND hVidWnd; // The window to be used for video
extern int VidAuto;
extern int nVidWidth,nVidHeight,nVidDepth,nVidRefresh;
extern int nVidFullscreen;
extern int bVidScanlines;
extern int bVidScanHalf;
extern int bVidScanDelay;
int VidInit();
int VidExit();
int VidFrame();
int VidPaint(int bValidate);

// misc.cpp
int AppDirectory();
int IntegerScale(RECT *pRect,int ScrnWidth,int ScrnHeight);
int GetClientScreenRect(HWND hWnd,RECT *pRect);
int SetBurnHighCol(int Depth);
int WndInMid(HWND hMid,HWND hBase);
char *InputCodeDesc(int c);

// DirectX -------------------------------------

// dinp.cpp
int DirInputInit(HINSTANCE hInst,HWND hPassWnd);
int DirInputExit();
int DirInputStart();
int DirInputState(int Code);
int DirInputFind();
int DirInputAxis(int i,int Axis);

// dsound.cpp
extern int nDSoundSamRate;           // sample rate
extern int nDSoundSegCount;          // Segs in the pdsbLoop buffer
extern int nDSoundSegLen;            // Seg length in samples (calculated from Rate/Fps)
extern int nDSoundFps;               // Application fps
extern short *DSoundNextSound;       // The next sound seg we will add to the sample loop
extern unsigned char bDSoundOkay;    // True if DSound was initted okay
extern unsigned char bDSoundPlaying; // True if the Loop buffer is playing
int DSoundInit(HWND hWnd);
int DSoundExit();
int DSoundPlay();
int DSoundStop();
int DSoundCheck();
extern int (*DSoundGetNextSound) (int bDraw); // Callback used to request more sound

// dto.cpp
extern int bDtoStretch;
extern int bDtoTripleBuffer; // 1 to enable triple buffering

// scrn.cpp
extern HWND hScrnWnd; // Handle to the screen window
int ScrnInit();
int ScrnExit();
int ScrnSize();
int ScrnTitle();

// ---------------------------------------------

// zipfn.cpp
struct ZipEntry { char *szName; int nLen; unsigned int nCrc; };
int ZipOpen(char *szZip);
int ZipClose();
int ZipGetList(struct ZipEntry **pList,int *pnListCount);
int ZipLoadFile(unsigned char *Dest,int nLen,int *pnWrote,int nEntry);

#define ZIP_LIST_FREE(List,nListCount) \
{ if (List!=NULL) { int i; for (i=0;i<nListCount;i++) \
  { if (List[i].szName!=NULL) free(List[i].szName);  List[i].szName=NULL; } \
    free(List); } List=NULL; nListCount=0; }

// stringset.cpp
class StringSet
{
public:
  char *szText; int nLen;
  // printf function to add text to the Bzip string
  int __cdecl Add(char *szFormat,...);
  int Reset();
  StringSet();
  ~StringSet();
};

// bzip.cpp
extern char *szBzipName[BZIP_MAX+1];
extern int nBzipError; // non-zero if there is a problem with the opened romset
extern StringSet BzipText; // Text which describes any problems with loading the zip
extern StringSet BzipDetail; // Text which describes in detail any problems with loading the zip
int BzipOpen();
int BzipClose();
int BzipInit();
int BzipExit();


// drv.cpp
extern int bDrvOkay; // 1 if the Driver has been initted okay, and it's okay to use the BurnDrv functions
extern char szAppRomPath[256];
int DrvInit(int nDrvNum);
int DrvExit();

// run.cpp
extern int bRunPause;
extern int bReadInputEveryFrame;
int RunIdle();
int RunMessageLoop();
int RunReset();

// input.cpp
int InputInit();
int InputExit();
int InputMake();
int InputTick();

// sel.cpp
int SelDialog(); // Choose a Burn driver

// cona.cpp
char *LabelCheck(char *s,char *szLabel);
int ConfigAppLoad();
int ConfigAppSave();

// cong.cpp
int ConfigGameLoad();
int ConfigGameSave();

#define QUOTE_MAX (128) // maximum length of "quoted strings"
int QuoteRead(char **pszQuote,char **pszEnd,char *szSrc); // Read a quoted string from szSrc and point to the end

// gami.cpp
#define SKIP_WS(s) for (;;) { if (*s!=' ' && *s!='\t') break;  s++; } // skip whitespace

struct GameInp
{
  unsigned char *pVal;  // Destination for the Input Value
  unsigned char nType;  // 0=binary (0,1) 1=analog (0x01-0xFF) 2=dip switch
  unsigned char nInput; // 0=undefined, 1=constant value, 2=on/off switch, 3=joystick axis, 4=slider
  unsigned char nConst; // the constant value
  unsigned short nCode; // the input code (for digital)
  unsigned char nJoy;   // the joystick number
  unsigned char nAxis;  // the joystick axis number
  unsigned short nSlider[2];  // Keys to use for slider
  short nSliderSpeed;   // speed which keys move the slider
  int nSliderValue;     // Current position of the slider
  short nSliderCenter;  // Speed the slider should center itself (high value = slow)
};
extern struct GameInp *GameInp;
extern unsigned int nGameInpCount;
int GameInpInit();
int GameInpExit();
char *InpToDesc(struct GameInp *pgi);
int GameInpBlank(int bDipSwitch);
int GameInpDefault();
int GameInpWrite(FILE *h);
int GameInpRead(char *szVal);

// panel.cpp
extern HWND hPanelDlg; // Handle to the Input Dialog
extern int bPanelDlgIsShown;
extern int nPanelHeight;
int PanelShow(int bShow);
int PanelCreate(HWND hParent);

// wave.cpp
extern FILE *WaveLog; // wave log file
int WaveLogStart(char *szName,int nSamRate);
int WaveLogStop();

// text.cpp
int TextInit();
int TextExit();
int TextRender();
int TextCopy();

// gamc.cpp
int GamcMisc(struct GameInp *pgi,char *szi);
int GamcAnalogKey(struct GameInp *pgi,char *szi,int nPlayer,int nSlide);
int GamcAnalogJoy(struct GameInp *pgi,char *szi,int nPlayer,int nJoy,int nSlide);
int GamcPlayer(struct GameInp *pgi,char *szi,int nPlayer,int nDevice);
int GamcPlayerHotRod(struct GameInp *pgi,char *szi,int nPlayer,int nSide);

// inpd.cpp
extern HWND hInpdDlg; // Handle to the Input Dialog
int InpdUpdate();
int InpdCreate();
int InpdListMake(int bBuild);

// inps.cpp
extern HWND hInpsDlg; // Handle to the Input Set Dialog
extern unsigned int nInpsInput; // The input number we are redefining
int InpsCreate();
int InpsUpdate();

// state.cpp
int StateLoad(char *szName,int bAll);
int StateSave(char *szName,int bAll);

// statec.cpp
int StateCompress(unsigned char **pDef,int *pnDefLen,int bAll);
int StateDecompress(unsigned char *Def,int nDefLen,int bAll);

// inpc.cpp
extern HWND hInpcDlg; // Handle to the Input Constant Dialog
extern unsigned int nInpcInput; // The input number we are redefining
int InpcCreate();

// stated.cpp
extern int bDrvSaveAll;
int StatedAuto(int bSave);
int StatedLoad();
int StatedSave();

// force feedback
extern int diMaxForce;
extern int diMinForce;
extern int diForceFreq;