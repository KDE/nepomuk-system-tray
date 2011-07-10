/*
 *   Copyright 2011 Serebriyskiy Artem <v.for.vandal@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
 
#include "nepomuk_engine.h"
#include "systraypluginmanager.h"

#include <QtCore/QMetaObject>
#include <QtCore/QMetaProperty>

#include <QtDebug>

using namespace Nepomuk;

NepomukEngine::NepomukEngine(QObject* parent, const QVariantList& args):
    Plasma::DataEngine(parent,args)
{
}


void NepomukEngine::init()
{
    setMinimumPollingInterval(1000);
}

bool NepomukEngine::sourceRequestEvent(const QString& name)
{
    Q_ASSERT(!m_plugins.contains(name));

    Nepomuk::SystrayPlugin * plugin = Nepomuk::SystrayPluginManager::self()->plugin(name);

    if ( plugin ) {
        m_plugins[name] = plugin;
        // Now connect necessary signals
        connectSignals(plugin);

        // Init
        plugin->init(false);
    }
    return false;
}

bool NepomukEngine::updateSourceEvent( const QString& name)
{
    Q_ASSERT(m_plugins.contains(name));

    Nepomuk::SystrayPlugin * plugin = m_plugins[name];
    Q_CHECK_PTR(plugin);

    // short status is always available. But it is cached.
    QString shortStatus = Nepomuk::SystrayPlugin::shortStatusToString(
        plugin->shortStatus()
        );

    qDebug() << "Name: " << name << " Short status:("<<plugin->shortStatus() << ") " << shortStatus;
    setData(name, QLatin1String(I18N_NOOP("shortStatus")),shortStatus);

    plugin->update();
    return true;
}
QStringList NepomukEngine::sources() const
{
    return SystrayPluginManager::self()->pluginNames();
}

void NepomukEngine::connectSignals( SystrayPlugin * plugin )
{
    // Before Qt 4.8 there is no way to connect to property Notify signal.
    // instead we will connect to changed() signal of plugin
    connect(plugin,SIGNAL(changed(Nepomuk::SystrayPlugin*)),
            this, SLOT(pluginChanged(Nepomuk::SystrayPlugin*))
           );

    connect(plugin, SIGNAL(statusMessageChanged(Nepomuk::SystrayPlugin*,QString)),
            this, SLOT(statusMessageChanged(Nepomuk::SystrayPlugin*,QString))
           );

    connect( plugin, SIGNAL(shortStatusChanged(Nepomuk::SystrayPlugin*,Nepomuk::SystrayPlugin::ShortStatus)),
           this, SLOT(shortStatusChanged(Nepomuk::SystrayPlugin*,Nepomuk::SystrayPlugin::ShortStatus))
           );
}

void NepomukEngine::pluginChanged(Nepomuk::SystrayPlugin* plugin)
{
    updateProperties(plugin);
}

void NepomukEngine::statusMessageChanged( Nepomuk::SystrayPlugin* plugin, QString statusMessage )
{
    setData(plugin->systemName(),QLatin1String(I18N_NOOP("statusMessage")), statusMessage);
}

void NepomukEngine::updateProperties(SystrayPlugin * plugin )
{

    // For all properties
    const QMetaObject * meta = plugin->metaObject();
    for( int i = meta->propertyOffset(); i <  meta->propertyCount(); i++)
    {
        QMetaProperty prop = meta->property(i);
        Q_ASSERT(prop.isValid());
        // Skip properties not marked as user
        if ( !prop.isUser() )
            continue;

        // Take it value
        QVariant value = prop.read(plugin);

        // setData
        setData(plugin->systemName(), QLatin1String(prop.name()), value);
    }
}

void NepomukEngine::shortStatusChanged( Nepomuk::SystrayPlugin* plugin , Nepomuk::SystrayPlugin::ShortStatus status)
{
    qDebug() << "Plugin: " << plugin->systemName() << " Short status: " << Nepomuk::SystrayPlugin::shortStatusToString(
                plugin->shortStatus()
                );

    // short status is always available
    setData(plugin->systemName(), QLatin1String(I18N_NOOP("shortStatus")),
            Nepomuk::SystrayPlugin::shortStatusToString(
                plugin->shortStatus()
                )
           );
}

K_EXPORT_PLASMA_DATAENGINE(nepomuk, NepomukEngine)
