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
#include "systemtrayicon.h"

#include "config.h"

#include "config/general.h"

using namespace LicqQtGui;

SystemTrayIcon::SystemTrayIcon(QMenu* menu)
  : DockIcon(),
    myTimerToggle(false)
{
  myTrayIcon = new QSystemTrayIcon(this);
  myTrayIcon->setContextMenu(menu);
  connect(myTrayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
      SLOT(trayActivated(QSystemTrayIcon::ActivationReason)));
  connect(this, SIGNAL(newToolTip(QString)), SLOT(updateToolTip(QString)));

  updateConfig();

  updateIconStatus();

  myTrayIcon->show();
}

void SystemTrayIcon::updateIconMessages(int newMsg, int sysMsg)
{
  static int timerId = 0;

  bool blink = (myBlink && (newMsg > 0 || sysMsg > 0));

  if (blink && timerId == 0)
  {
    timerId = startTimer(500);
  }
  else if (!blink && timerId != 0)
  {
    killTimer(timerId);
    timerId = 0;
  }

  DockIcon::updateIconMessages(newMsg, sysMsg);
}

void SystemTrayIcon::updateStatusIcon()
{
  DockIcon::updateStatusIcon();
  updateIcon();
}

void SystemTrayIcon::updateEventIcon()
{
  DockIcon::updateEventIcon();
  updateIcon();
}

void SystemTrayIcon::updateConfig()
{
  myBlink = Config::General::instance()->trayBlink();
  updateIconMessages(myNewMsg, mySysMsg);
}

void SystemTrayIcon::timerEvent(QTimerEvent* /* event */)
{
  myTimerToggle = !myTimerToggle;
  updateIcon();
}

void SystemTrayIcon::updateIcon()
{
  if (myEventIcon == NULL ||
      myEventIcon->isNull() ||
      (myBlink && myTimerToggle))
    myTrayIcon->setIcon(*myStatusIcon);
  else
    myTrayIcon->setIcon(*myEventIcon);
}

void SystemTrayIcon::trayActivated(QSystemTrayIcon::ActivationReason reason)
{
  switch (reason)
  {
    case QSystemTrayIcon::Trigger:
      emit clicked();
      break;

    case QSystemTrayIcon::MiddleClick:
      emit middleClicked();
      break;

    default:
      break;
  }
}

void SystemTrayIcon::updateToolTip(QString toolTip)
{
  myTrayIcon->setToolTip(toolTip);
}
