#include "gal.h"
// Galaxy Force HW - Memory

static unsigned char *Mem=NULL,*MemEnd=NULL;
static unsigned char *RamStart=NULL,*RamEnd=NULL;
unsigned char *GalRomM=NULL, *GalRomX=NULL, *GalRomY=NULL;
unsigned char *GalRamMFF=NULL, *GalRamXFF=NULL, *GalRamYFF=NULL;
unsigned char *GalRam0C=NULL, *GalRamM180=NULL, *GalRamM188=NULL;
unsigned char *GalRamM19=NULL, *GalRamX18=NULL;
unsigned char *GalPcm=NULL; int GalPcmLen=0;

// This routine is called first to determine how much memory is needed (MemEnd-(unsigned char *)0),
// and then to set up all the pointers
static int MemIndex()
{
  unsigned char *Next; Next=Mem;
  GalRomM=Next; Next+=0x040000; // cpu M program
  GalRomX=Next; Next+=0x040000; // cpu X program
  GalRomY=Next; Next+=0x080000; // cpu Y program
  GalLineData=Next; Next+=GalLineDataLen; // Line sprite data
  GalSprData =Next; Next+=GalSprDataLen; // Normal sprite data
  RamStart  =Next;
  GalRamMFF =Next; Next+=0x010000;   
  GalRamXFF =Next; Next+=0x010000;   
  GalRamYFF =Next; Next+=0x010000;   
  GalRam0C  =Next; Next+=0x010000; // shared?
  GalRamM180=Next; Next+=0x000800; // video ic27-28
  GalRamM188=Next; Next+=0x000800; // video ic29-30
  GalRamM19 =Next; Next+=0x010000; // video ic104/5 (palette?)
  GalRamX18 =Next; Next+=0x010000; // video ic3/4
  BsysZ80    =Next; Next+=0x010000; // z80 memory
  RamEnd    =Next;
  GalPcm    =Next; Next+=GalPcmLen; // pcm data
  BsysPal=(unsigned int *)Next; Next+=BsysPalLen*sizeof(unsigned int);
  MemEnd   =Next;
  return 0;
}

static int LoadRoms()
{
  int Ret=0,i=0;
  // Load program roms and byteswap
  Ret=BsysLoadCode(GalRomM,0,2); if (Ret!=0) return 1;
  Ret=BsysLoadCode(GalRomX,2,2); if (Ret!=0) return 1;
  Ret=BsysLoadCode(GalRomY,4,4); if (Ret!=0) return 1;

  // Load line sprite data
  if ((GalGame&0xf0)==0x30)
  {
    // Two 256k roms
    Ret=BurnLoadRom(GalLineData+0x00000, 8,2);
    Ret=BurnLoadRom(GalLineData+0x00001, 9,2);
  }
  else if ((GalGame&0xf0)==0x50)
  {
    // Four 256k roms
    Ret=BurnLoadRom(GalLineData+0x00000, 8,2);
    Ret=BurnLoadRom(GalLineData+0x00001, 9,2);
    Ret=BurnLoadRom(GalLineData+0x80000,10,2);
    Ret=BurnLoadRom(GalLineData+0x80001,11,2);
  }
  else
  {
    // Four 128k roms
    Ret=BurnLoadRom(GalLineData+0x00000, 8,2);
    Ret=BurnLoadRom(GalLineData+0x00001, 9,2);
    Ret=BurnLoadRom(GalLineData+0x40000,10,2);
    Ret=BurnLoadRom(GalLineData+0x40001,11,2);
  }

  // Load the sprite roms and interleve by 8
  if ((GalGame&0xf0)==0x30) // Rail Chase
  {
    // 24 x 512k roms
    for (i=0;i<24;i++)
    {
      BurnLoadRom(GalSprData+0x400000*(i>>3)+(i&7),12+i,8);
    }
  }
  else if ((GalGame&0xe0)==0x40) // G-Loc / strike fighter
  {
    // 32 x 512k roms
    for (i=0;i<32;i++)
    {
      BurnLoadRom(GalSprData+0x400000*(i>>3)+(i&7),12+i,8);
    }
  }
  else
  {
    // 32 x 128k roms
    for (i=0;i<32;i++)
    {
      BurnLoadRom(GalSprData+0x100000*(i>>3)+(i&7),12+i,8);
    }
  }

  // Load the z80 Rom
  BurnLoadRom(BsysZ80,44,1);

  // Load PCM data
  memset(GalPcm,0x80,GalPcmLen); // fill sound with silence

  if ((GalGame&0xf0)==0x30 || (GalGame&0xe0)==0x40) // Rail Chase or G-Loc/Strike Fighter
  {
    // Rail Chase
    BurnLoadRom(GalPcm+0x000000,47,1);
    BurnLoadRom(GalPcm+0x080000,46,1);
    BurnLoadRom(GalPcm+0x100000,45,1);
  }
  else
  {
    BurnLoadRom(GalPcm+0x000000,45,1);
    BurnLoadRom(GalPcm+0x080000,46,1);
    BurnLoadRom(GalPcm+0x0a0000,47,1);
  }
  return 0;
}

int GalMemInit()
{
  int Ret=0,Len=0;

  GalLineDataLen=0x100000;
  if ((GalGame&0xf0)==0x30) // Rail Chase
  {
    GalSprDataLen=0xc00000; // 24x512k
    GalPcmLen=0x180000;
  }
  else if ((GalGame&0xe0)==0x40) // G-Loc/Strike Fighter
  {
    GalSprDataLen=0x1000000; // 32x512k
    GalPcmLen=0x180000;
  }
  else
  {
    GalSprDataLen=0x400000; // 32x128k
    GalPcmLen=0xc0000;
  }

  BsysPalLen=0x4000;

  // Find out how much memory is needed
  Mem=NULL; MemIndex();
  Len=MemEnd-(unsigned char *)0;
  Mem=(unsigned char *)malloc(Len); if (Mem==NULL)  return 1;
  memset(Mem,0,Len); // blank all memory
  MemIndex(); // Index the allocated memory

  Ret=LoadRoms(); if (Ret!=0) return 1; // Load the roms into memory

  Ret=GalCpuInit(); if (Ret!=0) return 1; // Init the cpus
  return 0;
}

int GalMemExit()
{
  SekExit(); // Deallocate 68000s

  // Deallocate all used memory
  free(Mem); Mem=NULL;

  BsysPal=NULL; BsysPalLen=0; // Blank palette info
  GalLineData=NULL; GalLineDataLen=0; // Blank line sprite data
  GalSprData =NULL; GalSprDataLen=0; // Blank sprite data

  GalPcmLen=0; // Blank pcm data length
  GalSprDataLen=0; // Blank sprite data length
  GalGame=0; // blank game
  return 0;
}

// Scan ram
int GalScan(int nAction,int *pnMin)
{
  struct BurnArea ba;
  memset(&ba,0,sizeof(ba));

  if (nAction&4) // Scan volatile ram
  {
    if (pnMin!=NULL) *pnMin=0x051200; // Return minimum compatible version

    ba.Data=RamStart;
    ba.nLen=RamEnd-RamStart;
    ba.szName="All Ram";
    BurnAcb(&ba);

    SekScan(nAction&3); // scan 68000 states

    CalcScan(); // Maths chip

    BsysZScan(nAction&3); // Scan AB Z80
    PcmScan(); // Scan PCM chip
    SCAN_VAR(GalInput)
    SCAN_VAR(GalLastGear)
    SCAN_VAR(GalCounter)
  }

  return 0;
}
