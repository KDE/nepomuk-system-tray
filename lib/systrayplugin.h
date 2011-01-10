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
 

#ifndef _NEPOMUK_SYSTRAY_PLUGIN
#define _NEPOMUK_SYSTRAY_PLUGIN
#include <kdemacros.h>
#include <QtCore/QObject>
#include <KXMLGUIClient>
#include <QString>
#include <QStringList>

#include "nepomuksystray_export.h"
class KAction;
class KActionCollection;
class KActionMenu;

namespace Nepomuk
{
    /*! \brief This class is a base for plugins for Nepomuk System tray
     * Methods that you must implement:
     *  - serviceInitialized - this slot is called when service report it's initialization
     *    status
     *  - serviceSystemStatusChanged - this slot is called by default implementations of
     *    \fn serviceRegistred and some other. Although you can reimplement these methods
     *    you still have to implement serviceSystemStatusChanged, even if it wouldn't do
     *    anything. This is done just to prevent logic erros.
     *  - doInit. In this method you should do all D-Bus related initialization.
     * Methods you should implement:
     *  - shortStatus. Report short status of the service. Default can distinguish between
     *    limited states
     *  - statusMessage. Return text describing status of the service. Default 
     *    implementation return empty string.
     * Signals you should emit:
     *  - shortStatusChanged - emit when short status is changed
     *  - statusMessageChanged - emit when status message is changed. 
     * <b> NEVER EMIT initializationFinished </b>
     *
     */
    class NEPOMUKSYSTRAY_EXPORT SystrayPlugin : public QObject, public KXMLGUIClient
    {
        Q_OBJECT;
        public:
            /*! \brief Constructs and partitialy initialize plugin
             * In this method you should perform all initialization that doesn't require
             * a lot of time. You shouldn't try to communicate with DBus services here
             * cause they may be hung and it would take a lot of time for QDBus to detect
             * and return an error. Use init() method for that.
             * \param serviceName Human-readable name of the service. Please don't use 
             * long names - this name will appear in menu
             * \param dbusServiceName Every service has the 'system' name. This is the
             * name that can be used in org.kde.NepomukServer:isServiceRunning D-Bus
             * method and this is the last part of the name of the endpoint for the 
             * service - org.kde.nepomuk.services.<dbusServiceName>. For example Strigi
             * has system name nepomukstigiservice.
             */
            SystrayPlugin(QString serviceName, QString dbusServiceName, QObject *); 

            /*! \brief Returl the full name of the Service
             * Please keep it not very long - this name will appear in the menu.
             * Example is "Strigi File Indexer" or, more preferably "Indexing Service"
             */
            QString serviceName() const;

            /*! \brief Return the short name of the Service
             * This name will be used as prefix to some top-level actions. So please keep
             * it short. Examples are "Strigi" or "Indexing". Do not append colon after name-
             * if it will be necessary, it will be done automatically
             * Please avoid spaces.
             * By default, short name is equal to service name. So usually you will have
             * to change it.
             */
            QString shortServiceName() const;

            /*! \brief Return the description of the service
             * This text will(may) appear in the toolTip. There are no restrictions
             * to size of the text. ( Well, you, of course, should avoid using a 10-pages
             * text as description )
             * \sa setServiceDescription()
             */
            QString serviceDescription() const;

#if 0
            /*! \brief Whether the service is user-oriented
             * Return true if you service is what user probably want to see in quick 
             * access. If the return value is false, then widgets that displays service
             * information will be hidden by default untill user explicitly ask to expand
             * them.
             * So, for example, for Strigi service return value is <i>true</i> and for
             * queryservice - <i>false</i>
             * As the rule of the thumb - if you don't allow user to control the service,
             * then you should return false
             * \note Default implementation will check that suspendResumeAction() is not
             * null
             */
            virtual bool userVisible() const { return actions() != 0; }

            /*! \brief This method returns all actions that this contol module expose in given enviroment
             * In given enviroment means that you can adjust list of the exposed actions
             * according to some external circumstances( such as API version and so on )
             * But you should guarantee that actions with same meaning will always have
             * the same key in collection.
             * You can return 0 ( this is the default value ) and this means that you
             * have no actions to expose.
             * Please be aware that if 0 is returned, then result of the \fn menu() 
             * call will be considered 0 too.
             * This action collection is used to allow user select items to be used
             * in top-level menu. So, if you have a rear case when some actions can
             * not be exposed, you may exclude them from this collection.
             * Returning empty collection will be interpreted in the right way and 
             * \fn menu() will still be checked.
             * This method will be called only after doInit()
             * \sa menu() 
             */
            virtual KActionCollection * actions() const { return 0;}

            /*! \brief This method return names of all possible exposed actions in all possible enviroments
             * The purpose of this function is to be used when configuring what actions
             * should go into the top level and so on. Cause there is no need in KAction
             * objects you don't need to return KActionCollection and you should return 
             * QStringList instead.
             * Default implementation can't do anything because KActionCollections is 
             * a very stange structure
             * This method can be called at any time and it should not depend from any
             * initialization function ( except constructor )
             */
#if 0
             * Default implementation will take all names from actions() result, so if 
             * you are exposing some actions only in case some conditions are met, you 
             * should overload this method
#endif
            virtual QStringList actionSystemNames() const;

            /*! \brief Return menu for this plugin.
             * This method is used to allow plugin return a menu that will be 
             * used as submenu in the main one.
             * The \fn actions() is not enough because KActionCollection doesn't 
             * allow organizing into submenus.
             * If 0 is returned, then the value of actions() will be considered.
             * If it is non-0, then menu will be build automatically.
             * If it is 0, then this will mean that plugin has no action and menus
             * to expose to the user
             * Default implementation return 0 .
             */
            virtual KActionMenu * menu() const { return 0; } 
#endif

            /*! \brief This is the predefined set of constants that describes the state of the service.
             * Of course, you service can have more states, e.g. "No space left on the device" or "I am in pain!". You can use
             * all this in the extended statuses. Do not introduce new constants for the ShortStatus.
             */
             enum ShortStatus { 
                 /* The service is running, making some job(s)  and everything is ok
                  */
                 Running, 
                 
                 /* The service has successfully started, but don't do any job right now. For
                  * example, successfuly started Strigi service that is not analyzing any file
                  * right now.
                  */
                 Idle, 
                 
                 /* The service has successfully started, but was suspended by user. It wouldn't
                  * do anything unless user resume it or service unresumes itself.
                  */
                 Suspended, 
                 
                 /* The service has not started. Either because it should'd not or because some error
                  * has happend. You should use this status when there D-Bus endpoint of the service
                  * doesn't exist
                  */
                 NotStarted,

                 /* The service has started ( so the D-Bus endpoint of the service does exist and service
                  * is not hanged ) but can't do anything because of some error. E.g. "No space left on the device"
                  * and so on
                  */
                 Failed
             };

             /*! \brief This method returns the short status of the service
              * If you service supports more statuses then select the most appropriate one. <b>Do not return
              * statuses not from ShortStatus.</b>
              * You <b>must</b> implement this method. It is vital!
              * Default implementation can only distinguish between Running, NotStarted
              * and Failed states
              */
             virtual ShortStatus shortStatus() const;

             /*! \brief Return the arbitrary service status description
              * This is the method where you can use all status messages you wan't to use.
              * But be aware that if shortStatus if the service is NotStarted 
              * status string wouldn't be shown.
              * If service doesn't have status message, return null or empty QString
              * If service has an error and you don't have a message for this error,
              * you should return null or empty string.
              * Default implemntation will return Null string.
              * \sa shortStatus()
              */
             virtual QString serviceStatusMessage() const;


             /*! \brief Return the service error message
              * This method will be called only if you short status is Failed or NotStarted
              * \note Please do not forget to handle in implementations of this method the 
              * situation when service has not been started and don't have a D-Bus endpoint.
              * If you don't know why, then simply return null or empty QString
              * Default implemntation will return Null string.
              * \sa serviceStatusMessage(), shortStatus()
              */
             //virtual QString serviceErrorMessage() const ;

             virtual ~SystrayPlugin();

             /*! \brief Return string name of ShortStatus value
              */
             static QString shortStatusToString(ShortStatus status);
         public Q_SLOTS:
            /*! \brief Initialization of the plugin
             * In this method you should perform all necessary initialization procedures
             * that requires sugnificatn amount of time: 
             * - Check that service is running
             * - Create necessary memebers, load icons and so on
             * To prevent errors you can't overload this method - instead
             * you should implement pure virtual protected function \fn doInit()
             */
             void init();
         Q_SIGNALS:
             /*! \brief Emit this signal when you short status has changed
              */
             void shortStatusChanged(Nepomuk::SystrayPlugin *);

             /*! \brief Emit this signal when your status message or error message has changed
              */
             void statusMessageChanged(Nepomuk::SystrayPlugin*);

             /*! \brief This signal is for system use. 
              * It is automatically emited when plugin finish initialization.
              * <b> NEVER SEND IT MANUALLY </b>
              */
             void initializationFinished(Nepomuk::SystrayPlugin *);
         protected Q_SLOTS:
             /*! \brief This slot is called when service is initialized
              * Default implementation will call serviceSystemStatusChanged()
              * You may wish to implement it
              */
             virtual void serviceInitialized(bool) = 0;

             /*! \brief This slot is called when D-Bus enpoind appears
              * Default implementation will call serviceSystemStatusChanged()
              * You may wish to implement it
              */
             virtual void serviceRegistered()
             { this->serviceSystemStatusChanged(); }
             
             /*! \brief This slot is called when D-Bus endpoint disappear
              * Default implementation will call serviceSystemStatusChanged()
              * You may wish to implement it
              */
             virtual void serviceUnregistered() 
             { this->serviceSystemStatusChanged(); }

             /*! \brief This function is called when owher of the service has changed
              * Default implementation will call serviceSystemStatusChanged()
              * You may wish to implement it
              */
             virtual void serviceOwnerChanged()
             { this->serviceSystemStatusChanged(); }

             /*! \brief Special method that is called by default when D-Bus status of the service is changed
              * This method is called by \fn serviceRegistered , \fn serviceUnregistered
              * \fn serviceOwnerChanged 
              * when service is registred, unregistred and so on.
              * You can implement this method or reimplement the above methods.
              */
             virtual void serviceSystemStatusChanged() = 0;
             
             /*! \brief This is convinience slot to emit all necessary signals when state of the service is cnanged
              * It will emit shortStatusChanged(), statusMessageChanged() and some others.
              * Use it when, for example, service is registered or unregistered and so on
              */
             void emitServiceStatusChanged();

         protected:
             /*! \brief Use this method to set the description of the service
              */
             void setServiceDescription( const QString &);


             /*! \brief Set short name of the service
              * See \fn shortServiceName for explanation
              */
             void setShortServiceName(const QString & );

             /*! \brief Return true if service was initialized
              * Checks that endpoint exist and return true if service is initialized
              */
             bool isServiceInitialized() const;

             /*! \brief Checks that necessary DBus endpoint for service exists.
              */
             bool isServiceRegistered() const;

             /*! \brief Return the 'system' name of the service
              * It is simply the parameter that you pass to constructor
              */
             QString dbusServiceName() const;

             /*! \brief Return the D-Bus address of the service
              */
             QString dbusServiceAddress() const;

             /*! \brief Function where real time-consuming initialization happens
              * You can do whatever you need here, but
              * <b>DO NOT SEND ANY SIGNALS FROM HERE</b>
              */
             virtual void doInit() = 0;
        private Q_SLOTS:
            void _k_serviceRegistered();
            void _k_serviceUnregistered();
            void _k_serviceOwnerChanged();

        private:
             //void updateControlInterface();
             class Private;
             Private * const d;

    };
}
#endif
