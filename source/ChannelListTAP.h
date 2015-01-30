#ifndef __CHANNELLISTTAPH__
#define __CHANNELLISTTAPH__

#define PROGRAM_NAME          "ChannelListSaver"
#define VERSION               "V0.3"
#define TAPID                 0x8E0A4271
#define AUTHOR                "chris86"
#define DESCRIPTION           "Import/Export of Sat, Transponder, Service, Favorites lists"

#define LOGDIR                "/ProgramFiles/Settings/ChannelListSaver"
#define LNGFILENAME           PROGRAM_NAME ".lng"
#define INIFILENAME           PROGRAM_NAME ".ini"


#define EXPORTFILENAME        "ProgramFiles/Channels"
#define CRLF                  "\r\n"

#define PROVIDERNAMELENGTH    21
#define NRPROVIDERNAMES       256
#define SERVICENAMESLENGTH    39996    // 40000 / 40004 / 39996 ***  ?


int   TAP_Main(void);
dword TAP_EventHandler(word event, dword param1, dword param2);


#endif
