/* This file is part of the KDE Project
   Copyright (c) 2010-2011 Serebriyskiy Artem <v.for.vandal@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */
 
#include "config.h"

namespace Nepomuk {
    class OrgKdeNepomukServiceControlInterface;
    /*! \brief This class is a singleton that communicates with NepomukServer via D-Bus
     * The purpose of this class is to handle situations when server is restarting and
     * automatically recreate interface
     */
    class ServerClient
    {
        Q_OBJECT;
        pubic:
            /*! \brief Return true if server has ben registred and interfaces was successfuly created
             */
            bool isConnected() const;
            OrgKdeNepomukServiceControlInterface * serviceContronInterface() const;
            static ServerClient * self();
        Q_SIGNALS:
            void serverRegistred();
            void serverUnregistred();
            void serverOwnerChanged();
    }
