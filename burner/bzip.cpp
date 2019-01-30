// Burner Zip module
#include "app.h"

// Zip files to search through
char *szBzipName[BZIP_MAX+1]={NULL};

int nBzipError=0; // non-zero if there is a problem with the opened romset

struct RomFind { unsigned char nState; int nZip; int nPos; }; // State is non-zero if found. 1 = found totally okay.
static struct RomFind *RomFind=NULL;
static int nRomCount=0;
static struct ZipEntry *List=NULL; static int nListCount=0; // List of entries for current zip file
static int nCurrentZip=-1; // Zip which is currently open

StringSet BzipText; // Text which describes any problems with loading the zip
StringSet BzipDetail; // Text which describes in detail any problems with loading the zip

static int bFoundSomeZips=0;
static char *GetFilename(char *szFull)
{
  int i,nLen;
  nLen=strlen(szFull); if (nLen<=0) return szFull;
  for (i=nLen-1;i>=0;i--) if (szFull[i]=='\\' || szFull[i]=='/') return szFull+i+1;
  return szFull;
}

static int FindRomByName(char *szName)
{
  struct ZipEntry *pl; int i;
  // Find the rom named szName in the List
  for (i=0,pl=List; i<nListCount; i++,pl++)
  { if (stricmp(szName,GetFilename(pl->szName))==0) return i; }
  return -1; // couldn't find the rom
}

static int FindRomByCrc(unsigned int nCrc)
{
  struct ZipEntry *pl; int i;
  // Find the rom named szName in the List
  for (i=0,pl=List; i<nListCount; i++,pl++)
  { if (nCrc==pl->nCrc) return i; }
  return -1; // couldn't find the rom
}

// Find rom number i from the pBzipDriver game
static int FindRom(int i)
{
  struct BurnRomInfo ri;
  int Ret=0; int nAka=0;
  memset(&ri,0,sizeof(ri));

  Ret=BurnDrvGetRomInfo(&ri,i);
  if (Ret!=0) return -2; // Failure: no such rom

  // Search for possible names first
  for (nAka=0;nAka<0x10000;nAka++)
  {
    char *szPossibleName=NULL; int Ret=0;
    Ret=BurnDrvGetRomName(&szPossibleName,i,nAka);
    if (Ret!=0) break; // No more rom names

    Ret=FindRomByName(szPossibleName);
    if (Ret>=0) return Ret;
  }

  // Failing that, search by crc (if we have one)
  if (ri.nCrc)
  {
    Ret=FindRomByCrc(ri.nCrc);
    if (Ret>=0) return Ret;
  }

  return -1; // Couldn't find the rom
}

static int RomDescribe(StringSet *pss,struct BurnRomInfo *pri)
{
  pss->Add ("The ");
  if (pri->nType&0x10) pss->Add ("essential ");
  if (pri->nType&0x01) pss->Add ("graphics ");
  if (pri->nType&0x02) pss->Add ("sound ");
  pss->Add ("rom ");
  return 0;
}

// Check the roms to see if they code, graphics etc are complete
static int CheckRoms()
{
  int i=0;
  nBzipError=0; // Assume romset is fine
  for (i=0;i<nRomCount;i++)
  {
    struct BurnRomInfo ri; int nState=0;
    memset(&ri,0,sizeof(ri));
    BurnDrvGetRomInfo(&ri,i); // Find information about the wanted rom
    nState=RomFind[i].nState; // Get the state of the rom in the zip file

    if (nState==0 && ri.nType) // (A type of 0 means empty slot - no rom)
    {
      char *szName="Unknown";
      RomDescribe(&BzipDetail,&ri);
      BurnDrvGetRomName(&szName,i,0);
      BzipDetail.Add("%s was not found.\n",szName);
    }

    if (ri.nType&0x10) // essential rom - without it the game may not run at all
    {
           if (nState==0) nBzipError|=0x01; // not found at all - game probably won't run
      else if (nState==3) nBzipError|=0x01; // incomplete       - game probably won't run
      else if (nState==1)                 ; // fine
      else                nBzipError|=0x10; // crc wrong, or rom too big - may run different
    }
    if (ri.nType&1)    // rom which contains graphics information
    {
           if (nState==0) nBzipError|=0x02; // not found at all
      else if (nState==3) nBzipError|=0x02; // incomplete
      else if (nState==1)                 ; // fine
      else                nBzipError|=0x20; // crc wrong, or rom too big - may look different
    }
    if (ri.nType&2)    // rom which contains sound information
    {
           if (nState==0) nBzipError|=0x04; // not found at all
      else if (nState==3) nBzipError|=0x04; // not found at all
      else if (nState==1)                 ; // sound code rom was fine
      else                nBzipError|=0x40; // crc wrong, or rom too big - may sound different
    }
  }

  if (bFoundSomeZips==0) nBzipError|=0x08; // No data at all!
  return 0;
}

static int BzipBurnLoadRom(unsigned char *Dest,int *pnWrote,int i)
{
  struct BurnRomInfo ri; int nWantZip=0;
  char szText[128]; char *szRomName=NULL;
  int Ret=0;

  if (i<0) return 1;  if (i>=nRomCount) return 1;  

  // Check for messages:
  BurnDrvGetRomName(&szRomName,i,0);
  if (szRomName==NULL) szRomName="unknown";
  sprintf (szText,"Loading %s",szRomName);
  SetWindowText(hScrnWnd,szText);
  for (;;)
  {
    MSG Msg; int Ret=0;
    Ret=PeekMessage(&Msg,NULL,0,0,PM_REMOVE);
    if (Ret==0) break; // no more messages
    DispatchMessage(&Msg);
  }

  if (RomFind[i].nState==0) return 1; // Rom not found in zip at all

  ri.nLen=0; BurnDrvGetRomInfo(&ri,i); // Get length

  nWantZip=RomFind[i].nZip; // Which zip file it is in
  
  if (nCurrentZip!=nWantZip) // If we haven't got the right zip file currently open
  {
    int Ret=0;
    ZipClose(); nCurrentZip=-1;
    Ret=ZipOpen(szBzipName[nWantZip]);
    if (Ret!=0) return 1;
    nCurrentZip=nWantZip;
  }

  // Read in file and return how many bytes we read
  Ret=ZipLoadFile(Dest,ri.nLen,pnWrote,RomFind[i].nPos);
  if (Ret!=0)
  {
    // Error loading from the zip file
    char szTemp[128]="";
    sprintf(szTemp,"%s reading %.30s from %.30s",Ret==2 ? "CRC error" : "Error",
      szRomName,GetFilename(szBzipName[nCurrentZip]));
    AppError(szTemp,1);
    return 1;
  }
  return 0;
}

int BzipOpen()
{
  int Ret=0; int i=0,MemLen=0; int z=0; // Zip name number

  bFoundSomeZips=0; // Haven't found zips yet

  if (szBzipName==NULL) return 1;

  BzipClose(); // Make sure nothing is open

  // reset information strings to nothing
  BzipText.Reset();
  BzipDetail.Reset();

  // Count the number of roms needed
  for (i=0; ;i++) { Ret=BurnDrvGetRomInfo(NULL,i);  if (Ret!=0) break; }
  nRomCount=i;

  if (nRomCount<=0) return 1;

  // Create an array for holding lookups for each rom -> zip entries
  MemLen=nRomCount*sizeof(struct RomFind);
  RomFind=(struct RomFind *)malloc(MemLen); if (RomFind==NULL) return 1;
  memset(RomFind,0,MemLen);

  for (z=0;z<BZIP_MAX;z++)
  {
    char *szName=NULL;
    
    szName=szBzipName[z];
    if (szName==NULL) break; // No more names in the szBzipName list

    ZipClose(); nCurrentZip=-1; // Close the last zip file if open
    // Open the rom zip file
    Ret=ZipOpen(szName);
    if (Ret==0)
    {
      BzipText.Add("Found %s okay.\n",szName); bFoundSomeZips=1;
      nCurrentZip=z; // Mark as open
      // Get the list of entries
      ZipGetList(&List,&nListCount);
    }
    else
    {
      BzipText.Add("Couldn't open %s\n",szName);
    }

    for (i=0;i<nRomCount;i++)
    {
      int nFind; struct BurnRomInfo ri;
      if (RomFind[i].nState==1) continue; // Already found this and it's okay

      memset(&ri,0,sizeof(ri));

      nFind=FindRom(i);
      if (nFind<0) continue; // Couldn't find this rom at all

      RomFind[i].nZip=z; // Remember which zip file it is in
      RomFind[i].nPos=nFind; RomFind[i].nState=1; // Set to found okay

      Ret=BurnDrvGetRomInfo(&ri,i); // get info about the rom
      if (List[nFind].nLen==ri.nLen)
      {
        if (ri.nCrc) // If we know the CRC
        {
          if (List[nFind].nCrc!=ri.nCrc) RomFind[i].nState=2; // Length okay, but CRC wrong
        }
      }
      else if (List[nFind].nLen<ri.nLen) RomFind[i].nState=3; // Too small
      else RomFind[i].nState=4; // Too big
      
      if (RomFind[i].nState!=1) RomDescribe(&BzipDetail,&ri);

      if (RomFind[i].nState==2)
      {
        BzipDetail.Add("%s has a CRC of %.8X not %.8X\n",
          GetFilename(List[nFind].szName),List[nFind].nCrc,ri.nCrc);
      }

      if (RomFind[i].nState==3)
        BzipDetail.Add("%s is %dk which is incomplete. (It should be %dk.)\n",
          GetFilename(List[nFind].szName),List[nFind].nLen>>10,ri.nLen>>10);

      if (RomFind[i].nState==4)
        BzipDetail.Add("%s is %dk which is too big. (It should be %dk.)\n",
          GetFilename(List[nFind].szName),List[nFind].nLen>>10,ri.nLen>>10);
    }

    ZIP_LIST_FREE(List,nListCount)
  }

  // Check the roms to see if they code, graphics etc are complete
  CheckRoms();

  if (bFoundSomeZips)
  {
    if (nBzipError==0) BzipText.Add ("The romset is fine.\n");
    if (nBzipError&0x7) BzipText.Add ("However the romset is INCOMPLETE.\n");
  
    if (nBzipError&0x01)
      BzipText.Add("Essential rom data is missing - the game probably won't run.\n");
    else
    {
      if (nBzipError&0x10)      BzipText.Add("Some essential roms are different. ");
      if (nBzipError&0x02)      BzipText.Add("Graphical data is missing. ");
      else if (nBzipError&0x20) BzipText.Add("Some graphics roms are different. ");
      if (nBzipError&0x04)      BzipText.Add("Sound data is missing. ");
      else if (nBzipError&0x40) BzipText.Add("Some sound roms are different. ");

      if (nBzipError&0x76)      BzipText.Add("\n");
    }
  }

  BurnExtLoadRom=BzipBurnLoadRom; // Okay to call our function to load each rom
  return 0;
}

int BzipClose()
{
  ZipClose(); nCurrentZip=-1; // Close the last zip file if open

  BurnExtLoadRom=NULL; // Can't call our function to load each rom anymore
  nBzipError=0; // reset romset errors

  if (RomFind!=NULL) free(RomFind);  RomFind=NULL;
  
  nRomCount=0;
  return 0;
}
