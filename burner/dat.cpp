// Burner data file module (for ROM managers)
#include "app.h"

int write_datfile(int nDatType)
{
	char cFilename[80];
	FILE *fDat=0;
	int nRet=0;
	int nOldSelect=0;
	int nGameSelect=0;
	int nParentSelect=0;

	if (nDatType==0)
		sprintf(cFilename, "Final Burn v%s (CMPro).dat", szAppBurnVer);
	else
		sprintf(cFilename, "Final Burn v%s (RomCenter).dat", szAppBurnVer);

	if ((fDat=fopen(cFilename, "w"))==0)
		return -1;

	nOldSelect=nBurnDrvSelect; // preserve the currently selected driver

	if (nDatType==0)
	{
		fprintf(fDat, "clrmamepro (\n");
		fprintf(fDat, "\tname \"Final Burn\"\n");
		fprintf(fDat, "\tdescription \"Final Burn v%s\"\n", szAppBurnVer);
		fprintf(fDat, "\tcategory \"Sega and Capcom games (68000 based)\"\n");
		fprintf(fDat, "\tversion %s\n", szAppBurnVer);
		fprintf(fDat, "\tauthor \"Final Burn\"\n");
		fprintf(fDat, "\tforcezipping zip\n");
		fprintf(fDat, ")\n\n");
	}
	else
	{
		fprintf(fDat, "[CREDITS]\n");
		fprintf(fDat, "Author=Final Burn\n");
		fprintf(fDat, "Version=%s\n", szAppBurnVer);
		fprintf(fDat, "Comment=Sega and Capcom games (68000 based)\n");
		fprintf(fDat, "[DAT]\n");
		fprintf(fDat, "version=2.00\n");
		fprintf(fDat, "[EMULATOR]\n");
		fprintf(fDat, "refname=Final Burn\n");
		fprintf(fDat, "version=Final Burn v%s\n", szAppBurnVer);
		fprintf(fDat, "[GAMES]\n");
	}

	for (nGameSelect=0;(unsigned int)nGameSelect<nBurnDrvCount;nGameSelect++)
	{
		char sgName[16];
		char spName[16];
		char *szName=NULL; int nRet=0;
		unsigned int i=0;

		nBurnDrvSelect=nGameSelect; // Switch to driver nGameSelect
		strcpy(sgName, BurnDrvText(0));

		nRet=BurnDrvGetZipName(&szName,0);
		sprintf(spName, szName);
		if (strchr(spName, '.')) *strchr(spName, '.')='\0';

		if (strcmp(sgName, spName))
		{
			nParentSelect=-1;
			for (i=0;i<nBurnDrvCount;i++)
			{
				nBurnDrvSelect=i; // Switch to driver i
				if (!strcmp(spName, BurnDrvText(0)))
				{
					nParentSelect=i;
				}
			}

			nBurnDrvSelect=nGameSelect; // restore driver select
		}
		else
			nParentSelect=nGameSelect;

		if (nDatType==0)
		{
			if (nParentSelect==-1)
			{
				fprintf(fDat, "# Missing parent %s. Someone should add it to Final Burn!\n\n", spName);
			}

			fprintf(fDat, "game (\n");
			fprintf(fDat, "\tname %s\n", sgName);
			fprintf(fDat, "\tdescription \"%s\"\n", BurnDrvText(1));

			if (nParentSelect!=nGameSelect && nParentSelect!=-1)
			{
				fprintf(fDat, "\tcloneof %s\n", spName);
				fprintf(fDat, "\tromof %s\n", spName);
			}
		}

		for (i=0; nRet==0 && i<100; i++) // up to 100 ROMs in one game
		{
			int nRetTmp=0;
			struct BurnRomInfo ri;
			int nLen; unsigned int nCrc;
			char *szPossibleName=NULL;
			int j, nMerged=0;

			memset(&ri,0,sizeof(ri));

			nRet=BurnDrvGetRomInfo(&ri,i);
			nRet+=BurnDrvGetRomName(&szPossibleName,i,0);

			if (ri.nLen==0) continue;

			if (nRet==0)
			{
				if (nParentSelect!=nGameSelect && nParentSelect!=-1)
				{
					nLen=ri.nLen; nCrc=ri.nCrc;
					nRetTmp=0;
					nMerged=0;

					nBurnDrvSelect=nParentSelect; // Switch to parent

					for (j=0; nRetTmp==0 && j<100; j++) // up to 100 ROMs in one game
					{
						struct BurnRomInfo riTmp;
						char *szPossibleNameTmp=NULL;

						memset(&riTmp,0,sizeof(riTmp));

						nRetTmp=BurnDrvGetRomInfo(&riTmp,j);
						nRetTmp+=BurnDrvGetRomName(&szPossibleNameTmp,j,0);

						if (riTmp.nLen==0) nRetTmp++;

						if (nRetTmp==0)
						{
							if (riTmp.nCrc==nCrc && !strcmp(szPossibleName, szPossibleNameTmp))
							{
								nMerged++;
								nRetTmp++;
							}
						}
					}

					nBurnDrvSelect=nGameSelect; // Switch back to game
				}

				if (nDatType==0)
				{
					if (!nMerged)
						fprintf(fDat, "\trom ( name %s size %d crc %08x )\n", szPossibleName, ri.nLen, ri.nCrc);
					else
						fprintf(fDat, "\trom ( name %s merge %s size %d crc %08x )\n", szPossibleName, szPossibleName, ri.nLen, ri.nCrc);
				}
				else
				{
					if (nParentSelect!=nGameSelect && nParentSelect!=-1)
					{
						nBurnDrvSelect=nParentSelect; // Switch to parent
						fprintf(fDat, "¬%s¬%s", spName, BurnDrvText(1));
						nBurnDrvSelect=nGameSelect; // Switch back to game
					}
					else
						fprintf(fDat, "¬%s¬%s", BurnDrvText(0), BurnDrvText(1));

					fprintf(fDat, "¬%s¬%s", BurnDrvText(0), BurnDrvText(1));

		   			fprintf(fDat, "¬%s¬%08x¬%d", szPossibleName, ri.nCrc, ri.nLen);

					if (nParentSelect!=nGameSelect && nParentSelect!=-1)
						fprintf(fDat, "¬%s", spName);

					if (!nMerged)
						fprintf(fDat, "¬¬\n");
					else
						fprintf(fDat, "¬%s¬\n", szPossibleName);
				}
			}
		}

		if (nDatType==0)
			fprintf(fDat, ")\n\n");
	}

	nBurnDrvSelect=nOldSelect;

	fclose(fDat);

	return nRet;
};
