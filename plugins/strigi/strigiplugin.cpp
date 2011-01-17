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
 

#include "strigiplugin.h"
#include "strigiserviceinterface.h"

#include <KPluginFactory>
#include <KGuiItem>
#include <KLocale>
#include <KDebug>
#include <KDualAction>
#include <KActionMenu>
#include <KActionCollection>
#include <KDesktopFile>
using namespace Nepomuk;

K_PLUGIN_FACTORY(StrigiSystrayPluginFactory, registerPlugin< Nepomuk::StrigiSystrayPlugin >();)
K_EXPORT_PLUGIN(StrigiSystrayPluginFactory("systraystrigiplugin"))

class StrigiSystrayPlugin::Private
{
    public:
        Private():srAction(0){;}
        KDualAction * srAction;
        //KActionCollection * actions;
        //KActionMenu * menu;
        OrgKdeNepomukStrigiInterface * strigiInteface;
};

StrigiSystrayPlugin::StrigiSystrayPlugin( QObject * parent,const QList<QVariant>&):
    SystrayPlugin(
            KDesktopFile("services","nepomukstrigiservice.desktop")
            ,"nepomukstrigiservice",
            parent),
    d(new Private())
{
    /* Init XMLGUI part */
    setXMLFile("systraystrigipluginui.rc");

    /* Init service and actions */
    this->setServiceDescription("Indexes files");
    /* Init menu */
    this->d->srAction = new KDualAction(0);
    d->srAction->setActiveGuiItem( KGuiItem( i18n( "Resume File Indexing" ) ) );
    d->srAction->setInactiveGuiItem( KGuiItem( i18n( "Suspend File Indexing" ) ) );
    d->srAction->setToolTip( i18n( "Suspend or resume the file indexer manually" ) );
    connect( d->srAction, SIGNAL( activeChangedByUser( bool ) ),
             this, SLOT( slotSuspend( bool ) ) );

    actionCollection()->addAction("suspresStrigi",d->srAction);
    /*
    d->actions = new KActionCollection(this);
    d->menu = new KActionMenu(i18n("Strigi File Indexer"),this);

    d->actions->addAction("suspend/resume", d->srAction);
    d->menu->addAction(d->srAction);
    */


}


void StrigiSystrayPlugin::doInit()
{
    d->strigiInteface = new OrgKdeNepomukStrigiInterface(dbusServiceAddress(), 
            '/' + dbusServiceName(),
            QDBusConnection::sessionBus(),
            this);

    /* Connect necessary signals first */
    /*
    connect(d->strigiInteface,SIGNAL(statusChanged()),
            this,SLOT(serviceSystemStatusChanged()));
            */

    connect(d->strigiInteface,SIGNAL(statusChanged()),
            this,SLOT(updateActions()));

    // We should connect signals that responsible for changing status message
    // here, but this subsystem is not ready

    /* Update actions */
    updateActions();
}

StrigiSystrayPlugin::~StrigiSystrayPlugin()
{
    //delete d->srAction;
    delete d;
}

/*
KActionCollection * StrigiSystrayPlugin::actions() const
{
    return d->actions;
}

KActionMenu * StrigiSystrayPlugin::menu() const
{
    return d->menu;
}
*/

void StrigiSystrayPlugin::serviceInitialized(bool success)
{
    setShortStatus(Running);
    serviceSystemStatusChanged();
}

void StrigiSystrayPlugin::serviceSystemStatusChanged()
{
    updateActions();
}

void StrigiSystrayPlugin::updateActions()
{

    d->srAction->setEnabled(false);
    if (!isServiceRegistered()) {
        kDebug() << "Service even doesn't exist( not registered)";
        return;
    }

    isServiceInitialized(SLOT(_k_ua_stage2(QDBusPendingCallWatcher*)));
}

void StrigiSystrayPlugin::_k_ua_stage2(QDBusPendingCallWatcher * watcher)
{
    if ( !watcher ) 
        return;

    watcher->deleteLater();
    QDBusPendingReply<bool> isInitialized = *watcher;
    if (!isInitialized.isValid() || !isInitialized.value()) {
        kDebug() << "Service is not initialized";
        return;
    }

    isServiceSuspended(SLOT(_k_ua_stage3(QDBusPendingCallWatcher *)));
}

void StrigiSystrayPlugin::_k_ua_stage3(QDBusPendingCallWatcher * watcher)
{
    if (!watcher)
        return;

    QDBusPendingReply<bool> isSuspended = *watcher;
    watcher->deleteLater();

    if (!isSuspended.isValid() ) { 
        // error. Probaly hungs
        d->srAction->setEnabled(false);
        kDebug() << "Checking for suspend. Failed to receive reply from service.\
            Error: " << isSuspended.error().message();
        return;
    }
    else {
        bool answer = isSuspended.value();
        d->srAction->setEnabled(true);
        if ( answer ) {
            // suspended
            d->srAction->setActive(true);
        }
        else {
            d->srAction->setActive(false);
        }
    }

    
}


QDBusPendingCallWatcher * StrigiSystrayPlugin::isServiceSuspendedRequest()
{
    QDBusPendingReply<bool> reply = d->strigiInteface->isSuspended();
    return new QDBusPendingCallWatcher(reply,0);
}

QDBusPendingCallWatcher * StrigiSystrayPlugin::isServiceRunningRequest()
{
    QDBusPendingReply<bool> reply = d->strigiInteface->isIndexing();
    return new QDBusPendingCallWatcher(reply,0);
}

void StrigiSystrayPlugin::slotSuspend(bool)
{
    // Check what to do - suspend or resume
    // We will check button, not service itself
    if ( d->srAction->isActive() ) { // Active means user asked to suspend
        QDBusPendingReply<> reply = d->strigiInteface->suspend();
    }
    else {
        QDBusPendingReply<> reply = d->strigiInteface->resume();
    }
    // Now force action updating. This is necessary for the case when
    // 1) User asked to suspend
    // 2) Some other processes suspended strigi
    // 3) We suspended strigi
    // 4) Cause of this strigi do not emit statusChanged() signal
    // 5) Cause of this updateActions was not called
    // So call it manually
    updateActions();
    kDebug() << "Suspend is called!";
}

