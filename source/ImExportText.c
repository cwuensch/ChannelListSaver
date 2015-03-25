#define _FILE_OFFSET_BITS  64
#define __USE_LARGEFILE64  1
#ifdef _MSC_VER
  #define __const const
#endif

#define _GNU_SOURCE
//#define  STACKTRACE     TRUE
#include                <string.h>
#include                <stdlib.h>
#include                <stdio.h>
#include                <tap.h>
#include                <libFireBird.h>
#include                "../../../../../Topfield/FireBirdLib/flash/FBLib_flash.h"
#include                "FlashSatTablesSetInfo.h"
#include                "ChannelListSaver.h"
#include                "ImExportText.h"

int CurLine;

char* ByteArrToStr(char *outStr, byte inArr[], int length)
{
  int i;
  TRACEENTER();

  if (outStr)
    outStr[0] = '\0';
  if (outStr && inArr)
  {
    for (i = 0; i < length; i++)
      sprintf(&outStr[3*i], "%2.2X ", inArr[i]);
    if (outStr[0])
      outStr[strlen(outStr)-1] = '\0';
  }
  TRACEEXIT();
  return outStr;
}
bool StrToByteArr(byte *outArr, char *inStr, int length)
{
  char *p;
  int   i;
  TRACEENTER();

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
      {
        TRACEEXIT();
        return FALSE;
      }
    }
    TRACEEXIT();
    return TRUE;
  }
  else
  {
    TRACEEXIT();
    return FALSE;
  }
}

char BoolToChar(bool inValue)
{
  return (inValue ? 'y' : 'n');
}
bool CharToBool(char inValue)
{
  return ((inValue == 'y') || (inValue == 'Y') || (inValue == '1'));
}

char* FECtoStr(byte inFEC)
{
  if (CurSystemType == ST_TMSS)
    switch (inFEC)
    {
      case FEC_AUTO:                     return "auto";
      case FEC_1_2:                      return "1/2";
      case FEC_2_3:                      return "2/3";
      case FEC_3_4:                      return "3/4";
      case FEC_5_6:                      return "5/6";
      case FEC_7_8:                      return "7/8";
      case FEC_8_9:                      return "8/9";
      case FEC_3_5:                      return "3/5";
      case FEC_4_5:                      return "4/5";
      case FEC_9_10:                     return "9/10";
      case FEC_RESERVED:                 return "reserved";
      case FEC_NO_CONV:                  return "none";
    }
  else
    if(inFEC == 0) return "-";

  static char outStr[10];
  sprintf(outStr, "%#3x", inFEC);
  return outStr;
}
byte StrToFEC(char *inStr)
{
  if (CurSystemType == ST_TMSS)
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
  }
  else
    if (strcmp(inStr, "-")        == 0)  return 0x0;

  long ret = strtol(inStr, NULL, 0);
  if (ret <= 0xF)
  {
    if (ret == 0)
      WriteLogCSf(PROGRAM_NAME, "  Warning: Suspect FEC-String: '%s' (line %d)", inStr, CurLine);
    return ret;
  }
  else
    WriteLogCSf(PROGRAM_NAME, "  Error: Invalid FEC-String: '%s' (line %d)", inStr, CurLine);
  return FEC_NO_CONV;
}

char* ModulationToStr(byte inMod)
{
  if (CurSystemType == ST_TMSS)
    switch (inMod)
    {
      case MODULATION_AUTO:              return "auto";
      case MODULATION_QPSK:              return "QPSK";
      case MODULATION_8PSK:              return "8PSK";
      case MODULATION_16QAM:             return "16QAM";
    }
  else if (CurSystemType == ST_TMSC)
    switch (inMod)
    {
      case 0x0:                          return "16QAM";
      case 0x1:                          return "32QAM";
      case 0x2:                          return "64QAM";
      case 0x3:                          return "128QAM";
      case 0x4:                          return "256QAM";
    }
  else
    if(inMod == 0) return "-";

  static char outStr[10];
  sprintf(outStr, "%#4x", inMod);
  return outStr;
}
byte StrToModulation(char *inStr)
{
  if (CurSystemType == ST_TMSS)
  {
    if (strcmp(inStr, "auto")   == 0)    return MODULATION_AUTO;
    if (strcmp(inStr, "QPSK")   == 0)    return MODULATION_QPSK;
    if (strcmp(inStr, "8PSK")   == 0)    return MODULATION_8PSK;
    if (strcmp(inStr, "16QAM")  == 0)    return MODULATION_16QAM;
  }
  else if (CurSystemType == ST_TMSC)
  {
    if (strcmp(inStr, "16QAM")  == 0)    return 0x0;
    if (strcmp(inStr, "32QAM")  == 0)    return 0x1;
    if (strcmp(inStr, "64QAM")  == 0)    return 0x2;
    if (strcmp(inStr, "128QAM") == 0)    return 0x3;
    if (strcmp(inStr, "256QAM") == 0)    return 0x4;
  }
  else
    if (strcmp(inStr, "-")      == 0)    return 0x0;

  long ret = strtol(inStr, NULL, 0);
  if (ret <= ((CurSystemType == ST_TMSS) ? 3 : 0xFF))
  {
    if (ret == 0)
      WriteLogCSf(PROGRAM_NAME, "  Warning: Suspect Modulation-String: '%s' (line %d)", inStr, CurLine);
    return ret;
  }
  else
    WriteLogCSf(PROGRAM_NAME, "  Error: Invalid Modulation-String: '%s' (line %d)", inStr, CurLine);
  return 0;
}

char* VideoTypeToStr(byte inVideo)
{
  static char outStr[10];
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
      WriteLogCSf(PROGRAM_NAME, "  Warning: Suspect VideoType-String: '%s' (line %d)", inStr, CurLine);
    return ret;
  }
  else
    WriteLogCSf(PROGRAM_NAME, "  Error: Invalid VideoType-String: '%s' (line %d)", inStr, CurLine);
  return STREAM_UNKNOWN;
}

char* AudioTypeToStr(word inAudio)
{
  static char outStr[10];
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
      WriteLogCSf(PROGRAM_NAME, "  Warning: Suspect AudioType-String: '%s' (line %d)", inStr, CurLine);
    return ret;
  }
  else
    WriteLogCSf(PROGRAM_NAME, "  Error: Invalid AudioType-String: '%s' (line %d)", inStr, CurLine);
  return STREAM_UNKNOWN;
}

bool ExportSettings_Text(char *FileName, char *AbsDirectory)
{
  tExportHeader         FileHeader;
  FILE                 *fExportFile = NULL;
  char                  AbsFileName[FBLIB_DIR_SIZE];
  int                   FileSize = 0, FileSizePos = 0, z = 0;
  int                   i, j;
  bool                  ret = FALSE;

  TRACEENTER();
  WriteLogCS(PROGRAM_NAME, "[Action] Exporting settings (Text)...");
  WriteLogCS(PROGRAM_NAME, "----------------------------------------");

  TAP_SPrint(AbsFileName, sizeof(AbsFileName), "%s/%s", AbsDirectory, FileName);
  fExportFile = fopen(AbsFileName, "w");
  if(fExportFile)
  {
    ret = TRUE;

    // Write the file header
    ret = (fprintf(fExportFile, "[ChannelListSaver]" CRLF)               > 0) && ret;
    ret = (fprintf(fExportFile, "FileVersion=%d" CRLF,  1)               > 0) && ret;
    FileSizePos = ftell(fExportFile);
    ret = (fprintf(fExportFile, "FileSize=%010d" CRLF,  0)               > 0) && ret;
    ret = (fprintf(fExportFile, "NrOfLines=%010d" CRLF,  0)              > 0) && ret;
    ret = (fprintf(fExportFile, "SystemType=%d" CRLF,   GetSystemType()) > 0) && ret;
    ret = (fprintf(fExportFile, "UTF8System=%d" CRLF,   isUTFToppy())    > 0) && ret;

    memset(&FileHeader, 0, sizeof(FileHeader));
    FileHeader.NrSatellites = FlashSatTablesGetTotal();
    for(i = 0; i < FileHeader.NrSatellites; i++)
      FileHeader.NrTransponders += FlashTransponderTablesGetTotal(i);
    TAP_Channel_GetTotalNum(&FileHeader.NrTVServices, &FileHeader.NrRadioServices);
    fprintf(fExportFile, "NrSatellites=%d" CRLF, FileHeader.NrSatellites);
    fprintf(fExportFile, "NrTransponders=%d" CRLF, FileHeader.NrTransponders);
    fprintf(fExportFile, "NrTVServices=%d" CRLF, FileHeader.NrTVServices);
    fprintf(fExportFile, "NrRadioServices=%d" CRLF, FileHeader.NrRadioServices);
    fprintf(fExportFile, "NrFavGroups=%d" CRLF, FlashFavoritesGetTotal());
    fprintf(fExportFile, "NrFavsPerGroup=%d" CRLF, NrFavsPerGroup);
    fprintf(fExportFile, CRLF);
    z = 13;

    //[Satellites]
    //#  ;                 ;       ;      ;    Supply;;      DiSEqC10;;    DiSEqC11;;    DiSeqC12;;    DiSEqC12Flags;;      Universal;;    Switch22;;     LowBand;;        HBFrq;;        Loop;;       Unused1;;     Unused2;;     Unused3;;     Unused4;;                   Unused5;;                ;                                                                   ;
    //#Nr; SatName         ; SatPos; NrTps;   LNB1; LNB2;   LNB1; LNB2;   LNB1; LNB2;   LNB1; LNB2;      LNB1; LNB2;       LNB1;  LNB2;   LNB1;  LNB2;   LNB1;  LNB2;   LNB1;  LNB2;   LNB1; LNB2;   LNB1; LNB2;   LNB1; LNB2;   LNB1; LNB2;   LNB1; LNB2;             LNB1;           LNB2;   Unused1;  Unknown1                                                         ;  Unused2
    //  1; Astra           ;    192;   136;     y;    y;      0;    0;      0;    0;      4;    4;   FE 7F 7F; FE 7F 7F;      y;     y;      n;     n;   9750;  9750;  10600; 10600;     y;    y;       0;    0;      0;    0;      0;    0;      0;    0;   FE 40 43 FF FF; FE 40 43 FF FF;    0xffff;  FF FF FF FF 01 01 00 01 00 00 00 00 B2 2E 02 04 6E 00 78 00 6E 00;  FF FF FF FF FF FF FF FF
    {
      tFlashSatTable         CurSat;
      char                   StringBuf1[100], StringBuf2[100];

      ret = (fprintf(fExportFile, "[Satellites]" CRLF)                   > 0) && ret;
//      ret = (fprintf(fExportFile, "#Nr; SatName;          SatPos; NrTps; LNB1Supply; LNB1DiSEqC10; LNB1DiSEqC11; LNB1DiSeqC12; LNB1DiSEqC12Flags; LNB1Universal; LNB1Switch22; LNB1LowBand; LNB1HBFrq; LNB1Loop; LNB1Unused1; LNB1Unused2; LNB1Unused3; LNB1Unused4; LNB1Unused5; LNB2Supply; LNB2DiSEqC10; LNB2DiSEqC11; LNB2DiSeqC12; LNB2DiSEqC12Flags; LNB2Universal; LNB2Switch22; LNB2LowBand; LNB2HBFrq; LNB2Loop; LNB2Unused1; LNB2Unused2; LNB2Unused3; LNB2Unused4; LNB2Unused5; Unused1; Unknown1; Unused2" CRLF) > 0) && ret;
      ret = (fprintf(fExportFile, "#  ;                 ;       ;      ;    Supply;;      DiSEqC10;;    DiSEqC11;;    DiSeqC12;;    DiSEqC12Flags;;      Universal;;    Switch22;;     LowBand;;        HBFrq;;    LoopThrough;;   Unused1;;     Unused2;;     Unused3;;     Unused4;;                    Unused5;;                ;                                                                   ;" CRLF) > 0) && ret;
      ret = (fprintf(fExportFile, "#Nr; SatName         ; SatPos; NrTps;   LNB1; LNB2;   LNB1; LNB2;   LNB1; LNB2;   LNB1; LNB2;      LNB1; LNB2;       LNB1;  LNB2;   LNB1;  LNB2;   LNB1;  LNB2;   LNB1;  LNB2;   LNB1; LNB2;   LNB1; LNB2;   LNB1; LNB2;   LNB1; LNB2;   LNB1; LNB2;             LNB1;           LNB2;   Unused1;  Unknown1                                                         ;  Unused2" CRLF) > 0) && ret;
      z += 3;

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
          ret = (fprintf(fExportFile, "%3c;  %3c;    %3hhu;  %3hhu;    %3hhu;  %3hhu;    %3hhu;  %3hhu;   %s; %s;  %5c; %5c;  %5c; %5c;  %5hu; %5hu;  %5hu; %5hu;   %3c;  %3c;    ",
                                       BoolToChar(CurSat.LNB[0].LNBSupply), BoolToChar(CurSat.LNB[1].LNBSupply),  CurSat.LNB[0].DiSEqC10, CurSat.LNB[1].DiSEqC10,  CurSat.LNB[0].DiSEqC11, CurSat.LNB[1].DiSEqC11,  CurSat.LNB[0].DiSEqC12, CurSat.LNB[1].DiSEqC12,  StringBuf1, StringBuf2,  BoolToChar(CurSat.LNB[0].UniversalLNB), BoolToChar(CurSat.LNB[1].UniversalLNB),  BoolToChar(CurSat.LNB[0].Switch22), BoolToChar(CurSat.LNB[1].Switch22),  CurSat.LNB[0].LowBand, CurSat.LNB[1].LowBand,  CurSat.LNB[0].HBFrq, CurSat.LNB[1].HBFrq,  BoolToChar(CurSat.LNB[0].LoopThrough), BoolToChar(CurSat.LNB[1].LoopThrough)) > 0) && ret;

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
          z++;
        }
        else
          WriteLogCSf(PROGRAM_NAME, "  Failed to decode sat %d!", i);
      }
      fprintf(fExportFile, CRLF);
      z++;
    }
    WriteLogCSf(PROGRAM_NAME, (ret) ? "%5d Satellites exported." : "%5d Satellites error!", i);

    //[Transponders]
    //# Nr; SatIdx;  Frequency; SymbRate; Channel;  BW;  TSID; ONWID;  NWID; Pilot;      FEC; Modulation; System; Pol; LPHP; ClockSync; Unknown1; Unknown2; Unknown3; Unknown4
    //   0;      1;      10729;    22000;       0;   0;  1050;     1;     0;     n;      2/3;       8PSK;  DVBS2;   V;    0;         n;        0;        0;        0;        0
    {
      tFlashTransponderTable CurTransponder;
      int                    NrTransponders;

      ret = (fprintf(fExportFile, "[Transponders]" CRLF)             > 0) && ret;
      z++;
      FileHeader.NrTransponders = 0;
      for(i = 0; i < FileHeader.NrSatellites; i++)
      {
        ret = (fprintf(fExportFile, "# Nr; SatIdx;  Frequency; SymbRate; Channel;  BW;  TSID; ONWID;  NWID; Pilot;      FEC; Modulation; System; Pol; LPHP; ClockSync; Unknown1; Unknown2; Unknown3; Unknown4" CRLF) > 0) && ret;
        z++;
        NrTransponders = FlashTransponderTablesGetTotal(i);
        for(j = 0; j < NrTransponders; j++)
        {
          if (FlashTransponderTablesGetInfo(i, j, &CurTransponder))
          {
            ret = (fprintf(fExportFile, "%4d;    %3hhu; %10lu;    %5hu;     %3hhu; %3hhu; %5hu; %5hu; %5hu;     "  // Nr; SatIdx; Frequency; SymbRate; Channel; BW; TSID; ONWID; NWID
                                        "%c; %8s;   %8s;  "                                                        // Pilot; FEC; Modulation
                                        "%5s;   %c;  %3hhu;         %c;   "                                        // System; Pol; LPHP; ClockSync
                                        "%#6x;   %#6x;     %#4x;   %#6x" CRLF,                                     // Unknown1; Unknown2; Unknown3; Unknown4
                                         j, CurTransponder.SatIndex, CurTransponder.Frequency, CurTransponder.SymbolRate, CurTransponder.ChannelNr, CurTransponder.Bandwidth, CurTransponder.TSID, CurTransponder.OriginalNetworkID, CurTransponder.NetworkID,
                                         BoolToChar(CurTransponder.Pilot), FECtoStr(CurTransponder.FEC), ModulationToStr(CurTransponder.Modulation),
                                         (CurTransponder.ModSystem ? "DVBS2" : (CurSystemType==ST_TMSS ? "DVBS" : "-")), (CurTransponder.Polarisation ? 'H' : (CurSystemType==ST_TMSS ? 'V' : '-')), CurTransponder.LPHP, BoolToChar(CurTransponder.ClockSync),
                                         CurTransponder.unused1, CurTransponder.unused2, CurTransponder.unused3, CurTransponder.unused4) > 0) && ret;
            z++;
          }
          else
            WriteLogCSf(PROGRAM_NAME, "  Failed to decode transponder %d!", j);
        }
        if (ret) FileHeader.NrTransponders += NrTransponders;
        fprintf(fExportFile, CRLF);
        z++;
      }
    }
    WriteLogCSf(PROGRAM_NAME, (ret) ? "%5d Transponders exported." : "%5d Transponders error!", FileHeader.NrTransponders);

    //[Services]
    //# Nr; ServiceName            ; SatIdx; TrpIdx; Tuner; VideoType; AudioType;  SvcID; PMTPID; PCRPID; VidPID; AudPID;   LCN; FDel; FCAS; FLock; FSkip; NameLck; Flags2; Unknown2;           ProviderName
    //   0; Das Erste              ;      1;     44;     3;     MPEG2;     MPEG1;  28106;    100;    101;    101;  32870;     0;    n;    n;     n;     n;       n;    0x1; 00 00 00 00 DC DD;  ARD
    for (j = 0; j <= 1; j++)
    {
      tFlashService          CurService;
      char                  *CurSvcType, *p;
      char                   StringBuf[20];

      CurSvcType = (j == 0) ? "TV" : "Radio";
      ret = (fprintf(fExportFile, "[%sServices]" CRLF, CurSvcType)       > 0) && ret;
      ret = (fprintf(fExportFile, "# Nr; ServiceName             ; SatIdx; TrpIdx; Tuner; VideoType; AudioType;  SvcID; PMTPID; PCRPID; VidPID; AudPID;   LCN; FDel; FCAS; FLock; FSkip; NameLck; Flags2;          Unknown2;  ProviderName" CRLF) > 0) && ret;
      z += 2;

      for(i = 0; i < ((j == 0) ? FileHeader.NrTVServices : FileHeader.NrRadioServices); i++)
      {
        if (FlashServiceGetInfo(((j == 0) ? SVC_TYPE_Tv : SVC_TYPE_Radio), i, &CurService))
        {
          while ((p = strchr(CurService.ServiceName, ';')))
            p[0] = '_';
          while ((p = strchr(CurService.ProviderName, ';')))
            p[0] = '_';

          ByteArrToStr(StringBuf, CurService.unknown2, sizeof(CurService.unknown2));
          ret = (fprintf(fExportFile, "%4d; %-24s;    %3hhu;  %5hu;   %3hhu;  "                                    // Nr; ServiceName; SatIndex; TransponderIndex; Tuner;
                                      "%8s;  %8s;  %5hu;  %5hu;  %5hu;  %5hu;  %5hu; "                             // VideoStreamType; AudioStreamType; ServiceID; PMTPID; PCRPID; VideoPID; AudioPID;
                                      "%5hu;    %c;    %c;     %c;     %c;       %c; %#6x; %s;  %s" CRLF,          // LCN; FlagDelete; FlagCAS; FlagLock; FlagSkip; NameLock; Flags2; Unknown2; ProviderName
                                       i, CurService.ServiceName, CurService.SatIndex, CurService.TransponderIndex, CurService.Tuner,
                                       VideoTypeToStr(CurService.VideoStreamType), AudioTypeToStr(CurService.AudioStreamType), CurService.ServiceID, CurService.PMTPID, CurService.PCRPID, CurService.VideoPID, CurService.AudioPID,
                                       CurService.LCN, BoolToChar(CurService.FlagDelete), BoolToChar(CurService.FlagCAS), BoolToChar(CurService.FlagLock), BoolToChar(CurService.FlagSkip), BoolToChar(CurService.NameLock), CurService.Flags2, StringBuf, CurService.ProviderName) > 0) && ret;
          z++;
        }
        else
          WriteLogCSf(PROGRAM_NAME, "  Failed to decode %s service %d!", CurSvcType, i);
      }
      fprintf(fExportFile, CRLF);
      z++;
      WriteLogCSf(PROGRAM_NAME, (ret) ? "%5d %sServices exported." : "%5d %sServices error!", i, CurSvcType);
    }

    //[Favorites]
    //#Nr; GroupName; T/R; SvcNums
    {
      tFavorites             CurFavGroup;
      char                  *p;

      ret = (fprintf(fExportFile, "[Favorites]" CRLF)                   > 0) && ret;
      ret = (fprintf(fExportFile, "#Nr; GroupName   ; T/R;  SvcNums" CRLF)  > 0) && ret;
      z += 2;

      FileHeader.NrFavGroups = 0;
      for (i = 0; i < NrFavGroups; i++)
      {
        if (FlashFavoritesGetInfo(i, &CurFavGroup))
        {
          while ((p = strchr(CurFavGroup.GroupName, ';')))
            p[0] = '_';
          ret = (fprintf(fExportFile, "%3d; %-12s;   %c;" , i, CurFavGroup.GroupName, ((CurFavGroup.SvcType[0]==0) ? 'T' : 'R')) > 0) && ret;
          for (j = 0; j < CurFavGroup.NrEntries; j++)
            ret = (fprintf(fExportFile, (j==0 ? "  %hu" : ", %hu"), CurFavGroup.SvcNum[j]) > 0) && ret;
//          ret = (fprintf(fExportFile, "; %#01X", CurFavGroup.unused1)   > 0) && ret;
          ret = (fprintf(fExportFile, CRLF)                             > 0) && ret;
          z++;
          if (ret) FileHeader.NrFavGroups++;
        }
//        else
//          WriteLogCSf(PROGRAM_NAME, "  Failed to decode favorite group %d!", i);
      }
    }
    WriteLogCSf(PROGRAM_NAME, (ret) ? "%5d Favorite-Groups exported." : "%5d Favorites error!", FileHeader.NrFavGroups);


    FileSize = ftell(fExportFile);
    fclose(fExportFile);

    fExportFile = fopen(AbsFileName, "r+");
    if(fExportFile)
    {
      fseek(fExportFile, FileSizePos, SEEK_SET);
      ret = (fprintf(fExportFile, "FileSize=%010d" CRLF, FileSize) > 0) && ret;
      ret = (fprintf(fExportFile, "NrOfLines=%010d" CRLF, z) > 0) && ret;      
      fclose(fExportFile);
    }
  }
  else
    WriteLogCS(PROGRAM_NAME, "  File not found!");

  if (ret)
    WriteLogCSf(PROGRAM_NAME, "--> Export '%s' (Text) successful.", FileName);
  else
  {
    if(TAP_Hdd_Exist(FileName)) TAP_Hdd_Delete(FileName);
    WriteLogCSf(PROGRAM_NAME, "--> Error during export '%s'.", FileName);
  }
  TRACEEXIT();
  return ret;
}

bool ImportSettings_Text(char *FileName, char *AbsDirectory, int OverwriteSatellites)  // 0: nie, 1: auto, 2: immer
{
  tExportHeader         FileHeader;
  size_t                BufSize = 0;
  char                 *Buffer = NULL;
  FILE                 *fImportFile = NULL;
  char                  AbsFileName[FBLIB_DIR_SIZE];
  unsigned long         fs;
  tScanMode             CurMode = SM_Start;
  bool                  HeaderCheck[3];
  int                   NrOfLines = 0;
  int                   NrSats=0, NrImpSatellites=0, NrImpTransponders=0, NrImpTVServices=0, NrImpRadioServices=0, NrImpFavGroups=0;
  bool                  ret = FALSE;
  int                   p;
  char                 *c;

  TRACEENTER();
  WriteLogCS(PROGRAM_NAME, "[Action] Importing settings (Text)...");
  WriteLogCS(PROGRAM_NAME, "----------------------------------------");
  CurLine = 0;

//  Buffer = (char*) malloc(BufSize);
//  if (Buffer)
  TAP_SPrint(AbsFileName, sizeof(AbsFileName), "%s/%s", AbsDirectory, FileName);
  fImportFile = fopen(AbsFileName, "r");
  if (fImportFile)
  {
    ret = TRUE;
    memset(&FileHeader, 0, sizeof(FileHeader));

    // Dateigröße bestimmen
    fseek(fImportFile, 0, SEEK_END);
    fs = ftell(fImportFile);
    rewind(fImportFile);

    while (ret && (getline(&Buffer, &BufSize, fImportFile) >= 0))
    {
      CurLine++;
      //Interpret the following characters as remarks: //
      c = strstr(Buffer, "//");
      if(c) *c = '\0';

      // Remove line breaks in the end
      p = strlen(Buffer);
      while (p && (Buffer[p-1] == '\r' || Buffer[p-1] == '\n' || Buffer[p-1] == ';'))
        Buffer[--p] = '\0';

//TAP_PrintNet("%d: %s\n", CurMode, Buffer);

      // Prüfung der ersten Zeile
      if (CurMode == SM_Start)
      {
        if ((strcmp(Buffer, "[ChannelListSaver]") == 0) || (strcmp(Buffer, "ï»¿[ChannelListSaver]") == 0))
        {
          int i = 0;
          for (i = 0; i < 3; i++)
            HeaderCheck[i] = FALSE;
          CurMode = SM_Header;
          continue;
        }
        else
        {
          WriteLogCS(PROGRAM_NAME, "  Error: Invalid file format (head line)!");
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
            WriteLogCS(PROGRAM_NAME, "  Error: Invalid file header!");
            ret = FALSE;
            continue;
          }
          WriteLogCSf(PROGRAM_NAME, "  New Section: '%s'", Buffer);
          if (strcmp(Buffer, "[Satellites]") == 0)
            CurMode = SM_Satellites;
          else if (strcmp(Buffer, "[Transponders]") == 0)
            CurMode = SM_Transponders;
          else if (strcmp(Buffer, "[TVServices]") == 0)
            CurMode = SM_TVServices;
          else if (strcmp(Buffer, "[RadioServices]") == 0)
            CurMode = SM_RadioServices;
          else if (strcmp(Buffer, "[Favorites]") == 0)
            CurMode = SM_Favorites;
          else
          {
            WriteLogCSf(PROGRAM_NAME, "  Warning: Unknown section: %s! (line %d)", Buffer, CurLine);
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
          else if (strcmp(Name, "NrOfLines") == 0)
            NrOfLines = Value;
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

          if ((CurMode==SM_Header) && HeaderCheck[0] && HeaderCheck[1] && HeaderCheck[2])
          {
            CurMode = SM_HeaderChecked;
            ret = ret && DeleteAllSettings(OverwriteSatellites == 2);
            WriteLogCS(PROGRAM_NAME, "Importing settings:");
          }
        }
      }

      // Beliebige Zeile gefunden -> importieren!
      switch (CurMode)
      {
        //[Satellites]
        //#  ;                 ;       ;      ;    Supply;;      DiSEqC10;;    DiSEqC11;;    DiSeqC12;;    DiSEqC12Flags;;      Universal;;    Switch22;;     LowBand;;        HBFrq;;        Loop;;       Unused1;;     Unused2;;     Unused3;;     Unused4;;                   Unused5;;                ;                                                                   ;
        //#Nr; SatName         ; SatPos; NrTps;   LNB1; LNB2;   LNB1; LNB2;   LNB1; LNB2;   LNB1; LNB2;      LNB1; LNB2;       LNB1;  LNB2;   LNB1;  LNB2;   LNB1;  LNB2;   LNB1;  LNB2;   LNB1; LNB2;   LNB1; LNB2;   LNB1; LNB2;   LNB1; LNB2;   LNB1; LNB2;             LNB1;           LNB2;   Unused1;  Unknown1                                                         ;  Unused2
        //  1; Astra           ;    192;   136;     y;    y;      0;    0;      0;    0;      4;    4;   FE 7F 7F; FE 7F 7F;      y;     y;      n;     n;   9750;  9750;  10600; 10600;     y;    y;       0;    0;      0;    0;      0;    0;      0;    0;   FE 40 43 FF FF; FE 40 43 FF FF;    0xffff;  FF FF FF FF 01 01 00 01 00 00 00 00 B2 2E 02 04 6E 00 78 00 6E 00;  FF FF FF FF FF FF FF FF
        case SM_Satellites:
        {
          tFlashSatTable         CurSat;
          char                   StringBuf1[100], StringBuf2[100];
          char                   LNBSupply[2], UniversalLNB[2], Switch22[2], LoopThrough[2];
          byte                   DiSEqC10[2], unused1[2], unused2[2], unused3[2], unused4[2];
          word                   LowBand[2];
          int                    BytesRead = 0;

          memset(&CurSat, 0, sizeof(tFlashSatTable));
          p = 0;

          // Nr; SatName; SatPosition; NrTransponders;
          ret = (sscanf(Buffer, "%*i ; %15[^;\r\n] ; %hu ; %hu ; %n",
                                  CurSat.SatName, &CurSat.SatPosition, &CurSat.NrOfTransponders, &BytesRead) == 3) && ret;
          p += BytesRead;
          RTrim(CurSat.SatName);

          // LNBxSupply; LNBxDiSEqC10; LNBxDiSEqC11; LNBxDiSeqC12; LNBxDiSEqC12Flags; LNBxUniversal; LNBxSwitch22; LNBxLowBand; LNBxHBFrq; LNBxLoop
          ret = (sscanf(&Buffer[p], "%c ; %c ;  %hhu ; %hhu ;  %hhu ; %hhu ;  %hhu ; %hhu ;  %9[^;\r\n] ; %9[^;\r\n] ;  %c ; %c ;  %c ; %c ;  %hu ; %hu ;  %hu ; %hu ;  %c ; %c ; %n",
                                      &LNBSupply[0], &LNBSupply[1],  &DiSEqC10[0], &DiSEqC10[1],  &CurSat.LNB[0].DiSEqC11, &CurSat.LNB[1].DiSEqC11,  &CurSat.LNB[0].DiSEqC12, &CurSat.LNB[1].DiSEqC12,  StringBuf1, StringBuf2,  &UniversalLNB[0], &UniversalLNB[1],  &Switch22[0], &Switch22[1],  &LowBand[0], &LowBand[1],  &CurSat.LNB[0].HBFrq, &CurSat.LNB[1].HBFrq,  &LoopThrough[0], &LoopThrough[1], &BytesRead) == 20) && ret;
          p += BytesRead;
          ret = StrToByteArr(CurSat.LNB[0].DiSEqC12Flags, StringBuf1, sizeof(CurSat.LNB[0].DiSEqC12Flags)) && ret;
          ret = StrToByteArr(CurSat.LNB[1].DiSEqC12Flags, StringBuf2, sizeof(CurSat.LNB[1].DiSEqC12Flags)) && ret;

          // LNBxUnused1; LNBxUnused2; LNBxUnused3; LNBxUnused4; LNBxUnused5;
          ret = (sscanf(&Buffer[p], "%hhi ; %hhi ;  %hhi ; %hhi ;  %hhi ; %hhi ;  %hhi ; %hhi ;  %15[^;\r\n] ; %15[^;\r\n] ; %n",
                                      &unused1[0], &unused1[1],  &unused2[0], &unused2[1],  &unused3[0], &unused3[1],  &unused4[0], &unused4[1],  StringBuf1, StringBuf2, &BytesRead) == 10) && ret;
          p += BytesRead;
          ret = StrToByteArr(CurSat.LNB[0].unused5, StringBuf1, sizeof(CurSat.LNB[0].unused5)) && ret;
          ret = StrToByteArr(CurSat.LNB[1].unused5, StringBuf2, sizeof(CurSat.LNB[1].unused5)) && ret;

          int i;
          for(i = 0; i <= 1; i++)
          {
            CurSat.LNB[i].LNBSupply    = CharToBool(LNBSupply[i]);
            CurSat.LNB[i].UniversalLNB = CharToBool(UniversalLNB[i]);
            CurSat.LNB[i].Switch22     = CharToBool(Switch22[i]);
            CurSat.LNB[i].LoopThrough  = CharToBool(LoopThrough[i]);
            CurSat.LNB[i].LowBand      = LowBand[i];
            CurSat.LNB[i].DiSEqC10     = DiSEqC10[i];
            CurSat.LNB[i].unused1      = unused1[i];
            CurSat.LNB[i].unused2      = unused2[i];
            CurSat.LNB[i].unused3      = unused3[i];
            CurSat.LNB[i].unused4      = unused4[i];
          }

          // Unused1; Unknown1; Unused2
          ret = (sscanf(&Buffer[p], "%hi ; %66[^;\r\n] ; %24[^;\r\n]" CRLF,
                                      &CurSat.unused1, StringBuf1, StringBuf2) == 3) && ret;
          ret = StrToByteArr(CurSat.unknown1, StringBuf1, sizeof(CurSat.unknown1)) && ret;
          ret = StrToByteArr(CurSat.unused2, StringBuf2, sizeof(CurSat.unused2)) && ret;

          CurSat.NrOfTransponders = 0;

          if (OverwriteSatellites != 2)  // auto oder nie
          {
            tFlashSatTable IstSat;
            memset(&IstSat, 0, sizeof(tFlashSatTable));

            if (FlashSatTablesGetInfo(NrSats, &IstSat))
            {
              if ((IstSat.SatPosition != CurSat.SatPosition) || (strcmp(IstSat.SatName, CurSat.SatName) != 0))
              {
                WriteLogCSf(PROGRAM_NAME, "  Warning: Satellite nr. %d does not match! (Import: '%s', Receiver: '%s')", NrSats, CurSat.SatName, IstSat.SatName);
                if (OverwriteSatellites == 1)
                {
                  WriteLogCS(PROGRAM_NAME, "  --> Will overwrite satellites from now...");
                  OverwriteSatellites = 2;
                }
              }
            }
            else
            {
              WriteLogCSf(PROGRAM_NAME, "  Warning: Satellite nr. %d ('%s') not found in receiver!", NrSats, CurSat.SatName);
              if (OverwriteSatellites == 1)
              {
                WriteLogCS(PROGRAM_NAME, "  --> Will overwrite satellites from now...");
                OverwriteSatellites = 2;
              }
              else 
              {
                ret = FALSE;
                break;  // NrSats nicht hochzählen
              }
            }
          }
          if (OverwriteSatellites == 2)  // auto oder nie
          {
            if (ret)
              ret = FlashSatTablesSetInfo(NrSats, &CurSat);

            if (ret)
              NrImpSatellites++;
            else
              WriteLogCSf(PROGRAM_NAME, "  Error in satellite nr. %d! (line %d)", NrSats, CurLine);
          }
          NrSats++;
          break;
        }

        //[Transponders]
        //# Nr; SatIdx;  Frequency; SymbRate; Channel;  BW;  TSID; ONWID;  NWID; Pilot;      FEC; Modulation; System; Pol; LPHP; ClockSync; Unknown1; Unknown2; Unknown3; Unknown4
        //   0;      1;      10729;    22000;       0;   0;  1050;     1;     0;     n;      2/3;       8PSK;  DVBS2;   V;    0;         n;        0;        0;        0;        0
        case SM_Transponders:
        {
          tFlashTransponderTable CurTransponder;
          char                   StringBuf1[10], StringBuf2[10], StringBuf3[10];
          char                   CharPilot, CharPolarisation, CharClockSync;

          memset(&CurTransponder, 0, sizeof(tFlashTransponderTable));
          ret = (sscanf(Buffer, "%*i ; %hhu ; %lu ; %hu ; %hhu ; %hhu ; %hu ; %hi ; %hi ; "      // Nr; SatIdx; Frequency; SymbRate; Channel; BW; TSID; ONWID; NWID
                                "%c ; %8[^;\r\n] ; %8[^;\r\n] ; "                                // Pilot; FEC; Modulation
                                "%6[^;\r\n] ; %c ; %hhu ; %c ; "                                 // System; Pol; LPHP; ClockSync
                                "%hi ; %hi ; %hhi ; %hi",                                        // Unknown1; Unknown2; Unknown3; Unknown4
                                  &CurTransponder.SatIndex, &CurTransponder.Frequency, &CurTransponder.SymbolRate, &CurTransponder.ChannelNr, &CurTransponder.Bandwidth, &CurTransponder.TSID, &CurTransponder.OriginalNetworkID, &CurTransponder.NetworkID,
                                  &CharPilot, StringBuf1, StringBuf2,
                                  StringBuf3, &CharPolarisation, &CurTransponder.LPHP, &CharClockSync,
                                  &CurTransponder.unused1, &CurTransponder.unused2, &CurTransponder.unused3, &CurTransponder.unused4) == 19) && ret;
          CurTransponder.Pilot      = CharToBool(CharPilot);
          CurTransponder.FEC        = StrToFEC(StringBuf1);
          CurTransponder.Modulation = StrToModulation(StringBuf2);
          if (strncmp(StringBuf3, "DVBS2", 5) == 0)
            CurTransponder.ModSystem = 1;
          if (CharPolarisation == 'H')
            CurTransponder.Polarisation = 1;
          CurTransponder.ClockSync = CharToBool(CharClockSync);

          if (ret && (CurTransponder.SatIndex < NrSats))
            ret = (FlashTransponderTablesAdd(CurTransponder.SatIndex, &CurTransponder) != -1);
          else
            ret = FALSE;

          if (ret)
            NrImpTransponders++;
          else
            WriteLogCSf(PROGRAM_NAME, "  Error in transponder nr. %d! (line %d)", NrImpTransponders, CurLine);
          break;
        }

        //[Services]
        //# Nr; ServiceName             ; SatIdx; TrpIdx; Tuner; VideoType; AudioType;  SvcID; PMTPID; PCRPID; VidPID; AudPID;   LCN; FDel; FCAS; FLock; FSkip; NameLck; Flags2; Unknown2;           ProviderName
        //   0; Das Erste               ;      1;     44;     3;     MPEG2;     MPEG1;  28106;    100;    101;    101;  32870;     0;    n;    n;     n;     n;       n;    0x1; 00 00 00 00 DC DD;  ARD
        case SM_TVServices:
        case SM_RadioServices:
        {
          tFlashService          CurService;
          char                   StringBuf1[10], StringBuf2[10], StringBuf3[20];
          char                   CharFlagDel, CharFlagCAS, CharFlagLock, CharFlagSkip, CharNameLock;

          memset(&CurService, 0, sizeof(tFlashService));

          ret = (sscanf(Buffer, "%*i ; %23[^;\r\n] ; %hhu ; %hu ; %hhu ; "                         // Nr; ServiceName; SatIndex; TransponderIndex; Tuner;
                                "%8[^;\r\n] ; %8[^;\r\n] ; %hu ; %hu ; %hu ; %hu ; %hu ; "         // VideoStreamType; AudioStreamType; ServiceID; PMTPID; PCRPID; VideoPID; AudioPID;
                                "%hu ; %c ; %c ; %c ; %c ; %c ; %hi ; %18[^;\r\n] ; %20[^;\r\n]",  // LCN; FlagDelete; FlagCAS; FlagLock; FlagSkip; NameLock; Flags2; Unknown2; ProviderName
                                  CurService.ServiceName, &CurService.SatIndex, &CurService.TransponderIndex, &CurService.Tuner,
                                  StringBuf1, StringBuf2, &CurService.ServiceID, &CurService.PMTPID, &CurService.PCRPID, &CurService.VideoPID, &CurService.AudioPID,
                                  &CurService.LCN, &CharFlagDel, &CharFlagCAS, &CharFlagLock, &CharFlagSkip, &CharNameLock, &CurService.Flags2, StringBuf3, CurService.ProviderName) >= 19) && ret;  // Wenn ProviderName empty -> wird korrekt auf 0xFFFF gesetzt

          RTrim(CurService.ServiceName);
          if (!CurService.ServiceName[0]) strcpy(CurService.ServiceName, "* No Name *");
          RTrim(CurService.ProviderName);
          CurService.VideoStreamType = StrToVideoType(StringBuf1);
          CurService.AudioStreamType = StrToAudioType(StringBuf2);
          CurService.FlagDelete = CharToBool(CharFlagDel);
          CurService.FlagCAS = CharToBool(CharFlagCAS);
          CurService.FlagLock = CharToBool(CharFlagLock);
          CurService.FlagSkip = CharToBool(CharFlagSkip);
          CurService.NameLock = CharToBool(CharNameLock);
          ret = StrToByteArr(CurService.unknown2, StringBuf3, sizeof(CurService.unknown2)) && ret;

          if (FileHeader.UTF8System != isUTFToppy())
            ConvertUTFStr(CurService.ServiceName, CurService.ServiceName, sizeof(CurService.ServiceName), !FileHeader.UTF8System);

          if (ret && (CurService.SatIndex < NrSats) && (CurService.TransponderIndex < FlashTransponderTablesGetTotal(CurService.SatIndex)))
            ret = FlashServiceAdd((CurMode==SM_TVServices) ? SVC_TYPE_Tv : SVC_TYPE_Radio, &CurService);
          else
            ret = FALSE;

          if (ret)
            (CurMode==SM_TVServices) ? NrImpTVServices++ : NrImpRadioServices++;
          else
            WriteLogCSf(PROGRAM_NAME, "  Error in %s-service nr. %d! (line %d)", (CurMode==SM_TVServices) ? "TV" : "Radio", (CurMode==SM_TVServices) ? NrImpTVServices : NrImpRadioServices, CurLine);
          break;
        }

        //[Favorites]
        //#Nr; GroupName; T/R; SvcNums
        case SM_Favorites:
        {
          tFavorites             CurFavGroup;
          word                   curSvcNum;
          char                   curSvcType;
          int                    BytesRead;
          char                  *p;

          memset(&CurFavGroup, 0, sizeof(tFavorites));
          ret = (sscanf(Buffer, "%*i ; %11[^;\r\n] ; %c ;%n", CurFavGroup.GroupName, &curSvcType, &BytesRead) == 2) && ret;
          RTrim(CurFavGroup.GroupName);

          if (ret)
          {
            int i = 0;
            p = &Buffer[BytesRead];
            while (p && p[0] && (i < NrFavsPerGroup))
            {
              if (*p == ',') p++;
              curSvcNum = strtol(p, &p, 0);
//              if (curSvcNum < (curSvcType == 'T' ? NrImpTVServices : NrImpRadioServices))
//              {
                CurFavGroup.SvcNum[CurFavGroup.NrEntries]  = curSvcNum;
                CurFavGroup.SvcType[CurFavGroup.NrEntries] = (curSvcType == 'T' ? SVC_TYPE_Tv : SVC_TYPE_Radio);
                CurFavGroup.NrEntries++;
//              }
              i++;
            }
#ifdef FULLDEBUG
  WriteLogCSf(PROGRAM_NAME, "  FavGroup %d: Name = '%s', ServiceType = '%c', Entries = %d: %d, %d, %d", NrImpFavGroups, CurFavGroup.GroupName, curSvcType, CurFavGroup.NrEntries, CurFavGroup.SvcNum[0],CurFavGroup.SvcNum[1],CurFavGroup.SvcNum[2]);
#endif
            ret = FlashFavoritesSetInfo(NrImpFavGroups, &CurFavGroup);
          }

          if (ret)
            NrImpFavGroups++;
          else
            WriteLogCSf(PROGRAM_NAME, "  Error in FavGroup nr. %d! (line %d)", NrImpFavGroups, CurLine);
          break;
        }

        default:
          break;
      }
    }
    fclose(fImportFile);

    WriteLogCSf(PROGRAM_NAME, "  %d of %d lines processed.", CurLine, NrOfLines);
    WriteLogCSf(PROGRAM_NAME, "  %d of %d Satellites, %d of %d Transponders, %d of %d TVServices, %d of %d RadioServices, %d of %d FavGroups imported.",
                               NrImpSatellites, FileHeader.NrSatellites, NrImpTransponders, FileHeader.NrTransponders, NrImpTVServices, FileHeader.NrTVServices, NrImpRadioServices, FileHeader.NrRadioServices, NrImpFavGroups, FileHeader.NrFavGroups);
  }
  else
    WriteLogCS(PROGRAM_NAME, "  File not found!");

  free(Buffer);
//  if(ret)
//    FlashProgram();

  if (ret)
    WriteLogCSf(PROGRAM_NAME, "--> Import '%s' (Text) successful.", FileName);
  else
    WriteLogCSf(PROGRAM_NAME, "--> Error during import '%s'.", FileName);
  TRACEEXIT();
  return ret;
}
