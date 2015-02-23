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
#include                <stdarg.h>
#include                <tap.h>
#include                <libFireBird.h>
#include                "../../../../../Topfield/FireBirdLib/flash/FBLib_flash.h"
#include                "FlashSatTablesSetInfo.h"
#include                "ChannelListTAP.h"
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
  LS_NrStrings
} tLngStrings;


// ============================================================================
//                              IMPLEMENTIERUNG
// ============================================================================
void OSDMenuMessageBoxDoScrollOver(word *event, dword *param1);

int                     ImportFormat = 0;  // 0 - Binary, 1 - Text, 2 - System
bool                    OverwriteSatellites = FALSE;

bool                    CSShowMessageBox = FALSE;
dword                   LastMessageBoxKey;


int TAP_Main(void)
{
  int Answer = 2;
  bool ret = TRUE;

  #if STACKTRACE == TRUE
    CallTraceInit();
    CallTraceEnable(TRUE);
  #endif
  TRACEENTER();

  WriteLogCS (PROGRAM_NAME, "***  ChannelListSaver " VERSION " started! (FBLib " __FBLIB_VERSION__ ") ***");
  WriteLogCS (PROGRAM_NAME, "=======================================================");
  WriteLogCSf(PROGRAM_NAME, "Receiver Model: %s (%u)", GetToppyString(GetSysID()), GetSysID());
  WriteLogCSf(PROGRAM_NAME, "Firmware: %s", GetApplVer());

  // Load Language Strings
  if(!LangLoadStrings(LNGFILENAME, LS_NrStrings, LAN_English, PROGRAM_NAME))
  {
    WriteLogCSf(PROGRAM_NAME, "Language file '%s' not found!", LNGFILENAME);
    OSDMenuInfoBoxShow(PROGRAM_NAME " " VERSION, "Language file not found!", 500);
    do
    {
      OSDMenuEvent(NULL, NULL, NULL);
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
      Answer = ShowConfirmationDialog("System-Backup 'Settings.std' gefunden.\nSoll diese nun importiert werden?");
      if (Answer == 1)
      {
        WriteLogCS(PROGRAM_NAME, "[Aktion] Importiere 'Settings.std' (System)...");
        ret = Appl_ImportChData("Settings.std");
        if (ret)
          WriteLogCS(PROGRAM_NAME, "--> Import erfolgreich!");
        else
          WriteLogCS(PROGRAM_NAME, "--> Fehler beim Import!");
      }
    }

    else if (TAP_Hdd_Exist(EXPORTFILENAME ".txt") && (ImportFormat == 1 || !TAP_Hdd_Exist(EXPORTFILENAME ".dat")))
    {
      Answer = ShowConfirmationDialog("Text-Datei '" EXPORTFILENAME "' gefunden.\nSoll diese nun importiert werden?");
      if (Answer == 1)
      {
        WriteLogCS(PROGRAM_NAME, "[Aktion] Importiere '" EXPORTFILENAME ".txt' (Text)...");
        DeleteTimers();
        ret = (DeleteAllSettings() &&
               ImportSettings_Text(EXPORTFILENAME ".txt", TAPFSROOT LOGDIR, OverwriteSatellites));
        if (ret)
          ShowErrorMessage("Einstellungen (Text) importiert.", NULL);
        else
          ShowErrorMessage("Fehler beim Import!\n\nBitte das Logfile prüfen...", NULL);
      }
    }

    else if(TAP_Hdd_Exist(EXPORTFILENAME ".dat"))
    {
      Answer = ShowConfirmationDialog("Binär-Datei '" EXPORTFILENAME ".dat' gefunden.\nSoll diese nun importiert werden?");
      if (Answer == 1)
      {
        WriteLogCS(PROGRAM_NAME, "[Aktion] Importiere '" EXPORTFILENAME ".dat' (binär)...");
        DeleteTimers();
        ret = (DeleteAllSettings() &&
               ImportSettings(EXPORTFILENAME ".dat", TAPFSROOT LOGDIR, OverwriteSatellites));
        if (ret)
          ShowErrorMessage("Einstellungen (Text) importiert.", NULL);
        else
          ShowErrorMessage("Fehler beim Import!\n\nBitte das Logfile prüfen...", NULL);
      }
    }

    else if (Answer == 2)
    {
      WriteLogCS(PROGRAM_NAME, "[Aktion] Exportiere Settings...");
      ret = ExportSettings(EXPORTFILENAME ".dat",      TAPFSROOT LOGDIR) && ret;
      ret = ExportSettings_Text(EXPORTFILENAME ".txt", TAPFSROOT LOGDIR) && ret;
      ret = Appl_ExportChData("Settings.std")                            && ret;
      if (ret) WriteLogCS(PROGRAM_NAME, "Export 'Settings.std' (System) erfolgreich.");
      if (ret)
        WriteLogCS(PROGRAM_NAME, "--> Export erfolgreich!");
      else
      {
        WriteLogCS(PROGRAM_NAME, "--> Fehler beim Export!");
        ShowErrorMessage("Fehler beim Export!\n\nBitte das Logfile prüfen...", NULL);
      }
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
  TRACEENTER();

  // Behandlung offener MessageBoxen (rekursiver Aufruf, auch bei DoNotReenter)
  if(CSShowMessageBox)
  {
    if(OSDMenuMessageBoxIsVisible())
    {
//      #ifdef Calibri_10_FontDataUC
        OSDMenuMessageBoxDoScrollOver(&event, &param1);
//      #endif
      OSDMenuEvent(&event, &param1, &param2);
    }
    if(!OSDMenuMessageBoxIsVisible())
      CSShowMessageBox = FALSE;
    param1 = 0;
  }

  TRACEEXIT();
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

void DebugServiceNames(char* FileName)
{
  FILE *fOut = NULL;
  char fn[512];
  char *p = NULL;
  TRACEENTER();

  TAP_SPrint(fn, sizeof(fn), "%s%s/%s", TAPFSROOT, LOGDIR, FileName);
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
  TRACEENTER();
  if(MessageBoxAllowScrollOver)
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
  TRACEEXIT();
}

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
  OSDMenuMessageBoxButtonAdd("Importieren");
  OSDMenuMessageBoxButtonAdd("Überschreiben");
  OSDMenuMessageBoxButtonAdd("Abbrechen");
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
    ImportFormat        =          INIGetInt("ImportFormat",               1,   0,    2);   // 0 - Binary, 1 - Text, 2 - System
    OverwriteSatellites =          INIGetInt("OverwriteSatellites",        0,   0,    1)   ==   1;
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
  INISetInt ("OverwriteSatellites", OverwriteSatellites ?  1  :  0);
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
      WriteLogCSf(PROGRAM_NAME, "Nicht unterstütztes System: %d!", CurSystemType);
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

int GetLengthOfServiceNames(void)
{
  TRACEENTER();
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

  TYPE_Service_TMSx         *p;
  int nTVServices, nRadioServices, i;
  TAP_Channel_GetTotalNum(&nTVServices, &nRadioServices);

  WriteLogCS(PROGRAM_NAME, "DeleteServiceNames()");
//  DebugServiceNames("vorher.dat");
//  char tmp[512];
  for (i = (nRadioServices - 1); i >= 0; i--)
  {
    if ((i == 0) && (nRadioServices > 1))
    {
      p = (TYPE_Service_TMSx*)(FIS_vFlashBlockRadioServices());
      p[1].NameOffset = p[0].NameOffset;
      Appl_DeleteRadioSvcName(1, FALSE);
    }
    else
      Appl_DeleteRadioSvcName(i, FALSE);
//    TAP_SPrint(tmp, "Rad%ld.dat", i);
//    DebugServiceNames(tmp);
  }
  for (i = (nTVServices - 1); i >= 0; i--)
  {
/*    if ((i == 0) && (nTVServices > 1))
    {
      p = (TYPE_Service_TMSx*)(FIS_vFlashBlockTVServices());
      p[1].NameOffset = p[0].NameOffset;
      Appl_DeleteTvSvcName(1, FALSE);
    }
    else  */
      Appl_DeleteTvSvcName(i, TRUE);
//    TAP_SPrint(tmp, "TV%ld.dat", i);
//    DebugServiceNames(tmp);
  }
//  DebugServiceNames("nachher.dat");
  TRACEEXIT();
}

bool DeleteAllSettings(void)
{
  TRACEENTER();
  bool ret = TRUE;

  WriteLogCS(PROGRAM_NAME, "DeleteAllSettings()");
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
    word                    nServices;

    p    = (TYPE_Service_TMSS*)(FIS_vFlashBlockTVServices());
    nSvc = (word*)FIS_vnTvSvc();
    if (p && nSvc)
    {
      nServices = *nSvc;
      *nSvc = 0;
      memset(p, 0, *nSvc * SIZE_Service_TMSx);
    }

    // Radio Services
    p    = (TYPE_Service_TMSS*)(FIS_vFlashBlockRadioServices());
    nSvc = (word*)FIS_vnRadioSvc();
    if (p && nSvc)
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
    if (p)
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
    if (p)
    {
      if (OverwriteSatellites)
      {
        memset(p, 0, FlashSatTablesGetTotal() * SIZE_SatInfo_TMSx);
      }
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

  TRACEEXIT();
  return ret;
}
