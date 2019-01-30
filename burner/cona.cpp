// Burner Config file module
#include "app.h"

static char *szConfig="cfg\\finalburn.ini";

// Read in the config file for the whole application
int ConfigAppLoad()
{
  char szLine[256];
  FILE *h=NULL;
  h=fopen(szConfig,"rt");
  if (h==NULL) return 1;

  // Go through each line of the config file
  for (;;)
  {
    int nLen=0;
    if (fgets(szLine,sizeof(szLine),h)==NULL) break; // End of config file

    nLen=strlen(szLine);
    // Get rid of the linefeed at the end
    if (szLine[nLen-1]==10) { szLine[nLen-1]=0; nLen--; }

#define VAR(x) { char *szValue; szValue=LabelCheck(szLine,#x); \
  if (szValue!=NULL) x=strtol(szValue,NULL,0); }
#define STR(x) { char *szValue; szValue=LabelCheck(szLine,#x " "); \
  if (szValue!=NULL) strcpy(x,szValue); }

    // Video
    VAR(nUseSysMemory)
    VAR(VidAuto)
    VAR(nVidWidth) VAR(nVidHeight) VAR(nVidDepth) VAR(nVidRefresh)
    VAR(bDtoStretch)
    VAR(bDtoTripleBuffer)
    VAR(bVidScanlines)
    VAR(bVidScanHalf)
    VAR(bVidScanDelay)

    // Sound
    VAR(nDSoundSamRate)
    VAR(nDSoundSegCount)
    VAR(bAppBassFilter)
    VAR(nBurnSoundChoice)

    // Other
    STR(szAppRomPath)
    VAR(bDrvSaveAll)
    VAR(bReadInputEveryFrame)

#undef STR
#undef VAR
  }

  fclose(h);
  return 0;
}

// Write out the config file for the whole application
int ConfigAppSave()
{
  FILE *h=NULL;
  h=fopen(szConfig,"wt");
  if (h==NULL) return 1;

  // Write title
  fprintf (h,"// " APP_TITLE " v%s Config File\n\n\n",szAppBurnVer);

#define VAR(x) fprintf (h,#x " %d\n",x);
#define STR(x) fprintf (h,#x " %s\n",x);

    fprintf (h,"// Video --------------------------------\n");

    fprintf (h,"\n// If set to one, automatically select the best video mode (ignoring below)\n");
    VAR(VidAuto)
    fprintf (h,"\n// The display mode to use for fullscreen\n");
    VAR(nVidWidth) VAR(nVidHeight) VAR(nVidDepth)
    fprintf (h,"\n// Specify the refresh rate - doesn't work at all on my machine though :(\n");
    VAR(nVidRefresh)
    fprintf (h,"\n// If set to one, allow stretching of the image to any size\n");
    VAR(bDtoStretch)
    fprintf (h,"\n// If set to one, try to use a triple buffer in fullscreen\n");
    VAR(bDtoTripleBuffer)
    fprintf (h,"\n// If set to one, draw scanlines to simulate a low-res monitor\n");
    VAR(bVidScanlines)
    fprintf (h,"\n// If set to one, draw odd scanlines inbetween too at 50%% intensity\n");
    VAR(bVidScanHalf)
    fprintf (h,"\n// If set to one, delay odd scanlines by one frame (reduces flicker)\n");
    VAR(bVidScanDelay)
    fprintf (h,"\n// Buffering method: -1=Autodetect best memory, 0=force video memory, 1=force system memory\n");
    VAR(nUseSysMemory)
    fprintf (h,"\n\n\n");

    fprintf (h,"// Sound --------------------------------\n");
    fprintf (h,"// The sample rate\n");
    VAR(nDSoundSamRate)
    fprintf (h,"\n// Number of frames in sound buffer (= sound lag)\n");
    VAR(nDSoundSegCount)
    fprintf (h,"\n// If set to one, filter the sound output to add more bass\n");
    VAR(bAppBassFilter)
    fprintf (h,"\n// Which sound codes to pass\n");
    VAR(nBurnSoundChoice)
    fprintf (h,"\n\n\n");

    fprintf (h,"// Other --------------------------------\n\n");
    fprintf (h,"// The path to search for rom zips. (Should end with a backslash)\n");
    STR(szAppRomPath)

    fprintf (h,"\n// If set to one, load and save all ram (the state).\n");
    VAR(bDrvSaveAll)
    fprintf (h,"\n// If set to one, read input for every frame even if the graphics is\n");
    fprintf (h,"// frame-skipping (controls could be more responsive, but emu could be slower).\n");
    VAR(bReadInputEveryFrame)
    fprintf (h,"\n\n\n");

#undef STR
#undef VAR

  fclose(h);
  return 0;
}

char *LabelCheck(char *s,char *szLabel)
{
  int nLen;
  if (s==NULL) return NULL;  if (szLabel==NULL) return NULL;
  nLen=strlen(szLabel);
  SKIP_WS(s) // skip whitespace

  if (strncmp(s,szLabel,nLen)!=0) return NULL; // doesn't match
  return s+nLen;
}

int QuoteRead(char **pszQuote,char **pszEnd,char *szSrc) // Read a quoted string from szSrc and point to the end
{
  static char szQuote[QUOTE_MAX]; char *s=NULL; int i=0;
  s=szSrc;
  SKIP_WS(s) // skip whitespace

  if (*s!='\"') return 1; // didn't start with a quote
  s++;

  // Copy the characters into szQuote
  for (i=0; i<QUOTE_MAX-1; i++,s++)
  {
    if (*s=='\"') { s++; break; } if (*s==0) break; // stop copying if end of line or end of string
    szQuote[i]=*s; // copy character to szQuote
  }
  szQuote[i]=0; // zero-terminate szQuote

  if (pszQuote!=NULL) *pszQuote=szQuote;
  if (pszEnd!=NULL)   *pszEnd=s;
  return 0;
}

