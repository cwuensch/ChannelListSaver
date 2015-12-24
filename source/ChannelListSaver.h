#ifndef __CHANNELLISTTAPH__
#define __CHANNELLISTTAPH__
#undef sprintf
#define TAP_SPrint    snprintf

#define PROGRAM_NAME          "ChannelListSaver"
#define VERSION               "0.7"
#define TAPID                 0x2A0A0002
#define AUTHOR                "chris86"
//#define DESCRIPTION           "Im-/Export of Sat-, Transponder- & Channel-Lists"
#define DESCRIPTION           "Im-/Export of Sat, Transponders, Channels & Favs."

#define LOGDIR                "/ProgramFiles/Settings/" PROGRAM_NAME
#define LOGFILENAME           PROGRAM_NAME ".log"
#define LNGFILENAME           PROGRAM_NAME ".lng"
#define INIFILENAME           PROGRAM_NAME ".ini"


#define EXPORTFILENAME        "Channels"
#define CRLF                  "\r\n"

#define PROVIDERNAMELENGTH    21
#define NRPROVIDERNAMES       256
#define SERVICENAMESLENGTH    39996    // 40000 / 40004 / 39996 ***  ?


typedef struct tProvicerName
{
  char name[PROVIDERNAMELENGTH];
} tProviderName;

typedef struct
{
  char                  Magic[6];     // TFchan
  short                 FileVersion;  // 1
  SYSTEM_TYPE           SystemType;
  bool                  UTF8System;
  unsigned long         FileSize;

  int                   SatellitesOffset;
  int                   TranspondersOffset;
  int                   TVServicesOffset;
  int                   RadioServicesOffset;
  int                   FavoritesOffset;
  int                   ProviderNamesOffset;
  int                   ServiceNamesOffset;

  int                   NrSatellites;
  int                   NrTransponders;
  int                   NrTVServices;
  int                   NrRadioServices;
  int                   ProviderNamesLength;
  int                   ServiceNamesLength;
  int                   NrFavGroups;
  int                   NrSvcsPerFavGroup;
} tExportHeader;


#define SYSTYPE 7
#if (SYSTYPE == 5)    // ST_TMSS
  typedef TYPE_SatInfo_TMSS             TYPE_SatInfo_TMSx;
  typedef TYPE_TpInfo_TMSS              TYPE_TpInfo_TMSx;
  typedef TYPE_Service_TMSS             TYPE_Service_TMSx;
#elif (SYSTYPE == 7)  // ST_TMSC
  typedef TYPE_SatInfo_TMSC             TYPE_SatInfo_TMSx;
  typedef TYPE_TpInfo_TMSC              TYPE_TpInfo_TMSx;
  typedef TYPE_Service_TMSC             TYPE_Service_TMSx;
#elif (SYSTYPE == 6)  // ST_TMST
  typedef TYPE_SatInfo_TMST             TYPE_SatInfo_TMSx;
  typedef TYPE_TpInfo_TMST              TYPE_TpInfo_TMSx;
  typedef TYPE_Service_TMST             TYPE_Service_TMSx;
#endif


// Globale Variablen
extern SYSTEM_TYPE      CurSystemType;
extern int              NrFavGroups;
extern int              NrFavsPerGroup;
extern size_t           SIZE_SatInfo_TMSx;
extern size_t           SIZE_TpInfo_TMSx;
extern size_t           SIZE_Service_TMSx;
extern size_t           SIZE_Favorites;


int   TAP_Main(void);
dword TAP_EventHandler(word event, dword param1, dword param2);

void  CloseLogMC(void);
void  WriteLogMC(char *ProgramName, char *s);
void  WriteLogMCf(char *ProgramName, const char *format, ...) __attribute__ ((format(__printf__, 2, 3)));
bool  ConvertUTFStr(char *DestStr, char *SourceStr, int MaxLen, bool ToUnicode);
bool  HDD_SetFileDateTime(char const *FileName, char const *AbsDirectory, dword NewDateTime);

bool  HDD_ImExportChData(char *FileName, char *AbsDirectory, bool Import);
int   GetLengthOfServiceNames(int *NrServiceNames);
int   GetLengthOfProvNames(int *NrProviderNames);
bool  DeleteTimers(void);
void  DeleteServiceNames(void);
bool  DeleteAllSettings(bool OverwriteSatellites);

#endif
