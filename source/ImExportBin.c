#define _FILE_OFFSET_BITS  64
#define __USE_LARGEFILE64  1
#ifdef _MSC_VER
  #define __const const
#endif

#define _GNU_SOURCE
#include                <string.h>
#include                <stdio.h>
#include                <tap.h>
#include                <libFireBird.h>
#include                "../../../../../Topfield/FireBirdLib/flash/FBLib_flash.h"
#include                "ChannelListTAP.h"
#include                "ImExportBin.h"


byte GetTransponderSatIndex_TMSx(TYPE_TpInfo_TMSx *pTpInfo)
{  
  return *((byte*) pTpInfo);
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
    WriteLogCSf(PROGRAM_NAME, (ret) ? "%d Satellites exportiert." : "Satellites Fehler!", FileHeader.NrSatellites);

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
    WriteLogCSf(PROGRAM_NAME, (ret) ? "%d Transponders exportiert." : "Transponders Fehler!", FileHeader.NrTransponders);

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
    WriteLogCSf(PROGRAM_NAME, (ret) ? "%d TVServices exportiert." : "TVServices Fehler!", FileHeader.NrTVServices);

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
    WriteLogCSf(PROGRAM_NAME, (ret) ? "%d RadioServices exportiert." : "RadioServices Fehler!", FileHeader.NrRadioServices);

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
    WriteLogCSf(PROGRAM_NAME, (ret) ? "%d Favourite-Groups exportiert." : "Favourites Fehler!", FileHeader.NrFavGroups);

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
      WriteLogCSf(PROGRAM_NAME, (ret) ? "%d ProviderNames exportiert." : "ProviderNames Fehler!", NRPROVIDERNAMES);

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
      WriteLogCS(PROGRAM_NAME, (ret) ? "ServiceNames exportiert." : "ServiceNames Fehler!");
    }

    FileHeader.FileSize = ftell(fExportFile);
    fclose(fExportFile);

    fExportFile = fopen(AbsFileName, "r+b");
    if(fExportFile)
    {
      ret = fwrite(&FileHeader, sizeof(tExportHeader), 1, fExportFile) && ret;
      fclose(fExportFile);
    }
  }
  else
    WriteLogCS(PROGRAM_NAME, "Datei nicht gefunden.");

  if (ret)
    WriteLogCSf(PROGRAM_NAME, "Export '%s' (binär) erfolgreich.", FileName);
  else
  {
    if(TAP_Hdd_Exist(FileName)) TAP_Hdd_Delete(FileName);
    WriteLogCSf(PROGRAM_NAME, "Export '%s' fehlgeschlagen.", FileName);
  }
  TRACEEXIT();
  return ret;
}


bool ImportSettings(char *FileName, char *AbsDirectory, bool OverwriteSatellites)
{
  tExportHeader         FileHeader;
  FILE                 *fImportFile = NULL;
  char                 *Buffer = NULL;
  char                  AbsFileName[FBLIB_DIR_SIZE];
  unsigned long         fs;
  int                   NrImpSatellites=0, NrImpTransponders=0, NrImpTVServices=0, NrImpRadioServices=0, NrImpFavGroups=0;
  int                   i, j, k;
  bool                  ret = FALSE;

  TRACEENTER();

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
          if (OverwriteSatellites)
          {
            TYPE_SatInfo_TMSS *p;

            p = (TYPE_SatInfo_TMSS*)FIS_vFlashBlockSatInfo();
            if (ret && p)
            {
              memcpy(p, Buffer + FileHeader.SatellitesOffset, FileHeader.NrSatellites * SIZE_SatInfo_TMSx);
              NrImpSatellites = FileHeader.NrSatellites;
            }
            else
              ret = FALSE;
          }
          WriteLogCSf(PROGRAM_NAME, (ret) ? "%d / %d Satellites importiert." : "Satellites Fehler!", NrImpSatellites, FileHeader.NrSatellites);
          if (!OverwriteSatellites)
            NrImpSatellites = FlashSatTablesGetTotal();

          {
            TYPE_TpInfo_TMSS *p;
            byte             *s;
            dword            *NrTransponders;
          
            p = (TYPE_TpInfo_TMSS*)(FIS_vFlashBlockTransponderInfo());
            NrTransponders = (dword*)(p) - 1;
            if (ret && p)
            {
              TAP_PrintNet("NrTransponders = %lu \n", *NrTransponders);
              memcpy(p, Buffer + FileHeader.TranspondersOffset, FileHeader.NrTransponders * SIZE_TpInfo_TMSx);
              *NrTransponders = FileHeader.NrTransponders;

              // Wenn ohne SAT:
              // nicht die Satelliten importieren, sondern hier die TransponderNr hochzählen:
              if (!OverwriteSatellites)
              {
                s = (byte*)FIS_vFlashBlockSatInfo();
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
                    *NrTransponders = i;
                    break;
                  }
                }
              }
              NrImpTransponders = *NrTransponders;
            }
            else
              ret = FALSE;
            WriteLogCSf(PROGRAM_NAME, (ret) ? "%d / %d Transponders importiert." : "Transponders Fehler!", NrImpTransponders, FileHeader.NrTransponders);
          }

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
                if ((pServices[i].SatIdx < NrImpSatellites) && (pServices[i].TPIdx < NrImpTransponders))
                {
//                  *nSvc = (word)(i+1);
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
                      pServices[i].ProviderIdx = Appl_SetProviderName("");
                  }
                  else
                    ret = FALSE;
                  pServices[i].NameLock = 0;
                  memcpy(&p[*nSvc], &pServices[i], SIZE_Service_TMSx);
                  *nSvc = *nSvc + 1;
                }
              }
//              memcpy(p, pServices, i * SIZE_Service_TMSx);
//              *nSvc = (word)(i);
              if (j==0)
                NrImpTVServices = *nSvc;
              else
                NrImpRadioServices = *nSvc;
              TAP_PrintNet("NrServices = %u \n", *nSvc);
            }
            else
              ret = FALSE;

            if (j==0)
              WriteLogCSf(PROGRAM_NAME, (ret) ? "%d / %d TVServices importiert." : "TVServices Fehler", NrImpTVServices, FileHeader.NrTVServices);
            else
              WriteLogCSf(PROGRAM_NAME, (ret) ? "%d / %d RadioServices importiert." : "RadioServices Fehler", NrImpRadioServices, FileHeader.NrRadioServices);
          }

          {
            tFavorites           *FavGroups;
            tFavorites           *CurFavGroup;
            char                 *p;

            p = (char*) FIS_vFlashBlockFavoriteGroup();
            FavGroups = (tFavorites*) (Buffer + FileHeader.FavoritesOffset);

            for (i = 0; i < FileHeader.NrFavGroups; i++)
            {
              if (ret)
              {
                CurFavGroup = &FavGroups[i];
                if (CurFavGroup->GroupName[0])
                {
                  for (j = 0; j < CurFavGroup->NrEntries; j++)
                  {
                    if (((CurFavGroup->SvcType[j]==SVC_TYPE_Tv) && (CurFavGroup->SvcNum[j] > NrImpTVServices)) || ((CurFavGroup->SvcType[j]!=SVC_TYPE_Tv) && (CurFavGroup->SvcNum[j] > NrImpRadioServices)))
                    {
                      CurFavGroup->NrEntries--;
                      for (k = j; k < CurFavGroup->NrEntries; k++)
                      {
                        CurFavGroup->SvcNum[k]  = CurFavGroup->SvcNum[k+1];
                        CurFavGroup->SvcType[k] = CurFavGroup->SvcType[k+1];
                      }
                    }
                  }
                  TAP_PrintNet("FavGroup %d: Name = '%s', Entries = %d\n", i, CurFavGroup->GroupName, CurFavGroup->NrEntries);
                  ret = ret && FlashFavoritesSetInfo(i, &FavGroups[i]);
                  if (ret) NrImpFavGroups++;
                }
              }
            }
          }
          WriteLogCSf(PROGRAM_NAME, (ret) ? "%d / %d Favourite-Groups importiert." : "Favourites Fehler!", NrImpFavGroups, FileHeader.NrFavGroups);
        }

        TAP_MemFree(Buffer);
      }
      else
        WriteLogCS(PROGRAM_NAME, "Nicht genung Speicher!");
    }
    else
      WriteLogCS(PROGRAM_NAME, "Header passt nicht!");
    fclose(fImportFile);
  }
  else
    WriteLogCS (PROGRAM_NAME, "Datei nicht gefunden!");
//  if(ret)
//    FlashProgram();

  if (ret)
    WriteLogCSf(PROGRAM_NAME, "Import '%s' (binär) erfolgreich.", FileName);
  else
    WriteLogCSf(PROGRAM_NAME, "Import '%s' fehlgeschlagen.", FileName);

  TRACEEXIT();
  return ret;
}
