// Burner Game Input
#include "app.h"

// Mapping of PC inputs to game inputs
struct GameInp *GameInp=NULL;
unsigned int nGameInpCount=0;

int GameInpBlank(int bDipSwitch)
{
  unsigned int i=0; struct GameInp *pgi=NULL;
  // Reset all inputs to undefined (even dip switches, if bDipSwitch==1)
  if (GameInp==NULL) return 1;

  // Get the targets in the library for the Input Values
  for (i=0,pgi=GameInp; i<nGameInpCount; i++,pgi++)
  {
    struct BurnInputInfo bii;
    memset(&bii,0,sizeof(bii));
    BurnDrvGetInputInfo(&bii,i);
    if (bDipSwitch==0 && bii.nType==2) continue; // Don't blank the dip switches

    memset(pgi,0,sizeof(*pgi));
    pgi->nType=bii.nType; // store input type
    pgi->pVal= bii.pVal;  // store input pointer to value
  }
  return 0;
}

int GameInpInit()
{
  unsigned int i=0; int nRet=0;
  // Count the number of inputs
  nGameInpCount=0;
  for (i=0;i<0x1000;i++)
  {
    nRet=BurnDrvGetInputInfo(NULL,i);
    if (nRet!=0) { nGameInpCount=i; break; } // end of input list
  }
  
  // Allocate space for all the inputs
  GameInp=(struct GameInp *)malloc(nGameInpCount*sizeof(struct GameInp));
  if (GameInp==NULL) return 1;
  GameInpBlank(1);

  return 0;
}

int GameInpExit()
{
  if (GameInp!=NULL) free(GameInp);  GameInp=NULL;
  nGameInpCount=0;
  return 0;
}

static char *InpToString(struct GameInp *pgi)
{
  static char szString[80];
  if (pgi->nInput==0) return "undefined";
  if (pgi->nInput==1) { sprintf (szString,"constant 0x%.2x",pgi->nConst); return szString; }
  if (pgi->nInput==2) { sprintf (szString,"switch 0x%.2x"  ,pgi->nCode); return szString; }
  if (pgi->nInput==3) { sprintf (szString,"joyaxis %d %d",pgi->nJoy,pgi->nAxis); return szString; }
  if (pgi->nInput==4)
  {
    sprintf (szString,"slider 0x%.2x 0x%.2x speed 0x%x center %d",
      pgi->nSlider[0],pgi->nSlider[1],pgi->nSliderSpeed,pgi->nSliderCenter);
    return szString;
  }
  if (pgi->nInput==5)
  {
    sprintf (szString,"joyslider %d %d speed 0x%x center %d",
      pgi->nJoy,pgi->nAxis,pgi->nSliderSpeed,pgi->nSliderCenter);
    return szString;
  }
  return "unknown";
}

char *InpToDesc(struct GameInp *pgi)
{
  static char szString[64];
  if (pgi->nInput==0) return "";
  if (pgi->nInput==1 && pgi->nConst==0) { return "-"; }
  if (pgi->nInput==2) { return InputCodeDesc(pgi->nCode); }
  return InpToString(pgi); // Just do the rest as they are in the config file
}

static char *SliderInfo(struct GameInp *pgi,char *s)
{
  char *szRet=NULL;
  pgi->nSliderSpeed=0x700; pgi->nSliderCenter=0; //defaults
  pgi->nSliderValue=0x8000; // Put slider in the middle

  szRet=LabelCheck(s,"speed");                  s=szRet; if (s==NULL) return s;
  pgi->nSliderSpeed=(short)strtol(s,&szRet,0);  s=szRet; if (s==NULL) return s;
  szRet=LabelCheck(s,"center");                 s=szRet; if (s==NULL) return s;
  pgi->nSliderCenter=(short)strtol(s,&szRet,0); s=szRet; if (s==NULL) return s;
  return szRet;
}

static int StringToInp(struct GameInp *pgi,char *s)
{
  char *szRet=NULL;
  SKIP_WS(s) // skip whitespace
  szRet=LabelCheck(s,"undefined"); if (szRet!=NULL) { pgi->nInput=0; return 0; }
  szRet=LabelCheck(s,"constant");
  if (szRet!=NULL)
  {
    pgi->nInput=1; s=szRet; pgi->nConst=(unsigned char)strtol(s,&szRet,0);
    return 0;
  }
  szRet=LabelCheck(s,"switch"); if (szRet!=NULL) { pgi->nInput=2; s=szRet; pgi->nCode=(unsigned short)strtol(s,&szRet,0); return 0; }

  // Analog using joystick axis:
  szRet=LabelCheck(s,"joyaxis");
  if (szRet!=NULL)
  {
    pgi->nInput=3; s=szRet; pgi->nJoy=(unsigned char)strtol(s,&szRet,0); if (szRet==NULL) return 1;
    s=szRet; pgi->nAxis=(unsigned char)strtol(s,&szRet,0);
    return 0;
  }

  // Analog using keyboard slider
  szRet=LabelCheck(s,"slider");
  if (szRet!=NULL)
  {
    s=szRet; pgi->nInput=4; pgi->nSlider[0]=0; pgi->nSlider[1]=0; // defaults
    pgi->nSlider[0]=(unsigned short)strtol(s,&szRet,0); s=szRet; if (s==NULL) return 1;
    pgi->nSlider[1]=(unsigned short)strtol(s,&szRet,0); s=szRet; if (s==NULL) return 1;
    szRet=SliderInfo(pgi,s); s=szRet; if (s==NULL) return 1; // Get remaining slider info
    return 0;
  }

  // Analog using joystick slider
  szRet=LabelCheck(s,"joyslider");
  if (szRet!=NULL)
  {
    s=szRet; pgi->nInput=5; pgi->nJoy=0; pgi->nAxis=0; // defaults
    pgi->nJoy= (unsigned char)strtol(s,&szRet,0); s=szRet; if (s==NULL) return 1;
    pgi->nAxis=(unsigned char)strtol(s,&szRet,0); s=szRet; if (s==NULL) return 1;
    szRet=SliderInfo(pgi,s); s=szRet; if (s==NULL) return 1; // Get remaining slider info
    return 0;
  }

  return 1;
}

// Find the input number by 
static unsigned int InputNameToNum(char *szName)
{
  unsigned int i=0;
  for (i=0;i<nGameInpCount;i++)
  {
    struct BurnInputInfo bii;
    BurnDrvGetInputInfo(&bii,i);
    if (stricmp(szName,bii.szName)==0) return i;
  }
  return ~0U;
}

static char *InputNumToName(unsigned int i)
{
  struct BurnInputInfo bii;
  bii.szName=NULL;
  BurnDrvGetInputInfo(&bii,i);
  if (bii.szName==NULL) return "unknown";
  return bii.szName;
}

static int GameInpAutoOne(struct GameInp *pgi,char *szi)
{
  GamcMisc(pgi,szi); // All misc controls
  GamcAnalogKey(pgi,szi,0  ,1); // Player 1 Analog = Kbd   Centering Slider
  GamcAnalogJoy(pgi,szi,1,1,1); // Player 2 Analog = Joy 1 Centering Slider

  GamcPlayer(pgi,szi,0,-1);     // Player 1 Digital = Kbd
  GamcPlayer(pgi,szi,1, 0);     // Player 2 Digital = Joy 1
  GamcPlayer(pgi,szi,2, 1);     // Player 3 Digital = Joy 2
  GamcPlayer(pgi,szi,3, 2);     // Player 4 Digital = Joy 3

  return 0;
}

// Auto-configure any undefined inputs to defaults
int GameInpDefault()
{
  unsigned int i=0; struct GameInp *pgi=NULL;
  // Write input types
  for (i=0,pgi=GameInp; i<nGameInpCount; i++,pgi++)
  {
    struct BurnInputInfo bii;
    if (pgi->nInput!=0) continue; // Already defined - leave it alone

    pgi->nInput=0; // Default to undefined

    // Get the extra info about the input
    bii.szInfo=NULL; BurnDrvGetInputInfo(&bii,i); if (bii.szInfo==NULL) bii.szInfo="";
    GameInpAutoOne(pgi,bii.szInfo);
  }
  return 0;
}

// Write all the GameInps out to config file 'h'
int GameInpWrite(FILE *h)
{
  unsigned int i=0;
  // Write input types
  for (i=0;i<nGameInpCount;i++)
  {
    char *szName=NULL; int nPad=0,j=0;
    szName=InputNumToName(i);
    fprintf (h,"input \"%s\" ",szName);
    nPad=16-strlen(szName); for (j=0;j<nPad;j++) fprintf (h," ");
    fprintf (h,"%s\n",InpToString(GameInp+i));
  }
  return 0;
}

// Read a GameInp in
int GameInpRead(char *szVal)
{
  int nRet=0; char *szQuote=NULL,*szEnd=NULL; unsigned int i=0;

  nRet=QuoteRead(&szQuote,&szEnd,szVal); if (nRet!=0) return 1;
  // Find which input number this refers to
  i=InputNameToNum(szQuote); if (i==~0U) return 1;

  // Parse the input description into the GameInp structure
  StringToInp(GameInp+i,szEnd);
  return 0;
}
