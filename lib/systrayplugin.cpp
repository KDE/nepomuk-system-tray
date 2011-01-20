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
#include <QDBusPendingCallWatcher>
#include <KDebug>
#include <KDesktopFile>
#include <KLocale>
#include <KAction>

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
        bool init;
        ShortStatus serviceShortStatus;
};

SystrayPlugin::SystrayPlugin(const KDesktopFile & serviceDesktopFile, QString dbusServiceName, QObject * parent):
    QObject(parent),
    d(new Private())
{
    static QString dbusBase = NSERVICE_DBUS_NAME_PREFIX".%1";
    d->init = false;
    d->name = serviceDesktopFile.readName();;
    d->shortName = d->name;
    d->description = serviceDesktopFile.readComment();
    d->dbusServiceName = dbusServiceName;
    d->dbusServiceAddress = dbusBase.arg(dbusServiceName);
    d->controlInterface = 0;
    d->watcher = 0;

}

void SystrayPlugin::init()
{
       QMetaObject::invokeMethod(this,"_k_performInit",Qt::QueuedConnection);
}

void SystrayPlugin::_k_performInit()
{
    if ( !d->init) {
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
        this->doInit();
        d->init = true;
        emit initializationFinished(this); 
        setShortStatus(NotStarted);
        shortStatusUpdate();
    }
}

bool SystrayPlugin::isInitialized() const
{
    return d->init;
}

SystrayPlugin::~SystrayPlugin()
{
    delete d;
}


QString SystrayPlugin::serviceName() const
{
    return d->name;
}

QString SystrayPlugin::shortServiceName() const
{
    return d->shortName;
}

QString SystrayPlugin::serviceDescription() const
{
    return d->description;
}

SystrayPlugin::ShortStatus SystrayPlugin::shortStatus() const
{
    return d->serviceShortStatus;
}
void SystrayPlugin::setShortServiceName(const QString & name)
{d->shortName = name;}

void SystrayPlugin::setServiceDescription( const QString & description )
{
    d->description = description;
}

void SystrayPlugin::setShortStatus( ShortStatus status)
{
    if (!d->init) 
        kDebug() << "Warning: Updating status on non-initialized plugin.";

    d->serviceShortStatus = status;
    emit shortStatusChanged(this, d->serviceShortStatus);
}


bool SystrayPlugin::isServiceRegistered() const
{
    QDBusReply<bool> answer =  QDBusConnection::sessionBus().interface()->isServiceRegistered(d->dbusServiceAddress);
    if (answer.isValid() )
        return answer.value();
    else 
        return false;
}

void SystrayPlugin::shortStatusUpdate() 
{
    if (!isServiceRegistered()) {
        setShortStatus(NotStarted);
        return;
    }

    isServiceInitialized(SLOT(_k_ssr_stage2(QDBusPendingCallWatcher*)));
}

void SystrayPlugin::isServiceInitialized(const char * answerSlot) const
{
    // Now check the service endpoint
    if (!d->controlInterface) {
        callWithNull(answerSlot);
    }

    QDBusPendingReply<bool> repl2 = d->controlInterface->isInitialized();
    QDBusPendingCallWatcher * watcher = new QDBusPendingCallWatcher(repl2,0);


    connect(watcher,SIGNAL(finished(QDBusPendingCallWatcher*)),
            this,answerSlot
           );

}

void SystrayPlugin::_k_ssr_stage2(QDBusPendingCallWatcher* watcher)
{
    QDBusPendingReply<bool> isInitialized = *watcher;
    if ( !isInitialized.isValid() ) {
        setShortStatus(Failed);
        return;
    }
    bool answer = isInitialized.value();
    if (!answer) {
        setShortStatus(Launching);
        return;
    }

    isServiceSuspended(SLOT(_k_ssr_stage3(QDBusPendingCallWatcher*)));

}

void SystrayPlugin::isServiceSuspended(const char * answerSlot)
{
    QDBusPendingCallWatcher * watcher = this->isServiceSuspendedRequest();
    if ( watcher) { 
        connect(watcher,SIGNAL(finished(QDBusPendingCallWatcher*)),
                this,answerSlot
               );
        return;
    }
    else {
        callWithNull(answerSlot);
        return;
    }
}

void SystrayPlugin::_k_ssr_stage3(QDBusPendingCallWatcher * watcher)
{
    if (watcher) {
        // Only if watcher != NULL that means service claims to support 
        // suspend functionality
        QDBusPendingReply<bool> isSuspended = *watcher;
        watcher->deleteLater();

        if (!isSuspended.isValid()) {
            // Failed
            kDebug() << "Hasn't received reply from service. Error:" << isSuspended.error().message();
            setShortStatus(Failed);
            return;
        }

        bool answer;
        answer = isSuspended.value();

        if (answer) {
            setShortStatus(Suspended);
            return;
        }
    }

    // Check for idle
    isServiceRunning(SLOT(_k_ssr_stage4(QDBusPendingCallWatcher*)));
}

void SystrayPlugin::isServiceRunning(const char * answerSlot)
{
    QDBusPendingCallWatcher * watcher = this->isServiceRunningRequest();
    if (watcher) {
        connect(watcher,SIGNAL(finished(QDBusPendingCallWatcher*)),
                this,answerSlot
               );
    }
    else {
        callWithNull(answerSlot);
        return;
    }
}

void SystrayPlugin::_k_ssr_stage4(QDBusPendingCallWatcher * watcher)
{
    if (watcher) {
        QDBusPendingReply<bool> isIndexing = *watcher;
        watcher->deleteLater();
        // Running, but may be idle
        if (!isIndexing.isValid()) {
            // Error. Probably hung
            setShortStatus(Failed);
            return;
        }
        else {
            bool answer = isIndexing.value();
            if ( answer ) {
                // Indexing
                setShortStatus(Running);
                return;
            }
            
            setShortStatus(Idle);
            return ;
        }
    }
    else {
        setShortStatus(Running);
        return;
    }
}

void SystrayPlugin::serviceStatusMessageUpdate() 
{
    emit statusMessageChanged(const_cast<SystrayPlugin*>(this),QString());
}

QString SystrayPlugin::dbusServiceName() const
{ return d->dbusServiceName; }

QString SystrayPlugin::dbusServiceAddress() const
{ return d->dbusServiceAddress; }

void SystrayPlugin::_k_serviceRegistered()
{
    setShortStatus(NotStarted);
    this->serviceRegistered();
}

void SystrayPlugin::_k_serviceUnregistered()
{
    setShortStatus(NotStarted);
    this->serviceUnregistered();
}

void SystrayPlugin::_k_serviceOwnerChanged()
{
    setShortStatus(Launching);
    this->serviceOwnerChanged();
}

void SystrayPlugin::callWithNull(const char * answerSlot) const
{
    QTimer::singleShot(0,const_cast<SystrayPlugin*>(this),answerSlot);
}

QString SystrayPlugin::shortStatusToString(ShortStatus status)
{
   switch ( status )
   {
       case (Launching) : return i18nc("@info:status Launching","Launching");
       case (Running) : return i18nc("@info:status Running","Running");
       case (Idle) : return i18nc("@info:status Idle", "Idle");
       case (Suspended) : return i18nc("@info:status Suspended", "Suspended");
       case (NotStarted) : return i18nc("@info:status Service is not working cause it should not", "Not started");
       case (Failed) : return i18nc("@info:status Service is not working because it met some error and crashed/not work as intended ", "Failed");
       default: return i18nc("@info:status Status unknown", "Unknown");
   } 
   return QString();
}
