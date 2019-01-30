#include "aft.h"
// After Burner hardware - sprites

unsigned char *AftSprSrc=NULL; // Original sprite table

static INLINE int DrawSprite(unsigned char *ps)
{
  int Pal;

  bss.Top   =((ps[ 1]<<8)|ps[ 0])&0x1ff; bss.Top-=0x100;
  bss.Height=((ps[11]<<8)|ps[10])&0x1ff; bss.Height++;

  bss.Left=((ps[5]<<8)|ps[4])&0x1ff; bss.Left-=0xbc;
  bss.Width=(ps[5]&0xfe)<<2;

  Pal=ps[0x0c]<<4;
  bss.pPal=BsysPal+Pal;

  if (ps[5]&0x80) { bss.Width=0x400-bss.Width; }
  bss.ClipWidth=bss.Width;

  // Find the bitmap in the rom data
  bss.Base=((ps[1]&0x0e)<<17) | (ps[3]<<10) | (ps[2]<<2);

  // Find the zoom values
  {
    int nZoomX,nZoomY;
    nZoomX=((ps[7]<<8)|ps[6])&0xfff;
    nZoomY=((ps[9]<<8)|ps[8])&0xfff;
    if (nZoomY==0) nZoomY=nZoomX;
    bss.IncX=nZoomX;
    bss.IncY=nZoomY;
  }

  bss.FlipX=0;
  if ((ps[9]&0x40)==0) { bss.Base+=4-(bss.Width>>1); bss.FlipX=1; }
  bss.DrawToLeft=((ps[9]&0x20)==0);
  bss.DrawToTop= ((ps[9]&0x80)==0);
  if (ps[5]&0x80) bss.IncY=-bss.IncY; // Flip Y (into sprite above)

  bss.Trans=0x8001; //normal transparent mask
  BsysSprDraw(); // Draw sprite
  return 0;
}

int AftSprDraw()
{
  int i=0; unsigned char *ps=NULL;

  ps=AftSprSrc;
  for (i=0; i<0x200; ps+=0x10,i++)
  {
    if (ps[1]&0x80) break; // end of list
    if (ps[1]&0x40) continue; // skip sprite
    DrawSprite(ps);
  }

  return 0;
}
