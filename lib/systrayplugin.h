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
 

#ifndef NEPOMUK_SYSTRAY_PLUGIN
#define NEPOMUK_SYSTRAY_PLUGIN

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QMetaType>

#include <KXMLGUIClient>

#include "nepomuksystray_export.h"
class KAction;
class QDBusPendingCallWatcher;
class KDesktopFile;

namespace Nepomuk
{
    /*! \brief This class is a base for plugins for Nepomuk System tray
     * You should inhert this class and implement several methods. Which methods should
     * be implemented depends from you target service. All Nepomuk services export the
     * control object to the D-Bus. SystrayPlugin will hide functions of this control
     * object behind it's own functions. And it provide some convinient virtual methods
     * like serviceRegisterd( called when D-Bus address appears ), serviceUnregistred (
     * called when D-Bus address disappears ) and some other. The default implementations
     * of these functions is enough for almost all system Nepomuk services ( like query
     * service and so on ). If you service provide suspend/resume functionality, then
     * you have to implement a few more functions ( very easy, by the word ). If you
     * service has really different API that you may be have to reimplement most of the
     * functions. 
     *
     * The service allow some user controlling interaction ( like suspend/resume ), 
     * then you should provide actions. Because SystrayPlugin inherits KXMLGUIClient,
     * it is very simple. See constructor description for more information. 
     *
     * Methods that you must implement:
     *  - serviceInitialized - this slot is called when service report it's initialization
     *    status
     *  - doInit. In this method you should do all D-Bus related initialization.
     * Methods you should/may implement:
     *  - serviceSystemStatusChanged - this slot is called by default implementations of
     *    \fn serviceRegistred, serviceUnregistered and may be other. Default implementation
     *    does nothing.
     *  - isServiceRunningRequest - if you service can distinguish between Running and Idle
     *    states
     *  - isServiceSuspendedRequest - if you service can be in suspended state.
     * Signals:
     * You should not emit any signals by hand, but rather use functions for it. E.g. to 
     * notify about changing of the status call setShortStatus() instead of emiting 
     * shortStatusChanged(). 
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
             * \param serviceDesktopFile This is the desktop file that describes the 
             * <b>service</b> ( not the plugin ). Some parameters ( like user visible 
             * service name )will be taken from it.
             * \param dbusServiceName Every service has the 'system' name. This is the
             * name that can be used in org.kde.NepomukServer:isServiceRunning D-Bus
             * method and this is the last part of the name of the endpoint for the 
             * service - org.kde.nepomuk.services.<dbusServiceName>. For example Strigi
             * has system name nepomukstigiservice.
             * @author Artem Serebriyskiy
             * 
             * In your own constructor you should usually perform the following actions
             * @code 
             * // Set file with XMUGUI settings
             * setXMLFile("myserviceui.rc");
             * // You can provide description of the service. The default description
             * // is taken from the passed desktop file
             * setServiceDescription("My service"); 
             * // Usually you should add actions here
             * actionCollection()->addAction("action1",someAction);
             * @endcode 
             */
            SystrayPlugin(const KDesktopFile & serviceDesktopFile, QString dbusServiceName, QObject *); 

            /*! \brief Returl the user-visible name of the Service
             * Example is "Strigi File Indexer". It is taken from the service .desktop
             * file. You can't change the service name.
             */
            QString serviceName() const;

            /*! \brief Return the short name of the Service
             * This name will be used as prefix to some top-level actions. So please keep
             * it short. Examples are "Strigi" or "Indexing". Do not append colon after name-
             * if it will be necessary, it will be done automatically
             * Please avoid spaces.
             * By default, short name is equal to service name. So usually you will have
             * to change it. You can do it with setShortServiceName().
             * \sa setShortServiceName()
             */
            QString shortServiceName() const;

            /*! \brief Return the description of the service
             * This text will(may) appear in some help messages. There are no restrictions
             * to size of the text. ( Well, you, of course, should avoid using a 10-pages
             * text as description )
             * \sa setServiceDescription()
             */
            QString serviceDescription() const;

            /*! \brief Whether the service is user-oriented
             * Return true if you service is what user probably want to see in quick 
             * access. If the return value is false, then widgets that displays service
             * information will be hidden by default untill user explicitly ask to expand
             * them.
             * So, for example, for Strigi service return value is <i>true</i> and for
             * queryservice - <i>false</i>
             * As the rule of the thumb - if you don't allow user to control the service,
             * then you should return false
             * \note Default implementation return false 
             */
            virtual bool userOriented() const { return false; }


            /*! \brief Whether or not this service has a status message
             * Default implementation returns false. Do not forget to overload this
             * method if you can provide statuses messages.
             */
            virtual bool hasStatusMessage() const { return false; }

            /*! \brief This is the predefined set of constants that describes the state of the service.
             * Of course, you service can have more states, e.g. 
             * "No space left on the device" or "I am in pain!". You can use
             * all this in the status messages. Do not introduce new constants 
             * for the ShortStatus. If you really need new constant, contact the 
             * maintainer
             */
             enum ShortStatus { 
                 /*! The service has started and now perform initialization procedures.
                  * Use this after DBus endpoint appears, but before service report about
                  * successful(or not) initialization. Using of this constant is optional
                  */
                 Launching,
                 /*! The service is running, making some job(s)  and everything is ok
                  */
                 Running, 
                 
                 /*! The service has successfully started, but don't do any job right now.
                  *  For example, successfuly started Strigi service that is not 
                  *  analyzing any file right now.
                  */
                 Idle, 
                 
                 /*! The service has successfully started, but was suspended by user.
                  *  It wouldn't do anything unless user resume it or the service 
                  *  unresumes itself.
                  */
                 Suspended, 
                 
                 /*! The service has not started. Either because it should'd not or 
                  * because some error has happend. You should use this status when 
                  * there D-Bus endpoint of the service doesn't exist
                  */
                 NotStarted,

                 /*! The service has started ( so the D-Bus endpoint of the service 
                  * does exist  but can't do anything because of some error or because it
                  * hung. 
                  * E.g. "No space left on the device", or simle segfault and so on
                  */
                 Failed,

             };
             Q_ENUMS(ShortStatus);

             /*! \brief This method returns the short status of the service
              * Return short status of the service. The short status is cached,
              * so this is unexpensive call.
              * \sa setShortStatus
              */
             ShortStatus shortStatus() const;

             /*! \brief This function will update status and status message of the service
              * The signals shortStatusChanged and statusMessageChanged with current 
              * status and current status messages will be 
              * emitted as the  result of the operation. This function is not expensive,
              * but can take some time to execute. It is build in the async manner.
              */
             void update();



             virtual ~SystrayPlugin();

             /*! \brief Return string name of ShortStatus value
              * This static function converts ShortStatus to QString.
              */
             static QString shortStatusToString(ShortStatus status);

             /*! \brief Return if plugin was initialized
              * Don't mix up - it doesn't check that service was initialized,
              * only that \fn init() was called.
              */
             bool isInitialized() const;

             /*! \brief Return the 'system' name of the service
              * It is simply the parameter that you pass to constructor
              */
             QString dbusServiceName() const;

             /*! \brief Return the D-Bus address of the service
              * This is the addres of the service on the D-Bus
              */
             QString dbusServiceAddress() const;

         public Q_SLOTS:
            /*! \brief Initialization of the plugin
             * This method will call your \fn doInit().
             * It is guarantee that init() will do the stuff only once - all
             * further calls will be silently ingored.
             */
             void init();
         Q_SIGNALS:
             /*! \brief Emit this signal when you short status has changed
              */
             void shortStatusChanged(Nepomuk::SystrayPlugin *, Nepomuk::SystrayPlugin::ShortStatus );

             /*! \brief Emit this signal when your status message or error message has changed
              */
             void statusMessageChanged(Nepomuk::SystrayPlugin*, QString);

             /*! \brief This signal is for system use. 
              * It is automatically emitted when plugin finish initialization.
              * <b> NEVER SEND IT MANUALLY </b>
              */
             void initializationFinished(Nepomuk::SystrayPlugin *);


             /*! \brief This signal emitted with reply about short status of the service
              */
             //void shortStatusReply( Nepomuk::SystrayPlugin::ShortStatus status) const;

             /*! \brief This signal emitted with reply for the request about service status message
              */
             //void serviceStatusMessageReply( QString statusMessage ) const;
         protected Q_SLOTS:
             /*! \brief This slot is called when service is initialized
              * Default implementation will call serviceSystemStatusChanged()
              * You may wish to implement it
              */
             virtual void serviceInitialized(bool) = 0;

             /*! \brief This slot is called when D-Bus enpoind appears
              * Default implementation will do nothing. But, before this method
              * will be called, shortStatus will be set to the Launching. 
              * You may wish to implement it if you need to do something on
              * service startup or you want to set another status.
              */
             virtual void serviceRegistered()
             { ; }
             
             /*! \brief This slot is called when D-Bus endpoint disappear
              * Default implementation will call do nothing. Behaviour
              * is like \fn serviceRegistered, but NotStarted will be set
              * as status befor this function call.
              * You may wish to implement it for the same reasons as serviceRegistered
              */
             virtual void serviceUnregistered() 
             { ; }

             /*! \brief This function is called when owher of the service has changed
              * Default implementation will do nothing.
              * Currently this function won't be called because of undefined
              * situation with Qt signals.
              * You may wish to implement it
              */
             virtual void serviceOwnerChanged()
             { ; }

             

         protected:
             /*! \brief Use this method to set the description of the service
              */
             void setServiceDescription( const QString &);


             /*! \brief Set short name of the service
              * See \fn shortServiceName for explanation
              */
             void setShortServiceName(const QString & );


             /*! \brief Set new short status of the service and emit necessary signals
              */
             void setShortStatus(ShortStatus status);


             /*! \brief Return true if service was initialized
              * Checks that endpoint exist. It will call answerSlot with 
              * QDBusPendingCallWatcher*, that embeds QDBusPendingReply<bool>.
              * Slot must have a signature
              * answerSlot(QDBusPendingCallWatcher* = 0). 
              */
             void isServiceInitialized(const char * answerMethod) const;

             /*! \brief Checks that necessary DBus endpoint for service exists.
              */
             bool isServiceRegistered() const;

             /*! \brief Function where real time-consuming initialization happens
             * In this method you should perform all necessary initialization procedures
             * that requires sugnificatn amount of time: 
             * - Check that service is running
             * - Create necessary memebers, load icons and so on
              * You can do whatever you need here, but
              * <b>DO NOT SEND ANY SIGNALS</b>
              */
             virtual void doInit() = 0;

             /*! \brief Return a reply for the request about whether service is suspended
              * Return 0 if you don't support this functionality.
              * Default implementation will return 0;
              * Example (for strigi ) is:
              * @code 
              * // strigiInterfaces is a QDBus Interface that points to strigi service.
              * QDBusPendingReply<bool> reply = d->strigiInteface->isSuspended();
              * return new QDBusPendingCallWatcher(reply,0);
              * @endcode
              */
             virtual QDBusPendingCallWatcher * isServiceSuspendedRequest()
             {return 0;}

             /*! \brief Return a reply for the request about whether service is suspended
              * Return 0 if you don't support this functionality.
              * Default implementation will return 0;
              * See \fn isServiceSuspendedRequest for explanation and examples
              */
             virtual QDBusPendingCallWatcher * isServiceRunningRequest()
             {return 0;}

             /*! \brief This function updates short status of the service
              * This function has to update the short status of the service and 
              * emint shortStatusChanged signal with new status. You can overload
              * it if you want, but default implementation is good enough.
              * If you implement it, then you <b>have to</b> make it asyncronious.
              */
             virtual void shortStatusUpdate();

             /*! \brief Return the arbitrary service status description
              * This is the method where you can use all status messages you wan't to use.
              * But be aware that if shortStatus if the service is NotStarted 
              * status string wouldn't be shown.
              * If service doesn't have status message, return null or empty QString
              * If service has an error and you don't have a message for this error,
              * you should return null or empty string.
              * Default implemntation will return Null string and will do it 
              * <b>synchroniously</b>. Be aware of deadlocks. You should rather
              * check hasStatusMessage() before using this method
              * Result should be returned with statusMessageChanged signal.
              *
              * \sa shortStatus()
              */
             virtual void serviceStatusMessageUpdate();
         private:
            /*! \brief This function checks that service is running and call given slot with result
             * Slot must have signature answerSlot(QDBusPendingCallWatcher * watchre = 0).
             *  If 
             * service doesn't support Idle/Running distinguish ( aka do not implement
             * isServiceRunningRequest ) then answerSlot will be called with NULL. 
             * So you must check that input parameter is not NULL.
             * Also do not forget to delete input QDBusPendingCallWatcher with
             * deleteLater().
             * @code
             * if (!watcher) {
             *    // Do something 
             * }
             * else {
             * watcher->deleteLater();
             *    // Do all you need here
             * }
             * @endcode
             */
            void isServiceRunning(const char* answerSlot);
            
            /*! \brief Return whether service is suspended
             * The behaviour is similar to the \fn isServiceRunning.
             */
            void isServiceSuspended(const char*);

        private Q_SLOTS:
            void _k_serviceRegistered();
            void _k_serviceUnregistered();
            void _k_serviceOwnerChanged();
            //void _k_isServiceInitializedReplyHandler(QDBusPendingCallWatcher*);
            void _k_performInit();
            void _k_ssr_stage2(QDBusPendingCallWatcher* = 0);
            void _k_ssr_stage3(QDBusPendingCallWatcher* = 0);
            void _k_ssr_stage4(QDBusPendingCallWatcher* = 0);

        private:
             void callWithNull(const char * answerSlot) const;
             //void updateControlInterface();
             class Private;
             Private * const d;

    };
}
Q_DECLARE_METATYPE(Nepomuk::SystrayPlugin::ShortStatus)
Q_DECLARE_METATYPE(Nepomuk::SystrayPlugin*)
#endif
