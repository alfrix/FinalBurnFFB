// Run module
#include "app.h"

static unsigned int nNormalLast=0; // Last value of timeGetTime()
static int nNormalFrac=0; // Extra fraction we did

int bRunPause=0;
static int bAppStep=0,bAppFastForward=0;
static int bAppDoFast=0;
static int nFastSpeed=6;

int bReadInputEveryFrame=1;

static int GetInput()
{
  static int i=0;
  InputMake(); // get input

  // Update Input dialog ever 3 frames
  if (i==0) InpdUpdate();
  i++; if (i>=3) i=0;

  // Update Input Set dialog
  InpsUpdate();
  return 0;
}

// With or without sound, run one frame.
// If bDraw is true, it's the last frame before we are up to date, and so we should draw the screen
static int RunFrame(int bDraw,int bPause)
{
  static int bOldStretch=0;
  if (bDraw)
  {
    if (bOldStretch && bDtoStretch==0) // image may have shrunk
    { InvalidateRect(hScrnWnd,NULL,1); UpdateWindow(hScrnWnd); } // blank scrn window
  }
  bOldStretch=bDtoStretch;

  // Get input (even if paused - so it'll show on the dialog)
  if (bReadInputEveryFrame)
  {
    GetInput(); // Get input on all frames
  }
  else
  {
    if (bDraw) GetInput(); // Get input on draw frames only
  }
  
  InputTick(); // Tick one frame of input (for sliders etc)

  if (!bPause)
  {
    if (bDraw)
    {
      VidFrame();  // Do one frame
      VidPaint(0); // paint the screen (no need to validate)
    }
    else
    {    
      // frame skipping
      pBurnDraw=NULL; // Make sure no image is drawn
      if (bDrvOkay)
      {
        BurnDrvFrame();
      }
    }
  }
  return 0;
}

// Callback used when DSound needs more sound
static int RunGetNextSound(int bDraw)
{
  if (DSoundNextSound==NULL) return 1;

  if (bRunPause && bAppStep==0)
  {
    memset(DSoundNextSound,0,nDSoundSegLen<<2); // Write silence into the buffer
    RunFrame(bDraw,1);
    return 0;
  }

  if (bAppDoFast)
  { int i; for (i=0;i<nFastSpeed;i++) RunFrame(0,0); } // do more frames

  // Render frame with sound
  pBurnSoundOut=NULL;
  if (!bAppStep) pBurnSoundOut=DSoundNextSound;
  RunFrame(bDraw,0);
  if (WaveLog!=NULL && pBurnSoundOut!=NULL)
    fwrite(pBurnSoundOut,1,nBurnSoundLen<<2,WaveLog); // log to the file
  pBurnSoundOut=NULL;
  
  bAppStep=0; // done one step

  return 0;
}

int RunIdle()
{
  int nTime=0,nCount=0; int i=0;

  if (bDSoundPlaying)
  {
    // Run with sound
    DSoundCheck();
    return 0;
  }

  // Run without sound
  nTime=timeGetTime()-nNormalLast;
  nCount=(nTime*nAppVirtualFps - nNormalFrac) /10000;
  if (nCount<=0) { Sleep(2); return 0;  } // No need to do anything for a bit

  nNormalFrac+=nCount*10000;
  nNormalLast+=nNormalFrac/nAppVirtualFps;
  nNormalFrac%=nAppVirtualFps;
  
  if (bAppDoFast) nCount*=nFastSpeed; // temporary increase virtual fps
  if (nCount>10) nCount=10; // Limit frame skipping
  if (bRunPause)
  {
    if (bAppStep) nCount=1; // Step one frame
    else
    {
      RunFrame(1,1); // paused
      return 0;
    }
  }
  bAppStep=0;

  for (i=0;i<nCount-1;i++) { RunFrame(0,0); } // mid-frames
  RunFrame(1,0); // end-frame
  return 0;
}

int RunReset()
{
  // reset the speed throttling code
  nNormalLast=0; nNormalFrac=0;
  if (!bDSoundPlaying)
  {
    // run without sound
    nNormalLast=timeGetTime();
  }
  return 0;
}

static int RunInit()
{
  // Try to run with sound
  DSoundGetNextSound=RunGetNextSound;
  DSoundPlay();

  RunReset();
  // Get the first input before we start, because we only
  // get input afterwards on draw frames.
  GetInput();
  return 0;
}

static int RunExit()
{
  nNormalLast=0;
  // Stop sound if it was playing
  DSoundStop();
  return 0;
}

// The main message loop
int RunMessageLoop()
{
  int bRestartVideo=0;

RunAgain:

  if (nVidFullscreen) { while (ShowCursor(0)>=0) ; } // hide cursor in fullscreen

  // Size the window for current game
  ScrnSize();

  RunInit();

  bRestartVideo=0;

  for (;;)
  {
    MSG Msg; int nRet=0;
    nRet=PeekMessage(&Msg,NULL,0,0,PM_REMOVE);
    if (nRet!=0)
    {
      // A message is waiting to be processed
      if (Msg.message==WM_QUIT) break; // Quit program

      if (Msg.message==WM_APP+0)
      {
        bRestartVideo=1;
        break;
      }

      if (Msg.message==WM_KEYDOWN && Msg.wParam==27)
      {
        if (nVidFullscreen)
        {
          nVidFullscreen=0;
          PostMessage(NULL,WM_APP+0,0,0);
        }
      }

      if ((Msg.message==WM_SYSKEYDOWN || Msg.message==WM_KEYDOWN) && (Msg.lParam&0x20000000))
      {
        // An Alt/AltGr-key was pressed
        switch (Msg.wParam)
        {
          case 'C': write_datfile(0); break;
          case 'D': write_datfile(0); break;

          case 'I': bAppFastForward=!bAppFastForward; break;
          case 'O': bAppStep=1; break;
          case 'P': bRunPause=!bRunPause; break;
          case 'L':
            // Cycle through layers
                 if (nBurnLayer==16) nBurnLayer=0xff; // Show all layers
            else if (nBurnLayer>16)  nBurnLayer=1; // Show first layer
            else nBurnLayer<<=1; // Step through layers
            VidFrame(); VidFrame(); // Render the screen (do twice for scanlines)
            VidPaint(0); // Paint the screen
          break;
          case 13 :
            nVidFullscreen=!nVidFullscreen; PostMessage(NULL,WM_APP+0,0,0);
          break;
        }
      }

      // Check for messages for dialogs etc.
      nRet=AppMessage(&Msg);
      if (nRet!=0)
      {
        DispatchMessage(&Msg);
      }
    }
    else
    {
      HWND hActive=GetActiveWindow();

      bAppDoFast=bAppFastForward;

      if (APP_TAKE_INPUT(hActive)) { if (KEY_DOWN(VK_F8)) bAppDoFast=1; }
      // No messages are waiting
      RunIdle();
    }
  }

  if (nVidFullscreen==0) { while (ShowCursor(1)< 0) ; } // show cursor in windowed

  RunExit();

  if (bRestartVideo)
  {
    int nRet=0;
    VidExit();
    MediaExit();

    MediaInit();

    // Reinit the video plugin
    nRet=VidInit();
    if (nRet!=0) { AppError("VidInit Failed",0); }
    goto RunAgain;
  }

  return 0;
}
