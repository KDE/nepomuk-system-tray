/* This file is part of the KDE Project
   Copyright (c) 2008-2009 Sebastian Trueg <trueg@kde.org>
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
 
#ifndef NEPOMUK_SYSTRAY_nepomukbackupsync_PLUGIN
#define NEPOMUK_SYSTRAY_nepomukbackupsync_PLUGIN

#include "systrayplugin.h"

#include <kdemacros.h>

#include <QtCore/QObject>


namespace Nepomuk
{
    /* This is the main class of your plugin */
    class nepomukbackupsyncSystrayPlugin : public SystrayPlugin
    {
        Q_OBJECT;
        public:
            /* Constructor and destructor.
             * Do not init DBus things in constructor. You should only 
             * init actions your private variables. Other stuff - to function
             * doInit() */
            nepomukbackupsyncSystrayPlugin(QObject *,const QList<QVariant>&);
            virtual ~nepomukbackupsyncSystrayPlugin();
            /* If true, then your plugin will be show by default to the user.
             * If false, then it will be showh only when user press "Show all"
             * checkbox in standad gui
             * As a rule of thumb - return true only if your plugin exports 
             * actions that user may want to interact with. Note, that you
             * can also export actions for developer. If you export actions
             * for developers only, then return false.
             */
            virtual bool userOriented() const { return true; }
        protected:
            /* This is function where you initialize DBus related interfaces,
             * connect them to signals and so on. You can initialize actions
             * here as well.
             * devMode indicates a special mode, when you could provide
             * extra actions, that are developers-only
             */ 
            virtual void doInit(bool devMode);

        protected Q_SLOTS:

            /* This function is called automatically when service
             * reports that it is initialized. This slot may be not 
             * called if system tray is launched some time after the
             * service. DO NOT RELY ON THIS SLOT
             */
            virtual void serviceInitialized(bool);

            void callBackup();
        private:
            class Private;
            Private * const  d;
    };
}

#endif
