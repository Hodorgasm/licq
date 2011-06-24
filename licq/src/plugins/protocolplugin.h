/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2011 Licq developers
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

#ifndef LICQDAEMON_PROTOCOLPLUGIN_H
#define LICQDAEMON_PROTOCOLPLUGIN_H

#include <licq/protocolplugin.h>

#include <queue>

#include <licq/thread/mutex.h>

namespace Licq
{

class ProtocolPlugin::Private
{
public:

private:
  unsigned long myProtocolId;
  std::string myDefaultHost;
  int myDefaultPort;

  std::queue<Licq::ProtocolSignal*> mySignals;
  Licq::Mutex mySignalsMutex;

  const char* (*myPpid)();
  unsigned long (*mySendFunctions)();

  friend class ProtocolPlugin;
};

} // namespace Licq

#endif
