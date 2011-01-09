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
#include <KPluginFactory>
#include <KService>
#include <KServiceTypeTrader>
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
    setStandardActionsEnabled( true );

    this->config = KGlobal::config();

    this->menu = new KMenu();
    menu->addTitle( i18n( "Search Service" ) );

    loadPlugins();


    setContextMenu( menu );


    KStandardAction::quit(kapp, SLOT(quit()), actionCollection());

}


Nepomuk::SystemTray::~SystemTray()
{
    delete this->menu;
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
    this->pluginsCurrentlyInitializing = 0;
    KService::List offers = KServiceTypeTrader::self()->query("NepomukSystray/Plugin");
 
    KService::List::const_iterator iter;
    for(iter = offers.begin(); iter < offers.end(); ++iter)
    {
       QString error;
       KService::Ptr service = *iter;
 
        KPluginFactory *factory = KPluginLoader(service->library()).factory();
 
        if (!factory)
        {
            kError(5001) << "KPluginFactory could not load the plugin:" << service->library();
            continue;
        }
 
        Nepomuk::SystrayPlugin *plugin = factory->create<Nepomuk::SystrayPlugin>(this);
 
       if (plugin) {
           kDebug() << "Load plugin:" << service->name(); 

           QString systemname = service->property("X-KDE-PluginInfo-Name",QVariant::String).toString();
           plugin->setObjectName(systemname);

           connect(plugin,SIGNAL(initializationFinished()),
                   this,SLOT(pluginInitialized())
                  );
           this->pluginsCurrentlyInitializing += 1;
           QMetaObject::invokeMethod(plugin,"init",Qt::QueuedConnection);

       } else {
           kDebug() << error;
       }
    }
}

void Nepomuk::SystemTray::pluginInitialized()
{
    // Get plugin instance
    Nepomuk::SystrayPlugin * plugin  = qobject_cast<Nepomuk::SystrayPlugin*>(sender());
    if (plugin) {
        kDebug() << "Plugin: " << plugin->objectName() << " finish initialization";
        Q_ASSERT(pluginsCurrentlyInitializing > 0);
        this->pluginsCurrentlyInitializing -= 1;
        kDebug() << "plugins uninitialized left: " << pluginsCurrentlyInitializing;
        // To prevent situations when user will send second pluginIntialized signal
        // Unfortunately this works only if plugin is in the same thread as SystemTray
        disconnect(plugin,SIGNAL(initializationFinished()),
                   this,SLOT(pluginInitialized())
                  );

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
           this->menu->addAction(act);
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
       this->actions.append(m);

       if ( pluginsCurrentlyInitializing == 0)
           finishOurInitialization();
    }
    else {
        kError(5001) << "Recieve signal not from plugin";
    }
}

void Nepomuk::SystemTray::finishOurInitialization()
{
    kDebug() << "Finishing our initialization";
    // Adding all submenus
    foreach( QAction * m, this->actions)
    {
       this->menu->addAction(m);
    }
    this->actions.clear();

    // Load last action - configure systray itself
    KAction* configAction = new KAction( menu );
    configAction->setText( i18n( "Configure System tray" ) );
    configAction->setIcon( KIcon( "configure" ) );
    connect( configAction, SIGNAL( triggered() ),
             this, SLOT( slotConfigure() ) );
    menu->addAction( configAction );
}

QStringList Nepomuk::SystemTray::toplevelActionNames(const QString & pluginName) const
{
    KConfigGroup pluginMenuGroup = config->group(pluginName); 
    QStringList toplevelNames = pluginMenuGroup.readEntry("toplevel",QStringList());
    return toplevelNames;
}
