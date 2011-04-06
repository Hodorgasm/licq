/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2010 Licq developers
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

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>

#include <licq/pluginbase.h>

#include "forwarder.h"
#include "pluginversion.h"

CLicqForwarder *licqForwarder;

const char *LP_Usage()
{
  static const char usage[] =
    "Usage:  Licq [options] -p forwarder -- [ -h ] [ -e ] [ -l <status> ] [ -d ]\n"
    "         -h          : help\n"
    "         -e          : start enabled\n"
    "         -l <status> : log on at startup\n"
    "         -d          : delete new messages after forwarding\n";
  return usage;
}

const char *LP_Name()
{
  static const char name[] = "ICQ Forwarder";
  return name;
}


const char *LP_Description()
{
  static const char desc[] = "ICQ message forwarder to email/icq";
  return desc;
}


const char *LP_Version()
{
  static const char version[] = PLUGIN_VERSION_STRING;
  return version;
}

const char *LP_Status()
{
  static const char enabled[] = "forwarding enabled";
  static const char disabled[] = "forwarding disabled";
  return licqForwarder->Enabled() ? enabled : disabled;
}

const char *LP_ConfigFile()
{
  return "licq_forwarder.conf";
}


bool LP_Init(int argc, char **argv)
{
  //char *LocaleVal = new char;
  //LocaleVal = setlocale (LC_ALL, "");
  //bindtextdomain (PACKAGE, LOCALEDIR);
  //textdomain (PACKAGE);

  // parse command line for arguments
  bool bEnable = false, bDelete = false;
  std::string startupStatus;
  int i = 0;
  while( (i = getopt(argc, argv, "hel:d")) > 0)
  {
    switch (i)
    {
    case 'h':  // help
      puts(LP_Usage());
      return false;
    case 'e': // enable
      bEnable = true;
      break;
    case 'l': //log on
        startupStatus = optarg;
      break;
    case 'd':
      bDelete = true;
      break;
    }
  }
  licqForwarder = new CLicqForwarder(bEnable, bDelete, startupStatus);
  return (licqForwarder != NULL);
}


int LP_Main()
{
  int nResult = licqForwarder->Run();
  licqForwarder->Shutdown();
  delete licqForwarder;
  return nResult;
}

