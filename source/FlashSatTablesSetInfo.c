#include                <string.h>
#include                "../../../../../Topfield/FireBirdLib/flash/FBLib_flash.h"
#include                "FlashSatTablesSetInfo.h"


bool FlashSatTablesEncode_ST_TMSS(TYPE_SatInfo_TMSS *Data, tFlashSatTable *SatTable, bool ResetNrTransponders)
{
  TRACEENTER();
  int i;

  if(!Data || !SatTable)
  {
    TRACEEXIT();
    return FALSE;
  }

  SatTable->NrOfTransponders = (ResetNrTransponders) ? 0 : Data->NrOfTransponders;

  memset(Data, 0, sizeof(TYPE_SatInfo_TMSS));
  Data->NrOfTransponders       = SatTable->NrOfTransponders;
  Data->unused1                = SatTable->unused1;
  Data->SatPosition            = SatTable->SatPosition;
  memcpy(Data->unknown1,         SatTable->unknown1, 22);
  memcpy(Data->unused2,          SatTable->unused2, 8);
  strncpy(Data->SatName,         SatTable->SatName, 15);

  //LNBs
  for (i = 0; i <= 1; i++)
  {
    Data->LNB[i].LNBSupply     = SatTable->LNB[i].LNBSupply;
    Data->LNB[i].unused1       = SatTable->LNB[i].unused1;
    Data->LNB[i].DiSEqC10      = SatTable->LNB[i].DiSEqC10;
    Data->LNB[i].unused2       = SatTable->LNB[i].unused2;
    Data->LNB[i].DiSEqC12      = SatTable->LNB[i].DiSEqC12;
    Data->LNB[i].UniversalLNB  = SatTable->LNB[i].UniversalLNB;
    Data->LNB[i].Switch22      = SatTable->LNB[i].Switch22;
    Data->LNB[i].LowBand       = SatTable->LNB[i].LowBand;
    Data->LNB[i].HBFrq         = SatTable->LNB[i].HBFrq;
    Data->LNB[i].unused3       = SatTable->LNB[i].unused3;
    Data->LNB[i].LoopThrough   = SatTable->LNB[i].LoopThrough;
    Data->LNB[i].unused4       = SatTable->LNB[i].unused4;
    Data->LNB[i].DiSEqC11      = SatTable->LNB[i].DiSEqC11;
    memcpy(Data->LNB[i].DiSEqC12Flags, SatTable->LNB[i].DiSEqC12Flags, 3);
    memcpy(Data->LNB[i].unused5,       SatTable->LNB[i].unused5, 5);
  }

  TRACEEXIT();
  return TRUE;
}

bool FlashSatTablesEncode_ST_TMST(TYPE_SatInfo_TMST *Data, tFlashSatTable *SatTable, bool ResetNrTransponders)
{
  TRACEENTER();

  if(!Data || !SatTable)
  {
    TRACEEXIT();
    return FALSE;
  }

  strncpy(Data->SatName, SatTable->SatName, 15);
  Data->SatName[15] = '\0';
  if (ResetNrTransponders)
    Data->NrOfTransponders = 0;

  TRACEEXIT();
  return TRUE;
}

bool FlashSatTablesEncode_ST_TMSC(TYPE_SatInfo_TMSC *Data, tFlashSatTable *SatTable, bool ResetNrTransponders)
{
  TRACEENTER();

  if(!Data || !SatTable)
  {
    TRACEEXIT();
    return FALSE;
  }

  if (ResetNrTransponders)
    Data->NrOfTransponders = 0;

  TRACEEXIT();
  return TRUE;
}


bool FlashSatTablesEncode(void *Data, tFlashSatTable *SatTable, bool ResetNrTransponders)
{
  bool ret;

  TRACEENTER();

  if(!Data || !SatTable)
  {
    TRACEEXIT();
    return FALSE;
  }

  ret = FALSE;
  switch(GetSystemType())
  {
    //Unknown and old 5k/6k systems are not supported
    case ST_UNKNOWN:
    case ST_S:
    case ST_ST:
    case ST_T:
    case ST_C:
    case ST_CT:
    case ST_T5700:
    case ST_T5800:
    case ST_TF7k7HDPVR: break;

    case ST_TMSS: ret = FlashSatTablesEncode_ST_TMSS(Data, SatTable, ResetNrTransponders); break;
    case ST_TMST: ret = FlashSatTablesEncode_ST_TMST(Data, SatTable, ResetNrTransponders); break;
    case ST_TMSC: ret = FlashSatTablesEncode_ST_TMSC(Data, SatTable, ResetNrTransponders); break;

    case ST_NRTYPES: break;
  }

  TRACEEXIT();
  return ret;
}


bool FlashSatTablesSetInfo(int SatNum, tFlashSatTable *SatTable)
{
  TRACEENTER();

  bool ret;
  int SatAnz = FlashSatTablesGetTotal();

  //SatNum out of range
  if((SatNum < 0) || (SatNum > SatAnz))
  {
    TRACEEXIT();
    return FALSE;
  }

  //SatTable is NULL
  if(!SatTable)
  {
    TRACEEXIT();
    return FALSE;
  }

  ret = FALSE;
  switch(GetSystemType())
  {
    //Unknown and old 5k/6k systems are not supported
    case ST_UNKNOWN:
    case ST_S:
    case ST_ST:
    case ST_T:
    case ST_C:
    case ST_CT:
    case ST_T5700:
    case ST_T5800:
    case ST_TF7k7HDPVR: break;

    case ST_TMSS:
    {
      TYPE_SatInfo_TMSS *p;

      p = (TYPE_SatInfo_TMSS*)FIS_vFlashBlockSatInfo();
      if(p)
      {
        p = p + SatNum;
        ret = FlashSatTablesEncode(p, SatTable, (SatNum == SatAnz));
      }
      break;
    }

    case ST_TMST:
    {
      TYPE_SatInfo_TMST *p;

      p = (TYPE_SatInfo_TMST*)FIS_vFlashBlockSatInfo();
      if(p) ret = FlashSatTablesEncode(p, SatTable, (SatNum == SatAnz));
      break;
    }

    case ST_TMSC:
    {
      TYPE_SatInfo_TMSC *p;

      p = (TYPE_SatInfo_TMSC*)FIS_vFlashBlockSatInfo();
      if(p) ret = FlashSatTablesEncode(p, SatTable, (SatNum == SatAnz));
      break;
    }

    case ST_NRTYPES: break;
  }

  TRACEEXIT();
  return ret;
}
