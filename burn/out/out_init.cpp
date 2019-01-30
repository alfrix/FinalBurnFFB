#include "out.h"
// Out Run - Init

static unsigned char *Data=NULL;

static int DrvSndGetBank(int Reg86)
{
  return (Reg86>>4)&7;
}

int OutInit()
{
  int Ret=0; int i=0;
  int DataLen=0;

  // Init memory and cpus
  Ret=OutMemInit(); if (Ret!=0) return 1;

  // Load program roms and byteswap
  Ret=BsysLoadCode(OutRom1,0,4); if (Ret!=0) return 1;
  Ret=BsysLoadCode(OutRom2,4,4); if (Ret!=0) return 1;

  // Allocate space for graphics
  DataLen=0x144000+0x20000+0x30000; // Sprites, Tiles, High Palette; Ground; Sound;
  Data=(unsigned char *)malloc(DataLen); if (Data==NULL) return 1;
  memset(Data,0,DataLen);

  // ------------  Sprites  -----------------------------------------
  BsysSprData=Data; BsysSprDataLen=0x100000;
  // Load the roms and interleve by 4
  for (i=0;i<8;i++) BurnLoadRom(BsysSprData+0x80000*(i>>2)+(i&3),14+i,4);

  // ------------  Tiles  -------------------------------------------
  BsysTileData=Data+0x100000; BsysTileDataLen=0x40000;
  BsysTilePalBase=0;
  // Load tile data (three bitfields)
  BsysLoadTiles(BsysTileData+0x00000, 8);
  BsysLoadTiles(BsysTileData+0x20000,11);

  // ------------  Palette  -----------------------------------------
  BsysPal=(unsigned int *)(Data+0x140000); BsysPalLen=0x1000;
  BsysPram=OutRam12;
  BsysPalInit();

  // ------------  Ground   -----------------------------------------
  OutGrBmap=Data+0x144000;
  BurnLoadRom(OutGrBmap+0x20000,22,1);  // Load 1-bit version in pcm (borrow the memory)
  BsysDecodeGr(OutGrBmap,OutGrBmap+0x20000); // Decode it to 8-bit
  OutGrRam =OutRam28;
  OutGrFlag=OutRam29;

  // ------------  Sound  -------------------------------------------
  BsysZ80=OutZ80;
  BsysZInit(); // z80 init
  PcmRom=Data+0x144000+0x20000; PcmBankCount=6; PcmBankSize=15; // 0x30000
  memset(PcmRom,0x80,0x30000); // fill sound with silence
  for (i=0;i<6;i++) BurnLoadRom(PcmRom+(i<<15),24+i,1);
  PcmGetBank=DrvSndGetBank;
  Ret=BurnLoadRom(OutZ80,23,1); if (Ret!=0) return 1;
  BsysSndInit(); // Init sound emulation

  OutRunReset(); // Reset machine

  return 0;
}

int OutExit()
{
  BsysSndExit();
  PcmRom=NULL; PcmBankCount=0; PcmBankSize=0;
  BsysZExit();
  BsysZ80=NULL;

  OutGrBmap=NULL;

  BsysPalExit();
  BsysPram=NULL; BsysPal=NULL; BsysPalLen=0;
  BsysTileData=NULL; BsysTileDataLen=0;
  BsysSprData=NULL; BsysSprDataLen=0;
  free(Data); Data=NULL;

  OutMemExit();
  return 0;
}
