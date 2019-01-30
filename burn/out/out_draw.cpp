#include "out.h"
// Out Run - Draw

static int DrawSprite(unsigned char *ps)
{
  int Pal;
  bss.Top=ps[0]; bss.Height=ps[11]+1;
  bss.Left=((ps[5]<<8)|ps[4])&0x1ff; bss.Left-=0xc0;
  bss.Width=(ps[5]&0xfe)<<2;
  bss.ClipWidth=bss.Width;
  Pal=ps[10]&0x7f;

  Pal+=0x80; Pal<<=4;
  bss.pPal=BsysPal+Pal;

  // Find the bitmap in the rom data
  bss.Base=((ps[1]&0x06)<<17) | (ps[3]<<10) | (ps[2]<<2);

  // Find the zoom values
  {
    int ZoomX,ZoomY;
    ZoomX=((ps[7]<<8)|ps[6])&0xfff;
    ZoomY=((ps[9]<<8)|ps[8])&0xfff;
    if (ZoomX==0x32c) ZoomX=0x3c2; // mistake in original machine? !
    if (ZoomY==0) ZoomY=ZoomX;
    bss.IncX=ZoomX; bss.IncY=ZoomY;
  }

  bss.FlipX=0;
  if ((ps[9]&0x40)==0) { bss.Base+=4-(bss.Width>>1); bss.FlipX=1; }
  bss.DrawToLeft=((ps[9]&0x20)==0);
  bss.DrawToTop =0;

  // Now that the sprite is in a standard format, draw it
  bss.Trans=0x8001; //normal transparent mask

  BsysSprDraw();

  return 0;
}

static int OutSprDraw()
{
  int i=0; unsigned char *ps=NULL;
  ps=OutRam13;
  for (i=0; i<0x80; ps+=0x10,i++)
  {
    if (ps[0]==0xff && ps[1]==0xff) break; // end of sprite list
    if (ps[1]&0x40) continue; // sprite not active
    DrawSprite(ps);
  }
  return 0;
}

int OutDrawDo()
{
  unsigned char Restore[4]; int Pos=0;

  BsysPalUpdate(); // Update the palette
  OutGrDraw();
  OutSprDraw();

  // Put text to display high and low gear
  Pos=0x10cb2;
  memcpy(Restore,OutRam10+Pos,4);
  OutRam10[Pos+0]=(OutComb&0x10) ? 'L' : 'H';
  OutRam10[Pos+2]=(OutComb&0x10) ? 'O' : 'I';
  OutRam10[Pos+1]=0x05;
  OutRam10[Pos+3]=0x05;

  BsysTextPage(OutRam10); // Draw text
  memcpy(OutRam10+Pos,Restore,4);
  return 0;
}
