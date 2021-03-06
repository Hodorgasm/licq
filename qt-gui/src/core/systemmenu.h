/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007-2014 Licq developers <licq-dev@googlegroups.com>
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

#ifndef SYSTEMMENU_H
#define SYSTEMMENU_H

#include "config.h"

#include <QMap>
#include <QMenu>

#include <licq/plugin/protocolplugin.h>
#include <licq/userid.h>

class QActionGroup;

namespace LicqQtGui
{
namespace SystemMenuPrivate
{
  class OwnerData;
}

/**
 * System menu available from main window and tray icon.
 */
class SystemMenu : public QMenu
{
  Q_OBJECT

public:
  /**
   * Constructor
   *
   * @param parent Parent widget
   */
  SystemMenu(QWidget* parent = 0);

  /**
   * Destructor
   */
  virtual ~SystemMenu();

  /**
   * Update the list of groups from the daemon. This must be called when the
   * list of groups is changed.
   */
  void updateGroups();

  /**
   * Get the status sub menu
   *
   * @return The status sub menu from the system menu
   */
  QMenu* getStatusMenu()
  { return myStatusMenu; }

  /**
   * Get the group sub menu
   *
   * @return The group sub menu from the system menu
   */
  QMenu* getGroupMenu()
  { return myGroupMenu; }

  /**
   * Get user main invisibility status from the status menu
   *
   * @return True if invisibility is checked
   */
  bool getInvisibleStatus() const
  { return myStatusInvisibleAction != NULL && myStatusInvisibleAction->isChecked(); }

  /**
   * Get user protocol invisibility status from the status menu
   *
   * @param ppid Protocol to get status for
   * @return True if protocol inivisibility is checked
   */
  bool getInvisibleStatus(const Licq::UserId& userId) const;

private slots:
  /**
   * Add menu entries for a new owner
   *
   * @param userId User id for the new owner
   */
  void addOwner(const Licq::UserId& userId);

  /**
   * Remove an owner from the menu
   *
   * @param userId User id of the owner to remove
   */
  void removeOwner(const Licq::UserId& userId);

  /**
   * Update icons in menu.
   */
  void updateIcons();

  /**
   * Update shortcuts in menu
   */
  void updateShortcuts();

  void aboutToShowMenu();
  void aboutToShowGroupMenu();
  void setMainStatus(QAction* action);
  void toggleMainInvisibleStatus();
  void setCurrentGroup(QAction* action);
  void updateAllUsers();
  void updateAllUsersInGroup();
  void saveAllUsers();
  void showOwnerManagerDlg();
  void showAddUserDlg();
  void showAddGroupDlg();
  void showAuthUserDlg();
  void showReqAuthDlg();
  void showEditGrpDlg();
  void showSettingsDlg();
  void showGPGKeyManager();

private:
  void updateMainStatuses();

  // Actions on top menu
  QAction* mySetArAction;
  QAction* myLogWinAction;
  QAction* myMiniModeAction;
  QAction* myShowOfflineAction;
  QAction* myShowEmptyGroupsAction;
  QAction* myOptionsAction;
  QAction* myAccountManagerAction;
  QAction* myKeyManagerAction;
  QAction* mySaveOptionsAction;
  QAction* myShutdownAction;

  // Actions on user menu
  QAction* myAddGroupAction;
  QAction* myUserAutorizeAction;
  QAction* myUserReqAutorizeAction;
  QAction* myUserPopupAllAction;
  QAction* myEditGroupsAction;

  // Actions not on menu
  QAction* myPopupMessageAction;
  QAction* myHideMainwinAction;
  QAction* myShowHeaderAction;

  // Actions on status menu
  QAction* myStatusOnlineAction;
  QAction* myStatusAwayAction;
  QAction* myStatusNotAvailableAction;
  QAction* myStatusOccupiedAction;
  QAction* myStatusDoNotDisturbAction;
  QAction* myStatusFreeForChatAction;
  QAction* myStatusOfflineAction;
  QAction* myStatusInvisibleAction;

  // Sub menus
  QMenu* myOwnerAdmMenu;
  QMenu* myUserAdmMenu;
  QMenu* myStatusMenu;
  QMenu* myGroupMenu;
  QMenu* myHelpMenu;

  QActionGroup* myStatusActions;
  QActionGroup* myUserGroupActions;

  QAction* myUserAdmActionSeparator;
  QAction* myGroupSeparator;
  QAction* myStatusSeparator;
  QAction* myStatusInvisibleSeparator;
  QAction* myIcqFollowMeSeparator;

  QMap<Licq::UserId, SystemMenuPrivate::OwnerData*> myOwnerData;
};

namespace SystemMenuPrivate
{
/**
 * Owner specific sub menues for the system menu
 */
class OwnerData : public QObject
{
  Q_OBJECT

public:
  /**
   * Constructor
   *
   * @param userId Owner id
   * @param protoName Name of protocol to show in menus
   * @param sendFunctions Send function capabilities for protocol
   * @param parent Parent widget
   */
  OwnerData(const Licq::UserId& userId, const Licq::ProtocolPlugin::Ptr protocol,
      SystemMenu* parent);

  /**
   * Destructor
   */
  virtual ~OwnerData();

  /**
   * Update icons in menu. This should be called when iconset is changed.
   */
  void updateIcons();

  /**
   * Get owner status menu
   *
   * @return Status menu for the owner
   */
  QMenu* getStatusMenu()
  { return myStatusMenu; }

  /// Get ICQ Follow Me menu
  QMenu* getIcqFollowMeMenu()
  { return myIcqFollowMeMenu; }

  /**
   * Get owner system menu
   *
   * @return System menu for the owner
   */
  QMenu* getOwnerAdmMenu()
  { return myOwnerAdmMenu; }

  /// Get ICQ User Search action
  QAction* getIcqUserSearchAction()
  { return myIcqUserSearchAction; }

  /// Get ICQ Random Chat action
  QAction* getIcqRandomChatAction()
  { return myIcqRandomChatAction; }

  /**
   * Get user protocol invisibility status from the status menu
   *
   * @return True if protocol inivisibility is checked
   */
  bool getInvisibleStatus() const
  { return myStatusInvisibleAction != NULL && myStatusInvisibleAction->isChecked(); }

  /**
   * Get away message usage for protocol
   *
   * @return True if away messages are supported
   */
  bool useAwayMessage() const
  { return myUseAwayMessage; }

  /// Get supported statuses for protocol
  unsigned long protocolStatuses() const
  { return myProtocolStatuses; }

private slots:
  void aboutToShowStatusMenu();
  void viewInfo();
  void viewHistory();
  void showSettingsDlg();
  void setStatus(QAction* action);
  void toggleInvisibleStatus();
  void aboutToShowIcqFollowMeMenu();
  void setIcqFollowMeStatus(QAction* action);
  void showIcqUserSearchDlg();
  void showIcqRandomChatSearchDlg();

private:
  Licq::UserId myUserId;
  bool myUseAwayMessage;
  unsigned long myProtocolStatuses;

  QMenu* myStatusMenu;
  QMenu* myOwnerAdmMenu;
  QMenu* myIcqFollowMeMenu;

  // Actions on owner menu
  QAction* myOwnerAdmInfoAction;
  QAction* myOwnerAdmHistoryAction;
  QAction* myOwnerAdmSettingsAction;

  // Actions on status menu
  QAction* myStatusOnlineAction;
  QAction* myStatusAwayAction;
  QAction* myStatusNotAvailableAction;
  QAction* myStatusOccupiedAction;
  QAction* myStatusDoNotDisturbAction;
  QAction* myStatusFreeForChatAction;
  QAction* myStatusOfflineAction;
  QAction* myStatusInvisibleAction;

  QActionGroup* myStatusActions;
  QActionGroup* myIcqFollowMeActions;
  QAction* myIcqUserSearchAction;
  QAction* myIcqRandomChatAction;
};

} // namespace SystemMenuPrivate

} // namespace LicqQtGui

#endif
