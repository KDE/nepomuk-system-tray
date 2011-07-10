/*
 *   Copyright 2011 Serebriyskiy Artem <v.for.vandal@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
 
#ifndef NEPOMUK_PLASMA_ENGINE_H_
#define NEPOMUK_PLASMA_ENGINE_H_
 
#include <Plasma/DataEngine>

#include "systrayplugin.h"

/**
 * This engine provides statuses for all Nepomuk services.
 *
 */
class NepomukEngine : public Plasma::DataEngine
{
    Q_OBJECT;
 
    public:
        // every engine needs a constructor with these arguments
        NepomukEngine(QObject* parent, const QVariantList& args);
 
    protected:
        // this virtual function is called when a new source is requested
        bool sourceRequestEvent(const QString& name);
 
        // this virtual function is called when an automatic update
        // is triggered for an existing source (ie: when a valid update
        // interval is set when requesting a source)
        bool updateSourceEvent(const QString& source);

        // Init method
        virtual void init();

        // Sources
        QStringList sources() const;
    protected Q_SLOTS:
        void pluginChanged(Nepomuk::SystrayPlugin*);
        // This special slot is required because status message is
        // not cached inside plugin.
        void statusMessageChanged( Nepomuk::SystrayPlugin*, QString );
        void shortStatusChanged( Nepomuk::SystrayPlugin* , Nepomuk::SystrayPlugin::ShortStatus );
    private:
        void connectSignals( Nepomuk::SystrayPlugin * );
        void updateProperties( Nepomuk::SystrayPlugin * );
        QHash<QString,Nepomuk::SystrayPlugin * > m_plugins;
};
 
#endif // TESTTIMEENGINE_H
