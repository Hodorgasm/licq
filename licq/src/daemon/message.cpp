#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "licq_message.h"
#include "licq_user.h"
#include "licq_translate.h"
#include "licq_icqd.h"

int CUserEvent::s_nId = 1;

//----CUserEvent::constructor---------------------------------------------------
CUserEvent::CUserEvent(unsigned short nSubCommand, unsigned short nCommand,
                       unsigned long nSequence, time_t tTime,
                       unsigned long nFlags)
{
   m_nSubCommand = nSubCommand;
   m_nCommand = nCommand;
   m_nSequence = nSequence;
   m_tTime = (tTime == TIME_NOW ? time(NULL) : tTime);
   m_nFlags = nFlags;
   m_eDir = D_RECEIVER;
   m_szText = NULL;
   // race condition here, but so unlikely that I don't care
   m_nId = s_nId++;
}


const char *CUserEvent::Text()
{
  if (m_szText == NULL)
    CreateDescription();

  return m_szText;
}



//-----CUserEvent::LicqVersionStr-----------------------------------------------
const char *CUserEvent::LicqVersionStr()
{
   static char s_szVersion[8];
   sprintf(s_szVersion, "v0.%d", LicqVersion());
   return (s_szVersion);
}


//-----CUserEvent::destructor---------------------------------------------------
CUserEvent::~CUserEvent()
{
  delete[] m_szText;
}


//-----NToNS--------------------------------------------------------------------
int AddStrWithColons(char *_szNewStr, const char *_szOldStr)
{
  unsigned long j = 0, i = 0;
  _szNewStr[j++] = ':';
  while (_szOldStr[i] != '\0')
  {
    _szNewStr[j++] = _szOldStr[i];
    if (_szOldStr[i] == '\n') _szNewStr[j++] = ':';
    i++;
  }
  if (_szNewStr[j - 1] == ':') j--;
  _szNewStr[j] = '\0';
  return (j);
}


//-----CUserEvent::AddToHistory-------------------------------------------------
int CUserEvent::AddToHistory_Header(direction _nDir, char *szOut)
{
  return sprintf(szOut, "[ %c | %04d | %04d | %04d | %ld ]\n",
          _nDir == D_RECEIVER ? 'R' : 'S',
          m_nSubCommand, m_nCommand, (unsigned short)(m_nFlags >> 16), m_tTime);
}


void CUserEvent::AddToHistory_Flush(ICQUser *u, char *szOut)
{
  if (u != NULL)
    u->WriteToHistory(szOut);
  else
  {
    ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
    o->WriteToHistory(szOut);
    gUserManager.DropOwner();
  }
}




//=====CEventMsg================================================================

CEventMsg::CEventMsg(const char *_szMessage, unsigned short _nCommand,
                     time_t _tTime, unsigned long _nFlags)
   : CUserEvent(ICQ_CMDxSUB_MSG, _nCommand, 0, _tTime, _nFlags)
{
  m_szMessage = strdup(_szMessage == NULL ? "" : _szMessage);
}


void CEventMsg::CreateDescription()
{
  m_szText = strdup(m_szMessage);
}


CEventMsg::~CEventMsg()
{
  free (m_szMessage);
}

void CEventMsg::AddToHistory(ICQUser *u, direction _nDir)
{
  char *szOut = new char[ (strlen(m_szMessage) << 1) + EVENT_HEADER_SIZE];
  int nPos = AddToHistory_Header(_nDir, szOut);
  AddStrWithColons(&szOut[nPos], m_szMessage);
  AddToHistory_Flush(u, szOut);
  delete [] szOut;
}


CEventMsg *CEventMsg::Parse(char *sz, unsigned short nCmd, time_t nTime, unsigned long nFlags)
{
  gTranslator.ServerToClient (sz);
  return new CEventMsg(sz, nCmd, nTime, nFlags);
}






//=====CEventFile===============================================================

CEventFile::CEventFile(const char *_szFilename, const char *_szFileDescription,
                       unsigned long _nFileSize, unsigned long _nSequence,
                       time_t _tTime, unsigned long _nFlags)
   : CUserEvent(ICQ_CMDxSUB_FILE, ICQ_CMDxTCP_START, _nSequence, _tTime, _nFlags)
{
  m_szFilename = strdup(_szFilename == NULL ? "" : _szFilename);
  m_szFileDescription = strdup(_szFileDescription == NULL ? "" : _szFileDescription);
  m_nFileSize = _nFileSize;
}


void CEventFile::CreateDescription()
{
  m_szText = new char[strlen(m_szFilename) + strlen(m_szFileDescription) + 64];
  sprintf(m_szText, "File: %s (%ld bytes)\nDescription: %s\n", m_szFilename,
          m_nFileSize, m_szFileDescription);

}


CEventFile::~CEventFile()
{
   free (m_szFilename);
   free (m_szFileDescription);
}

void CEventFile::AddToHistory(ICQUser *u, direction _nDir)
{
  char *szOut = new char[(strlen(m_szFilename) + strlen(m_szFileDescription)) * 2 + 16 + EVENT_HEADER_SIZE];
  int nPos = AddToHistory_Header(_nDir, szOut);
  nPos += sprintf(&szOut[nPos], ":%s\n", m_szFilename);
  nPos += sprintf(&szOut[nPos], ":%ld\n", m_nFileSize);
  AddStrWithColons(&szOut[nPos], m_szFileDescription);
  AddToHistory_Flush(u, szOut);
  delete [] szOut;
}



//=====CEventUrl================================================================

CEventUrl::CEventUrl(const char *_szUrl, const char *_szUrlDescription,
                     unsigned short _nCommand, time_t _tTime,
                     unsigned long _nFlags)
   : CUserEvent(ICQ_CMDxSUB_URL, _nCommand, 0, _tTime, _nFlags)
{
   m_szUrl = strdup(_szUrl == NULL ? "" : _szUrl);
   m_szUrlDescription = strdup(_szUrlDescription == NULL ? "" : _szUrlDescription);
}


void CEventUrl::CreateDescription()
{
  m_szText = new char[strlen(m_szUrl) + strlen(m_szUrlDescription) + 64];
  sprintf(m_szText, "URL: %s\nDescription: %s\n", m_szUrl, m_szUrlDescription);
}


CEventUrl::~CEventUrl()
{
   free (m_szUrl);
   free (m_szUrlDescription);
}

void CEventUrl::AddToHistory(ICQUser *u, direction _nDir)
{
  char *szOut = new char[(strlen(m_szUrlDescription) << 1) +
                   (strlen(m_szUrl) << 1) + EVENT_HEADER_SIZE];
  int nPos = AddToHistory_Header(_nDir, szOut);
  nPos += sprintf(&szOut[nPos], ":%s\n", m_szUrl);
  AddStrWithColons(&szOut[nPos], m_szUrlDescription);
  AddToHistory_Flush(u, szOut);
  delete [] szOut;
}


CEventUrl *CEventUrl::Parse(char *sz, unsigned short nCmd, time_t nTime, unsigned long nFlags)
{
  // parse the message into url and url description
  char **szUrl = new char*[2]; // desc, url
  if (!ParseFE(sz, &szUrl, 2))
  {
    delete []szUrl;
    return NULL;
  }

  // translating string with Translation Table
  gTranslator.ServerToClient(szUrl[0]);
  CEventUrl *e = new CEventUrl(szUrl[1], szUrl[0], nCmd, nTime, nFlags);
  delete []szUrl;

  return e;
}

//=====CEventChat===============================================================

CEventChat::CEventChat(const char *szReason, unsigned long nSequence,
                       time_t tTime, unsigned long nFlags)
   : CUserEvent(ICQ_CMDxSUB_CHAT, ICQ_CMDxTCP_START, nSequence, tTime, nFlags)
{
  m_szReason = strdup(szReason ==  NULL ? "" : szReason);
  m_szClients = NULL;
  m_nPort = 0;
}

CEventChat::CEventChat(const char *szReason, const char *szClients,
   unsigned short nPort, unsigned long nSequence,
   time_t tTime, unsigned long nFlags)
   : CUserEvent(ICQ_CMDxSUB_CHAT, ICQ_CMDxTCP_START, nSequence, tTime, nFlags)
{
  m_szReason = strdup(szReason ==  NULL ? "" : szReason);
  if (nPort == 0)
    m_szClients = NULL;
  else
    m_szClients = strdup(szClients);
  m_nPort = nPort;
}

void CEventChat::CreateDescription()
{
  if (m_szClients == NULL)
    m_szText = strdup(m_szReason);
  else
  {
    m_szText = (char *)malloc(strlen(m_szReason) + strlen(m_szClients) + 128);
    sprintf(m_szText, "%s\n--------------------\nMultiparty:\n%s", m_szReason, m_szClients);
  }
}


CEventChat::~CEventChat()
{
  free(m_szReason);
  if (m_szClients != NULL) free(m_szClients);
}


void CEventChat::AddToHistory(ICQUser *u, direction _nDir)
{
  char *szOut = new char[(strlen(Text()) << 1) + EVENT_HEADER_SIZE];
  int nPos = AddToHistory_Header(_nDir, szOut);
  AddStrWithColons(&szOut[nPos], Text());
  AddToHistory_Flush(u, szOut);
  delete [] szOut;
}


//=====CEventAdded==============================================================
CEventAdded::CEventAdded(unsigned long _nUin, const char *_szAlias,
                         const char *_szFirstName,const char *_szLastName,
                         const char *_szEmail, unsigned short _nCommand,
                         time_t _tTime, unsigned long _nFlags)
   : CUserEvent(ICQ_CMDxSUB_ADDEDxTOxLIST, _nCommand, 0, _tTime, _nFlags)
{
  m_szAlias = strdup(_szAlias);
  m_szFirstName = strdup(_szFirstName);
  m_szLastName = strdup(_szLastName);
  m_szEmail = strdup(_szEmail);
  m_nUin = _nUin;
}

void CEventAdded::CreateDescription()
{
  m_szText = new char[strlen(m_szAlias) + strlen(m_szFirstName) +
                      strlen(m_szLastName) + strlen(m_szEmail) + 128];
  sprintf(m_szText, "%s (%s %s, %s), uin %ld, added you to their contact list.\n",
          m_szAlias, m_szFirstName, m_szLastName, m_szEmail, m_nUin);
}


CEventAdded::~CEventAdded()
{
  free (m_szAlias);
  free (m_szFirstName);
  free (m_szLastName);
  free (m_szEmail);
}

void CEventAdded::AddToHistory(ICQUser *u, direction _nDir)
{
  char *szOut = new char[(strlen(m_szAlias) + strlen(m_szFirstName) +
                    strlen(m_szLastName) + strlen(m_szEmail)) * 2 + 16 +
                   EVENT_HEADER_SIZE];
  int nPos = AddToHistory_Header(_nDir, szOut);
  nPos += sprintf(&szOut[nPos], ":%ld\n:%s\n:%s\n:%s\n:%s\n", m_nUin,
                  m_szAlias, m_szFirstName, m_szLastName, m_szEmail);
  AddToHistory_Flush(u, szOut);
  delete [] szOut;
}



//=====CEventAuthReq===============================================================
CEventAuthRequest::CEventAuthRequest(unsigned long _nUin, const char *_szAlias,
                       const char *_szFirstName,const char *_szLastName,
                       const char *_szEmail, const char *_szReason,
                       unsigned short _nCommand, time_t _tTime,
                       unsigned long _nFlags)
   : CUserEvent(ICQ_CMDxSUB_AUTHxREQUEST, _nCommand, 0, _tTime, _nFlags)
{
   m_szAlias = strdup(_szAlias);
   m_szFirstName = strdup(_szFirstName);
   m_szLastName = strdup(_szLastName);
   m_szEmail = strdup(_szEmail);
   m_szReason = strdup(_szReason);
   m_nUin = _nUin;
}

void CEventAuthRequest::CreateDescription()
{
  m_szText = new char[strlen(m_szAlias) + strlen(m_szFirstName)
                      + strlen(m_szLastName) + strlen(m_szEmail)
                      + strlen(m_szReason) + 128];
  sprintf(m_szText, "%s (%s %s, %s), uin %ld, requests authorization to add you to their contact list:\n%s\n",
          m_szAlias, m_szFirstName, m_szLastName, m_szEmail, m_nUin, m_szReason);
}


CEventAuthRequest::~CEventAuthRequest()
{
  free (m_szAlias);
  free (m_szFirstName);
  free (m_szLastName);
  free (m_szEmail);
  free (m_szReason);
}


void CEventAuthRequest::AddToHistory(ICQUser *u, direction _nDir)
{
  char *szOut = new char[(strlen(m_szAlias) + strlen(m_szFirstName) +
                    strlen(m_szLastName) + strlen(m_szEmail) +
                    strlen(m_szReason)) * 2 + 16 +
                   EVENT_HEADER_SIZE];
  int nPos = AddToHistory_Header(_nDir, szOut);
  nPos += sprintf(&szOut[nPos], ":%ld\n:%s\n:%s\n:%s\n:%s\n", m_nUin,
                  m_szAlias, m_szFirstName, m_szLastName, m_szEmail);
  AddStrWithColons(&szOut[nPos], m_szReason);
  AddToHistory_Flush(u, szOut);
  delete [] szOut;
}



//=====CEventAuthGranted========================================================
CEventAuthGranted::CEventAuthGranted(unsigned long _nUin, const char *_szMessage,
                       unsigned short _nCommand, time_t _tTime,
                       unsigned long _nFlags)
   : CUserEvent(ICQ_CMDxSUB_AUTHxGRANTED, _nCommand, 0, _tTime, _nFlags)
{
  m_szMessage = _szMessage == NULL ? strdup("") : strdup(_szMessage);
  m_nUin = _nUin;
}

void CEventAuthGranted::CreateDescription()
{
  m_szText = new char[strlen(m_szMessage) + 128];
  sprintf(m_szText, "Uin %ld has authorized you:\n%s\n",
          m_nUin, m_szMessage);
}


CEventAuthGranted::~CEventAuthGranted()
{
  free (m_szMessage);
}

void CEventAuthGranted::AddToHistory(ICQUser *u, direction _nDir)
{
  char *szOut = new char[strlen(m_szMessage) * 2 + 16 +
                   EVENT_HEADER_SIZE];
  int nPos = AddToHistory_Header(_nDir, szOut);
  nPos += sprintf(&szOut[nPos], ":%ld\n", m_nUin);
  AddStrWithColons(&szOut[nPos], m_szMessage);
  AddToHistory_Flush(u, szOut);
  delete [] szOut;
}



//=====CEventAuthRefused==========================================================
CEventAuthRefused::CEventAuthRefused(unsigned long _nUin, const char *_szMessage,
                       unsigned short _nCommand, time_t _tTime,
                       unsigned long _nFlags)
   : CUserEvent(ICQ_CMDxSUB_AUTHxREFUSED, _nCommand, 0, _tTime, _nFlags)
{
  m_szMessage = _szMessage == NULL ? strdup("") : strdup(_szMessage);
  m_nUin = _nUin;
}

void CEventAuthRefused::CreateDescription()
{
  m_szText = new char[strlen(m_szMessage) + 128];
  sprintf(m_szText, "Uin %ld has refused to authorized you:\n%s\n",
          m_nUin, m_szMessage);
}


CEventAuthRefused::~CEventAuthRefused()
{
  free (m_szMessage);
}

void CEventAuthRefused::AddToHistory(ICQUser *u, direction _nDir)
{
  char *szOut = new char[strlen(m_szMessage) * 2 + 16 +
                   EVENT_HEADER_SIZE];
  int nPos = AddToHistory_Header(_nDir, szOut);
  nPos += sprintf(&szOut[nPos], ":%ld\n", m_nUin);
  AddStrWithColons(&szOut[nPos], m_szMessage);
  AddToHistory_Flush(u, szOut);
  delete [] szOut;
}



//====CEventHtmlPanel===========================================================
CEventWebPanel::CEventWebPanel(const char *_szName, char *_szEmail,
                               const char *_szMessage, unsigned short _nCommand,
                               time_t _tTime, unsigned long _nFlags)
   : CUserEvent(ICQ_CMDxSUB_WEBxPANEL, _nCommand, 0, _tTime, _nFlags)
{
  m_szName = strdup(_szName);
  m_szEmail = strdup(_szEmail);
  m_szMessage = strdup(_szMessage);
}

void CEventWebPanel::CreateDescription()
{
  m_szText = new char[strlen(m_szName) + strlen(m_szEmail) + strlen(m_szMessage) + 64];
  sprintf(m_szText, "Message from %s (%s) through web panel:\n%s\n",
          m_szName, m_szEmail, m_szMessage);
}


CEventWebPanel::~CEventWebPanel()
{
  free (m_szName);
  free (m_szEmail);
  free (m_szMessage);
}


void CEventWebPanel::AddToHistory(ICQUser *u, direction _nDir)
{
  char *szOut = new char[(strlen(m_szName) + strlen(m_szEmail) +
                    strlen(m_szMessage)) * 2 +
                   EVENT_HEADER_SIZE];
  int nPos = AddToHistory_Header(_nDir, szOut);
  nPos += sprintf(&szOut[nPos], ":%s\n:%s\n", m_szName, m_szEmail);
  AddStrWithColons(&szOut[nPos], m_szMessage);
  AddToHistory_Flush(u, szOut);
  delete [] szOut;
}


//====CEventEmailPager==========================================================
CEventEmailPager::CEventEmailPager(const char *_szName, char *_szEmail,
                                   const char *_szMessage, unsigned short _nCommand,
                                   time_t _tTime, unsigned long _nFlags)
   : CUserEvent(ICQ_CMDxSUB_EMAILxPAGER, _nCommand, 0, _tTime, _nFlags)
{
  m_szName = strdup(_szName);
  m_szEmail = strdup(_szEmail);
  m_szMessage = strdup(_szMessage);
}

void CEventEmailPager::CreateDescription()
{
  m_szText = new char[strlen(m_szName) + strlen(m_szEmail) + strlen(m_szMessage) + 64];
  sprintf(m_szText, "Message from %s (%s) through email pager:\n%s\n",
          m_szName, m_szEmail, m_szMessage);
}


CEventEmailPager::~CEventEmailPager()
{
  free (m_szName);
  free (m_szEmail);
  free (m_szMessage);
}

void CEventEmailPager::AddToHistory(ICQUser *u, direction _nDir)
{
  char *szOut = new char[(strlen(m_szName) + strlen(m_szEmail) +
                    strlen(m_szMessage)) * 2 +
                   EVENT_HEADER_SIZE];
  int nPos = AddToHistory_Header(_nDir, szOut);
  nPos += sprintf(&szOut[nPos], ":%s\n:%s\n", m_szName, m_szEmail);
  AddStrWithColons(&szOut[nPos], m_szMessage);
  AddToHistory_Flush(u, szOut);
  delete [] szOut;
}


//====CEventContactList========================================================
CEventContactList::CEventContactList(ContactList &cl,
                                     unsigned short nCommand,
                                     time_t tTime, unsigned long nFlags)
  : CUserEvent(ICQ_CMDxSUB_CONTACTxLIST, nCommand, 0, tTime, nFlags)
{
  m_vszFields = cl;
}

void CEventContactList::CreateDescription()
{
  m_szText = new char [m_vszFields.size() * 32 + 128];
  char *szEnd = m_szText;
  szEnd += sprintf(m_szText, "Contact list (%d contacts):\n", m_vszFields.size());
  ContactList::iterator iter;
  for (iter = m_vszFields.begin(); iter != m_vszFields.end(); iter++)
  {
    szEnd += sprintf(szEnd, "%s (%ld)\n", (*iter)->Alias(), (*iter)->Uin());
  }

}


CEventContactList::~CEventContactList()
{
  ContactList::iterator iter;
  for (iter = m_vszFields.begin(); iter != m_vszFields.end(); iter++)
    delete *iter;
}


void CEventContactList::AddToHistory(ICQUser *u, direction _nDir)
{
  char *szOut = new char[m_vszFields.size() * 32 + EVENT_HEADER_SIZE];
  int nPos = AddToHistory_Header(_nDir, szOut);
  ContactList::iterator iter;
  for (iter = m_vszFields.begin(); iter != m_vszFields.end(); iter++)
    nPos += sprintf(&szOut[nPos], ":%ld\n:%s\n", (*iter)->Uin(), (*iter)->Alias());
  AddToHistory_Flush(u, szOut);
  delete [] szOut;
}


CEventContactList *CEventContactList::Parse(char *sz, unsigned short nCmd, time_t nTime, unsigned long nFlags)
{
  unsigned short i = 0;
  while ((unsigned char)sz[i++] != 0xFE);
  sz[--i] = '\0';
  int nNumContacts = atoi(sz);
  char **szFields = new char*[nNumContacts * 2 + 1];
  if (!ParseFE(&sz[++i], &szFields, nNumContacts * 2 + 1))
  {
    delete []szFields;
    return NULL;
  }

  // Translate the aliases
  ContactList vc;
  for (i = 0; i < nNumContacts * 2; i += 2)
  {
    gTranslator.ServerToClient(szFields[i + 1]);
    vc.push_back(new CContact(atoi(szFields[i]), szFields[i + 1]));
  }
  delete[] szFields;

  return new CEventContactList(vc, nCmd, nTime, nFlags);
}






CEventUnknownSysMsg::CEventUnknownSysMsg(unsigned short _nSubCommand,
                             unsigned short _nCommand, unsigned long _nUin,
                             const char *_szMsg,
                             time_t _tTime, unsigned long _nFlags)
   : CUserEvent(_nSubCommand, _nCommand, 0, _tTime, _nFlags | E_UNKNOWN)
{
  m_szMsg = _szMsg == NULL ? strdup("") : strdup(_szMsg);
  m_nUin = _nUin;
}

void CEventUnknownSysMsg::CreateDescription()
{
  m_szText = new char [strlen(m_szMsg) + 128];
  sprintf(m_szText, "Unknown system message (0x%04X) from %ld:\n%s\n",
          m_nSubCommand, m_nUin, m_szMsg);
}


CEventUnknownSysMsg::~CEventUnknownSysMsg()
{
  free(m_szMsg);
}

void CEventUnknownSysMsg::AddToHistory(ICQUser * /*u*/, direction /*_nDir*/)
{
/*  char *szOut = new char[strlen(m_szMsg) * 2 + 16 + EVENT_HEADER_SIZE];
  int nPos = sprintf(szOut, "[ %c | 0000 | %04d | %04d | %ld ]\n",
          _nDir == D_RECEIVER ? 'R' : 'S',
          m_nCommand, (unsigned short)(m_nFlags >> 16), m_tTime);
  nPos += sprintf(&szOut[nPos], ":%ld\n", m_nUin);
  AddStrWithColons(&szOut[nPos], m_szMsg);
  AddToHistory_Flush(u, szOut);
  delete [] szOut;*/
}


//=====EventDescriptions=====================================================

static const int MAX_EVENT = 26;

static const char *szEventTypes[27] =
{ "Saved Event",
  "Message",
  "Chat Request",
  "File Transfer",
  "URL",
  "",
  "Authorization Request",
  "AUthorization Refused",
  "Authorization Granted",
  "",
  "",
  "",
  "Added to Contact List",
  "Web Panel",
  "Email Pager",
  "",
  "",
  "",
  "",
  "Contact List",
  "",
  "",
  "",
  "",
  "",
  "",
  "User Info"
};


const char *CUserEvent::Description()
{
  // not thread-safe, but I'm lazy
  static char desc[128];

  if (SubCommand() > MAX_EVENT ||
      szEventTypes[SubCommand()][0] == '\0')
    strcpy(desc, "Unknown Event");
  else
  {
    strcpy(desc, szEventTypes[SubCommand()]);
    if (IsCancelled())
      strcat(desc, " (cancelled)");
  }
  return desc;
}

