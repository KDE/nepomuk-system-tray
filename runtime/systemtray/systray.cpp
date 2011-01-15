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
 
#include "systray.h"
#include "systrayplugin.h"
#include "systraypluginmanager.h"
#include "mainwidget.h"

#include <KApplication>
#include <KGlobal>
#include <KMenu>
#include <KDualAction>
#include <KActionMenu>
#include <KLocale>
#include <KIcon>
#include <KToolInvocation>
#include <KActionCollection>
#include <KXMLGUIFactory>
#include <KXMLGUIBuilder>
#include <KConfigGroup>
#include <KDebug>
#include <QtGui/QAction>
#include <QtCore/QStringBuilder>
#include <QtCore/QtDebug>



Nepomuk::SystemTray::SystemTray( QWidget* parent )
    : KStatusNotifierItem( parent )
    //m_lastIndex(0)
{
    setCategory( SystemServices );
    setStatus( Passive );
    setIconByName( "nepomuk" );
    setTitle( i18n( "Nepomuk Services" ) );
    //setStandardActionsEnabled( true );
    
    /* Init XMLGUI part */
    QString xmlfile = KGlobal::mainComponent().componentName() + "ui.rc";
    setXMLFile(xmlfile);

    // Passing 0 leads to some strange segfaults.
    // So pass new QWidget() instead
    m_builder = new KXMLGUIBuilder(new QWidget());
    m_factory = new KXMLGUIFactory(m_builder,this);

    /* Add some basic actions */
    KAction* configAction = new KAction( this );
    configAction->setText( i18n( "Configure Nepomuk" ) );
    configAction->setIcon( KIcon( "configure" ) );
    connect( configAction, SIGNAL( triggered() ),
             this, SLOT( slotConfigure() ) );
    KXMLGUIClient::actionCollection()->addAction("configNepomuk",configAction);

    KStandardAction::quit(kapp, SLOT(quit()), KXMLGUIClient::actionCollection());

    // Adding client
    m_factory->addClient(this);

    // Loading plugins
    loadPlugins();


    setToolTipIconByName("nepomuk");
    setToolTipTitle(i18n("Nepomuk"));


    // Create associated window
    m_mainWidget = new MainWidget(m_factory);

    setAssociatedWidget( m_mainWidget );
}


Nepomuk::SystemTray::~SystemTray()
{
    //delete this->menu;

    delete m_factory;
    delete m_builder;
}




void Nepomuk::SystemTray::slotConfigure()
{
    QStringList args;
    args << "kcm_nepomuk";
    KToolInvocation::kdeinitExec("kcmshell4", args);
}


void Nepomuk::SystemTray::loadPlugins()
{
    m_pluginsCurrentlyInitializing = 0;
    foreach(SystrayPlugin* plugin, SystrayPluginManager::self()->plugins())
    {
        Q_CHECK_PTR(plugin);

       connect(plugin,SIGNAL(initializationFinished(Nepomuk::SystrayPlugin*)),
               this,SLOT(pluginInitialized(Nepomuk::SystrayPlugin*))
              );
       m_pluginsCurrentlyInitializing += 1;
       plugin->init();
    }
}

void Nepomuk::SystemTray::pluginInitialized(Nepomuk::SystrayPlugin * plugin)
{
    kDebug() << "Plugin: " << plugin->objectName() << " finish initialization";
    Q_ASSERT(m_pluginsCurrentlyInitializing > 0);
    this->m_pluginsCurrentlyInitializing -= 1;
    //kDebug() << "plugins uninitialized left: " << m_pluginsCurrentlyInitializing;
    // To prevent situations when user will send second pluginIntialized signal
    // Unfortunately this works only if plugin is in the same thread as SystemTray
    disconnect(plugin,SIGNAL(initializationFinished(Nepomuk::SystrayPlugin*)),
               this,SLOT(pluginInitialized(Nepomuk::SystrayPlugin*))
              );
    // We connect plugin's signal shortStatusChanged with plugin's slot
    // shortStatusRequest through our proxy signal because it is correct way.
#if 0
    connect(plugin,SIGNAL(shortStatusChanged(Nepomuk::SystrayPlugin*)),
            this,SIGNAL(mirrorSignal())
           );
    connect(this,SIGNAL(mirrorSignal()),
            plugin,SLOT(shortStatusRequest())
           );
#endif
    // Add it to the factory
    m_factory->addClient(plugin);

    // Assign index and add current status to the cache
    connect(plugin,SIGNAL(shortStatusChanged(Nepomuk::SystrayPlugin*,Nepomuk::SystrayPlugin::ShortStatus)),
            this,SLOT(updateToolTip(Nepomuk::SystrayPlugin*,Nepomuk::SystrayPlugin::ShortStatus))
            );
    m_pluginsIndexes[plugin] = m_statusCache.size();
    m_statusCache.append(SystrayPlugin::shortStatusToString(plugin->shortStatus()));

   // Add it to the main widget
   m_mainWidget->addPlugin(plugin);

   if ( m_pluginsCurrentlyInitializing == 0)
       finishOurInitialization();
}

void Nepomuk::SystemTray::finishOurInitialization()
{
    //kDebug() << "Finishing our initialization";
    // Adding all submenus
    /*
    foreach( QAction * m, this->actions)
    {
       this->menu->addAction(m);
    }
    this->actions.clear();
    */

    QWidget * w = m_factory->container("trayMenu",this);
    KMenu * trayMenu = qobject_cast<KMenu*>(w);
    if (!trayMenu) {
        kError() << "Failed to retrieve menu";
        return;
    }
    //menu->addAction( configAction );
    setContextMenu(trayMenu);

    // Set tooltip
    buildToolTip();

}

void Nepomuk::SystemTray::updateToolTip(Nepomuk::SystrayPlugin * plugin, Nepomuk::SystrayPlugin::ShortStatus status)
{
    

    kDebug() << "Plugin " << plugin->objectName() << "changed status to " << SystrayPlugin::shortStatusToString(status);

    // Get plugin index
    int index = m_pluginsIndexes[plugin];
    m_statusCache[index] = Nepomuk::SystrayPlugin::shortStatusToString(status);

    // Refresh tooltip
    buildToolTip();
}

void Nepomuk::SystemTray::buildToolTip()
{
    //kDebug() << "ToolTip: " << m_statusCache.join("\n");
    this->setToolTipSubTitle(m_statusCache.join("\n"));
}

