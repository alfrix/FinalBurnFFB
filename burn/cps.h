// CPS ----------------------------------

// cps.cpp
extern int Cps; // 1==CPS1 2==CPS2
extern int nCpsCycles; // Cycles per frame
extern unsigned char *CpsGfx;  extern unsigned int nCpsGfxLen;  // All the graphics
extern unsigned char *CpsRom;  extern unsigned int nCpsRomLen;  // Program Rom (as in rom)
extern unsigned char *CpsCode; extern unsigned int nCpsCodeLen; // Program Rom (decrypted)
extern unsigned char *CpsZRom; extern unsigned int nCpsZRomLen; // Z80 Roms
extern          char *CpsQSam; extern unsigned int nCpsQSamLen; // QSound Sample Roms
extern unsigned char *CpsAd;   extern unsigned int nCpsAdLen;   // ADPCM Data
extern int CpsId[];    // Board ID
extern int CpsMult[];  // Multiply ports
extern int CpsCalc[];  // Calc values
extern int CpsLcReg;   // Address of layer controller register
extern int CpsPmReg[]; // CPS1 priority mask registers
extern int CpsLayEn[]; // bits for layer enable
extern int (*CpsFind0)(int t); // Sprites
extern int (*CpsFind1)(int t); // Scroll 1
extern int (*CpsFind2)(int t); // Scroll 2
extern int (*CpsFind3)(int t); // Scroll 3

int CpsInit();
int CpsExit();
int CpsLoadTiles(unsigned char *Tile,int nStart);
int CpsLoadTilesByte(unsigned char *Tile,int nStart);
int Cps2LoadTiles(unsigned char *Tile,int nStart);

// cps_rot.cpp
int RotInit();
int RotExit();
int RotStart();
int RotStop();

// cps_pal.cpp
extern unsigned int *CpsPal; // Hicolor version of palette
int CpsPalInit();
int CpsPalExit();
int CpsPalUpdate(unsigned char *pNewPal,int bRecalcAll);

// cps_mem.cpp
extern unsigned char *CpsRam90;
extern unsigned char *CpsZRamC0,*CpsZRamF0;
extern unsigned char *CpsSavePal,*CpsSaveReg;
extern unsigned char *CpsRam708,*CpsReg,*CpsFrg;
extern unsigned char *CpsRamFF;
int CpsMemInit();
int CpsMemExit();
int CpsAreaScan(int nAction,int *pnMin);
int CpsScodePoll();

// cps_run.cpp
int CpsRunInit();
int CpsRunExit();
int CpsRunReset();
void CpsGetPalette(int Start,int Count);
int CpsFrame();
static INLINE unsigned char *CpsFindGfxRam(int nAddr,int nLen)
{
  nAddr&=0xffffff; // 24-bit bus
  if (nAddr>=0x900000 && nAddr+nLen<=0x930000) return CpsRam90+nAddr-0x900000;
  return NULL;
}

// cps_rw.cpp
#define CPSINPSET INP(000) INP(001) INP(010) INP(011) INP(018) INP(020) INP(021) INP(177)
extern unsigned char CpsReset;
extern unsigned char Cpi01A,Cpi01C,Cpi01E;
extern int CpsDial[2]; // Forgotton Worlds Dial

// prototype for input bits
#define INP(nnn) extern unsigned char CpsInp##nnn[8];
CPSINPSET
#undef  INP

int CpsRwInit();
int CpsRwExit();
int CpsRwGetInp();
unsigned char CpsReadByte(unsigned int a);
void CpsWriteByte(unsigned int a,unsigned char d);
unsigned short CpsReadWord(unsigned int a);
void CpsWriteWord(unsigned int a,unsigned short d);

// cps_draw.cpp
extern unsigned char CpsRecalcPal; // Flag - If it is 1, recalc the whole palette
extern int CpsLcReg; // Address of layer controller register
extern int CpsLayEn[4]; // bits for layer enable
int CpsDraw();

// cps_eep.cpp
int CpsEepromBlank();
int CpsEepromReset();
void CpsEepromWrite(int b);
int CpsEepromRead();
int CpsEepromScan();

// qs.cpp
int QsndInit();
int QsndExit();
int QsndSectRun(int nStart,int nEnd);
int QsndScan(int nAction);

// qs_z.cpp
int QsndZInit();
int QsndZExit();
int QsndZScan(int nAction);

// qs_c.cpp
extern int nQscRate;
int QscReset();
int QscScan(int nAction);
int QscDispInfo(int *pbKey,unsigned int *pnVol,int *pbDidKeyOn,int i);
void QscWrite(int a,int d);
int QscUpdate(short *Dest,int nLen);

// cpst.cpp
extern unsigned int *CpstPal;
extern unsigned int nCpstType; extern int nCpstX,nCpstY;
extern unsigned int nCpstTile; extern int nCpstFlip;
extern short *CpstRowShift;
extern unsigned int CpstPmsk; // Pixel mask
int CpsBgHigh(int a);

static INLINE void CpstSetPal(int nPal)
{ nPal<<=4; nPal&=0x7f0; CpstPal=CpsPal+nPal; }

int CpstOne();

// ctv.cpp
extern unsigned int nCtvRollX,nCtvRollY;
extern unsigned char *pCtvTile; // Pointer to tile data
extern int nCtvTileAdd; // Amount to add after each tile line
extern unsigned char *pCtvLine; // Pointer to output bitmap
typedef int (*CtvDoFn) ();
extern CtvDoFn CtvDoX[];
int CtvReady();

// nCpstType constants
#define CTT_FLIPX (   1)
#define CTT_CARE  (   2)
#define CTT_ROWS  (   4)
#define CTT_8X8   (   0)
#define CTT_16X16 (   8)
#define CTT_32X32 (0x18)
#define CTT_PMSK  (0x20)
// To get size do (nCpstType&24)+8

// cps_obj.cpp
int CpsObjInit();
int CpsObjExit();
int CpsObjGet();
int CpsObjDraw(int nLevelFrom,int nLevelTo);

// cps_scr.cpp
extern int CpsScrHigh; // If 1, draw the 'high' pixels of the background layer
int CpsScr1Draw(unsigned char *Base,int sx,int sy);
int CpsScr3Draw(unsigned char *Base,int sx,int sy);

// cpsr.cpp
extern unsigned char *CpsrBase; // Tile data base
extern int nCpsrScrX,nCpsrScrY; // Basic scroll info
extern unsigned short *CpsrRows; // Row scroll table, 0x400 words long
extern int nCpsrRowStart; // Start of row scroll (can wrap?)

// Information needed to draw a line
struct CpsrLineInfo
{
  int nStart; // 0-0x3ff - where to start drawing tiles from
  int nWidth; // 0-0x400 - width of scroll shifts
  // e.g. for no rowscroll at all, nWidth=0
  int nTileStart; // Range of tiles which are visible onscreen
  int nTileEnd;   // (e.g. 0x20 -> 0x50 , wraps around to 0x10)
  short Rows[16]; // 16 row scroll values for this line
  int nMaxLeft,nMaxRight; // Maximum row shifts left and right
};
extern struct CpsrLineInfo CpsrLineInfo[15];
int CpsrPrepare();

// cpsrd.cpp
int CpsrRender();

// dc_input.cpp
extern unsigned char CpiAllThree[4];
extern struct BurnInputInfo CpsFsi[31];
int CpsFsAllThree();
int CpsFsStopOpposite();

// For exclusive controls
#define ONLY_ONE(nnn,a,b) \
  if (CpsInp##nnn[a] && CpsInp##nnn[b]) { CpsInp##nnn[a]=0; CpsInp##nnn[b]=0; }

// ps.cpp
extern unsigned char PsndIrq,PsndCode,PsndFade; // Irq trigger/Sound code/fade sent to the z80 program
extern int nPsndIrqPeriod;
int PsndInit();
int PsndExit();
int PsndSectRun(int nStart,int nEnd);
int PsndScan(int nAction);

// ps_z.cpp
int PsndZInit();
int PsndZExit();
int PsndZRun(int nWant);
int PsndZScan(int nAction);

// ps_m.cpp
extern int bPsmOkay; // 1 if the module is okay
int PsmInit();
int PsmExit();
int PsmSect(int nStart,int nEnd);

// ps_a.cpp
extern int nPsaRate;
extern int nPsaSpeed;
extern unsigned char nPsaStatus;
int PsaInit();
int PsaExit();
int PsaRender(short *Dest,int nLen);
void PsaWrite(unsigned char d);
