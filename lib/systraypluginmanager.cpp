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
#include "systraypluginmanager.h"
#include "systrayplugin.h"
 
#include <KPluginFactory>
#include <KService>
#include <KServiceTypeTrader>
#include <KDebug>


using namespace Nepomuk;

class SystrayPluginManager::Private
{
    public:
        QList<SystrayPlugin*> plugins;
};

SystrayPluginManager::SystrayPluginManager():
    d(new Private())
{
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
           QString systemname = service->property("X-KDE-PluginInfo-Name",QVariant::String).toString();
           plugin->setObjectName(systemname);
           kDebug() << "Load plugin:" << service->name(); 
           // Add it to the list
           d->plugins.append(plugin);

       } else {
           kDebug() << error;
       }
    }

}

QList<SystrayPlugin*> SystrayPluginManager::plugins() const
{
    return d->plugins;
}

SystrayPluginManager* SystrayPluginManager::self()
{
    static SystrayPluginManager * manager = new SystrayPluginManager();
    return manager;
}

SystrayPluginManager::~SystrayPluginManager()
{; }

