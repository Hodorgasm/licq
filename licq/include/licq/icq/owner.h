/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2012 Licq developers <licq-dev@googlegroups.com>
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

#ifndef LICQ_ICQ_OWNER_H
#define LICQ_ICQ_OWNER_H

#include "../contactlist/owner.h"

namespace Licq
{

/**
 * An ICQ protocol account
 */
class IcqOwner : public Owner
{
public:
  void SetWebAware(bool b)     {  m_bWebAware = b; save(SaveOwnerInfo); }
  void SetWebAwareStatus(char c)                { SetWebAware(c); }
  void SetHideIp(bool b)       {  m_bHideIp = b; save(SaveOwnerInfo); }
  bool WebAware() const                         { return m_bWebAware; }
  bool HideIp() const                           { return m_bHideIp; }

  /// Current random chat group (0=none)
  unsigned randomChatGroup() const              { return myRandomChatGroup; }

protected:
  /// Constructor
  IcqOwner(const UserId& id);

  /// Destructor
  virtual ~IcqOwner();

  bool m_bWebAware;
  bool m_bHideIp;
  unsigned myRandomChatGroup;
};

/**
 * Read mutex guard for Licq::IcqOwner
 */
class IcqOwnerReadGuard : public OwnerReadGuard
{
public:
  // Derived costructors
  IcqOwnerReadGuard(const UserId& userId)
    : OwnerReadGuard(userId)
  { }
  IcqOwnerReadGuard()
    : OwnerReadGuard(LICQ_PPID)
  { }
  IcqOwnerReadGuard(const IcqOwner* owner, bool locked = false)
    : OwnerReadGuard(owner, locked)
  { }
  IcqOwnerReadGuard(IcqOwnerReadGuard* guard)
    : OwnerReadGuard(guard)
  { }

  // Access operators
  const IcqOwner* operator*() const
  { return dynamic_cast<const IcqOwner*>(OwnerReadGuard::operator*()); }
  const IcqOwner* operator->() const
  { return dynamic_cast<const IcqOwner*>(OwnerReadGuard::operator->()); }
};

/**
 * Write mutex guard for Licq::IcqOwner
 */
class IcqOwnerWriteGuard : public OwnerWriteGuard
{
public:
  // Derived costructors
  IcqOwnerWriteGuard(const UserId& userId)
    : OwnerWriteGuard(userId)
  { }
  IcqOwnerWriteGuard()
    : OwnerWriteGuard(LICQ_PPID)
  { }
  IcqOwnerWriteGuard(IcqOwner* owner, bool locked = false)
    : OwnerWriteGuard(owner, locked)
  { }
  IcqOwnerWriteGuard(IcqOwnerWriteGuard* guard)
    : OwnerWriteGuard(guard)
  { }

  // Access operators
  IcqOwner* operator*() const
  { return dynamic_cast<IcqOwner*>(OwnerWriteGuard::operator*()); }
  IcqOwner* operator->() const
  { return dynamic_cast<IcqOwner*>(OwnerWriteGuard::operator->()); }
};

} // namespace Licq

#endif
