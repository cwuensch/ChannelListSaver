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

typedef struct
{
  word AudioPID:13;
  word AudioTypeFlag:2;
  word AudioAutoSelect:1;
} tAudioPIDFlags;

typedef struct
{
  byte                  UniCableSatPosition:1;
  byte                  UniCableunused:7;
  word                  UniCableUserBand:4;
  word                  UniCableFrq:12;
  byte                  unused6[2];
} tUnicableFlags;

bool          ExportSettings_Text(char *FileName, char *AbsDirectory);
bool          ImportSettings_Text(char *FileName, char *AbsDirectory, int OverwriteSatellites, bool RestoreNameLock);  // 0: nie, 1: auto, 2: immer

#endif
