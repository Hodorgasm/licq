/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2012-2013 Licq developers <licq-dev@googlegroups.com>
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

#include "owner.h"

#include <licq/inifile.h>

using namespace LicqMsn;

Owner::Owner(const Licq::UserId& id)
  : Licq::User(id, false), Licq::Owner(id), User(id, false)
{
  Licq::IniFile& conf(userConf());
  conf.get("ListVersion", myListVersion, 0);
}

Owner::~Owner()
{
  // Empty
}

void Owner::saveOwnerInfo()
{
  Licq::Owner::saveOwnerInfo();

  Licq::IniFile& conf(userConf());
  conf.set("ListVersion", myListVersion);
}
