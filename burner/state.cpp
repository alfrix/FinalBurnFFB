// Driver Save State module
#include "app.h"

// If bAll=0 save/load all volatile ram to .fs
// If bAll=1 save/load all ram to .fs

static FILE *h=NULL;
// ------------ State len --------------------
static int nTotalLen=0;
static int StateLenAcb(struct BurnArea *pba) { nTotalLen+=pba->nLen; return 0; }

static int StateInfo(int *pnLen,int *pnMinVer,int bAll)
{
  int nMin=0;
  nTotalLen=0;
  BurnAcb=StateLenAcb;
  BurnAreaScan(8,&nMin);  // scan nvram
  if (bAll)
  {
    int m;
    BurnAreaScan(4,&m); // scan volatile ram
    if (m>nMin) nMin=m; // If voltaile ram needs a higher version, up the minimum
  }
  *pnLen=nTotalLen; *pnMinVer=nMin;
  return 0;
}


// ------------ State load --------------------
int StateLoad(char *szName,int bAll)
{
  int nLen=0; int nMin=0,nFileVer=0,nFileMin=0;
  int t1=0,t2=0;
  char *szHeader="fs  ";
  char ReadHeader[4];
  char szForName[9];
  int nFileLen=0;
  unsigned char *Def=NULL; int nDefLen=0; // Deflated version
  int nRet=0;

  if (bDrvOkay==0) return 1;

  StateInfo(&nLen,&nMin,bAll);
  if (nLen<=0) return 1; // No memory to load
  h=fopen(szName,"rb");
  if (h==NULL) return 1;

  fseek(h,0,SEEK_END); nFileLen=ftell(h); fseek(h,0,SEEK_SET);
  if (nFileLen<=0x20) { fclose(h); return 1; } // Not big enough

  memset(ReadHeader,0,4);
  fread(ReadHeader,1,4,h); // Read header
  if (memcmp(ReadHeader,szHeader,4)!=0) { fclose(h); return 2; } // Not the right file type

  fread(&nFileVer,1,4,h); // Version of FB that this file was saved from

  fread(&t1,1,4,h); // Min version of FB that NV  data will work with
  fread(&t2,1,4,h); // Min version of FB that All data will work with
  if (bAll) nFileMin=t2; else nFileMin=t1; // Get the min version number which applies to us

  memset(szForName,0,sizeof(szForName));
  fread(szForName,1,8,h);

  // Check if the save state is okay
  if (strcmp(szForName,BurnDrvText(0))!=0) { fclose(h); return 3; } // The save state is for the wrong game
  if (nFileVer<nMin    ) { fclose(h); return 4; } // Error - this state is too old and cannot be loaded.
  if (nBurnVer<nFileMin) { fclose(h); return 5; } // Error - emulator is too old to load this state

  fseek(h,8,SEEK_CUR); // Skip padding
  // Now at 0x20 in file

  nDefLen=nFileLen-0x20;
  Def=(unsigned char *)malloc(nDefLen); if (Def==NULL) { fclose(h); return 1; }
  memset(Def,0,nDefLen);
  fread(Def,1,nDefLen,h); // Read in deflated block

  fclose(h); h=NULL;

  nRet=StateDecompress(Def,nDefLen,bAll); // Decompress block into driver
  free(Def); // free deflated block
  if (nRet!=0) return 1;

  return 0;
}

// ------------ State save --------------------
int StateSave(char *szName,int bAll)
{
  int nLen=0; int nNvMin=0,nAMin=0; int nZero=0;
  char *szHeader="fs  ";
  char szGame[9];
  unsigned char *Def=NULL; int nDefLen=0; // Deflated version
  int nRet=0;

  if (bDrvOkay==0) return 1;

  StateInfo(&nLen,&nNvMin,0); // Get minimum version for NV part
  nAMin=nNvMin; if (bAll) StateInfo(&nLen,&nAMin,1); // Get minimum version for All data

  if (nLen<=0) return 1; // No memory to load

  h=fopen(szName,"wb");
  if (h==NULL) return 1;

  fwrite(szHeader,1,4,h);  // Header
  fwrite(&nBurnVer,1,4,h); // Version of FB this was saved from
  fwrite(&nNvMin,1,4,h);   // Min version of FB NV  data will work with
  fwrite(&nAMin,1,4,h);    // Min version of FB All data will work with
  memset(szGame,0,sizeof(szGame));
  sprintf (szGame,"%.8s",BurnDrvText(0));
  fwrite(szGame,1,8,h);    // Game name
  fwrite(&nZero,1,4,h); fwrite(&nZero,1,4,h); // padding

  nRet=StateCompress(&Def,&nDefLen,bAll); // Compress block from driver and return deflated buffer
  if (Def==NULL) { fclose(h); return 1; }
  nRet=fwrite(Def,1,nDefLen,h); // Write block to disk
  free(Def); fclose(h); h=NULL; // free deflated block and close file
  
  if (nRet!=nDefLen) return 1; // error writing block to disk

  return 0;
}
// --------------------------------------------
