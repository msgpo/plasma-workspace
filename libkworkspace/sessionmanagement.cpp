/*
   Copyright (C) 2019 David Edmundson <davidedmundson@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the Lesser GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the Lesser GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "sessionmanagement.h"

#include "sessionmanagementbackend.h"

#include <KSharedConfig>
#include <KAuthorized>

#include "kscreenlocker_interface.h"
#include "screenlocker_interface.h"
#include "logoutprompt_interface.h"
#include "shutdown_interface.h"

// add a constructor with the servica names and paths pre-populated
class LogoutPromptIface : public OrgKdeLogoutPromptInterface {
    Q_OBJECT
public:
    LogoutPromptIface():
        OrgKdeLogoutPromptInterface(QStringLiteral("org.kde.LogoutPrompt"), QStringLiteral("/LogoutPrompt"), QDBusConnection::sessionBus())
    {}
};

class ShutdownIface : public OrgKdeShutdownInterface {
    Q_OBJECT
public:
    ShutdownIface():
        OrgKdeShutdownInterface(QStringLiteral("org.kde.ksmserver"), QStringLiteral("/Shutdown"), QDBusConnection::sessionBus())
    {}
};

SessionManagement::SessionManagement(QObject *parent):
    QObject(parent)
{
    auto backend = SessionBackend::self();
    connect(backend, &SessionBackend::stateChanged, this, &SessionManagement::stateChanged);
    connect(backend, &SessionBackend::canShutdownChanged, this, &SessionManagement::canShutdownChanged);
    connect(backend, &SessionBackend::canRebootChanged, this, &SessionManagement::canRebootChanged);
    connect(backend, &SessionBackend::canSuspendChanged, this, &SessionManagement::canSuspendChanged);
    connect(backend, &SessionBackend::canHybridSuspendChanged, this, &SessionManagement::canHybridSuspendChanged);
    connect(backend, &SessionBackend::canHibernateChanged, this, &SessionManagement::canHibernateChanged);
}

bool SessionManagement::canShutdown()
{
    return canLogout() && SessionBackend::self()->canShutdown();
}

bool SessionManagement::canReboot()
{
    return canLogout() && SessionBackend::self()->canReboot();
}

bool SessionManagement::canLogout()
{
    // checking both is for compatibility with old kiosk configs
    // authorizeAction is the "correct" one
    return KAuthorized::authorizeAction(QStringLiteral("logout")) && KAuthorized::authorize(QStringLiteral("logout"));
}

bool SessionManagement::canSuspend()
{
    return SessionBackend::self()->canSuspend();
}

bool SessionManagement::canHybridSuspend()
{
    return SessionBackend::self()->canHybridSuspend();
}

bool SessionManagement::canHibernate()
{
    return SessionBackend::self()->canHibernate();
}

bool SessionManagement::canSwitchUser()
{
    return KAuthorized::authorizeAction(QStringLiteral("start_new_session"));
}

bool SessionManagement::canLock()
{
    return KAuthorized::authorizeAction(QStringLiteral("lock_screen"));
}

SessionManagement::State SessionManagement::state()
{
    return SessionBackend::self()->state();
}

void SessionManagement::requestShutdown()
{
    if (!canShutdown()) {
        return;
    }
    if (SessionBackend::self()->confirmLogout()) {
        LogoutPromptIface iface;
        iface.promptShutDown();
    } else {
        ShutdownIface iface;
        iface.logoutAndShutdown();
    }
}

void SessionManagement::requestReboot()
{
    if (!canReboot()) {
        return;
    }
    if (SessionBackend::self()->confirmLogout()) {
        LogoutPromptIface iface;
        iface.promptReboot();
    } else {
        ShutdownIface iface;
        iface.logoutAndReboot();
    }
}

void SessionManagement::requestLogout()
{
    if (!canLogout()) {
        return;
    }
    if (SessionBackend::self()->confirmLogout()) {
        LogoutPromptIface iface;
        iface.promptLogout();
    } else {
        ShutdownIface iface;
        iface.logout();
    }
}

void SessionManagement::suspend()
{
    if (!canSuspend()) {
        return;
    }
    SessionBackend::self()->suspend();
}

void SessionManagement::hybridSuspend()
{
    if (!canHybridSuspend()) {
        return;
    }
    SessionManagement::hybridSuspend();
}

void SessionManagement::hibernate()
{
    if (!canHibernate()) {
        return;
    }
    SessionBackend::self()->hibernate();
}

void SessionManagement::lock()
{
    if (!canLock()) {
        return;
    }
    OrgFreedesktopScreenSaverInterface iface(QStringLiteral("org.freedesktop.ScreenSaver"), QStringLiteral("/ScreenSaver"), QDBusConnection::sessionBus());
    iface.Lock();
}

void SessionManagement::switchUser()
{
    if (!canSwitchUser()) {
        return;
    }
    OrgKdeScreensaverInterface iface(QStringLiteral("org.freedesktop.ScreenSaver"), QStringLiteral("/ScreenSaver"), QDBusConnection::sessionBus());
    iface.SwitchUser();
}

#include "sessionmanagement.moc"
