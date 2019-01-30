#include "burnint.h"
// CPS (rotation)

static unsigned char *RotBuf=NULL;
static unsigned char *pSaveDraw=NULL;
static int nSavePitch=0;

int RotInit()
{
  RotBuf=(unsigned char *)malloc(224*384*4);
  if (RotBuf==NULL) return 1;
  return 0;
}

int RotExit()
{
  if (RotBuf!=NULL) free(RotBuf);  RotBuf=NULL;
  return 0;
}

int RotStart()
{
  pSaveDraw=NULL; nSavePitch=0;
  // Point to Rotate buffer instead
  if (pBurnDraw==NULL) return 1;
  pSaveDraw=pBurnDraw;
  nSavePitch=nBurnPitch;
  if (RotBuf==NULL) return 1;
  pBurnDraw=RotBuf;
  nBurnPitch=384*4;
  return 0;
}

int RotStop()
{
  int y=0;
  unsigned char *psl,*pdl;

  // Point to original buffer
  if (pSaveDraw==NULL) return 1;
  pBurnDraw=pSaveDraw;
  nBurnPitch=nSavePitch;

  if (RotBuf==NULL) return 1;

  psl=RotBuf+383*nBurnBpp;
  pdl=pBurnDraw;
  for (y=0;y<384;y++,psl-=nBurnBpp,pdl+=nBurnPitch)
  {
    unsigned char *ps,*pd; int x;
    ps=psl; pd=pdl;

    if (nBurnBpp==2)
    {
      for (x=0;x<224;x++,ps+=384*4,pd+=2)
      { *((unsigned short *)pd)=*((unsigned short *)ps); }
    }
    else if (nBurnBpp==3)
    {
      for (x=0;x<224;x++,ps+=384*4,pd+=3)
      { pd[0]=ps[0]; pd[1]=ps[1]; pd[2]=ps[2]; }
    }
    else if (nBurnBpp==4)
    {
      for (x=0;x<224;x++,ps+=384*4,pd+=4)
      { *((unsigned int *)pd)=*((unsigned int *)ps); }
    }
    else if (nBurnBpp==1)
    {
      for (x=0;x<224;x++,ps+=384*4,pd++)
      { pd[0]=ps[0]; }
    }
  }
  return 0;
}
