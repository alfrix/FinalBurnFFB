#include "aft.h"
// After Burner HW - Memory

static unsigned char *Mem=NULL,*MemEnd=NULL;
unsigned char *AftRom1,*AftRom2;

static unsigned char *RamStart,*RamEnd;
unsigned char *AftRam0C,*AftRam10,*AftRam12,*AftRam14,*AftRam16;
unsigned char *AftRam29,*AftRam2E,*AftRamFF;

// This routine is called first to determine how much memory is needed (MemEnd-(unsigned char *)0),
// and then to set up all the pointers
static int MemIndex()
{
  unsigned char *Next; Next=Mem;
  AftRom1=Next; Next+=0x080000;    // cpu1 program
  AftRom2=Next; Next+=0x080000;    // cpu2 program

  BsysTileDataLen=0x40000; BsysTileData=Next; Next+=BsysTileDataLen; // tile data
  BsysSprDataLen=0x200000; BsysSprData =Next; Next+=BsysSprDataLen; // sprite data
  AftGrBmap=Next; Next+=0x40000;  // road bitmaps

  PcmBankCount=6; PcmBankSize=16; // bank length = 1<<16 = 0x10000
  PcmRom=Next; Next+=0x060000; // pcm data

  RamStart=Next;
  AftRam0C=Next; Next+=0x011000;   // 0C=tile screen
  AftRam10=Next; Next+=0x002000;   // 10=sprites
  AftRam12=Next; Next+=0x004000;   // 12=palette
  AftRam29=Next; Next+=0x008000;   // 29/2A=shared ram
  AftRam2E=Next; Next+=0x002004;   // +4 because it includes the 0ee001 flag
  AftRamFF=Next; Next+=0x008000;
  BsysZ80=Next;  Next+=0x010000; // z80 memory
  RamEnd=Next;

  BsysPalLen=0x2000;
  BsysPal=(unsigned int *)Next; Next+=BsysPalLen*sizeof(unsigned int);
  MemEnd=Next;
  return 0;
}

static int LoadRoms()
{
  int Ret=0; int i=0;
  // Load program roms
  Ret=BsysLoadCode(AftRom1,0,4); if (Ret!=0) return 1;
  Ret=BsysLoadCode(AftRom2,4,4); if (Ret!=0) return 1;

  // Load tile data (three bitfields)
  BsysLoadTiles(BsysTileData,8);

  // Load the sprite roms and interleve by 4
  for (i=0;i<16;i++)
  {
    BurnLoadRom(BsysSprData+0x80000*(i>>2)+(3-(i&3)),11+i,4);
  }

  // Load ground graphics
  BurnLoadRom(AftGrBmap+0x40000,27,1); // Borrow 0x10000 of memory afterwards
  // Decode it to 8-bit
  BsysDecodeGr(AftGrBmap        ,AftGrBmap+0x40000);
  BsysDecodeGr(AftGrBmap+0x20000,AftGrBmap+0x48000);
  memset(AftGrBmap+0x40000,0,0x10000); // Re-blank borrowed memory

  // Load the z80 Rom
  BurnLoadRom(BsysZ80,28,1);

  // Load PCM data
  memset(PcmRom,0x80,0x60000); // fill sound with silence
  BurnLoadRom(PcmRom+0x00000,29,1);
  BurnLoadRom(PcmRom+0x20000,30,1);
  BurnLoadRom(PcmRom+0x40000,31,1);
  return 0;
}

int AftMemInit()
{
  int Len=0,Ret=0;
  // Find out how much memory is needed
  Mem=NULL; MemIndex();
  Len=MemEnd-(unsigned char *)0;
  Mem=(unsigned char *)malloc(Len); if (Mem==NULL) return 1;
  memset(Mem,0,Len); // blank all memory 
  MemIndex(); // Index the allocated memory

  Ret=LoadRoms(); if (Ret!=0) return 1; // Load the roms into memory
  return 0;
}

int AftMemExit()
{
  BsysSprData=NULL;  BsysSprDataLen=0;
  BsysTileData=NULL; BsysTileDataLen=0;

  // Deallocate all used memory
  if (Mem!=NULL) free(Mem);  Mem=NULL;
  return 0;
}

// Scan ram
int AftScan(int nAction,int *pnMin)
{
  struct BurnArea ba;
  memset(&ba,0,sizeof(ba));

  if (nAction&4) // Scan volatile ram
  {
    if (pnMin!=NULL) *pnMin=0x051501; // Return minimum compatible version

    ba.Data=AftRam0C; ba.nLen=0x011000; ba.szName="AftRam0C"; BurnAcb(&ba);
    ba.Data=AftRam10; ba.nLen=0x002000; ba.szName="AftRam10"; BurnAcb(&ba);
    ba.Data=AftRam12; ba.nLen=0x004000; ba.szName="AftRam12"; BurnAcb(&ba);
    ba.Data=AftRam29; ba.nLen=0x008000; ba.szName="AftRam29"; BurnAcb(&ba);
    ba.Data=AftRam2E; ba.nLen=0x002004; ba.szName="AftRam2E"; BurnAcb(&ba);
    ba.Data=AftRamFF; ba.nLen=0x008000; ba.szName="AftRamFF"; BurnAcb(&ba);
    ba.Data=BsysZ80+0xf100; ba.nLen=0xf00; ba.szName="Z80Ram"; BurnAcb(&ba);

    SekScan(nAction&3); // scan 68000 states

    CalcScan();

    BsysZScan(nAction&3); // Scan AB Z80
    PcmScan(); // Scan PCM chip

    SCAN_VAR(AftInputSel)
  }

  return 0;
}
