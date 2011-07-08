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

#include "servicewidget.h"
#include "systrayplugin.h"

#include <KXMLGUIFactory>
#include <KDebug>
#include <KMenu>

#include <QtGui/QMessageBox>

using namespace Nepomuk;

class SystrayServiceWidget::Private
{
    public:
        SystrayPlugin * plugin;
        KXMLGUIFactory * factory;
};


SystrayServiceWidget::SystrayServiceWidget(SystrayPlugin * plugin, KXMLGUIFactory * factory, QWidget * parent):
    QFrame(parent),
    d(new Private())
{
    this->setupUi(this);
    this->infoPushButton->setIcon(KIcon(QLatin1String("dialog-information")));
    connect( this->infoPushButton, SIGNAL(clicked()),
	    this, SLOT(onShowInfo())
	   );

    d->plugin = plugin;
    if (!factory) {
        factory = plugin->factory();
    }
    if (!factory) {
        // Well, then plugin has not been assigned
        // Do nothing, simply return
        kError() << "Unhandled situation with factory. Reprot a bug please";
        return;
    }
    d->factory = factory;
    // FIXME Recreate factory if no factory is passed

    if (plugin->isInitialized()) {
        kDebug() << "Plugin already initialized";
        doInit(d->plugin);
    }
    else {
        connect(plugin,SIGNAL(initializationFinished(Nepomuk::SystrayPlugin*)),
                this,SLOT(doInit(Nepomuk::SystrayPlugin*))
               );
    }
}

SystrayServiceWidget::~SystrayServiceWidget()
{
    delete d;
}

void SystrayServiceWidget::doInit(SystrayPlugin * plugin)
{
    // It only works if we do not allow to change d->plugin in runtime
    Q_ASSERT(d->plugin == plugin);

    // Handle signals
    disconnect(plugin,SIGNAL(initializationFinished(Nepomuk::SystrayPlugin*)),
            this,SLOT(doInit(Nepomuk::SystrayPlugin*))
           );
    connect(plugin,SIGNAL(shortStatusChanged(Nepomuk::SystrayPlugin*,Nepomuk::SystrayPlugin::ShortStatus)),
            this,SLOT(onShortStatusChanged(Nepomuk::SystrayPlugin*,Nepomuk::SystrayPlugin::ShortStatus))
           );
    connect(plugin,SIGNAL(statusMessageChanged(Nepomuk::SystrayPlugin*,QString)),
            this,SLOT(onStatusMessageChanged(Nepomuk::SystrayPlugin*,QString))
           );

    // Handle some constant parameters - name of service, menu etc
    this->nameLabel->setText(plugin->serviceName());

    // Get menu
    QWidget * w = d->factory->container(plugin->dbusServiceName() + QLatin1String("_actions"),plugin);
    KMenu * actionsMenu = qobject_cast<KMenu*>(w);
    if (!actionsMenu) {
        kDebug() << "Failed to retrieve menu";
        this->actionsButton->setEnabled(false);
    }
    else if (actionsMenu->isEmpty() ) {
        this->actionsButton->setEnabled(false);
    }
    else {
        this->actionsButton->setMenu(actionsMenu);
        this->actionsButton->setEnabled(true);
    }

    this->statusLabel->setText(
            shortStatusLabelText(plugin->shortStatus())
            );
}

void SystrayServiceWidget::onShortStatusChanged(SystrayPlugin* plugin, SystrayPlugin::ShortStatus status)
{
    this->statusLabel->setText(
            shortStatusLabelText(status)
            );
}

void SystrayServiceWidget::onStatusMessageChanged(Nepomuk::SystrayPlugin*,QString message)
{
}

void SystrayServiceWidget::onShowInfo()
{
    QString description;
    if ( d->plugin->isInitialized() )
	description = d->plugin->serviceDescription();
    else
	description = i18nc("@info:status Plugin has not initialize yet.","Plugin has not initialize yet. Try few milliseconds later"); 

    QMessageBox::information(this,d->plugin->serviceName(),description);
}

void SystrayServiceWidget::setShown(bool shown)
{
    if (shown) 
        this->show();
    else 
        this->hide();
}

bool SystrayServiceWidget::userOriented() const
{
    return d->plugin->userOriented();
}

QString SystrayServiceWidget::shortStatusLabelText(SystrayPlugin::ShortStatus status)
{
    static QString shortStatusStringTemplate = QString::fromLatin1("<font color='%1'>%2</font>");
    QColor textColor;
    switch(status)
    {
        case( SystrayPlugin::Running):
        case (SystrayPlugin::Idle): { textColor = QColor::fromRgbF(0,1,0); break; }
        case (SystrayPlugin::Suspended): { textColor = QColor::fromRgbF(0.5,0.5,0); break; }
        case (SystrayPlugin::Launching): { textColor = QColor::fromRgbF(0,0,1); break; }
        case(SystrayPlugin::Failed) : { textColor = QColor::fromRgbF(1.0,0,0); break; }
        default : { textColor = QColor::fromRgbF(0,0,0); break; }
    }
    return shortStatusStringTemplate.arg(textColor.name(),SystrayPlugin::shortStatusToString(status));

}
