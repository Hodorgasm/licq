#ifndef ICQPACKET_H
#define ICQPACKET_H

#include "licq_user.h"
#include "licq_buffer.h"
#include "licq_socket.h"
#include "licq_icq.h"

class CICQColor;


unsigned short ReversePort(unsigned short p);


//=====Packet===================================================================

class CPacket
{
public:
   CBuffer *getBuffer()  { return buffer; };
   virtual CBuffer *Finalize(INetSocket *) { return NULL; }

//   CPacket& operator=(const CPacket &);
//   CPacket& operator+=(const CPacket &);
//   friend CPacket operator+(const CPacket &, const CPacket &);

   virtual const unsigned long  Sequence() = 0;
   virtual const unsigned short SubSequence() = 0;
   virtual const unsigned short Command() = 0;
   virtual const unsigned short SubCommand() = 0;

   static void SetMode(char c) { s_nMode = c; }
   static char Mode()  { return s_nMode; }
   static void SetLocalIp(unsigned long n)  {  s_nLocalIp = n; }
   static void SetLocalPort(unsigned short n)  {  s_nLocalPort = n; }
   static void SetRealIp(unsigned long n)  {  s_nRealIp = n; }
   static void SetIps(INetSocket *s);
   static bool Firewall() { return s_nLocalIp != s_nRealIp; }
   static unsigned long RealIp() { return s_nRealIp; }
   static unsigned long LocalIp() { return s_nLocalIp; }

protected:
   CBuffer *buffer;
   unsigned short m_nSize;

   static unsigned long s_nLocalIp;
   static unsigned long s_nRealIp;
   static unsigned short s_nLocalPort;
   static char s_nMode;
};

//=====ServerTCP===============================================================

//-----SrvPacketTcp------------------------------------------------------------

class CSrvPacketTcp : public CPacket
{
public:
  virtual ~CSrvPacketTcp();

  virtual const unsigned long  Sequence() { return m_nSequence; }
  virtual const unsigned short SubSequence() { return m_nSubSequence; }

    // do we need this ?
  virtual const unsigned short Command()  { return m_nChannel; }
  virtual const unsigned short SubCommand()  { return 0; }

  virtual CBuffer *Finalize(INetSocket *);

protected:
  CSrvPacketTcp(unsigned char);
  void InitBuffer();

  static bool s_bRegistered;
  static unsigned short s_nSequence;
  static unsigned short s_nSubSequence;

  unsigned char m_nChannel;
  unsigned short m_nSequence;
  unsigned short m_nSubSequence;

  char *m_szSequenceOffset;
};

//=====UDP======================================================================

//-----PacketUdp----------------------------------------------------------------
class CPacketUdp : public CPacket
{
public:
   virtual ~CPacketUdp();

   virtual CBuffer *Finalize(INetSocket *);
   virtual const unsigned long  Sequence() { return m_nSequence; }
   virtual const unsigned short SubSequence() { return m_nSubSequence; }
   virtual const unsigned short Command()  { return m_nCommand; }
   virtual const unsigned short SubCommand()  { return 0; }
protected:
   CPacketUdp(unsigned short _nCommand);
   void InitBuffer();

   static bool s_bRegistered;

#if ICQ_VERSION == 2
   unsigned short m_nVersion;
   unsigned short m_nCommand;
   unsigned short m_nSequence;
   unsigned short m_nSubSequence;
#elif ICQ_VERSION == 4
   unsigned short m_nVersion;
   unsigned short m_nRandom;
   unsigned short m_nZero;
   unsigned short m_nCommand;
   unsigned short m_nSequence;
   unsigned short m_nSubSequence;
   unsigned long  m_nCheckSum;
#else
   unsigned short m_nVersion;
   unsigned long  m_nZero;
   unsigned long  m_nSessionId;
   unsigned short m_nCommand;
   unsigned short m_nSequence;
   unsigned short m_nSubSequence;
   unsigned long  m_nCheckSum;
#endif

   static unsigned short s_nSequence;
   static unsigned short s_nSubSequence;
   static unsigned long  s_nSessionId;
};



#if ICQ_VERSION == 2 || ICQ_VERSION > 6
//-----Register----------------------------------------------------------------
// Doesn't actually descend from CPacketUdp in version 2 but we keep the
// name the same for simplicity
class CPU_Register : public CPacket
{
public:
  CPU_Register(const char *_szPasswd);
  virtual ~CPU_Register();

  virtual const unsigned long  Sequence() { return m_nSequence; }
  virtual const unsigned short SubSequence() { return 0; }
  virtual const unsigned short Command()  { return m_nCommand; }
  virtual const unsigned short SubCommand() { return 0; }
protected:
  virtual unsigned long getSize()	{ return 1; }

  /* 02 00 FC 03 01 00 02 00 04 00 65 66 67 00 72 00 00 00 00 00 00 00 */
  unsigned short m_nVersion;
  unsigned short m_nCommand;
  unsigned short m_nSequence;
  unsigned short m_nUnknown1;
  unsigned short m_nPasswdLen;
  char          *m_szPasswd;
  unsigned long  m_nUnknown2;
  unsigned long  m_nUnknown3;
};
#elif ICQ_VERSION == 4 || ICQ_VERSION == 5
//-----Register----------------------------------------------------------------
class CPU_Register : public CPacketUdp
{
public:
  CPU_Register(const char *_szPasswd);
};
#endif

//-----Logon--------------------------------------------------------------------
class CPU_Logon : public CSrvPacketTcp
{
public:
  CPU_Logon(const char *_szPassword, const char *_szUin, unsigned short _nLogonStatus);
protected:
  unsigned long  m_nLogonStatus;
  unsigned long  m_nTcpVersion;
};

//-----Logoff-------------------------------------------------------------------
class CPU_Logoff : public CSrvPacketTcp
{
public:
   CPU_Logoff();
};

//-----SendCookie---------------------------------------------------------------
class CPU_SendCookie : public CSrvPacketTcp
{
public:
  CPU_SendCookie(const char *, int len);
  virtual ~CPU_SendCookie();

protected:
  char *m_szCookie;
};



//-----CommonFamily----------------------------------------------------------
class CPU_CommonFamily : public CSrvPacketTcp
{
public:
  CPU_CommonFamily(unsigned short Family, unsigned short SubType);

protected:
  void InitBuffer();

private:
    unsigned short m_nFamily;
    unsigned short m_nSubType;
};

class CPU_GenericFamily : public CPU_CommonFamily
{
public:
    CPU_GenericFamily(unsigned short Family, unsigned short SubType);

};

//-----ImICQ------------------------------------------------------------------
class CPU_ImICQ : public CPU_CommonFamily
{
public:
    CPU_ImICQ();
};

//-----ICQMode------------------------------------------------------------------
class CPU_ICQMode : public CPU_CommonFamily
{
public:
    CPU_ICQMode();
};

//-----CapabilitySettings-------------------------------------------------------------
class CPU_CapabilitySettings : public CPU_CommonFamily
{
public:
  CPU_CapabilitySettings();
};

//-----RateAck-----------------------------------------------------------------
class CPU_RateAck : public CPU_CommonFamily
{
public:
  CPU_RateAck();
};

//-----GenericUinList------------------------------------------------------------
class CPU_GenericUinList : public CPU_CommonFamily
{
public:
   CPU_GenericUinList(UinList &uins, unsigned short Family, unsigned short Subtype);
   CPU_GenericUinList(unsigned long _nUin, unsigned short Family, unsigned short Subtype);
};

//-----SetStatus----------------------------------------------------------------
class CPU_SetStatus : public CPU_CommonFamily
{
public:
   CPU_SetStatus(unsigned long _nNewStatus);

private:
   unsigned long m_nNewStatus;
};

class CPU_SetLogonStatus : public CPU_CommonFamily
{
public:
   CPU_SetLogonStatus(unsigned long _nNewStatus);

private:
   unsigned long m_nNewStatus;
};


//-----ClientReady--------------------------------------------------------------
class CPU_ClientReady : public CPU_CommonFamily
{
public:
   CPU_ClientReady();
};

//-----ClientAckNameInfo--------------------------------------------------------
class CPU_AckNameInfo : public CPU_CommonFamily
{
public:
   CPU_AckNameInfo();
};

//-----RequestSysMsg------------------------------------------------------------
class CPU_RequestSysMsg : public CPU_CommonFamily
{
public:
   CPU_RequestSysMsg();
};

//-----SysMsgDoneAck------------------------------------------------------------
class CPU_SysMsgDoneAck : public CPU_CommonFamily
{
public:
  CPU_SysMsgDoneAck(unsigned short nId);
};


//-----Ack---------------------------------------------------------------------
class CPU_Ack : public CPacketUdp
{
public:
#if ICQ_VERSION == 2
   CPU_Ack(unsigned short _nSequence);
#else
   CPU_Ack(unsigned short _nSequence, unsigned short _nSubSequence);
#endif
};


//-----AddUser------------------------------------------------------------------
class CPU_AddUser : public CPacketUdp
{
public:
   CPU_AddUser(unsigned long _nAddedUin);
protected:
   /* 02 00 3C 05 06 00 50 A5 82 00 8F 76 20 00 */
   unsigned long  m_nAddedUin;
};


//-----SearchByInfo--------------------------------------------------------------
class CPU_SearchByInfo : public CPU_CommonFamily
{
public:
   CPU_SearchByInfo(const char *_sAlias, const char *_sFirstName,
                    const char *_sLastName, const char *_sEmail);
   virtual const unsigned short SubCommand()   { return m_nMetaCommand; }
   unsigned long Uin()  { return 0; }
protected:
   unsigned long m_nMetaCommand;
};


//-----SearchByUin--------------------------------------------------------------
class CPU_SearchByUin : public CPacketUdp
{
public:
   CPU_SearchByUin(unsigned long nUin);
};


//-----SearchWhitePages---------------------------------------------------------
class CPU_SearchWhitePages : public CPU_CommonFamily
{
public:
   CPU_SearchWhitePages(const char *szFirstName, const char *szLastName,
                   const char *szAlias, const char *szEmail,
                   unsigned short nMinAge, unsigned short nMaxAge,
                   char nGender, char nLanguage, const char *szCity,
                   const char *szState, unsigned short nCountryCode,
                   const char *szCoName, const char *szCoDept,
                   const char *szCoPos, bool bOnlineOnly);
	  virtual const unsigned short SubCommand()		{ return m_nMetaCommand; }
	  unsigned long Uin()  { return 0; }
protected:
		unsigned long m_nMetaCommand;
};


//-----UpdatePersonalBasicInfo--------------------------------------------------
class CPU_UpdatePersonalBasicInfo : public CPacketUdp
{
public:
   CPU_UpdatePersonalBasicInfo(const char *_sAlias, const char *_sFirstName,
                               const char *_sLastName, const char *_sEmail,
                               bool _bAuthorization);

   const char *Alias()  { return m_szAlias; }
   const char *FirstName()  { return m_szFirstName; }
   const char *LastName()  { return m_szLastName; }
   const char *Email()  { return m_szEmail; }
   bool Authorization()  { return m_nAuthorization == 0; }
protected:
   char *m_szAlias;
   char *m_szFirstName;
   char *m_szLastName;
   char *m_szEmail;
   char m_nAuthorization;
};


//-----UpdatePersonalExtInfo-------------------------------------------------------
class CPU_UpdatePersonalExtInfo : public CPacketUdp
{
public:
   CPU_UpdatePersonalExtInfo(const char *_sCity, unsigned short _nCountry,
                             const char *_sState, unsigned short _nAge,
                             char _cSex, const char *_sPhone,
                             const char *_sHomepage, const char *_sAbout,
                             unsigned long _nZipcode);

   const char *City()  { return m_szCity; }
   unsigned short Country()  { return m_nCountry; }
   const char *State()  { return m_szState; }
   unsigned short Age()  { return m_nAge; }
   char Sex()  { return m_cSex; }
   const char *PhoneNumber()  { return m_szPhone; }
   const char *Homepage()  { return m_szHomepage; }
   const char *About()  { return m_szAbout; }
   unsigned long Zipcode() { return m_nZipcode; }
protected:
   char           *m_szCity;
   unsigned short m_nCountry;
   char           m_cTimezone;
   char           *m_szState;
   unsigned short m_nAge;
   char           m_cSex;
   char           *m_szPhone;
   char           *m_szHomepage;
   char           *m_szAbout;
   unsigned long  m_nZipcode;
};


//-----Ping---------------------------------------------------------------------
class CPU_Ping : public CSrvPacketTcp
{
public:
   CPU_Ping();
};


//-----ThroughServer------------------------------------------------------------
class CPU_ThroughServer : public CPU_CommonFamily
{
public:
   CPU_ThroughServer(unsigned long _nDestinationUin, unsigned char format, char *_sMessage);
protected:
   unsigned long  m_nDestinationUin;
};

class CPU_AckThroughServer : public CPU_CommonFamily
{
public:
    CPU_AckThroughServer(unsigned long Uin, unsigned long timestamp1, unsigned long timestamp2,
                         unsigned short Cookie, unsigned char msgType, unsigned char msgFlags,
                         unsigned short len);

};

//-----ReverseTCPRequest--------------------------------------------------------

class CPU_ReverseTCPRequest : public CPacketUdp
{
public:
   CPU_ReverseTCPRequest(unsigned long _nDestinationUin, unsigned long _nIP,
                         unsigned short _nPort, unsigned short _nPort2);
protected:
   unsigned long  m_nDestinationUin;
};

//-----Authorize----------------------------------------------------------------
class CPU_Authorize : public CPacketUdp
{
public:
   CPU_Authorize(unsigned long _nAuthorizeUin);
protected:
   /* 02 00 56 04 05 00 50 A5 82 00 A7 B8 19 00 08 00 01 00 00 */
   unsigned long m_nAuthorizeUin;
};




//-----SetRandomChatGroup----------------------------------------------------
class CPU_SetRandomChatGroup : public CPacketUdp
{
public:
  CPU_SetRandomChatGroup(unsigned long nGroup);
  unsigned long Group() { return m_nGroup; }
protected:
  unsigned long m_nGroup;
};



//-----RandomChatSearch----------------------------------------------------
class CPU_RandomChatSearch : public CPacketUdp
{
public:
  CPU_RandomChatSearch(unsigned long nGroup);
protected:
  unsigned long m_nGroup;
};



//-----Meta_SetGeneralInfo------------------------------------------------------
class CPU_Meta_SetGeneralInfo : public CPacketUdp
{
public:
  CPU_Meta_SetGeneralInfo(const char *szAlias,
                          const char *szFirstName,
                          const char *szLastName,
                          const char *szEmailPrimary,
                          const char *szEmailSecondary,
                          const char *szEmailOld,
                          const char *szCity,
                          const char *szState,
                          const char *szPhoneNumber,
                          const char *szFaxNumber,
                          const char *szAddress,
                          const char *szCellularNumber,
                          const char *szZipCode,
                          unsigned short nCountryCode,
                          bool bHideEmail);
  virtual const unsigned short SubCommand()  { return m_nMetaCommand; }
protected:
  unsigned short m_nMetaCommand;

  char *m_szAlias;
  char *m_szFirstName;
  char *m_szLastName;
  char *m_szEmailPrimary;
  char *m_szEmailSecondary;
  char *m_szEmailOld;
  char *m_szCity;
  char *m_szState;
  char *m_szPhoneNumber;
  char *m_szFaxNumber;
  char *m_szAddress;
  char *m_szCellularNumber;
  char *m_szZipCode;
  unsigned short m_nCountryCode;
  char m_nTimezone;
  char m_nHideEmail;

friend class CICQDaemon;
};


//-----Meta_SetMoreInfo------------------------------------------------------
class CPU_Meta_SetMoreInfo : public CPacketUdp
{
public:
  CPU_Meta_SetMoreInfo(unsigned short nAge,
                       char nGender,
                       const char *szHomepage,
                       unsigned short nBirthYear,
                       char nBirthMonth,
                       char nBirthDay,
                       char nLanguage1,
                       char nLanguage2,
                       char nLanguage3);
  virtual const unsigned short SubCommand()  { return m_nMetaCommand; }
protected:
  unsigned short m_nMetaCommand;

  unsigned short m_nAge;
  char m_nGender;
  char *m_szHomepage;
  unsigned short m_nBirthYear;
  char m_nBirthMonth;
  char m_nBirthDay;
  char m_nLanguage1;
  char m_nLanguage2;
  char m_nLanguage3;

friend class CICQDaemon;
};


//-----Meta_SetWorkInfo------------------------------------------------------
class CPU_Meta_SetWorkInfo : public CPacketUdp
{
public:
  CPU_Meta_SetWorkInfo(const char *szCity,
                       const char *szState,
                       const char *szPhoneNumber,
                       const char *szFaxNumber,
                       const char *szAddress,
                       const char *szName,
                       const char *szDepartment,
                       const char *szPosition,
                       const char *szHomepage);
  virtual const unsigned short SubCommand()  { return m_nMetaCommand; }
protected:
  unsigned short m_nMetaCommand;

  char *m_szCity;
  char *m_szState;
  char *m_szPhoneNumber;
  char *m_szFaxNumber;
  char *m_szAddress;
  char *m_szName;
  char *m_szDepartment;
  char *m_szPosition;
  char *m_szHomepage;

friend class CICQDaemon;
};


//-----Meta_SetAbout---------------------------------------------------------
class CPU_Meta_SetAbout : public CPacketUdp
{
public:
  CPU_Meta_SetAbout(const char *szAbout);
  ~CPU_Meta_SetAbout();
  virtual const unsigned short SubCommand()  { return m_nMetaCommand; }
protected:
  unsigned short m_nMetaCommand;

  char *m_szAbout;

friend class CICQDaemon;
};


//-----SetPassword---------------------------------------------------------
class CPU_SetPassword : public CPU_CommonFamily
{
public:
  CPU_SetPassword(const char *szPassword);
  virtual const unsigned short SubCommand()  { return m_nMetaCommand; }
  unsigned long Uin()	{ return 0; }
protected:
  unsigned short m_nMetaCommand;
	
	char *m_szPassword;
	
friend class CICQDaemon;
};


//-----Meta_SetSecurityInfo--------------------------------------------------
class CPU_Meta_SetSecurityInfo : public CPU_CommonFamily
{
public:
  CPU_Meta_SetSecurityInfo(bool bAuthorization,
                           bool bHideIp,
                           bool bWebAware);
  virtual const unsigned short SubCommand()  { return m_nMetaCommand; }

  bool Authorization()  { return m_nAuthorization == 0; }
  bool HideIp()         { return m_nHideIp == 1; }
  bool WebAware()       { return m_nWebAware == 1; }
protected:
  unsigned short m_nMetaCommand;
  char m_nAuthorization;
  char m_nHideIp;
  char m_nWebAware;
};


//-----Meta_RequestAllInfo------------------------------------------------------
class CPU_Meta_RequestAllInfo : public CPU_CommonFamily
{
public:
  CPU_Meta_RequestAllInfo(unsigned long _nUin);
  virtual const unsigned short SubCommand()  { return m_nMetaCommand; }
  unsigned long Uin()  {  return m_nUin; }
protected:
  unsigned short m_nMetaCommand;
  unsigned long  m_nUin;
};


//-----Meta_RequestBasicInfo------------------------------------------------------
class CPU_Meta_RequestBasicInfo : public CPU_CommonFamily
{
public:
  CPU_Meta_RequestBasicInfo(unsigned long _nUin);
  virtual const unsigned short SubCommand()  { return m_nMetaCommand; }
  unsigned long Uin()  {  return m_nUin; }
protected:
  unsigned short m_nMetaCommand;
  unsigned long  m_nUin;
};


//=====TCP======================================================================
bool Decrypt_Client(CBuffer *pkt, unsigned long version);
void Encrypt_Client(CBuffer *pkt, unsigned long version);

//-----PacketTcp_Handshake------------------------------------------------------
class CPacketTcp_Handshake : public CPacket
{
public:
  virtual const unsigned long  Sequence()   { return 0; }
  virtual const unsigned short SubSequence()   { return 0; }
  virtual const unsigned short Command()    { return ICQ_CMDxTCP_HANDSHAKE; }
  virtual const unsigned short SubCommand() { return 0; }
};

//-----PacketTcp_Handshake------------------------------------------------------
/* FF 03 00 00 00 3D 62 00 00 50 A5 82 00 CF 60 AD 95 CF 60 AD 95 04 3D 62
   00 00 */
class CPacketTcp_Handshake_v2 : public CPacketTcp_Handshake
{
public:
  CPacketTcp_Handshake_v2(unsigned long _nLocalPort);
protected:
  unsigned long  m_nLocalPort;
  unsigned long  m_nLocalHost;
};


class CPacketTcp_Handshake_v4 : public CPacketTcp_Handshake
{
public:
  CPacketTcp_Handshake_v4(unsigned long _nLocalPort);
protected:
  unsigned long  m_nLocalPort;
  unsigned long  m_nLocalHost;
};


class CPacketTcp_Handshake_v6 : public CPacketTcp_Handshake
{
public:
  CPacketTcp_Handshake_v6(unsigned long nDestinationUin,
     unsigned long nSessionId, unsigned short nLocalPort = 0);
  CPacketTcp_Handshake_v6(CBuffer *);

  char Handshake() { return m_nHandshake; }
  unsigned short VersionMajor() { return m_nVersionMajor; }
  unsigned short VersionMinor() { return m_nVersionMinor; }
  unsigned long DestinationUin() { return m_nDestinationUin; }
  unsigned short LocalPort() { return m_nLocalPort; }
  unsigned long SourceUin() { return m_nSourceUin; }
  unsigned long LocalIp()  { return m_nLocalIp; }
  unsigned long RealIp()  { return m_nRealIp; }
  char Mode()  { return m_nMode; }
  unsigned long SessionId() { return m_nSessionId; }

protected:
  char m_nHandshake;
  unsigned short m_nVersionMajor;
  unsigned short m_nVersionMinor;
  unsigned long m_nDestinationUin;
  unsigned long m_nSourceUin;
  unsigned short m_nLocalPort;
  unsigned long m_nLocalIp;
  unsigned long m_nRealIp;
  char m_nMode;
  unsigned long m_nSessionId;
};

class CPacketTcp_Handshake_v7 : public CPacketTcp_Handshake
{
public:
  CPacketTcp_Handshake_v7(unsigned long nDestinationUin,
     unsigned long nSessionId, unsigned short nLocalPort = 0);
  CPacketTcp_Handshake_v7(CBuffer *);

  char Handshake() { return m_nHandshake; }
  unsigned short VersionMajor() { return m_nVersionMajor; }
  unsigned short VersionMinor() { return m_nVersionMinor; }
  unsigned long DestinationUin() { return m_nDestinationUin; }
  unsigned short LocalPort() { return m_nLocalPort; }
  unsigned long SourceUin() { return m_nSourceUin; }
  unsigned long LocalIp()  { return m_nLocalIp; }
  unsigned long RealIp()  { return m_nRealIp; }
  char Mode()  { return m_nMode; }
  unsigned long SessionId() { return m_nSessionId; }

protected:
  char m_nHandshake;
  unsigned short m_nVersionMajor;
  unsigned short m_nVersionMinor;
  unsigned long m_nDestinationUin;
  unsigned long m_nSourceUin;
  unsigned short m_nLocalPort;
  unsigned long m_nLocalIp;
  unsigned long m_nRealIp;
  char m_nMode;
  unsigned long m_nSessionId;
};


class CPacketTcp_Handshake_Ack : public CPacketTcp_Handshake
{
public:
  CPacketTcp_Handshake_Ack();
};



//-----CPacketTcp---------------------------------------------------------------
class CPacketTcp : public CPacket
{
public:
   virtual ~CPacketTcp();

   virtual CBuffer *Finalize(INetSocket *);
   virtual const unsigned long  Sequence()   { return m_nSequence; }
   virtual const unsigned short SubSequence()   { return 0; }
   virtual const unsigned short Command()    { return m_nCommand; }
   virtual const unsigned short SubCommand() { return m_nSubCommand; }

   char *LocalPortOffset()  {  return m_szLocalPortOffset; }
   unsigned short Level()  { return m_nLevel; }
   unsigned short Version()  { return m_nVersion; }
protected:
   CPacketTcp(unsigned long _nCommand, unsigned short _nSubCommand,
      const char *szMessage, bool _bAccept, unsigned short nLevel, ICQUser *_cUser);
   void InitBuffer();
   void PostBuffer();
   void InitBuffer_v2();
   void PostBuffer_v2();
   void InitBuffer_v4();
   void PostBuffer_v4();
   void InitBuffer_v6();
   void PostBuffer_v6();

   unsigned long  m_nSourceUin;
   unsigned long  m_nCommand;
   unsigned short m_nSubCommand;
   char          *m_szMessage;
   unsigned long  m_nLocalPort;
   unsigned short m_nStatus;
   unsigned short m_nMsgType;
   unsigned long  m_nSequence;

   char *m_szLocalPortOffset;
   unsigned short m_nLevel;
   unsigned short m_nVersion;
};


//-----Message------------------------------------------------------------------
class CPT_Message : public CPacketTcp
{
public:
   CPT_Message(char *_sMessage, unsigned short nLevel, bool bMR,
    CICQColor *pColor, ICQUser *pUser);
};


//-----Url----------------------------------------------------------------------
/* BA 95 47 00 03 00 EE 07 00 00 BA 95 47 00 04 00 24 00 67 6F 6F 64 20 70 6F
   72 6E 20 73 69 74 65 FE 68 74 74 70 3A 2F 2F 63 6F 6F 6C 70 6F 72 74 6E 2E
   63 6F 6D 00 81 61 1D 9E 7F 00 00 01 3F 07 00 00 04 00 00 10 00 03 00 00 00 */
class CPT_Url : public CPacketTcp
{
public:
   CPT_Url(char *_sMessage, unsigned short nLevel, bool bMR,
    CICQColor *pColor, ICQUser *pUser);
};


class CPT_ContactList : public CPacketTcp
{
public:
   CPT_ContactList(char *szMessage, unsigned short nLevel, bool bMR,
    CICQColor *pColor, ICQUser *pUser);
};




//-----ReadAwayMessage----------------------------------------------------------
class CPT_ReadAwayMessage : public CPacketTcp
{
public:
   CPT_ReadAwayMessage(ICQUser *_cUser);
   /* 76 1E 3F 00 03 00 EE 07 00 00 76 1E 3F 00 E8 03 01 00 00 81 61 1D 9D 81 61
      1D 9D C9 05 00 00 04 00 00 10 00 FE FF FF FF */
};


//-----ChatRequest--------------------------------------------------------------
  /* 50 A5 82 00 03 00 EE 07 00 00 50 A5 82 00 02 00 0D 00 63 68 61 74 20 72
     65 71 75 65 73 74 00 CF 60 AD D3 CF 60 AD D3 28 12 00 00 04 00 00 10 00
     01 00 00 00 00 00 00 00 00 00 00 06 00 00 00 */
class CPT_ChatRequest : public CPacketTcp
{
public:
  CPT_ChatRequest(char *_sMessage, const char *szChatUsers, unsigned short nPort,
     unsigned short nLevel, ICQUser *pUser);
};


//-----FileTransfer-------------------------------------------------------------
class CPT_FileTransfer : public CPacketTcp
{
public:
   CPT_FileTransfer(const char *_szFilename, const char *_szDescription,
      unsigned short nLevel, ICQUser *pUser);
   bool IsValid()  { return m_bValid; };
   const char *GetFilename()  { return m_szFilename; };
   const char *GetDescription()  { return m_szMessage; };
   unsigned long GetFileSize()  { return m_nFileSize; };
protected:
   bool m_bValid;

   /* 50 A5 82 00 03 00 EE 07 00 00 50 A5 82 00 03 00 0F 00 74 68 69 73 20 69
      73 20 61 20 66 69 6C 65 00 CF 60 AD D3 CF 60 AD D3 60 12 00 00 04 00 00
      10 00 00 00 00 00 09 00 4D 61 6B 65 66 69 6C 65 00 55 0C 00 00 00 00 00
      00 04 00 00 00 */
   char           *m_szFilename;
   unsigned long  m_nFileSize;
};


//-----OpenSecureChannel------------------------------------------------------------
class CPT_OpenSecureChannel : public CPacketTcp
{
public:
  CPT_OpenSecureChannel(ICQUser *pUser);
};


class CPT_CloseSecureChannel : public CPacketTcp
{
public:
  CPT_CloseSecureChannel(ICQUser *pUser);
};




//++++Ack++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/* 50 A5 82 00 03 00 DA 07 00 00 50 A5 82 00 01 00 01 00 00 CF 60 AD D3 CF
   60 AD D3 60 12 00 00 04 00 00 00 00 02 00 00 00 */
class CPT_Ack : public CPacketTcp
{
protected:
  CPT_Ack(unsigned short _nSubCommand, unsigned long _nSequence,
     bool _bAccept, bool _bUrgent, ICQUser *_cUser);
};


//-----AckGeneral------------------------------------------------------------
class CPT_AckGeneral : public CPT_Ack
{
public:
  CPT_AckGeneral(unsigned short nSubCommand, unsigned long nSequence,
     bool bAccept, bool bUrgent, ICQUser *pUser);
};


//-----AckKey------------------------------------------------------------
class CPT_AckOldSecureChannel : public CPT_Ack
{
public:
  CPT_AckOldSecureChannel(unsigned long nSequence, ICQUser *pUser);
};


//-----AckKey------------------------------------------------------------
class CPT_AckOpenSecureChannel : public CPT_Ack
{
public:
  CPT_AckOpenSecureChannel(unsigned long nSequence, bool ok, ICQUser *pUser);
};


//-----AckKey------------------------------------------------------------
class CPT_AckCloseSecureChannel : public CPT_Ack
{
public:
  CPT_AckCloseSecureChannel(unsigned long nSequence, ICQUser *pUser);
};


#if 0
//-----AckMessage------------------------------------------------------------
class CPT_AckMessage : public CPT_Ack
{
public:
  CPT_AckMessage(unsigned long _nSequence, bool _bAccept, bool _bUrgent, ICQUser *_cUser);
};


//-----AckReadAwayMessage-------------------------------------------------------
class CPT_AckReadAwayMessage : public CPT_Ack
{
public:
   CPT_AckReadAwayMessage(unsigned short _nSubCommand, unsigned long _nSequence,
                          bool _bAccept, ICQUser *_cUser);
};


//-----AckUrl-------------------------------------------------------------------
class CPT_AckUrl : public CPT_Ack
{
public:
  CPT_AckUrl(unsigned long _nSequence, bool _bAccept, bool _bUrgent, ICQUser *_cUser);
};


//-----AckContactList----------------------------------------------------------
class CPT_AckContactList : public CPT_Ack
{
public:
  CPT_AckContactList(unsigned long _nSequence, bool _bAccept, bool _bUrgent,
                     ICQUser *_cUser);
};
#endif

//-----AckChatRefuse------------------------------------------------------------
class CPT_AckChatRefuse : public CPT_Ack
{
public:
  CPT_AckChatRefuse(const char *_sReason, unsigned long _nSequence, ICQUser *_cUser);
   /* 50 A5 82 00 03 00 DA 07 00 00 50 A5 82 00 02 00 03 00 6E 6F 00 CF 60 AD
      95 CF 60 AD 95 1E 3C 00 00 04 01 00 00 00 01 00 00 00 00 00 00 00 00 00
      00 01 00 00 00 */
};


//-----AckChatAccept------------------------------------------------------------
class CPT_AckChatAccept : public CPT_Ack
{
public:
  CPT_AckChatAccept(unsigned short _nPort, unsigned long _nSequence,
                    ICQUser *_cUser);
   /* 50 A5 82 00 03 00 DA 07 00 00 50 A5 82 00 02 00 01 00 00 CF 60 AD 95 CF
      60 AD 95 1E 3C 00 00 04 00 00 00 00 01 00 00 40 78 00 00 78 40 00 00 02
      00 00 00 */
   unsigned long m_nPort;
};



//-----AckFileAccept------------------------------------------------------------
class CPT_AckFileAccept : public CPT_Ack
{
public:
  CPT_AckFileAccept(unsigned short _nPort, unsigned long _nSequence,
                    ICQUser *_cUser);
protected:
   /* 50 A5 82 00 03 00 DA 07 00 00 50 A5 82 00 03 00 01 00 00 D1 EF 04 9F 7F
      00 00 01 4A 1F 00 00 04 00 00 00 00 20 3A 00 00 01 00 00 00 00 00 00 3A
      20 00 00 05 00 00 00 */

   unsigned long  m_nFileSize;  // not used in the ack
   unsigned long  m_nPort;

};


//-----AckFileRefuse------------------------------------------------------------
class CPT_AckFileRefuse : public CPT_Ack
{
public:
  CPT_AckFileRefuse(const char *_sReason, unsigned long _nSequence, ICQUser *_cUser);
protected:
   /* 50 A5 82 00 03 00 DA 07 00 00 50 A5 82 00 03 00 0A 00 6E 6F 20 74 68 61
      6E 6B 73 00 D1 EF 04 9F 7F 00 00 01 4A 1F 00 00 04 01 00 00 00 00 00 00
      00 01 00 00 00 00 00 00 00 00 00 00 03 00 00 00 */
};


//++++Cancel++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CPT_Cancel : public CPacketTcp
{
protected:
   CPT_Cancel(unsigned short _nSubCommand, unsigned long _nSequence,
              ICQUser *_cUser);
};


//-----ChatCancel---------------------------------------------------------------
class CPT_CancelChat : public CPT_Cancel
{
public:
   CPT_CancelChat(unsigned long _nSequence, ICQUser *_cUser);
   /* 50 A5 82 00 03 00 D0 07 00 00 50 A5 82 00 02 00 01 00 00 CF 60 AD D3 CF
      60 AD D3 28 12 00 00 04 00 00 00 00 01 00 00 00 00 00 00 00 00 00 00 06
      00 00 00 */
};


//-----FileCancel---------------------------------------------------------------
class CPT_CancelFile : public CPT_Cancel
{
public:
   CPT_CancelFile(unsigned long _nSequence, ICQUser *_cUser);
   /* 50 A5 82 00 03 00 D0 07 00 00 50 A5 82 00 02 00 01 00 00 CF 60 AD D3 CF
      60 AD D3 28 12 00 00 04 00 00 00 00 01 00 00 00 00 00 00 00 00 00 00 06
      00 00 00 */
};



#endif
