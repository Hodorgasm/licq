/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010 Licq developers
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

#ifndef LICQ_USERID_H
#define LICQ_USERID_H

#include <string>

// Known Protocol IDs
// Should be local per protocol but defined here as they can also be useful for plugins
#define LICQ_PPID 0x4C696371    // "Licq" (ICQ & AIM)
#define MSN_PPID 0x4D534E5F     // "MSN_" (MSN)
#define JABBER_PPID 0x584D5050  // "XMPP" (Jabber)

namespace Licq
{

// Convenience function to convert protocolId to a string
// ret must be able to hold at least 5 characters
inline char* protocolId_toStr(char* ret, unsigned long protocolId)
{
  ret[0] = ((protocolId & 0xFF000000) >> 24);
  ret[1] = ((protocolId & 0x00FF0000) >> 16);
  ret[2] = ((protocolId & 0x0000FF00) >> 8);
  ret[3] = ((protocolId & 0x000000FF));
  ret[4] = '\0';
  return ret;
}


/**
 * Identity of a user
 */
class UserId
{
public:
  /**
   * Default constructor
   * Creates an invalid user id
   */
  UserId()
      : myProtocolId(0)
  { /* Empty */ }

  /**
   * Constructor
   * Creates a user id from account id and protocol id
   */
  UserId(const std::string& accountId, unsigned long protocolId)
      : myProtocolId(protocolId), myAccountId(normalizeId(accountId, protocolId))
  { /* Empty */ }

  /**
   * Copy constructor
   */
  UserId(const UserId& userId)
      : myProtocolId(userId.myProtocolId), myAccountId(userId.myAccountId)
  { /* Empty */ }

  /**
   * Assignment operator
   */
  UserId& operator=(const UserId& userId)
  { myProtocolId = userId.myProtocolId; myAccountId = userId.myAccountId; return *this; }

  /**
   * Test if two user ids are the same
   */
  bool operator==(const UserId& userId) const
  { return (myProtocolId == userId.myProtocolId && myAccountId == userId.myAccountId); }

  /**
   * Test if two user ids are not equal
   */
  bool operator!=(const UserId& userId) const
  { return !(*this == userId); }

  /**
   * Determine sort order of user ids
   */
  bool operator<(const UserId& userId) const
  { return (myProtocolId < userId.myProtocolId || (myProtocolId == userId.myProtocolId && myAccountId < userId.myAccountId)); }

  /**
   * Get protocol id part of user id
   *
   * @return protocol id if user id is valid, otherwise zero
   */
  unsigned long protocolId() const
  { return myProtocolId; }

  /**
   * Get account id part of user id
   *
   * @return account id if user id is valid, otherwise an empty string
   */
  const std::string& accountId() const
  { return myAccountId; }

  /**
   * Check if user id is valid
   *
   * @return True if user id is valid
   */
  bool isValid() const
  { return (myProtocolId != 0); }

  /**
   * Convert user id to string (for use in debug printouts etc.)
   *
   * @return A printable string of user id
   */
  std::string toString() const
  { char p[5]; protocolId_toStr(p, myProtocolId); return p + myAccountId; }

  /**
   * Normalize an account id
   * TODO: This should be done in protocol plugins, not here
   *
   * @param accountId Account id
   * @param ppid Protocol instance id
   * @return Normalized account id
   */
  static std::string normalizeId(const std::string& accountId, unsigned long protocolId);

private:
  unsigned long myProtocolId;
  std::string myAccountId;
};

} // namespace Licq

#endif