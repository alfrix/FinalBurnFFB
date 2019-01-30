#include "aft.h"
// After Burner - 'Ground' emulation

unsigned char *AftGrBmap=NULL; // Ground bitmap length 0x40000 bytes

int AftGrDraw()
{
  int y=0;
  unsigned short *Grn=NULL;
  unsigned char *pLine=NULL;

  Grn=(unsigned short *)AftRam2E;
  pLine=pBurnDraw;

  if (AftGame!=3)
  {
    for (y=0;y<224; y++,pLine+=nBurnPitch)
    {
      BurnFillLine(pLine,BsysPal[0x1720]);
    }
    return 0;
  }

  for (y=0;y<224; y++,pLine+=nBurnPitch)
  {
    int Vert=0,Flip=0,BackCol=0,Cadd=0;
    int x=0;
    unsigned char *pPix=NULL;
    unsigned char *GrLine=NULL;
    int sx[2]={0,0};

    Vert=Grn[0x100+y];
    if (Vert&0x800)
    {
      int c;
      // Fill line
      c=BsysPal[0x1780+(Vert&0x7f)];
      BurnFillLine(pLine,c);
      continue;
    }
    Vert&=0x1ff;

    // Get horizontal shift of road
    sx[0]=Grn[0x200+y]; sx[0]-=0x550;
    sx[1]=Grn[0x500+y]; sx[1]-=0x550;

    // Get flip
    Flip=Grn[0x600+y];
    BackCol=(Flip>>8)&0xf;
    Cadd=Flip&1;

    pPix=pLine;
    GrLine=AftGrBmap+0x20000+((Vert&0x1fe)<<8);
    for (x=0; x<320; pPix+=nBurnBpp,x++,sx[0]++,sx[1]++)
    {
      int c[2]={0,0},Col=0;
      
      Col=GrLine[x];

      // Get bitmap pixels if in range
      if ((sx[0]&0xfffffe00)==0) c[0]=GrLine[sx[0]];
      if ((sx[1]&0xfffffe00)==0) c[1]=GrLine[sx[1]];
      Col=c[0]; if (c[1]>Col) Col=c[1];

      if (Col==0) PutPix(pPix,BsysPal[ 0x1730+BackCol]);
      if (Col==3) PutPix(pPix,BsysPal[ 0x1708+( Flip    &1) ]);
      if (Col==2) PutPix(pPix,BsysPal[ 0x170a+((Flip>>1)&1) ]);
      if (Col==1) PutPix(pPix,BsysPal[ 0x170c+((Flip>>2)&1) ]);
      if (Col==4) PutPix(pPix,BsysPal[ 0x170e+((Flip>>3)&1) ]);
    }
  }

  return 0;
}
