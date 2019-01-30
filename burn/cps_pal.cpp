#include "burnint.h"
// CPS (palette)

static int nCpsPalCount=0x800; // Total number of colors
static unsigned char *CpsPalSrc=NULL; // Copy of current input palette

unsigned int *CpsPal=NULL; // Hicolor version of palette

static INLINE unsigned int CalcCol(unsigned short a,int i)
{
  int r,g,b,f;
  //FFFF RRRR GGGG BBBB
  // To truecolor:
  r=(a&0x0f00)>>4; // rrrr 0000
  g=(a&0x00f0)   ; // gggg 0000
  b=(a&0x000f)<<4; // bbbb 0000
  f=(a&0xf000)>>12;
  f+=5;
  r*=f; r/=15+5;
  g*=f; g/=15+5;
  b*=f; b/=15+5;

  return BurnHighCol(r,g,b,i);
}

static int CalcAll()
{
  int i; unsigned short *ps; unsigned int *pp;
  ps=(unsigned short *)CpsPalSrc; pp=CpsPal;
  for (i=0; i<nCpsPalCount; i++,ps++,pp++) *pp=CalcCol(*ps,i);
  return 0;
}

int CpsPalInit()
{
  int nLen=0;

  nLen=nCpsPalCount<<1;
  CpsPalSrc=(unsigned char *)malloc(nLen); if (CpsPalSrc==NULL) return 1;
  memset(CpsPalSrc,0,nLen);

  nLen=nCpsPalCount*sizeof(unsigned int);
  CpsPal=(unsigned int *)malloc(nLen);     if (CpsPal==NULL) return 1;

  // Set CpsPal to initial values
  CalcAll();
  
  return 0;
}

int CpsPalExit()
{
  if (CpsPal   !=NULL) free(CpsPal   );  CpsPal   =NULL;
  if (CpsPalSrc!=NULL) free(CpsPalSrc);  CpsPalSrc=NULL;
  return 0;
}

// Update CpsPal with the new palette at pNewPal (length 0x1000 bytes)
int CpsPalUpdate(unsigned char *pNewPal,int bRecalcAll)
{
  int i; unsigned short *ps,*pn;

  ps=(unsigned short *)CpsPalSrc;
  pn=(unsigned short *)pNewPal;

  // If we are recalculating the whole palette, just copy to CpsPalSrc
  // and recalculate it all
  if (bRecalcAll)
  { memcpy(ps,pn,nCpsPalCount<<1); CalcAll(); return 0; }

  for (i=0; i<nCpsPalCount; i++,ps++,pn++)
  {
    unsigned short n;
    n=*pn;
    if (*ps==n)
    {
      // Color hasn't changed - great!
      continue;
    }

    *ps=n; // Update our copy of the palette

    CpsPal[i]=CalcCol(n,i);
  }
  return 0;
}
