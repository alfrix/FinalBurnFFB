#include "burnint.h"
// CPS - Draw

unsigned char CpsRecalcPal=0; // Flag - If it is 1, recalc the whole palette

static int LayDraw[4]={-1,-1,-1,-1}; // Order to draw layers ([0]:top -> [3]:most covered)
static int LayMask=0; // Layer mask (1=draw)
static int LayerCont=0;

static int DrawScroll1()
{
  int Off=0,ScrX=0,ScrY=0;
  unsigned char *Find=NULL;

  // Draw Scroll 1
  Off=*((unsigned short *)(CpsSaveReg+0x02)); Off<<=8;

  // Get scroll coordinates
  ScrX=*((unsigned short *)(CpsSaveReg+0x0c)); // Scroll 1 X
  ScrY=*((unsigned short *)(CpsSaveReg+0x0e)); // Scroll 1 Y
  ScrX+=0x40; ScrY+=0x10;

  Off&=0xffc000; Find=CpsFindGfxRam(Off,0x4000); if (Find==NULL) return 1;
  
  CpsScr1Draw(Find,ScrX,ScrY);
  return 0;
}

static INLINE int DrawScroll2Init()
{
  int nScr2Off=0; int n=0;

  // Draw Scroll 2
  nScr2Off =*((unsigned short *)(CpsSaveReg+0x04)); nScr2Off<<=8;
  nScr2Off&=0xffc000; CpsrBase=CpsFindGfxRam(nScr2Off,0x4000);
  if (CpsrBase==NULL) return 1;

  // Get scroll coordinates
  nCpsrScrX=*((unsigned short *)(CpsSaveReg+0x10)); // Scroll 2 X
  nCpsrScrX+=0x40; nCpsrScrX&=0x3ff; 

  nCpsrScrY=*((unsigned short *)(CpsSaveReg+0x12)); // Scroll 2 Y
  nCpsrScrY+=0x10; nCpsrScrY&=0x3ff;

  // Get row scroll information
  n=*((unsigned short *)(CpsSaveReg+0x22));

  CpsrRows=NULL;
  if (n&1)
  {
    int nTab,nStart;
    // Find row scroll table:
    nTab=*((unsigned short *)(CpsSaveReg+0x08)); nTab<<=8; nTab&=0xfff800;
    CpsrRows=(unsigned short *)CpsFindGfxRam(nTab,0x800);

    // Find start offset
    nStart=*((unsigned short *)(CpsSaveReg+0x20));
    nCpsrRowStart=nStart+16;
  }

  // Prepare Scroll 2
  CpsrPrepare();
  return 0;
}

static INLINE int DrawScroll2Exit()
{
  CpsrBase=NULL;
  nCpsrScrX=NULL;  nCpsrScrY=NULL;  CpsrRows=NULL;
  return 0;
}

static INLINE int DrawScroll2Do()
{
  if (CpsrBase==NULL) return 1;
  CpsrRender();
  return 0;
}

static int DrawScroll3()
{
  int nOff=0,nScrX=0,nScrY=0;
  unsigned char *Find=NULL;

  // Draw Scroll 3
  nOff=*((unsigned short *)(CpsSaveReg+0x06)); nOff<<=8;

  // Get scroll coordinates
  nScrX=*((unsigned short *)(CpsSaveReg+0x14)); // Scroll 3 X
  nScrY=*((unsigned short *)(CpsSaveReg+0x16)); // Scroll 3 Y
  nScrX+=0x40; nScrY+=0x10;
  
  nOff&=0xffc000; Find=CpsFindGfxRam(nOff,0x4000); if (Find==NULL) return 1;

  CpsScr3Draw(Find,nScrX,nScrY);
  return 0;
}

// -----------------------------------------------------------------------------

static void LayInit()
{
  // Get bits from Layer Controller
  LayMask=0;
  if (LayerCont&CpsLayEn[1]) LayMask|=2;
  if (LayerCont&CpsLayEn[2]) LayMask|=4;
  if (LayerCont&CpsLayEn[3]) LayMask|=8;

  // Layer control:
  LayDraw[0]=(LayerCont>>12)&3; // top layer
  LayDraw[1]=(LayerCont>>10)&3;
  LayDraw[2]=(LayerCont>> 8)&3;
  LayDraw[3]=(LayerCont>> 6)&3; // bottom layer (most covered up)

  // Check for repeated layers and if there are any, the lower layer is omitted
#define CRP(a,b) if (LayDraw[a]==LayDraw[b]) LayDraw[b]=-1;
  CRP(0,1) CRP(0,2) CRP(0,3) CRP(1,2) CRP(1,3) CRP(2,3)
#undef CRP
}

static void LayDrawCps1()
{
  int i=0;
  // Draw layers
  for (i=3;i>=0;i--)
  {
    switch (LayDraw[i])
    {
      case 0:
      if (nBurnLayer&1) CpsObjDraw(0,7);
      if (i>=3) break;
      // Draw BG-High
      CpsScrHigh=1;
      switch (LayDraw[i+1])
      {
        case 1: if ((nBurnLayer&16) && (LayMask&2)) DrawScroll1();   break;
        case 2: if ((nBurnLayer&16) && (LayMask&4)) DrawScroll2Do(); break;
        case 3: if ((nBurnLayer&16) && (LayMask&8)) DrawScroll3();   break;
      }
      CpsScrHigh=0;
      break;

      case 1: if ((nBurnLayer&2) && (LayMask&2)) DrawScroll1();   break;
      case 2: if ((nBurnLayer&4) && (LayMask&4)) DrawScroll2Do(); break;
      case 3: if ((nBurnLayer&8) && (LayMask&8)) DrawScroll3();   break;
    }
  }
}

static void LayDrawCps2()
{
  int Prio[4]={0,0,0,0};
  int SpriteTo=0; // We have drawn sprites up to this priority number minus one
  int i=0;
  int LayPri=0;

  LayPri=(CpsFrg[4]<<8) | CpsFrg[5]; // Layer priority register at word(400004)
  // Layer priority (if greator or equal to sprite priority, it covers them)
  Prio[0]=(LayPri>> 4)&7;
  Prio[1]=(LayPri>> 8)&7;
  Prio[2]=(LayPri>>12)&7;

  // Draw layers
  for (i=3;i>=0;i--)
  {
    int n=LayDraw[i]; // Find out which layer to draw

    if (n>=1)
    {
      int Pri=0;
      // Find out the priority of this layer
      Pri=Prio[n-1];
      // LayDraw sprites up to this priority
      if (Pri<SpriteTo) SpriteTo=0; // go back for more sprites
      if (nBurnLayer&1) CpsObjDraw(SpriteTo,Pri);
      SpriteTo=Pri+1;
    }

    // Then Draw the scroll layer on top
         if (n==1) { if ((nBurnLayer&2) && (LayMask&2)) DrawScroll1(); }
    else if (n==2) { if ((nBurnLayer&4) && (LayMask&4)) DrawScroll2Do(); }
    else if (n==3) { if ((nBurnLayer&8) && (LayMask&8)) DrawScroll3();   }
  }

  if (SpriteTo<=7) { if (nBurnLayer&1) CpsObjDraw(SpriteTo,7); } // draw the remaining sprites
}

int CpsDraw()
{
  CtvReady(); // Point to correct tile drawing functions
  BurnClearSize(384,224); // Don't use default clear screen because of vertical games
  // Update Palette
  CpsPalUpdate(CpsSavePal,CpsRecalcPal); // recalc whole palette if needed
  CpsRecalcPal=0;

  // Get layer controller value
  LayerCont=*((unsigned short *)(CpsSaveReg+CpsLcReg));
  // Get the order of the layers
  LayInit();
  DrawScroll2Init();
  // Draw layers 
  if (Cps==2) LayDrawCps2();
  else        LayDrawCps1();
  DrawScroll2Exit();
  return 0;
}
