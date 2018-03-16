#define _LARGEFILE_SOURCE
#define _LARGEFILE64_SOURCE
#define __USE_LARGEFILE64  1
#define _FILE_OFFSET_BITS  64
#ifdef _MSC_VER
  #define __const const
#endif
//#undef malloc
//#undef free

//#define __ALTEFBLIB__

//#define  STACKTRACE     TRUE
#define _GNU_SOURCE
#include                <stdlib.h>
#include                <stdio.h>
#include                <stdarg.h>
#include                <string.h>
#include                <utime.h>
#include                <sys/stat.h>
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
  LS_ImportQuestion,
  LS_Binary,
  LS_Text,
  LS_System,
  LS_AnswerImport,
  LS_AnswerOverwrite,
  LS_AnswerCancel,
  LS_ExportSuccess,
  LS_ImportSuccess,
  LS_ExportError,
  LS_ImportError,
  LS_NrStrings
} tLngStrings;

char* DefaultStrings[LS_NrStrings] =
{
  "SystemType ist unbekannt.\nBitte die FirmwareTMS.dat überprüfen!",
  "%s-Datei '%s' gefunden.\nSoll diese nun importiert werden?",
  "Binär",
  "Text",
  "System",
  "Import",
  "Ersetzen",
  "Abbruch",
  "Einstellungen erfolgreich exportiert.",
  "Einstellungen (%s) importiert.",
  "Fehler beim Export!\nBitte das Logfile prüfen...",
  "Fehler beim Import!\nBitte das Logfile prüfen..."
 };

#define LangGetString(x)  LangGetStringDefault(x, DefaultStrings[x])


//static void  OSDMenuMessageBoxDoScrollOver(word *event, dword *param1);

static void  CreateRootDir(void);
static char  SysTypeToStr(void);
static bool  InitSystemType(void);
static int   ShowConfirmationDialog(char *MessageStr);
static void  ShowErrorMessage(char *MessageStr, char *TitleStr);
static void  LoadINI(void);
static void  SaveINI(void);
static word* GetSETimerID(int TimerNr);
static tTimerExt* SaveTimers(int *NrOldTimers);
static void  RestoreTimers(tTimerExt *OldTimers, int NrOldTimers);


// Globale Variablen
SYSTEM_TYPE             CurSystemType;
int                     NrFavGroups;
int                     NrFavsPerGroup;
size_t                  SIZE_SatInfo_TMSx;
size_t                  SIZE_TpInfo_TMSx;
size_t                  SIZE_Service_TMSx;
size_t                  SIZE_Favorites;

//static bool             CSShowMessageBox = FALSE;
static char            *TxtFileName = TXTFILENAME, *DatFileName = DATFILENAME, *StdFileName = STDFILENAME;
static tTimerExt       *OldTimers = NULL;
static int              NrOldTimers = 0;
static int              ImportFormat = 0;  // 0 - Binary, 1 - Text, 2 - System
static int              OverwriteSatellites = 1;  // 0 - nie, 1 - auto, 2 - immer
static bool             RestoreNameLock = FALSE;
static bool             SilentMode = FALSE;
static bool             CLSFinished = FALSE;
static char             TempStr[512], *pMessage = NULL;

#define minmax(v,d,a,b)  ( ( ((v) < (a)) || ((v) > (b)) ) ? (d) : (v) )


// ============================================================================
//                              IMPLEMENTIERUNG
// ============================================================================
int TAP_Main(void)
{
  tParameters*          StartParameter = NULL;
  int                   Answer = 2;
  bool                  ret = TRUE;

  #if STACKTRACE == TRUE
    CallTraceInit();
    CallTraceEnable(TRUE);
  #endif
  TRACEENTER();

  CreateRootDir();
  WriteLogMC (PROGRAM_NAME, "***  ChannelListSaver " VERSION " started! (FBLib " __FBLIB_VERSION__ ") ***");
  WriteLogMC (PROGRAM_NAME, "=======================================================");
  WriteLogMCf(PROGRAM_NAME, "Receiver Model: %s (%u), System Type: TMS-%c (%d)", GetToppyString(GetSysID()), GetSysID(), SysTypeToStr(), GetSystemType());
  WriteLogMCf(PROGRAM_NAME, "Firmware: %s", GetApplVer());

//  TAP_EnterNormalNoInfo();

  // Load Language Strings
  if (TAP_GetSystemVar(SYSVAR_OsdLan) != LAN_German)
    if(!LangLoadStrings(TAPFSROOT LOGDIR "/" LNGFILENAME, LS_NrStrings, LAN_English))
    {
/*      WriteLogMCf(PROGRAM_NAME, "Language file '%s' not found!\r\n", LNGFILENAME);
      OSDMenuInfoBoxShow(PROGRAM_NAME " " VERSION, "Language file not found!", 500);
      CSShowMessageBox = TRUE;
      do
      {
        TAP_SystemProc();
        TAP_Sleep(1);
      } while(OSDMenuInfoBoxIsVisible());

      CloseLogMC();
      TRACEEXIT();
      return 0;  */
    }

  // DEBUG-AUSGABEN
  if ((void*)FIS_fwAppl_ExportChData() == NULL)
    WriteLogMC("Warning", "Firmware function FIS_fwAppl_ExportChData() not found!");
  if ((void*)FIS_fwAppl_ImportChData() == NULL)
    WriteLogMC("Warning", "Firmware function FIS_fwAppl_ImportChData() not found!");
  if ((void*)FIS_fwTimeToLinux() == NULL)
    WriteLogMC("DEBUG", "Firmware function FIS_fwTimeToLinux() not found!");

/*  dword (*_PvrTimeToLinux)(dword) = NULL;
  _PvrTimeToLinux = (void*)FIS_fwTimeToLinux();
  if(_PvrTimeToLinux)
    WriteLogMC("DEBUG", "_PvrTimeToLinux gefunden!");

  byte Sec;
  dword CurTime = Now(&Sec);
  WriteLogMCf("DEBUG", "Current time: Topfield=%u", CurTime);
  dword CurLinuxTime = PvrTimeToLinux(CurTime) + Sec;
  WriteLogMCf("DEBUG", "Current time: PVRtoLinux=%u, %s", CurLinuxTime, ctime((time_t*) &CurLinuxTime));
  dword CurUnixTime = TF2UnixTime(CurTime) + Sec;
  WriteLogMCf("DEBUG", "Current time: TF2UnixTime=%u, %s", CurUnixTime, ctime((time_t*) &CurUnixTime)); */


  // Main-Funktion
  if (InitSystemType())
  {
    TAP_Hdd_ChangeDir(LOGDIR);
    LoadINI();
    WriteLogMCf(PROGRAM_NAME, "Settings: ImportFormat=%d, OverwriteSatellites=%d", ImportFormat, OverwriteSatellites);

    // auf Start-Parameter prüfen
    StartParameter = (tParameters*) HDD_TAP_GetStartParameter();
    if (StartParameter)
    {
      WriteLogMCf(PROGRAM_NAME, "StartParameters: %s, ImportFormat=%hhu, silent=%hhu, FileName=%s", (StartParameter->ImExport ? "Import" : "Export"), StartParameter->ImportFormat, StartParameter->SilentMode, StartParameter->FileName);
      ImportFormat = StartParameter->ImportFormat;
      SilentMode = StartParameter->SilentMode;
      if (SilentMode)
        Answer = (2 - StartParameter->ImExport);
      if (StartParameter->FileName && *StartParameter->FileName)
        switch (StartParameter->ImportFormat)
        {
          case 0: DatFileName = StartParameter->FileName; break;
          case 1: TxtFileName = StartParameter->FileName; break;
          case 2: StdFileName = StartParameter->FileName; break;
        }
    }

    if (TAP_Hdd_Exist(StdFileName) && (ImportFormat == 2 || (!StartParameter && !TAP_Hdd_Exist(DatFileName) && !TAP_Hdd_Exist(TxtFileName))))
    {
      TAP_SPrint(TempStr, sizeof(TempStr), LangGetString(LS_ImportQuestion), LangGetString(LS_System), StdFileName);
      if (!SilentMode) Answer = ShowConfirmationDialog(TempStr);
      if (Answer == 1)
        ret = HDD_ImExportChData(StdFileName, TAPFSROOT LOGDIR, TRUE);
    }

    else if (TAP_Hdd_Exist(TxtFileName) && (ImportFormat == 1 || (!StartParameter && !TAP_Hdd_Exist(DatFileName))))
    {
      TAP_SPrint(TempStr, sizeof(TempStr), LangGetString(LS_ImportQuestion), LangGetString(LS_Text), TxtFileName);
      if (!SilentMode) Answer = ShowConfirmationDialog(TempStr);
      if (Answer == 1)
      {
//        WriteLogMC(PROGRAM_NAME, "[Aktion] Importiere '" EXPORTFILENAME ".txt' (Text)...");
        #ifdef FULLDEBUG
          HDD_ImExportChData("Settings_vor.std", TAPFSROOT LOGDIR, FALSE);
        #endif
        OldTimers = SaveTimers(&NrOldTimers);
        ret = ImportSettings_Text(TxtFileName, TAPFSROOT LOGDIR, OverwriteSatellites, RestoreNameLock);
        if (OldTimers)
        {
          RestoreTimers(OldTimers, NrOldTimers);
          TAP_MemFree(OldTimers);
          OldTimers = NULL;
        }
        #ifdef FULLDEBUG
          ExportSettings("Debug_AfterTxtImport.dat", TAPFSROOT LOGDIR);
        #endif
        if (!SilentMode)
        {
          if (ret)
          {
            TAP_SPrint(TempStr, sizeof(TempStr), LangGetString(LS_ImportSuccess), LangGetString(LS_Text));
            pMessage = TempStr;
          }
          else
            pMessage = LangGetString(LS_ImportError);
        }
      }
    }

    else if(TAP_Hdd_Exist(DatFileName) && (ImportFormat == 0 || !StartParameter))
    {
      TAP_SPrint(TempStr, sizeof(TempStr), LangGetString(LS_ImportQuestion), LangGetString(LS_Binary), DatFileName);
      if (!SilentMode) Answer = ShowConfirmationDialog(TempStr);
      if (Answer == 1)
      {
//        WriteLogMC(PROGRAM_NAME, "[Aktion] Importiere '" EXPORTFILENAME ".dat' (binär)...");
        #ifdef FULLDEBUG
          HDD_ImExportChData("Settings_vor.std", TAPFSROOT LOGDIR, FALSE);
        #endif
        OldTimers = SaveTimers(&NrOldTimers);
        ret = ImportSettings(DatFileName, TAPFSROOT LOGDIR, OverwriteSatellites, RestoreNameLock);
        if (OldTimers)
        {
          RestoreTimers(OldTimers, NrOldTimers);
          TAP_MemFree(OldTimers);
          OldTimers = NULL;
        }
        #ifdef FULLDEBUG
          ExportSettings("Debug_AfterBinImport.dat", TAPFSROOT LOGDIR);
        #endif
        if (!SilentMode)
        {
          if (ret)
          {
            TAP_SPrint(TempStr, sizeof(TempStr), LangGetString(LS_ImportSuccess), LangGetString(LS_Binary));
            pMessage = TempStr;
          }
          else
            pMessage = LangGetString(LS_ImportError);
        }
      }
    }

    if (Answer == 2)
    {
      WriteLogMC(PROGRAM_NAME, "[Action] Exporting settings...");
      ret = TRUE;
      if (!StartParameter || ImportFormat == 0)
        ret = ExportSettings(DatFileName,      TAPFSROOT LOGDIR) && ret;
      if (!StartParameter || ImportFormat == 1)
        ret = ExportSettings_Text(TxtFileName, TAPFSROOT LOGDIR) && ret;
      if (!StartParameter || ImportFormat == 2)
        ret =(HDD_ImExportChData(StdFileName,  TAPFSROOT LOGDIR, FALSE) || ((void*)FIS_fwAppl_ExportChData() == NULL)) && ret;
      if (!ret && !SilentMode)
        pMessage = LangGetString(LS_ExportError);
    }

    if (StartParameter)
      StartParameter->ResultCode = ret;
  }
  else
  {
    WriteLogMC(PROGRAM_NAME, "Unknown SystemType. Please check FirmwareTMS.dat!\r\n");
    pMessage = LangGetString(LS_UnknownSystemType);
  }
  CLSFinished = 1;

  TRACEEXIT();
  return 1;
}


dword TAP_EventHandler(word event, dword param1, dword param2)
{
  TRACEENTER();
  static bool           DoNotReenter = FALSE;
//  static dword          SysState, SysSubState;

  TRACEENTER();

  // Behandlung offener MessageBoxen (rekursiver Aufruf, auch bei DoNotReenter)
//  if(CSShowMessageBox)
//  {
    if(OSDMenuMessageBoxIsVisible() || OSDMenuInfoBoxIsVisible())
    {
      if(OSDMenuMessageBoxIsVisible())
      {
//        if(event == EVT_KEY) LastMessageBoxKey = param1;
        #ifdef __ALTEFBLIB__
          OSDMenuMessageBoxDoScrollOver(&event, &param1);
        #endif
      }
      OSDMenuEvent(&event, &param1, &param2);
      param1 = 0;
    }
//    if(!OSDMenuMessageBoxIsVisible() && !OSDMenuInfoBoxIsVisible())
//     CSShowMessageBox = FALSE;
//  }

  // OSDs und TAP-Beendigung dürfen nicht im selben Durchlauf erfolgen, wie HDD_ImExportChData()!
  if (event == EVT_IDLE && CLSFinished && !DoNotReenter)
  {
    DoNotReenter = TRUE;
    if (pMessage)
      ShowErrorMessage(pMessage, NULL);

//    if (SysSubState != 0)
//      TAP_EnterNormalNoInfo();

    if (CLSFinished == 1)
      WriteLogMC(PROGRAM_NAME, "ChannelListSaver Exit.\r\n");
    else
      WriteLogMC(PROGRAM_NAME, "TAP is still running!! (should not occur)");
    CloseLogMC();
    LangUnloadStrings();
    CLSFinished++;
    DoNotReenter = FALSE;
    TAP_Exit();
  }

  TRACEEXIT();
  return param1;
}


// ----------------------------------------------------------------------------
//                              Hilfsfunktionen
// ----------------------------------------------------------------------------
FILE *fLogMC = NULL;

void CreateRootDir(void)
{
  //Check & Create Folders
  HDD_TAP_PushDir();
  HDD_ChangeDir("/ProgramFiles");
  if(!TAP_Hdd_Exist("Settings")) TAP_Hdd_Create("Settings", ATTR_FOLDER);
  HDD_ChangeDir("Settings");
  if(!TAP_Hdd_Exist(PROGRAM_NAME)) TAP_Hdd_Create(PROGRAM_NAME, ATTR_FOLDER);
  HDD_TAP_PopDir();
}

void CloseLogMC(void)
{
  if (fLogMC)
  {
    fclose(fLogMC);

    //As the log would receive the Linux time stamp (01.01.2000), adjust to the PVR's time
    struct utimbuf      times;
    times.actime = PvrTimeToLinux(Now(NULL));
    times.modtime = times.actime;
    utime(TAPFSROOT LOGDIR "/" LOGFILENAME, &times);
  }
  fLogMC = NULL;
}

void WriteLogMC(char *ProgramName, char *Text)
{
  char                 *TS = NULL;
  byte                  Sec;

  TS = TimeFormat(Now(&Sec), Sec, TIMESTAMP_YMDHMS);

  if (!fLogMC)
  {
    fLogMC = fopen(TAPFSROOT LOGDIR "/" LOGFILENAME, "ab");
    setvbuf(fLogMC, NULL, _IOLBF, 4096);  // zeilenweises Buffering
  }
  if (fLogMC)
  {
    fprintf(fLogMC, "%s %s\r\n", TS, Text);
//    close(fLogMC);
  }

//  if (Console)
  {
    TAP_PrintNet("%s %s: %s\n", TS, ((ProgramName && ProgramName[0]) ? ProgramName : ""), Text);
  }
}

void WriteLogMCf(char *ProgramName, const char *format, ...)
{
  char Text[512];

  if(format)
  {
    va_list args;
    va_start(args, format);
    vsnprintf(Text, sizeof(Text), format, args);
    va_end(args);
    WriteLogMC(ProgramName, Text);
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

bool ConvertUTFStr(char *DestStr, char *SourceStr, int MaxLen, bool ToUnicode)
{
  char *TempStr = NULL;
  TRACEENTER();

  TempStr = (char*) TAP_MemAlloc(MaxLen * 2);
  if (TempStr)
  {
    memset(TempStr, 0, sizeof(TempStr));
    if (ToUnicode)
    {
      #ifdef __ALTEFBLIB__
        if (SourceStr[0] < 0x20) SourceStr++;
        StrToUTF8(SourceStr, TempStr);
      #else
        StrToUTF8(SourceStr, TempStr, 9);
      #endif
    }
    else
      StrToISO(SourceStr, TempStr);

    if (!ToUnicode)
    {
      if ((SourceStr[0] >= 0x20) && (strlen(TempStr) < strlen(SourceStr)))
      {
        DestStr[0] = 0x05;
        DestStr++;
        MaxLen--;
      }
      else if (SourceStr[0] >= 0x15)
      {
        DestStr[0] = 0x05;
        DestStr++;
        SourceStr++;
      }
    }

    TempStr[MaxLen-1] = 0;
    if (ToUnicode && ((TempStr[strlen(TempStr)-1] & 0xC0) == 0xC0))
      TempStr[strlen(TempStr)-1] = 0;
    strcpy(DestStr, TempStr);

    TAP_MemFree(TempStr);
    TRACEEXIT();
    return TRUE;
  }
  TRACEEXIT();
  return FALSE;
}

bool HDD_SetFileDateTime(char const *FileName, char const *AbsDirectory, dword NewDateTime)
{
  char                  AbsFileName[FBLIB_DIR_SIZE];
  struct stat64         statbuf;
  struct utimbuf        utimebuf;

  if(FileName && AbsDirectory && (NewDateTime > 0xd0790000))
  {
    TAP_SPrint(AbsFileName, sizeof(AbsFileName), "%s/%s", AbsDirectory, FileName);
    if(lstat64(AbsFileName, &statbuf) == 0)
    {
      utimebuf.actime = statbuf.st_atime;
      utimebuf.modtime = PvrTimeToLinux(NewDateTime);
      utime(AbsFileName, &utimebuf);
      TRACEEXIT();
      return TRUE;
    }
  }
  TRACEEXIT();
  return FALSE;
}

bool HDD_ImExportChData(char *FileName, char *AbsDirectory, bool Import)
{
//  static char           AbsDir2[FBLIB_DIR_SIZE];
  static tDirEntry     *_hddTapFolder = NULL;
  tDirEntry             FolderStruct, OldTapFolder;
  char                  AbsFileName[FBLIB_DIR_SIZE];
//  char                 *RelFileName = NULL;
  bool                  ret = FALSE;

  TRACEENTER();
  HDD_TAP_PushDir();
  WriteLogMCf(PROGRAM_NAME, (Import ? "[Action] Importing '%s' (System)..." : "[Action] Exporting '%s' (System)..."), FileName);
  WriteLogMC(PROGRAM_NAME, "----------------------------------------");

  TAP_SPrint(AbsFileName, sizeof(AbsFileName), "%s/%s", AbsDirectory, FileName);
//  RelFileName = &AbsFileName[21];

  // Create/empty the file, if not exists
  if(!Import)
    fclose(fopen(AbsFileName, "w"));
  
  //Get the current TAP folder variable
  if(!_hddTapFolder)
  {
    _hddTapFolder = (tDirEntry*)FIS_vHddTapFolder();
    if(!_hddTapFolder)
    {
      TRACEEXIT();
      return FALSE;
    }
  }

  //Initialize the directory structure
  memset(&FolderStruct, 0, sizeof(tDirEntry));
  FolderStruct.Magic = 0xbacaed31;

  //Save the current directory resources and change into our directory (current directory of the TAP)
  ApplHdd_SaveWorkFolder();
//  strcpy(AbsDir2, &AbsDirectory[1]);  // must be static! Do not include the leading slash!
  if (!ApplHdd_SelectFolder(&FolderStruct, &AbsDirectory[1]))
  {
    ApplHdd_SetWorkFolder(&FolderStruct);
    memcpy(&OldTapFolder, (void*)_hddTapFolder, sizeof(OldTapFolder));
    memcpy((void*)_hddTapFolder, &FolderStruct, sizeof(FolderStruct));
    if (Import)
      ret = Appl_ImportChData(FileName);
    else
      ret = Appl_ExportChData(FileName);
    memcpy((void*)_hddTapFolder, &OldTapFolder, sizeof(OldTapFolder));
  }
  ApplHdd_RestoreWorkFolder();

  if (ret)
  {
    if (!Import)
      HDD_SetFileDateTime(FileName, AbsDirectory, Now(NULL));
    WriteLogMCf(PROGRAM_NAME, (Import ? "--> Import '%s' successful." : "--> Export '%s' successful."), FileName);
  }
  else
  {
    if(!Import)
      remove(AbsFileName);
    WriteLogMCf(PROGRAM_NAME, (Import ? "--> Error during import '%s'!" : "--> Error during export '%s'!"), FileName);
  }
  HDD_TAP_PopDir();

  TRACEEXIT();
  return ret;
}

/*void DebugServiceNames(char* FileName)
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
}*/


// ----------------------------------------------------------------------------
//                           MessageBox-Funktionen
// ----------------------------------------------------------------------------
#ifdef __ALTEFBLIB__
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
  int ret;

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
//  CSShowMessageBox = TRUE;
  while (OSDMenuMessageBoxIsVisible())
  {
    TAP_SystemProc();
    TAP_Sleep(1);
  }
  ret = 0;
  if ((OSDMenuMessageBoxLastButton() != (dword) -1) && (OSDMenuMessageBoxLastButton() != 2))
    ret = ((int)OSDMenuMessageBoxLastButton()) + 1;

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
//  CSShowMessageBox = TRUE;
  while (OSDMenuMessageBoxIsVisible())
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
  FILE                 *f = NULL;
  char                 *Buffer = NULL;
  size_t                BufSize = 0;
  char                 *c = NULL;
  int                   p;
  char                  Name[50];
  long                  Value = 0;

  TRACEENTER();

  if ((f = fopen(TAPFSROOT LOGDIR "/" INIFILENAME, "rb")) != NULL)
  {
    while (getline(&Buffer, &BufSize, f) >= 0)
    {
      //Interpret the following characters as remarks: //
      c = strstr(Buffer, "//");
      if(c) *c = '\0';

      // Remove line breaks in the end
      p = strlen(Buffer);
      while (p && (Buffer[p-1] == '\r' || Buffer[p-1] == '\n' || Buffer[p-1] == ';'))
        Buffer[--p] = '\0';

      if (sscanf(Buffer, "%49[^= ] = %lu", Name, &Value) == 2)
      {
        if      (strncmp(Name, "ImportFormat",        12) == 0)   ImportFormat         =  minmax(Value,   1,   0,   2);
        else if (strncmp(Name, "OverwriteSatellites", 19) == 0)   OverwriteSatellites  =  minmax(Value,   1,   0,   2);
        else if (strncmp(Name, "RestoreNameLock",     15) == 0)   RestoreNameLock      =  (Value != 0);
      }
    }
    fclose(f);
  }
  else
    SaveINI();

  TRACEEXIT();
}

void SaveINI(void)
{
  FILE *f = NULL;
  TRACEENTER();

  if ((f = fopen(TAPFSROOT LOGDIR "/" INIFILENAME, "wb")) != NULL)
  {
    fprintf(f, "[ChannelListSaver]\r\n");
    fprintf(f, "%s=%d\r\n",  "ImportFormat",        ImportFormat);
    fprintf(f, "%s=%d\r\n",  "OverwriteSatellites", OverwriteSatellites);
    fprintf(f, "%s=%d\r\n",  "RestoreNameLock",     RestoreNameLock  ?  1  :  0);
    fclose(f);
    HDD_SetFileDateTime(INIFILENAME, TAPFSROOT LOGDIR, 0);
  }
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
      WriteLogMCf(PROGRAM_NAME, "Unsupported system type: %d!", CurSystemType);
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
      WriteLogMC(PROGRAM_NAME, "Unsupported favorites structure!");
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

    for (i = 0; i < Result; i++)
    {
      if ((i==0) || (p1[i-1]==0))
      {
        if (p1[i])
        {
          if (NrServiceNames) *NrServiceNames = *NrServiceNames + 1;
        }
        else
        {
          Result = i;
          break;
        }
      }
    }
  }
  TRACEEXIT();
  return Result;
}

int GetLengthOfProvNames(int *NrProviderNames)
{
  TRACEENTER();
  int Result = 0;
  int i = 0;
  tProviderName *p;
  p = (tProviderName*)(FIS_vFlashBlockProviderInfo());

  if(p)
  {
    for (i = 0; i <= NRPROVIDERNAMES; i++)
      if ((i == NRPROVIDERNAMES) || (!p[i].name[0])) break;
  }
  if (NrProviderNames) *NrProviderNames = i;
  Result = i * PROVIDERNAMELENGTH;
  TRACEEXIT();
  return Result;
}

word* GetSETimerID(int TimerNr)
{
  byte                 *p;
  TYPE_Timer_TMSC      *t;
  word                 *ret = NULL;
  TRACEENTER();

  p = (byte*)(FIS_vFlashBlockTimer());
  if(p)
  {
    p += (TimerNr * FlashTimerStructSize());
    t  = (TYPE_Timer_TMSC*) p;
    ret = &(t->unused5);
  }
  TRACEEXIT();
  return ret;
}

tTimerExt* SaveTimers(int *NrOldTimers)
{
  int                   Count, i, j=0;
  tTimerExt            *Timers;
  TRACEENTER();

  Count = TAP_Timer_GetTotalNum();
  Timers = (tTimerExt*) TAP_MemAlloc(Count * sizeof(tTimerExt));
  if (Timers)
  {
    for(i = 0; i < Count; i++)
    {
      // Ermittle TimerInfos
      if (TAP_Timer_GetInfo(i, &Timers[j].TimerInfo))
      {
        // Ermittle ServiceID, SatIndex und TransponderID
        TYPE_Service_TMSx *p = (TYPE_Service_TMSx*) ((Timers[j].TimerInfo.svcType==SVC_TYPE_Tv) ? FIS_vFlashBlockTVServices() : FIS_vFlashBlockRadioServices());
        word           *nSvc = (word*)              ((Timers[j].TimerInfo.svcType==SVC_TYPE_Tv) ? FIS_vnTvSvc() : FIS_vnRadioSvc());
        if (p && nSvc && (Timers[j].TimerInfo.svcNum < *nSvc))
        {
          p += Timers[j].TimerInfo.svcNum;
          Timers[j].ServiceID = p->SVCID;
          Timers[j].Frequency = GetTransponderFreq_TMSx(p->TPIdx);
          Timers[j].SETimerID = *(GetSETimerID(i));
          if (Timers[j].Frequency) j++;
        }
      }
    }
    WriteLogMCf(PROGRAM_NAME, "  %d / %d timers have been saved.", j, Count);
  }
  *NrOldTimers = j;
  TRACEEXIT();
  return Timers;
}

void RestoreTimers(tTimerExt *OldTimers, int NrOldTimers)
{
  int                   Count=0, i, j;
  TRACEENTER();

  if (OldTimers)
  {
    for(i = 0; i < NrOldTimers; i++)
    {
      // Finde Service mit passendem SvcType, SvcID und Frequency
      TYPE_Service_TMSx *p = (TYPE_Service_TMSx*) ((OldTimers[i].TimerInfo.svcType==SVC_TYPE_Tv) ? FIS_vFlashBlockTVServices() : FIS_vFlashBlockRadioServices());
      word           *nSvc = (word*)              ((OldTimers[i].TimerInfo.svcType==SVC_TYPE_Tv) ? FIS_vnTvSvc() : FIS_vnRadioSvc());
      if (p && nSvc)
      {
        for (j = 0; j < *nSvc; j++)
        {
          if ((p[j].SVCID == OldTimers[i].ServiceID) && (GetTransponderFreq_TMSx(p[j].TPIdx) == OldTimers[i].Frequency))
          {
            OldTimers[i].TimerInfo.svcNum = j;
            if (TAP_Timer_Add(&OldTimers[i].TimerInfo) == 0)
            {
              *(GetSETimerID(i)) = OldTimers[Count].SETimerID;
              Count++;
            }
            break;
          }
        }
      }
    }
    WriteLogMCf(PROGRAM_NAME, "  %d / %d timers have been restored.", Count, NrOldTimers);
  }
  TRACEEXIT();
}

bool DeleteTimers(void)
{
  int                   Count = 0;
  bool                  ret = TRUE;
  TRACEENTER();

  while(TAP_Timer_GetTotalNum() > 0)
  {
    ret = TAP_Timer_Delete(0) && ret;
    Count++;
  }
  WriteLogMCf(PROGRAM_NAME, "  %d timers have been deleted.", Count);
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

  WriteLogMC(PROGRAM_NAME, "  DeleteServiceNames()");
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

bool DeleteAllSettings(bool DeleteSatellites)
{
  TRACEENTER();
  bool ret = TRUE;

  WriteLogMCf(PROGRAM_NAME, "Deleting all settings (DeleteSat=%s):", (DeleteSatellites ? "true" : "false"));
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
        memset(p1, 0, min(p2 - p1, SERVICENAMESLENGTH));
    #endif
  }

  {
    // Provider Names
    char                 *p;

    p = (char*)(FIS_vFlashBlockProviderInfo());
    if(ret && p)
      memset(p, 0, GetLengthOfProvNames(NULL));
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
      memset(p, 0, nServices * SIZE_Service_TMSx);
    }

    // Radio Services
    p    = (TYPE_Service_TMSS*)(FIS_vFlashBlockRadioServices());
    nSvc = (word*)FIS_vnRadioSvc();
    if (ret && p && nSvc)
    {
      nServices = *nSvc;
      *nSvc = 0;
      memset(p, 0, nServices * SIZE_Service_TMSx);
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
      if (DeleteSatellites)
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

  if(!ret) WriteLogMC(PROGRAM_NAME, "  Error while deleting old settings!");
  TRACEEXIT();
  return ret;
}
