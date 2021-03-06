/* This file is part of the KDE Project
   Copyright (c) 2008 Sebastian Trueg <trueg@kde.org>
   Copyright (c) 2010-2011 Serebriyskiy Artem <v.for.vandal@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef NEPOMUK_SERVICE_SYSTRAY_H
#define NEPOMUK_SERVICE_SYSTRAY_H

#include <QtCore/QList>

#include <KStatusNotifierItem>
#include <KSharedConfig>
#include <KXMLGUIClient>

#include "systrayplugin.h"

class QTimer;
class KXMLGUIFactory;
class KXMLGUIBuilder;

namespace Nepomuk {
    class SystrayPlugin;
    class MainWidget;

    class SystemTray : public KStatusNotifierItem, public KXMLGUIClient
    {
        Q_OBJECT
        public:
            SystemTray( QWidget* parent );
            ~SystemTray();

        private Q_SLOTS:
            void slotConfigure();
            void pluginInitialized(Nepomuk::SystrayPlugin*);
            void updateToolTip(Nepomuk::SystrayPlugin * ,Nepomuk::SystrayPlugin::ShortStatus);
            void updateStatuses();

        Q_SIGNALS:
            // This signal is used to react on plugin::shortStatusChanged signal
            // and call plugin::shortStatusRequest slot. This way every time short 
            // status is changed, we request for this status
            //void mirrorSignal();
        private:
            void loadPlugins();
            void finishOurInitialization();
            void buildToolTip();
            // This counter is used to count how many plugins are performing
            // initialization in the moment. Each time init() is called,
            // it's value is increased by one.
            // It only increases in loadPlugins() function. After this function ends,
            // it's value equal to the amount of loaded pugins.
            // Each time pluginInitialized() is called ( that happens when plugin 
            // has send a signal ) it's values is decreased by one
            // When it's value reachs 0 this means that all plugins are now
            // initialized.
            // So we can finish overall initialization
            int m_pluginsCurrentlyInitializing;
            KXMLGUIFactory * m_factory;
            KXMLGUIBuilder * m_builder;
            // The following 3 members are used to cache statuses
            //int m_lastIndex;
            QHash<SystrayPlugin*,int> m_pluginsIndexes;
            QStringList m_statusCache;
            MainWidget * m_mainWidget;
            QTimer * m_timer;
            int m_updateInterval;
	    bool devMode;

    };
}

#endif

