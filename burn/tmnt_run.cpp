#include "burnint.h"
// TMNT - Run module

unsigned char bTmntRecalcPal=0;

static unsigned char *Mem=NULL,*MemEnd=NULL;
static unsigned char *RamStart=NULL,*RamEnd=NULL;
static unsigned char *Rom,*Ram06,*Ram14;
unsigned char *TmntRam08,*TmntRam10,*TmntTile;
static unsigned char bInt5=0; // 1 if we are doing int 5

// This routine is called first to determine how much memory is needed (MemEnd-(unsigned char *)0),
// and then to set up all the pointers
static int MemIndex()
{
  unsigned char *Next; Next=Mem;
      Rom  =Next;   Next+=0x060000; // 68000 program
  RamStart =Next;
      Ram14=Next;   Next+=0x000800; // spr ram
  TmntRam10=Next;   Next+=0x008000; // tile ram
  TmntRam08=Next;   Next+=0x001000; // palette
      Ram06=Next;   Next+=0x004000;
  RamEnd   =Next;
  TmntTile =Next;   Next+=0x300000; // Tile bitmaps 
  MemEnd=Next;
  return 0;
}

static int Interleve2(unsigned char *pd,int i,int nLen)
{
  unsigned char *pt; unsigned short *pts,*ptd; int a;
  pt=(unsigned char *)malloc(nLen); if (pt==NULL) return 1;
  memset(pt,0,nLen);
  BurnLoadRom(pt,i,1);

  nLen>>=1; pts=(unsigned short *)pt; ptd=(unsigned short *)pd;
  for (a=0;a<nLen; a++, pts++, ptd+=2)  *ptd=*pts;

  free(pt);
  return 0;
}

static int LoadRoms()
{
  int nRet=0;
  // Load program roms and byteswap
  nRet=BurnLoadRom(Rom+0x00001,0,2); if (nRet!=0) return 1;
  nRet=BurnLoadRom(Rom+0x00000,1,2); if (nRet!=0) return 1;
  nRet=BurnLoadRom(Rom+0x40001,2,2); if (nRet!=0) return 1;
  nRet=BurnLoadRom(Rom+0x40000,3,2); if (nRet!=0) return 1;

  Interleve2(TmntTile+0x000000,4,0x80000);
  Interleve2(TmntTile+0x000002,5,0x80000);
  Interleve2(TmntTile+0x100000,6,0x80000);
  Interleve2(TmntTile+0x100002,7,0x80000);
  Interleve2(TmntTile+0x200000,8,0x80000);
  Interleve2(TmntTile+0x200002,9,0x80000);
  return 0;
}

static unsigned char TmntReadByte(unsigned int a)
{
  if (a>=0xa0000 && a<0xa0020) return (unsigned char)(~TmntAoo[(a>>1)&0xf]);
  return 0xff;
}

static void TmntWriteByte(unsigned int a,unsigned char d)
{
  unsigned int ab; ab=(a>>16)&0xff;
  if (ab==0x08)   { TmntPalWrite(a,d); return; } // Palette write
  if (a==0xa0001) { if (d&0x20) bInt5=1; else bInt5=0;  return; }
}

int TmntInit()
{
  int nRet=0; int nLen=0;

  // Find out how much memory is needed
  Mem=NULL; MemIndex();
  nLen=MemEnd-(unsigned char *)0;
  Mem=(unsigned char *)malloc(nLen); if (Mem==NULL)  return 1;
  memset(Mem,0,nLen); // blank all memory
  MemIndex(); // Index the allocated memory

  nRet=LoadRoms(); if (nRet!=0) return 1; // Load the roms into memory

  SekInit(1); // Allocate 1 68000

  SekOpen(0); 
  // Map in memory:
  // ----------------- Cpu 1 ------------------------
  SekMemory(    Rom  ,0x000000,0x05FFFF,SM_ROM); // 68000 Rom
  SekMemory(    Ram06,0x060000,0x063FFF,SM_RAM);
  SekMemory(TmntRam08,0x080000,0x080FFF,SM_ROM); // Write goes through handler
  SekMemory(TmntRam10,0x100000,0x107FFF,SM_RAM);
  SekMemory(    Ram14,0x140000,0x1407FF,SM_RAM);
  SekExt[0].ReadByte =TmntReadByte;
  SekExt[0].WriteByte=TmntWriteByte;
  // ------------------------------------------------
  SekReset(); bInt5=0;
  SekClose();

  TmntPalInit();
  return 0;
}

int TmntExit()
{
  TmntPalExit();

  SekExit(); // Deallocate 68000

  // Deallocate all used memory
  if (Mem!=NULL) free(Mem);  Mem=NULL;
  return 0;
}

static int TmntDraw()
{
  TmntPalUpdate(bTmntRecalcPal); // Recalc whole pal if needed
  bTmntRecalcPal=0;

  BurnClearScreen();
  TmntTileDraw();

  return 0;
}

int TmntFrame()
{
  TmntInpMake();

  SekOpen(0); 
  if (bInt5) SekInterrupt(5);
  SekRun(8000000/60); // 8mhz
  SekClose();

  if (pBurnDraw!=NULL) TmntDraw();
  return 0;
}

int TmntScan(int nAction,int *pnMin)
{
  if (nAction&4) // Scan volatile ram
  {
    struct BurnArea ba;
    if (pnMin!=NULL) *pnMin=0x010601; // Return minimum compatible version

    memset(&ba,0,sizeof(ba));
    ba.Data=RamStart;
    ba.nLen=RamEnd-RamStart;
    ba.szName="All Ram";
    BurnAcb(&ba);

    SekScan(nAction&3); // scan 68000 states
    SCAN_VAR(bInt5)
  }
  return 0;
}
