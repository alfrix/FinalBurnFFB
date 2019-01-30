#include "sys.h"
// System 16 - Main

static int DrawSprite(unsigned short *Spr)
{
  int Bank=0,Pal=0;
  memset(&bss,0,sizeof(bss));

  bss.Top=Spr[0]&0xff;
  bss.Height=(Spr[0]>>8)-(Spr[0]&0xff);

  bss.Left=Spr[1]&0x1ff; bss.Left-=0xb6;
  bss.Width=(Spr[2]&0xff)<<2;
  bss.ClipWidth=bss.Width;

  // Find the bitmap in the rom data
  Bank=Spr[4]>>8;
  Bank=BsysSprBank[Bank&0xf];
  bss.Base=Bank<<17;

  bss.Base+=Spr[3]<<1;
  bss.Base+=bss.Width>>1;

  Pal=Spr[4]&0x3f; Pal|=0x40; Pal<<=4;
  bss.pPal=BsysPal+Pal;

  // Now that the sprite is in a standard format, draw it
  bss.Trans=0x8001; //normal transparent mask

  bss.IncX=Spr[5]&0x3ff; bss.IncX>>=1; bss.IncX+=0x200;
  bss.IncY=bss.IncX;

  if (Spr[2]&0x100) { bss.Base+=2-(bss.Width>>1); bss.FlipX=1; }

  if (bss.Width&0x200) { bss.Width=0x400-bss.Width; bss.IncY=-bss.IncY; } // Vertically Flipped

  BsysSprDraw();

  return 0;
}

static int BsysDrawObj()
{
  int i=0; unsigned char *ps=NULL;

  ps=BsysObj;
  for (i=0; i<0x100; ps+=0x10,i++)
  {
    if (ps[5]&0x80) break; // End of sprite list
    DrawSprite((unsigned short *)ps);
  }

  return 0;
}

int BsysDraw()
{
  // --------------- Draw graphics -------------------
  BsysPalUpdate(); // Update the palette

  BurnClearScreen();
  BsysBgPage(BsysTile); // Draw tile layer
  BsysDrawObj(); // Draw sprites
  BsysTextPage(BsysTile); // Draw text layer
  return 0;
}
