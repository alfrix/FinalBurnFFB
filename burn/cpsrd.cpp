#include "burnint.h"

// CPS Scroll2 with Row scroll - Draw
static int KnowBlank=-1; // The tile we know is blank

static INLINE unsigned short *FindTile(int fx,int fy)
{
  int p; unsigned short *pst;
  // Find tile address
  p=((fy&0x30)<<8) | ((fx&0x3f)<<6) | ((fy&0x0f)<<2);
  pst=(unsigned short *)(CpsrBase + p);
  return pst;
}

// Draw a tile line without Row Shift
static void TileLine(int y,int sx)
{
  int bVCare=0;
  int x,ix,iy,sy;

  if (y<0 || y>=14-1) bVCare=1; // Take care on the edges

  ix=(sx>>4)+1; sx&=15; sx=16-sx;
  sy=16-(nCpsrScrY&15); iy=(nCpsrScrY>>4)+1;
  nCpstY=sy+(y<<4);
  
  for (x=-1; x<24; x++)
  {
    unsigned short *pst; int t,g,a; int Ret;
    // Don't need to clip except around the border
    if (bVCare || x<0 || x>=24-1) nCpstType=CTT_16X16 | CTT_CARE;
    else nCpstType=CTT_16X16;

    pst=FindTile(ix+x,iy+y);
    t=pst[0]; if (t==KnowBlank) continue; // Don't draw: we know it's blank
    // Get tile address
    g=CpsFind2(t); if (g<0) { KnowBlank=t; continue; } // Finder says it's blank

    a=pst[1];
    // Handle bg-high
    nCpstType&=~CTT_PMSK;
    if (CpsScrHigh)
    {
      if (CpsBgHigh(a)) { KnowBlank=t; continue; } // Blank in high version of scroll
    }

    CpstSetPal(0x40 | (a&0x1f));
    nCpstX=sx+(x<<4); nCpstTile=g; nCpstFlip=(a>>5)&3;
    Ret=CpstOne();  if (Ret) KnowBlank=t; // Remember it was blank for next time
  }
}

// Draw a tile line with Row Shift
static void TileLineRows(int y,struct CpsrLineInfo *pli)
{
  int sy,iy,x;
  int nTileCount;
  int nLimLeft,nLimRight;
  int bVCare=0;

  if (y<0 || y>=14-1) bVCare=1; // Take care on the edges

  nTileCount=pli->nTileEnd-pli->nTileStart;

  sy=16-(nCpsrScrY&15); iy=(nCpsrScrY>>4)+1;
  nCpstY=sy+(y<<4);
  CpstRowShift=pli->Rows;

  // If these rowshift limits go off the edges, we should take
  // care drawing the tile.
  nLimLeft =pli->nMaxLeft;
  nLimRight=pli->nMaxRight;
  for (x=0; x<nTileCount; x++,
    nLimLeft+=16, nLimRight+=16)
  {
    unsigned short *pst; int t,g,a; int tx; int bCare; int Ret;
    tx=pli->nTileStart+x;

    // See if we have to clip vertically anyway
    bCare=bVCare;
    if (bCare==0) // If we don't...
    {
      // Check screen limits of this tile
      if (nLimLeft <      0) bCare=1; // Will cross left egde
      if (nLimRight> 384-16) bCare=1; // Will cross right edge
    }
    if (bCare) nCpstType=CTT_16X16 | CTT_ROWS | CTT_CARE;
    else       nCpstType=CTT_16X16 | CTT_ROWS;

    pst=FindTile(tx,iy+y);
    t=pst[0]; if (t==KnowBlank) continue; // Don't draw: we know it's blank
    // Get tile address
    g=CpsFind2(t); if (g<0) { KnowBlank=t; continue; } // Finder says it's blank
    a=pst[1];
    // Handle bg-high
    nCpstType&=~CTT_PMSK;
    if (CpsScrHigh)
    {
      if (CpsBgHigh(a)) { KnowBlank=t; continue; } // Blank in high version of scroll
    }

    CpstSetPal(0x40 | (a&0x1f));

    nCpstX=x<<4; nCpstTile=g; nCpstFlip=(a>>5)&3;
    Ret=CpstOne();  if (Ret) KnowBlank=t; // Remember it was blank for next time
  }
}

int CpsrRender()
{
  int y; struct CpsrLineInfo *pli;
  if (CpsrBase==NULL) return 1;

  KnowBlank=-1; // We don't know which tile is blank yet

  for (y=-1,pli=CpsrLineInfo; y<14; y++,pli++)
  {
    if (pli->nWidth==0)
      TileLine(y,pli->nStart); // no rowscroll needed
    else
      TileLineRows(y,pli); // row scroll
  }
  return 0;
}
