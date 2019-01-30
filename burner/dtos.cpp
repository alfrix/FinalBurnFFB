// DirectDraw Three Output Screen (dtos) code
#include "app.h"
#include "dto.h"

static int nDtosWidth=0,nDtosHeight=0; // screen size
static int nDtosDepth=0,nDtosBpp=0;
static int bDtosScan=0;
static unsigned char *DtosMem=NULL; // Memory buffer
static int nDtosMemPitch=0; // Memory buffer pitch
IDirectDrawSurface *pddsDtos=NULL; // The screen surface
static int nHalfMask=0;

static INLINE int GetSurfaceDepth(IDirectDrawSurface *pddsSurf)
{
  int nDepth=8; int nRet=0;
  DDPIXELFORMAT ddpf;
  if (pddsSurf==NULL) return nDepth;

  // Find out the pixelformat of the screen surface
  memset(&ddpf,0,sizeof(ddpf));
  ddpf.dwSize=sizeof(ddpf);
  nRet=pddsSurf->GetPixelFormat(&ddpf);
  
  if (nRet>=0)
  {    
    nDepth=ddpf.dwRGBBitCount;
    if (nDepth==16 && ddpf.dwGBitMask==0x03e0) nDepth=15;
  }
  return nDepth;
}

// Create a buffer in normal memory
static int DtosMakeMem()
{
  int nMemLen=0;
  nDtosMemPitch=nDtosWidth*nDtosBpp;
  nMemLen=nDtosHeight*nDtosMemPitch;
  DtosMem=(unsigned char *)malloc(nMemLen);
  if (DtosMem==NULL) return 1;
  memset(DtosMem,0,nMemLen);
  return 0;
}

static int AutodetectUseSys()
{
  // Try to autodetect the best secondary buffer type to use, based on the cards capabilities
  DDCAPS ddc;
  if (DtoDD==NULL) return 1;
  memset(&ddc,0,sizeof(ddc)); ddc.dwSize=sizeof(ddc); DtoDD->GetCaps(&ddc,NULL);

  // If it can do a hardware stretch use video memory
  if (ddc.dwCaps&DDCAPS_BLTSTRETCH) return 0;
  // Otherwise use system memory:
  return 1;
}

// Create a secondary DD surface for the screen
static int DtosMakeSurf()
{
  int nRet=0; int nUseSys=0;
  DDSURFACEDESC ddsd;
  if (DtoDD==NULL) return 1;

  nUseSys=nUseSysMemory;
  if (nUseSys<0) { nUseSys=AutodetectUseSys(); }

TryAgain:
  memset(&ddsd,0,sizeof(ddsd));
  ddsd.dwSize=sizeof(ddsd);
  ddsd.dwFlags=DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT;
  ddsd.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN;
       if (nUseSys==0) ddsd.ddsCaps.dwCaps|=DDSCAPS_VIDEOMEMORY; 
  else if (nUseSys==1) ddsd.ddsCaps.dwCaps|=DDSCAPS_SYSTEMMEMORY;

  ddsd.dwWidth=nDtosWidth; ddsd.dwHeight=nDtosHeight;
  if (bDtosScan) ddsd.dwHeight<<=1;

  nRet=DtoDD->CreateSurface(&ddsd,&pddsDtos,NULL);
  if (nRet<0 && nUseSys!=1) { nUseSys=1; goto TryAgain; } // Try again in system memory
  if (nRet<0) return 1;  if (pddsDtos==NULL) return 1;

  DtoClear(pddsDtos);

  return 0;
}

int DtosInit()
{
  int nRet=0;
  if (DtoDD==NULL) return 1;  if (bDrvOkay==0) return 1;
  bDtosScan=bVidScanlines?1:0;

  // Get the game screen size
  BurnDrvGetScreen(&nDtosWidth,&nDtosHeight);

  nDtosDepth=GetSurfaceDepth(DtoPrim); // Get color depth of primary surface
  nDtosBpp=(nDtosDepth+7)>>3;
  nBurnBpp=nDtosBpp; // Set Burn library Bytes per pixel
  // Make the mask to mask out all but the lowest intensity bit
  nHalfMask=0;
       if (nDtosDepth==15) nHalfMask=0xfbdefbde;
  else if (nDtosDepth==16) nHalfMask=0xf7def7de;
  else nHalfMask=0xfefefefe;

  // Use our callback to get colors:
  SetBurnHighCol(nDtosDepth);
  // Recalculate the whole palette next time BurnDraw is called
  // This will use our callback for the whole palette
  BurnRecalcPal();

  // Make the normal memory buffer
  nRet=DtosMakeMem(); if (nRet!=0) { DtosExit(); return 1; }

  // Make the DirectDraw secondary surface
  nRet=DtosMakeSurf(); if (nRet!=0) { DtosExit(); return 1; }
  return 0;
}

int DtosExit()
{
  RELEASE(pddsDtos)
  nDtosMemPitch=0;
  if (DtosMem!=NULL) free(DtosMem);  DtosMem=NULL;

  nHalfMask=0; nDtosBpp=0; nDtosDepth=0;
  nDtosWidth=0; nDtosHeight=0; 

  nBurnBpp=0; // Blank the Burn library Bytes per pixel
  bDtosScan=0;
  return 0;
}

// Copy DtosMem to pddsDtos
static int DtosMemToSurf(int nField,int nHalf)
{
  DDSURFACEDESC ddsd; int nRet=0;
  unsigned char *pd=NULL,*ps=NULL; int y=0;
  unsigned char *Surf=NULL; int nPitch=0;

  if (pddsDtos==NULL) return 1;

  // Lock the surface so we can write to it
  memset(&ddsd,0,sizeof(ddsd));
  ddsd.dwSize=sizeof(ddsd);
  nRet=pddsDtos->Lock(NULL,&ddsd,DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT,NULL);
  if (ddsd.lpSurface==NULL) return 1;

  Surf=(unsigned char *)ddsd.lpSurface;
  nPitch=ddsd.lPitch;
  if (bDtosScan)
  {
    if (nField) Surf+=nPitch; // copy to odd fields
    nPitch<<=1;
  }

  pd=Surf; ps=DtosMem;
  for (y=0;y<nDtosHeight; y++,pd+=nPitch,ps+=nDtosMemPitch)
  {
    if (nHalf==0)
    {
      memcpy(pd,ps,nDtosMemPitch);
    }
    else
    {
      unsigned char *psEnd,*pdp,*psp;
      psEnd=ps+nDtosMemPitch;
      pdp=pd; psp=ps;
      do
      {
        unsigned int t;
        t=*((unsigned int *)(psp));
        t=(t&nHalfMask)>>1;
        *((unsigned int *)(pdp))=t;
        psp+=4; pdp+=4;
      }
      while (psp<psEnd);
    }

  }

  nRet=pddsDtos->Unlock(NULL);
  return 0;
}

// Run one frame and render the screen
int DtosFrame()
{
  if (DtosMem==NULL) return 1;

  // delayed 50% scanlines:
  if (bDtosScan && bVidScanHalf && bVidScanDelay   ) DtosMemToSurf(0,1);

  pBurnDraw=DtosMem; nBurnPitch=nDtosMemPitch;

  if (bDrvOkay)
  {
    BurnDrvFrame(); // Run one frame and draw the screen
  }
  else
  {
    // Blank the screen
    memset(DtosMem,0,nDtosMemPitch*nDtosHeight);
  }
  pBurnDraw=NULL; nBurnPitch=0;

  // non-delayed 50% scanlines:
  if (bDtosScan && bVidScanHalf && bVidScanDelay==0) DtosMemToSurf(0,1);

  DtosMemToSurf(1,0); // Copy the memory buffer to the directdraw buffer for later blitting
  return 0;
}

// Paint the Dtos surface onto the primary surface
int DtosPaint(int bValidate)
{
  int nRet=0;
  RECT Dest={0,0,0,0};

  if (DtoPrim==NULL) return 1;
  if (pddsDtos==NULL) return 1;

  GetClientScreenRect(hVidWnd,&Dest);
  if (bPanelDlgIsShown) Dest.top+=nPanelHeight;

  if (bDtoStretch==0) IntegerScale(&Dest,nDtosWidth,nDtosHeight<<bDtosScan);

  if (pddsDtos->IsLost()) { pddsDtos->Restore(); } // Restore surface if lost

  if (DtoPrim->IsLost())
  {
    DtoPrim->Restore();  // Restore surface if lost
    if (nVidFullscreen) DtoPrimClear(); // Clear the edges
  }

  if (DtoBack!=NULL)
  {
    // Triple bufferring
    nRet=DtoBack->Blt(&Dest,pddsDtos,NULL,DDBLT_WAIT,NULL);
    if (nRet<0) return 1;
    nRet=DtoPrim->Flip(NULL,DDFLIP_WAIT);
  }
  else
  {
    // Normal
    nRet=DtoPrim->Blt(&Dest,pddsDtos,NULL,DDBLT_WAIT,NULL);
    if (nRet<0) return 1;
  }

  if (bValidate)
  {
    // Validate the rectangle we just drew
    POINT c={0,0};
    ClientToScreen(hVidWnd,&c);
    Dest.left-=c.x; Dest.right-=c.x;
    Dest.top-=c.y; Dest.bottom-=c.y;
    ValidateRect(hVidWnd,&Dest);
  }
  return 0;
}

