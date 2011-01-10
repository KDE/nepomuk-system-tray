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
 
#include "systrayplugin.h"
#include "systraypluginmanager.h"
#include "systray.h"

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
#include <QAction>
#include <QtDebug>



Nepomuk::SystemTray::SystemTray( QWidget* parent )
    : KStatusNotifierItem( parent )
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
    configAction->setText( i18n( "Configure System tray" ) );
    configAction->setIcon( KIcon( "configure" ) );
    connect( configAction, SIGNAL( triggered() ),
             this, SLOT( slotConfigure() ) );
    KXMLGUIClient::actionCollection()->addAction("configNepomuk",configAction);

    KStandardAction::quit(kapp, SLOT(quit()), KXMLGUIClient::actionCollection());

    // Adding client
    m_factory->addClient(this);

    // Loading plugins
    loadPlugins();


    //setContextMenu( menu );



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


void Nepomuk::SystemTray::updateTooltip()
{
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
       QMetaObject::invokeMethod(plugin,"init",Qt::QueuedConnection);
    }
}

void Nepomuk::SystemTray::pluginInitialized(Nepomuk::SystrayPlugin * plugin)
{
    kDebug() << "Plugin: " << plugin->objectName() << " finish initialization";
    Q_ASSERT(m_pluginsCurrentlyInitializing > 0);
    this->m_pluginsCurrentlyInitializing -= 1;
    kDebug() << "plugins uninitialized left: " << m_pluginsCurrentlyInitializing;
    // To prevent situations when user will send second pluginIntialized signal
    // Unfortunately this works only if plugin is in the same thread as SystemTray
    disconnect(plugin,SIGNAL(initializationFinished(Nepomuk::SystrayPlugin*)),
               this,SLOT(pluginInitialized(Nepomuk::SystrayPlugin*))
              );

    // Add it to the factory
    m_factory->addClient(plugin);
#if 0
   // Get it actions
   KActionCollection * coll = plugin->actions();
   if ( !coll ) {
       kDebug() << "No actions are exposed";
       return;
   }
   QStringList toplevelItems = toplevelActionNames(plugin->objectName());
   // Add top level actions to the menu directly
   foreach ( const QString & iname, toplevelItems)
   {
       QAction * act = coll->action(iname);
       if (!act)
           continue;
       this->contextMenu()->addAction(act);
   }
   // Get menu
   KActionMenu * m = plugin->menu();
   if (!m) {
       kDebug() << "No menu is provided";
       return;
   }
   kDebug() << "Actions exposed in menu: " << m->menu()->actions().size(); 
   // Add submenu to the special list.
   // This is necessary to prevent mixing submenus and top-level itmes
   // cause Qt doesn't support adding actions to the arbitrary
   // plases in the menu
   m_actions.append(m);
#endif
   if ( m_pluginsCurrentlyInitializing == 0)
       finishOurInitialization();
}

void Nepomuk::SystemTray::finishOurInitialization()
{
    kDebug() << "Finishing our initialization";
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


    setAssociatedWidget( contextMenu() );
}

QStringList Nepomuk::SystemTray::toplevelActionNames(const QString & pluginName) const
{
    KConfigGroup pluginMenuGroup = KGlobal::config()->group(pluginName); 
    QStringList toplevelNames = pluginMenuGroup.readEntry("toplevel",QStringList());
    return toplevelNames;
}
