// Burn - Drivers module
#include "burnint.h"

// This is our dprintf() filler function, if the application is not printing debug messages
extern "C" int __cdecl BurnDprintfFiller(char *szFormat,...) { (void)szFormat; return 0; }
extern "C" int (__cdecl *dprintf) (char *szFormat,...)=BurnDprintfFiller;

// Version number, written as  vN.NNN  or  vN.NNNpp
//          (0xNNNNpp)

int nBurnVer=0x051700; // Version number of the library

unsigned int nBurnDrvCount=0; // Count of game drivers
unsigned int nBurnDrvSelect=~0U; // Which game driver is selected

// Burn Draw:
unsigned char *pBurnDraw=NULL; // Pointer to correctly sized bitmap
int nBurnPitch=0; // Pitch between each line
int nBurnBpp=0;   // Bytes per pixel

int nBurnSoundRate=0; // sample rate of sound or zero for no sound
int nBurnSoundLen=0;  // length in samples per frame
short *pBurnSoundOut=NULL; // pointer to output buffer
int nBurnSoundChoice=1+2; // &1 play sound effects &2=play music
unsigned char nBurnLayer=0xff; // Can be used externally to select which layers to show

extern struct BurnDriver
  BurnDrvRoyalAscot2,
  BurnDrvSysAltbeast,
  BurnDrvSysShinobi,
  BurnDrvSysGoldnaxe,
  BurnDrvCpsMvsc,
  BurnDrvCpsMsword,
  BurnDrvCpsMercs,
  BurnDrvCps1941,
  BurnDrvCpsForgottn,
  BurnDrvCpsCsclubj,
  BurnDrvCpsStriderj,
  BurnDrvCpsUnsquad,
  BurnDrvCpsKnights,
  BurnDrvCpsSfa3,
  BurnDrvStrkfgtr,
  BurnDrvCpsBatcirj,
  BurnDrvCpsArmwar,
  BurnDrvCpsDdsom,
  BurnDrvCpsSfz2aj,
  BurnDrvGLoc,
  BurnDrvCpsSpf2xj,
  BurnDrvCpsStrider,
  BurnDrvCpsXmvsf,
  BurnDrvCpsCybotsj,
  BurnDrvCps19xx,
  BurnDrvCpsQndream,
  BurnDrvCpsSlam2e,
  BurnDrvCpsXmcota,
  BurnDrvCpsXmcotaj,
  BurnDrvCpsSf2,
  BurnDrvCpsFfight,
  BurnDrvCpsSsf2t,
  BurnDrvCpsSfa2,
  BurnDrvCpsMsh,
  BurnDrvCpsAvsp,
  BurnDrvCpsSsf2,
  BurnDrvCpsDdtod,
  BurnDrvCpsGhouls,
  BurnDrvCpsMegaman,
  BurnDrvCpsVampj,
  BurnDrvCpsCexp,
  BurnDrvCpsVsavj,
  BurnDrvCpsSfzMusicDdsom,
  BurnDrvCpsSfzMusicMshvsf,
  BurnDrvCpsSfzMusicXmvsf,
  BurnDrvCpsSfa,
  BurnDrvCpsGhouls,
  BurnDrvCpsSf2ce,
  BurnDrvCpsSf2t,
  BurnDrvCpsSfzch,
  BurnDrvTmnt,
  BurnDrvRailCha,
  BurnDrvPwrDrift,
  BurnDrvPwrDriftJpn,
  BurnDrvGforce2,
  BurnDrvGforce2Jpn,
  BurnDrvAburner2,
  BurnDrvAburner1,
  BurnDrvThbladej,
  BurnDrvOutrun;

static struct BurnDriver *pDriver[]=
{
// System 16
  &BurnDrvAburner1,
  &BurnDrvAburner2,
  &BurnDrvGforce2,
  &BurnDrvGforce2Jpn,
  &BurnDrvGLoc,
  &BurnDrvOutrun,
  &BurnDrvPwrDrift,
  &BurnDrvPwrDriftJpn,
  &BurnDrvRailCha,
  &BurnDrvRoyalAscot2,
  &BurnDrvStrkfgtr,
  &BurnDrvThbladej,

  &BurnDrvSysAltbeast,
  &BurnDrvSysGoldnaxe,
  &BurnDrvSysShinobi,

// CPS
  &BurnDrvCps1941,
  &BurnDrvCps19xx,
  &BurnDrvCpsAvsp,
  &BurnDrvCpsArmwar,
  &BurnDrvCpsBatcirj,
  &BurnDrvCpsCsclubj,
  &BurnDrvCpsCybotsj,
  &BurnDrvCpsDdsom,
  &BurnDrvCpsDdtod,
  &BurnDrvCpsFfight,
  &BurnDrvCpsForgottn,
  &BurnDrvCpsGhouls,
  &BurnDrvCpsKnights,
  &BurnDrvCpsMegaman,
  &BurnDrvCpsMercs,
  &BurnDrvCpsMsh,
  &BurnDrvCpsMsword,
  &BurnDrvCpsMvsc,
  &BurnDrvCpsQndream,
  &BurnDrvCpsSlam2e,
  &BurnDrvCpsSf2,
  &BurnDrvCpsSf2ce,
  &BurnDrvCpsSf2t,
  &BurnDrvCpsSfa,
  &BurnDrvCpsSfa2,
  &BurnDrvCpsSfa3,
  &BurnDrvCpsSfzch,
  &BurnDrvCpsSfz2aj,
  &BurnDrvCpsSpf2xj,
  &BurnDrvCpsSsf2,
  &BurnDrvCpsSsf2t,
  &BurnDrvCpsStrider,
  &BurnDrvCpsStriderj,
  &BurnDrvCpsUnsquad,
  &BurnDrvCpsVampj,
  &BurnDrvCpsVsavj,
  &BurnDrvCpsXmcota,
  &BurnDrvCpsXmcotaj,
  &BurnDrvCpsXmvsf
};

extern "C" int BurnLibInit()
{
  BurnLibExit(); // Make sure exited
  nBurnDrvCount=sizeof(pDriver)/sizeof(pDriver[0]); // count available drivers
  return 0;
}

extern "C" int BurnLibExit()
{
  nBurnDrvCount=0;
  return 0;
}

static int BurnExtInputFiller(unsigned int i) { (void)i; return 0; }
extern "C" int (*BurnExtInput) (unsigned int i) = BurnExtInputFiller;  // App-defined func to get the state of each input

// Static functions which forward to each driver's data and functions

// Get the text fields for the driver
extern "C" char *BurnDrvText(unsigned int i)
{
  // Limit to the available fields
  if (i>=sizeof(pDriver[0]->szText)/sizeof(pDriver[0]->szText[0])) return "";
  return pDriver[nBurnDrvSelect]->szText[i];
}

// Get the zip names for the driver
extern "C" int BurnDrvGetZipName(char **pszName,unsigned int i)
{ return pDriver[nBurnDrvSelect]->GetZipName(pszName,i); } // Forward to drivers function

extern "C" int BurnDrvGetRomInfo(struct BurnRomInfo *pri,unsigned int i)
{ return pDriver[nBurnDrvSelect]->GetRomInfo(pri,i); } // Forward to drivers function

extern "C" int BurnDrvGetRomName(char **pszName,unsigned int i,int nAka)
{ return pDriver[nBurnDrvSelect]->GetRomName(pszName,i,nAka); } // Forward to drivers function

extern "C" int BurnDrvGetInputInfo(struct BurnInputInfo *pii,unsigned int i)
{ return pDriver[nBurnDrvSelect]->GetInputInfo(pii,i); } // Forward to drivers function

// Get the screen size
extern "C" int BurnDrvGetScreen(int *pnWidth,int *pnHeight)
{
  *pnWidth =pDriver[nBurnDrvSelect]->nWidth;
  *pnHeight=pDriver[nBurnDrvSelect]->nHeight;
  return 0;
}

// Init game emulation (loading any needed roms)
extern "C" int BurnDrvInit()
{
  if (nBurnDrvSelect>=nBurnDrvCount) return 1;
  return pDriver[nBurnDrvSelect]->Init(); // Forward to drivers function
} 

// Exit game emulation
extern "C" int BurnDrvExit()
{ return pDriver[nBurnDrvSelect]->Exit(); } // Forward to drivers function

// Do one frame of game emulation
extern "C" int BurnDrvFrame()
{ return pDriver[nBurnDrvSelect]->Frame(); } // Forward to drivers function

// Refresh Palette
extern "C" int BurnRecalcPal()
{
  unsigned char *pr;
  pr=pDriver[nBurnDrvSelect]->pRecalcPal;
  if (pr==NULL) return 1;
  *pr=1; // Signal for the driver to refresh it's palette
  return 0;
}

int BurnClearSize(int w,int h)
{
  unsigned char *pl; int y;
  // clear the screen to zero
  for (pl=pBurnDraw,y=0; y<h; pl+=nBurnPitch,y++)
    memset(pl,0x00,w*nBurnBpp);
  return 0;
}

int BurnClearScreen()
{
  struct BurnDriver *pbd;
  pbd=pDriver[nBurnDrvSelect];
  BurnClearSize(pbd->nWidth,pbd->nHeight);
  return 0;
}

// Byteswaps an area of memory
int BurnByteswap(unsigned char *pm,int nLen)
{
  int i=0;
  nLen>>=1;
  for (i=0;i<nLen;i++,pm+=2)
  {
    unsigned char t;
    t=pm[0]; pm[0]=pm[1]; pm[1]=t;
  }
  return 0;
}

static unsigned int (BurnHighColFiller) (int r,int g,int b,int i)
{
  (void)r; (void)g; (void)b; (void)i;
  if (g) return 0xffffffff; else return 0;
}

// Callback used to convert truecolor colors into output (highcol) format
extern "C" unsigned int (*BurnHighCol) (int r,int g,int b,int i) = BurnHighColFiller;

// Area (Memory) scanning
int BurnAreaScan(int nAction,int *pnMin) // &1=for reading &2=for writing &4=Volatile &8=Non-Volatile
{
  // Forward to drivers function
  int (*pas) (int nAction,int *pnMin);
  pas=pDriver[nBurnDrvSelect]->AreaScan;
  if (pas!=NULL) return pas(nAction,pnMin);
  return 1;
}

static int DefAcb (struct BurnArea *pba) { (void)pba; return 1; }
int (*BurnAcb) (struct BurnArea *pba)=DefAcb; // Area callback

// Function: return &1 if sound code is music
int (*BurnScode) (unsigned int n)=NULL;
int BurnScodePass(unsigned int n)
{
  int nType;
  if (BurnScode==NULL) return 1; // pass all codes
  // Find out if it's a sound effect or music
  nType=BurnScode(n); nType&=3;
  if (nType)
  {
    if ((nType&nBurnSoundChoice)==0) return 0; // user doesn't want this code passed
  }
  return 1;
}
