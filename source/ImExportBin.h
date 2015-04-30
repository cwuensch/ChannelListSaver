#ifndef __IMEXPORTBINH__
#define __IMEXPORTBINH__

byte          GetTransponderSatIndex_TMSx(TYPE_TpInfo_TMSx *pTpInfo);
word          GetNrTranspOfSat_TMSx(TYPE_SatInfo_TMSx *pSatInfo);
word*         GetpNrTranspOfSat_TMSx(TYPE_SatInfo_TMSx *pSatInfo);

bool          ExportSettings(char *FileName, char *AbsDirectory);
bool          ImportSettings(char *FileName, char *AbsDirectory, int OverwriteSatellites);  // 0: nie, 1: auto, 2: immer

#endif
