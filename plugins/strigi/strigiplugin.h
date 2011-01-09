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
 
#ifndef _NEPOMUK_SYSTRAY_STRIGI_PLUGIN
#define _NEPOMUK_SYSTRAY_STRIGI_PLUGIN
#include <kdemacros.h>
#include <QtCore/QObject>

#include "systrayplugin.h"

namespace Nepomuk
{
    class StrigiSystrayPlugin : public SystrayPlugin
    {
        Q_OBJECT;
        public:
            StrigiSystrayPlugin(QObject *,const QList<QVariant>&);
            virtual ~StrigiSystrayPlugin();
            virtual KActionCollection * actions() const; 
            virtual KActionMenu * menu() const; 
            virtual ShortStatus shortStatus() const;
            virtual void serviceStatusChanged();
            //virtual QString serviceStatusMessage() const;
            //virtual QString serviceErrorMessage() const;
        protected:
            virtual void doInit();

        protected Q_SLOTS:
            virtual void serviceRegistered();
             
            virtual void serviceUnregistered(); 

            virtual void serviceInitialized(bool);

            virtual void serviceOwnerChanged();
             
            void updateActions();
        private Q_SLOTS:
            void slotSuspend(bool);
        private:
            class Private;
            Private * const  d;
    };
}

#endif
