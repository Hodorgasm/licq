/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2004-2013 Licq developers <licq-dev@googlegroups.com>
 *
 * Licq is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Licq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Licq; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "msnpacket.h"

#include <cctype>
#include <cstdio>
#include <cstring>
#include <string>

#include <licq/crypto.h>
#include <licq/daemon.h>

#include "msn_constants.h"
#include "pluginversion.h"

using namespace LicqMsn;
using std::string;

static inline bool is_base64(unsigned char c)
{
  return (isalnum(c) || (c == '+') || (c == '/'));
}

static string MSN_Base64Encode(const string& strIn)
{
  const unsigned char* szIn = (const unsigned char*)(strIn.c_str());
  size_t nLen = strIn.size();
  string ret;
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];

  while (nLen--)
  {
    char_array_3[i++] = *(szIn++);
    if (i == 3)
    {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for(i = 0; (i <4) ; i++)
        ret += base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if (i)
  {
    for(j = i; j < 3; j++)
      char_array_3[j] = '\0';

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (j = 0; (j <= i + 1); j++)
      ret += base64_chars[char_array_4[j]];

    while((i++ < 2))
      ret += '=';
  }

  return ret;
}

string MSN_Base64Decode(string const& strIn)
{
  int nLen = strIn.size();
  int i = 0;
  int j = 0;
  int nIn = 0;
  unsigned char char_array_4[4], char_array_3[3];
  string ret;

  while (nLen-- && (strIn[nIn] != '=') && is_base64(strIn[nIn]))
  {
    char_array_4[i++] = strIn[nIn];
    nIn++;
    if (i ==4)
    {
      for (i = 0; i < 4; i++)
        char_array_4[i] = base64_chars.find(char_array_4[i]);

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; i < 3; i++)
        ret += char_array_3[i];
      i = 0;
    }
  }

  if (i) {
    for (j = i; j <4; j++)
      char_array_4[j] = 0;

    for (j = 0; j <4; j++)
      char_array_4[j] = base64_chars.find(char_array_4[j]);

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; j < i - 1; j++) ret += char_array_3[j];
  }

  return ret;
}

unsigned short CMSNPacket::s_nSequence = 0;

CMSNPacket::CMSNPacket(bool _bPing)
{
  m_pBuffer = 0;
  m_szCommand = 0;
  m_nSize = 0;
  m_bPing = _bPing;

  if (s_nSequence > 9999)
   s_nSequence = 0;
  m_nSequence = s_nSequence++;
}

void CMSNPacket::InitBuffer()
{
  if (strlen(m_szCommand) == 0)
    return;
  char buf[32];
    
  if (m_bPing)
    m_nSize += snprintf(buf, 32, "%s", m_szCommand) + 2;
  else
    m_nSize += snprintf(buf, 32, "%s %hu ", m_szCommand, m_nSequence) + 2; //don't forget \r\n
  
  m_pBuffer = new CMSNBuffer(m_nSize);
  m_pBuffer->packRaw(buf, strlen(buf));
}

std::string CMSNPacket::CreateGUID()
{
  char guid[37];
  static int x = 1;
  x++;
  // Create a random UID
  srand(time(0)+x);
  sprintf(guid, "%4X%4X-%4X-%4X-%4X-%4X%4X%4X",
	 rand() % 0xFFFF, rand() % 0xFFFF, rand() % 0xFFFF, rand() % 0xFFFF,
	 rand() % 0xFFFF, rand() % 0xFFFF, rand() % 0xFFFF, rand() % 0xFFFF);
  return guid;
}

CMSNPayloadPacket::CMSNPayloadPacket(char msgType)
  : CMSNPacket(),
  myMsgType(msgType)
{
  m_nPayloadSize = 0;
}

void CMSNPayloadPacket::InitBuffer()
{
  if (strlen(m_szCommand) == 0)
    return;
  char buf[32];
  
  m_nSize = snprintf(buf, 32, "%s %hu %c %lu\r\n", m_szCommand, m_nSequence,
      myMsgType, m_nPayloadSize);
  m_nSize += m_nPayloadSize;
  
  m_pBuffer = new CMSNBuffer(m_nSize);
  m_pBuffer->packRaw(buf, strlen(buf));
}

CMSNP2PPacket::CMSNP2PPacket(const string& toEmail, unsigned long nSessionId,
			     unsigned long nBaseId,unsigned long nDataOffsetHI,
			     unsigned long nDataOffsetLO,
			     unsigned long nDataSizeHI, unsigned long nDataSizeLO,
			     unsigned long nLen, unsigned long nFlag,
			     unsigned long nAckId, unsigned long nAckUniqueId,
			     unsigned long nAckDataHI,unsigned long nAckDataLO)
  : CMSNPayloadPacket('A'),
    myToEmail(toEmail)
{
  m_nSessionId = nSessionId;
  m_nBaseId = nBaseId;
  m_nDataOffsetLO = nDataOffsetLO;
  m_nDataOffsetHI = nDataOffsetHI;
  m_nDataSizeLO = nDataSizeLO;
  m_nDataSizeHI = nDataSizeHI;
  m_nLen = nLen;
  m_nFlag = nFlag;
  m_nAckId = nAckId;
  m_nAckUniqueId = nAckUniqueId;
  m_nAckDataLO = nAckDataLO;
  m_nAckDataHI = nAckDataHI;
}

CMSNP2PPacket::~CMSNP2PPacket()
{
  // Empty
}

void CMSNP2PPacket::InitBuffer()
{
  m_szCommand = strdup("MSG");

  // The message
  char szMsgBuf[128];
  snprintf(szMsgBuf, sizeof(szMsgBuf)-1,
           "MIME-Version: 1.0\r\n"
           "Content-Type: application/x-msnmsgrp2p\r\n"
           "P2P-Dest: %s\r\n"
           "\r\n", myToEmail.c_str());

  m_nPayloadSize += 4 + 48 + strlen(szMsgBuf);

  char buf[32];
  m_nSize = snprintf(buf, 32, "%s %hu D %lu\r\n", m_szCommand,
		     m_nSequence, m_nPayloadSize);
  m_nSize += m_nPayloadSize;

  m_pBuffer = new CMSNBuffer(m_nSize);
  m_pBuffer->packRaw(buf, strlen(buf));
  m_pBuffer->packRaw(szMsgBuf, strlen(szMsgBuf));

  // Binary header - 48 bytes
  m_pBuffer->packUInt32LE(m_nSessionId);
  m_pBuffer->packUInt32LE(m_nBaseId);
  m_pBuffer->packUInt32LE(m_nDataOffsetLO);
  m_pBuffer->packUInt32LE(m_nDataOffsetHI);
  m_pBuffer->packUInt32LE(m_nDataSizeLO);
  m_pBuffer->packUInt32LE(m_nDataSizeHI);
  m_pBuffer->packUInt32LE(m_nLen);
  m_pBuffer->packUInt32LE(m_nFlag);
  m_pBuffer->packUInt32LE(m_nAckId);
  m_pBuffer->packUInt32LE(m_nAckUniqueId);
  m_pBuffer->packUInt32LE(m_nAckDataLO);
  m_pBuffer->packUInt32LE(m_nAckDataHI);
}

CPS_MSNVersion::CPS_MSNVersion() : CMSNPacket()
{
  m_szCommand = strdup("VER");
  char szParams[] = "MSNP9 MSNP8 CVR0";
  m_nSize += strlen(szParams);
  InitBuffer();

  m_pBuffer->packRaw(szParams, strlen(szParams));
  m_pBuffer->packRaw("\r\n", 2);
}

CPS_MSNClientVersion::CPS_MSNClientVersion(const string& username) : CMSNPacket()
{
  m_szCommand = strdup("CVR");
  //TODO customize locale en_US = 0x0409
  char szParams[] = "0x0409 winnt 6.0 i386 MSNMSGR 6.0.0602 MSMSGS ";
  m_nSize += strlen(szParams) + username.size();
  InitBuffer();

  m_pBuffer->packRaw(szParams, strlen(szParams));
  m_pBuffer->packRaw(username);
  m_pBuffer->packRaw("\r\n", 2);
}

CPS_MSNUser::CPS_MSNUser(const string& username) : CMSNPacket()
{
  m_szCommand = strdup("USR");
  char szParams[] = "TWN I ";
  m_nSize += strlen(szParams) + username.size();
  InitBuffer();

  m_pBuffer->packRaw(szParams, strlen(szParams));
  m_pBuffer->packRaw(username);
  m_pBuffer->packRaw("\r\n", 2);
}

CPS_MSNSendTicket::CPS_MSNSendTicket(const string& ticket)
  : CMSNPacket()
{
  m_szCommand = strdup("USR");
  string params = "TWN S ";
  m_nSize += params.size() + ticket.size();
  InitBuffer();

  m_pBuffer->packRaw(params);
  m_pBuffer->packRaw(ticket);
  m_pBuffer->packRaw("\r\n", 2);
}

CPS_MSNChangeStatus::CPS_MSNChangeStatus(string& status) : CMSNPacket()
{
  m_szCommand = strdup("CHG");
  char szParams[] = " 268435500";
  m_nSize += strlen(szParams) + 3;
  InitBuffer();
  m_pBuffer->packRaw(status.c_str(), status.size());
  m_pBuffer->packRaw(szParams, strlen(szParams));
  m_pBuffer->packRaw("\r\n", 2);
}

CPS_MSNLogoff::CPS_MSNLogoff() : CMSNPacket(true)
{
  m_szCommand = strdup("OUT");
  m_nSize += 0;
  InitBuffer();

  m_pBuffer->packRaw("\r\n", 2);
}

CPS_MSNSync::CPS_MSNSync(unsigned long nVersion) : CMSNPacket()
{
  m_szCommand = strdup("SYN");
  char szParams[15];
  int nSize = sprintf(szParams, "%lu", nVersion);
  m_nSize += nSize;
  InitBuffer();

  m_pBuffer->packRaw(szParams, nSize);
  m_pBuffer->packRaw("\r\n", 2);
}

CPS_MSNChallenge::CPS_MSNChallenge(const std::string& hash) : CMSNPacket()
{
  m_szCommand = strdup("QRY");
  const char *szParams = "msmsgs@msnmsgr.com 32";
  m_nSize += strlen(szParams) + 32; //payload
  InitBuffer();
  
  const string toHash = (hash + "Q1P7W2E4J9R8U3S5").substr(0, 64);
  const string hexDigest = Licq::Md5::hashToHexString(toHash);

  m_pBuffer->packRaw(szParams, strlen(szParams));
  m_pBuffer->packRaw("\r\n", 2);
  m_pBuffer->packRaw(hexDigest);
}

CPS_MSNSetPrivacy::CPS_MSNSetPrivacy() : CMSNPacket()
{
  //TODO
  m_szCommand = strdup("GTC");
  char szParams[] = "N";
  m_nSize += strlen(szParams);
  InitBuffer();

  m_pBuffer->packRaw(szParams, strlen(szParams));
  m_pBuffer->packRaw("\r\n", 2);
}

CPS_MSNAddUser::CPS_MSNAddUser(const string& username, const char* list)
  : CMSNPacket()
{
  m_szCommand = strdup("ADD");
  m_nSize += strlen(list) + (username.size() * 2) + 2;
  InitBuffer();

  m_pBuffer->packRaw(list, strlen(list));
  m_pBuffer->packRaw(" ", 1);
  m_pBuffer->packRaw(username);
  m_pBuffer->packRaw(" ", 1);
  m_pBuffer->packRaw(username);
  m_pBuffer->packRaw("\r\n", 2);
}

CPS_MSNRemoveUser::CPS_MSNRemoveUser(const string& username, const char* list)
  : CMSNPacket()
{
  m_szCommand = strdup("REM");
  m_nSize += strlen(list) + username.size() + 1;
  InitBuffer();

  m_pBuffer->packRaw(list, strlen(list));
  m_pBuffer->packRaw(" ", 1);
  m_pBuffer->packRaw(username);
  m_pBuffer->packRaw("\r\n", 2);
}

CPS_MSNRenameUser::CPS_MSNRenameUser(const string& username, const string& newNick)
  : CMSNPacket()
{
  m_szCommand = strdup("REA");
  m_nSize += username.size() + newNick.size() + 1;
  InitBuffer();

  m_pBuffer->packRaw(username);
  m_pBuffer->packRaw(" ", 1);
  m_pBuffer->packRaw(newNick);
  m_pBuffer->packRaw("\r\n", 2);
}

CPS_MSN_SBStart::CPS_MSN_SBStart(const string& cookie, const string& username)
  : CMSNPacket()
{
  m_szCommand = strdup("USR");
  m_nSize += cookie.size() + username.size() + 1;
  InitBuffer();

  m_pBuffer->packRaw(username);
  m_pBuffer->packRaw(" ", 1);
  m_pBuffer->packRaw(cookie);
  m_pBuffer->packRaw("\r\n", 2);
}

CPS_MSN_SBAnswer::CPS_MSN_SBAnswer(const string& session, const string& cookie,
    const string& username)
  : CMSNPacket()
{
  m_szCommand = strdup("ANS");
  m_nSize += session.size() + cookie.size() + username.size() + 2;
  InitBuffer();

  m_pBuffer->packRaw(username);
  m_pBuffer->packRaw(" ", 1);
  m_pBuffer->packRaw(cookie);
  m_pBuffer->packRaw(" ", 1);
  m_pBuffer->packRaw(session);
  m_pBuffer->packRaw("\r\n", 2);
}

CPS_MSNMessage::CPS_MSNMessage(const char *szMsg)
  : CMSNPayloadPacket('A')
{
  m_szCommand = strdup("MSG");
  char szParams[] = "MIME-Version: 1.0\r\n"
    "Content-Type: text/plain; charset=UTF-8\r\n"
    "X-MMS-IM-Format: FN=MS%20Sans%20Serif; EF=; CO=0; CS=0; PF=0\r\n"
    "\r\n";
  m_nPayloadSize = strlen(szMsg) + strlen(szParams);
  CMSNPayloadPacket::InitBuffer();
  
  m_szMsg = strdup(szMsg);

  m_pBuffer->packRaw(szParams, strlen(szParams));
  m_pBuffer->packRaw(m_szMsg, strlen(m_szMsg));
}

CPS_MsnClientCaps::CPS_MsnClientCaps()
   : CMSNPayloadPacket('U')
{
  m_szCommand = strdup("MSG");
  string params = string("MIME-Version: 1.0\r\n"
      "Content-Type: text/x-clientcaps\r\n\r\n"
      "Client-Name: Licq ") + Licq::gDaemon.Version() + " (MSN " PLUGIN_VERSION_STRING ")\r\n";
  m_nPayloadSize = params.size();

  CMSNPayloadPacket::InitBuffer();
  m_pBuffer->packRaw(params);
}

CPS_MSNPing::CPS_MSNPing() : CMSNPacket(true)
{
  m_szCommand = strdup("PNG");
  m_nSize += 0;
  InitBuffer();

  m_pBuffer->packRaw("\r\n", 2);
}

CPS_MSNXfr::CPS_MSNXfr() : CMSNPacket()
{
  m_szCommand = strdup("XFR");
  m_nSize += 2;
  InitBuffer();

  m_pBuffer->packRaw("SB\r\n", 4);
}

CPS_MSNCall::CPS_MSNCall(const string& username) : CMSNPacket()
{
  m_szCommand = strdup("CAL");
  m_nSize += username.size();
  InitBuffer();

  m_pBuffer->packRaw(username);
  m_pBuffer->packRaw("\r\n", 2);
}

CPS_MSNTypingNotification::CPS_MSNTypingNotification(const string& email)
  : CMSNPayloadPacket('N')
{
  m_szCommand = strdup("MSG");
  char szParams1[] = "MIME-Version: 1.0\r\n"
    "Content-Type: text/x-msmsgscontrol\r\n"
    "TypingUser: ";
  char szParams2[] = "\r\n\r\n\r\n";    
  m_nPayloadSize = strlen(szParams1) + strlen(szParams2) + email.size();
  CMSNPayloadPacket::InitBuffer();  

  m_pBuffer->packRaw(szParams1, strlen(szParams1));
  m_pBuffer->packRaw(email);
  m_pBuffer->packRaw(szParams2, strlen(szParams2));
}

CPS_MSNCancelInvite::CPS_MSNCancelInvite(const string& cookie, const string& code)
  : CMSNPayloadPacket('N')
{
  m_szCommand = strdup("MSG");

  char payload[512];
  snprintf(payload, sizeof(payload),
      "MIME-Version: 1.0\r\n"
      "Content-Type: text/x-msmsgsinvite; charset=UTF-8\r\n"
      "\r\n"
      "Invitation-Command: CANCEL\r\n"
      "Cancel-Code: %s\r\n"
      "Invitation-Cookie: %s\r\n"
      "\r\n",
      code.c_str(), cookie.c_str());

  m_nPayloadSize = strlen(payload);
  CMSNPayloadPacket::InitBuffer();
  m_pBuffer->packRaw(payload, m_nPayloadSize);
}

CPS_MSNInvitation::CPS_MSNInvitation(const string& toEmail, const string& fromEmail, const string& msnObject)
  : CMSNP2PPacket(toEmail)
{
  std::string branchGUID = CreateGUID();
  myCallGUID = CreateGUID();
  string strMSNObject64 = MSN_Base64Encode(msnObject);

  char szBodyBuf[512];
  char szHeaderBuf[512];

  m_nSessionId = rand();

  // The invite message body
  snprintf(szBodyBuf, sizeof(szBodyBuf)-1,
	   "EUF-GUID: %s\r\n"
	   "SessionID: %ld\r\n"
	   "AppID: 1\r\n"
	   "Context: %s\r\n"
	   "\r\n", DP_EUF_GUID, m_nSessionId, strMSNObject64.c_str());

  // The Invite header
  snprintf(szHeaderBuf, sizeof(szHeaderBuf)-1,
	   "INVITE MSNMSGR:%s MSNSLP/1.0\r\n"
	   "To: <msnmsgr:%s>\r\n"
	   "From: <msnmsgr:%s>\r\n"
	   "Via: MSNSLP/1.0/TLP ;branch={%s}\r\n"
	   "CSeq: 0\r\n"
	   "Call-ID: {%s}\r\n"
	   "Max-Forwards: 0\r\n"
	   "Content-Type: application/x-msnmsgr-sessionreqbody\r\n"
	   "Content-Length: %lu\r\n"
	   "\r\n", toEmail.c_str(), toEmail.c_str(), fromEmail.c_str(), branchGUID.c_str(),
	   myCallGUID.c_str(), static_cast<unsigned long>(strlen(szBodyBuf)+1));

  string strMsg = szHeaderBuf;
  strMsg += szBodyBuf;
  strMsg += '\0';

  srand(time(0));
  m_nBaseId = 4 + rand();
  m_nSessionId = 0;
  m_nAckId = rand();
  m_nDataSizeLO = strlen(szBodyBuf)+strlen(szHeaderBuf)+1;
  m_nLen = strlen(szBodyBuf)+strlen(szHeaderBuf)+1;
  m_nPayloadSize = strMsg.size();
  CMSNP2PPacket::InitBuffer();

  m_pBuffer->packRaw(strMsg);

  // Footer
  m_pBuffer->packUInt32LE(0);
}

CPS_MSNP2PBye::CPS_MSNP2PBye(const string& toEmail, const string& fromEmail,
    const string& callId, unsigned long _nBaseId, unsigned long _nAckId,
    unsigned long /* _nDataSizeHI */, unsigned long /* _nDataSizeLO */)
  : CMSNP2PPacket(toEmail, 0, _nBaseId, 0, 0, 0, 4, 0, 0, _nAckId, 0, 0, 0)
		     //SizeHI, _nDataSizeLO)
{
  std::string branchGUID = CreateGUID();

  char szMsgBuf[768];

  snprintf(szMsgBuf, sizeof(szMsgBuf)-1,
	   "BYE MSNMSGR:%s MSNSLP/1.0\r\n"
	   "To: <msnmsgr:%s>\r\n"
	   "From: <msnmsgr:%s>\r\n"
	   "Via: MSNSLP/1.0/TLP ;branch={%s}\r\n"
	   "CSeq: 0\r\n"
	   "Call-ID: {%s}\r\n"
	   "Max-Forwards: 0\r\n"
	   "Content-Type: application/x-msnmsgr-sessionclosebody\r\n"
	   "Content-Length: 3\r\n"
	   "\r\n"
	   "\r\n",
      toEmail.c_str(), toEmail.c_str(), fromEmail.c_str(),
      branchGUID.c_str(), callId.c_str());

  string strMsg = szMsgBuf;
  strMsg += '\0';

  srand(time(0));
  m_nSessionId = 0;
  m_nAckUniqueId = 0;
  m_nDataSizeLO = strlen(szMsgBuf)+1;
  m_nLen = strlen(szMsgBuf)+1;
  m_nPayloadSize = strMsg.size();
  CMSNP2PPacket::InitBuffer();

  m_pBuffer->packRaw(strMsg);

  // Footer
  m_pBuffer->packUInt32LE(0);
}

CPS_MSNP2PAck::CPS_MSNP2PAck(const string& toEmail, unsigned long _nSessionId,
			     unsigned long _nBaseId, unsigned long _nAckId,
			     unsigned long _nAckBaseId,
			     unsigned long _nDataSizeHI,
			     unsigned long _nDataSizeLO)
  : CMSNP2PPacket(toEmail, _nSessionId, _nBaseId, 0, 0, _nDataSizeHI, _nDataSizeLO,  0, 0x02,
		  _nAckId, _nAckBaseId, _nDataSizeHI, _nDataSizeLO)
{
  // No data...
  CMSNP2PPacket::InitBuffer();

  m_pBuffer->packUInt32LE(0);
}
