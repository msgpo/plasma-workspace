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
#pragma once

#include <QObject>
#include "kworkspace_export.h"

/**
 * Public facing (plasma/krunner whatever API)
 *
 */
class KWORKSPACE_EXPORT SessionManagement: public QObject
{
    Q_OBJECT
    Q_PROPERTY(State state READ state NOTIFY stateChanged)

    Q_PROPERTY(bool canShutdown READ canShutdown NOTIFY canShutdownChanged)
    Q_PROPERTY(bool canReboot READ canReboot NOTIFY canRebootChanged)
    Q_PROPERTY(bool canLogout READ canLogout NOTIFY canLogoutChanged)
    Q_PROPERTY(bool canSuspend READ canSuspend NOTIFY canSuspendChanged)
    Q_PROPERTY(bool canHibernate READ canHibernate NOTIFY canHibernateChanged)
    Q_PROPERTY(bool canSwitchUser READ canSwitchUser NOTIFY canSwitchUserChanged)
    Q_PROPERTY(bool canLock READ canLock NOTIFY canLockChanged)

public:
    enum State {
        Loading,
        Ready,
        Error
    };
    Q_ENUM(State)

    SessionManagement(QObject *parent = nullptr);
    ~SessionManagement() override = default;

    State state();

    bool canShutdown();
    bool canReboot();
    bool canLogout();
    bool canSuspend();
    bool canHybridSuspend();
    bool canHibernate();
    bool canSwitchUser();
    bool canLock();

public Q_SLOTS:
    /**
     * These requestX methods will either launch a prompt to shutdown or
     * The user may cancel it at any point in the process
     */
    void requestShutdown();
    void requestReboot();
    void requestLogout();

    void suspend();
    void hybridSuspend();
    void hibernate();

    void switchUser();
    void lock();

Q_SIGNALS:
    void stateChanged();
    void canShutdownChanged();
    void canRebootChanged();
    void canLogoutChanged();
    void canSuspendChanged();
    void canHybridSuspendChanged();
    void canHibernateChanged();
    void canSwitchUserChanged();
    void canLockChanged();
    void aboutToSuspend();
    void resumingFromSuspend();
private:
    void *d; //unused, just reserving the space in case we go ABI stable
};

