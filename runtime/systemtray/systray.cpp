/* This file is part of the KDE Project
   Copyright (c) 2008-2009 Sebastian Trueg <trueg@kde.org>
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
 
#include "systray.h"
#include "systrayplugin.h"
#include "systraypluginmanager.h"
#include "mainwidget.h"

#include <KApplication>
#include <KGlobal>
#include <KMenu>
#include <KLocale>
#include <KIcon>
#include <KAction>
#include <KToolInvocation>
#include <KActionCollection>
#include <KXMLGUIFactory>
#include <KXMLGUIBuilder>
#include <KConfigGroup>
#include <KDebug>
#include <QtGui/QAction>
#include <QtCore/QStringBuilder>
#include <QtCore/QtDebug>
#include <QtCore/QTimer>



Nepomuk::SystemTray::SystemTray( QWidget* parent )
    : KStatusNotifierItem( parent )
    //m_lastIndex(0)
{
    setCategory( SystemServices );
    setStatus( Passive );
    setIconByName( QLatin1String("nepomuk") );
    setTitle( i18n( "Nepomuk Services" ) );
    //setStandardActionsEnabled( true );
    
    /* Init XMLGUI part */
    QString xmlfile = KGlobal::mainComponent().componentName() + QLatin1String("ui.rc");
    setXMLFile(xmlfile);

    // Passing 0 leads to some strange segfaults.
    // So pass new QWidget() instead
    m_builder = new KXMLGUIBuilder(new QWidget());
    m_factory = new KXMLGUIFactory(m_builder,this);

    /* Add some basic actions */
    KAction* configAction = new KAction( this );
    configAction->setText( i18n( "Configure Nepomuk" ) );
    configAction->setIcon( KIcon( QLatin1String("configure") ) );
    connect( configAction, SIGNAL( triggered() ),
             this, SLOT( slotConfigure() ) );
    KXMLGUIClient::actionCollection()->addAction(QLatin1String("configNepomuk"),configAction);


    // Adding client
    m_factory->addClient(this);


    // Check for dev mode
    KSharedConfig::Ptr cfg = KGlobal::config();
    KConfigGroup group = cfg->group("main");
    this->devMode = group.readEntry("devMode",false);
    kDebug() << "Dev mode: " << this->devMode;
    
    // Loading plugins
    loadPlugins();


    setToolTipIconByName(QLatin1String("nepomuk"));
    setToolTipTitle(i18n("Nepomuk"));


    // Create associated window
    m_mainWidget = new MainWidget(m_factory);

    setAssociatedWidget( m_mainWidget );

    m_timer = 0;
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
    args << QLatin1String("kcm_nepomuk");
    KToolInvocation::kdeinitExec(QLatin1String("kcmshell4"), args);
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
       plugin->init(this->devMode);
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

    QWidget * w = m_factory->container(QLatin1String("trayMenu"),this);
    KMenu * trayMenu = qobject_cast<KMenu*>(w);
    if (!trayMenu) {
        kError() << "Failed to retrieve menu";
        return;
    }
    //menu->addAction( configAction );
    setContextMenu(trayMenu);

    // Set tooltip
    buildToolTip();

    // Start timer
    KConfigGroup group = KGlobal::config()->group("main");
    int defaultInterval = 5000;
    m_updateInterval = group.readEntry("updateInterval",defaultInterval);
    if (m_updateInterval < 1000 ) {
        m_updateInterval = 5000;
    }

    m_timer = new QTimer();
    m_timer->setInterval(m_updateInterval);
    connect(m_timer,SIGNAL(timeout()),
            this,SLOT(updateStatuses())
           );
    m_timer->setSingleShot(false);
    m_timer->start();

    // Redrawing main widget
    m_mainWidget->updateSize();

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
    //this->setToolTipSubTitle(m_statusCache.join(QLatin1String("\n")));
    int running = 0;
    int notRunning = 0;
    for( QHash<SystrayPlugin *,int>::const_iterator pit = m_pluginsIndexes.begin();
            pit != m_pluginsIndexes.end();
            ++pit
       )
    {
        SystrayPlugin * plugin = pit.key();
        int shortStatus = plugin->shortStatus(); 
        if ( shortStatus == SystrayPlugin::Running or 
                shortStatus == SystrayPlugin::Suspended or 
                shortStatus == SystrayPlugin::Idle
           )
            running++;
        else
            notRunning++;
    }

    QString toolTipText;
    if ( running > 0 and notRunning > 0 ) {
       toolTipText =  
            QString(
                i18nc(
                    "@info:tooltip Summary of service statuses in tooltip."
                    "Not running means failed with error or didn't start at all"
                    "Running means service successfully launched",
                    "Not Running: %1 Running: %2",
                    QString::number(notRunning),
                    QString::number(running)
                    )
                );
    }
    else if ( running == 0 ) {
        toolTipText = i18nc(
                "@info:tooltip All services are not running( failed or not started",
                "No running services"
                );
    }
    else {
        toolTipText = i18nc(
                "@info:tooltip All services started and none failed",
                "All services running"
                );
    }
    this->setToolTipSubTitle(toolTipText);
}

void Nepomuk::SystemTray::updateStatuses()
{
    QHash<SystrayPlugin*,int>::const_iterator it = m_pluginsIndexes.constBegin();
    QHash<SystrayPlugin*,int>::const_iterator it_end = m_pluginsIndexes.constEnd();
    for(; it!= it_end; it++)
    {
        it.key()->update();
    }
}
