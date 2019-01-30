// System 16 ---------------------------------------------
#include "../burnint.h"

// Calc for AB and GF hardware
#include "calc.h"

// sys.cpp
extern unsigned char BsysReset;
extern unsigned char *BsysRom; extern unsigned int BsysRomLen; // Program Rom 0x000000-0x0?FFFF
extern unsigned char *BsysTile; extern unsigned char BsysTileBank; // Tile Ram len 0x011000
extern unsigned char *BsysObj ; extern unsigned char BsysObjBank ; // Obj Ram  len 0x001000
extern unsigned char *BsysPram; extern unsigned char BsysPramBank; // Pal Ram
extern unsigned char *BsysRam;                                     // Work Ram len 0x004000
extern unsigned char BsysKey[4][8];
extern unsigned char BsysInp[4],BsysDip[2];
extern unsigned char *BsysSprBank;

int BsysInit();
int BsysExit();
int BsysRunReset();
int BsysFrame();

// sys_draw.cpp
int BsysDraw();

// sys_mem.cpp
int BsysInputInfo(struct BurnInputInfo *pii,unsigned int i);
int BsysRunInit();
int BsysRunExit();

// sys_misc.cpp
int BsysLoadCode(unsigned char *Rom,int Start,int Count);
int BsysLoadTiles(unsigned char *Tile,int Start);
int BsysZeroSprites(unsigned char *Spr,int Len);
int BsysDecodeGr(unsigned char *Dest,unsigned char *Src);

// sys_pal.cpp
extern unsigned int *BsysPal; // Pointer to high-color palette (output format)
extern unsigned int BsysPalLen; // number of colors, must be a power of two
int BsysPalInit();
int BsysPalExit();
int BsysPalUpdate();
void BsysPalWriteByte(unsigned int a,unsigned char d);
void BsysPalWriteWord(unsigned int a,unsigned short d);

// sys_spr.cpp
struct BsysSprite
{
  int Left,Top;    // Coords to start drawing from (on the screen)
  int Height;      // Height of the sprite (in pixels on the screen)
  int Width;       // Width of the sprite  (in pixels in the data)
  int ClipWidth;   // Clip to this width though
  unsigned int *pPal;// Pointer to highpal palette
  unsigned short Trans; // Transparent data (0x0001 = color 0 trasnparent)
  int Base;        // Base of graphics in the rom data
  int IncX;        // Amount to increase x and y by (in the data) for
  int IncY;        //   each screen unit (fixed point 23.9)
  unsigned char FlipX; // Flip left to right
  unsigned char DrawToLeft,DrawToTop;  // Draw right to left/Draw bottom to top
};

extern unsigned char *BsysSprData; // Sprite bitmap data
extern int BsysSprDataLen; // Length of above
extern struct BsysSprite bss; // Sprite to draw
int BsysSprDraw();

// sys_tile.cpp
extern unsigned char *BsysTileData; // Pointer to tile bitmap data
extern int BsysTileDataLen; // Length of data    (must be power of 2)
extern int BsysTilePalBase; // Palette Base
int BsysTileOne(unsigned char *Dest,int Tile,int nPal);
int BsysTextPage(unsigned char *RamTile);
int BsysBgPage(unsigned char *RamTile);

// sys_pcm.cpp
extern int PcmRate; // sample rate to render at
extern unsigned char *PcmRom; // Pointer to the Pcm banks
extern int PcmBankCount; // Count of the number of banks
extern int PcmBankSize; // Bank length is 1<<PcmBankSize bytes
int PcmInit();
int PcmExit();
int PcmScan();
int PcmUpdate(short *Dest,int nLen); // Add nLen*2 16-bit samples (stereo interleved) into Dest
extern int (*PcmGetBank) (int Reg86);
unsigned char PcmRead(unsigned short a);
void PcmWrite(unsigned short a,unsigned char d);

// sys_snd.cpp
extern int BsysSndOkay; // 1 if the module is initted okay
int BsysSndInit();
int BsysSndExit();
int BsysSndFrameStart();
int BsysSndRenderTo(int nPos);
int BsysSndFrameStop();

// sys_z80.cpp
extern unsigned char *BsysZ80;
extern int BsysFmTimer; // The z80 cycle count when FM timer A will carry out
extern unsigned char BsysSndCode; // The sound code to pass to the z80 program
int BsysZScan(int nAction);
int BsysZInit();
int BsysZExit();

int DforceFeedback(int xdirection,int ydirection,int force);