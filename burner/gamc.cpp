// Burner Game Control
#include "app.h"

static char szPlay[4][4]={"p1 ","p2 ","p3 ","p4 "};

#define KEY(x) { pgi->nInput=2; pgi->nCode=(unsigned short)(x); }

// Configure the misc game controls
int GamcMisc(struct GameInp *pgi,char *szi)
{
  // Digital controls
  if (strcmp(szi,"diag"    )==0) KEY(VK_F2)
  if (strcmp(szi,"reset"   )==0) KEY(VK_F3)
  if (strcmp(szi,"p1 start")==0) KEY('1')
  if (strcmp(szi,"p2 start")==0) KEY('2')
  if (strcmp(szi,"p3 start")==0) KEY('3')
  if (strcmp(szi,"p4 start")==0) KEY('4')
  if (strcmp(szi,"p1 coin" )==0) KEY('5')
  if (strcmp(szi,"p2 coin" )==0) KEY('6')
  if (strcmp(szi,"p3 coin" )==0) KEY('7')
  if (strcmp(szi,"p4 coin" )==0) KEY('8')
  if (strcmp(szi,"service" )==0) KEY('9')

  // Dip switches - set to a constant and get the value from the driver
  if (strcmp(szi,"dip")==0) { pgi->nInput=1; pgi->nConst=*(pgi->pVal); }
  return 0;
}

int GamcAnalogKey(struct GameInp *pgi,char *szi,int nPlayer,int nSlide)
{
  char *szSearch=NULL; int k0=0,k1=0;
  szSearch=szPlay[nPlayer&3];
  if (strncmp(szSearch,szi,3)!=0) return 1; // Not our player
  szi+=3;

  if (*szi == 0) return 1;
  if (strcmp(szi+1,"-axis")!=0) return 1;

  // Keyboard
  if (szi[0]=='x') { k0=VK_LEFT; k1=VK_RIGHT; }
  if (szi[0]=='y') { k0=VK_UP; k1=VK_DOWN; }
  if (szi[0]=='z') { k0='F'; k1='V'; }
  if (k0==0) return 1;

  pgi->nInput=4;
  pgi->nSlider[0]=(unsigned char)k0;
  pgi->nSlider[1]=(unsigned char)k1;

  pgi->nSliderValue=0x8000; // Put slider in the middle
  if (nSlide==2)
  {
    // Sliding
    pgi->nSliderSpeed=0x700; pgi->nSliderCenter=0;
  }
  else
  {
    // Sliding (centering)
    pgi->nSliderSpeed=0xe00; pgi->nSliderCenter=10;
  }

  return 0;
}

int GamcAnalogJoy(struct GameInp *pgi,char *szi,int nPlayer,int nJoy,int nSlide)
{
  char *szSearch=NULL;
  szSearch=szPlay[nPlayer&3];
  if (strncmp(szSearch,szi,3)!=0) return 1; // Not our player
  szi+=3;

  if (szi[0]==0) return 1;
  if (strcmp(szi+1,"-axis")!=0) return 1;

  pgi->nJoy=(unsigned char)nJoy;
  // Joystick
  if (strcmp(szi,"x-axis")==0) { pgi->nAxis=0; }
  if (strcmp(szi,"y-axis")==0) { pgi->nAxis=1; }
  if (strcmp(szi,"z-axis")==0) { pgi->nAxis=2; }
  if (strcmp(szi,"r-axis")==0) { pgi->nAxis=3; }

  pgi->nSliderValue=0x8000; // Put slider in the middle
  if (nSlide==2)
  {
    // Sliding
    pgi->nInput=5; pgi->nSliderSpeed=0x700; pgi->nSliderCenter=0;
  }
  else if (nSlide==1)
  {
    // Sliding (centering)
    pgi->nInput=5; pgi->nSliderSpeed=0xe00; pgi->nSliderCenter=10;
  }
  else
  {
    // Absolute
    pgi->nInput=3; 
  }

  return 0;
}

// Set a Game Input to use Device 'nDevice' if it belongs to 'nPlayer'
// -2 = nothing  -1 == keyboard, 0 == joystick 1, 1 == joystick 2 etc...
int GamcPlayer(struct GameInp *pgi,char *szi,int nPlayer,int nDevice)
{
  char *szSearch=NULL; int JoyBase=0;
  szSearch=szPlay[nPlayer&3];
  if (strncmp(szSearch,szi,3)!=0) return 1; // Not our player
  szi+=3;

  if (nDevice<=-2)
  {
    int Ours=0;
    if (strcmp(szi,"up"   )==0) Ours=1;
    if (strcmp(szi,"down" )==0) Ours=1;
    if (strcmp(szi,"left" )==0) Ours=1;
    if (strcmp(szi,"right")==0) Ours=1;
    if (strncmp(szi,"fire ",5)==0) Ours=1;

    if (!Ours) return 1;
    pgi->nInput=1; pgi->nConst=0; // Constant zero
    return 0;
  }
  // Now check the rest of it
  if (nDevice==-1)
  {
    // Keyboard
    if (strcmp(szi,"up"   )==0)  KEY(VK_UP)
    if (strcmp(szi,"down" )==0)  KEY(VK_DOWN)
    if (strcmp(szi,"left" )==0)  KEY(VK_LEFT)
    if (strcmp(szi,"right")==0)  KEY(VK_RIGHT)
    if (strcmp(szi,"fire 1")==0) KEY('Z')
    if (strcmp(szi,"fire 2")==0) KEY('X')
    if (strcmp(szi,"fire 3")==0) KEY('C')
    if (strcmp(szi,"fire 4")==0) KEY('A')
    if (strcmp(szi,"fire 5")==0) KEY('S')
    if (strcmp(szi,"fire 6")==0) KEY('D')
    return 0;
  }

  // Joystick
  JoyBase =0x4000;
  JoyBase|=nDevice<<8;

  if (strcmp(szi,"up"   )==0) KEY(JoyBase+0x01)
  if (strcmp(szi,"down" )==0) KEY(JoyBase+0x02)
  if (strcmp(szi,"left" )==0) KEY(JoyBase+0x03)
  if (strcmp(szi,"right")==0) KEY(JoyBase+0x04)
  if (strncmp(szi,"fire ",5)==0)
  {
    char *szb=szi+5; int nButton;
    nButton=strtol(szb,NULL,0);
    if (nButton>=1) nButton--;
    KEY(JoyBase+0x10+nButton)
  }
  return 0;
}

#undef KEY
