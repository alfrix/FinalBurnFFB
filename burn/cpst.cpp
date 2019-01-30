#include "burnint.h"

// CPS Tiles
unsigned int *CpstPal=NULL;

// Arguments for the tile draw function
unsigned int nCpstType=0; int nCpstX=0,nCpstY=0;
unsigned int nCpstTile=0; int nCpstFlip=0;
short *CpstRowShift=NULL;
unsigned int CpstPmsk=0; // Pixel mask

int CpsBgHigh(int a)
{
  // Handle BG-High
  unsigned short Mask;
  Mask=*((unsigned short *)(CpsSaveReg+CpsPmReg[(a>>7)&3]));
  if (Mask==0x7fff) return 0; // Draw normally
  if (Mask==0) return 1; // Tile is blank
  nCpstType|=CTT_PMSK; CpstPmsk=Mask;
  return 0;
}

int CpstOne()
{
  int Fun; int Size;
  Size=(nCpstType&24)+8;

  if (nCpstType&CTT_CARE)
  {
    if ((nCpstType&CTT_ROWS)==0)
    {
      // Return if not visible at all
      if (nCpstX<-Size) return 0;
      if (nCpstX>=384)   return 0;
      if (nCpstY<-Size) return 0;
      if (nCpstY>=224)   return 0;
    }
    nCtvRollX=0x4000017f + nCpstX * 0x7fff;
    nCtvRollY=0x400000df + nCpstY * 0x7fff;
  }

  // Restrict to loaded graphics data (we have a gap of 0x200 at the end)
  if (nCpstTile>=nCpsGfxLen) return 1;
  pCtvTile=CpsGfx+nCpstTile;

  // Find pLine (pointer to first pixel)
  pCtvLine=pBurnDraw + nCpstY*nBurnPitch + nCpstX*nBurnBpp;

  if (Size==32) nCtvTileAdd=16; else nCtvTileAdd=8;

  if (nCpstFlip&2)
  {
    // Flip vertically
         if (Size==16) { nCtvTileAdd= -8; pCtvTile+=15* 8; }
    else if (Size==32) { nCtvTileAdd=-16; pCtvTile+=31*16; }
    else               { nCtvTileAdd= -8; pCtvTile+= 7* 8; }
  }

  Fun =nCpstType&0x3e;
  Fun|=nCpstFlip&1;

  return CtvDoX[Fun]();
}
