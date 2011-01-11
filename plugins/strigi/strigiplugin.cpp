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
    SystrayPlugin(i18n("Strigi File Indexer"),"nepomukstrigiservice",parent),
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
    connect(d->strigiInteface,SIGNAL(statusChanged()),
            this,SLOT(serviceStatusChanged()));

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

void StrigiSystrayPlugin::serviceStatusChanged()
{
    this->updateActions();
    // Call parent function to emit necessary signals
    SystrayPlugin::emitServiceStatusChanged();
}

void StrigiSystrayPlugin::serviceSystemStatusChanged()
{
    this->serviceStatusChanged();
}

void StrigiSystrayPlugin::serviceInitialized(bool success)
{
    this->serviceStatusChanged();
}

void StrigiSystrayPlugin::updateActions()
{

    if(!isServiceInitialized()) {
        kDebug() << "Service is not initialized";
        d->srAction->setEnabled(false);
        return;
    }

    QDBusPendingReply<bool> repl1 = d->strigiInteface->isSuspended();
    repl1.waitForFinished();
    if (!repl1.isValid() ) { 
        // error. Probaly hungs
        d->srAction->setEnabled(false);
        kDebug() << "Checking for suspend. Failed to recive reply from service.\
            Error: " << repl1.error().message();
        return;
    }
    else {
        bool answer = repl1.value();
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

SystrayPlugin::ShortStatus StrigiSystrayPlugin::shortStatus() const
{
    if (!isServiceRegistered()) {
        return NotStarted;
    }
    else if (!isServiceInitialized()) {
        return Failed;
    }
    else {
        QDBusPendingReply<bool> repl1;
        repl1 = d->strigiInteface->isSuspended();
        repl1.waitForFinished();
        if (!repl1.isValid()) {
            // Error. Probaly hung
            return Failed;
        }
        bool answer = repl1.value();
        if ( answer ) {
            // suspended
            return Suspended;
        }
        else {
            // Running, but may be idle
            repl1 = d->strigiInteface->isIndexing();
            repl1.waitForFinished();
            if (!repl1.isValid()) {
                // Error. Probably hung
                return Failed;
            }
            bool answer2 = repl1.value();
            if ( answer2 ) {
                // Indexing
                return Running;
            }
            
            return Idle;
        }
    }
}


void StrigiSystrayPlugin::slotSuspend(bool)
{
    // Check what to do - suspend or resume
    // We will check button, not service itself
    if ( d->srAction->isActive() ) { // Active means user asked to suspend
        QDBusPendingReply<> reply = d->strigiInteface->suspend();
        //reply.waitForFinish();
    }
    else {
        QDBusPendingReply<> reply = d->strigiInteface->resume();
        //reply.waitForFinish();
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

/*
QStringList StrigiSystrayPlugin::actionSystemNames() const
{
    static QStringList answer;
    static bool init = false;
    if ( !init) {
        answer << "suspend/resume";
        init = false;
    }
    return answer;
}
*/
