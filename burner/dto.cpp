// DirectDraw Three Output (dto) code
#include "app.h"
#include "dto.h"

IDirectDraw2 *DtoDD=NULL; // DirectDraw interface
IDirectDrawSurface *DtoPrim=NULL; // Primary surface
IDirectDrawSurface *DtoBack=NULL; // Back buffer surface

int bDtoStretch=0;
int bDtoTripleBuffer=0; // 1 to enable triple buffering
static int nDispStatus=0;

typedef HRESULT (WINAPI *DDCreateFun) (GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter );
typedef HRESULT (WINAPI *DDCreateClipperFun) ( DWORD dwFlags, LPDIRECTDRAWCLIPPER FAR *lplpDDClipper, IUnknown FAR *pUnkOuter );

int DtoClear(IDirectDrawSurface *Surf)
{
  // Clear a surface to black
  int nRet=0;  DDBLTFX BltFx;
  if (Surf==NULL) return 1;
  memset(&BltFx,0,sizeof(BltFx));
  BltFx.dwSize=sizeof(BltFx);
  nRet=Surf->Blt(NULL,NULL,NULL,DDBLT_COLORFILL,&BltFx);
  if (nRet<1) return 1;
  return 0;
}

int DtoPrimClear()
{
  if (DtoPrim==NULL) return 1;
  DtoClear(DtoPrim); // Clear 1st page
  if (DtoBack==NULL) return 0;
  DtoClear(DtoBack); // Clear 2nd page
  DtoPrim->Flip(NULL,DDFLIP_WAIT); // wait till the flip actually occurs
  DtoClear(DtoBack); // Clear 3rd page
  return 0; 
}

static int DtoPrimInit(int bTriple)
{
  DDSURFACEDESC ddsd; int nRet=0;

  // Create the primary surface
  memset(&ddsd,0,sizeof(ddsd));
  ddsd.dwSize=sizeof(ddsd);
  if (bTriple)
  {
    // Make a primary surface capable of triple bufferring
    ddsd.dwFlags=DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
    ddsd.ddsCaps.dwCaps=DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP |
      DDSCAPS_COMPLEX | DDSCAPS_VIDEOMEMORY;
    ddsd.dwBackBufferCount=2;
  }
  else
  {
    ddsd.dwFlags=DDSD_CAPS;
    ddsd.ddsCaps.dwCaps=DDSCAPS_PRIMARYSURFACE;
  }

  DtoPrim=NULL;
  nRet=DtoDD->CreateSurface(&ddsd,&DtoPrim,NULL);
  if (nRet<0 || DtoPrim==NULL) return 1;

  if (bTriple==0) return 0; // We are done

  // Get the back buffer
  memset(&ddsd.ddsCaps,0,sizeof(ddsd.ddsCaps));
  ddsd.ddsCaps.dwCaps=DDSCAPS_BACKBUFFER; 

  DtoBack=NULL;
  nRet=DtoPrim->GetAttachedSurface(&ddsd.ddsCaps,&DtoBack);
  if (nRet<0 || DtoBack==NULL) { RELEASE(DtoPrim)  return 1; } // Failed to make triple buffer

  DtoPrimClear(); // Clear all three pages
  return 0;
}

static int DtoClipInit()
{
  IDirectDrawClipper *pddcClipper=NULL; // The clipper
  DDCreateClipperFun DDCreateClipper=NULL;
  int nRet=0;

  if (DDrawDll==NULL) return 1;

  DDCreateClipper=(DDCreateClipperFun)GetProcAddress(DDrawDll,"DirectDrawCreateClipper");
  if (DDCreateClipper!=NULL)
  {
    // Create a clipper
    nRet=DDCreateClipper(0,&pddcClipper,NULL);
  }

  if (pddcClipper!=NULL)
  {
    nRet=pddcClipper->SetHWnd(0,hVidWnd);
    if (nRet>=0)
    {
      DtoPrim->SetClipper(pddcClipper);
      RELEASE(pddcClipper)
    }
  }
  return 0;
}

static int DtoExit();

static int DtoInit()
{
  DDCreateFun DDCreate=NULL; int nRet=0;
  IDirectDraw *dd1=NULL;

  // Dynamically get the DirectDrawCreate function
  if (DDrawDll==NULL) return 1;
  DDCreate=(DDCreateFun)GetProcAddress(DDrawDll,"DirectDrawCreate");
  if (DDCreate==NULL) return 1;

  // Create the DirectDraw object
  nRet=DDCreate(NULL,&dd1,NULL);
  if (nRet<0) return 1;  if (dd1==NULL) return 1;

  // Get the higher DirectDraw interface
  DtoDD=NULL;
  nRet=dd1->QueryInterface(IID_IDirectDraw2,(void **)&DtoDD);
  RELEASE(dd1)
  if (DtoDD==NULL) return 1; 

  DtoPrim=NULL; DtoBack=NULL; // No primary surface yet

  // Remember the changes to the display
  nDispStatus=0;
  if (nVidFullscreen)
  {
    char szTemp[64]="";
    nRet=DtoDD->SetCooperativeLevel(hVidWnd,DDSCL_EXCLUSIVE|DDSCL_FULLSCREEN);
    if (nRet>=0) nDispStatus|=1; // changed coop level

    nRet=DtoDD->SetDisplayMode(nVidWidth,nVidHeight,nVidDepth,nVidRefresh,0);
    if (nRet>=0) nDispStatus|=2; // set display mode
    if (nRet<0)
    {
      sprintf (szTemp,"Error setting \"%dx%dx%dbpp (%dhz)\" display mode",
        nVidWidth,nVidHeight,nVidDepth,nVidRefresh);
      AppError(szTemp,1);
    }

    if (bDtoTripleBuffer)
    {
      nRet=DtoPrimInit(1); // Try to make triple buffer
      if (nRet!=0)
      {
        AppError("Could not make Triple Buffer",1);
        // If we fail, fail entirely and make a normal buffer
        RELEASE(DtoBack) RELEASE(DtoPrim)
      }
    }
  }
  else
  {
    DtoDD->SetCooperativeLevel(hVidWnd,DDSCL_NORMAL);
  }

  if (DtoPrim==NULL)
  {
    // No primary surface yet
    nRet=DtoPrimInit(0); // Try normal
    // If we fail, fail entirely
    if (nRet!=0) { RELEASE(DtoBack) RELEASE(DtoPrim) }
  }

  if (DtoPrim==NULL) { DtoExit(); return 1; } // No primary surface

  // Make a clipper in windowed mode
  if (nVidFullscreen==0) DtoClipInit();

  // Init the buffer surfaces
  DtosInit();
  return 0;
}

static int DtoExit()
{
  DtosExit();

  RELEASE(DtoPrim)  DtoBack=NULL; // a single call releases all surfaces

  // Undo the changes we made to the display
  if (nDispStatus&2) DtoDD->RestoreDisplayMode();
  if (nDispStatus&1) DtoDD->SetCooperativeLevel(hVidWnd,DDSCL_NORMAL);
  nDispStatus=0;

  RELEASE(DtoDD)
  return 0;
}

// The video output plugin:
struct VidOut VidOutDto={DtoInit,DtoExit,DtosFrame,DtosPaint};
