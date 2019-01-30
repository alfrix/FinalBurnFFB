#include "burnint.h"

// CPS Scroll (Background Layers)

int CpsScrHigh=0; // If 1, draw the 'high' pixels of the background layer

// Base = 0x4000 long tile map
// sx=Scroll X value, sy=Scroll Y value,

int CpsScr1Draw(unsigned char *Base,int sx,int sy)
{
  int x,y;
  int ix,iy;
  int KnowBlank=-1; // The tile we know is blank

  ix=(sx>>3)+1; iy=(sy>>3)+1;
  sx&=7; sy&=7; sx=8-sx; sy=8-sy;

  for (y=-1; y<28; y++)
  {
    for (x=-1; x<48; x++)
    {
      int t,g,a; int Ret;
      unsigned short *pst;
      int fx,fy,p;
      fx=ix+x; fy=iy+y; // fx/fy= 0 to 63
      
      // Find tile address
      p=((fy&0x20)<<8) | ((fx&0x3f)<<7) | ((fy&0x1f)<<2);
      pst=(unsigned short *)(Base + p);

      t=pst[0]; if (t==KnowBlank) continue; // Don't draw: we know it's blank
      // Get tile address
      g=CpsFind1(t); if (g<0) { KnowBlank=t; continue; } // Finder says it's blank

      a=pst[1];
      // Handle bg-high
      nCpstType=0;
      if (CpsScrHigh)
      {
        if (CpsBgHigh(a)) { KnowBlank=t; continue; } // Blank in high version of scroll
      }
      
      CpstSetPal(0x20 | (a&0x1f));

      // Don't need to clip except around the border
      if (x<0 || x>=48-1 || y<0 || y>=28-1) nCpstType|=CTT_8X8 | CTT_CARE;
      else nCpstType|=CTT_8X8;

      nCpstX=sx+(x<<3); nCpstY=sy+(y<<3);
      nCpstTile=g; nCpstFlip=(a>>5)&3;

      Ret=CpstOne();  if (Ret) KnowBlank=t; // Remember it was blank for next time
    }
  }
  return 0;
}

int CpsScr3Draw(unsigned char *Base,int sx,int sy)
{
  int x,y;
  int ix,iy;
  int KnowBlank=-1; // The tile we know is blank
  ix=(sx>>5)+1; iy=(sy>>5)+1;
  sx&=31; sy&=31; sx=32-sx; sy=32-sy;

  for (y=-1; y<7; y++)
  {
    for (x=-1; x<12; x++)
    {
      int t,g,a; int Ret;
      unsigned short *pst;
      int fx,fy,p;
      fx=ix+x; fy=iy+y; // fx/fy= 0 to 63
      
      // Find tile address
      p=((fy&0x38)<<8) | ((fx&0x3f)<<5) | ((fy&0x07)<<2);
      pst=(unsigned short *)(Base + p);

      t=pst[0]; if (t==KnowBlank) continue; // Don't draw: we know it's blank
      // Get tile address
      g=CpsFind3(t); if (g<0) { KnowBlank=t; continue; } // Finder says it's blank

      a=pst[1];
      // Handle bg-high
      nCpstType=0;
      if (CpsScrHigh)
      {
        if (CpsBgHigh(a)) { KnowBlank=t; continue; } // Blank in high version of scroll
      }

      CpstSetPal(0x60 | (a&0x1f));

      // Don't need to clip except around the border
      if (x<0 || x>=12-1 || y<0 || y>=7-1) nCpstType|=CTT_32X32 | CTT_CARE;
      else nCpstType|=CTT_32X32;

      nCpstX=sx+(x<<5); nCpstY=sy+(y<<5);
      nCpstTile=g; nCpstFlip=(a>>5)&3;

      Ret=CpstOne();  if (Ret) KnowBlank=t; // Remember it was blank for next time
    }
  }

  return 0;
}
