/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010 Erik Johansson <erijo@licq.org>
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

#ifndef LICQ_DAEMON_H
#define LICQ_DAEMON_H

#include <boost/noncopyable.hpp>

namespace Licq
{

class LogService;
class PluginManager;

class Daemon : private boost::noncopyable
{
public:
  /**
   * Get the plugin manager instance.
   *
   * @return The global plugin manager.
   */
  virtual PluginManager& getPluginManager() = 0;
  virtual const PluginManager& getPluginManager() const = 0;

  virtual LogService& getLogService() = 0;

protected:
  virtual ~Daemon() { /* Empty */ }
};

} // namespace Licq

#endif