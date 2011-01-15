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
#include <QDBusPendingCallWatcher>
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
        bool init;
        //static OrgKdeNepomukServiceManagerInterface * mainServer();
};

SystrayPlugin::SystrayPlugin(QString serviceName, QString dbusServiceName, QObject * parent):
    QObject(parent),
    d(new Private())
{
    static QString dbusBase = NSERVICE_DBUS_NAME_PREFIX".%1";
    d->init = false;
    d->name = serviceName;
    d->shortName = serviceName;
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

void SystrayPlugin::isServiceInitialized(const char * answerSlot) const
{
    // Now check the service endpoint
    if (!d->controlInterface) {
        int fakeAnswer;
        bool falseAnswer = false;
        QMetaObject::invokeMethod(
                const_cast<SystrayPlugin*>(this),
                answerSlot,
                Qt::QueuedConnection,
                Q_RETURN_ARG(int, fakeAnswer),
                Q_ARG(bool,falseAnswer)
                );
        return;
    }

    QDBusPendingReply<bool> repl2 = d->controlInterface->isInitialized();
    QDBusPendingCallWatcher * watcher = new QDBusPendingCallWatcher(repl2,0);

    // Add answerSlot as dynamic proprety
    watcher->setProperty("_k_forwardSlot",QVariant(answerSlot));

    connect(watcher,SIGNAL(finished(QDBusPendingCallWatcher*)),
            this,SLOT(_k_isServiceInitializedReplyHandler(QDBusPendingCallWatcher*))
           );

}

void SystrayPlugin::_k_isServiceInitializedReplyHandler(QDBusPendingCallWatcher* reply)
{
    //kDebug() << "Called _k_isServiceInitializedReplyHandler";
    QByteArray tmpString = reply->property("_k_forwardSlot").toString().toAscii();
    const char * forwardSlot = tmpString.data(); 
    //kDebug() << "Forward slot is: " << forwardSlot;
    bool answer;
    QDBusPendingReply<bool> repl2 = *reply;
    if (!repl2.isValid()) {
        kDebug() << "Hasn't recieved reply from service. Error:" << repl2.error().message();
        answer = false;
    }
    else {
        answer = repl2.value();
    }

    int fakeAnswer;
    if (!QMetaObject::invokeMethod(this,forwardSlot,Qt::DirectConnection, Q_ARG(bool,answer))) {
        kDebug() << "Forwarding call failed";
    }
    reply->deleteLater();
}

bool SystrayPlugin::isServiceRegistered() const
{
    QDBusReply<bool> answer =  QDBusConnection::sessionBus().interface()->isServiceRegistered(d->dbusServiceAddress);
    if (answer.isValid() )
        return answer.value();
    else 
        return false;
}

void SystrayPlugin::shortStatusRequest() const
{
    if (!isServiceRegistered()) {
        emit shortStatusReply(NotStarted);
        return;
    }

    isServiceInitialized(
            "_k_ssr_stage2"
            );
}

void SystrayPlugin::_k_ssr_stage2(bool isInitialized)
{
    //kDebug() << "_k_ssr_stage2 called";
    if (isInitialized) {
        emit shortStatusReply(Running);
    }
    else {
        emit shortStatusReply(Failed);
    }
}

void SystrayPlugin::serviceStatusMessageRequest() const
{
    emit serviceStatusMessageReply(QString());
}

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

void SystrayPlugin::serviceSystemStatusChanged()
{
    shortStatusRequest();
    if (this->hasStatusMessage())
        this->serviceStatusMessageRequest();
}

QString SystrayPlugin::shortStatusToString(ShortStatus status)
{
   switch ( status )
   {
       case (Running) : return i18nc("@info:status Running","Running");
       case (Idle) : return i18nc("@info:status Idle", "Idle");
       case (Suspended) : return i18nc("@info:status Suspended", "Suspended");
       case (NotStarted) : return i18nc("@info:status Service is not working cause it should not", "Not started");
       case (Failed) : return i18nc("@info:status Service is not working because it met some error and crashed/not work as intended ", "Failed");
       default: return i18nc("@info:status Status unknown", "Unknown");
   } 
   return QString();
}
