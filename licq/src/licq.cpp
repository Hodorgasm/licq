// -*- c-basic-offset: 2 -*-
/* ----------------------------------------------------------------------------
 * Licq - A ICQ Client for Unix
 * Copyright (C) 1998 - 2009 Licq developers
 *
 * This program is licensed under the terms found in the LICENSE file.
 */

#include "config.h"

#include <boost/foreach.hpp>
#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <fcntl.h>
#include <getopt.h>
#include <iostream>
#include <pwd.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "licq.h"
#include <licq_constants.h>
#include "licq_log.h"
#include "licq/exceptions/exception.h"
#include <licq/inifile.h>
#include <licq/utility.h>
#include "licq/version.h"

#include "contactlist/usermanager.h"
#include "daemon.h"
#include "fifo.h"
#include "gettext.h"
#include "icq/icq.h"
#include "logging/streamlogsink.h"
#include "oneventmanager.h"
#include "plugins/pluginmanager.h"
#include "sarmanager.h"
#include "statistics.h"
#include "support.h"

using namespace std;
using Licq::GeneralPlugin;
using Licq::ProtocolPlugin;
using LicqDaemon::Daemon;
using LicqDaemon::PluginManager;
using LicqDaemon::gDaemon;
using LicqDaemon::gFifo;
using LicqDaemon::gOnEventManager;
using LicqDaemon::gSarManager;
using LicqDaemon::gPluginManager;
using LicqDaemon::gStatistics;
using LicqDaemon::gUserManager;
using Licq::gUtilityManager;

/*-----Start OpenSSL code--------------------------------------------------*/

#ifdef USE_OPENSSL
#include <openssl/ssl.h>
#include <openssl/dh.h>
#include <openssl/opensslv.h>

extern SSL_CTX *gSSL_CTX;
extern SSL_CTX *gSSL_CTX_NONICQ;

// AUTOGENERATED by dhparam
static DH *get_dh512()
        {
        static unsigned char dh512_p[]={
                0xFF,0xD3,0xF9,0x7C,0xEB,0xFE,0x45,0x2E,0x47,0x41,0xC1,0x8B,
                0xF7,0xB9,0xC6,0xF2,0x40,0xCF,0x10,0x8B,0xF3,0xD7,0x08,0xC7,
                0xF0,0x3F,0x46,0x7A,0xAD,0x71,0x6A,0x70,0xE1,0x76,0x8F,0xD9,
                0xD4,0x46,0x70,0xFB,0x31,0x9B,0xD8,0x86,0x58,0x03,0xE6,0x6F,
                0x08,0x9B,0x16,0xA0,0x78,0x70,0x6C,0xB1,0x78,0x73,0x52,0x3F,
                0xD2,0x74,0xED,0x9B,
                };
        static unsigned char dh512_g[]={
                0x02,
                };
        DH *dh;

        if ((dh=DH_new()) == NULL) return(NULL);
        dh->p=BN_bin2bn(dh512_p,sizeof(dh512_p),NULL);
        dh->g=BN_bin2bn(dh512_g,sizeof(dh512_g),NULL);
        if ((dh->p == NULL) || (dh->g == NULL))
                { DH_free(dh); return(NULL); }
        return(dh);
        }

#ifdef SSL_DEBUG
void ssl_info_callback(SSL *s, int where, int ret)
{
    const char *str;
    int w;

    w = where & ~SSL_ST_MASK;

    if (w & SSL_ST_CONNECT) str="SSL_connect";
    else if (w & SSL_ST_ACCEPT) str="SSL_accept";
    else str="undefined";

    if (where & SSL_CB_LOOP)
    {
        gLog.Info("%s%s:%s\n",L_SSLxSTR,str,SSL_state_string_long(s));
    }
    else if (where & SSL_CB_ALERT)
    {
        str=(where & SSL_CB_READ)?"read":"write";
        gLog.Info("%sSSL3 alert %s:%s:%s\n",L_SSLxSTR,
            str,
            SSL_alert_type_string_long(ret),
            SSL_alert_desc_string_long(ret));
    }
    else if (where & SSL_CB_EXIT)
    {
        if (ret == 0)
            gLog.Info("%s%s:failed in %s\n",L_SSLxSTR,
                str,SSL_state_string_long(s));
        else if (ret < 0)
        {
        gLog.Info("%s%s:%s\n",L_SSLxSTR,str,SSL_state_string_long(s));
        }
    }
    else if (where & SSL_CB_ALERT)
    {
        str=(where & SSL_CB_READ)?"read":"write";
        gLog.Info("%sSSL3 alert %s:%s:%s\n",L_SSLxSTR,
            str,
            SSL_alert_type_string_long(ret),
            SSL_alert_desc_string_long(ret));
    }
    else if (where & SSL_CB_EXIT)
    {
        if (ret == 0)
            gLog.Info("%s%s:failed in %s\n",L_SSLxSTR,
                str,SSL_state_string_long(s));
        else if (ret < 0)
        {
            gLog.Info("%s%s:error in %s\n",L_SSLxSTR,
                str,SSL_state_string_long(s));
        }
    }
}
#endif
#endif
/*-----End of OpenSSL code-------------------------------------------------*/

// Sets up BASE_DIR to be the absolute path to the user's config base
// directory. BASE_DIR will both start and end with a slash (/).
static bool setupBaseDirPath(const char* path)
{
  const size_t len = ::strlen(path);

  // Add slash to the end if needed
  const char* slash = (len > 0 && path[len - 1] != '/') ? "/" : "";

  int ret;
  if (len > 0 && path[0] == '/')
    ret = ::snprintf(BASE_DIR, MAX_FILENAME_LEN, "%s%s", path, slash);
  else
  {
    // Get current working directory
    // Linux uses PATH_MAX, BSD uses MAXPATHLEN, either seems to work on both
    char cwd[PATH_MAX];
    if (::getcwd(cwd, sizeof(cwd)) == NULL)
    {
      ::fprintf(stderr, tr("Could not get current working directory\n"));
      return false;
    }

    // Tidy up path if it is a "simple" relative path
    if (::strncmp(path, "./", 2) == 0)
      path += 2;

    // Construct an absolute path
    ret = ::snprintf(BASE_DIR, MAX_FILENAME_LEN, "%s/%s%s", cwd, path, slash);
  }

  if (ret <= 0 || ret >= MAX_FILENAME_LEN)
  {
    ::fprintf(stderr, tr("Could not create config base directory path\n"));
    return false;
  }

  return true;
}

/**
 * Prints the @a error to stderr (by means of gLog), and if the user is running
 * X, tries to show a dialog with the error.
 */
void displayFatalError(const char* error, int useLicqLog)
{
  if (useLicqLog)
    gLog.Error(error);
  else
    fprintf(stderr, "\n%s\n", error);

  // Try to show the error if we're running X
  if (getenv("DISPLAY") != NULL)
  {
    pid_t child = fork();
    if (child == 0)
    {
      // execlp never returns (except on error).
      execlp("kdialog", "kdialog", "--error", error, NULL);
      execlp("Xdialog", "Xdialog", "--title", "Error", "--msgbox", error, "0", "0", NULL);
      execlp("xmessage", "xmessage", "-center", error, NULL);

      exit(EXIT_FAILURE);
    }
    else if (child != -1)
    {
      int status;
      waitpid(child, &status, 0);
    }
  }
}

void handleExitSignal(int signal)
{
  gLog.Info(tr("%sReceived signal %d, exiting.\n"), L_ENDxSTR, signal);
  gDaemon.Shutdown();
}

/*-----Helper functions for CLicq::UpgradeLicq-----------------------------*/
int SelectUserUtility(const struct dirent *d)
{
  const char* pcDot = strrchr(d->d_name, '.');
  if (pcDot == NULL) return (0);
  return (strcmp(pcDot, ".uin") == 0);
}

int SelectHistoryUtility(const struct dirent *d)
{
  const char* pcDot = strchr(d->d_name, '.');
  if (pcDot == NULL) return (0);
  return (strcmp(pcDot, ".history") == 0 ||
          strcmp(pcDot, ".history.removed") == 0);
}

char **global_argv = NULL;
int global_argc = 0;

CLicq::CLicq() :
  myConsoleLogLevel(0)
{
}

bool CLicq::Init(int argc, char **argv)
{
  myConsoleLog.reset(new LicqDaemon::StreamLogSink(std::cerr));
  myConsoleLog->setLogLevel(Licq::Log::Error, true);
  myConsoleLog->setLogLevel(Licq::Log::Warning, true);
  myLogService.registerLogSink(myConsoleLog);

  char *szRedirect = NULL;
  vector <char *> vszPlugins;
  vector <char *> vszProtoPlugins;

  // parse command line for arguments
  bool bHelp = false;
  bool bFork = false;
  bool bBaseDir = false;
  bool bForceInit = false;
  bool bCmdLinePlugins = false;
  bool bCmdLineProtoPlugins = false;
  bool bRedirect_ok = false;
  bool bUseColor = true;
  // Check the no one is trying session management on us
  if (argc > 1 && strcmp(argv[1], "-session") == 0)
  {
    fprintf(stderr, tr("Session management is not supported by Licq.\n"));
  }
  else
  {
    int i = 0;
#ifdef __GLIBC__
    while( (i = getopt(argc, argv, "--hd:b:p:l:Io:fcv")) > 0)
#else
    while( (i = getopt(argc, argv, "hd:b:p:l:Io:fcv")) > 0)
#endif
    {
      switch (i)
      {
      case 'h':  // help
        PrintUsage();
        bHelp = true;
        break;
      case 'b':  // base directory
        if (!setupBaseDirPath(optarg))
          return false;
        bBaseDir = true;
        break;
      case 'd':  // debug level
        myConsoleLogLevel = atol(optarg);
        break;
      case 'c':  // use color
        bUseColor = false;
        break;
      case 'I':  // force init
        bForceInit = true;
        break;
      case 'p':  // new plugin
        vszPlugins.push_back(strdup(optarg));
        bCmdLinePlugins = true;
        break;
      case 'l':  // new protocol plugin
        vszProtoPlugins.push_back(strdup(optarg));
        bCmdLineProtoPlugins = true;
        break;
      case 'o':  // redirect stderr
        szRedirect = strdup(optarg);
        break;
      case 'f':  // fork
        bFork = true;
        break;
      case 'v':  // show version
        printf(tr("%s version %s, compiled on %s\n"), PACKAGE, LICQ_VERSION_STRING, __DATE__);
        return false;
        break;
      }
    }
  }

  // Save the command line arguments in case anybody cares
  global_argc = argc;
  global_argv = argv;

  // Fork into the background
  if (bFork && fork()) exit(0);

  // See if redirection works, set bUseColor to false if we redirect
  // to a file.
  if (szRedirect)
    bRedirect_ok = Redirect(szRedirect);

  if(!isatty(STDERR_FILENO))
    bUseColor = false;

  // Dump all initial errors and warnings to the console log regardless of the
  // requested debug level.
  myConsoleLog->setLogLevels(myConsoleLogLevel);
  myConsoleLog->setLogLevel(Licq::Log::Warning, true);
  myConsoleLog->setLogLevel(Licq::Log::Error, true);
  myConsoleLog->setUseColors(bUseColor);

  // Redirect stdout and stderr if asked to
  if (szRedirect) {
    if (bRedirect_ok)
      gLog.Info(tr("%sOutput redirected to \"%s\".\n"), L_INITxSTR, szRedirect);
    else
      gLog.Warn(tr("%sRedirection to \"%s\" failed:\n%s%s.\n"), L_WARNxSTR,
                szRedirect, L_BLANKxSTR, strerror(errno));
    free (szRedirect);
    szRedirect = NULL;
  }

  // if no base directory set on the command line then get it from HOME
  if (!bBaseDir)
  {
    const char* home = ::getenv("HOME");
    if (home == NULL || home[0] != '/')
    {
      gLog.error(tr("$HOME not set or invalid; "
                    "unable to determine config base directory"));
      return false;
    }
    int ret = ::snprintf(BASE_DIR, MAX_FILENAME_LEN, "%s/.licq/", home);
    if (ret <= 0 || ret >= MAX_FILENAME_LEN)
    {
      gLog.error(tr("Could not create config base directory path"));
      return false;
    }
  }

  // check if user has conf files installed, install them if not
  if ( (access(BASE_DIR, F_OK) < 0 || bForceInit) && !Install() )
    return false;

  // Define the directory for all the shared data
  strncpy(SHARE_DIR, INSTALL_SHAREDIR, MAX_FILENAME_LEN);
  SHARE_DIR[MAX_FILENAME_LEN - 1] = '\0';

  strncpy(LIB_DIR, INSTALL_LIBDIR, MAX_FILENAME_LEN);
  LIB_DIR[MAX_FILENAME_LEN - 1] = '\0';

  // FIXME: ICQ should be put into its own plugin. This is just a dummy
  // plugin. It can't really be stopped...
  gPluginManager.loadProtocolPlugin("", true, true);

  // Load up the plugins
  vector <char *>::iterator iter;
  for (iter = vszPlugins.begin(); iter != vszPlugins.end(); ++iter)
  {
    GeneralPlugin::Ptr plugin = LoadPlugin(*iter, argc, argv, !bHelp);
    if (!plugin)
      return false;
    if (bHelp)
    {
      fprintf(stderr,
              "----------\nLicq Plugin: %s %s\n%s\n",
              plugin->getName(),
              plugin->getVersion(),
              plugin->getUsage());
    }
    free(*iter);
  }
  for (iter = vszProtoPlugins.begin(); iter != vszProtoPlugins.end(); ++iter)
  {
    ProtocolPlugin::Ptr plugin = LoadProtoPlugin(*iter, !bHelp);
    if (!plugin)
      return false;
    if (bHelp)
    {
      fprintf(stderr,
              "----------\nLicq Protocol Plugin: %s %s\n",
              plugin->getName(),
              plugin->getVersion());
    }
    free(*iter);
  }
  if (bHelp) return false;

  // Check pid
  // We do this by acquiring a write lock on the pid file and never closing the file.
  // When Licq is killed (normally or abnormally) the file will be closed by the operating
  // system and the lock released.
  char szKey[32];
  string pidfile = BASE_DIR;
  pidfile += "licq.pid";

  // Never close pidFile!
  int pidFile = open(pidfile.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
  if (pidFile < 0)
  {
    // We couldn't open (or create) the file for writing.
    // If the file doesn't exists we continue without lockfile protection.
    // If it does exist, we bail out.
    struct stat buf;
    if (stat(pidfile.c_str(), &buf) < 0 && errno == ENOENT)
    {
      gLog.Warn(tr("%sLicq: %s cannot be opened for writing.\n"
                   "%s      skipping lockfile protection.\n"),
          L_WARNxSTR, pidfile.c_str(), L_BLANKxSTR);
    }
    else
    {
      const size_t ERR_SIZE = 511;
      char error[ERR_SIZE + 1];

      // Try to read the pid of running Licq instance.
      FILE* fs = fopen(pidfile.c_str(), "r");
      if (fs != NULL)
      {
          fgets(szKey, 32, fs);
          pid_t pid = atol(szKey);

          snprintf(error, ERR_SIZE,
                   tr("%sLicq: Already running at pid %d.\n"
                      "%s      Kill process or remove %s.\n"),
            L_ERRORxSTR, pid, L_BLANKxSTR, pidfile.c_str());
          fclose(fs);
      }
      else
      {
        snprintf(error, ERR_SIZE,
                 tr("%sLicq: Unable to determine pid of running Licq instance.\n"),
                 L_ERRORxSTR);
      }

      error[ERR_SIZE] = '\0';
      displayFatalError(error, 1);

      return false;
    }
  }
  else
  {
    struct flock lock;
    lock.l_type = F_WRLCK; // Write lock is exclusive lock
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0; // Lock file through to the end of file

    if (fcntl(pidFile, F_SETLK, &lock) != 0)
    {
      // Failed to get the lock => Licq is already running
      const size_t ERR_SIZE = 511;
      char error[ERR_SIZE + 1];

      if (fcntl(pidFile, F_GETLK, &lock) != 0)
      {
        snprintf(error, ERR_SIZE,
                tr("%sLicq: Unable to determine pid of running Licq instance.\n"),
                L_ERRORxSTR);
      }
      else
      {
        snprintf(error, ERR_SIZE,
                tr("%sLicq: Already running at pid %d.\n"),
                L_ERRORxSTR, lock.l_pid);
      }

      error[ERR_SIZE] = '\0';
      displayFatalError(error, 1);

      return false;
    }
    else
    {
      // Save our pid in the file
      ftruncate(pidFile, 0);
      int size = snprintf(szKey, 32, "%d\n", getpid());
      write(pidFile, szKey, (size > 32 ? 32 : size));
    }
  }

  // Open the config file
  Licq::IniFile licqConf("licq.conf");
  if (!licqConf.loadFile())
  {
    gLog.error("Could not load config file '%s'", licqConf.filename().c_str());
    return false;
  }

  // Verify the version
  licqConf.setSection("licq");
  unsigned nVersion;
  licqConf.get("Version", nVersion, 0);
  if (nVersion < LICQ_MAKE_VERSION(1, 2, 8))
  {
    gLog.Info("%sUpgrading config file formats.\n", L_SBLANKxSTR);
    if (upgradeLicq128(licqConf))
      gLog.Info("%sUpgrade completed.\n", L_SBLANKxSTR);
    else
    {
      gLog.Warn("%sUpgrade failed. Please save your licq directory and\n"
                "%sreport this as a bug.\n", L_ERRORxSTR, L_BLANKxSTR);
      return false;
    }
  }
  else if (nVersion < LICQ_VERSION)
  {
    licqConf.set("Version", LICQ_VERSION);
    licqConf.writeFile();
  }

  // Find and load the protocol plugins before the UI plugins
  if (!bHelp && !bCmdLineProtoPlugins)
  {
    unsigned nNumProtoPlugins = 0;
    if (licqConf.setSection("plugins", false) && licqConf.get("NumProtoPlugins", nNumProtoPlugins) && nNumProtoPlugins > 0)
    {
      char szKey[20];
      for (unsigned i = 0; i < nNumProtoPlugins; i++)
      {
        string pluginName;
        sprintf(szKey, "ProtoPlugin%d", i+1);
        if (!licqConf.get(szKey, pluginName))
          continue;
        if (!LoadProtoPlugin(pluginName.c_str()))
          return false;
      }
    }
  }


  // Find and load the plugins from the conf file
  if (!bHelp && !bCmdLinePlugins)
  {
    unsigned nNumPlugins = 0;
    string pluginName;
    if (licqConf.setSection("plugins", false) && licqConf.get("NumPlugins", nNumPlugins) && nNumPlugins > 0)
    {
      for (unsigned i = 0; i < nNumPlugins; i++)
      {
        sprintf(szKey, "Plugin%d", i + 1);
        if (!licqConf.get(szKey, pluginName))
          continue;

        bool loaded = LoadPlugin(pluginName.c_str(), argc, argv);

        // Make upgrade from 1.3.x and older easier by automatically switching from kde/qt-gui to kde4/qt4-gui
        if (!loaded && pluginName == "kde-gui")
        {
          gLog.Warn(tr("%sPlugin kde-gui is no longer available, trying to load kde4-gui instead.\n"), L_WARNxSTR);
          loaded = LoadPlugin("kde4-gui", argc, argv);
        }
        if (!loaded && (pluginName == "qt-gui" || pluginName == "kde-gui"))
        {
          gLog.Warn(tr("%sPlugin %s is no longer available, trying to load qt4-gui instead.\n"), L_WARNxSTR, pluginName.c_str());
          loaded = LoadPlugin("qt4-gui", argc, argv);
        }

        if (!loaded)
          return false;
      }
    }
    else  // If no plugins, try some defaults one by one
    {
      const char* plugins[] = {"qt4-gui", "kde4-gui", "console"};
      unsigned short i = 0, size = sizeof(plugins) / sizeof(char*);

      GeneralPlugin::Ptr plugin;
      while (i < size && !plugin)
        plugin = LoadPlugin(plugins[i++], argc, argv);
      if (!plugin)
        return false;
    }
  }

#ifdef USE_OPENSSL
  // Initialize SSL
  SSL_library_init();
  gSSL_CTX = SSL_CTX_new(TLSv1_method());
  gSSL_CTX_NONICQ = SSL_CTX_new(TLSv1_method());
#if OPENSSL_VERSION_NUMBER >= 0x00905000L
  SSL_CTX_set_cipher_list(gSSL_CTX, "ADH:@STRENGTH");
#else
  SSL_CTX_set_cipher_list(gSSL_CTX, "ADH");
#endif

#ifdef SSL_DEBUG
  SSL_CTX_set_info_callback(gSSL_CTX, (void (*)(const SSL *,int, int))ssl_info_callback);
#endif
  SSL_load_error_strings();

  DH *dh = get_dh512();
  SSL_CTX_set_tmp_dh(gSSL_CTX, dh);
  DH_free(dh);
#endif

  // Start things going
  if (!LicqDaemon::gUserManager.Load())
    return false;
  gDaemon.initialize(this);
  gOnEventManager.initialize();
  gSarManager.initialize();
  gStatistics.initialize();
  string filename = SHARE_DIR;
  filename += Daemon::UtilityDir;
  gUtilityManager.loadUtilities(filename);

  // Create the daemon
  gIcqProtocol.initialize();

  return true;
}

CLicq::~CLicq()
{
  gFifo.shutdown();

  myLogService.unregisterLogSink(myConsoleLog);
}


const char *CLicq::Version()
{
  static const char version[] = LICQ_VERSION_STRING;
  return version;
}


/*-----------------------------------------------------------------------------
 * UpgradeLicq
 *
 * Upgrades the config files to the current version.
 *---------------------------------------------------------------------------*/
bool CLicq::upgradeLicq128(Licq::IniFile& licqConf)
{
  string strBaseDir = BASE_DIR;
  Licq::IniFile ownerFile("owner.uin");
  if (!ownerFile.loadFile())
    return false;

  // Get the UIN
  unsigned long nUin;
  ownerFile.setSection("user");
  ownerFile.get("Uin", nUin, 0);

  // Set the new version number
  licqConf.setSection("licq");
  licqConf.set("Version", LICQ_VERSION);

  // Create the owner section and fill it
  licqConf.setSection("owners");
  licqConf.set("NumOfOwners", 1);
  licqConf.set("Owner1.Id", nUin);
  licqConf.set("Owner1.PPID", "Licq");

  // Add the protocol plugins info
  licqConf.setSection("plugins");
  licqConf.set(string("NumProtoPlugins"), 0);
  licqConf.writeFile();

  // Rename owner.uin to owner.Licq
  if (rename((strBaseDir + "owner.uin").c_str(), (strBaseDir + "owner.Licq").c_str()))
    return false;

  // Update all the user files and update users.conf
  struct dirent **UinFiles;
  string strUserDir = strBaseDir + "users";
  int n = scandir_alpha_r(strUserDir.c_str(), &UinFiles, SelectUserUtility);
  if (n != 0)
  {
    Licq::IniFile userConf("users.conf");
    if (!userConf.loadFile())
      return false;
    userConf.setSection("users");
    userConf.set("NumOfUsers", n);
    for (int i = 0; i < n; i++)
    {
      char szKey[20];
      snprintf(szKey, sizeof(szKey), "User%d", i+1);
      string strFileName = strUserDir + "/" + UinFiles[i]->d_name;
      string strNewName = UinFiles[i]->d_name;
      strNewName.replace(strNewName.find(".uin", 0), 4, ".Licq");
      string strNewFile = strUserDir + "/" + strNewName;
      if (rename(strFileName.c_str(), strNewFile.c_str()))
        return false;
      userConf.set(szKey, strNewName);
    }
    userConf.writeFile();
  }

  // Rename the history files
  struct dirent **HistoryFiles;
  string strHistoryDir = strBaseDir + "history";
  int nNumHistory = scandir_alpha_r(strHistoryDir.c_str(), &HistoryFiles,
    SelectHistoryUtility);
  if (nNumHistory)
  {
    for (unsigned short i = 0; i < nNumHistory; i++)
    {
      string strFileName = strHistoryDir + "/" + HistoryFiles[i]->d_name;
      string strNewFile = strHistoryDir + "/" + HistoryFiles[i]->d_name;
      strNewFile.replace(strNewFile.find(".history", 0), 8, ".Licq.history");
      if (rename(strFileName.c_str(), strNewFile.c_str()))
        return false;
    }
  }
  
  return true;
}

/*-----------------------------------------------------------------------------
 * LoadPlugin
 *
 * Loads the given plugin using the given command line arguments.
 *---------------------------------------------------------------------------*/
GeneralPlugin::Ptr CLicq::
LoadPlugin(const char *_szName, int argc, char **argv, bool keep)
{
  // Set up the argument vector
  static int argcndx = 0;
  int argccnt = 0;
  // Step up to the first delimiter if we have done nothing yet
  if (argcndx == 0)
  {
    while (++argcndx < argc && strcmp(argv[argcndx], "--") != 0)
      ;
  }
  if (argcndx < argc)
  {
    while (++argcndx < argc && strcmp(argv[argcndx], "--") != 0)
      argccnt++;
  }
  return gPluginManager.loadGeneralPlugin(_szName, argccnt,
      &argv[argcndx - argccnt], keep);
}


ProtocolPlugin::Ptr CLicq::
LoadProtoPlugin(const char *_szName, bool keep)
{
  return gPluginManager.loadProtocolPlugin(_szName, keep);
}


int CLicq::Main()
{
  int nResult = 0;

  if (gPluginManager.getGeneralPluginsCount() == 0)
  {
    gLog.Warn(tr("%sNo plugins specified on the command-line (-p option).\n"
                 "%sSee the README for more information.\n"),
              L_WARNxSTR, L_BLANKxSTR);
    return nResult;
  }

  gFifo.initialize();

  if (!gIcqProtocol.start())
    return 1;

  // Run the plugins
  gPluginManager.startAllPlugins();

  // Reset to requested log level
  myConsoleLog->setLogLevels(myConsoleLogLevel);

  try
  {
    bool bDaemonShutdown = false;

    while (true)
    {
      if (bDaemonShutdown)
        gPluginManager.waitForPluginExit(PluginManager::MaxWaitPlugin);
      else
      {
        if (gPluginManager.waitForPluginExit() == 0)
        {
          bDaemonShutdown = true;
          continue;
        }
      }
    }
  }
  catch (const Licq::Exception&)
  {
    // Empty
  }

  gPluginManager.cancelAllPlugins();

  pthread_t* t = gDaemon.Shutdown();
  pthread_join(*t, NULL);

  gUserManager.shutdown();

  return gPluginManager.getGeneralPluginsCount();
}


void CLicq::PrintUsage()
{
  printf(tr("%s version %s.\n"
         "Usage:  Licq [-h] [-d #] [-b configdir] [-I] [-p plugin] [-l protoplugin] [-o file] [-- <plugin #1 parameters>] [-- <plugin #2 parameters>...]\n\n"
         " -h : this help screen (and any plugin help screens as well)\n"
         " -d : set what information is logged to standard output:\n"
         "        1  status information\n"
         "        2  unknown packets\n"
         "        4  errors\n"
         "        8  warnings\n"
         "       16  all packets\n"
         "      add values together for multiple options\n"
         " -c : disable color at standard output\n"
         " -b : set the base directory for the config and data files (~/.licq by default)\n"
         " -I : force initialization of the given base directory\n"
         " -p : load the given plugin library\n"
         " -l : load the given protocol plugin library\n"
         " -o : redirect stderr to <file>, which can be a device (ie /dev/ttyp4)\n"),
         PACKAGE, LICQ_VERSION_STRING);
}


void CLicq::SaveLoadedPlugins()
{
  char szKey[20];

  Licq::IniFile licqConf("licq.conf");
  licqConf.loadFile();

  licqConf.setSection("plugins");

  Licq::GeneralPluginsList general;
  gPluginManager.getGeneralPluginsList(general);

  licqConf.set("NumPlugins", general.size());

  unsigned short i = 1;
  BOOST_FOREACH(GeneralPlugin::Ptr plugin, general)
  {
    sprintf(szKey, "Plugin%d", i++);
    licqConf.set(szKey, plugin->getLibraryName());
  }

  Licq::ProtocolPluginsList protocols;
  gPluginManager.getProtocolPluginsList(protocols);

  licqConf.set("NumProtoPlugins", (protocols.size() - 1));

  i = 1;
  BOOST_FOREACH(ProtocolPlugin::Ptr plugin, protocols)
  {
    if (!plugin->getLibraryName().empty())
    {
      sprintf(szKey, "ProtoPlugin%d", i++);
      licqConf.set(szKey, plugin->getLibraryName());
    }
  }

  licqConf.writeFile();
}


void CLicq::ShutdownPlugins()
{
  // Save plugins
  if (gPluginManager.getGeneralPluginsCount() > 0)
    SaveLoadedPlugins();

  gPluginManager.shutdownAllPlugins();
}


bool CLicq::Install()
{
  // Create the directory if necessary
  if (mkdir(BASE_DIR, 0700) == -1 && errno != EEXIST)
  {
    fprintf(stderr, "Couldn't mkdir %s: %s\n", BASE_DIR, strerror(errno));
    return (false);
  }
  string cmd = BASE_DIR;
  cmd += "users";
  if (mkdir(cmd.c_str(), 0700) == -1 && errno != EEXIST)
  {
    fprintf(stderr, "Couldn't mkdir %s: %s\n", cmd.c_str(), strerror(errno));
    return (false);
  }

  // Create licq.conf
  Licq::IniFile licqConf("licq.conf");
  licqConf.loadFile();
  licqConf.setSection("licq");
  licqConf.set("Version", LICQ_VERSION);
  licqConf.setSection("plugins");
  licqConf.set(string("NumPlugins"), 0);
  licqConf.set(string("NumProtoPlugins"), 0);
  licqConf.setSection("network");
  licqConf.setSection("onevent");
  licqConf.setSection("groups");
  licqConf.set(string("NumOfGroups"), 0);
  licqConf.setSection("owners");
  licqConf.set(string("NumOfOwners"), 0);
  licqConf.writeFile();

  // Create users.conf
  Licq::IniFile usersConf("users.conf");
  usersConf.loadFile();
  usersConf.setSection("users");
  usersConf.set(string("NumOfUsers"), 0);
  usersConf.writeFile();

  return(true);
}
