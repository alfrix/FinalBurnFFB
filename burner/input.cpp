// Burner Input module
#include "app.h"

// 1 if current input is on (i.e. emu in foreground)
static int bCinpOkay=0;

int CinpState(int nCode)
{
  // Return off, if current input is turned off
  if (bCinpOkay==0) return 0;
  return DirInputState(nCode); // Read from Direct Input
}

// Read the axis of a joystick
int CinpAxis(int i,int nAxis)
{
  // Return off, if current input is turned off
  if (bCinpOkay==0) return 0;
  return DirInputAxis(i,nAxis); // Read from Direct Input
}

int InputMake()
{
  unsigned int i=0; struct GameInp *pgi=NULL;
  HWND hActive=GetActiveWindow();
  
  bCinpOkay=0;
  if (APP_TAKE_INPUT(hActive)) 
  {
    DirInputStart(); // Poll joysticks etc
    bCinpOkay=1; // Current input is okay
  }

  for (i=0,pgi=GameInp; i<nGameInpCount; i++,pgi++)
  {
    if (pgi->pVal==NULL) continue;
    if (pgi->nInput==0) *(pgi->pVal)=0; // undefined
    if (pgi->nInput==1) *(pgi->pVal)=pgi->nConst; // constant value

    if (pgi->nInput==2)
    {
      // Switch
      int s=0; s=CinpState(pgi->nCode);
      if (pgi->nType==1)
      {
        // Set analog controls to full
        if (s) *(pgi->pVal)=0xff; else *(pgi->pVal)=0x01;
      }
      else
      {
        // Binary controls
        if (s) *(pgi->pVal)=1;    else *(pgi->pVal)=0;
      }
    }

    if (pgi->nInput==3)
    {
      // Map Joystick axis to analog control
      int nJoy=0;
      nJoy=CinpAxis(pgi->nJoy,pgi->nAxis);
      nJoy/=0x200;  nJoy+=0x80;
      if (nJoy<0x01) nJoy=0x01;  if (nJoy>0xff) nJoy=0xff; // clip
      *(pgi->pVal)=(unsigned char)nJoy;
    }

    if (pgi->nInput==4 || pgi->nInput==5)
    {
      *(pgi->pVal)=(unsigned char)(pgi->nSliderValue>>8);
    }

  }
  return 0;
}

// Do one frames worth of keyboard input sliders
int InputTick()
{
  unsigned int i=0; struct GameInp *pgi=NULL;

  for (i=0,pgi=GameInp; i<nGameInpCount; i++,pgi++)
  {
    int nAdd=0;
    if (pgi->nInput!=4 && pgi->nInput!=5) continue; // not a slider

    if (pgi->nInput==4)
    {
      // Get states of the two keys
      nAdd=0;
      if (CinpState(pgi->nSlider[0])) nAdd-=0x100;
      if (CinpState(pgi->nSlider[1])) nAdd+=0x100;
    }

    if (pgi->nInput==5)
    {
      // Get state of the axis
      nAdd=CinpAxis(pgi->nJoy,pgi->nAxis);
      nAdd/=0x100;
    }
    // nAdd is now -0x100 to +0x100

    // Change to slider speed
    nAdd*=pgi->nSliderSpeed; nAdd/=0x100;

    // If centering, attract to center
    if (pgi->nSliderCenter)
    {
      int v;
      v=pgi->nSliderValue-0x8000;
      v*=(pgi->nSliderCenter-1);
      v/= pgi->nSliderCenter;
      v+=0x8000;
      pgi->nSliderValue=v;
    }

    pgi->nSliderValue+=nAdd;
    // Limit slider
    if (pgi->nSliderValue<0x0100) pgi->nSliderValue=0x0100;
    if (pgi->nSliderValue>0xff00) pgi->nSliderValue=0xff00;
  }
  return 0;
}

int InputInit()
{
  return 0;
}

int InputExit()
{
  return 0;
}
