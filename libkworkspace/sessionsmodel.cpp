/*
    Copyright 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
*/

#include "sessionsmodel.h"

#include <KAuthorized>
#include <KUser>
#include <KLocalizedString>

#include "login1_manager_interface.h"
#include "login1_session_interface.h"
#include "login1_user_interface.h"

SessionsModel::SessionsModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    reload();
}

bool SessionsModel::canSwitchUser() const
{
    return KAuthorized::authorizeAction(QLatin1String("switch_user"));
}

bool SessionsModel::canStartNewSession() const
{
    return KAuthorized::authorizeAction(QLatin1String("start_new_session"));
}

bool SessionsModel::includeUnusedSessions() const
{
    return m_includeUnusedSessions;
}

void SessionsModel::setIncludeUnusedSessions(bool includeUnusedSessions)
{
    if (m_includeUnusedSessions != includeUnusedSessions) {
        m_includeUnusedSessions = includeUnusedSessions;

        reload();

        emit includeUnusedSessionsChanged();
    }
}

void SessionsModel::switchUser(int index)
{
    m_backend->switchUser(index);
}

void SessionsModel::startNewSession()
{
    if (!canStartNewSession()) {
        return;
    }
    m_backend->startNewSession();
}

void SessionsModel::reload()
{
//    m_backend->reload();
}

void SessionsModel::setShowNewSessionEntry(bool showNewSessionEntry)
{
    if (!canStartNewSession()) {
        return;
    }

    if (showNewSessionEntry == m_showNewSessionEntry) {
        return;
    }

    int row = QSortFilterProxyModel::rowCount();
    if (showNewSessionEntry) {
        beginInsertRows(QModelIndex(), row, row);
        m_showNewSessionEntry = showNewSessionEntry;
        endInsertRows();
    } else {
        beginRemoveRows(QModelIndex(), row, row);
        m_showNewSessionEntry = showNewSessionEntry;
        endRemoveRows();
    }
    emit countChanged();
}


QVariant SessionsModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() > rowCount(QModelIndex())) {
        return QVariant();
    }

    //our extra entry
    if (index.row() == QSortFilterProxyModel::rowCount()) {
        switch (static_cast<Role>(role)) {
        case Role::RealName: return i18n("New Session");
        case Role::IconName: return QStringLiteral("list-add");
        case Role::Name: return i18n("New Session");
        case Role::DisplayNumber: return 0; //NA
        case Role::VtNumber: return -1; //an invalid VtNumber - which we'll use to indicate it's to start a new session
        case Role::Session: return 0; //NA
        case Role::IsTty: return false; //NA
        default: return QVariant();
        }
    }

    return QSortFilterProxyModel::data(index, role);
}

int SessionsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return QSortFilterProxyModel::rowCount() + (m_showNewSessionEntry ? 1 : 0);
}

QHash<int, QByteArray> SessionsModel::roleNames() const
{
    return {
        {static_cast<int>(Role::Name), QByteArrayLiteral("name")},
        {static_cast<int>(Role::RealName), QByteArrayLiteral("realName")},
        {static_cast<int>(Role::Icon), QByteArrayLiteral("icon")},
        {static_cast<int>(Role::IconName), QByteArrayLiteral("iconName")},
        {static_cast<int>(Role::DisplayNumber), QByteArrayLiteral("displayNumber")},
        {static_cast<int>(Role::VtNumber), QByteArrayLiteral("vtNumber")},
        {static_cast<int>(Role::Session), QByteArrayLiteral("session")},
        {static_cast<int>(Role::IsTty), QByteArrayLiteral("isTty")}
    };
}

SessionModelInternal::SessionModelInternal(QObject *parent)
    :QAbstractListModel (parent)
{
}

LogindSessonModel::LogindSessonModel(QObject *parent)
    :SessionModelInternal (parent)
{
    // This is currently blocking, but we can rely on logind and I want to deploy in stages

    auto manager = new OrgFreedesktopLogin1ManagerInterface(QStringLiteral("org.freedesktop.login1"), QStringLiteral("/org/freedesktop/login1"), QDBusConnection::systemBus(), this);
    auto pendingSessions = manager->ListSessions(); //For current seat?
    pendingSessions.waitForFinished();

    for (auto it: pendingSessions.value()) {
        SessionEntry s;
        QScopedPointer<OrgFreedesktopLogin1SessionInterface> login1Session(new OrgFreedesktopLogin1SessionInterface(manager->service(), it.sessionPath.path(), QDBusConnection::systemBus(), this));
        s.name = it.userName;
        s.vtNumber = login1Session->vTNr();

        beginInsertRows(QModelIndex(), rowCount(), rowCount());
        m_sessions.append(s);
        endInsertRows();
    }

//    session->user();
//    session->className();
//    session->state();
//    session->user();
//    session->vTNr();
//    session->display();
}

QVariant LogindSessonModel::data(const QModelIndex &index, int role) const
{
    const auto &item = m_sessions.at(index.row());
    switch (static_cast<SessionsModel::Role>(role)) {
    case SessionsModel::Role::RealName: return item.realName;
    case SessionsModel::Role::Icon: return item.icon;
    case SessionsModel::Role::Name: return item.name;
    case SessionsModel::Role::DisplayNumber: return item.displayNumber;
    case SessionsModel::Role::VtNumber: return item.vtNumber;
    case SessionsModel::Role::Session: return item.session;
    case SessionsModel::Role::IsTty: return item.isTty;
    default: return QVariant();
    }
}

int LogindSessonModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_sessions.count();
}
