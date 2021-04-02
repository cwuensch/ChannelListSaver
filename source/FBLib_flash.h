#ifndef FBLIB_FLASH_H
#define FBLIB_FLASH_H

  #include "libFireBird.h"

  #define TIMER_NAME    "FireBirdLib.rec"
  #define TIMER_DATE    0xffff0000          //22.04.2038 00:00
  #define MAXFAVS       30
  #define MAXSAT        255

  typedef struct
  {
    byte                SatIdx;             //0x00
    byte                VideoStreamType;    //0x01

    word                TPIdx:10;           //0x02
    word                TunerNum:2;
    word                SkipFlag:1;
    word                LockFlag:1;
    word                CASFlag:1;
    word                DelFlag:1;

    word                SVCID;              //0x04
    word                PMTPID;             //0x06
    word                PCRPID;             //0x08
    word                VideoPID;           //0x0a
    word                AudioPID:13;        //0x0c
    word                AudioTypeFlag:2;
    word                AudioAutoSelect:1;
    word                NameLock:1;         //0x0e
    word                Flags2:15;

    dword               NameOffset;         //0x10
    word                ProviderIdx;        //0x14
    word                LCN;                //0x16
    byte                unknown2[6];        //0x18
    word                AudioStreamType;    //0x1e
  } TYPE_Service_TMSS;

  #define TYPE_Service_TMST   TYPE_Service_TMSS
  #define TYPE_Service_TMSC   TYPE_Service_TMSS

  bool FlashServiceDecode(void *Data, tFlashService *Service);
  bool FlashServiceDelete(void *Data);
  bool FlashServiceDelServiceName(int SvcType, int SvcNum);
  bool FlashServiceEncode(void *Data, tFlashService *Service);

  typedef tFlashLNB       TYPE_LNB_TMSS;
  typedef tFlashSatTable  TYPE_SatInfo_TMSS;

  typedef struct
  {
    word                  NrOfTransponders;
    char                  SatName[MAX_SatName];      //Contains DVBT
  } TYPE_SatInfo_TMST;

  typedef struct
  {
    word                  NrOfTransponders;
  } TYPE_SatInfo_TMSC;

  typedef struct
  {
    byte                SatIdx;

    word                Polar:1;              // 0=V, 1=H
    word                TPMode:3;             // TPMode is either 000 for "normal" or 001 for "SmaTV" ("SmaTV" not seen in the wild)
    word                ModulationSystem:1;   // 0=DVBS, 1=DVBS2
    word                ModulationType:2;     // 0=Auto, 1=QPSK, 2=8PSK, 3=16QAM
    word                FECMode:4;            // 0x0 = AUTO, 0x1 = 1_2, 0x2 = 2_3, 0x3 = 3_4,
                                              // 0x4 = 5_6 , 0x5 = 7_8, 0x6 = 8_9, 0x7 = 3_5,
                                              // 0x8 = 4_5, 0x9 = 9_10, 0xa = reserved, 0xf = NO_CONV
    word                Pilot:1;
    word                unused2:4;

    byte                unused3;
    dword               Frequency;
    word                SymbolRate;
    word                TSID;

    word                AllowTimeSync:1;
    word                unused4:15;

    word                OriginalNetworkID;
  }__attribute__((packed)) TYPE_TpInfo_TMSS;

  typedef struct
  {
    byte                SatIdx;
    byte                ChannelNr;
    byte                Bandwidth;
    byte                unused1;
    dword               Frequency;
    word                TSID;
    byte                LPHP;
    byte                unused2;
    word                OriginalNetworkID;
    word                NetworkID;
  }__attribute__((packed)) TYPE_TpInfo_TMST;

  typedef struct
  {
    union {
      dword             Frequency;
      struct {
        dword           SatIdx:8;
        dword           Frequency2:24;
      };
    };
    word                SymbolRate;
    word                TSID;
    word                OriginalNetworkID;
    byte                ModulationType;
    byte                unused1;
  }__attribute__((packed)) TYPE_TpInfo_TMSC;

  typedef struct
  {
    word                TunerIndex:8;       //0000
    word                RecMode:3;
    word                DemuxPath:2;
    word                ManualRec:1;
    word                unused1:2;          //uumd drrr tttt tttt

    byte                SatIndex;           //0002

    byte                ServiceType:1;      //0003
    byte                ReservationType:3;
    byte                unused2:4;          //uuuu rrrt

    word                ServiceID;          //0004
    word                Duration;           //0006
    byte                unused3;            //0008
    char                FileName[131];      //0009
    dword               StartTime;          //008C
    dword               EndTime;            //0090
    word                PMTPID;             //0094
    byte                isRec;              //0096
    byte                NameSet;            //0097
    byte                unused4;            //0098
    byte                EPGMarker;          //0099
    word                unused5;            //009a
    dword               unknown1;           //009c
    dword               EventID1;           //00a0
    dword               EventID2;           //00a4
    word                ServiceIndex;       //00a8
    byte                unused8[8];         //00aa
    byte                IceTV;              //00b2
    byte                unused9[5];         //00b3  // Padding?
  }__attribute__((packed)) TYPE_Timer_generic; //184 bytes

  typedef struct
  {
    TYPE_Timer_generic  TimerInfo;   //184 bytes
    TYPE_TpInfo_TMSS    TpInfo;      // 32 bytes  // (Alex: 100 bytes?)
  }__attribute__((packed)) TYPE_Timer_TMSS; //216 bytes

  typedef struct
  {
    TYPE_Timer_generic  TimerInfo;   //184 bytes
    byte                unused10[8];
    TYPE_TpInfo_TMST    TpInfo;      // 16 bytes
  }__attribute__((packed)) TYPE_Timer_TMST; //208 bytes

  typedef struct
  {
    TYPE_Timer_generic  TimerInfo;   //184 bytes
    TYPE_TpInfo_TMST    TpInfo;      // 16 bytes
  }__attribute__((packed)) TYPE_Timer_TMST200; //200 bytes

  typedef struct
  {
    TYPE_Timer_generic  TimerInfo;
    byte                unused10[8];
    dword               rs_timestamp1;
    char                rs_episodeCRID[64];
    char                rs_seriesCRID[64];
    dword               rs_unknown1;
    dword               rs_unknown2;
    dword               rs_timestamp2;
    dword               rs_unknown3;
    dword               rs_unknown4;
    TYPE_TpInfo_TMST    TpInfo;
  }__attribute__((packed)) TYPE_Timer_TMST360;      //DMC 2015-11-06 360 byte structure for the 5300

  typedef struct
  {
    TYPE_Timer_generic  TimerInfo;   //184 bytes
    TYPE_TpInfo_TMSC    TpInfo;      // 12 bytes
  }__attribute__((packed)) TYPE_Timer_TMSC; //196 bytes

  void FlashReindexTimers(int SvcType, int FromSvcNum, int ToSvcNum);

  typedef struct
  {
    dword               StartTime;
    dword               EndTime;
    word                Duration;

    word                ReservationType:3;  //see TYPE_ReservationType
    word                unknown1:4;
    word                DelOrig:1;
    word                unknown2:1;         //always 1?
    word                NrOfFiles:6;
    word                unknown3:1;
    dword               AutomaticMode;
    char                FileName[50][128];
  } TYPE_AutoDescrambleTimer;

  void FlashReindexFavorites(int SvcType, int FromSvcNum, int ToSvcNum);

  typedef struct
  {
    char                GroupName[12];
    word                SvcNum[50];
    byte                SvcType[50];
    byte                NrEntries;
    byte                unused1;
  } tFavorites1050;

#endif
