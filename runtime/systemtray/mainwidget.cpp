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
#include <QtGui/QSizePolicy>
#include <QtCore/QtDebug>
#include <QtCore/QObject>
#include <KDebug>

using namespace Nepomuk;

MainWidget::MainWidget(KXMLGUIFactory * factory, QWidget * parent, int approxWidgetsShown ):
    QWidget(parent)
{
    this->setupUi(this); 
    this->setAttribute(Qt::WA_DeleteOnClose,false);
    this->servicesArea->setWidgetResizable(true);
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
    if ( approxWidgetsShown < 1 )
        m_approxWidgetsShown = 1;
    else
        m_approxWidgetsShown = approxWidgetsShown;

    connect( this->showAllCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(showAll(bool))
           );
}

void MainWidget::addPlugin( SystrayPlugin * plugin )
{
    SystrayServiceWidget * w = new SystrayServiceWidget(plugin,m_factory);

#if 0
    if ( !w->userOriented()) {
        // Hide it if checkbox is not selected
        if ( !showAllCheckBox->isChecked() ) {
            w->hide();
        }
        // Attach it to the signal
        /*
        connect( this->showAllCheckBox, SIGNAL(toggled(bool)),
                w, SLOT(setShown(bool))
               );
               */
    }
#endif
    this->pluginLayout->insertWidget(0,w);
    w->setFrameStyle(QFrame::Raised|QFrame::StyledPanel);
    w->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    pluginWidgets << w;
    showAll(showAllCheckBox->isChecked());
}

void MainWidget::showAll(bool toggled)
{
    
    int result_height = 0;
    int width = showAllCheckBox->size().width();

    int count = 0;

    foreach(SystrayServiceWidget * wc, pluginWidgets)
    {

        QSize size = wc->size();

        if ( toggled or wc->userOriented() ) {
            // Adjust size only if it is not too much already
            // and QSize object is valid
            if ( count <= m_approxWidgetsShown  and size.isValid() )
                // Dirty hack
                result_height += size.height();
            wc->show();
            count++;
        }
        else 
            wc->hide();

        width = qMax(width,size.width());
        
    }

    //qDebug() << "Childrens: " << count;
    //qDebug() << "Result size hint " << QSize(width,result_height);

    //this->servicesAreaContents->adjustSize();
    //qDebug() << "Area size " << this->realMainWidget->size();
    //qDebug() << "Area size hint" << this->realMainWidget->sizeHint();
    this->resize(
            this->size().width(), // dirty hack 
            result_height + showAllCheckBox->height()*1.5 + 100);

}
