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
 

#include "nepomukbackupsyncplugin.h"

/* Uncomment next line if you need DBus interface. See CMakeLists.txt
 * for the name of the generated header
 */
// #include <nepomukbackupsyncserviceinterface.h>

#include <QtCore/QString>

#include <KPluginFactory>
#include <KGuiItem>
#include <KLocale>
#include <KDebug>
#include <KDualAction>
#include <KActionMenu>
#include <KToolInvocation>
#include <KDesktopFile>
#include <KActionCollection>
using namespace Nepomuk;

K_PLUGIN_FACTORY(nepomukbackupsyncSystrayPluginFactory, registerPlugin< Nepomuk::nepomukbackupsyncSystrayPlugin >();)
K_EXPORT_PLUGIN(nepomukbackupsyncSystrayPluginFactory("systraynepomukbackupsyncplugin"))

class nepomukbackupsyncSystrayPlugin::Private
{
    public:
        Private():
            callBackupAction(0) {;}

        KAction * callBackupAction;
};

nepomukbackupsyncSystrayPlugin::nepomukbackupsyncSystrayPlugin( QObject * parent,const QList<QVariant>&):
    SystrayPlugin(
            KDesktopFile("services",QLatin1String("nepomukbackupsync.desktop")),
            QLatin1String("nepomukbackupsync"),
            parent),
    d(new Private())
{
    /* Init XMLGUI part. Usually you should not change this line -
     * it is perfectly correct
     */
    setXMLFile(QLatin1String("systraynepomukbackupsyncpluginui.rc"));

    this->d->callBackupAction = new KAction(0);
    d->callBackupAction->setText( i18nc("@action:inmenu","Backup Nepomuk") );

    connect(d->callBackupAction, SIGNAL(triggered()),
            this, SLOT(callBackup())
           );

    actionCollection()->addAction(QLatin1String("backup"),d->callBackupAction);



}


void nepomukbackupsyncSystrayPlugin::doInit()
{
    ;
}

nepomukbackupsyncSystrayPlugin::~nepomukbackupsyncSystrayPlugin()
{
    // Delete members and private class here.
    // DO NOT DELETE ACTIONS: It cause segfault for unknown reason:
    // DONT! -> delete d->action1;

    delete d;
}

void nepomukbackupsyncSystrayPlugin::serviceInitialized(bool success)
{
    if ( success )
        setShortStatus(Running);
    else
        setShortStatus(Failed);
}

void nepomukbackupsyncSystrayPlugin::callBackup()
{
    QString error;
    int ret = 
        KToolInvocation::startServiceByDesktopName(
                QLatin1String("nepomukbackup"),
                QString(),
                &error,
                0,
                0,
                QByteArray(),
                false
                );


    if ( ret > 0 ) {
        qDebug() << error;
    }
}
