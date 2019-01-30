// Driver State Compression module
#include "app.h"
#include "zlib.h"

static unsigned char *Comp=NULL; static int nCompLen=0; // Compressed data buffer
static int nCompFill=0; // How much of the buffer has been filled so far
static z_stream Zstr; // Deflate stream

static int CompEnlarge(int nAdd)
{
  void *NewMem=NULL;
  // Need to make more room in the compressed buffer
  NewMem=realloc(Comp,nCompLen+nAdd);
  if (NewMem==NULL) return 1;
  Comp=(unsigned char *)NewMem;
  memset(Comp+nCompLen,0,nAdd); // Blank the new memory
  nCompLen+=nAdd;
  return 0;
}

static int CompGo(int bFinish)
{
  int bNoRoom=0; int nResult=0;
  int nAvailOut=0; int nRet=0;
GoAgain:
  // Point to the remainder of out buffer
  Zstr.next_out=Comp+nCompFill;
  nAvailOut=nCompLen-nCompFill; if (nAvailOut<0) nAvailOut=0;
  Zstr.avail_out=nAvailOut;

  // Try to deflate into the buffer (there may not be enough room though)
  if (bFinish) nResult=deflate(&Zstr,Z_FINISH);  // Try to deflate and finish
  else         nResult=deflate(&Zstr,0);         // deflate

  if (bFinish)
  { if (nResult!=Z_OK && nResult!=Z_STREAM_END) return 1; } // Error
  else
  { if (nResult!=Z_OK) return 1; } // Error

  nCompFill=Zstr.next_out-Comp; // Update how much has been filled

  bNoRoom=0;
  if (bFinish==0 && Zstr.avail_out<=0) bNoRoom=1; // output buffer is full
  if (bFinish    && nResult==Z_OK)     bNoRoom=1; // output buffer is full (there wasn't room to finish)

  if (bNoRoom)
  {
    // There wasn't enough room in the output buffer
    nRet=CompEnlarge(4*1024); if (nRet!=0) return 1;
    goto GoAgain;
  }
  return 0;
}

static int StateCompressAcb(struct BurnArea *pba)
{
  // Set the data as the next available input
  Zstr.next_in=(unsigned char *)pba->Data;
  Zstr.avail_in=pba->nLen;
  CompGo(0); // Compress this Area
  Zstr.avail_in=0;
  Zstr.next_in=NULL;
  return 0;
}

// Compress a state using deflate
int StateCompress(unsigned char **pDef,int *pnDefLen,int bAll)
{
  int nRet=0; void *NewMem=NULL;
  memset(&Zstr,0,sizeof(Zstr));

  Comp=NULL; nCompLen=0; nCompFill=0; // Begin with a zero-length buffer
  nRet=CompEnlarge(8*1024);
  if (nRet!=0) return 1;

  nRet=deflateInit(&Zstr,Z_DEFAULT_COMPRESSION);
  BurnAcb=StateCompressAcb; // callback our function with each area
            BurnAreaScan(8|1,NULL);  // scan nvram,        read (from driver -> compress)
  if (bAll) BurnAreaScan(4|1,NULL);  // scan volatile ram, read (from driver -> compress)

  // Finish off
  CompGo(1);

  nRet=deflateEnd(&Zstr);

  // Size down
  NewMem=realloc(Comp,nCompFill);
  if (NewMem!=NULL) { Comp=(unsigned char *)NewMem; nCompLen=nCompFill; }

  // Return the buffer
  if (pDef    !=NULL) *pDef    =Comp;
  if (pnDefLen!=NULL) *pnDefLen=nCompFill;
  return 0;
}
// -----------------------------------------------------------

static int StateDecompressAcb(struct BurnArea *pba)
{
  int nRet=0;
  Zstr.next_out=(unsigned char *)pba->Data;
  Zstr.avail_out=pba->nLen;
  nRet=inflate(&Zstr,Z_SYNC_FLUSH);
  Zstr.avail_out=0;
  Zstr.next_out=NULL;
  return 0;
}

// Decompress a state using inflate
int StateDecompress(unsigned char *Def,int nDefLen,int bAll)
{
  int nRet=0;
  memset(&Zstr,0,sizeof(Zstr));
  nRet=inflateInit(&Zstr);

  // Set all of the buffer as available input
  Zstr.next_in=(unsigned char *)Def;
  Zstr.avail_in=nDefLen;

  BurnAcb=StateDecompressAcb; // callback our function with each area
            BurnAreaScan(8|2,NULL);  // scan nvram,        write (to driver <- decompress)
  if (bAll) BurnAreaScan(4|2,NULL);  // scan volatile ram, write (to driver <- decompress)

  nRet=inflateEnd(&Zstr);
  memset(&Zstr,0,sizeof(Zstr));

  return 0;
}
