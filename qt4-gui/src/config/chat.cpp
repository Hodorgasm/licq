// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007 Licq developers
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

#include "chat.h"

#include "config.h"

#include <QApplication>
#include <QDesktopWidget>

#include <licq_file.h>

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::Config::Chat */

Config::Chat* Config::Chat::myInstance = NULL;

void Config::Chat::createInstance(QObject* parent)
{
  myInstance = new Config::Chat(parent);
}

Config::Chat::Chat(QObject* parent)
  : QObject(parent),
    myBlockUpdates(false),
    myColorsHaveChanged(false)
{
  // Empty
}

void Config::Chat::loadConfiguration(CIniFile& iniFile)
{
  char szTemp[255];

  iniFile.SetSection("appearance");
  iniFile.ReadBool("ManualNewUser", myManualNewUser, false);
  iniFile.ReadBool("SendFromClipboard", mySendFromClipboard, true);
  iniFile.ReadBool("MsgChatView", myMsgChatView, true );
  iniFile.ReadBool("TabbedChatting", myTabbedChatting, true);
  iniFile.ReadBool("ShowHistory", myShowHistory, true);
  iniFile.ReadBool("ShowNotices", myShowNotices, true);
  iniFile.ReadBool("AutoPosReplyWin", myAutoPosReplyWin, true);
  iniFile.ReadBool("AutoSendThroughServer", myAutoSendThroughServer, false);
  iniFile.ReadBool("NoSendClose", myShowSendClose, true);
  myShowSendClose = !myShowSendClose;
  iniFile.ReadNum("ChatMessageStyle", myChatMsgStyle, 0);
  iniFile.ReadBool("ChatVerticalSpacing", myChatVertSpacing, true);
  iniFile.ReadBool("ChatAppendLinebreak", myChatAppendLineBreak, false);
  iniFile.ReadBool("FlashTaskbar", myFlashTaskbar, true);
  iniFile.ReadBool("MsgWinSticky", myMsgWinSticky, false);
  iniFile.ReadBool("SingleLineChatMode", mySingleLineChatMode, false);
  iniFile.ReadBool("CheckSpellingEnabled", myCheckSpelling, false);
  iniFile.ReadBool("ShowUserPic", myShowUserPic, false);
  iniFile.ReadBool("ShowUserPicHidden", myShowUserPicHidden, false);
  iniFile.ReadStr("DateFormat", szTemp, "hh:mm:ss");
  myChatDateFormat = QString::fromLatin1(szTemp);
  iniFile.ReadNum("HistoryMessageStyle", myHistMsgStyle, 0);
  iniFile.ReadBool("HistoryVerticalSpacing", myHistVertSpacing, true);
  iniFile.ReadBool("HistoryReverse", myReverseHistory, true);
  iniFile.ReadStr("HistoryDateFormat", szTemp, "hh:mm:ss");
  myHistDateFormat = QString::fromLatin1(szTemp);

  iniFile.ReadStr("ReceiveMessageColor", szTemp, "red");
  myRecvColor = QString::fromLatin1(szTemp);
  iniFile.ReadStr("ReceiveHistoryColor", szTemp, "lightpink");
  myRecvHistoryColor = QString::fromLatin1(szTemp);
  iniFile.ReadStr("SentMessageColor", szTemp, "blue");
  mySentColor = QString::fromLatin1(szTemp);
  iniFile.ReadStr("SentHistoryColor", szTemp, "lightblue");
  mySentHistoryColor = QString::fromLatin1(szTemp);
  iniFile.ReadStr("NoticeColor", szTemp, "darkgreen");
  myNoticeColor = QString::fromLatin1(szTemp);
  iniFile.ReadStr("TabOnTypingColor", szTemp, "yellow");
  myTabTypingColor = QString::fromLatin1(szTemp);
  iniFile.ReadStr("ChatBackground", szTemp, "white");
  myChatBackColor = QString::fromLatin1(szTemp);

  iniFile.SetSection("functions");
  iniFile.ReadBool("AutoClose", myAutoClose, true);
  iniFile.ReadNum("AutoPopup", myAutoPopup, 0);
  iniFile.ReadBool("AutoFocus", myAutoFocus, true);
  iniFile.ReadBool("PopupAutoResponse", myPopupAutoResponse, true);

  iniFile.SetSection("locale");
  iniFile.ReadStr("DefaultEncoding", szTemp, "");
  myDefaultEncoding = szTemp;
  iniFile.ReadBool("ShowAllEncodings", myShowAllEncodings, false);

  iniFile.SetSection("geometry");
  short xPos, yPos, wVal, hVal;
  iniFile.ReadNum("EventDialog.X", xPos, 0);
  iniFile.ReadNum("EventDialog.Y", yPos, 0);
  iniFile.ReadNum("EventDialog.W", wVal, 0);
  iniFile.ReadNum("EventDialog.H", hVal, 0);
  if (xPos > QApplication::desktop()->width() - 16)
    xPos = 0;
  if (yPos > QApplication::desktop()->height() - 16)
    yPos = 0;
  myDialogRect.setRect(xPos, yPos, wVal, hVal);
}

void Config::Chat::saveConfiguration(CIniFile& iniFile) const
{
  iniFile.SetSection("appearance");
  iniFile.WriteBool("ManualNewUser", myManualNewUser);
  iniFile.WriteBool("SendFromClipboard", mySendFromClipboard);
  iniFile.WriteBool("MsgChatView", myMsgChatView);
  iniFile.WriteBool("TabbedChatting", myTabbedChatting);
  iniFile.WriteBool("ShowHistory", myShowHistory);
  iniFile.WriteBool("ShowNotices", myShowNotices);
  iniFile.WriteBool("AutoPosReplyWin", myAutoPosReplyWin);
  iniFile.WriteBool("AutoSendThroughServer", myAutoSendThroughServer);
  iniFile.WriteBool("NoSendClose", !myShowSendClose);
  iniFile.WriteBool("FlashTaskbar", myFlashTaskbar);
  iniFile.WriteBool("MsgWinSticky", myMsgWinSticky);
  iniFile.WriteBool("SingleLineChatMode", mySingleLineChatMode);
  iniFile.WriteBool("CheckSpellingEnabled", myCheckSpelling);
  iniFile.WriteBool("ShowUserPic", myShowUserPic);
  iniFile.WriteBool("ShowUserPicHidden", myShowUserPicHidden);

  iniFile.WriteNum("ChatMessageStyle", myChatMsgStyle);
  iniFile.WriteBool("ChatVerticalSpacing", myChatVertSpacing);
  iniFile.WriteBool("ChatAppendLinebreak", myChatAppendLineBreak);
  iniFile.WriteStr("ReceiveMessageColor", myRecvColor.toLatin1());
  iniFile.WriteStr("ReceiveHistoryColor", myRecvHistoryColor.toLatin1());
  iniFile.WriteStr("SentMessageColor", mySentColor.toLatin1());
  iniFile.WriteStr("SentHistoryColor", mySentHistoryColor.toLatin1());
  iniFile.WriteStr("NoticeColor", myNoticeColor.toLatin1());
  iniFile.WriteStr("TabOnTypingColor", myTabTypingColor.toLatin1());
  iniFile.WriteStr("ChatBackground", myChatBackColor.toLatin1());
  iniFile.WriteStr("DateFormat", myChatDateFormat.toLatin1());
  iniFile.WriteNum("HistoryMessageStyle", myHistMsgStyle);
  iniFile.WriteBool("HistoryVerticalSpacing", myHistVertSpacing);
  iniFile.WriteBool("HistoryReverse", myReverseHistory);
  iniFile.WriteStr("HistoryDateFormat", myHistDateFormat.toLatin1());

  iniFile.SetSection("functions");
  iniFile.WriteBool("AutoClose", myAutoClose);
  iniFile.WriteNum("AutoPopup", myAutoPopup);
  iniFile.WriteBool("AutoFocus", myAutoFocus);
  iniFile.WriteBool("PopupAutoResponse", myPopupAutoResponse);

  iniFile.SetSection("locale");
  iniFile.WriteStr("DefaultEncoding", myDefaultEncoding);
  iniFile.WriteBool("ShowAllEncodings", myShowAllEncodings);

  iniFile.SetSection("geometry");
  iniFile.WriteNum("EventDialog.X", static_cast<short>(myDialogRect.x()));
  iniFile.WriteNum("EventDialog.Y", static_cast<short>(myDialogRect.y()));
  iniFile.WriteNum("EventDialog.W", static_cast<short>(myDialogRect.width()));
  iniFile.WriteNum("EventDialog.H", static_cast<short>(myDialogRect.height()));
}

void Config::Chat::blockUpdates(bool block)
{
  myBlockUpdates = block;

  if (block)
    return;

  if (myColorsHaveChanged)
  {
    myColorsHaveChanged = false;
    emit chatColorsChanged();
  }
}

void Config::Chat::setAutoClose(bool autoClose)
{
  if (autoClose == myAutoClose)
    return;

  myAutoClose = autoClose;
}

void Config::Chat::setAutoPopup(unsigned short autoPopup)
{
  if (autoPopup == myAutoPopup)
    return;

  myAutoPopup = autoPopup;
}

void Config::Chat::setAutoFocus(bool autoFocus)
{
  if (autoFocus == myAutoFocus)
    return;

  myAutoFocus = autoFocus;
}

void Config::Chat::setManualNewUser(bool manualNewUser)
{
  if (manualNewUser == myManualNewUser)
    return;

  myManualNewUser = manualNewUser;
}

void Config::Chat::setSendFromClipboard(bool sendFromClipboard)
{
  if (sendFromClipboard == mySendFromClipboard)
    return;

  mySendFromClipboard = sendFromClipboard;
}

void Config::Chat::setMsgChatView(bool msgChatView)
{
  if (msgChatView == myMsgChatView)
    return;

  myMsgChatView = msgChatView;
}

void Config::Chat::setShowAllEncodings(bool showAllEncodings)
{
  if (showAllEncodings == myShowAllEncodings)
    return;

  myShowAllEncodings = showAllEncodings;
}

void Config::Chat::setDefaultEncoding(QByteArray defaultEncoding)
{
  if (defaultEncoding == myDefaultEncoding)
    return;

  myDefaultEncoding = defaultEncoding;
}

void Config::Chat::setTabbedChatting(bool tabbedChatting)
{
  if (tabbedChatting == myTabbedChatting)
    return;

  myTabbedChatting = tabbedChatting;
}

void Config::Chat::setShowHistory(bool showHistory)
{
  if (showHistory == myShowHistory)
    return;

  myShowHistory = showHistory;
}

void Config::Chat::setShowNotices(bool showNotices)
{
  if (showNotices == myShowNotices)
    return;

  myShowNotices = showNotices;
}

void Config::Chat::setShowUserPic(bool showUserPic)
{
  if (showUserPic == myShowUserPic)
    return;

  myShowUserPic = showUserPic;
}

void Config::Chat::setShowUserPicHidden(bool showUserPicHidden)
{
  if (showUserPicHidden == myShowUserPicHidden)
    return;

  myShowUserPicHidden = showUserPicHidden;
}

void Config::Chat::setPopupAutoResponse(bool popupAutoResponse)
{
  if (popupAutoResponse == myPopupAutoResponse)
    return;

  myPopupAutoResponse = popupAutoResponse;
}

void Config::Chat::setAutoPosReplyWin(bool autoPosReplyWin)
{
  if (autoPosReplyWin == myAutoPosReplyWin)
    return;

  myAutoPosReplyWin = autoPosReplyWin;
}

void Config::Chat::setAutoSendThroughServer(bool autoSendThroughServer)
{
  if (autoSendThroughServer == myAutoSendThroughServer)
    return;

  myAutoSendThroughServer = autoSendThroughServer;
}

void Config::Chat::setShowSendClose(bool showSendClose)
{
  if (showSendClose == myShowSendClose)
    return;

  myShowSendClose = showSendClose;
}

void Config::Chat::setChatVertSpacing(bool chatVertSpacing)
{
  if (chatVertSpacing == myChatVertSpacing)
    return;

  myChatVertSpacing = chatVertSpacing;
}

void Config::Chat::setChatAppendLineBreak(bool chatAppendLineBreak)
{
  if (chatAppendLineBreak == myChatAppendLineBreak)
    return;

  myChatAppendLineBreak = chatAppendLineBreak;
}

void Config::Chat::setFlashTaskbar(bool flashTaskbar)
{
  if (flashTaskbar == myFlashTaskbar)
    return;

  myFlashTaskbar = flashTaskbar;
}

void Config::Chat::setMsgWinSticky(bool msgWinSticky)
{
  if (msgWinSticky == myMsgWinSticky)
    return;

  myMsgWinSticky = msgWinSticky;
}

void Config::Chat::setSingleLineChatMode(bool singleLineChatMode)
{
  if (singleLineChatMode == mySingleLineChatMode)
    return;

  mySingleLineChatMode = singleLineChatMode;
}

void Config::Chat::setCheckSpelling(bool checkSpelling)
{
  if (checkSpelling == myCheckSpelling)
    return;

  myCheckSpelling = checkSpelling;
}

void Config::Chat::setHistVertSpacing(bool histVertSpacing)
{
  if (histVertSpacing == myHistVertSpacing)
    return;

  myHistVertSpacing = histVertSpacing;
}

void Config::Chat::setReverseHistory(bool reverseHistory)
{
  if (reverseHistory == myReverseHistory)
    return;

  myReverseHistory = reverseHistory;
}

void Config::Chat::setChatMsgStyle(unsigned short chatMsgStyle)
{
  if (chatMsgStyle == myChatMsgStyle)
    return;

  myChatMsgStyle = chatMsgStyle;
}

void Config::Chat::setHistMsgStyle(unsigned short histMsgStyle)
{
  if (histMsgStyle == myHistMsgStyle)
    return;

  myHistMsgStyle = histMsgStyle;
}

void Config::Chat::setChatDateFormat(QString chatDateFormat)
{
  if (chatDateFormat == myChatDateFormat)
    return;

  myChatDateFormat = chatDateFormat;
}

void Config::Chat::setHistDateFormat(QString histDateFormat)
{
  if (histDateFormat == myHistDateFormat)
    return;

  myHistDateFormat = histDateFormat;
}

void Config::Chat::setRecvHistoryColor(QString recvHistoryColor)
{
  if (recvHistoryColor == myRecvHistoryColor)
    return;

  myRecvHistoryColor = recvHistoryColor;
  changeChatColors();
}

void Config::Chat::setSentHistoryColor(QString sentHistoryColor)
{
  if (sentHistoryColor == mySentHistoryColor)
    return;

  mySentHistoryColor = sentHistoryColor;
  changeChatColors();
}

void Config::Chat::setRecvColor(QString recvColor)
{
  if (recvColor == myRecvColor)
    return;

  myRecvColor = recvColor;
  changeChatColors();
}

void Config::Chat::setSentColor(QString sentColor)
{
  if (sentColor == mySentColor)
    return;

  mySentColor = sentColor;
  changeChatColors();
}

void Config::Chat::setNoticeColor(QString noticeColor)
{
  if (noticeColor == myNoticeColor)
    return;

  myNoticeColor = noticeColor;
  changeChatColors();
}

void Config::Chat::setTabTypingColor(QString tabTypingColor)
{
  if (tabTypingColor == myTabTypingColor)
    return;

  myTabTypingColor = tabTypingColor;
  changeChatColors();
}

void Config::Chat::setChatBackColor(QString chatBackColor)
{
  if (chatBackColor == myChatBackColor)
    return;

  myChatBackColor = chatBackColor;
  changeChatColors();
}

void Config::Chat::setDialogRect(const QRect& geometry)
{
  if (geometry.isValid())
    myDialogRect = geometry;
}

void Config::Chat::changeChatColors()
{
  if (myBlockUpdates)
    myColorsHaveChanged = true;
  else
    emit chatColorsChanged();
}
