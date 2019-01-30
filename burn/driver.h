// Filler file for ym2151.c

typedef           char  INT8;
typedef unsigned  char UINT8;
typedef          short  INT16;
typedef unsigned short UINT16;
typedef            int  INT32;
typedef unsigned   int UINT32;
typedef void (*mem_write_handler)(unsigned int offset,unsigned int data);

// Make the INLINE macro
#undef INLINE
#define INLINE __inline
#pragma warning (disable:4018) // '!=' : signed/unsigned mismatch
#pragma warning (disable:4146)
#pragma warning (disable:4244)
#pragma warning (disable:4100)
#pragma warning (disable:4710) // inline not inlined

static INLINE void __cdecl logerror(char *szFormat,...) { (void)szFormat; }
