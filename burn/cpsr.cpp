#include "burnint.h"

// CPS Scroll2 with Row scroll support

unsigned char *CpsrBase=NULL; // Tile data base
int nCpsrScrX=0,nCpsrScrY=0; // Basic scroll info
unsigned short *CpsrRows=NULL; // Row scroll table, 0x400 words long
int nCpsrRowStart=0; // Start of row scroll (can wrap?)
static int nShiftY=0;

struct CpsrLineInfo CpsrLineInfo[15];

static INLINE void GetRowsRange(int *pnStart,int *pnWidth,int nRowFrom,int nRowTo)
{
  int i,nStart,nWidth;

  // Get the range of scroll values within nRowCount rows
  // Start with zero range
  nStart=CpsrRows[nRowFrom&0x3ff]; nStart&=0x3ff; nWidth=0;
  for (i=nRowFrom;i<nRowTo;i++)
  {
    int nViz; int nDiff;
    nViz=CpsrRows[i&0x3ff]; nViz&=0x3ff;
    // Work out if this is on the left or the right of our
    // start point.
    nDiff=nViz-nStart;
    // clip to 10-bit signed
    nDiff=((nDiff+0x200)&0x3ff)-0x200;
    if (nDiff>=0)
    {
      // On the right
      if (nDiff>=nWidth) nWidth=nDiff; // expand width to cover it
    }
    else
    {
      // On the left
      nStart+=nDiff; nStart&=0x3ff;
      nWidth-=nDiff; // expand width to cover it
    }    
  }

  if (nWidth>0x400) nWidth=0x400;

  *pnStart=nStart;
  *pnWidth=nWidth;
}

static int PrepareRows()
{
  int y; struct CpsrLineInfo *pli; int r;
  // Calculate the amount of pixels to shift each
  // row of the tile lines, assuming we draw tile x at
  // (x-pli->nTileStart)<<4  -  i.e. 0, 16, ...

  r=nShiftY-16;
  for (y=-1,pli=CpsrLineInfo; y<14; y++,pli++)
  {
    // Maximum row scroll left and right on this line
    int nMaxLeft=0,nMaxRight=0;
    int ty; short *pr;
    
    if (CpsrRows==NULL)
    {
      // No row shift - all the same
      int v;
      v =(pli->nTileStart<<4)-nCpsrScrX;
      nMaxLeft=v; nMaxRight=v;
      for (ty=0,pr=pli->Rows; ty<16; ty++,pr++)
      {
        *pr=(short)v;
      }
    }
    else
    {
      for (ty=0,pr=pli->Rows; ty<16; ty++,pr++,r++)
      {
        // Get the row offset, if it's in range
        if (r>=0 && r<224)
        {
          int v;
          v =(pli->nTileStart<<4)-nCpsrScrX;
          v-=CpsrRows[(nCpsrRowStart+r)&0x3ff];
          // clip to 10-bit signed
          v+=0x200; v&=0x3ff; v-=0x200;
          *pr=(short)v;
               if (v<nMaxLeft)  nMaxLeft=v;
          else if (v>nMaxRight) nMaxRight=v;
        }
        else
        {
          *pr=0;
        }
      }
    }

    pli->nMaxLeft =nMaxLeft;
    pli->nMaxRight=nMaxRight;
  }

  return 0;
}

// Prepare to draw Scroll 2 with rows, by seeing how much
// row scroll each tile line uses (pli->nStart/nWidth),
// and finding which tiles are visible onscreen (pli->nTileStart/End).

int CpsrPrepare()
{
  int y; struct CpsrLineInfo *pli;
  if (CpsrBase==NULL) return 1;

  nShiftY=16-(nCpsrScrY&15);

  for (y=-1,pli=CpsrLineInfo; y<14; y++,pli++)
  {
    int nStart=0,nWidth=0;

    if (CpsrRows!=NULL)
    {
      int nRowFrom,nRowTo;
      // Find out which rows we need to check
      nRowFrom=(y<<4)+nShiftY;
      nRowTo=nRowFrom+16;
      if (nRowFrom<0) nRowFrom=0;
      if (nRowTo>224) nRowTo=224;

      // Shift by row table start offset
      nRowFrom+=nCpsrRowStart;
      nRowTo  +=nCpsrRowStart;

      // Find out what range of scroll values there are for this line
      GetRowsRange(&nStart,&nWidth,nRowFrom,nRowTo);
    }

    nStart+=nCpsrScrX;
    nStart&=0x3ff;

    // Save info in CpsrLineInfo table
    pli->nStart=nStart;
    pli->nWidth=nWidth;
    // Find range of tiles to draw to see whole width:
    pli->nTileStart=nStart>>4;
    pli->nTileEnd=(nStart+nWidth+0x18f)>>4;
  }

  PrepareRows();
  return 0;
}
