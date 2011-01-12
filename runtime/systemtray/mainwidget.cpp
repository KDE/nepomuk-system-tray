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
#include "mainwidget.h"
#include "servicewidget.h"
#include "systrayplugin.h"

#include <QtGui/QWidget>
#include <QtGui/QWidget>
#include <QtGui/QSizePolicy>
#include <KDebug>

using namespace Nepomuk;

MainWidget::MainWidget(KXMLGUIFactory * factory, QWidget * parent ):
    QWidget(parent)
{
    this->setupUi(this); 
    //m_pluginLayout = new QVBoxLayout();
    //m_pluginLayout->setObjectName("pluginLayout");
    
    // Workaround stupid scroll area behaviour
#if 0
    QWidget * newBaseWidget = new QWidget();
    newBaseWidget->setLayout(m_pluginLayout);
    this->servicesArea->setWidget(newBaseWidget);
#endif
    //this->servicesAreaContents->setLayout(m_pluginLayout);

    m_factory = factory;
}

void MainWidget::addPlugin( SystrayPlugin * plugin )
{
    SystrayServiceWidget * w = new SystrayServiceWidget(plugin,m_factory);

    if ( !plugin->userOriented()) {
        // Hide it if checkbox is not selected
        if ( !showAllCheckBox->isChecked() ) {
            w->hide();
        }
        // Attach it to the signal
        connect( this->showAllCheckBox, SIGNAL(toggled(bool)),
                w, SLOT(setShown(bool))
               );
    }
    this->pluginLayout->insertWidget(0,w);
    w->setFrameStyle(QFrame::Raised|QFrame::StyledPanel);
    w->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
}
