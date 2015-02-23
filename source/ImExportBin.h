#ifndef __IMEXPORTBINH__
#define __IMEXPORTBINH__

byte   GetTransponderSatIndex_TMSx(TYPE_TpInfo_TMSx *pTpInfo);
word*  GetpNrTranspOfSat_TMSx(TYPE_SatInfo_TMSx *pSatInfo);

bool   ExportSettings(char *FileName, char *AbsDirectory);
bool   ImportSettings(char *FileName, char *AbsDirectory, bool OverwriteSatellites);

#endif