#ifndef __IMEXPORTTEXTH__
#define __IMEXPORTTEXTH__

typedef enum
{
  SM_Start,
  SM_Header,
  SM_HeaderChecked,
  SM_Satellites,
  SM_Transponders,
  SM_TVServices,
  SM_RadioServices,
  SM_Favorites,
  SM_Ignore
} tScanMode;


bool          ExportSettings_Text(char *FileName, char *AbsDirectory);
bool          ImportSettings_Text(char *FileName, char *AbsDirectory, int OverwriteSatellites);  // 0: nie, 1: auto, 2: immer

#endif
