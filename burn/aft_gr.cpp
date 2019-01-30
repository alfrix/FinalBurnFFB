#include "aft.h"
// After Burner - 'Ground' emulation

int AftGrDraw()
{
  int y=0;
  unsigned short *Grn=NULL;
  unsigned char *pl=NULL;

  Grn=(unsigned short *)OutGrRam;
  pl=pBurnDraw;
  for (y=0;y<224; y++,pl+=nBurnPitch)
  {
    int Vert=Grn[y];

    if (Vert&0x800)
    {
      int c;
      // Just fill line with z&0x7f
      c=BsysPal[0x780+(Vert&0x7f)]; BurnFillLine(pl,c);
      continue;
    }

  return 0;
}
