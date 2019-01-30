// Video Output - (calls all the Vid Out plugins)
#include "app.h"

HWND hVidWnd=NULL; // The window to be used for video
int VidAuto=1;
int nVidWidth=0,nVidHeight=0,nVidDepth=0,nVidRefresh=0;
int nVidFullscreen=0;
int bVidScanlines=0;
int bVidScanHalf=1;
int bVidScanDelay=1;
static unsigned int nVidSelect=0; // Which video output is selected

extern struct VidOut
  VidOutDto,
  VidOutDisp
  ;

static struct VidOut *pVidOut[]=
{
  &VidOutDto,
  &VidOutDisp,
};

#define VID_LEN (sizeof(pVidOut)/sizeof(pVidOut[0]))

// Forward to VidOut functions
int VidInit()
{
  if (nVidSelect>=VID_LEN) return 1;

  if (VidAuto)
  {
    int w=0,h=0;
    if (bDrvOkay) BurnDrvGetScreen(&w,&h);

    nVidWidth=640; nVidHeight=480; nVidDepth=16;

    if (w==224 && h==384)
    {
      // CPS vertical game
      if (bVidScanlines) { nVidWidth=1024; nVidHeight=768; nVidDepth=16; }
      else               { nVidWidth=512;  nVidHeight=384; nVidDepth=16; }
    }
  }

  return pVidOut[nVidSelect]->Init();
}

int VidExit()
{
  int Ret=0;
  if (nVidSelect>=VID_LEN) return 1;
  Ret=pVidOut[nVidSelect]->Exit();

  if (VidAuto)
  {
    nVidWidth=0; nVidHeight=0; nVidDepth=0;
  }
  return Ret;
}

int VidFrame()
{
  if (nVidSelect>=VID_LEN) return 1;
  return pVidOut[nVidSelect]->Frame();
}

// If bValidate is 1, the video code should use ValidateRect
// to validate the hVidWnd rectangle it draws.
int VidPaint(int bValidate)
{
  if (nVidSelect>=VID_LEN) return 1;
  return pVidOut[nVidSelect]->Paint(bValidate);
}
