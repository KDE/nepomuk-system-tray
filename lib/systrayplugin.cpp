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
#include "systrayplugin.h"
#include "servicemanagerinterface.h"
#include "servicecontrolinterface.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <KDebug>
#include <KLocale>

using namespace Nepomuk;

class SystrayPlugin::Private
{
    public:
        QString name;
        QString shortName;
        QString description;
        QString dbusServiceName;
        QString dbusServiceAddress;
        OrgKdeNepomukServiceControlInterface * controlInterface;
        QDBusServiceWatcher * watcher;
        QStringList actionNamesCache;
        //static OrgKdeNepomukServiceManagerInterface * mainServer();
};

SystrayPlugin::SystrayPlugin(QString serviceName, QString dbusServiceName, QObject * parent):
    QObject(parent),
    d(new Private())
{
    static QString dbusBase = NSERVICE_DBUS_NAME_PREFIX".%1";
    d->name = serviceName;
    d->shortName = serviceName;
    d->dbusServiceName = dbusServiceName;
    d->dbusServiceAddress = dbusBase.arg(dbusServiceName);
    d->controlInterface = 0;
    // Trying to create service control interface
    d->controlInterface = new OrgKdeNepomukServiceControlInterface(
            d->dbusServiceAddress,
            NEPOMUK_SERVICECONTROL_PATH,
            QDBusConnection::sessionBus(),
            this);

    connect(d->controlInterface, SIGNAL(serviceInitialized(bool)),
            this,SLOT(serviceInitialized(bool))
           );

    // Setup watching for service registration.
    // We need this because service can be restarted for example.
    d->watcher = new QDBusServiceWatcher(
            d->dbusServiceAddress,
            QDBusConnection::sessionBus(),
            QDBusServiceWatcher::WatchForRegistration|QDBusServiceWatcher::WatchForUnregistration | QDBusServiceWatcher::WatchForOwnerChange,
            this);
    connect(d->watcher,SIGNAL(serviceRegistered(const QString &)),
            this, SLOT(_k_serviceRegistered())
           );
    connect(d->watcher,SIGNAL(serviceUnregistered(const QString &)),
            this, SLOT(_k_serviceUnregistered())
           );
    /*
    connect(d->watcher,SIGNAL(serviceOwnerChanged(const QString &)),
            this, SLOT(_k_serviceOwnerChanged())
           );
           */

}

void SystrayPlugin::init()
{
    this->doInit();
    emit initializationFinished(this); 
}

SystrayPlugin::~SystrayPlugin()
{
    delete d;
}


#if 0
KActionMenu * SystrayPlugin::menu() const
{ return (this->actions() == 0)?0:this->actions()->menu(); }
#endif

QString SystrayPlugin::serviceName() const
{
    return d->name;
}

QString SystrayPlugin::shortServiceName() const
{
    return d->shortName;
}

void SystrayPlugin::setShortServiceName(const QString & name)
{d->shortName = name;}

QString SystrayPlugin::serviceDescription() const
{
    return d->description;
}

void SystrayPlugin::setServiceDescription( const QString & description )
{
    d->description = description;
}

bool SystrayPlugin::isServiceInitialized() const
{
#if 0
    /* Ask org.kde.NepomukServer first */

    // Check that server exists
    QScopedPointer<OrgKdeNepomukServiceManagerInterface> nserverIface = new OrgKdeNepomukServiceManagerInterface(
            NEPOMUKSERVER_ADDRESS,
            NEPOMUKSERVER_SERVICEMANAGER_PATH,
            QDBusConnection::sessionBus(),
            0);

    if (!nserverIface->isValid() )
        return false;

    // Ask it about status of the service
    QDBusPendingReply<bool> repl1 = nserverIface->isServiceRunning(d->dbusServiceName);

    repl1->waitForFinished();
    if ( !repl1->isValid() ) {
        // Then it looks like NepomukServer doesn't respond for messages
        // Probably it hangs. So the rest of services probably too.
        // return false.
        return false;
    }

    bool answer1 = repl1->value();

    if (!answer1)
        return false;
#endif
    // Now check the service endpoint

    QDBusPendingReply<bool> repl2 = d->controlInterface->isInitialized();
    repl2.waitForFinished();

    if (!repl2.isValid()) {
        kDebug() << "Hasn't recieved reply from service. Error:" << repl2.error().message();
        return false;
    }

    bool answer2 = repl2.value();

    return answer2;
}

#if 0
// THis function is unnecessary now
// It is conservated
void SystrayPlugin::updateControlInterface()
{
    if (d->controlInterface)
        delete d->controlInterface;

    // Trying to create service control interface
    d->controlInterface = new OrgKdeNepomukServiceControlInterface(
            d->dbusServiceAddress,
            NEPOMUK_SERVICECONTROL_PATH,
            QDBusConnection::sessionBus(),
            this);
    // Check that we succeeded
    if (!d->controlInterface->isValid() ) {
        // we are not
        delete d->controlInterface;
        d->controlInterface = 0;
    }
}
#endif

bool SystrayPlugin::isServiceRegistered() const
{
    QDBusReply<bool> answer =  QDBusConnection::sessionBus().interface()->isServiceRegistered(d->dbusServiceAddress);
    if (answer.isValid() )
        return answer.value();
    else 
        return false;
}

SystrayPlugin::ShortStatus SystrayPlugin::shortStatus() const
{
    if (!isServiceRegistered()) {
        return NotStarted;
    }
    else if (!isServiceInitialized()) {
        return Failed;
    }
    else return Running;
}

#if 0
QStringList SystrayPlugin::actionSystemNames() const
{
    if (d->actionNamesCache.isEmpty() ) {
        KActionCollection * collection = this->actions();
        if ( !collection ) {
            kDebug() << "Plugin do not expose any actions";
            return d->actionNamesCache;
        }
        if (collection->count() == 0) {
            kDebug() << "Plugin do not expose any actions";
            return d->actionNamesCache;
        }
        else {
            // Fill cache
        }
    return QStringList();
}
#endif

void SystrayPlugin::emitServiceStatusChanged()
{
    emit shortStatusChanged(this);
    emit statusMessageChanged(this);
}

QString SystrayPlugin::serviceStatusMessage() const
{
    return QString();
}

#if 0
QString SystrayPlugin::serviceErrorMessage() const
{
    return QString();
}
#endif 

QString SystrayPlugin::dbusServiceName() const
{ return d->dbusServiceName; }

QString SystrayPlugin::dbusServiceAddress() const
{ return d->dbusServiceAddress; }

void SystrayPlugin::_k_serviceRegistered()
{
    this->serviceRegistered();
}

void SystrayPlugin::_k_serviceUnregistered()
{
    this->serviceUnregistered();
}

void SystrayPlugin::_k_serviceOwnerChanged()
{
    this->serviceOwnerChanged();
}

/*
OrgKdeNepomukServiceManagerInterface * SystrayPlugin::Private::mainServer()
{
    static OrgKdeNepomukServiceManagerInterface * _s = new OrgKdeNepomukServiceManagerInterface(NEPOMUKSERVER_ADDRESS,NEPOMUKSERVER_SERVICEMANAGER_PATH, QDBusConnection::sessionBus(),0);
    return _s;
}
*/

QString SystrayPlugin::shortStatusToString(ShortStatus status)
{
   switch ( status )
   {
       case (Running) : return i18n("Running");
       case (Idle) : return i18n("Idle");
       case (Suspended) : return i18n("Suspended");
       case (NotStarted) : return i18n("Not started");
       case (Failed) : return i18n("Failed");
       default: return i18n("Status unknown");
   } 
   return QString();
}
