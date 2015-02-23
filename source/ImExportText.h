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


char* ByteArrToStr(char *outStr, byte inArr[], int length);
bool StrToByteArr(byte *outArr, char *inStr, int length);

char BoolToChar(bool inValue);
bool CharToBool(char inValue);

char* FECtoStr(char *outStr, byte inFEC);
byte StrToFEC(char *inStr);

char* ModulationToStr(char *outStr, byte inMod);
byte StrToModulation(char *inStr);

char* VideoTypeToStr(char *outStr, byte inVideo);
byte StrToVideoType(char *inStr);

char* AudioTypeToStr(char *outStr, word inAudio);
word StrToAudioType(char *inStr);

bool ExportSettings_Text(char *FileName, char *AbsDirectory);
bool ImportSettings_Text(char *FileName, char *AbsDirectory, bool OverwriteSatellites);

#endif
