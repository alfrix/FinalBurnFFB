#include "burnint.h"
// CPS (general)

int Cps=0; // 1==CPS1 2==CPS2

int nCpsCycles=0; // Cycles per frame
unsigned char *CpsGfx =NULL; unsigned int nCpsGfxLen =0; // All the graphics
unsigned char *CpsRom =NULL; unsigned int nCpsRomLen =0; // Program Rom (as in rom)
unsigned char *CpsCode=NULL; unsigned int nCpsCodeLen=0; // Program Rom (decrypted)
unsigned char *CpsZRom=NULL; unsigned int nCpsZRomLen=0; // Z80 Roms
         char *CpsQSam=NULL; unsigned int nCpsQSamLen=0; // QSound Sample Roms
unsigned char *CpsAd  =NULL; unsigned int nCpsAdLen  =0; // ADPCM Data
int CpsId[2]={0,0}; // Board ID
int CpsMult[4]={0,0,0,0}; // Multiply ports
int CpsCalc[2]={0,0}; // Calc values
int CpsLcReg=0; // Address of layer controller register
int CpsPmReg[4]={0,0,0,0}; // CPS1 priority mask registers
int CpsLayEn[4]={0,0,0,0}; // bits for layer enable

// Tile address find functions
// Small games (<=0x800000 graphics)
static int StdFind02  (int t) { return (t&0x0ffff)<<7; } // Sprites/Scroll 2
static int StdFind1   (int t) { return (t&0x1ffff)<<6; } // Scroll 1
static int StdFind3   (int t) { return (t&0x03fff)<<9; } // Scroll 3
// Big games (>0x800000 graphics)
static int StdBigFind0(int t) { return t<<7; }
static int StdBigFind1(int t) { return (t|0x20000)<<6; }
static int StdBigFind2(int t) { return (t|0x10000)<<7; }
static int StdBigFind3(int t) { return (t|0x04000)<<9; }

int (*CpsFind0)(int t)=StdFind02;
int (*CpsFind1)(int t)=StdFind1;
int (*CpsFind2)(int t)=StdFind02;
int (*CpsFind3)(int t)=StdFind3;

// Separate out the bits of a byte
static INLINE unsigned int Separate(unsigned int b)
{
  unsigned int a;
  a=b;                                   // 00000000 00000000 00000000 11111111
  a=((a&0x000000f0)<<12)|(a&0x0000000f); // 00000000 00001111 00000000 00001111
  a=((a&0x000c000c)<< 6)|(a&0x00030003); // 00000011 00000011 00000011 00000011
  a=((a&0x02020202)<< 3)|(a&0x01010101); // 00010001 00010001 00010001 00010001
  return a;
}
// Precalculated table of the Separate function
static unsigned int SepTable[256];

static int SepTableCalc()
{
  int i=0;
  static int bDone=0;
  if (bDone) return 0; // Already done it
  for (i=0;i<256;i++) SepTable[i]=Separate(i);
  bDone=1; // done it
  return 0;
}

// Allocate space and load up a rom
static int LoadUp(unsigned char **pRom,int *pnRomLen,int nNum)
{
  unsigned char *Rom=NULL; int nRet=0;
  struct BurnRomInfo ri;
  
  ri.nLen=0; BurnDrvGetRomInfo(&ri,nNum); // Find out how big the rom is
  if (ri.nLen<=0) return 1;
  // Load the rom
  Rom=(unsigned char *)malloc(ri.nLen);

  if (Rom==NULL) return 1;
  nRet=BurnLoadRom(Rom,nNum,1);  if (nRet!=0) { free(Rom); return 1; }
  // Success
  *pRom=Rom; *pnRomLen=ri.nLen;
  return 0;
}

// ----------------------------CPS1--------------------------------
// Load 1 rom and interleve in the CPS style:
// rom  : aa bb
// --ba --ba --ba --ba --ba --ba --ba --ba 8 pixels (four bytes)
//                                                  (skip four bytes)

static int CpsLoadOne(unsigned char *Tile,int nNum,int nWord,int nShift)
{
  int i=0;
  unsigned char *Rom=NULL; int nRomLen=0;
  unsigned char *pt=NULL,*pr=NULL;

  LoadUp(&Rom,&nRomLen,nNum); if (Rom==NULL) return 1;

  nRomLen&=~1; // make sure even

  for (i=0,pt=Tile,pr=Rom; i<nRomLen; pt+=8)
  {
    unsigned int Pix; // Eight pixels
    unsigned char b;
    b=*pr++; i++; Pix=SepTable[b];
    if (nWord) { b=*pr++; i++; Pix|=SepTable[b]<<1; }

    Pix<<=nShift;
    *((unsigned int *)pt)|=Pix;
  }

  free(Rom);
  return 0;
}

int CpsLoadTiles(unsigned char *Tile,int nStart)
{
  // left  side of 16x16 tiles
  CpsLoadOne(Tile  , nStart  ,1,0);
  CpsLoadOne(Tile  , nStart+1,1,2);
  // right side of 16x16 tiles
  CpsLoadOne(Tile+4, nStart+2,1,0);
  CpsLoadOne(Tile+4, nStart+3,1,2);
  return 0;
}

int CpsLoadTilesByte(unsigned char *Tile,int nStart)
{
  CpsLoadOne(Tile  ,nStart+ 0,0,0);
  CpsLoadOne(Tile  ,nStart+ 1,0,1);
  CpsLoadOne(Tile  ,nStart+ 2,0,2);
  CpsLoadOne(Tile  ,nStart+ 3,0,3);
  CpsLoadOne(Tile+4,nStart+ 4,0,0);
  CpsLoadOne(Tile+4,nStart+ 5,0,1);
  CpsLoadOne(Tile+4,nStart+ 6,0,2);
  CpsLoadOne(Tile+4,nStart+ 7,0,3);
  return 0;
}

// ----------------------------CPS2--------------------------------
// Load 1 rom and interleve in the CPS2 style:
// rom  : aa bb -- -- (4 bytes)
// --ba --ba --ba --ba --ba --ba --ba --ba 8 pixels (four bytes)
//                                                  (skip four bytes)

//  memory 000000-100000 are in even word fields of first 080000 section
//  memory 100000-200000 are in  odd word fields of first 080000 section
// i=ABCD nnnn nnnn nnnn nnnn n000
// s=00AB Cnnn nnnn nnnn nnnn nnD0

static INLINE void Cps2Load100000(unsigned char *Tile,unsigned char *Sect,int nShift)
{
  unsigned char *pt,*pEnd,*ps;
  pt=Tile; pEnd=Tile+0x100000; ps=Sect;
  
  do
  {
    unsigned int Pix; // Eight pixels
    Pix =SepTable[ps[0]];
    Pix|=SepTable[ps[1]]<<1;
    Pix<<=nShift;
    *((unsigned int *)pt)|=Pix;

    pt+=8; ps+=4;
  }
  while (pt<pEnd);
}

static int Cps2LoadOne(unsigned char *Tile,int nNum,int nShift)
{
  unsigned char *Rom=NULL; int nRomLen=0;
  int b; unsigned char *pt,*pr;

  LoadUp(&Rom,&nRomLen,nNum); if (Rom==NULL) return 1;

  // Go through each section
  pt=Tile; pr=Rom;
  for (b=0;b<nRomLen>>19;b++)
  {
    Cps2Load100000(pt,pr  ,nShift); pt+=0x100000;
    Cps2Load100000(pt,pr+2,nShift); pt+=0x100000;
    pr+=0x80000;
  }

  free(Rom);
  return 0;
}

int Cps2LoadTiles(unsigned char *Tile,int nStart)
{
  // left  side of 16x16 tiles
  Cps2LoadOne(Tile  ,nStart  ,0);
  Cps2LoadOne(Tile  ,nStart+1,2);
  // right side of 16x16 tiles
  Cps2LoadOne(Tile+4,nStart+2,0);
  Cps2LoadOne(Tile+4,nStart+3,2);
  return 0;
}

// ----------------------------------------------------------------

int CpsInit()
{
  int MemLen=0; int i=0;
  if (Cps==1) nCpsCycles= 8000000/60;
  else        nCpsCycles=11800000/60;
  MemLen=nCpsGfxLen+nCpsRomLen+nCpsCodeLen+nCpsZRomLen+nCpsQSamLen+nCpsAdLen;
  // Allocate Gfx, Rom and Z80 Roms
  CpsGfx=(unsigned char *)malloc(MemLen);
  if (CpsGfx==NULL) return 1;
  memset(CpsGfx,0,MemLen);

  CpsRom  =CpsGfx +nCpsGfxLen;
  CpsCode =CpsRom +nCpsRomLen;
  CpsZRom =CpsCode+nCpsCodeLen;
  CpsQSam =(char *)(CpsZRom+nCpsZRomLen);
  CpsAd   =(unsigned char *)(CpsQSam+nCpsQSamLen);

  // Create Gfx addr mask
  for (i=0;i<31;i++) { if ( (1<<i) >= (int)nCpsGfxLen) break; }

  // Default tile finding functions
  if (nCpsGfxLen<=0x800000) { CpsFind0=StdFind02; CpsFind1=StdFind1; CpsFind2=StdFind02; CpsFind3=StdFind3; }
  else { CpsFind0=StdBigFind0; CpsFind1=StdBigFind1; CpsFind2=StdBigFind2; CpsFind3=StdBigFind3; }

  if (nCpsZRomLen>=5)
  {
    // 77->cfff and rst 00 in case driver doesn't load
    CpsZRom[0]=0x3e; CpsZRom[1]=0x77;
    CpsZRom[2]=0x32; CpsZRom[3]=0xff; CpsZRom[4]=0xcf;
    CpsZRom[5]=0xc7;
  }

  SepTableCalc(); // Precalc the separate table

  CpsReset=0;
  // Blank dip switches
  Cpi01A=Cpi01C=Cpi01E=00;

  // Usual Layer Controller Register address
  CpsLcReg=0x66;
  // Usual Priority Mask Registers addresses
  CpsPmReg[0]=0x68; CpsPmReg[1]=0x6a; CpsPmReg[2]=0x6c; CpsPmReg[3]=0x6e;
  // Usual bits for Layer Enable
  CpsLayEn[1]=2; CpsLayEn[2]=4; CpsLayEn[3]=8;

  memset(&CpsId,0,sizeof(CpsId)); // Assume no id
  memset(&CpsMult,0,sizeof(CpsMult)); // Assume no multiply ports
  // Usual ADPCM
  nPsndIrqPeriod=0;
  nPsaSpeed=7576;

  BurnScode=NULL;
  return 0;
}

int CpsExit()
{
  BurnScode=NULL;
  nPsaSpeed=0;
  nPsndIrqPeriod=0;
  memset(&CpsMult,0,sizeof(CpsMult));
  memset(&CpsId,0,sizeof(CpsId));
  CpsLayEn[1]=0; CpsLayEn[2]=0; CpsLayEn[3]=0;
  CpsLcReg=0;
  CpsAd=NULL;   nCpsAdLen=0;
  CpsQSam=NULL; nCpsQSamLen=0;
  CpsZRom=NULL; nCpsZRomLen=0;
  CpsCode=NULL; nCpsCodeLen=0;
  CpsRom =NULL; nCpsRomLen=0;
  if (CpsGfx!=NULL) free(CpsGfx);  CpsGfx=NULL;
  nCpsCycles=0;
  Cps=0;
  return 0;
}
