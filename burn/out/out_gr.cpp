#include "out.h"
// Out Run - 'Ground' emulation

unsigned char *OutGrBmap=NULL; // Ground bitmap length 0x20000 bytes
unsigned char *OutGrRam=NULL;  // Ground ram length 0x1000 bytes
unsigned char *OutGrFlag=NULL; // Flag ram length 2 bytes

int OutGrDraw()
{
  int y=0;
  unsigned short *Grn=NULL;
  unsigned char *pl=NULL;

  Grn=(unsigned short *)OutGrRam;
  pl=pBurnDraw;
  for (y=0;y<224; y++,pl+=nBurnPitch)
  {
    unsigned int c;
    int Vert; int Flip;
    int RoadOn;
    unsigned char *GrLine;
    int x; unsigned char *pPix;
    int sx[2];

    Vert=Grn[y];
    if (Vert&0x800)
    {
      // Just fill line with z&0x7f
      c=BsysPal[0x780+(Vert&0x7f)]; BurnFillLine(pl,c);
      continue;
    }

    Vert&=0x1ff;
    Flip=Grn[0x600+Vert];

    RoadOn=OutGrFlag[0]^2;  // road 0 == RoadOn&2  road 1 == RoadOn&1

    GrLine=OutGrBmap+((Vert<<8)&0x1fe00);

    // Get horizontal shift of road (200-400)
    sx[0]=Grn[0x200+Vert]-0x5f8;
    sx[1]=Grn[0x400+Vert]-0x5f8;
    for (pPix=pl,x=0; x<320; pPix+=nBurnBpp,x++,sx[0]++,sx[1]++)
    {
      int c[2]={0,0},Col;

      // Get bitmap pixels if in range
      if ((RoadOn&2) && (sx[0]&0xfffffe00)==0) c[0]=GrLine[sx[0]];
      if ((RoadOn&1) && (sx[1]&0xfffffe00)==0) c[1]=GrLine[sx[1]];

      Col=c[0]; if (c[1]>Col) Col=c[1];

      if (Col==0) PutPix(pPix,BsysPal[0x430+((Flip>>8)&0xf)]);

      if (Flip&1)
      {
        if (Col==1) PutPix(pPix,BsysPal[0x405]);
        if (Col==2) PutPix(pPix,BsysPal[0x403]);
        if (Col==3) PutPix(pPix,BsysPal[0x401]);
        if (Col==4) PutPix(pPix,BsysPal[0x407]);
      }
      else
      {
        if (Col==1) PutPix(pPix,BsysPal[0x404]);
        if (Col==2) PutPix(pPix,BsysPal[0x402]);
        if (Col==3) PutPix(pPix,BsysPal[0x400]);
        if (Col==4) PutPix(pPix,BsysPal[0x406]);
      }
    }
  }
  return 0;
}
