// 68000 (Sixty Eight K) Interface
#include "../burnint.h"

static int nSekCount=0; // Number of allocated 68000s
struct SekExt *SekExt=NULL,*pSekExt=NULL;


int nSekActive=0; // The cpu which is currently being emulated

// ======== Default 68000 memory accesses ================
static unsigned char DefReadByte(unsigned int a) { (void)a; return 0; }
static void DefWriteByte(unsigned int a,unsigned char d) { (void)a; (void)d; }
static unsigned short DefReadWord(unsigned int a) { SEK_DEF_READ_WORD(a) }
static void DefWriteWord(unsigned int a,unsigned short d) { SEK_DEF_WRITE_WORD(a,d) }
static unsigned int DefReadLong(unsigned int a) { SEK_DEF_READ_LONG(a) }
static void DefWriteLong(unsigned int a,unsigned int d) { SEK_DEF_WRITE_LONG(a,d) }
// ======== End of Default 68000 memory accesses ================

// Mapped Memory lookup
#define FIND_R(x) pSekExt->MemMap[ (x>>SEK_SHIFT)&SEK_MASK            ]
// Mapped Memory lookup (+SEK_WADD   for write)
#define FIND_W(x) pSekExt->MemMap[((x>>SEK_SHIFT)&SEK_MASK)+SEK_WADD  ]
// Mapped Memory lookup (+SEK_WADD*2 for fetch)
#define FIND_F(x) pSekExt->MemMap[((x>>SEK_SHIFT)&SEK_MASK)+SEK_WADD*2]

// ======== General 68000 memory accesses ================
static INLINE unsigned char ReadByte(unsigned int a)
{
  unsigned char *pr; pr=FIND_R(a);
  if (pr!=NULL) { a^=1; return pr[a&SEK_PAGEM]; }
  return pSekExt->ReadByte(a);
}

static INLINE unsigned char FetchByte(unsigned int a)
{
  unsigned char *pr; pr=FIND_F(a);
  if (pr!=NULL) { a^=1; return pr[a&SEK_PAGEM]; }
  return pSekExt->ReadByte(a);
}

static INLINE void WriteByte(unsigned int a,unsigned char d)
{
  unsigned char *pr; pr=FIND_W(a);
  if (pr!=NULL) { a^=1; pr[a&SEK_PAGEM]=(unsigned char)d; return; }
  pSekExt->WriteByte(a,d);
}

static INLINE unsigned short ReadWord(unsigned int a)
{
  unsigned char *pr; pr=FIND_R(a);
  if (pr!=NULL) { return *((unsigned short *)(pr+(a&SEK_PAGEM))); }
  return pSekExt->ReadWord(a);
}

static INLINE unsigned short FetchWord(unsigned int a)
{
  unsigned char *pr; pr=FIND_F(a);
  if (pr!=NULL) { return *((unsigned short *)(pr+(a&SEK_PAGEM))); }
  return pSekExt->ReadWord(a);
}

static INLINE void WriteWord(unsigned int a,unsigned short d)
{
  unsigned char *pr; pr=FIND_W(a);
  if (pr!=NULL) { *((unsigned short *)(pr+(a&SEK_PAGEM)))=(unsigned short)d; return; }
  pSekExt->WriteWord(a,d);
}

static INLINE unsigned int ReadLong(unsigned int a)
{
  unsigned char *pr; pr=FIND_R(a);
  if (pr!=NULL) { unsigned int r; r=*((unsigned int *)(pr+(a&SEK_PAGEM))); r=((r&0xffff0000)>>16)|((r&0x0000ffff)<<16); return r; }
  return pSekExt->ReadLong(a);
}

static INLINE unsigned int FetchLong(unsigned int a)
{
  unsigned char *pr; pr=FIND_F(a);
  if (pr!=NULL) { unsigned int r; r=*((unsigned int *)(pr+(a&SEK_PAGEM))); r=((r&0xffff0000)>>16)|((r&0x0000ffff)<<16); return r; }
  return pSekExt->ReadLong(a);
}

static INLINE void WriteLong(unsigned int a,unsigned int d)
{
  unsigned char *pr; pr=FIND_W(a);
  if (pr!=NULL) { d=((d&0xffff0000)>>16)|((d&0x0000ffff)<<16); *((unsigned int *)(pr+(a&SEK_PAGEM)))=d; return; }
  pSekExt->WriteLong(a,d);
}

// ======== End of General 68000 memory accesses ================

// ======================= A68K =================================
#ifdef EMU_A68K
struct A68KContext *SekRegs=NULL;

extern "C" unsigned char *OP_ROM=NULL,*OP_RAM=NULL;
extern "C" int m68k_ICount=0;
extern "C" unsigned int mem_amask=0xffffff; // 24-bit bus
extern "C" unsigned int mame_debug=0,cur_mrhard=0,m68k_illegal_opcode=0,illegal_op=0,illegal_pc=0,opcode_entry=0; // filler

static unsigned char  __fastcall AsekRead8 (unsigned int a) { return ReadByte(a); }
static unsigned short __fastcall AsekRead16(unsigned int a) { return ReadWord(a); }
static unsigned int   __fastcall AsekRead32(unsigned int a) { return ReadLong(a); }
static unsigned char  __fastcall AsekFetch8 (unsigned int a) { return FetchByte(a); }
static unsigned short __fastcall AsekFetch16(unsigned int a) { return FetchWord(a); }
static unsigned int   __fastcall AsekFetch32(unsigned int a) { return FetchLong(a); }
static void __fastcall AsekWrite8 (unsigned int a,unsigned char d)  { WriteByte(a,d); }
static void __fastcall AsekWrite16(unsigned int a,unsigned short d) { WriteWord(a,d); }
static void __fastcall AsekWrite32(unsigned int a,unsigned int d)   { WriteLong(a,d); }
void __fastcall AsekChangePc(unsigned int pc)
{
  unsigned char *pr; pr=FIND_F(pc);
  // Jump to mapped memory area:
  OP_ROM=pr-(pc&~SEK_PAGEM);
  // don't bother calling us back unless it's outside the 64k segment
  M68000_regs.AsmBank=(pc>>16);
}

struct A68KInter
{
  unsigned int OpcodeXor;
  unsigned char  (__fastcall *Read8) (unsigned int a);
  unsigned short (__fastcall *Read16)(unsigned int a);
  unsigned int   (__fastcall *Read32)(unsigned int a);
  void (__fastcall *Write8)  (unsigned int a,unsigned char d);
  void (__fastcall *Write16) (unsigned int a,unsigned short d);
  void (__fastcall *Write32) (unsigned int a,unsigned int d);
  void (__fastcall *ChangePc)(unsigned int a);
  unsigned char  (__fastcall *PcRel8) (unsigned int a);
  unsigned short (__fastcall *PcRel16)(unsigned int a);
  unsigned int   (__fastcall *PcRel32)(unsigned int a);
  unsigned short (__fastcall *Dir16)(unsigned int a);
  unsigned int   (__fastcall *Dir32)(unsigned int a);
};

extern "C" struct A68KInter a68k_memory_intf=
{
  0,
  AsekRead8,
  AsekRead16,
  AsekRead32,
  AsekWrite8,
  AsekWrite16,
  AsekWrite32,
  AsekChangePc,
  AsekFetch8,
  AsekFetch16,
  AsekFetch32,
  AsekRead16, // unused
  AsekRead32, // unused
};

#endif
// ==============================================================

// Memory accesses (non-emu specific)
unsigned int SekFetch(unsigned int a) { return (unsigned int)FetchByte(a); }

#ifdef EMU_A68K
static int A68KIrqCallback(int nIrq)
{ (void)nIrq; return -1; } // Do autovectored interrupt
static int A68KDoReset()
{
  if (pSekExt->ResetCallback==NULL) return 0;
  return pSekExt->ResetCallback();
}
static int (*A68KResetCallback)()=A68KDoReset;
#endif

static int SekSetup(struct A68KContext *psr)
{
  psr->IrqCallback=A68KIrqCallback;
  psr->pResetCallback=&A68KResetCallback;
  // nCpuType=0 means 68000
  return 0;
}

int SekInit(int nCount)
{
  int nMemLen=0,i=0; struct SekExt *ps=NULL;
  SekExit(); // make sure exited

  nSekCount=nCount;

  // Allocate cpu extenal data (memory map etc)
  nMemLen=nSekCount*sizeof(struct SekExt);
  SekExt=(struct SekExt *)malloc(nMemLen);
  if (SekExt==NULL) { SekExit(); return 1; }
  memset(SekExt,0,nMemLen);

  // Put in default memory handlers
  for (i=0,ps=SekExt; i<nSekCount; i++,ps++)
  {
    ps-> ReadByte= DefReadByte;
    ps->WriteByte=DefWriteByte;
    ps-> ReadWord= DefReadWord;
    ps->WriteWord=DefWriteWord;
    ps-> ReadLong= DefReadLong;
    ps->WriteLong=DefWriteLong;
  }

  // Allocate emu-specific cpu states
#ifdef EMU_A68K
  nMemLen=nSekCount*sizeof(struct A68KContext);
  SekRegs=(struct A68KContext *)malloc(nMemLen);
  if (SekRegs==NULL) { SekExit(); return 1; }

  memset(SekRegs,0,nMemLen);
  for (i=0;i<nSekCount;i++) SekSetup(SekRegs+i); // Setup each cpu context
  
  M68000_RESET(); // Init cpu emulator
#endif
  return 0;
}

int SekExit()
{
  // Deallocate cpu extenal data (memory map etc)
#ifdef EMU_A68K
  if (SekRegs!=NULL) free(SekRegs);  SekRegs=NULL;
#endif
  // Deallocate other context data
  if (SekExt!=NULL)  free(SekExt);   SekExt=NULL;
  nSekCount=0;
  pSekExt=NULL;
  return 0;
}

// Map memory for fast access
// Note - each page is 1<<SEK_BITS.
// You can map smaller areas, but not more than one within the page
// e.g. you can't do SekMemory(RamA,0,3,SM_RAM) SekMemory(RamB,3,7,SM_RAM)

int SekMemory(unsigned char *Mem,unsigned int nStart,unsigned int nEnd,int nType)
{
  unsigned int i;

  // Add to memory map
  for (i=(nStart&~SEK_PAGEM); i<=nEnd; i+=SEK_PAGE_SIZE)
  {
    unsigned char *Ptr; unsigned char **pMemMap;
    Ptr=Mem-nStart+i;
    pMemMap=SekExt[nSekActive].MemMap+(i>>SEK_SHIFT);

    if (nType&SM_READ)  pMemMap[0         ]=Ptr;  // Read
    if (nType&SM_WRITE) pMemMap[SEK_WADD  ]=Ptr;  // Write
    if (nType&SM_FETCH) pMemMap[SEK_WADD*2]=Ptr;  // Fetch
  }
  return 0;
}

// Set up the two callbacks (these are always called from the assembler)
int SekReset()
{
#ifdef EMU_A68K
  // A68K has no internal support for resetting the processor, so manually fetch the SR and PC.
  unsigned int sp,pc;

  // Get the Stack pointer (aka A7) and PC from 0123 and 4567 of the rom.
  sp=FetchLong(0); pc=FetchLong(4);

  M68000_regs.pc=pc;
  M68000_regs.a[7]=sp; M68000_regs.srh=0x27; // start in supervisor state
  AsekChangePc(M68000_regs.pc);

#endif
  return 0;
}

int SekScan(int nAction)
{
  // Scan the 68000 states
  struct BurnArea ba;
  int i=0;
  memset(&ba,0,sizeof(ba));

  ba.Data=SekRegs;
  ba.nLen=nSekCount*sizeof(SekRegs[0]);
  ba.szName="68000";
  if (nAction&1)
  {
    // Blank pointers
    for (i=0;i<nSekCount;i++)
    {
      SekRegs[i].IrqCallback=NULL;
      SekRegs[i].pResetCallback=NULL;
    }
  }
  BurnAcb(&ba);
  if (nAction&3)
  {
    // Pointers could have been written over
    for (i=0;i<nSekCount;i++) SekSetup(SekRegs+i); // Resetup each cpu context
  }

  return 0;
}
