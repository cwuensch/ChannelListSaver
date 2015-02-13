#define _FILE_OFFSET_BITS  64
#define __USE_LARGEFILE64  1
#ifdef _MSC_VER
  #define __const const
#endif
//#undef malloc
//#undef free


#define _GNU_SOURCE
#include                <string.h>
#include                <stdio.h>
#include                <stdlib.h>
#include                <tap.h>
#include                <libFireBird.h>
#include                "../../../../../Topfield/FireBirdLib/flash/FBLib_flash.h"
#include                "FlashSatTablesSetInfo.h"
#include                "ChannelListTAP.h"

TAP_ID                  (TAPID);
TAP_PROGRAM_NAME        (PROGRAM_NAME" "VERSION);
TAP_PROGRAM_VERSION     (VERSION);
TAP_AUTHOR_NAME         (AUTHOR);
TAP_DESCRIPTION         (DESCRIPTION);
TAP_ETCINFO             (__DATE__);


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



// ============================================================================
//                              IMPLEMENTIERUNG
// ============================================================================
  
bool                         CSShowMessageBox = FALSE;
dword                        LastMessageBoxKey;

SYSTEM_TYPE                  CurSystemType;
int                          NrFavGroups = 0;
int                          NrFavsPerGroup = 0;
size_t                       SIZE_SatInfo_TMSx = 0;
size_t                       SIZE_TpInfo_TMSx  = 0;
size_t                       SIZE_Service_TMSx = 0;
size_t                       SIZE_Favorites = 0;


bool InitSystemType(void)
{
  bool ret = TRUE;

  CurSystemType = GetSystemType();
  switch (CurSystemType)
  {
    case ST_TMSS:
      SIZE_SatInfo_TMSx = sizeof(TYPE_SatInfo_TMSS);
      SIZE_TpInfo_TMSx  = sizeof(TYPE_TpInfo_TMSS);
      SIZE_Service_TMSx = sizeof(TYPE_Service_TMSS);
      break;
    
    case ST_TMSC:
      SIZE_SatInfo_TMSx = sizeof(TYPE_SatInfo_TMSC);
      SIZE_TpInfo_TMSx  = sizeof(TYPE_TpInfo_TMSC);
      SIZE_Service_TMSx = sizeof(TYPE_Service_TMSC);
      break;

    case ST_TMST:
      SIZE_SatInfo_TMSx = sizeof(TYPE_SatInfo_TMST);
      SIZE_TpInfo_TMSx  = sizeof(TYPE_TpInfo_TMST);
      SIZE_Service_TMSx = sizeof(TYPE_Service_TMST);
      break;

    default:
      TAP_PrintNet("Nicht unterstütztes System!");
      ret = FALSE;
      break;
  }

  FlashFavoritesGetParameters(&NrFavGroups, &NrFavsPerGroup);
  switch (NrFavsPerGroup)
  {
    case 50:
      SIZE_Favorites = sizeof(tFavorites1050);
      break;

    case 100:
      SIZE_Favorites = sizeof(tFavorites);
      break;

    default:
      TAP_PrintNet("Nicht unterstützte Favoriten-Struktur!");
      ret = FALSE;
      break;
  }
  return ret;
}

/*
bool FlashSatTablesDecode_ST_TMSx(void *Data, tFlashSatTable *SatTable)
{
  if(!Data || !SatTable)
    return FALSE;

  switch(CurSystemType)
  {
    case ST_TMSS: return FlashSatTablesDecode_ST_TMSS(Data, SatTable); break;
    case ST_TMST: return FlashSatTablesDecode_ST_TMST(Data, SatTable); break;
    case ST_TMSC: return FlashSatTablesDecode_ST_TMSC(Data, SatTable); break;
    default:      return FALSE;
  }
}

bool FlashTransponderTablesDecode_ST_TMSx(void *Data, tFlashTransponderTable *TransponderTable)
{
  if(!Data || !TransponderTable)
    return FALSE;

  switch(CurSystemType)
  {
    case ST_TMSS: return FlashTransponderTablesDecode_ST_TMSS(Data, TransponderTable); break;
    case ST_TMST: return FlashTransponderTablesDecode_ST_TMST(Data, TransponderTable); break;
    case ST_TMSC: return FlashTransponderTablesDecode_ST_TMSC(Data, TransponderTable); break;
    default:      return FALSE;
  }
}
bool FlashTransponderTablesEncode_ST_TMSx(void *Data, tFlashTransponderTable *TransponderTable)
{
  if(!Data || !TransponderTable)
    return FALSE;

  switch(CurSystemType)
  {
    case ST_TMSS: return FlashTransponderTablesEncode_ST_TMSS(Data, TransponderTable); break;
    case ST_TMST: return FlashTransponderTablesEncode_ST_TMST(Data, TransponderTable); break;
    case ST_TMSC: return FlashTransponderTablesEncode_ST_TMSC(Data, TransponderTable); break;
    default:      return FALSE;
  }
}

bool FlashServiceDecode_ST_TMSx(void *Data, tFlashService *Service)
{
  if(!Data || !Service)
    return FALSE;

  switch(CurSystemType)
  {
    case ST_TMSS: return FlashServiceEncode_ST_TMSS(Data, Service); break;
    case ST_TMST: return FlashServiceEncode_ST_TMST(Data, Service); break;
    case ST_TMSC: return FlashServiceEncode_ST_TMSC(Data, Service); break;
    default:      return FALSE;
  }
}
bool FlashServiceEncode_ST_TMSx(void *Data, tFlashService *Service)
{
  if(!Data || !Service)
    return FALSE;

  switch(CurSystemType)
  {
    case ST_TMSS: return FlashServiceEncode_ST_TMSS(Data, Service); break;
    case ST_TMST: return FlashServiceEncode_ST_TMST(Data, Service); break;
    case ST_TMSC: return FlashServiceEncode_ST_TMSC(Data, Service); break;
    default:      return FALSE;
  }
} */



void ImportTransponder(void)
{
  tFlashSatTable            SatTable;
  tFlashTransponderTable    TransponderTable;
  FILE                     *fSetting;
  char                     *Line = NULL, *CurrentSOL, *p;
  size_t                    len = 0;
  int                       ParamCount, i, NrSats, SatIndex;
  char                      Params[50][50];
  bool                      DoParse;
  int                       Count;

  Count = 0;
  if(TAP_Hdd_Exist(EXPORTFILENAME ".dat"))
  {
    fSetting = fopen(TAPFSROOT "/" EXPORTFILENAME ".dat", "r");
    if(fSetting)
    {
      DoParse = FALSE;

      while(getline(&Line, &len, fSetting) != -1)
      {
        //Interpret the following characters as remarks: ; # //
        p = strchr(Line, ';');  if(p) *p = '\0';
        p = strchr(Line, '#');  if(p) *p = '\0';
        p = strstr(Line, "//"); if(p) *p = '\0';

        //Remove CR and LF
        p = strchr(Line, '\r');  if(p) *p = '\0';
        p = strchr(Line, '\n');  if(p) *p = '\0';

        //Replace all TABs with SPACEs
        StrReplace(Line, "\t", " ");

        //Trim all spaces from the end of the line
        RTrim(Line);

        //Check if this is a block header
        if(Line[0] == '[') DoParse = strcmp(Line, "[Transponder]") == 0;

        if(DoParse)
        {
          //Separate the line into the parameters
          ParamCount = 0;
          memset(Params, 0, sizeof(Params));
          CurrentSOL = Line;
          while(CurrentSOL[0])
          {
            //Make wure, the line doesn't start with a SPACE
            while(CurrentSOL[0] == ' ') CurrentSOL++;

            //Find the next SPACE in the current line
            p = strchr(CurrentSOL, ' ');

            //If there is no SPACE, point to EOL
            if(p == NULL) p = CurrentSOL + strlen(CurrentSOL);

            strncpy(Params[ParamCount], CurrentSOL, p - CurrentSOL);
            ParamCount++;

            CurrentSOL = p;
          }

          //Check if enough parameter are available
          if(ParamCount == 15)
          {
            //Decode and add the transponder
            //Translate the SatName (parameter 0) into the sat index
            NrSats = FlashSatTablesGetTotal();
            SatIndex = -1;
            for(i = 0; i < NrSats; i++)
            {
              FlashSatTablesGetInfo(i, &SatTable);
              if(strcmp(SatTable.SatName, Params[0]) == 0)
              {
                SatIndex = i;
                break;
              }
            }

            if(SatIndex != -1)
            {
              //#SatName          Frq    SymbRate  Channel Bandw  TSID ONWID NWID Pilot FEC     Modulation  System  Pol LPHP  ClockSync
              //Astra             10729  22000     0      0       041a 0001  0000 N     2/3     8PSK        DVBS2   V   0     N

              memset(&TransponderTable, 0, sizeof(tFlashTransponderTable));
              TransponderTable.SatIndex = SatIndex;
              TransponderTable.Frequency = strtol(Params[1], NULL, 10);
              TransponderTable.SymbolRate = strtol(Params[2], NULL, 10);
              TransponderTable.ChannelNr = strtol(Params[3], NULL, 10);
              TransponderTable.Bandwidth = strtol(Params[4], NULL, 10);
              TransponderTable.TSID = strtol(Params[5], NULL, 16);
              TransponderTable.OriginalNetworkID = strtol(Params[6], NULL, 16);
              TransponderTable.NetworkID = strtol(Params[7], NULL, 16);
              TransponderTable.Pilot = (Params[8][0] == 'Y') || (Params[8][0] == 'y');

              switch(Params[9][0])
              {
                case 'A':
                case 'a': TransponderTable.FEC = 0; break;
                case '1': TransponderTable.FEC = 1; break;
                case '2': TransponderTable.FEC = 2; break;
                case '3':
                {
                  //Two cases here 3/4 and 3/5
                  if(Params[9][2] == '4')
                    TransponderTable.FEC = 3;
                  else
                    TransponderTable.FEC = 7;

                  break;
                }
                case '5': TransponderTable.FEC = 4; break;
                case '7': TransponderTable.FEC = 5; break;
                case '8': TransponderTable.FEC = 6; break;
                case '4': TransponderTable.FEC = 8; break;
                case '9': TransponderTable.FEC = 9; break;
                case 'N':
                case 'n': TransponderTable.FEC = 0xf; break;
              }

              switch(Params[10][0])
              {
                case 'A':
                case 'a': TransponderTable.Modulation = 0; break;
                case 'Q':
                case 'q': TransponderTable.Modulation = 1; break;
                case '8': TransponderTable.Modulation = 2; break;
                case '1': TransponderTable.Modulation = 3; break;
              }

              if(strcmp(Params[11], "DVBS") == 0)
                TransponderTable.ModSystem = 0;
              else
                TransponderTable.ModSystem = 1;

              TransponderTable.Polarisation = ((Params[12][0] == 'H') || (Params[12][0] == 'h')) ? 1 : 0;
              TransponderTable.LPHP = strtol(Params[13], NULL, 10);
              TransponderTable.ClockSync = (Params[14][0] == 'Y') || (Params[14][0] == 'y');

              if(FlashTransponderTablesAdd(SatIndex, &TransponderTable))
              {
                Count++;
              }
              else
              {
                TAP_PrintNet("Failed to add transponder '%s'\n", Line);
              }
            }
            else
            {
              TAP_PrintNet("Didn't find a matching sat for '%s'\n", Line);
            }
          }
          else
          {
            //Ignore block headers
            if((ParamCount > 0) && (Line[0] != '[')) TAP_PrintNet("Failed to import '%s'\n", Line);
          }
        }
      }
      free(Line);
      fclose(fSetting);
    }
  }

  TAP_PrintNet("%d transponders have been added\n", Count);
}

/*void DeleteTransponder(int SatIndex)
{
  int Count;

  Count = 0;
  while(FlashTransponderTablesGetTotal(SatIndex) > 0)
  {
    FlashTransponderTablesDel(SatIndex, 0);
    Count++;
  }

  TAP_PrintNet("%d transponder have been deleted.\n", Count);
}*/

void DeleteTimers(void)
{
  int Count;

  Count = 0;
  while(TAP_Timer_GetTotalNum() > 0)
  {
    TAP_Timer_Delete(0);
    Count++;
  }
  TAP_PrintNet("%d timer have been deleted.\n", Count);
}

int GetLengthOfServiceNames(void)
{
  int Result = 0;
  int i;
  char *p1, *p2;
  p1 = (char*)(FIS_vFlashBlockServiceName());
  p2 = (char*)(FIS_vFlashBlockProviderInfo());

  if(p1)
  {
    Result = SERVICENAMESLENGTH;
    if (p2 && (p2 > p1) && (p2-p1 < Result))
      Result = p2 - p1;

    for (i = 0; i < Result-1; i++)
    {
      if (!p1[i] && !p1[i+1])
      {
        Result = i+1;
        break;
      }
    }
  }
  return Result;
}

void DebugServiceNames(char* FileName)
{
  FILE *fOut = NULL;
  char fn[512];
  char *p = NULL;

  TAP_SPrint(fn, "%s/%s", TAPFSROOT, FileName);
  fOut = fopen(fn, "wb");
  p = (char*)(FIS_vFlashBlockServiceName());
  if(p && fOut)
  {
    fwrite(p, 1, SERVICENAMESLENGTH, fOut);
    fclose(fOut);
  }
}

void DeleteServiceNames(void)
{
  void  (*Appl_DeleteTvSvcName)(unsigned short, bool);
  void  (*Appl_DeleteRadioSvcName)(unsigned short, bool);
  Appl_DeleteTvSvcName    = (void*)FIS_fwAppl_DeleteTvSvcName();
  Appl_DeleteRadioSvcName = (void*)FIS_fwAppl_DeleteRadioSvcName();

  int nTVServices, nRadioServices, i;
  TAP_Channel_GetTotalNum(&nTVServices, &nRadioServices);

//  DebugServiceNames("vorher.dat");
//  char tmp[512];
  for (i = (nRadioServices - 1); i >= 0; i--)
  {
    Appl_DeleteRadioSvcName(i, FALSE);
//    TAP_SPrint(tmp, "Rad%ld.dat", i);
//    DebugServiceNames(tmp);
  }
  for (i = (nTVServices - 1); i >= 0; i--)
  {
    Appl_DeleteTvSvcName(i, FALSE);
//    TAP_SPrint(tmp, "TV%ld.dat", i);
//    DebugServiceNames(tmp);
  }
//  DebugServiceNames("nachher.dat");
}

bool DeleteAllSettings(void)
{
  bool ret = TRUE;

  {
    // Favourites
    char                 *p;
    p = (char*) FIS_vFlashBlockFavoriteGroup();

    if (p)
    {
      memset(p, 0, NrFavGroups * SIZE_Favorites);
    }
    else ret = FALSE;
  }

  {
    // Service Names
    char                 *p1, *p2;

    DeleteServiceNames();
//    p1 = (char*)(FIS_vFlashBlockServiceName());
//    p2 = (char*)(FIS_vFlashBlockProviderInfo());
//    if(p1 && p2 && (p2 > p1))
//      memset(p, 0, min(p2 - p1, SERVICENAMESLENGTH));
  }

  {
    // Provider Names
    char                 *p;

    p = (char*)(FIS_vFlashBlockProviderInfo());
    if(p)
      memset(p, 0, PROVIDERNAMELENGTH * NRPROVIDERNAMES);
  }

  {
    // TV Services
    TYPE_Service_TMSS      *p;
    word                   *nSvc;

    p    = (TYPE_Service_TMSS*)(FIS_vFlashBlockTVServices());
    nSvc = (word*)FIS_vnTvSvc();
    if (p && nSvc)
    {
      memset(p, 0, *nSvc * SIZE_Service_TMSx);
      *nSvc = 0;
    }

    // Radio Services
    p    = (TYPE_Service_TMSS*)(FIS_vFlashBlockRadioServices());
    nSvc = (word*)FIS_vnRadioSvc();
    if (p && nSvc)
    {
      memset(p, 0, *nSvc * SIZE_Service_TMSx);
      *nSvc = 0;
    }
    else ret = FALSE;
  }

  {
    // Transponders
    TYPE_TpInfo_TMSS *p;
    dword            *NrTransponders;
          
    p = (TYPE_TpInfo_TMSS*)(FIS_vFlashBlockTransponderInfo());
    if (p)
    {
      NrTransponders = (dword*)(p) - 1;
      memset(p, 0, *NrTransponders * SIZE_TpInfo_TMSx);
      *NrTransponders = 0;
    }
    else ret = FALSE;
  }

  {
    // Satellites
    TYPE_SatInfo_TMSS *p;
    p = (TYPE_SatInfo_TMSS*)FIS_vFlashBlockSatInfo();
    if (p)
      memset(p, 0, FlashSatTablesGetTotal() * SIZE_SatInfo_TMSx);
    else ret = FALSE;
  }

  return ret;
}



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




char* ByteArrToStr(char *outStr, byte inArr[], int length)
{
  int i;

  if (outStr)
    outStr[0] = '\0';
  if (outStr && inArr)
  {
    for (i = 0; i < length; i++)
      sprintf(&outStr[3*i], "%02.2X ", inArr[i]);
    if (outStr[0])
      outStr[strlen(outStr)-1] = '\0';
  }
  return outStr;
}
bool StrToByteArr(byte *outArr, char *inStr, int length)
{
  char *p;
  int   i;

  if (outArr)
  {
    memset(outArr, 0, length);
    p = strtok(inStr, " ");
    i = 0;
    while (p)
    {
      if (i < length)
      {
        outArr[i] = strtol(p, NULL, 16);
        p = strtok(NULL, " ");
        i++;
      }
      else
        return FALSE;
    }
    return TRUE;
  }
  else
    return FALSE;
}

char BoolToChar(bool inValue)
{
  return (inValue ? 'y' : 'n');
}
bool CharToBool(char inValue)
{
  return ((inValue == 'y') || (inValue == 'Y'));
}

char* FECtoStr(char *outStr, byte inFEC)
{
  if (CurSystemType == ST_TMSS)
    switch (inFEC)
    {
      case FEC_AUTO:                   return "auto";
      case FEC_1_2:                    return "1/2";
      case FEC_2_3:                    return "2/3";
      case FEC_3_4:                    return "3/4";
      case FEC_5_6:                    return "5/6";
      case FEC_7_8:                    return "7/8";
      case FEC_8_9:                    return "8/9";
      case FEC_3_5:                    return "3/5";
      case FEC_4_5:                    return "4/5";
      case FEC_9_10:                   return "9/10";
      case FEC_RESERVED:               return "reserved";
      case FEC_NO_CONV:                return "none";
    }
  sprintf(outStr, "%#3x", inFEC);
  return outStr;
}
byte StrToFEC(char *inStr)
{
  if (strcmp(inStr, "auto")     == 0)  return FEC_AUTO;
  if (strcmp(inStr, "1/2")      == 0)  return FEC_1_2;
  if (strcmp(inStr, "2/3")      == 0)  return FEC_2_3;
  if (strcmp(inStr, "3/4")      == 0)  return FEC_3_4;
  if (strcmp(inStr, "5/6")      == 0)  return FEC_5_6;
  if (strcmp(inStr, "7/8")      == 0)  return FEC_7_8;
  if (strcmp(inStr, "8/9")      == 0)  return FEC_8_9;
  if (strcmp(inStr, "3/5")      == 0)  return FEC_3_5;
  if (strcmp(inStr, "4/5")      == 0)  return FEC_4_5;
  if (strcmp(inStr, "9/10")     == 0)  return FEC_9_10;
  if (strcmp(inStr, "reserved") == 0)  return FEC_RESERVED;
  if (strcmp(inStr, "none")     == 0)  return FEC_NO_CONV;

  long ret = strtol(inStr, NULL, 0);
  if (ret <= 0xF)
  {
    if (ret == 0)
      TAP_PrintNet("Warning: Suspekter FEC-String: '%s'\n", inStr);
    return ret;
  }
  else
    TAP_PrintNet("Error: Ungültiger FEC-String: '%s'\n", inStr);
  return FEC_NO_CONV;
}

char* ModulationToStr(char *outStr, byte inMod)
{
  if (CurSystemType == ST_TMSS)
    switch (inMod)
    {
      case MODULATION_AUTO:            return "auto";
      case MODULATION_QPSK:            return "QPSK";
      case MODULATION_8PSK:            return "8PSK";
      case MODULATION_16QAM:           return "16QAM";
    }
  else if (CurSystemType == ST_TMSC)
    switch (inMod)
    {
      case 0x0:                        return "16QAM";
      case 0x1:                        return "32QAM";
      case 0x2:                        return "64QAM";
      case 0x3:                        return "128QAM";
      case 0x4:                        return "256QAM";
    }
  sprintf(outStr, "%#4x", inMod);
  return outStr;
}
byte StrToModulation(char *inStr)
{
  if (CurSystemType == ST_TMSS)
  {
    if (strcmp(inStr, "auto")   == 0)  return MODULATION_AUTO;
    if (strcmp(inStr, "QPSK")   == 0)  return MODULATION_QPSK;
    if (strcmp(inStr, "8PSK")   == 0)  return MODULATION_8PSK;
    if (strcmp(inStr, "16QAM")  == 0)  return MODULATION_16QAM;
  }
  else if (CurSystemType == ST_TMSC)
  {
    if (strcmp(inStr, "16QAM")  == 0)  return 0x0;
    if (strcmp(inStr, "32QAM")  == 0)  return 0x1;
    if (strcmp(inStr, "64QAM")  == 0)  return 0x2;
    if (strcmp(inStr, "128QAM") == 0)  return 0x3;
    if (strcmp(inStr, "256QAM") == 0)  return 0x4;
  }

  long ret = strtol(inStr, NULL, 0);
  if (ret <= ((CurSystemType == ST_TMSS) ? 3 : 0xFF))
  {
    if (ret == 0)
      TAP_PrintNet("Warning: Suspekter Modulation-String: '%s'\n", inStr);
    return ret;
  }
  else
    TAP_PrintNet("Error: Ungültiger Modulation-String: '%s'\n", inStr);
  return 0;
}

char* VideoTypeToStr(char *outStr, byte inVideo)
{
  switch (inVideo)
  {
    case STREAM_VIDEO_MPEG1:           return "MPEG1";
    case STREAM_VIDEO_MPEG2:           return "MPEG2";
    case STREAM_VIDEO_MPEG4_PART2:     return "MPEG4.2";
    case STREAM_VIDEO_MPEG4_H263:      return "H263";
    case STREAM_VIDEO_MPEG4_H264:      return "H264";
    case STREAM_VIDEO_VC1:             return "VC1";
    case STREAM_VIDEO_VC1SM:           return "VC1SM";
    case STREAM_UNKNOWN:               return "unknown";
    default:
      sprintf(outStr, "%#4x", inVideo);
      return outStr;
  }
}
byte StrToVideoType(char *inStr)
{
  if (strcmp(inStr, "MPEG1")   == 0)   return STREAM_VIDEO_MPEG1;
  if (strcmp(inStr, "MPEG2")   == 0)   return STREAM_VIDEO_MPEG2;
  if (strcmp(inStr, "MPEG4.2") == 0)   return STREAM_VIDEO_MPEG4_PART2;
  if (strcmp(inStr, "H263")    == 0)   return STREAM_VIDEO_MPEG4_H263;
  if (strcmp(inStr, "H264")    == 0)   return STREAM_VIDEO_MPEG4_H264;
  if (strcmp(inStr, "VC1")     == 0)   return STREAM_VIDEO_VC1;
  if (strcmp(inStr, "VC1SM")   == 0)   return STREAM_VIDEO_VC1SM;
  if (strcmp(inStr, "unknown") == 0)   return STREAM_UNKNOWN;

  long ret;
  ret = strtol(inStr, NULL, 0);
  if (ret <= 0xFF)
  {
    if (ret == 0)
      TAP_PrintNet("Warning: Suspekter VideoType-String: '%s'\n", inStr);
    return ret;
  }
  else
    TAP_PrintNet("Error: Ungültiger VideoType-String: '%s'\n", inStr);
  return STREAM_UNKNOWN;
}

char* AudioTypeToStr(char *outStr, word inAudio)
{
  switch (inAudio)
  {
    case STREAM_AUDIO_MP3:             return "MP3";
    case STREAM_AUDIO_MPEG1:           return "MPEG1";
    case STREAM_AUDIO_MPEG2:           return "MPEG2";
    case STREAM_AUDIO_MPEG4_AC3_PLUS:  return "AC3plus";
    case STREAM_AUDIO_MPEG4_AAC:       return "AAC";
    case STREAM_AUDIO_MPEG4_AAC_PLUS:  return "AACplus";
    case STREAM_AUDIO_MPEG4_AC3:       return "AC3";
    case STREAM_AUDIO_MPEG4_DTS:       return "DTS";
    case STREAM_UNKNOWN:               return "unknown";
    default:
      sprintf(outStr, "%#6x", inAudio);
      return outStr;
  }
}
word StrToAudioType(char *inStr)
{
  if (strcmp(inStr, "MP3")     == 0)   return STREAM_AUDIO_MP3;
  if (strcmp(inStr, "MPEG1")   == 0)   return STREAM_AUDIO_MPEG1;
  if (strcmp(inStr, "MPEG2")   == 0)   return STREAM_AUDIO_MPEG2;
  if (strcmp(inStr, "AC3plus") == 0)   return STREAM_AUDIO_MPEG4_AC3_PLUS;
  if (strcmp(inStr, "AAC")     == 0)   return STREAM_AUDIO_MPEG4_AAC;
  if (strcmp(inStr, "AACplus") == 0)   return STREAM_AUDIO_MPEG4_AAC_PLUS;
  if (strcmp(inStr, "AC3")     == 0)   return STREAM_AUDIO_MPEG4_AC3;
  if (strcmp(inStr, "DTS")     == 0)   return STREAM_AUDIO_MPEG4_DTS;
  if (strcmp(inStr, "unknown") == 0)   return STREAM_UNKNOWN;

  long ret;
  ret = strtol(inStr, NULL, 0);
  if (ret < 0xFFFF)
  {
    if (ret == 0)
      TAP_PrintNet("Warning: Suspekter AudioType-String: '%s'\n", inStr);
    return ret;
  }
  else
    TAP_PrintNet("Error: Ungültiger AudioType-String: '%s'\n", inStr);
  return STREAM_UNKNOWN;
}

bool ExportSettings_Text(void)
{
  tExportHeader         FileHeader;
  FILE                 *fExportFile = NULL;
  int                   FileSize = 0, FileSizePos = 0;
  int                   i, j;
  bool                  ret = FALSE;

  TRACEENTER();
  TAP_PrintNet("Starte Text Export...\n");

  fExportFile = fopen(TAPFSROOT "/" EXPORTFILENAME ".txt", "w");
  if(fExportFile)
  {
    ret = TRUE;

    // Write the file header
    ret = (fprintf(fExportFile, "[ChannelListSaver]" CRLF)               > 0) && ret;
    ret = (fprintf(fExportFile, "FileVersion=%d" CRLF,  1)               > 0) && ret;
    FileSizePos = ftell(fExportFile);
    ret = (fprintf(fExportFile, "FileSize=%010d" CRLF,  0)               > 0) && ret;
    ret = (fprintf(fExportFile, "SystemType=%d" CRLF,   GetSystemType()) > 0) && ret;
    ret = (fprintf(fExportFile, "UTF8System=%d" CRLF,   isUTFToppy())    > 0) && ret;

    TAP_Channel_GetTotalNum(&FileHeader.NrTVServices, &FileHeader.NrRadioServices);
    fprintf(fExportFile, "NrSatellites=%d" CRLF, FlashSatTablesGetTotal());
    fprintf(fExportFile, "NrTransponders=%d" CRLF, 0);
    fprintf(fExportFile, "NrTVServices=%d" CRLF, FileHeader.NrTVServices);
    fprintf(fExportFile, "NrRadioServices=%d" CRLF, FileHeader.NrRadioServices);
    fprintf(fExportFile, "NrFavGroups=%d" CRLF, NrFavGroups);
    fprintf(fExportFile, CRLF);

    //[Satellites]
    //#  ;                 ;       ;      ;    Supply;;      DiSEqC10;;    DiSEqC11;;    DiSeqC12;;    DiSEqC12Flags;;      Universal;;    Switch22;;     LowBand;;        HBFrq;;        Loop;;       Unused1;;     Unused2;;     Unused3;;     Unused4;;                   Unused5;;                ;                                                                   ;
    //#Nr; SatName         ; SatPos; NrTps;   LNB1; LNB2;   LNB1; LNB2;   LNB1; LNB2;   LNB1; LNB2;      LNB1; LNB2;       LNB1;  LNB2;   LNB1;  LNB2;   LNB1;  LNB2;   LNB1;  LNB2;   LNB1; LNB2;   LNB1; LNB2;   LNB1; LNB2;   LNB1; LNB2;   LNB1; LNB2;             LNB1;           LNB2;   Unused1;  Unknown1                                                         ;  Unused2
    //  1; Astra           ;    192;   136;     1;    1;      0;    0;      0;    0;      4;    4;   FE 7F 7F; FE 7F 7F;      1;     1;      0;     0;   9750;  9750;  10600; 10600;     1;    1;       0;    0;      0;    0;      0;    0;      0;    0;   FE 40 43 FF FF; FE 40 43 FF FF;    0xffff;  FF FF FF FF 01 01 00 01 00 00 00 00 B2 2E 02 04 6E 00 78 00 6E 00;  FF FF FF FF FF FF FF FF
    {
      tFlashSatTable         CurSat;
      char                   StringBuf1[100], StringBuf2[100];

      ret = (fprintf(fExportFile, "[Satellites]" CRLF)                   > 0) && ret;
//      ret = (fprintf(fExportFile, "#Nr; SatName;          SatPos; NrTps; LNB1Supply; LNB1DiSEqC10; LNB1DiSEqC11; LNB1DiSeqC12; LNB1DiSEqC12Flags; LNB1Universal; LNB1Switch22; LNB1LowBand; LNB1HBFrq; LNB1Loop; LNB1Unused1; LNB1Unused2; LNB1Unused3; LNB1Unused4; LNB1Unused5; LNB2Supply; LNB2DiSEqC10; LNB2DiSEqC11; LNB2DiSeqC12; LNB2DiSEqC12Flags; LNB2Universal; LNB2Switch22; LNB2LowBand; LNB2HBFrq; LNB2Loop; LNB2Unused1; LNB2Unused2; LNB2Unused3; LNB2Unused4; LNB2Unused5; Unused1; Unknown1; Unused2" CRLF) > 0) && ret;
      ret = (fprintf(fExportFile, "#  ;                 ;       ;      ;    Supply;;      DiSEqC10;;    DiSEqC11;;    DiSeqC12;;    DiSEqC12Flags;;      Universal;;    Switch22;;     LowBand;;        HBFrq;;    LoopThrough;;   Unused1;;     Unused2;;     Unused3;;     Unused4;;                    Unused5;;                ;                                                                   ;" CRLF) > 0) && ret;
      ret = (fprintf(fExportFile, "#Nr; SatName         ; SatPos; NrTps;   LNB1; LNB2;   LNB1; LNB2;   LNB1; LNB2;   LNB1; LNB2;      LNB1; LNB2;       LNB1;  LNB2;   LNB1;  LNB2;   LNB1;  LNB2;   LNB1;  LNB2;   LNB1; LNB2;   LNB1; LNB2;   LNB1; LNB2;   LNB1; LNB2;   LNB1; LNB2;             LNB1;           LNB2;   Unused1;  Unknown1                                                         ;  Unused2" CRLF) > 0) && ret;

      FileHeader.NrSatellites = FlashSatTablesGetTotal();
      for(i = 0; i < FileHeader.NrSatellites; i++)
      {
        if (FlashSatTablesGetInfo(i, &CurSat))
        {
          // Nr; SatName; SatPosition; NrTransponders;
          ret = (fprintf(fExportFile, "%3d; %-16s;  %5hu; %5hu;   ",
                                       i, CurSat.SatName, CurSat.SatPosition, CurSat.NrOfTransponders) > 0) && ret;
/*          for (j = 0; j <= 1; j++)
          {
            // LNBxSupply; LNBxDiSEqC10; LNBxDiSEqC11; LNBxDiSeqC12; LNBxDiSEqC12Flags; LNBxUniversal; LNBxSwitch22; LNBxLowBand; LNBxHBFrq; LNBxLoop; LNBxUnused1; LNBxUnused2; LNBxUnused3; LNBxUnused4; LNBxUnused5;
            ByteArrToStr(StringBuf1, CurSat.LNB[j].DiSEqC12Flags, sizeof(CurSat.LNB[j].DiSEqC12Flags));
            ByteArrToStr(StringBuf2, CurSat.LNB[j].unused5, sizeof(CurSat.LNB[j].unused5));
            ret = (fprintf(fExportFile, "%3hhu; %3hhu; %3hhu; %3hhu; %s; %5hu; %5hu; %5hu; %5hu; %3hhu; %#4x; %#4x; %#4x; %#4x; %s; ",
                                         CurSat.LNB[j].LNBSupply, CurSat.LNB[j].DiSEqC10, CurSat.LNB[j].DiSEqC11, CurSat.LNB[j].DiSEqC12, StringBuf1, CurSat.LNB[j].UniversalLNB, CurSat.LNB[j].Switch22, CurSat.LNB[j].LowBand, CurSat.LNB[j].HBFrq, CurSat.LNB[j].LoopThrough, CurSat.LNB[j].unused1, CurSat.LNB[j].unused2, CurSat.LNB[j].unused3, CurSat.LNB[j].unused4, StringBuf2) > 0) && ret;
          } */

          // LNBxSupply; LNBxDiSEqC10; LNBxDiSEqC11; LNBxDiSeqC12; LNBxDiSEqC12Flags; LNBxUniversal; LNBxSwitch22; LNBxLowBand; LNBxHBFrq; LNBxLoop
          ByteArrToStr(StringBuf1, CurSat.LNB[0].DiSEqC12Flags, sizeof(CurSat.LNB[0].DiSEqC12Flags));
          ByteArrToStr(StringBuf2, CurSat.LNB[1].DiSEqC12Flags, sizeof(CurSat.LNB[1].DiSEqC12Flags));
          ret = (fprintf(fExportFile, "%3hhu;  %3hhu;    %3hhu;  %3hhu;    %3hhu;  %3hhu;    %3hhu;  %3hhu;   %s; %s;  %5hu; %5hu;  %5hu; %5hu;  %5hu; %5hu;  %5hu; %5hu;   %3hhu;  %3hhu;    ",
                                       CurSat.LNB[0].LNBSupply, CurSat.LNB[1].LNBSupply,  CurSat.LNB[0].DiSEqC10, CurSat.LNB[1].DiSEqC10,  CurSat.LNB[0].DiSEqC11, CurSat.LNB[1].DiSEqC11,  CurSat.LNB[0].DiSEqC12, CurSat.LNB[1].DiSEqC12,  StringBuf1, StringBuf2,  CurSat.LNB[0].UniversalLNB, CurSat.LNB[1].UniversalLNB,  CurSat.LNB[0].Switch22, CurSat.LNB[1].Switch22,  CurSat.LNB[0].LowBand, CurSat.LNB[1].LowBand,  CurSat.LNB[0].HBFrq, CurSat.LNB[1].HBFrq,  CurSat.LNB[0].LoopThrough, CurSat.LNB[1].LoopThrough) > 0) && ret;

          // LNBxUnused1; LNBxUnused2; LNBxUnused3; LNBxUnused4; LNBxUnused5;
          ByteArrToStr(StringBuf1, CurSat.LNB[0].unused5, sizeof(CurSat.LNB[0].unused5));
          ByteArrToStr(StringBuf2, CurSat.LNB[1].unused5, sizeof(CurSat.LNB[1].unused5));
          ret = (fprintf(fExportFile, "%#4x; %#4x;   %#4x; %#4x;   %#4x; %#4x;   %#4x; %#4x;   %s; %s;    ",
                                       CurSat.LNB[0].unused1, CurSat.LNB[1].unused1,  CurSat.LNB[0].unused2, CurSat.LNB[1].unused2,  CurSat.LNB[0].unused3, CurSat.LNB[1].unused3,  CurSat.LNB[0].unused4, CurSat.LNB[1].unused4,  StringBuf1, StringBuf2) > 0) && ret;

          // Unused1; Unknown1; Unused2
          ByteArrToStr(StringBuf1, CurSat.unknown1, sizeof(CurSat.unknown1));
          ByteArrToStr(StringBuf2, CurSat.unused2, sizeof(CurSat.unused2));
          ret = (fprintf(fExportFile, "%#6x;  %s;  %s" CRLF,
                                       CurSat.unused1, StringBuf1, StringBuf2) > 0) && ret;
        }
        else
          TAP_PrintNet("Failed to decode sat %d!\n", i);
      }
      fprintf(fExportFile, CRLF);
    }
    TAP_PrintNet((ret) ? "%d Satellites exportiert.\n" : "Satellites Fehler!\n", i);

    //[Transponders]
    //# Nr; SatIdx;  Frequency; SymbRate; Channel;  BW;  TSID;  ONWID;   NWID; Pilot;      FEC; Modulation; System; Pol; LPHP; ClockSync; Unknown1; Unknown2; Unknown3; Unknown4
    //   0;      1;      10729;    22000;       0;   0;  1050;    0x1;      0;     n;      2/3;       8PSK;  DVBS2;   V;    0;         n;        0;        0;        0;        0
    {
      tFlashTransponderTable CurTransponder;
      int                    NrTransponders;
      char                   StringBuf1[10], StringBuf2[10];

      ret = (fprintf(fExportFile, "[Transponders]" CRLF)             > 0) && ret;
      for(i = 0; i < FileHeader.NrSatellites; i++)
      {
        ret = (fprintf(fExportFile, "# Nr; SatIdx;  Frequency; SymbRate; Channel;  BW;  TSID;  ONWID;   NWID; Pilot;      FEC; Modulation; System; Pol; LPHP; ClockSync; Unknown1; Unknown2; Unknown3; Unknown4" CRLF) > 0) && ret;
        NrTransponders = FlashTransponderTablesGetTotal(i);
        FileHeader.NrTransponders += NrTransponders;
        for(j = 0; j < NrTransponders; j++)
        {
          if (FlashTransponderTablesGetInfo(i, j, &CurTransponder))
          {
            ret = (fprintf(fExportFile, "%4d;    %3hhu; %10lu;    %5hu;     %3hhu; %3hhu; %5hu; %#6x; %#6x;     "  // Nr; SatIdx; Frequency; SymbRate; Channel; BW; TSID; ONWID; NWID
                                        "%c; %8s;   %8s;  "                                                        // Pilot; FEC; Modulation
                                        "%5s;   %c;  %3hhu;         %c;   "                                        // System; Pol; LPHP; ClockSync
                                        "%#6x;   %#6x;     %#4x;   %#6x" CRLF,                                     // Unknown1; Unknown2; Unknown3; Unknown4
                                         j, CurTransponder.SatIndex, CurTransponder.Frequency, CurTransponder.SymbolRate, CurTransponder.ChannelNr, CurTransponder.Bandwidth, CurTransponder.TSID, CurTransponder.OriginalNetworkID, CurTransponder.NetworkID,
                                         BoolToChar(CurTransponder.Pilot), FECtoStr(StringBuf1, CurTransponder.FEC), ModulationToStr(StringBuf2, CurTransponder.Modulation),
                                         (CurTransponder.ModSystem ? "DVBS2" : "DVBS"), (CurTransponder.Polarisation ? 'H' : 'V'), CurTransponder.LPHP, BoolToChar(CurTransponder.ClockSync),
                                         CurTransponder.unused1, CurTransponder.unused2, CurTransponder.unused3, CurTransponder.unused4) > 0) && ret;
          }
          else
            TAP_PrintNet("Failed to decode transponder %d!\n", j);
        }
        fprintf(fExportFile, CRLF);
      }
    }
    TAP_PrintNet((ret) ? "%d Transponders exportiert.\n" : "Transponders Fehler!\n", FileHeader.NrTransponders);

    //[Services]
    //# Nr; ServiceName            ; SatIdx; TrpIdx; Tuner; VideoType; AudioType;  SvcID; PMTPID; PCRPID; VidPID; AudPID;   LCN; FDel; FCAS; FLock; FSkip; NameLck; Flags2; Unknown2;           ProviderName
    //   0; Das Erste              ;      1;     44;     3;     MPEG2;     MPEG1;  28106;    100;    101;    101;  32870;     0;    n;    n;     n;     n;       n;    0x1; 00 00 00 00 DC DD;  ARD
    for (j = 0; j <= 1; j++)
    {
      tFlashService          CurService;
      char                  *CurSvcType, *p;
      char                   StringBuf1[20], StringBuf2[20], StringBuf3[20];

      CurSvcType = (j == 0) ? "TV" : "Radio";
      ret = (fprintf(fExportFile, "[%sServices]" CRLF, CurSvcType)       > 0) && ret;
      ret = (fprintf(fExportFile, "# Nr; ServiceName            ; SatIdx; TrpIdx; Tuner; VideoType; AudioType;  SvcID; PMTPID; PCRPID; VidPID; AudPID;   LCN; FDel; FCAS; FLock; FSkip; NameLck; Flags2;          Unknown2;  ProviderName" CRLF) > 0) && ret;

      for(i = 0; i < ((j == 0) ? FileHeader.NrTVServices : FileHeader.NrRadioServices); i++)
      {
        if (FlashServiceGetInfo(((j == 0) ? SVC_TYPE_Tv : SVC_TYPE_Radio), i, &CurService))
        {
          while ((p = strchr(CurService.ServiceName, ';')))
            p[0] = '_';
          while ((p = strchr(CurService.ProviderName, ';')))
            p[0] = '_';

          ByteArrToStr(StringBuf3, CurService.unknown2, sizeof(CurService.unknown2));
          ret = (fprintf(fExportFile, "%4d; %-23s;    %3hhu;  %5hu;   %3hhu;  "                                    // Nr; ServiceName; SatIndex; TransponderIndex; Tuner;
                                      "%8s;  %8s;  %5hu;  %5hu;  %5hu;  %5hu;  %5hu; "                             // VideoStreamType; AudioStreamType; ServiceID; PMTPID; PCRPID; VideoPID; AudioPID;
                                      "%5hu;    %c;    %c;     %c;     %c;       %c; %#6x; %s;  %s" CRLF,          // LCN; FlagDelete; FlagCAS; FlagLock; FlagSkip; NameLock; Flags2; Unknown2; ProviderName
                                       i, CurService.ServiceName, CurService.SatIndex, CurService.TransponderIndex, CurService.Tuner,
                                       VideoTypeToStr(StringBuf1, CurService.VideoStreamType), AudioTypeToStr(StringBuf2, CurService.AudioStreamType), CurService.ServiceID, CurService.PMTPID, CurService.PCRPID, CurService.VideoPID, CurService.AudioPID,
                                       CurService.LCN, BoolToChar(CurService.FlagDelete), BoolToChar(CurService.FlagCAS), BoolToChar(CurService.FlagLock), BoolToChar(CurService.FlagSkip), BoolToChar(CurService.NameLock), CurService.Flags2, StringBuf3, CurService.ProviderName) > 0) && ret;
        }
        else
          TAP_PrintNet("Failed to decode %s service %d!\n", CurSvcType, i);
      }
      fprintf(fExportFile, CRLF);
      TAP_PrintNet((ret) ? "%d %sServices exportiert.\n" : "%d %sServices Fehler!\n", i, CurSvcType);
    }

    //[Favorites]
    // GroupName = T/R, SvcNums
    {
      tFavorites             CurFavGroup;

      ret = (fprintf(fExportFile, "[Favorites]" CRLF)                   > 0) && ret;
      ret = (fprintf(fExportFile, "#GroupName = T/R, SvcNums" CRLF)     > 0) && ret;

      FileHeader.NrFavGroups = NrFavGroups;
      FileHeader.NrSvcsPerFavGroup = NrFavsPerGroup;
      for (i = 0; i < FileHeader.NrFavGroups; i++)
      {
        if (FlashFavoritesGetInfo(i, &CurFavGroup))
        {
          ret = (fprintf(fExportFile, "%s = %c" , CurFavGroup.GroupName, ((CurFavGroup.SvcType[0]==0) ? 'T' : 'R')) > 0) && ret;
          for (j = 0; j < CurFavGroup.NrEntries; j++)
            ret = (fprintf(fExportFile, ", %hu", CurFavGroup.SvcNum[j]) > 0) && ret;
//          for (j = 0; j < CurFavGroup.NrEntries; j++)
//            ret = (fprintf(fExportFile, (j > 0) ? ", %hhu": CRLF "%hhu", CurFavGroup.SvcType[j]) > 0) && ret;
//          ret = (fprintf(fExportFile, CRLF "%#01X" CRLF CRLF, CurFavGroup.unused1) > 0) && ret;
          ret = (fprintf(fExportFile, CRLF)                             > 0) && ret;
        }
//        else
//          TAP_PrintNet("Failed to decode favorite group %d!\n", i);
      }
    }
    TAP_PrintNet((ret) ? "Favorites ok\n" : "Favorites Fehler\n");


    FileSize = ftell(fExportFile);
    fclose(fExportFile);

    fExportFile = fopen(TAPFSROOT "/" EXPORTFILENAME ".txt", "r+");
    if(fExportFile)
    {
      fseek(fExportFile, FileSizePos, SEEK_SET);
      ret = (fprintf(fExportFile, "FileSize=%010d" CRLF, FileSize) > 0) && ret;
      fclose(fExportFile);
    }
  }
  else
    TAP_PrintNet("Datei nicht gefunden\n");

  if (!ret)
    if(TAP_Hdd_Exist(EXPORTFILENAME ".txt")) TAP_Hdd_Delete(EXPORTFILENAME ".txt");
  TAP_PrintNet((ret) ? "Export erfolgreich\n" : "Export fehlgeschlagen\n");
  TRACEEXIT();
  return ret;
}

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

bool ImportSettings_Text(void)
{
  tExportHeader         FileHeader;
  size_t                BufSize = 0;
  char                 *Buffer = NULL;
  FILE                 *fImportFile = NULL;
  unsigned long         fs;
  tScanMode             CurMode = SM_Start;
  bool                  HeaderCheck[3];
  int                   NrImpSatellites=0, NrImpTransponders=0, NrImpTVServices=0, NrImpRadioServices=0, NrImpFavGroups=0;
  bool                  ret = FALSE;
  int                   i, j;

  TRACEENTER();
  TAP_PrintNet("Starte Text Import...\n");

//  Buffer = (char*) malloc(BufSize);
//  if (Buffer)
  fImportFile = fopen(TAPFSROOT "/" EXPORTFILENAME ".txt", "r");
  if (fImportFile)
  {
    ret = TRUE;

    // Dateigröße bestimmen
    fseek(fImportFile, 0, SEEK_END);
    fs = ftell(fImportFile);
    rewind(fImportFile);

    while (ret && (getline(&Buffer, &BufSize, fImportFile) >= 0))
    {
      int p = strlen(Buffer);
      while ((Buffer[p-1] == '\r') || (Buffer[p-1] == '\n'))
      {
        Buffer[p-1] = '\0';
        if (p > 0) p = p-1;
      }
TAP_PrintNet("%d: %s\n", CurMode, Buffer);

      // Prüfung der ersten Zeile
      if (CurMode == SM_Start)
      {
        if (strcmp(Buffer, "[ChannelListSaver]") == 0)
        {
          CurMode = SM_Header;
          for (i = 0; i < (int)sizeof(HeaderCheck); i++);
            HeaderCheck[i] = FALSE;
          continue;
        }
        else
        {
          TAP_PrintNet("Fehler: Ungültiges Dateiformat!\n");
          ret = FALSE;
          break;
        }
      }

      // Kommentare und Sektionen
      switch (Buffer[0])
      {
        case '\0':
          continue;

        case '%':
        case ';':
        case '#':
        case '/':
          continue;

        // Neue Sektion gefunden
        case '[':
        {
          if (CurMode == SM_Header)
          {
            TAP_PrintNet("Fehler: Ungültiger Header!\n");
            ret = FALSE;
            continue;
          }
          if (strcmp(Buffer, "[Satellites]") == 0)
            CurMode = SM_Satellites;
          else if (strcmp(Buffer, "[Transponders]") == 0)
            CurMode = SM_Satellites;
          else if (strcmp(Buffer, "[TVServices]") == 0)
            CurMode = SM_TVServices;
          else if (strcmp(Buffer, "[RadioServices]") == 0)
            CurMode = SM_RadioServices;
          else if (strcmp(Buffer, "[Favorites]") == 0)
            CurMode = SM_Favorites;
          else
          {
            TAP_PrintNet("Warnung: Unbekannte Sektion: %s!\n", Buffer);
            CurMode = SM_Ignore;
          }
          continue;
        }
      }

      // Header überprüfen
      if ((CurMode == SM_Header) || (CurMode == SM_HeaderChecked))
      {
        char            Name[50];
        unsigned long   Value;

        if (sscanf(Buffer, "%49[^= ] = %lu", Name, &Value) == 2)
        {
          if (strcmp(Name, "FileVersion") == 0)
          {
            FileHeader.FileVersion = Value;
            if (Value == 1)
              HeaderCheck[0] = TRUE;
          }
          else if (strcmp(Name, "FileSize") == 0)
          {
            FileHeader.FileSize = Value;
            if (Value == fs)
              HeaderCheck[1] = TRUE;
          }
          else if (strcmp(Name, "SystemType") == 0)
          {
            FileHeader.SystemType = (SYSTEM_TYPE) Value;
            if (Value == CurSystemType)
              HeaderCheck[2] = TRUE;
          }
          else if (strcmp(Name, "UTF8System") == 0)
            FileHeader.UTF8System = Value;
          else if (strcmp(Name, "NrSatellites") == 0)
            FileHeader.NrSatellites = Value;
          else if (strcmp(Name, "NrTransponders") == 0)
            FileHeader.NrTransponders = Value;
          else if (strcmp(Name, "NrTVServices") == 0)
            FileHeader.NrTVServices = Value;
          else if (strcmp(Name, "NrRadioServices") == 0)
            FileHeader.NrRadioServices = Value;
          else if (strcmp(Name, "NrFavGroups") == 0)
            FileHeader.NrFavGroups = Value;

          if (HeaderCheck[0] && HeaderCheck[1] && HeaderCheck[2])
            CurMode = SM_HeaderChecked;
        }
        continue;
      }

      // Beliebige Zeile gefunden -> importieren!
      switch (CurMode)
      {
        //[Satellites]
        //#  ;                 ;       ;      ;    Supply;;      DiSEqC10;;    DiSEqC11;;    DiSeqC12;;    DiSEqC12Flags;;      Universal;;    Switch22;;     LowBand;;        HBFrq;;        Loop;;       Unused1;;     Unused2;;     Unused3;;     Unused4;;                   Unused5;;                ;                                                                   ;
        //#Nr; SatName         ; SatPos; NrTps;   LNB1; LNB2;   LNB1; LNB2;   LNB1; LNB2;   LNB1; LNB2;      LNB1; LNB2;       LNB1;  LNB2;   LNB1;  LNB2;   LNB1;  LNB2;   LNB1;  LNB2;   LNB1; LNB2;   LNB1; LNB2;   LNB1; LNB2;   LNB1; LNB2;   LNB1; LNB2;             LNB1;           LNB2;   Unused1;  Unknown1                                                         ;  Unused2
        //  1; Astra           ;    192;   136;     1;    1;      0;    0;      0;    0;      4;    4;   FE 7F 7F; FE 7F 7F;      1;     1;      0;     0;   9750;  9750;  10600; 10600;     1;    1;       0;    0;      0;    0;      0;    0;      0;    0;   FE 40 43 FF FF; FE 40 43 FF FF;    0xffff;  FF FF FF FF 01 01 00 01 00 00 00 00 B2 2E 02 04 6E 00 78 00 6E 00;  FF FF FF FF FF FF FF FF
        case SM_Satellites:
        {
          tFlashSatTable         CurSat;
          char                   StringBuf1[100], StringBuf2[100];
          byte                   LNBSupply[2], DiSEqC10[2], LoopThrough[2], unused1[2], unused2[2], unused3[2], unused4[2];
          word                   UniversalLNB[2], Switch22[2], LowBand[2];

          memset(&CurSat, 0, sizeof(tFlashSatTable));

          // Nr; SatName; SatPosition; NrTransponders;
          ret = (sscanf(Buffer, "%i ; %15[^;\r\n] ; %hi ; %hi ; ",
                                  &i, CurSat.SatName, &CurSat.SatPosition, &CurSat.NrOfTransponders) == 4) && ret;
          RTrim(CurSat.SatName);

          // LNBxSupply; LNBxDiSEqC10; LNBxDiSEqC11; LNBxDiSeqC12; LNBxDiSEqC12Flags; LNBxUniversal; LNBxSwitch22; LNBxLowBand; LNBxHBFrq; LNBxLoop
          ret = (sscanf(Buffer, "%hhi ; %hhi ;  %hhi ; %hhi ;  %hhi ; %hhi ;  %hhi ; %hhi ;  %9[^;\r\n] ; %9[^;\r\n] ;  %hi ; %hi ;  %hi ; %hi ;  %hi ; %hi ;  %hi ; %hi ;  %hhi ; %hhi ; ",
                                  &LNBSupply[0], &LNBSupply[1],  &DiSEqC10[0], &DiSEqC10[1],  &CurSat.LNB[0].DiSEqC11, &CurSat.LNB[1].DiSEqC11,  &CurSat.LNB[0].DiSEqC12, &CurSat.LNB[1].DiSEqC12,  StringBuf1, StringBuf2,  &UniversalLNB[0], &UniversalLNB[1],  &Switch22[0], &Switch22[1],  &LowBand[0], &LowBand[1],  &CurSat.LNB[0].HBFrq, &CurSat.LNB[1].HBFrq,  &LoopThrough[0], &LoopThrough[1]) == 20) && ret;
          ret = StrToByteArr(CurSat.LNB[0].DiSEqC12Flags, StringBuf1, sizeof(CurSat.LNB[0].DiSEqC12Flags)) && ret;
          ret = StrToByteArr(CurSat.LNB[1].DiSEqC12Flags, StringBuf2, sizeof(CurSat.LNB[1].DiSEqC12Flags)) && ret;

          // LNBxUnused1; LNBxUnused2; LNBxUnused3; LNBxUnused4; LNBxUnused5;
          ret = (sscanf(Buffer, "%hhi ; %hhi ;  %hhi ; %hhi ;  %hhi ; %hhi ;  %hhi ; %hhi ;  %15[^;\r\n] ; %15[^;\r\n] ; ",
                                  &unused1[0], &unused1[1],  &unused2[0], &unused2[1],  &unused3[0], &unused3[1],  &unused4[0], &unused4[1],  StringBuf1, StringBuf2) == 10) && ret;
          ret = StrToByteArr(CurSat.LNB[0].unused5, StringBuf1, sizeof(CurSat.LNB[0].unused5)) && ret;
          ret = StrToByteArr(CurSat.LNB[1].unused5, StringBuf2, sizeof(CurSat.LNB[1].unused5)) && ret;

          for(j = 0; j <= 1; j++)
          {
            CurSat.LNB[j].LNBSupply    = LNBSupply[j];
            CurSat.LNB[j].DiSEqC10     = DiSEqC10[j];
            CurSat.LNB[j].UniversalLNB = UniversalLNB[j];
            CurSat.LNB[j].Switch22     = Switch22[j];
            CurSat.LNB[j].LowBand      = LowBand[j];
            CurSat.LNB[j].LoopThrough  = LoopThrough[j];
            CurSat.LNB[j].unused1      = unused1[j];
            CurSat.LNB[j].unused2      = unused2[j];
            CurSat.LNB[j].unused3      = unused3[j];
            CurSat.LNB[j].unused4      = unused4[j];
          }

          // Unused1; Unknown1; Unused2
          ret = (sscanf(Buffer, "%hi ; %66[^;\r\n] ; %24[^;\r\n]" CRLF,
                                  &CurSat.unused1, StringBuf1, StringBuf2) == 3) && ret;
          ret = StrToByteArr(CurSat.unknown1, StringBuf1, sizeof(CurSat.unknown1)) && ret;
          ret = StrToByteArr(CurSat.unused2, StringBuf2, sizeof(CurSat.unused2)) && ret;

          CurSat.NrOfTransponders = 0;
//          if (ret)
//            ret = FlashSatTablesSetInfo(NrImpSatellites, &CurSat);

          if (ret)
            NrImpSatellites++;
          else
            TAP_PrintNet("Fehler in Satellit Nr. %d!\n", NrImpSatellites);
          break;
        }

        //[Transponders]
        //# Nr; SatIdx;  Frequency; SymbRate; Channel;  BW;  TSID;  ONWID;   NWID; Pilot;      FEC; Modulation; System; Pol; LPHP; ClockSync; Unknown1; Unknown2; Unknown3; Unknown4
        //   0;      1;      10729;    22000;       0;   0;  1050;    0x1;      0;     n;      2/3;       8PSK;  DVBS2;   V;    0;         n;        0;        0;        0;        0
        case SM_Transponders:
        {
          tFlashTransponderTable CurTransponder;
          char                   StringBuf1[10], StringBuf2[10], StringBuf3[10];
          char                   CharPilot, CharPolarisation, CharClockSync;

          memset(&CurTransponder, 0, sizeof(tFlashTransponderTable));

          ret = (sscanf(Buffer, "%i ; %hhi ; %li ; %hi ; %hhi ; %hhi ; %hi ; %hi ; %hi ; "       // Nr; SatIdx; Frequency; SymbRate; Channel; BW; TSID; ONWID; NWID
                                "%c ; %8[^;\r\n] ; %8[^;\r\n] ; "                                // Pilot; FEC; Modulation
                                "%6[^;\r\n] ; %c ; %hhi ; %c ; "                                 // System; Pol; LPHP; ClockSync
                                "%hi ; %hi ; %hhi ; %hi",                                        // Unknown1; Unknown2; Unknown3; Unknown4
                                  &j, &CurTransponder.SatIndex, &CurTransponder.Frequency, &CurTransponder.SymbolRate, &CurTransponder.ChannelNr, &CurTransponder.Bandwidth, &CurTransponder.TSID, &CurTransponder.OriginalNetworkID, &CurTransponder.NetworkID,
                                  &CharPilot, StringBuf1, StringBuf2,
                                  StringBuf3, &CharPolarisation, &CurTransponder.LPHP, &CharClockSync,
                                  &CurTransponder.unused1, &CurTransponder.unused2, &CurTransponder.unused3, &CurTransponder.unused4) == 20) && ret;
          CurTransponder.Pilot      = CharToBool(CharPilot);
          CurTransponder.FEC        = StrToFEC(StringBuf1);
          CurTransponder.Modulation = StrToModulation(StringBuf2);
          if (strncmp(StringBuf3, "DVBS2", 5))
            CurTransponder.ModSystem = 1;
          if (CharPolarisation == 'H')
            CurTransponder.Polarisation = 1;
          CurTransponder.ClockSync = CharToBool(CharClockSync);

          if (ret && (CurTransponder.SatIndex < NrImpSatellites))
            ret = TRUE;
//            ret = FlashTransponderTablesAdd(CurTransponder.SatIndex, &CurTransponder);
          else
            ret = FALSE;

          if (ret)
            NrImpTransponders++;
          else
            TAP_PrintNet("Fehler in Transponder Nr. %d!\n", NrImpTransponders);
          break;
        }

        //[Services]
        //# Nr; ServiceName            ; SatIdx; TrpIdx; Tuner; VideoType; AudioType;  SvcID; PMTPID; PCRPID; VidPID; AudPID;   LCN; FDel; FCAS; FLock; FSkip; NameLck; Flags2; Unknown2;           ProviderName
        //   0; Das Erste              ;      1;     44;     3;     MPEG2;     MPEG1;  28106;    100;    101;    101;  32870;     0;    n;    n;     n;     n;       n;    0x1; 00 00 00 00 DC DD;  ARD
        case SM_TVServices:
        case SM_RadioServices:
        {
          tFlashService          CurService;
          char                   StringBuf1[10], StringBuf2[10], StringBuf3[20];
          char                   CharFlagDel, CharFlagCAS, CharFlagLock, CharFlagSkip, CharNameLock;

          memset(&CurService, 0, sizeof(tFlashService));

          ret = (sscanf(Buffer, "%i ; %22[^;\r\n] ; %hhi ; %hi ; %hhi ; "                          // Nr; ServiceName; SatIndex; TransponderIndex; Tuner;
                                "%8[^;\r\n] ; %8[^;\r\n] ; %hi ; %hi ; %hi ; %hi ; %hi ; "         // VideoStreamType; AudioStreamType; ServiceID; PMTPID; PCRPID; VideoPID; AudioPID;
                                "%hi ; %c ; %c ; %c ; %c ; %c ; %hi ; %18[^;\r\n] ; %39[^;\r\n]",  // LCN; FlagDelete; FlagCAS; FlagLock; FlagSkip; NameLock; Flags2; Unknown2; ProviderName
                                  &i, CurService.ServiceName, &CurService.SatIndex, &CurService.TransponderIndex, &CurService.Tuner,
                                  StringBuf1, StringBuf2, &CurService.ServiceID, &CurService.PMTPID, &CurService.PCRPID, &CurService.VideoPID, &CurService.AudioPID,
                                  &CurService.LCN, &CharFlagDel, &CharFlagCAS, &CharFlagLock, &CharFlagSkip, &CharNameLock, &CurService.Flags2, StringBuf3, CurService.ProviderName) == 21) && ret;

          RTrim(CurService.ServiceName);
          RTrim(CurService.ProviderName);
          CurService.VideoStreamType = StrToVideoType(StringBuf1);
          CurService.AudioStreamType = StrToAudioType(StringBuf2);
          CurService.FlagDelete = CharToBool(CharFlagDel);
          CurService.FlagCAS = CharToBool(CharFlagCAS);
          CurService.FlagLock = CharToBool(CharFlagLock);
          CurService.FlagSkip = CharToBool(CharFlagSkip);
          CurService.NameLock = CharToBool(CharNameLock);
          ret = StrToByteArr(CurService.unknown2, StringBuf3, sizeof(CurService.unknown2)) && ret;

          if (ret && (CurService.SatIndex < NrImpSatellites) && (CurService.TransponderIndex < NrImpTransponders))
            ret = FlashServiceAdd((CurMode==SM_TVServices) ? SVC_TYPE_Tv : SVC_TYPE_Radio, &CurService);
          else
            ret = FALSE;

          if (ret)
            ret = TRUE;
//            (CurMode==SM_TVServices) ? NrImpTVServices++ : NrImpRadioServices++;
          else
            TAP_PrintNet("Fehler in %s-Service Nr. %d!\n", (CurMode==SM_TVServices) ? "TV" : "Radio", (CurMode==SM_TVServices) ? NrImpTVServices : NrImpRadioServices);
          break;
        }

        //[Favorites]
        // GroupName = T/R, SvcNums
        case SM_Favorites:
        {
          tFavorites             CurFavGroup;
          word                   curSvcNum;
          char                   curSvcType;
          int                    p;

          memset(&CurFavGroup, 0, sizeof(tFavorites));
          sscanf(Buffer, "%11s = %c%n", CurFavGroup.GroupName, &curSvcType, &p);

          j = 0;
          while (j < NrFavsPerGroup)
          {
            curSvcNum = strtol(&Buffer[p], NULL, 0);
            if ((curSvcNum > 0) && (curSvcNum < NrImpTVServices + NrImpRadioServices))
            {
              CurFavGroup.SvcNum[j] = curSvcNum;
              CurFavGroup.SvcType[j] = (curSvcType == 'T' ? SVC_TYPE_Tv : SVC_TYPE_Radio);
              CurFavGroup.NrEntries++;
            }
            j++;
          }
//          for (j = 0; j < CurFavGroup.NrEntries; j++)
//            ret = (fprintf(fExportFile, (j > 0) ? ", %hhu": CRLF "%hhu", CurFavGroup.SvcType[j]) > 0) && ret;
//          ret = (fprintf(fExportFile, CRLF "%#01X" CRLF CRLF, CurFavGroup.unused1) > 0) && ret;

          // bescheuerter Workaround für zu strenge Prüfung in FlashFavoritesSetInfo()
//          (p + i * SIZE_Favorites)[0] = '*';
          TAP_PrintNet("FavGroup %d: Name = '%s', Entries = %d\n", i, CurFavGroup.GroupName, CurFavGroup.NrEntries);

//          if (ret)
//            ret = FlashFavoritesSetInfo(NrImpFavGroups, &CurFavGroup);

          if (ret)
            NrImpFavGroups++;
          else
            TAP_PrintNet("Fehler in FavGroup Nr. %d!\n", NrImpFavGroups);
          break;
        }

        default:
          break;
      }
    }
    fclose(fImportFile);
  }
  else
    TAP_PrintNet("Datei nicht gefunden\n");

  free(Buffer);
//  if(ret)
//    FlashProgram();

  TAP_PrintNet("%d von %d Satelliten, %d von %d Transponder, %d von %d TVServices, %d von %d RadioServices, %d von %d FavGroups importiert.\n",
                NrImpSatellites, FileHeader.NrSatellites, NrImpTransponders, FileHeader.NrTransponders, NrImpTVServices, FileHeader.NrTVServices, NrImpRadioServices, FileHeader.NrRadioServices, NrImpFavGroups, FileHeader.NrFavGroups);

  TAP_PrintNet((ret) ? "Export erfolgreich\n" : "Export fehlgeschlagen\n");
  TRACEEXIT();
  return ret;
}


bool ExportSettings()
{
  tExportHeader         FileHeader;
  FILE                 *fExportFile = NULL;
  int                   i;
  bool                  ret = FALSE;

  TRACEENTER();
  TAP_PrintNet("Starte Export...\n");

  fExportFile = fopen(TAPFSROOT "/" EXPORTFILENAME ".dat", "wb");
  if(fExportFile)
  {
    ret = TRUE;

    // Write the file header
    memset(&FileHeader, 0, sizeof(FileHeader));
    strncpy(FileHeader.Magic, "TFchan", 6);
    FileHeader.FileVersion = 1;
    FileHeader.SystemType = GetSystemType();
    FileHeader.UTF8System = isUTFToppy();

    // Now write the data blocks to the file
    ret = fwrite(&FileHeader, sizeof(tExportHeader), 1, fExportFile) && ret;
    {
      TYPE_SatInfo_TMSS *p;
      FileHeader.SatellitesOffset = ftell(fExportFile);
      p = (TYPE_SatInfo_TMSS*)FIS_vFlashBlockSatInfo();
      if(p)
      {
        FileHeader.NrSatellites = FlashSatTablesGetTotal();
        ret = fwrite(&FileHeader.NrSatellites, sizeof(FileHeader.NrSatellites), 1, fExportFile) && ret;
        FileHeader.SatellitesOffset = ftell(fExportFile);
        if (FileHeader.NrSatellites > 0)
          ret = fwrite(p, SIZE_SatInfo_TMSx, FileHeader.NrSatellites, fExportFile) && ret;
      }
    }
    TAP_PrintNet((ret) ? "%d Satellites exportiert.\n" : "Satellites Fehler!\n", FileHeader.NrSatellites);

    {
      TYPE_TpInfo_TMSS *p;
      FileHeader.TranspondersOffset = ftell(fExportFile);
      p = (TYPE_TpInfo_TMSS*)(FIS_vFlashBlockTransponderInfo());
      if(p)
      {
        for(i = 0; i < FileHeader.NrSatellites; i++)
          FileHeader.NrTransponders += FlashTransponderTablesGetTotal(i);
        ret = fwrite(&FileHeader.NrTransponders, sizeof(FileHeader.NrTransponders), 1, fExportFile) && ret;
        FileHeader.TranspondersOffset = ftell(fExportFile);
        if (FileHeader.NrTransponders > 0)
          ret = fwrite(p, SIZE_TpInfo_TMSx, FileHeader.NrTransponders, fExportFile) && ret;
      }
    }
    TAP_PrintNet((ret) ? "%d Transponders exportiert.\n" : "Transponders Fehler!\n", FileHeader.NrTransponders);

    {
      TYPE_Service_TMSS *p;
      FileHeader.TVServicesOffset = ftell(fExportFile);
      p = (TYPE_Service_TMSS*)(FIS_vFlashBlockTVServices());
      if(p)
      {
        int Muell;
        TAP_Channel_GetTotalNum(&FileHeader.NrTVServices, &Muell);
        ret = fwrite(&FileHeader.NrTVServices, sizeof(FileHeader.NrTVServices), 1, fExportFile) && ret;
        FileHeader.TVServicesOffset = ftell(fExportFile);
        if (FileHeader.NrTVServices > 0)
          ret = fwrite(p, SIZE_Service_TMSx, FileHeader.NrTVServices, fExportFile) && ret;
      }
    }
    TAP_PrintNet((ret) ? "%d TVServices exportiert.\n" : "TVServices Fehler!\n", FileHeader.NrTVServices);

    {
      TYPE_Service_TMSS *p;
      FileHeader.RadioServicesOffset = ftell(fExportFile);
      p = (TYPE_Service_TMSS*)(FIS_vFlashBlockRadioServices());
      if(p)
      {
        int Muell;
        TAP_Channel_GetTotalNum(&Muell, &FileHeader.NrRadioServices);
        ret = fwrite(&FileHeader.NrRadioServices, sizeof(FileHeader.NrRadioServices), 1, fExportFile) && ret;
        FileHeader.RadioServicesOffset = ftell(fExportFile);
        if (FileHeader.NrRadioServices > 0)
          ret = fwrite(p, SIZE_Service_TMSx, FileHeader.NrRadioServices, fExportFile) && ret;
      }
    }
    TAP_PrintNet((ret) ? "%d RadioServices exportiert.\n" : "RadioServices Fehler!\n", FileHeader.NrRadioServices);

    {
      tFavorites FavGroup;
      FileHeader.NrFavGroups = NrFavGroups;
      FileHeader.NrSvcsPerFavGroup = NrFavsPerGroup;
      ret = fwrite(&FileHeader.NrFavGroups, sizeof(FileHeader.NrFavGroups), 1, fExportFile) && ret;
      FileHeader.FavoritesOffset = ftell(fExportFile);
      for (i = 0; i < FileHeader.NrFavGroups; i++)
      {
        memset(&FavGroup, 0, sizeof(tFavorites));
        FlashFavoritesGetInfo(i, &FavGroup);
        ret = fwrite(&FavGroup, sizeof(tFavorites), 1, fExportFile) && ret;
      }
    }
    TAP_PrintNet((ret) ? "%d Favourite-Groups exportiert.\n" : "Favourites Fehler!\n", FileHeader.NrFavGroups);

    {
      char *p1, *p2;
      p1 = (char*)(FIS_vFlashBlockServiceName());
      p2 = (char*)(FIS_vFlashBlockProviderInfo());
      int NrProviderNames = NRPROVIDERNAMES;

      ret = fwrite(&NrProviderNames, sizeof(NrProviderNames), 1, fExportFile) && ret;
      FileHeader.ProviderNamesOffset = ftell(fExportFile);
      if(p2)
      {
        FileHeader.ProviderNamesLength = PROVIDERNAMELENGTH * NRPROVIDERNAMES;  // ***  5380 ?
        ret = fwrite(p2, 1, FileHeader.ProviderNamesLength, fExportFile) && ret;
      }
      TAP_PrintNet((ret) ? "%d ProviderNames exportiert.\n" : "ProviderNames Fehler!\n", NRPROVIDERNAMES);

      int NrServices = FileHeader.NrTVServices + FileHeader.NrRadioServices;
      ret = fwrite(&NrServices, sizeof(FileHeader.NrTVServices), 1, fExportFile) && ret;
      FileHeader.ServiceNamesOffset = ftell(fExportFile);
      if(p1)
      {
//        FileHeader.ServiceNamesLength = 40004;   // 40000 / 39996 ***  ?
//        if(p2)
//          FileHeader.ServiceNamesLength = p2 - p1;
        FileHeader.ServiceNamesLength = GetLengthOfServiceNames();
        ret = fwrite(p1, 1, FileHeader.ServiceNamesLength, fExportFile) && ret;
      }
      TAP_PrintNet((ret) ? "ServiceNames exportiert.\n" : "ServiceNames Fehler!\n");
    }

    FileHeader.FileSize = ftell(fExportFile);
    fclose(fExportFile);

    fExportFile = fopen(TAPFSROOT "/" EXPORTFILENAME ".dat", "r+b");
    if(fExportFile)
    {
      ret = fwrite(&FileHeader, sizeof(tExportHeader), 1, fExportFile) && ret;
      fclose(fExportFile);
    }
  }
  else
    TAP_PrintNet("Datei nicht gefunden\n");

  if (!ret)
    if(TAP_Hdd_Exist(EXPORTFILENAME ".dat")) TAP_Hdd_Delete(EXPORTFILENAME ".dat");
  TAP_PrintNet((ret) ? "Export erfolgreich\n" : "Export fehlgeschlagen\n");
  TRACEEXIT();
  return ret;
}


bool ImportSettings()
{
  tExportHeader         FileHeader;
  FILE                 *fImportFile = NULL;
  unsigned long         fs;
  int                   i, j;
  bool                  ret = FALSE;
  char                 *Buffer = NULL;

  TRACEENTER();
  TAP_PrintNet ("Starte Import...\n");

  fImportFile = fopen(TAPFSROOT "/" EXPORTFILENAME ".dat", "rb");
  if(fImportFile)
  {
    // Dateigröße bestimmen um Puffer zu allozieren
    fseek(fImportFile, 0, SEEK_END);
    fs = ftell(fImportFile);
    rewind(fImportFile);

    // Header prüfen
    if (  (fread(&FileHeader, sizeof(tExportHeader), 1, fImportFile))
       && (strncmp(FileHeader.Magic, "TFchan", 6) == 0)
       && (FileHeader.FileVersion == 1)
       && (FileHeader.FileSize == fs)
       && (FileHeader.SystemType == GetSystemType())
       && ((FileHeader.SystemType == ST_TMSS) || (FileHeader.SystemType == ST_TMSC) || (FileHeader.SystemType == ST_TMST)))
    {
      Buffer = (char*) TAP_MemAlloc(fs * sizeof(char));
      if (Buffer)
      {
        rewind(fImportFile);
        if (fread(Buffer, 1, fs, fImportFile) == fs)
        {
          ret = TRUE;

          // Now write the data blocks from the file to the RAM
          {
            TYPE_SatInfo_TMSS *p;

            p = (TYPE_SatInfo_TMSS*)FIS_vFlashBlockSatInfo();
            if (ret && p)
            {
              memcpy(p, Buffer + FileHeader.SatellitesOffset, FileHeader.NrSatellites * SIZE_SatInfo_TMSx);
//              *NrSatellitesTest = FileHeader.NrSatellites;
            }
            else
              ret = FALSE;
          }
          TAP_PrintNet((ret) ? "%d Satellites importiert.\n" : "Satellites Fehler!\n", FileHeader.NrSatellites);

          {
            TYPE_TpInfo_TMSS *p;
            dword            *NrTransponders;
          
            p = (TYPE_TpInfo_TMSS*)(FIS_vFlashBlockTransponderInfo());
            NrTransponders = (dword*)(p) - 1;
            if (ret && p)
            {
              TAP_PrintNet("NrTransponders = %lu \n", *NrTransponders);
              memcpy(p, Buffer + FileHeader.TranspondersOffset, FileHeader.NrTransponders * SIZE_TpInfo_TMSx);
              *NrTransponders = FileHeader.NrTransponders;
            }
            else
              ret = FALSE;
          }
          TAP_PrintNet((ret) ? "%d Transponders importiert.\n" : "Transponders Fehler!\n", FileHeader.NrTransponders);

          for (j = 0; j <= 1; j++)
          {
            char*                 (*Appl_AddSvcName)(char const*);
            word                  (*Appl_SetProviderName)(char const*);
            TYPE_Service_TMSx      *p;
            word                   *nSvc;

            Appl_AddSvcName       = (void*)FIS_fwAppl_AddSvcName();
            Appl_SetProviderName  = (void*)FIS_fwAppl_SetProviderName();

            char* SvcNameBuf = Buffer + FileHeader.ServiceNamesOffset;
            char* PrvNameBuf = Buffer + FileHeader.ProviderNamesOffset;

            p    = (TYPE_Service_TMSx*) ((j==0) ? FIS_vFlashBlockTVServices() : FIS_vFlashBlockRadioServices());
            nSvc =              (word*) ((j==0) ? FIS_vnTvSvc()               : FIS_vnRadioSvc());
            i    = 0;

            if (ret && p && nSvc)
            {
//              memcpy(p, Buffer + FileHeader.TVServicesOffset, FileHeader.NrTVServices * SIZE_Service_TMSx);
//              *nSvc = FileHeader.NrTVServices;
//              TAP_PrintNet("NrTVServices = %lu \n", *nSvc);

              TYPE_Service_TMSx* pServices;
              pServices = (TYPE_Service_TMSx*) (Buffer + ((j==0) ? FileHeader.TVServicesOffset : FileHeader.RadioServicesOffset));
              for (i = 0; i < ((j==0) ? FileHeader.NrTVServices : FileHeader.NrRadioServices); i++)
              {
//                *nSvc = (word)(i+1);
                if (Appl_AddSvcName)
                {
                  if (pServices[i].NameOffset < (dword)FileHeader.ServiceNamesLength)
                  {
TAP_PrintNet("%s\n", &SvcNameBuf[pServices[i].NameOffset]);
                    pServices[i].NameOffset = (dword)Appl_AddSvcName(&SvcNameBuf[pServices[i].NameOffset]);
                  }
                  else
                    pServices[i].NameOffset = (dword)Appl_AddSvcName("***Dummy***");
                }
                else
                  ret = FALSE;
                if (Appl_SetProviderName)
                {
                  if (pServices[i].ProviderIdx * PROVIDERNAMELENGTH < FileHeader.ProviderNamesLength)
                    pServices[i].ProviderIdx = Appl_SetProviderName(&PrvNameBuf[pServices[i].ProviderIdx * PROVIDERNAMELENGTH]);
                  else
                    pServices[i].ProviderIdx = Appl_SetProviderName("***Dummy***");
                }
                else
                  ret = FALSE;
                pServices[i].NameLock = 0;
//                memcpy(&p[i], &pServices[i], SIZE_Service_TMSx);
//                *nSvc = (word)(i+1);
              }
              memcpy(p, pServices, i * SIZE_Service_TMSx);
              *nSvc = (word)(i);
              TAP_PrintNet("NrServices = %lu \n", *nSvc);
            }
            else
              ret = FALSE;

            if (j==0)
              TAP_PrintNet((ret) ? "%d TVServices importiert.\n" : "TVServices Fehler\n", i)
            else
              TAP_PrintNet((ret) ? "%d RadioServices importiert.\n" : "RadioServices Fehler\n", i);
          }

          {
            tFavorites           *FavGroups;
            char                 *p;

            p = (char*) FIS_vFlashBlockFavoriteGroup();
            FavGroups = (tFavorites*) (Buffer + FileHeader.FavoritesOffset);

            for (i = 0; i < FileHeader.NrFavGroups; i++)
            {
              if (ret)
              {
                if (FavGroups[i].GroupName[0])
                {
                  // bescheuerter Workaround für zu strenge Prüfung in FlashFavoritesSetInfo()
                  (p + i * SIZE_Favorites)[0] = '*';
                  TAP_PrintNet("FavGroup %d: Name = '%s', Entries = %d\n", i, FavGroups[i].GroupName, FavGroups[i].NrEntries);
                  ret = ret && FlashFavoritesSetInfo(i, &FavGroups[i]);
                }
              }
            }
          }
          TAP_PrintNet((ret) ? "%d Favourite-Groups importiert.\n" : "Favourites Fehler!\n", i);
        }

        TAP_MemFree(Buffer);
      }
      else
        TAP_PrintNet ("Nicht genung Speicher!\n");
    }
    else
      TAP_PrintNet ("Header passt nicht!\n");
    fclose(fImportFile);
  }
  else
    TAP_PrintNet ("Datei nicht gefunden!\n");
//  if(ret)
//    FlashProgram();
  TAP_PrintNet((ret) ? "Import erfolgreich\n" : "Import fehlgeschlagen\n");
  TRACEEXIT();
  return ret;
}


// ----------------------------------------------------------------------------
//                           MessageBox-Funktionen
// ----------------------------------------------------------------------------
// Die Funktionen zeigt einen Informationsdialog (OK) an, und wartet auf die Bestätigung des Benutzers.
void ShowErrorMessage(char *MessageStr, char *TitleStr)
{
  dword OldSysState, OldSysSubState;

  TRACEENTER();
  HDD_TAP_PushDir();
  TAP_GetState(&OldSysState, &OldSysSubState);

  OSDMenuMessageBoxInitialize((TitleStr) ? TitleStr : PROGRAM_NAME, MessageStr);
  OSDMenuMessageBoxDoNotEnterNormalMode(TRUE);
  OSDMenuMessageBoxButtonAdd("OK");
  OSDMenuMessageBoxShow();
  CSShowMessageBox = TRUE;
  while (CSShowMessageBox)
  {
    TAP_SystemProc();
    TAP_Sleep(1);
  }

  TAP_Osd_Sync();
  if(OldSysSubState != 0) TAP_EnterNormalNoInfo();

  HDD_TAP_PopDir();
  TRACEEXIT();
}


int TAP_Main(void)
{
  #if STACKTRACE == TRUE
    CallTraceInit();
    CallTraceEnable(TRUE);
    TRACEENTER();
  #endif


/*
  CURL *test = NULL;
  FILE *out = NULL, *in = NULL;
  out = fopen("/mnt/hd/MediaFiles/Curl.mp4", "wb");
  in = fopen("/tmp/test.txt", "r+b");

  test = curl_easy_init();
  curl_easy_setopt(test, CURLOPT_URL, "http://r2---sn-xjpm-4g5e.googlevideo.com/videoplayback?source=youtube&upn=zijuk6VfMzk&mv=u&sparams=dur,id,ip,ipbits,itag,mime,mm,ms,mv,pl,ratebypass,source,upn,expire&mime=video%2Fmp4&mt=1422829184&ms=au&ip=148.251.123.45&key=yt5&signature=9B2882C7D512BAA2EF50F426EB8E83D51F792003.B704D4B8F32041DC651559C9797AE93C685878B3&ratebypass=yes&expire=1422851242&ipbits=0&mm=31&dur=2158.828&id=o-AJwD7b_5E9885aBGx5zCc8A8QiKioRsDmW-S2TXZ2EsW&fexp=900718,902404,907263,916600,927622,930676,931995,9405118,9406557,943917,947225,948124,952302,952605,952901,955301,957201,958101,959701&sver=3&pl=26&itag=22&title=REUPLOAD%20%7C%7C%20ZDF%20Heute%20Show%2014%2E11%2E2014%20mit%20Oliver%20Welke%20-%20HD50"); // "http://mvideos.daserste.de/videoportal/Film/c_500000/502515/format599229.mp4");
  curl_easy_setopt(test, CURLOPT_REFERER, "http://www.youtube.com");
//  curl_easy_setopt(test, CURLOPT_COOKIEFILE, "/tmp/test");
  curl_easy_setopt(test, CURLOPT_FILE, out);
  TAP_PrintNet("curl return: %d\n", curl_easy_perform(test));
  curl_easy_cleanup(test);

  fclose(in);
  fclose(out);
  return 0;
*/
    
  if (InitSystemType())
  {
    TAP_Hdd_ChangeDir("/");
    if(TAP_Hdd_Exist(EXPORTFILENAME ".txt"))
    {
//      DeleteTimers();
//      DeleteAllSettings();
      ImportSettings_Text();
      ShowErrorMessage("Einstellungen (Text) importiert.", NULL);
    }
/*    else if(TAP_Hdd_Exist(EXPORTFILENAME ".dat"))
    {
      DeleteTimers();
      DeleteAllSettings();
//      DeleteFavourites();
//      DeleteServices();
//      DeleteTransponder(1);
//      ImportTransponder();
      ImportSettings();
//      Appl_ImportChData("Settings.std");
      ShowErrorMessage("Einstellungen (binär) importiert.", NULL);
    }
*/    else
    {
      ExportSettings();
      ExportSettings_Text();
      Appl_ExportChData("Settings.std");
      ShowErrorMessage("Einstellungen exportiert.", NULL);
    }
  }
  return 0;
}

dword TAP_EventHandler(word event, dword param1, dword param2)
{
  TRACEENTER();

  // Behandlung offener MessageBoxen (rekursiver Aufruf, auch bei DoNotReenter)
  if(CSShowMessageBox)
  {
    if(OSDMenuMessageBoxIsVisible())
      OSDMenuEvent(&event, &param1, &param2);
    if(!OSDMenuMessageBoxIsVisible())
      CSShowMessageBox = FALSE;
    param1 = 0;
  }

  TRACEEXIT();
  return param1;
}
