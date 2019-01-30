#include "burnint.h"
// CPS Objs (sprites)

// Our copy of the sprite table
static unsigned char *ObjMem=NULL;

static int nMax=0;
static int nGetNext=0;

// Object frames, so you can lag the Objs by nFrameCount-1 frames
struct ObjFrame { int ShiftX,ShiftY; unsigned char *Obj; int Count; };
static int nFrameCount=0;
static struct ObjFrame of[3];

struct ObjRect { int x1,y1,x2,y2; };
static unsigned char PriGrid[24*14]; // Priority grid (used for fix)

int CpsObjInit()
{
  int i=0;
  nMax=0x100; if (Cps==2) nMax=0x400; // CPS2 has more sprites

  if (Cps==2) nFrameCount=3; // CPS2 sprites lagged by 2 frames
  else        nFrameCount=2; // CPS1 sprites lagged by 1 frames

  ObjMem=(unsigned char *)malloc((nMax<<3)*nFrameCount);
  if (ObjMem==NULL) return 1;

  // Set up the frame buffers
  for (i=0;i<nFrameCount;i++)
  { of[i].Obj=ObjMem+(nMax<<3)*i; of[i].Count=0; }

  nGetNext=0;
  return 0;
}

int CpsObjExit()
{
  int i=0;
  for (i=0;i<nFrameCount;i++)
  { of[i].Obj=NULL; of[i].Count=0; }

  if (ObjMem!=NULL) free(ObjMem);  ObjMem=NULL;

  nFrameCount=0;
  nMax=0;
  return 0;
}

static INLINE void GetObjRect(struct ObjRect *por,unsigned short *ps)
{
  int x,y,a,bx,by;
  x=ps[0]; y=ps[1]; a=ps[3];
  x&=0x03ff; x^=0x200; x-=0x200;
  y&=0x03ff; y^=0x200; y-=0x200;
  x-=0x40; y-=0x10;

  // Find out sprite size
  bx=((a>> 8)&15)+1;
  by=((a>>12)&15)+1;

  por->x1=x; por->y1=y;
  por->x2=x+(bx<<4);
  por->y2=y+(by<<4);
}

// Change a Object rect to a 16x16 resolution rect (and clip to screen)
static INLINE void RectToInt(struct ObjRect *por)
{
  por->x1= por->x1    >>4; if (por->x1<0)  por->x1=0;
  por->y1= por->y1    >>4; if (por->y1<0)  por->y1=0;
  por->x2=(por->x2+15)>>4; if (por->x2>24) por->x2=24;
  por->y2=(por->y2+15)>>4; if (por->y2>14) por->y2=14;
}

// Write a priority level to a rectangle
static INLINE void SaveObjRect(struct ObjRect *pir,int nLev)
{
  unsigned char *pl,*pp; int x,y;
  for (y=pir->y1, pl=PriGrid+pir->y1*24+pir->x1; y<pir->y2; y++,pl+=24)
  {
    for (x=pir->x1, pp=pl; x<pir->x2; x++,pp++) *pp=(unsigned char)nLev;
  }
}

// Returns the lowest priority level in a rectangle
static INLINE int LoadObjRect(struct ObjRect *pir)
{
  int nLow=0xff; unsigned char *pl,*pp; int x,y;
  for (y=pir->y1, pl=PriGrid+pir->y1*24+pir->x1; y<pir->y2; y++,pl+=24)
  {
    for (x=pir->x1, pp=pl; x<pir->x2; x++,pp++)
    { int c; c=*pp; if (c<nLow) nLow=c; }
  }
  return nLow;
}

// Quick fixes the problem of high priority sprites earlier in the sprite list
// incorrectly covering low priority ones.
static INLINE void QuickFix(unsigned short *ps)
{
  int v; struct ObjRect or; int nThere;
  v=ps[0]>>13;
  if (v==4) return; // AvsP hack

  GetObjRect(&or,ps); RectToInt(&or); // Get rectangle for this object
  nThere=LoadObjRect(&or); // just get the existing level

  if (v>nThere)
  {
    // A problem sprite: this sprite is higher priority, but is earlier in the list
    // and so should be underneath.
    v=nThere; // equal the level which is already there so this sprite can go under it
    ps[0]&=0x1fff; ps[0]|=v<<13; // Put the level back in
  }

  SaveObjRect(&or,v); // Save the level of this sprite in the priority table
}

// Get CPS sprites into Obj
int CpsObjGet()
{
  int i; unsigned char *pg,*po;
  struct ObjFrame *pof;
  unsigned char *Get=NULL;

  pof=of+nGetNext;

  pof->Count=0; po=pof->Obj;
  pof->ShiftX=-0x40; pof->ShiftY=-0x10;

  if (Cps==2)
  {
    Get=CpsRam708; // CPS2 sprites
    pof->ShiftX=-CpsFrg[0x9];
    pof->ShiftY=-CpsFrg[0xb];
  }
  else
  {
    // CPS1 sprites
    int nOff=0; nOff=*((unsigned short *)(CpsReg+0x00)); nOff<<=8;
    nOff&=0xfff800; Get=CpsFindGfxRam(nOff,0x800);
  }

  if (Get==NULL) return 1;

  memset(PriGrid,0xff,sizeof(PriGrid)); // Reset priority grid

  // Find out how many sprites there are
  for (pg=Get,i=0; i<nMax; pg+=8,i++)
  {
    unsigned short *ps=(unsigned short *)pg;

    if (Cps==2)
    {
      if (ps[1] &0x8000) break; // end of sprite list?
      if (ps[0]==0 && ps[1]==0x0100 && ps[2]==0 && ps[3]==0xff00) break; // Slammasters end of sprite list?
    }
    else        { if (ps[3]==0xff00) break; } // end of sprite list

    if ((ps[0]|ps[3])==0) continue; // sprite blank

    // Okay - this sprite is active:
    memcpy(po,pg,8); // copy it over

    pof->Count++; po+=8;
  }

  // Quick fix the priority problem
  for (po=pof->Obj+((pof->Count-1)<<3); po>=pof->Obj; po-=8)
  {
    unsigned short *ps=(unsigned short *)po;
    QuickFix(ps);
  }

  nGetNext++; if (nGetNext>=nFrameCount) nGetNext=0;

  return 0;
}

// Delay sprite drawing by one frame
int CpsObjDraw(int nLevelFrom,int nLevelTo)
{
  int i; unsigned short *ps; int nPsAdd;
  struct ObjFrame *pof;

  // Draw the earliest frame we have in history
  pof=of+nGetNext;

  // Point to Obj list
  ps=(unsigned short *)pof->Obj; nPsAdd=4;
  if (Cps==1) { ps+=(pof->Count-1)<<2; nPsAdd=-nPsAdd; } // CPS1 is reversed
  
  // Go through all the Objs
  for (i=0; i<pof->Count; i++,ps+=nPsAdd)
  {
    int x,y,n,a,bx,by,dx,dy; int nFlip;

    if (Cps==2)
    {
      int v; v=ps[0]>>13;
      // Check if sprite is between these levels
      if (v<nLevelFrom) continue;
      if (v>nLevelTo) continue;
    }

    x=ps[0]; y=ps[1]; n=ps[2]; a=ps[3];

    if (Cps==2)
    {
      // CPS2 coords are 10 bit signed (-512 to 511)
      x&=0x03ff; x^=0x200; x-=0x200;
      y&=0x03ff; y^=0x200; y-=0x200;
    }
    else
    {
      // CPS1 coords are 9 bit signed?
      if (x>=0x1c0) x-=0x200;
      y&=0x01ff; y^=0x100; y-=0x100;
    }
    x+=pof->ShiftX;
    y+=pof->ShiftY;

    n|=(ps[1]&0x6000)<<3; // high bits of address
    n=CpsFind0(n); // Find real tile address
    if (n<0) continue; // Finder says it's blank

    // Find the palette for the tiles on this sprite
    CpstSetPal(a&0x1f);

    nFlip=(a>>5)&3;
    // Find out sprite size
    bx=((a>> 8)&15)+1;
    by=((a>>12)&15)+1;
          
    // Take care with tiles if the sprite goes off the screen
    if (x<0 || y<0 || x+(bx<<4)>384 || y+(by<<4)>224)
    {
      nCpstType=CTT_16X16 | CTT_CARE;
    }
    else
    {
      nCpstType=CTT_16X16;
    }

    nCpstFlip=nFlip;
    for (dy=0;dy<by;dy++)
    {
      for (dx=0;dx<bx;dx++)
      {
        int ex,ey;
        if (nFlip&1) ex=(bx-dx-1); else ex=dx;
        if (nFlip&2) ey=(by-dy-1); else ey=dy;

        nCpstX=x+(ex<<4); nCpstY=y+(ey<<4);
        nCpstTile=n+(dy<<11)+(dx<<7); 
        CpstOne();
      }
    }

  }
  return 0;
}
