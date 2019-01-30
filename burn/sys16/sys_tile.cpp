#include "sys.h"
// System 16 - Tiles

unsigned char *BsysTileData=NULL; // Pointer to tile bitmap data
int BsysTileDataLen=0; // Length of data    (must be power of 2)

int BsysTilePalBase=0; // Palette Base

// Draw one tile 'Tile' at location pDest on the bitmap
static int OneTile(unsigned char *Dest,int Tile,int Pal)
{
  unsigned int *pBmap;   // pointer to tile bitmap
  unsigned int Pack;    // pixels for tile line
  unsigned char *pLine;  // pointer to dest line
  unsigned int *TilePal; // pointer to tile palette
  int x=0,y=0;
  
  TilePal=BsysPal+((Pal<<3)&(BsysPalLen-8));

  pBmap=(unsigned int *)( BsysTileData+((Tile<<5)&(BsysTileDataLen-0x20)) );
  for (pLine=Dest,y=0; y<8; pLine+=nBurnPitch,y++)
  {
    unsigned char *pPix;
    Pack=*pBmap++;

    for (pPix=pLine,x=0; x<8; pPix+=nBurnBpp,x++)
    {
      int c;
      c=Pack&15;
      if (c) PutPix(pPix,TilePal[Pack&7]);
      
      Pack>>=4;
    }
  }
  return 0;
}

int BsysTextPage(unsigned char *RamTile)
{
  int ty; unsigned char *pLine;

  for (pLine=pBurnDraw,ty=0; ty<28; pLine+=(nBurnPitch<<3),ty++)
  {
    int tx; unsigned char *pCell;

    for (pCell=pLine,tx=0; tx<40; pCell+=(nBurnBpp<<3),tx++)
    {
      int Tile,Pal,mx,my,Off;

      mx=tx-0x28; mx&=0x3f; my=ty&0x3f;
      Off=(my<<7)|(mx<<1); Off&=0xffe;
      Tile=*(unsigned short *)(RamTile+0x10000+Off);

      Pal=(Tile>>9)&7; Pal+=BsysTilePalBase;
      Tile&=0x1ff;

      if (Tile!=0 && Tile!=0x20) OneTile(pCell,Tile,Pal);
    }
  }
  return 0;
}

static int BgPage(unsigned char *RamTile,int PageNum)
{
  int NameSel=0; // Name page select
  int sx=0,sy=0; // Scroll values
  int x=0,y=0;
  int ix=0,iy=0;
  unsigned char *pLine=NULL;

// $E82     : Background page select
// $E92     : Background vertical scroll / column scroll enable
// $E9A     : Background horizontal scroll / row scroll enable

  // Get scroll values
  PageNum<<=1;
  NameSel=*(unsigned short *)(RamTile+0x10e80+PageNum);
  sy     =*(unsigned short *)(RamTile+0x10e90+PageNum);
  sx     =*(unsigned short *)(RamTile+0x10e98+PageNum);
  sx=0xbc-sx;
  sy-=0x100;

  ix=(sx>>3)+1; iy=(sy>>3)+1;
  sx=8-(sx&7); sy=8-(sy&7);

  pLine=pBurnDraw+sy*nBurnPitch;
  for (y=0; y<28-1; y++, pLine+=nBurnPitch<<3)
  {
    unsigned char *Dest=NULL;
    Dest=pLine+sx*nBurnBpp;

    for (x=0; x<40-1; x++, Dest+=nBurnBpp<<3)
    {
      int fx,fy,NamePage,Off,Tile,Pal;
      fx=ix+x; fy=iy+y; // fx/fy=integer tile address

      // Find out which name page this is 
      NamePage=NameSel;
      if ((fx&0x40)==0) NamePage>>=4;
      if ((fy&0x20)==0) NamePage>>=8;
      NamePage&=0xf; NamePage<<=12;
      
      // Find the tile on the page
      fx&=0x3f; fy&=0x1f; Off=(fy<<7)|(fx<<1); Off&=0xffe; Off|=NamePage;
      Tile=*(unsigned short *)(RamTile+Off);

      Pal=(Tile>>6)&0x7f; Pal+=BsysTilePalBase;
      Tile&=0x1fff;

      OneTile(Dest,Tile,Pal);
    }
  }

  return 0;
}

int BsysBgPage(unsigned char *RamTile)
{
  BgPage(RamTile,1); // Background
  BgPage(RamTile,0); // Foreground
  return 0;
}
