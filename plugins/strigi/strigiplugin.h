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
 
#ifndef NEPOMUK_SYSTRAY_STRIGI_PLUGIN
#define NEPOMUK_SYSTRAY_STRIGI_PLUGIN

#include "systrayplugin.h"

#include <kdemacros.h>

#include <QtCore/QObject>


namespace Nepomuk
{
    class StrigiSystrayPlugin : public SystrayPlugin
    {
        Q_OBJECT;
        public:
            StrigiSystrayPlugin(QObject *,const QList<QVariant>&);
            virtual ~StrigiSystrayPlugin();
            virtual void shortStatusRequest() const;
            virtual bool userOriented() const { return true; }
        protected:
            virtual void doInit();

        protected Q_SLOTS:

            virtual void serviceInitialized(bool);
             
            void updateActions();
        private Q_SLOTS:
            void slotSuspend(bool);
            
            void isServiceSuspended(const char * answerSlot);
            void isServiceIndexing(const char * answerSlot);
            // Pipeline for short status request
            void _k_ssr_stage2(bool);
            void _k_ssr_stage3(QDBusPendingCallWatcher*);
            void _k_ssr_stage4(QDBusPendingCallWatcher*);

            // Pipeline for updateActions
            void _k_ua_stage2(bool);
            void _k_ua_stage3(QDBusPendingCallWatcher*);
        private:
            class Private;
            Private * const  d;
    };
}

#endif
