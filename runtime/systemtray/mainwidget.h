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

#ifndef NEPOMUK_SYSTEMTRAY_MAINWIDGET_H
#define NEPOMUK_SYSTEMTRAY_MAINWIDGET_H

#include <QtGui/QMainWindow>

#include "ui_mainWidget.h"

class QVBoxLayout;
class KXMLGUIFactory;

namespace Nepomuk {
    class SystrayPlugin;
    class SystrayServiceWidget;
    class MainWidget : public QWidget, public Ui_mainWidget
    {
        Q_OBJECT;
        public:
            /*! \brief Constructor
             * \param approxWidgetsShown - This parameter is used to determine size
             * of main widget. The height of widget will be approximately
             *  approxWidgetsShown*height_of_service_widgets. 
             */
            explicit MainWidget(KXMLGUIFactory * factory, QWidget * parent = 0, int approxWidgetsShown = 5);
            void addPlugin( SystrayPlugin * plugin);
        //Q_SIGNALS:
            /* This signal is necessary for top window
             * to change it's size accordingly
             */
            void updateSize() {
                showAll(showAllCheckBox->isChecked());
            }
        private Q_SLOTS:
            void showAll(bool);
        private:
            QVBoxLayout * m_pluginLayout;
            KXMLGUIFactory * m_factory;
            // This memeber is only necessary 
            // to adjust size of main widget dynamically.
            // When number of services displayed changes,
            // function will determine how many plugins are
            // visible and create new size of main widget.
            QList<SystrayServiceWidget*> pluginWidgets;
            int  m_approxWidgetsShown;
    };
}
#endif
