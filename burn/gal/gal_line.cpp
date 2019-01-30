#include "gal.h"
// Galaxy Force HW - line sprites

// GalLineSrc = pointer to cpu M 188000 usually
unsigned char *GalLineSrc=NULL; // Original line sprite table

unsigned char *GalLineData=NULL; // Line Sprite bitmap data
int GalLineDataLen=0; // Length of above

int GalLineDrawSprite(int i,unsigned char *ps)
{
  int Pal;
  (void)i;

  memset(&bss,0,sizeof(bss));
  bss.IncX=bss.IncY=0x200;

  bss.Top=ps[0]; bss.Height=ps[1]-ps[0];
  
  bss.Left=(ps[3]<<8)|ps[2]; bss.Left&=0x1ff; bss.Left-=0xb8;
  bss.Width=ps[4]<<2;
  bss.ClipWidth=bss.Width;
  
  bss.Base=(ps[7]<<9) | (ps[6]<<1);

  if (ps[5]&1)
  {
    bss.Base+=2;
    bss.FlipX=1;
  }
  else
  {
    bss.Base+=bss.Width>>1;
  }

  bss.Base&=0x1ffff;
  bss.Base|=(ps[9]&3)<<17; // Doing it this way allows some wraparound within the bank

  Pal=0x80+(ps[8]&0x7f);
  
  bss.pPal=BsysPal+(Pal<<4);

  bss.Trans=0x8001; //normal transparent mask
  BsysSprDraw();
  return 0;
}

int GalLineDraw()
{
  int i=0; unsigned char *ps;
  // Setup up the BsysSpr for the Line sprites:
  BsysSprData=GalLineData; BsysSprDataLen=GalLineDataLen;
  for (i=0,ps=GalLineSrc; i<0x80; i++,ps+=16)
  {
    if (ps[5]&0x80) break; // end of list
    if (ps[5]&0x40) continue; // skip sprite
    GalLineDrawSprite(i,ps);
  }
  BsysSprData=NULL; BsysSprDataLen=0;

  return 0;
}
