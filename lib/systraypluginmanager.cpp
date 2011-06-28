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
    KService::List offers = KServiceTypeTrader::self()->query(QLatin1String("NepomukSystray/Plugin"));
    qDebug() << "Loading plugins. Discovered: " << offers.size();
 
    KService::List::const_iterator iter;
    for(iter = offers.constBegin(); iter < offers.constEnd(); ++iter)
    {
        QString error;
        KService::Ptr service = *iter;
 
        KPluginLoader loader(service->library());
        KPluginFactory *factory = loader.factory();
 
        if (!factory)
        {
            kError(5001) << "KPluginFactory could not load the plugin:" << service->library() << " Error: " << loader.errorString();

            continue;
        }
 
        Nepomuk::SystrayPlugin *plugin = factory->create<Nepomuk::SystrayPlugin>(this);
 
       if (plugin) {
           const QString systemname = service->property(QLatin1String("X-KDE-PluginInfo-Name"),QVariant::String).toString();
           plugin->setObjectName(systemname);
           kDebug() << "Load plugin:" << service->name() << " defined in " << service->entryPath(); 
           // Add it to the list
           d->plugins.append(plugin);

       } else {
           kDebug() << "Failed to load plugin." << error;
       }
    }
    qDebug() << "Loaded plugins: " << d->plugins.size();

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
{ 
   delete d; 
}

