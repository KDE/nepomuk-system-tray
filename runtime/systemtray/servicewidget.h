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

#ifndef NEPOMUK_SYSTRAY_SERVICEWIDGET_H_
#define NEPOMUK_SYSTRAY_SERVICEWIDGET_H_

#include <QtGui/QWidget>

#include "ui_serviceWidget.h"

class KXMLGUIFactory;

namespace  Nepomuk {
    class SystrayPlugin;

    class SystrayServiceWidget : public QFrame, public Ui_serviceWidget
    {
        Q_OBJECT;
        public:
            SystrayServiceWidget(SystrayPlugin * plugin, KXMLGUIFactory * factory = 0, QWidget * parent = 0);
        public Q_SLOTS:
            void setShown(bool);
        private Q_SLOTS:
            void doInit(Nepomuk::SystrayPlugin * plugin);
            void onShortStatusChanged();
            void onStatusMessageChanged();
        private:
            class Private;
            Private * const d;
    };
}


#endif
