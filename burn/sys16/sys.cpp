#include "sys.h"
// System 16 - Main

unsigned char BsysReset=0;
unsigned char *BsysRom=NULL;  unsigned int BsysRomLen=0;    // Program Rom
unsigned char *BsysTile=NULL; unsigned char BsysTileBank=0; // Tile Ram len 0x011000
unsigned char *BsysObj =NULL; unsigned char BsysObjBank =0; // Obj Ram  len 0x001000
unsigned char *BsysPram=NULL; unsigned char BsysPramBank=0; // Pal Ram  len 0x001000
unsigned char *BsysRam=NULL;                                // Work Ram len 0x004000
unsigned char BsysKey[4][8]={0};
unsigned char BsysInp[4]={0,0,0,0},BsysDip[2]={0,0};
static unsigned char DefSprBank[]=
{
   0, 1, 2, 3,
   4, 5, 6, 7,
   8, 9,10,11,
  12,13,14,15
};
unsigned char *BsysSprBank=DefSprBank;

int BsysInit()
{
  unsigned int MemLen=0;
  
  // Allocate space for all memory
  MemLen=BsysRomLen+BsysTileDataLen+BsysSprDataLen+0x019004;
  BsysRom=(unsigned char *)malloc(MemLen); if (BsysRom==NULL) return 1;
  memset(BsysRom,0,MemLen);
  BsysTileData=BsysRom+BsysRomLen;
  BsysSprData =BsysTileData+BsysTileDataLen;
  BsysTile    =BsysSprData+BsysSprDataLen;
  BsysObj     =BsysTile+0x011000;
  BsysPram    =BsysObj +0x001000; BsysPalLen=0x800;
  BsysRam     =BsysPram+0x001000;
  BsysPal     =(unsigned int *)(BsysRam+0x004000);
  BsysPalInit();

  BsysTilePalBase=0;
  // Usual banks:
  BsysTileBank=0x40;
  BsysObjBank =0x44;
  BsysPramBank=0x84;

  BsysDip[0]=0x00;
  BsysDip[1]=0x02; // Advertise sound

  return 0;
}

int BsysExit()
{
  // Blank banks
  BsysTileBank=0;
  BsysObjBank =0;
  BsysPramBank=0;
  BsysTilePalBase=0;

  BsysSprBank=DefSprBank;

  BsysPalExit();
  // Deallocate space for all memory
  free(BsysRom);
  BsysPal=NULL; BsysPalLen=0;
  BsysTile=BsysObj=BsysPram=BsysRam=NULL;
  BsysSprData=NULL; BsysSprDataLen=0;
  BsysTileData=NULL; BsysTileDataLen=0;
  BsysRom=NULL; BsysRomLen=0;
  return 0;
}

int BsysRunReset()
{
  SekOpen(0);
  SekReset();
  SekClose();
  return 0;
}

int BsysFrame()
{
  int i=0;
  if (BsysReset) BsysRunReset();

  // Compile keys into bits to return from [0xc41000]
  for (i=0;i<4;i++)
  {
    int b=0,v=0;
    for (b=0;b<8;b++) { if (BsysKey[i][b]) v|=1<<b; }
    BsysInp[i]=(unsigned char)v;
  }

  SekOpen(0);
  SekInterrupt(4); SekRun(166667);
  SekClose();

  if (pBurnDraw!=NULL) BsysDraw();
  return 0;
}
