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

#pragma once

#include <QSortFilterProxyModel>
#include <QAbstractListModel>

#include "kworkspace_export.h"

class OrgFreedesktopLogin1SessionInterface;

struct SessionEntry {
    QString realName;
    QString icon;
    QString name;
    QString displayNumber;
    QString session;
    int vtNumber;
    bool isTty;
};

class SessionModelInternal;

class KWORKSPACE_EXPORT SessionsModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(bool canSwitchUser READ canSwitchUser CONSTANT)
    Q_PROPERTY(bool canStartNewSession READ canStartNewSession CONSTANT)
    Q_PROPERTY(bool showNewSessionEntry MEMBER m_showNewSessionEntry WRITE setShowNewSessionEntry NOTIFY showNewSessionEntryChanged)
    Q_PROPERTY(bool includeUnusedSessions READ includeUnusedSessions WRITE setIncludeUnusedSessions NOTIFY includeUnusedSessionsChanged)
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    explicit SessionsModel(QObject *parent = nullptr);
    ~SessionsModel() override = default;

    enum class Role {
        RealName = Qt::DisplayRole,
        Icon = Qt::DecorationRole, //path to a file
        Name = Qt::UserRole + 1,
        DisplayNumber,
        VtNumber,
        Session,
        IsTty,
        IconName //name of an icon
    };

    bool canSwitchUser() const;
    bool canStartNewSession() const;
    bool includeUnusedSessions() const;

    void setShowNewSessionEntry(bool showNewSessionEntry);
    void setIncludeUnusedSessions(bool includeUnusedSessions);

    Q_INVOKABLE void reload();
    Q_INVOKABLE void switchUser(int vt);
    Q_INVOKABLE void startNewSession();

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

signals:
    void showNewSessionEntryChanged();
    void countChanged();
    void includeUnusedSessionsChanged();

    void switchedUser(int vt);
    void startedNewSession();

private:
    QVector<SessionEntry> m_data;
    SessionModelInternal *m_backend;

    bool m_showNewSessionEntry = false;
    bool m_includeUnusedSessions = true;
};

class SessionModelInternal : public QAbstractListModel
{
public:
    virtual void switchUser(int index) {};
    virtual void startNewSession() {};

protected:
    SessionModelInternal(QObject *parent = nullptr);
    ~SessionModelInternal() override = default;
};

class LogindSessonModel : public SessionModelInternal {
public:
    LogindSessonModel(QObject *parent);
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    // SessionModelInternal interface
//    void switchUser(int index) override;
//    void startNewSession() override;
private:
    QVector<SessionEntry> m_sessions;
};
