// 68000 (Sixty Eight K) Interface - header file

#define EMU_A68K // Use the 'A68K' (Make68K) Assembler 68000 emulator

// Number of bits used for each page in the fast memory map
#define SEK_BITS (14) // 14 = 0x4000 page size
#define SEK_PAGE_COUNT (1<<(24-SEK_BITS)) // Number of pages
#define SEK_SHIFT SEK_BITS  // Shift value = page bits
#define SEK_PAGE_SIZE (1<<SEK_BITS) // Page size
#define SEK_PAGEM (SEK_PAGE_SIZE-1)
#define SEK_WADD SEK_PAGE_COUNT // value to add for write section = Number of pages
#define SEK_MASK (SEK_WADD-1)

#ifdef EMU_A68K
extern "C" void __cdecl M68000_RUN();
extern "C" void __cdecl M68000_RESET();
// The format of the data in a68k.asm (at the _M68000_regs location)
struct A68KContext
{
  unsigned int d[8],a[8];
  unsigned int isp,srh,ccr,xc,pc,irq,sr;
  int (*IrqCallback) (int nIrq);
  unsigned int ppc;
  void *pResetCallback;
  unsigned int sfc,dfc,usp,vbr;
  unsigned int AsmBank,CpuVersion;
};
extern "C" struct A68KContext M68000_regs;

extern struct A68KContext *SekRegs;
extern "C" unsigned char *OP_ROM,*OP_RAM;
extern "C" int m68k_ICount;
void __fastcall AsekChangePc(unsigned int pc);
#endif

struct SekExt
{
  // Mapped memory pointers to Rom and Ram areas (Read then Write)
  // These memory areas must be allocated multiples of the page size
  // with a 4 byte over-run area
  // lookup for each page (*3 for read, write and fetch)
  unsigned char *MemMap[SEK_PAGE_COUNT*3];
  // If MemMap[i] is NULL, use the handler functions
  unsigned char (*ReadByte)(unsigned int a);
  void (*WriteByte)(unsigned int a,unsigned char d);
  unsigned short (*ReadWord)(unsigned int a);
  void (*WriteWord)(unsigned int a,unsigned short d);
  unsigned int (*ReadLong)(unsigned int a);
  void (*WriteLong)(unsigned int a,unsigned int d);
  int (*ResetCallback) ();
};
#define SEK_DEF_READ_WORD(a) { unsigned short d; d=(unsigned short)(pSekExt->ReadByte(a)<< 8); d|=(unsigned short)(pSekExt->ReadByte((a)+1)); return d; }
#define SEK_DEF_WRITE_WORD(a,d) { pSekExt->WriteByte((a),(unsigned char)((d)>>8)); pSekExt->WriteByte((a)+1,(unsigned char)((d)&0xff)); }
#define SEK_DEF_READ_LONG(a) { unsigned int d; d=pSekExt->ReadWord(a)<<16; d|=pSekExt->ReadWord((a)+2); return d; }
#define SEK_DEF_WRITE_LONG(a,d) { pSekExt->WriteWord((a),(unsigned short)((d)>>16)); pSekExt->WriteWord((a)+2,(unsigned short)((d)&0xffff)); }
extern struct SekExt *SekExt,*pSekExt;
extern int nSekActive; // The cpu which is currently being emulated


unsigned int SekFetch(unsigned int a);

int SekInit(int nCount);
int SekExit();

// Open a cpu
static INLINE void SekOpen(int i)
{
  nSekActive=i; pSekExt=SekExt+nSekActive; // Point to cpu context
#ifdef EMU_A68K
  M68000_regs=SekRegs[nSekActive]; // copy cpu state into A68K
  AsekChangePc(M68000_regs.pc); // Define OP_ROM
#endif
}
// Close a cpu
static INLINE void SekClose()
{
#ifdef EMU_A68K
  SekRegs[nSekActive]=M68000_regs; // get cpu state back from A68K
#endif
}

// Exec the opened cpu
static INLINE int SekRun(int Cycles)
{
#ifdef EMU_A68K
  int Done;
  m68k_ICount=Cycles;
  if (M68000_regs.irq==0x80) { Done=Cycles; } // Cpu is in stopped state till interrupt
  else { M68000_RUN(); Done=Cycles-m68k_ICount; }
  return Done; // Return the number of cycles actually done
#else
  return 0;
#endif
}

// Cause interrupt on the opened cpu
static INLINE void SekInterrupt(int i)
{
#ifdef EMU_A68K
  M68000_regs.irq=i; // raise irq (gets lowered after taken)
#endif
}

// SekMemory types:
#define SM_READ  (1)
#define SM_WRITE (2)
#define SM_FETCH (4)
#define SM_ROM (SM_READ|SM_FETCH)
#define SM_RAM (SM_READ|SM_WRITE|SM_FETCH)
int SekMemory(unsigned char *Mem,unsigned int nStart,unsigned int nEnd,int nType);
int SekReset();
static INLINE int SekPc(int i)
{
#ifdef EMU_A68K
  if (i<0) return M68000_regs.pc;   // Currently open pc
  else return SekRegs[i].pc; // Any cpu pc
#else
  return 0;
#endif
}

int SekScan(int nAction);

// dasm.cpp
int DasmArea(int Pc,int Before,int After);
