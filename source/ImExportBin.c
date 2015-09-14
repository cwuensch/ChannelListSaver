#define _LARGEFILE_SOURCE
#define _LARGEFILE64_SOURCE
#define __USE_LARGEFILE64  1
#define _FILE_OFFSET_BITS  64
#ifdef _MSC_VER
  #define __const const
#endif

//#define  STACKTRACE     TRUE
#define _GNU_SOURCE
#include                <string.h>
#include                <stdio.h>
#include                <tap.h>
#include                <libFireBird.h>
#include                "../../../../../Topfield/FireBirdLib/flash/FBLib_flash.h"
#include                "ChannelListSaver.h"
#include                "ImExportBin.h"


byte GetTransponderSatIndex_TMSx(TYPE_TpInfo_TMSx *pTpInfo)
{  
  return *((byte*) pTpInfo);
}
word GetNrTranspOfSat_TMSx(TYPE_SatInfo_TMSx *pSatInfo)
{  
  return *((word*) pSatInfo);
}
word* GetpNrTranspOfSat_TMSx(TYPE_SatInfo_TMSx *pSatInfo)
{  
  return ((word*) pSatInfo);
}


bool ExportSettings(char *FileName, char *AbsDirectory)
{
  tExportHeader         FileHeader;
  FILE                 *fExportFile = NULL;
  char                  AbsFileName[FBLIB_DIR_SIZE];
  int                   i;
  bool                  ret = FALSE;

  TRACEENTER();
  WriteLogCS(PROGRAM_NAME, "[Action] Exporting settings (binary)...");
  WriteLogCS(PROGRAM_NAME, "----------------------------------------");

  TAP_SPrint(AbsFileName, sizeof(AbsFileName), "%s/%s", AbsDirectory, FileName);
  fExportFile = fopen(AbsFileName, "wb");
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
      // [Satellites]
      TYPE_SatInfo_TMSS *p;
      FileHeader.SatellitesOffset = ftell(fExportFile);
      p = (TYPE_SatInfo_TMSS*)FIS_vFlashBlockSatInfo();
      if(p)
      {
        FileHeader.NrSatellites = FlashSatTablesGetTotal();
        ret = fwrite(&FileHeader.NrSatellites, sizeof(FileHeader.NrSatellites), 1, fExportFile) && ret;
        FileHeader.SatellitesOffset = ftell(fExportFile);
        if (FileHeader.NrSatellites > 0)
          ret = (fwrite(p, SIZE_SatInfo_TMSx, FileHeader.NrSatellites, fExportFile) == (dword)FileHeader.NrSatellites) && ret;
      }
    }
    WriteLogCSf(PROGRAM_NAME, (ret) ? "%5d Satellites exported." : "Satellites error (%d)!", FileHeader.NrSatellites);

    {
      // [Transponders]
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
          ret = (fwrite(p, SIZE_TpInfo_TMSx, FileHeader.NrTransponders, fExportFile) == (dword)FileHeader.NrTransponders) && ret;
      }
    }
    WriteLogCSf(PROGRAM_NAME, (ret) ? "%5d Transponders exported." : "Transponders error (%d)!", FileHeader.NrTransponders);

    {
      // [TVServices]
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
          ret = (fwrite(p, SIZE_Service_TMSx, FileHeader.NrTVServices, fExportFile) == (dword)FileHeader.NrTVServices) && ret;
      }
    }
    WriteLogCSf(PROGRAM_NAME, (ret) ? "%5d TVServices exported." : "TVServices error (%d)!", FileHeader.NrTVServices);

    {
      // [RadioServices]
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
          ret = (fwrite(p, SIZE_Service_TMSx, FileHeader.NrRadioServices, fExportFile) == (dword)FileHeader.NrRadioServices) && ret;
      }
    }
    WriteLogCSf(PROGRAM_NAME, (ret) ? "%5d RadioServices exported." : "RadioServices error (%d)!", FileHeader.NrRadioServices);

    {
      // [Favorites]
      tFavorites FavGroup;
      FileHeader.NrFavGroups = FlashFavoritesGetTotal();
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
    WriteLogCSf(PROGRAM_NAME, (ret) ? "%5d Favorite-Groups exported." : "Favorites error (%d)!", FileHeader.NrFavGroups);

    {
      char *p1, *p2;
      p1 = (char*)(FIS_vFlashBlockServiceName());
      p2 = (char*)(FIS_vFlashBlockProviderInfo());
      int NrProviderNames, NrServiceNames;

      // [ProviderNames]
      FileHeader.ProviderNamesLength = GetLengthOfProvNames(&NrProviderNames);
      ret = fwrite(&NrProviderNames, sizeof(NrProviderNames), 1, fExportFile) && ret;
      FileHeader.ProviderNamesOffset = ftell(fExportFile);
      if(p2)
      {
        ret = (fwrite(p2, 1, FileHeader.ProviderNamesLength, fExportFile) == (dword)FileHeader.ProviderNamesLength) && ret;
      }
      WriteLogCSf(PROGRAM_NAME, (ret) ? "%5d ProviderNames exported (%d Bytes)." : "ProviderNames error (%d, %d Bytes)!", NrProviderNames, FileHeader.ProviderNamesLength);

      // [ServiceNames]
      FileHeader.ServiceNamesLength = GetLengthOfServiceNames(&NrServiceNames);
      ret = fwrite(&NrServiceNames, sizeof(NrServiceNames), 1, fExportFile) && ret;
      FileHeader.ServiceNamesOffset = ftell(fExportFile);
      if(p1)
      {
        ret = (fwrite(p1, 1, FileHeader.ServiceNamesLength, fExportFile) == (dword)FileHeader.ServiceNamesLength) && ret;
      }
      WriteLogCSf(PROGRAM_NAME, (ret) ? "%5d ServiceNames exported (%d Bytes)." : "ServiceNames error (%d, %d Bytes)!", NrServiceNames, FileHeader.ServiceNamesLength);
    }

    FileHeader.FileSize = ftell(fExportFile);
    fclose(fExportFile);

    // Write FileSize into header
    fExportFile = fopen(AbsFileName, "r+b");
    if(fExportFile)
    {
      ret = fwrite(&FileHeader, sizeof(tExportHeader), 1, fExportFile) && ret;
      fclose(fExportFile);
      HDD_SetFileDateTime(&AbsFileName[1], "", Now(NULL));
    }
  }
  else
    WriteLogCS(PROGRAM_NAME, "  File not found!");

  if (ret)
    WriteLogCSf(PROGRAM_NAME, "--> Export '%s' (binary) successful.", FileName);
  else
  {
    if(TAP_Hdd_Exist(FileName)) TAP_Hdd_Delete(FileName);
    WriteLogCSf(PROGRAM_NAME, "--> Error during export '%s'.", FileName);
  }
  TRACEEXIT();
  return ret;
}


bool ImportSettings(char *FileName, char *AbsDirectory, int OverwriteSatellites, bool RestoreNameLock)  // 0: nie, 1: auto, 2: immer
{
  tExportHeader         FileHeader;
  FILE                 *fImportFile = NULL;
  char                 *Buffer = NULL;
  char                  AbsFileName[FBLIB_DIR_SIZE];
  unsigned long         fs;
  int                   NrImpSatellites=0, NrImpTransponders=0, NrImpTVServices=0, NrImpRadioServices=0, NrImpFavGroups=0;
  bool                  ret = FALSE;

  TRACEENTER();
  WriteLogCS(PROGRAM_NAME, "[Action] Importing settings (binary)...");
  WriteLogCS(PROGRAM_NAME, "----------------------------------------");

  TAP_SPrint(AbsFileName, sizeof(AbsFileName), "%s/%s", AbsDirectory, FileName);
  fImportFile = fopen(AbsFileName, "rb");
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
          // [Satellites]
          byte *s;
          s = (byte*)FIS_vFlashBlockSatInfo();
          if (s)
          {
            if (OverwriteSatellites != 2)  // auto oder nie
            {
              tFlashSatTable CurSat, IstSat;
              int i = 0;

              NrImpSatellites = FlashSatTablesGetTotal();
              for (i = 0; i < FileHeader.NrSatellites; i++)
              {
                FlashSatTablesDecode(Buffer + FileHeader.SatellitesOffset + i * SIZE_SatInfo_TMSx, &CurSat);
                if (i < NrImpSatellites)
                {
                  FlashSatTablesDecode(s + i * SIZE_SatInfo_TMSx, &IstSat);
                  if ((IstSat.SatPosition != CurSat.SatPosition) || (strcmp(IstSat.SatName, CurSat.SatName) != 0))
                  {
                    WriteLogCSf(PROGRAM_NAME, "  Warning: Satellite nr. %d does not match! (Import: '%s', Receiver: '%s')", i, CurSat.SatName, IstSat.SatName);
                    if (OverwriteSatellites == 1)
                    {
                      WriteLogCS(PROGRAM_NAME, "  --> Will overwrite satellites...");
                      OverwriteSatellites = 2;
//                      break;
                    }
                  }
                }
                else
                {
                  WriteLogCSf(PROGRAM_NAME, "  Warning: Satellite nr. %d ('%s') not found in receiver!", i, CurSat.SatName);
                  if (OverwriteSatellites)
                  {
                    if (OverwriteSatellites == 1)
                      WriteLogCS(PROGRAM_NAME, "  --> Will overwrite satellites...");
                    OverwriteSatellites = 2;
//                    break;
                  }
                  else ret = FALSE;
                }
              }
            }
          }
          else
            ret = FALSE;

          if (ret)
          {
            ret = ret && DeleteAllSettings(OverwriteSatellites == 2);
            WriteLogCS(PROGRAM_NAME, "Importing settings:");

            if (ret && (OverwriteSatellites == 2))
            {
              int i;
              byte *newSatellites;
              newSatellites = (byte*)Buffer + FileHeader.SatellitesOffset;
              for (i = 0; i < FileHeader.NrSatellites; i++)
              {
                word* NrTranspondersOfSat = GetpNrTranspOfSat_TMSx((TYPE_SatInfo_TMSx*)(newSatellites + i * SIZE_SatInfo_TMSx));
                *NrTranspondersOfSat      = 0;
              }
              memcpy(s, newSatellites, FileHeader.NrSatellites * SIZE_SatInfo_TMSx);
//              memset(s + FileHeader.NrSatellites * SIZE_SatInfo_TMSx, 0, SIZE_SatInfo_TMSx);
              NrImpSatellites = FileHeader.NrSatellites;
            }
            WriteLogCSf(PROGRAM_NAME, (ret) ? "%5d / %-5d Satellites imported." : "  Satellites error (%d / %d)!", ((OverwriteSatellites==2) ? NrImpSatellites : 0), FileHeader.NrSatellites);
          }

          if (ret)
          {
            // [Transponders]
            TYPE_TpInfo_TMSS *p;
            dword            *NrTransponders;
          
            p = (TYPE_TpInfo_TMSS*)(FIS_vFlashBlockTransponderInfo());
            NrTransponders = (dword*)(p) - 1;
            if (p)
            {
//              TAP_PrintNet("NrTransponders = %lu \n", *NrTransponders);
              memcpy(p, Buffer + FileHeader.TranspondersOffset, FileHeader.NrTransponders * SIZE_TpInfo_TMSx);
              *NrTransponders = FileHeader.NrTransponders;

              // Wenn ohne SAT:
              // nicht die Satelliten importieren, sondern hier die TransponderNr hochzählen:
//              if (OverwriteSatellites != 2)
//              {
                int i;
                for (i = 0; i < FileHeader.NrTransponders; i++)
                {
                  byte SatIdx                 = GetTransponderSatIndex_TMSx((TYPE_TpInfo_TMSx*)(Buffer + FileHeader.TranspondersOffset + i * SIZE_TpInfo_TMSx));
                  if (SatIdx < NrImpSatellites)
                  {
                    word* NrTranspondersOfSat = GetpNrTranspOfSat_TMSx((TYPE_SatInfo_TMSx*)(s + SatIdx * SIZE_SatInfo_TMSx));
                    *NrTranspondersOfSat      = *NrTranspondersOfSat + 1;
                  }
                  else
                  {
                    WriteLogCSf(PROGRAM_NAME, "  Error: Invalid sat index in transponder %d.", i);
                    *NrTransponders = i;
                    ret = FALSE;
                    break;
                  }
                }
//              }
              NrImpTransponders = *NrTransponders;
            }
            else
              ret = FALSE;
            WriteLogCSf(PROGRAM_NAME, (ret) ? "%5d / %-5d Transponders imported." : "  Transponders error (%d / %d)!", NrImpTransponders, FileHeader.NrTransponders);
          }

          // [Services]
          int j = 0;
          for (j = 0; ret && (j <= 1); j++)
          {
            char*                 (*Appl_AddSvcName)(char const*);
            word                  (*Appl_SetProviderName)(char const*);
            TYPE_Service_TMSx      *p;
            word                   *nSvc;
            char                    SvcName[MAX_SvcName + 1], *pSvcName;
            int                     i;

            Appl_AddSvcName       = (void*)FIS_fwAppl_AddSvcName();
            Appl_SetProviderName  = (void*)FIS_fwAppl_SetProviderName();

            p    = (TYPE_Service_TMSx*) ((j==0) ? FIS_vFlashBlockTVServices() : FIS_vFlashBlockRadioServices());
            nSvc =              (word*) ((j==0) ? FIS_vnTvSvc()               : FIS_vnRadioSvc());

            char* SvcNameBuf = Buffer + FileHeader.ServiceNamesOffset;
            char* PrvNameBuf = Buffer + FileHeader.ProviderNamesOffset;

            if (p && nSvc)
            {
//              memcpy(p, Buffer + FileHeader.TVServicesOffset, FileHeader.NrTVServices * SIZE_Service_TMSx);
//              *nSvc = FileHeader.NrTVServices;
//              TAP_PrintNet("NrTVServices = %lu \n", *nSvc);

              TYPE_Service_TMSx* newServices;
              newServices = (TYPE_Service_TMSx*) (Buffer + ((j==0) ? FileHeader.TVServicesOffset : FileHeader.RadioServicesOffset));
              for (i = 0; i < ((j==0) ? FileHeader.NrTVServices : FileHeader.NrRadioServices); i++)
              {
                if ((newServices[i].SatIdx < NrImpSatellites) && (newServices[i].TPIdx < GetNrTranspOfSat_TMSx((TYPE_SatInfo_TMSx*)(s + newServices[i].SatIdx * SIZE_SatInfo_TMSx))))
                {
//                  *nSvc = (word)(i+1);
                  if (Appl_AddSvcName)
                  {
                    if (newServices[i].NameOffset < (dword)FileHeader.ServiceNamesLength)
                    {
                      pSvcName = &SvcNameBuf[newServices[i].NameOffset];
                      if (FileHeader.UTF8System != isUTFToppy())
                        if (ConvertUTFStr(SvcName, pSvcName, MAX_SvcName+1, !FileHeader.UTF8System))
                          pSvcName = SvcName;
                      newServices[i].NameOffset = (dword)Appl_AddSvcName(pSvcName);
                    }
                    else
                      newServices[i].NameOffset = (dword)Appl_AddSvcName("* No Name *");
                  }
                  else
                    ret = FALSE;
                  if (Appl_SetProviderName)
                  {
                    if (newServices[i].ProviderIdx * PROVIDERNAMELENGTH < FileHeader.ProviderNamesLength)
                      newServices[i].ProviderIdx = Appl_SetProviderName(&PrvNameBuf[newServices[i].ProviderIdx * PROVIDERNAMELENGTH]);
                    else
                      newServices[i].ProviderIdx = Appl_SetProviderName("");
                  }
                  else
                    ret = FALSE;
                  if (!RestoreNameLock)
                    newServices[i].NameLock = 0;
                  memcpy(&p[*nSvc], &newServices[i], SIZE_Service_TMSx);
                  *nSvc = *nSvc + 1;
                }
                else
                {
                  WriteLogCSf(PROGRAM_NAME, "  Warning: Invalid sat (%d) or transponder (%d) index for %sService %d! (NrImpSats=%d, NrTps[%d]=%d)", newServices[i].SatIdx, newServices[i].TPIdx, ((j==0) ? "TV" : "Radio"), i, NrImpSatellites, newServices[i].SatIdx, GetNrTranspOfSat_TMSx((TYPE_SatInfo_TMSx*)(s + newServices[i].SatIdx * SIZE_SatInfo_TMSx)));
                  ret = FALSE;
                }
              }
//              memcpy(p, newServices, i * SIZE_Service_TMSx);
//              *nSvc = (word)(i);
              if (j==0)
                NrImpTVServices = *nSvc;
              else
                NrImpRadioServices = *nSvc;
//              TAP_PrintNet("NrServices = %u \n", *nSvc);
            }
            else
              ret = FALSE;

            if (j==0)
              WriteLogCSf(PROGRAM_NAME, (ret) ? "%5d / %-5d TVServices imported." : "  TVServices error (%d / %d)!", NrImpTVServices, FileHeader.NrTVServices);
            else
              WriteLogCSf(PROGRAM_NAME, (ret) ? "%5d / %-5d RadioServices imported." : "  RadioServices error (%d / %d)!", NrImpRadioServices, FileHeader.NrRadioServices);
          }

          if (ret)
          {
            // [Favorites]
            tFavorites           *FavGroups;
            tFavorites           *CurFavGroup;
            int                   i;

            FavGroups = (tFavorites*) (Buffer + FileHeader.FavoritesOffset);

            for (i = 0; i < FileHeader.NrFavGroups; i++)
            {
              CurFavGroup = &FavGroups[i];
              if (CurFavGroup->GroupName[0])
              {
/*                int j, k;
                for (j = 0; j < CurFavGroup->NrEntries; j++)
                {
                  if (((CurFavGroup->SvcType[j]==SVC_TYPE_Tv) && (CurFavGroup->SvcNum[j] >= NrImpTVServices)) || ((CurFavGroup->SvcType[j]==SVC_TYPE_Radio) && (CurFavGroup->SvcNum[j] >= NrImpRadioServices)))
                  {
                    CurFavGroup->NrEntries--;
                    for (k = j; k < CurFavGroup->NrEntries; k++)
                    {
                      CurFavGroup->SvcNum[k]  = CurFavGroup->SvcNum[k+1];
                      CurFavGroup->SvcType[k] = CurFavGroup->SvcType[k+1];
                    }
                  }
                }  */
#ifdef FULLDEBUG
  TAP_PrintNet("FavGroup %d: Name = '%s', Entries = %d\n", i, CurFavGroup->GroupName, CurFavGroup->NrEntries);
#endif
                if (FlashFavoritesSetInfo(NrImpFavGroups, &FavGroups[i]))
                  NrImpFavGroups++;
                else
                  ret = FALSE;
              }
            }
            WriteLogCSf(PROGRAM_NAME, (ret) ? "%5d / %-5d Favorite-Groups imported." : "  Favorites error (%d / %d)!", NrImpFavGroups, FileHeader.NrFavGroups);
          }
        }

        TAP_MemFree(Buffer);
      }
      else
        WriteLogCS(PROGRAM_NAME, "  Not enough memory!");
    }
    else
      WriteLogCS(PROGRAM_NAME, "  Invalid header format!");
    fclose(fImportFile);
  }
  else
    WriteLogCS (PROGRAM_NAME, "  File not found!");
//  if(ret)
//    FlashProgram();

  if (ret)
    WriteLogCSf(PROGRAM_NAME, "--> Import '%s' (binary) successful.", FileName);
  else
    WriteLogCSf(PROGRAM_NAME, "--> Error during import '%s'.", FileName);

  TRACEEXIT();
  return ret;
}
