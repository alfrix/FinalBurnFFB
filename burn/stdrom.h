// Standard Rom Info/Name functions


struct StdRomInfo
{ char szName[16]; int nLen; unsigned int nCrc; int nType; };


// A function to pick a rom, or return NULL if i is out of range
#define STD_ROM_PICK(Name) \
static struct StdRomInfo *Name##PickRom(unsigned int i) \
{ \
  if (i>=sizeof(Name##RomDesc)/sizeof(Name##RomDesc[0])) return NULL; \
  return Name##RomDesc+i; \
}


// Standard rom functions for returning Length, Crc, Type and one one Name
#define STD_ROM_FN(Name) \
static int Name##RomInfo(struct BurnRomInfo *pri,unsigned int i) \
{ \
  struct StdRomInfo *por=Name##PickRom(i); \
  if (por==NULL) return 1; \
  if (pri!=NULL) { pri->nLen =por->nLen; pri->nCrc =por->nCrc; pri->nType=por->nType; } \
  return 0; \
} \
  \
static int Name##RomName(char **pszName,unsigned int i,int nAka) \
{ \
  struct StdRomInfo *por=Name##PickRom(i); \
  if (por==NULL) return 1;  if (nAka!=0) return 1; \
  *pszName=por->szName; \
  return 0; \
}
