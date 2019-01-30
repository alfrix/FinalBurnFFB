// Burner Config for Game file module
#include "app.h"

static char *GameConfigName()
{
  // Return the path of the config file for this game
  static char szName[32];
  sprintf (szName,"cfg\\%.8s.ini",BurnDrvText(0));
  return szName;
}

// Read in the config file for the game
int ConfigGameLoad()
{
  char szLine[256];
  FILE *h=NULL;

  h=fopen(GameConfigName(),"rt");
  if (h==NULL) return 1;

  // Go through each line of the config file
  for (;;)
  {
    int nLen=0; char *szValue=NULL;
    if (fgets(szLine,sizeof(szLine),h)==NULL) break; // End of config file

    nLen=strlen(szLine);
    // Get rid of the linefeed at the end
    if (szLine[nLen-1]==10) { szLine[nLen-1]=0; nLen--; }

    szValue=LabelCheck(szLine,"input");
    if (szValue!=NULL) GameInpRead(szValue);
  }

  fclose(h);
  return 0;
}

// Write out the config file for the game
int ConfigGameSave()
{
  FILE *h=NULL;

  h=fopen(GameConfigName(),"wt");
  if (h==NULL) return 1;

  // Write title
  fprintf (h,"// " APP_TITLE " v%s Config File for the game %s\n\n",
    szAppBurnVer,BurnDrvText(1));

  GameInpWrite(h);

  fclose(h);
  return 0;
}
