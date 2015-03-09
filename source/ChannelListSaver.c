#define _FILE_OFFSET_BITS  64
#define __USE_LARGEFILE64  1
#ifdef _MSC_VER
  #define __const const
#endif
//#undef malloc
//#undef free

#define _GNU_SOURCE
//#define  STACKTRACE     TRUE
#include                <stdlib.h>
#include                <stdio.h>
#include                <stdarg.h>
#include                <tap.h>
#include                <libFireBird.h>
#include                "../../../../../Topfield/FireBirdLib/flash/FBLib_flash.h"
#include                "FlashSatTablesSetInfo.h"
#include                "ChannelListSaver.h"
#include                "ImExportBin.h"
#include                "ImExportText.h"

TAP_ID                  (TAPID);
TAP_PROGRAM_NAME        (PROGRAM_NAME" "VERSION);
TAP_PROGRAM_VERSION     (VERSION);
TAP_AUTHOR_NAME         (AUTHOR);
TAP_DESCRIPTION         (DESCRIPTION);
TAP_ETCINFO             (__DATE__);


typedef enum
{
  LS_UnknownSystemType,
  LS_ImportSysQuestion,
  LS_ImportBinQuestion,
  LS_ImportTxtQuestion,
  LS_AnswerImport,
  LS_AnswerOverwrite,
  LS_AnswerCancel,
  LS_ImportBinSuccess,
  LS_ImportTxtSuccess,
  LS_ImportError,
  LS_ExportSuccess,
  LS_ExportError,
  LS_NrStrings
} tLngStrings;

void OSDMenuMessageBoxDoScrollOver(word *event, dword *param1);

// Globale Variablen
int                     ImportFormat = 0;  // 0 - Binary, 1 - Text, 2 - System
int                     OverwriteSatellites = 1;  // 0 - nie, 1 - auto, 2 - immer

bool                    CSShowMessageBox = FALSE;
dword                   LastMessageBoxKey;


// ============================================================================
//                              IMPLEMENTIERUNG
// ============================================================================

int TAP_Main(void)
{
  char                  TempStr[512];
  int                   Answer = 2;
  bool                  ret = TRUE;

  #if STACKTRACE == TRUE
    CallTraceInit();
    CallTraceEnable(TRUE);
  #endif
  TRACEENTER();

  WriteLogCS (PROGRAM_NAME, "***  ChannelListSaver " VERSION " started! (FBLib " __FBLIB_VERSION__ ") ***");
  WriteLogCS (PROGRAM_NAME, "=======================================================");
  WriteLogCSf(PROGRAM_NAME, "Receiver Model: %s (%u), System Type: TMS-%c (%d)", GetToppyString(GetSysID()), GetSysID(), SysTypeToStr(), GetSystemType());
  WriteLogCSf(PROGRAM_NAME, "Firmware: %s", GetApplVer());

//  TAP_EnterNormalNoInfo();

  // Load Language Strings
  if(!LangLoadStrings(LNGFILENAME, LS_NrStrings, LAN_English, PROGRAM_NAME))
  {
    WriteLogCSf(PROGRAM_NAME, "Language file '%s' not found!\r\n", LNGFILENAME);
    OSDMenuInfoBoxShow(PROGRAM_NAME " " VERSION, "Language file not found!", 500);
    CSShowMessageBox = TRUE;
    do
    {
      TAP_SystemProc();
      TAP_Sleep(1);
    } while(OSDMenuInfoBoxIsVisible());

    TRACEEXIT();
    return 0;
  }

  // Main-Funktion
  if (InitSystemType())
  {
    TAP_Hdd_ChangeDir(LOGDIR);
    LoadINI();

    if (TAP_Hdd_Exist("Settings.std") && (ImportFormat == 2 || (!TAP_Hdd_Exist(EXPORTFILENAME ".dat") && !TAP_Hdd_Exist(EXPORTFILENAME ".txt"))))
    {
      TAP_SPrint(TempStr, sizeof(TempStr), LangGetString(LS_ImportSysQuestion), "Settings.std");
      Answer = ShowConfirmationDialog(TempStr);
      if (Answer == 1)
        ret = HDD_ImExportChData("Settings.std", TAPFSROOT LOGDIR, TRUE);
    }

    else if (TAP_Hdd_Exist(EXPORTFILENAME ".txt") && (ImportFormat == 1 || !TAP_Hdd_Exist(EXPORTFILENAME ".dat")))
    {
      TAP_SPrint(TempStr, sizeof(TempStr), LangGetString(LS_ImportTxtQuestion), EXPORTFILENAME ".txt");
      Answer = ShowConfirmationDialog(TempStr);
      if (Answer == 1)
      {
//        WriteLogCS(PROGRAM_NAME, "[Aktion] Importiere '" EXPORTFILENAME ".txt' (Text)...");
        #ifdef FULLDEBUG
          HDD_ImExportChData("Settings_vor.std", TAPFSROOT LOGDIR, FALSE);
        #endif
        ret = (DeleteAllSettings(OverwriteSatellites==2) &&
               ImportSettings_Text(EXPORTFILENAME ".txt", TAPFSROOT LOGDIR, OverwriteSatellites));
        #ifdef FULLDEBUG
          ExportSettings("Debug_AfterTxtImport.dat", TAPFSROOT LOGDIR);
        #endif
        if (ret)
          ShowErrorMessage(LangGetString(LS_ImportTxtSuccess), NULL);
        else
          ShowErrorMessage(LangGetString(LS_ImportError), NULL);
      }
    }

    else if(TAP_Hdd_Exist(EXPORTFILENAME ".dat"))
    {
      TAP_SPrint(TempStr, sizeof(TempStr), LangGetString(LS_ImportBinQuestion), EXPORTFILENAME ".dat");
      Answer = ShowConfirmationDialog(TempStr);
      if (Answer == 1)
      {
//        WriteLogCS(PROGRAM_NAME, "[Aktion] Importiere '" EXPORTFILENAME ".dat' (binär)...");
        #ifdef FULLDEBUG
          HDD_ImExportChData("Settings_vor.std", TAPFSROOT LOGDIR, FALSE);
        #endif
        ret = (DeleteAllSettings(OverwriteSatellites==2) &&
               ImportSettings(EXPORTFILENAME ".dat", TAPFSROOT LOGDIR, OverwriteSatellites));
        #ifdef FULLDEBUG
          ExportSettings("Debug_AfterBinImport.dat", TAPFSROOT LOGDIR);
        #endif
        if (ret)
          ShowErrorMessage(LangGetString(LS_ImportBinSuccess), NULL);
        else
          ShowErrorMessage(LangGetString(LS_ImportError), NULL);
      }
    }

    if (Answer == 2)
    {
//      WriteLogCS(PROGRAM_NAME, "[Aktion] Exportiere Settings...");
      ret = ExportSettings(EXPORTFILENAME ".dat",      TAPFSROOT LOGDIR);
      ret = ExportSettings_Text(EXPORTFILENAME ".txt", TAPFSROOT LOGDIR) && ret;
      ret = HDD_ImExportChData("Settings.std",         TAPFSROOT LOGDIR, FALSE) && ret;
      if (!ret)
        ShowErrorMessage(LangGetString(LS_ExportError), NULL);
    }
  }
  else
    ShowErrorMessage(LangGetString(LS_UnknownSystemType), NULL);

  LangUnloadStrings();
  WriteLogCS(PROGRAM_NAME, "ChannelListSaver Exit.\r\n");
  TRACEEXIT();
  return 0;
}


dword TAP_EventHandler(word event, dword param1, dword param2)
{
//  TRACEENTER();

  // Behandlung offener MessageBoxen (rekursiver Aufruf, auch bei DoNotReenter)
  if(CSShowMessageBox)
  {
    if(OSDMenuMessageBoxIsVisible() || OSDMenuInfoBoxIsVisible())
    {
      if(OSDMenuMessageBoxIsVisible())
      {
        if(event == EVT_KEY) LastMessageBoxKey = param1;
        #ifndef FBLIB_OLD
          OSDMenuMessageBoxDoScrollOver(&event, &param1);
        #endif
      }
      OSDMenuEvent(&event, &param1, &param2);
    }
    if(!OSDMenuMessageBoxIsVisible() && !OSDMenuInfoBoxIsVisible())
     CSShowMessageBox = FALSE;
    param1 = 0;
  }

//  TRACEEXIT();
  return param1;
}


// ----------------------------------------------------------------------------
//                              Hilfsfunktionen
// ----------------------------------------------------------------------------
void WriteLogCS(char *ProgramName, char *s)
{
  static bool FirstCall = TRUE;

  HDD_TAP_PushDir();

  if(FirstCall)
  {
    HDD_ChangeDir("/ProgramFiles");
    if(!TAP_Hdd_Exist("Settings")) TAP_Hdd_Create("Settings", ATTR_FOLDER);
    HDD_ChangeDir("Settings");
    if(!TAP_Hdd_Exist(PROGRAM_NAME)) TAP_Hdd_Create(PROGRAM_NAME, ATTR_FOLDER);
    FirstCall = FALSE;
  }

  TAP_Hdd_ChangeDir(LOGDIR);
  LogEntry(PROGRAM_NAME ".log", ProgramName, TRUE, TIMESTAMP_YMDHMS, s);
  HDD_TAP_PopDir();
}
void WriteLogCSf(char *ProgramName, const char *format, ...)
{
  char Text[512];

  if(format)
  {
    va_list args;
    va_start(args, format);
    vsnprintf(Text, sizeof(Text), format, args);
    va_end(args);
    WriteLogCS(ProgramName, Text);
  }
}

char SysTypeToStr(void)
{
  switch (GetSystemType())
  {
    case ST_TMSS:  return 'S';
    case ST_TMSC:  return 'C';
    case ST_TMST:  return 'T';
    default:       return '?';
  }
}

bool HDD_ImExportChData(char *FileName, char *AbsDirectory, bool Import)
{
  tDirEntry             FolderStruct;
  char                  AbsFileName[FBLIB_DIR_SIZE]; //, TmpFileName[FBLIB_DIR_SIZE];
  bool                  ret = FALSE;

  TRACEENTER();
  HDD_TAP_PushDir();
  WriteLogCSf(PROGRAM_NAME, (Import ? "[Action] Importing '%s' (System)..." : "[Action] Exporting '%s' (System)..."), FileName);
  WriteLogCS(PROGRAM_NAME, "----------------------------------------");

  TAP_SPrint(AbsFileName, sizeof(AbsFileName), "%s/%s", &AbsDirectory[21], FileName);
//  TAP_SPrint(TmpFileName, sizeof(TmpFileName), TAPFSROOT "/ProgramFiles/%s", FileName);
//  rename(AbsFileName, TmpFileName); 

  //Initialize the directory structure
  memset(&FolderStruct, 0, sizeof(tDirEntry));
  FolderStruct.Magic = 0xbacaed31;

  //Save the current directory resources and change into our directory (current directory of the TAP)
//  ApplHdd_SaveWorkFolder();
//  if (!ApplHdd_SelectFolder(&FolderStruct, &AbsDirectory[1]))   //do not include the leading slash
  {
//    ApplHdd_SetWorkFolder(&FolderStruct);
    if (Import)
      ret = Appl_ImportChData(AbsFileName);
    else
      ret = Appl_ExportChData(AbsFileName);
  }
//  ApplHdd_RestoreWorkFolder();

//  rename(TmpFileName, AbsFileName);
  if (ret)
    WriteLogCSf(PROGRAM_NAME, (Import ? "--> Import '%s' successful." : "--> Export '%s' successful."), AbsFileName);
  else
    WriteLogCSf(PROGRAM_NAME, (Import ? "--> Error during import '%s'!" : "--> Error during export '%s'!"), AbsFileName);
  HDD_TAP_PopDir();

  TRACEEXIT();
  return ret;
}

void DebugServiceNames(char* FileName)
{
  FILE *fOut = NULL;
  char fn[512];
  char *p = NULL;
  TRACEENTER();

  TAP_SPrint(fn, sizeof(fn), TAPFSROOT LOGDIR "/%s", FileName);
  fOut = fopen(fn, "wb");
  p = (char*)(FIS_vFlashBlockServiceName());
  if(p && fOut)
  {
    fwrite(p, 1, SERVICENAMESLENGTH, fOut);
    fclose(fOut);
  }
  TRACEEXIT();
}


// ----------------------------------------------------------------------------
//                           MessageBox-Funktionen
// ----------------------------------------------------------------------------
#ifndef FBLIB_OLD
  #define STDSTRINGSIZE   256
  #define MAXMBBUTTONS    5

  typedef struct
  {
    dword                 NrButtons;
    dword                 CurrentButton;
    char                  Button[MAXMBBUTTONS][STDSTRINGSIZE];
    char                  Title[STDSTRINGSIZE];
    char                  Text[STDSTRINGSIZE];
//    tFontData            *FontColorPickerTitle;
//    tFontData            *FontColorPickerCursor;
  } tMessageBox;

  extern tMessageBox      MessageBox;
  bool                    MessageBoxAllowScrollOver = FALSE;

  void  OSDMenuMessageBoxAllowScrollOver()
  {
    MessageBoxAllowScrollOver = TRUE;
  }

  void OSDMenuMessageBoxDoScrollOver(word *event, dword *param1)
  {
//    TRACEENTER();
    if(MessageBoxAllowScrollOver && (MessageBox.NrButtons > 1))
    {
      if ((*event == EVT_KEY) && (*param1 == RKEY_Left))
      {
        if(MessageBox.CurrentButton == 0)
        {
          MessageBox.CurrentButton = MessageBox.NrButtons - 1;
          OSDMenuMessageBoxShow();
          *param1 = 0;
        }
      }
      if ((*event == EVT_KEY) && (*param1 == RKEY_Right))
      {  
        if(MessageBox.CurrentButton >= (MessageBox.NrButtons - 1))
        {
          MessageBox.CurrentButton = 0;
          OSDMenuMessageBoxShow();
          *param1 = 0;
        }
      }
    }
//    TRACEEXIT();
  }
#endif

// Die Funktionen zeigt eine Bestätigungsfrage (Import/Überschreiben/Abbrechen) an, und wartet auf die Bestätigung des Benutzers.
// Nach Beendigung der Message kehrt das TAP in den Normal-Mode zurück, FALLS dieser zuvor aktiv war.
// Beim Beenden wird das entsprechende OSDRange gelöscht. Überdeckte Bereiche anderer OSDs werden NICHT wiederhergestellt.
int ShowConfirmationDialog(char *MessageStr)
{
  dword OldSysState, OldSysSubState;
  bool ret;

  TRACEENTER();
  HDD_TAP_PushDir();
  TAP_GetState(&OldSysState, &OldSysSubState);

  OSDMenuMessageBoxInitialize(PROGRAM_NAME, MessageStr);
//  OSDMenuSaveMyRegion(rgnSegmentList);
  OSDMenuMessageBoxDoNotEnterNormalMode(TRUE);
  OSDMenuMessageBoxAllowScrollOver();
  OSDMenuMessageBoxButtonAdd(LangGetString(LS_AnswerImport));
  OSDMenuMessageBoxButtonAdd(LangGetString(LS_AnswerOverwrite));
  OSDMenuMessageBoxButtonAdd(LangGetString(LS_AnswerCancel));
  OSDMenuMessageBoxButtonSelect(2);
  OSDMenuMessageBoxShow();
  CSShowMessageBox = TRUE;
  while (CSShowMessageBox)
  {
    TAP_SystemProc();
    TAP_Sleep(1);
  }
  ret = 0;
  if ((LastMessageBoxKey == RKEY_Ok) && (OSDMenuMessageBoxLastButton() != 2))
    ret = OSDMenuMessageBoxLastButton() + 1;

  TAP_Osd_Sync();
  if(OldSysSubState != 0) TAP_EnterNormalNoInfo();
//  OSDMenuFreeStdFonts();

  HDD_TAP_PopDir();
  TRACEEXIT();
  return ret;
}

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


// ----------------------------------------------------------------------------
//                            INI-Funktionen
// ----------------------------------------------------------------------------
void LoadINI(void)
{
  TRACEENTER();

  INILOCATION IniFileState;

  HDD_TAP_PushDir();
  HDD_ChangeDir(LOGDIR);
  IniFileState = INIOpenFile(INIFILENAME, PROGRAM_NAME);
  if((IniFileState != INILOCATION_NotFound) && (IniFileState != INILOCATION_NewFile))
  {
    ImportFormat        = INIGetInt("ImportFormat",        1, 0, 2);   // 0 - Binary, 1 - Text, 2 - System
    OverwriteSatellites = INIGetInt("OverwriteSatellites", 1, 0, 2);
  }
  INICloseFile();

  if(IniFileState == INILOCATION_NewFile)
    SaveINI();
  HDD_TAP_PopDir();

  TRACEEXIT();
}

void SaveINI(void)
{
  TRACEENTER();

  HDD_TAP_PushDir();
  HDD_ChangeDir(LOGDIR);
  INIOpenFile(INIFILENAME, PROGRAM_NAME);
  INISetInt ("ImportFormat",        ImportFormat);
  INISetInt ("OverwriteSatellites", OverwriteSatellites);
  INISaveFile(INIFILENAME, INILOCATION_AtCurrentDir, NULL);
  INICloseFile();
  HDD_TAP_PopDir();

  TRACEEXIT();
}


// ----------------------------------------------------------------------------
//                           Hauptfunktionen
// ----------------------------------------------------------------------------
bool InitSystemType(void)
{
  TRACEENTER();
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
      WriteLogCSf(PROGRAM_NAME, "Unsupported system type: %d!", CurSystemType);
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
      WriteLogCS(PROGRAM_NAME, "Nicht unterstützte Favoriten-Struktur!");
      ret = FALSE;
      break;
  }

  TRACEEXIT();
  return ret;
}

int GetLengthOfServiceNames(int *NrServiceNames)
{
  TRACEENTER();
  int Result = 0;
  int i;
  char *p1, *p2;
  p1 = (char*)(FIS_vFlashBlockServiceName());
  p2 = (char*)(FIS_vFlashBlockProviderInfo());

  if(NrServiceNames) *NrServiceNames = 0;
  if(p1)
  {
    Result = SERVICENAMESLENGTH;
    if (p2 && (p2 > p1) && (p2-p1 < Result))
      Result = p2 - p1;

    for (i = 0; i < Result-1; i++)
    {
      if (!p1[i])
      {
        if (NrServiceNames) *NrServiceNames = *NrServiceNames + 1;
        if (!p1[i+1])
        {
          Result = i+1;
          break;
        }
      }
    }
  }
  TRACEEXIT();
  return Result;
}

bool DeleteTimers(void)
{
  TRACEENTER();
  int Count;
  bool ret = TRUE;

  Count = 0;
  while(TAP_Timer_GetTotalNum() > 0)
  {
    ret = TAP_Timer_Delete(0) && ret;
    Count++;
  }
  WriteLogCSf(PROGRAM_NAME, "%d timers have been deleted.", Count);
  TRACEEXIT();
  return ret;
}

void DeleteServiceNames(void)
{
  TRACEENTER();
  void  (*Appl_DeleteTvSvcName)(unsigned short, bool);
  void  (*Appl_DeleteRadioSvcName)(unsigned short, bool);
  Appl_DeleteTvSvcName    = (void*)FIS_fwAppl_DeleteTvSvcName();
  Appl_DeleteRadioSvcName = (void*)FIS_fwAppl_DeleteRadioSvcName();

  int                   nTVServices, nRadioServices, i;
  TAP_Channel_GetTotalNum(&nTVServices, &nRadioServices);

  WriteLogCS(PROGRAM_NAME, "DeleteServiceNames()");
//  char tmp[512];
//  DebugServiceNames("vorher.dat");
  for (i = (nRadioServices - 1); i >= 0; i--)
  {
    Appl_DeleteRadioSvcName(i, FALSE);
//    TAP_SPrint(tmp, sizeof(tmp), "Rad%d.dat", i);
//    DebugServiceNames(tmp);
  }

  for (i = (nTVServices - 1); i >= 0; i--)
  {
    Appl_DeleteTvSvcName(i, FALSE);
//    TAP_SPrint(tmp, sizeof(tmp), "TV%d.dat", i);   // BUG: egal, was man tut - der Eintrag in 0 wird niemals gelöscht :-(
//    DebugServiceNames(tmp);
  }

//  DebugServiceNames("nachher.dat");
  TRACEEXIT();
}

bool DeleteAllSettings(bool OverwriteSatellites)
{
  TRACEENTER();
  bool ret = TRUE;

  WriteLogCS(PROGRAM_NAME, "[Action] Deleting all settings...");
  WriteLogCS(PROGRAM_NAME, "----------------------------------------");
  DeleteTimers();
  {
    // Favorites
    char                 *p;
    p = (char*) FIS_vFlashBlockFavoriteGroup();

    if (p)
      memset(p, 0, NrFavGroups * SIZE_Favorites);
    else ret = FALSE;
  }

  {
    // Service Names
    char                 *p1, *p2;

    DeleteServiceNames();
    #ifndef FULLDEBUG
      p1 = (char*)(FIS_vFlashBlockServiceName());
      p2 = (char*)(FIS_vFlashBlockProviderInfo());
      if(p1 && p2 && (p2 > p1))
        memset(p, 0, min(p2 - p1, SERVICENAMESLENGTH));
    #endif
  }

  {
    // Provider Names
    char                 *p;

    p = (char*)(FIS_vFlashBlockProviderInfo());
    if(ret && p)
      memset(p, 0, PROVIDERNAMELENGTH * NRPROVIDERNAMES);
  }

  {
    // TV Services
    TYPE_Service_TMSS      *p;
    word                   *nSvc;
    word                    nServices;

    p    = (TYPE_Service_TMSS*)(FIS_vFlashBlockTVServices());
    nSvc = (word*)FIS_vnTvSvc();
    if (ret && p && nSvc)
    {
      nServices = *nSvc;
      *nSvc = 0;
      memset(p, 0, *nSvc * SIZE_Service_TMSx);
    }

    // Radio Services
    p    = (TYPE_Service_TMSS*)(FIS_vFlashBlockRadioServices());
    nSvc = (word*)FIS_vnRadioSvc();
    if (ret && p && nSvc)
    {
      nServices = *nSvc;
      *nSvc = 0;
      memset(p, 0, *nSvc * SIZE_Service_TMSx);
    }
    else ret = FALSE;
  }

  {
    // Transponders
    TYPE_TpInfo_TMSS *p;
    dword            *pNrTransponders;
    dword             NrTransponders;
          
    p = (TYPE_TpInfo_TMSS*)(FIS_vFlashBlockTransponderInfo());
    if (ret && p)
    {
      pNrTransponders = (dword*)(p) - 1;
      NrTransponders = *pNrTransponders;
      *pNrTransponders = 0;
      memset(p, 0, NrTransponders * SIZE_TpInfo_TMSx);
    }
    else ret = FALSE;
  }

  {
    // Satellites
    int   i;
    byte *p;
    p = (byte*)FIS_vFlashBlockSatInfo();
    if (ret && p)
    {
      if (OverwriteSatellites)
        memset(p, 0, FlashSatTablesGetTotal() * SIZE_SatInfo_TMSx);
      else
      {
        // Wenn ohne SAT:
        // nicht die Satelliten löschen, sondern deren TransponderNrs auf 0 setzen
        for (i = 0; i < FlashSatTablesGetTotal(); i++)
        {
          word* NrTranspondersOfSat = GetpNrTranspOfSat_TMSx((TYPE_SatInfo_TMSx*)(p + i * SIZE_SatInfo_TMSx));
          *NrTranspondersOfSat      = 0;
        }
      }
    }
    else ret = FALSE;
  }

  if(!ret) WriteLogCS(PROGRAM_NAME, "Error while deleting old settings!");
  TRACEEXIT();
  return ret;
}
