#include "sys.h"
// System 16 - Palette

unsigned int *BsysPal=NULL; // Pointer to high-color palette (output format)
unsigned int BsysPalLen=0; // number of colors, must be a power of two

int BsysPalInit()
{
  return 0;
}

int BsysPalExit()
{
  BsysPalLen=0;
  return 0;
}

static INLINE unsigned int CalcCol(unsigned char *ps,int i)
{
  unsigned int a; int r,g,b;
  a=*((unsigned short *)ps);

  // 0bgr BBBB GGGG RRRR
  // To truecolour:
  r=(a&0x000f)<<1;    // r rrr0
  g=(a&0x00f0)>>3;    // g ggg0
  b=(a&0x0f00)>>7;    // b bbb0
  if (a&0x1000) r|=1; // r rrrr
  if (a&0x2000) g|=1; // g gggg
  if (a&0x4000) b|=1; // b bbbb

  r=r*255/31;
  g=g*255/31;
  b=b*255/31;

  return BurnHighCol(r,g,b,i);
}

// Update BsysHigh with the current palette in BsysSrc
int BsysPalUpdate()
{
  unsigned int i; unsigned char *ps; unsigned int *ph;
  for (i=0,ps=BsysPram,ph=BsysPal; i<BsysPalLen; i++,ps+=2,ph++)
  {
    *ph=CalcCol(ps,i);
  }
  return 0;
}

// Write a word to BsysSrc
void BsysPalWriteByte(unsigned int a,unsigned char d)
{
  a^=1;
  a&=(BsysPalLen<<1)-1; // Clip to palette length
  BsysPram[a]=d; // write byte
}

// Write a word to BsysSrc
void BsysPalWriteWord(unsigned int a,unsigned short d)
{
  a&=(BsysPalLen<<1)-2; // Clip to palette length
  *((unsigned short *)(BsysPram+a))=d; // write word
}
