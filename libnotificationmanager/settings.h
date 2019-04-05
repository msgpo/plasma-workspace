/*
 * Copyright 2019 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <QObject>

#include <KSharedConfig>

#include <QDateTime>
#include <QScopedPointer>
#include <QString>

#include "notificationmanager_export.h"

namespace NotificationManager
{

/**
 * @short TODO
 *
 * TODO
 *
 * @author Kai Uwe Broulik <kde@privat.broulik.de>
 **/
class NOTIFICATIONMANAGER_EXPORT Settings : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool criticalPopupsInDoNotDisturbMode READ criticalPopupsInDoNotDisturbMode WRITE setCriticalPopupsInDoNotDisturbMode NOTIFY settingsChanged)
    Q_PROPERTY(bool keepCriticalAlwaysOnTop READ keepCriticalAlwaysOnTop WRITE setKeepCriticalAlwaysOnTop NOTIFY settingsChanged)
    Q_PROPERTY(bool lowPriorityPopups READ lowPriorityPopups WRITE setLowPriorityPopups NOTIFY settingsChanged)
    Q_PROPERTY(bool lowPriorityHistory READ lowPriorityHistory WRITE setLowPriorityHistory NOTIFY settingsChanged)

    /**
     * The notification popup position on screen.
     * Near widget means they should be positioned closely to where the plasmoid is located on screen.
     * This is also the default.
     */
    Q_PROPERTY(PopupPosition popupPosition READ popupPosition WRITE setPopupPosition NOTIFY settingsChanged)

    /**
     * The default timeout for notification popups that do not have an explicit timeout set,
     * in milliseconds. Default is 5000ms (5 seconds).
     */
    Q_PROPERTY(int popupTimeout READ popupTimeout WRITE setPopupTimeout RESET resetPopupTimeout NOTIFY settingsChanged)

    Q_PROPERTY(bool jobsInTaskManager READ jobsInTaskManager WRITE setJobsInTaskManager /*RESET resetJobsInTaskManager*/ NOTIFY settingsChanged)
    Q_PROPERTY(bool jobsInNotifications READ jobsInNotifications WRITE setJobsInNotifications /*RESET resetJobsPopup*/ NOTIFY settingsChanged)
    Q_PROPERTY(bool permanentJobPopups READ permanentJobPopups WRITE setPermanentJobPopups /*RESET resetAutoHideJobsPopup*/ NOTIFY settingsChanged)

    Q_PROPERTY(bool badgesInTaskManager READ badgesInTaskManager WRITE setBadgesInTaskManager NOTIFY settingsChanged)

    /**
     * A list of desktop entries of applications that have been seen sending a notification.
     */
    Q_PROPERTY(QStringList knownApplications READ knownApplications NOTIFY knownApplicationsChanged)

    // TODO check how heavy this is
    Q_PROPERTY(QStringList popupBlacklistedApplications READ popupBlacklistedApplications NOTIFY settingsChanged)
    Q_PROPERTY(QStringList popupBlacklistedServices READ popupBlacklistedServices NOTIFY settingsChanged)

    Q_PROPERTY(QStringList doNotDisturbPopupWhitelistedApplications
               READ doNotDisturbPopupWhitelistedApplications
               NOTIFY settingsChanged)
    Q_PROPERTY(QStringList doNotDisturbPopupWhitelistedServices
               READ doNotDisturbPopupWhitelistedServices
               NOTIFY settingsChanged)

    Q_PROPERTY(QStringList historyBlacklistedApplications READ historyBlacklistedApplications NOTIFY settingsChanged)
    Q_PROPERTY(QStringList historyBlacklistedServices READ historyBlacklistedServices NOTIFY settingsChanged)

    Q_PROPERTY(QDateTime notificationsInhibitedUntil
               READ notificationsInhibitedUntil
               WRITE setNotificationsInhibitedUntil
               RESET resetNotificationsInhibitedUntil
               NOTIFY settingsChanged)

    Q_PROPERTY(bool notificationsInhibitedByApplication
               READ notificationsInhibitedByApplication
               NOTIFY notificationsInhibitedByApplicationChanged)

    Q_PROPERTY(QStringList notificationInhibitionApplications
               READ notificationInhibitionApplications
               NOTIFY notificationInhibitionApplicationsChanged)

    Q_PROPERTY(QStringList notificationInhibitionReasons
               READ notificationInhibitionReasons
               NOTIFY notificationInhibitionApplicationsChanged)

    /**
     * Whether notification sounds should be disabled
     *
     * This does not reflect the actual mute state of the Notification Sounds
     * but only remembers what value was assigned to this property.
     *
     * When @c true the notification sound stream is muted.
     * When @c false the notification sound stream is unmuted only if it was
     * previously muted by having set this property @c true. This is to avoid
     * unmuting notification sounds if the user themselves have already muted them.
     */
    Q_PROPERTY(bool notificationSoundsInhibited
               WRITE setNotificationSoundsInhibited
               NOTIFY NotificationSoundsInhibitedChanged)

    /**
     * Whether the notification sound stream is actually muted
     */
    Q_PROPERTY(bool notificationSoundsMuted
               READ notificationSoundsMuted
               WRITE setNotificationSoundsMuted
               NOTIFY notificationSoundsMutedChanged)

    /**
     * Whether to update the properties immediately when they are changed on disk
     *
     * This can be undesirable for a settings dialog where outside changes
     * should not suddenly cause the UI to change.
     *
     * Default is true
     */
    Q_PROPERTY(bool live READ live WRITE setLive NOTIFY liveChanged)

    /**
     * Whether the settings have changed and need to be saved
     *
     * @sa save()
     */
    Q_PROPERTY(bool dirty READ dirty NOTIFY dirtyChanged)

public:
    explicit Settings(QObject *parent = nullptr);
    Settings(const KSharedConfig::Ptr &config, QObject *parent = nullptr);
    ~Settings() override;

    enum PopupPosition {
        NearWidget = 0, // TODO better name? CloseToWidget? AtWidget? AroundWidget?
        TopLeft,
        TopCenter,
        TopRight,
        BottomLeft,
        BottomCenter,
        BottomRight
    };
    Q_ENUM(PopupPosition)

    enum NotificationBehavior {
        ShowPopups = 1 << 1,
        ShowPopupsInDoNotDisturbMode = 1 << 2,
        ShowInHistory = 1 << 3,
        ShowBadges = 1 << 4
    };
    Q_ENUM(NotificationBehavior)
    Q_DECLARE_FLAGS(NotificationBehaviors, NotificationBehavior)
    Q_FLAG(NotificationBehaviors)

    Q_INVOKABLE NotificationBehaviors applicationBehavior(const QString &desktopEntry) const;
    Q_INVOKABLE void setApplicationBehavior(const QString &desktopEntry, NotificationBehaviors behaviors);

    Q_INVOKABLE NotificationBehaviors serviceBehavior(const QString &desktopEntry) const;
    Q_INVOKABLE void setServiceBehavior(const QString &desktopEntry, NotificationBehaviors behaviors);

    Q_INVOKABLE void registerKnownApplication(const QString &desktopEntry);
    Q_INVOKABLE void forgetKnownApplication(const QString &desktopEntry);

    Q_INVOKABLE void load();
    Q_INVOKABLE void save();
    Q_INVOKABLE void defaults();

    bool live() const;
    void setLive(bool live);

    bool dirty() const;

    bool criticalPopupsInDoNotDisturbMode() const;
    void setCriticalPopupsInDoNotDisturbMode(bool enable);

    bool keepCriticalAlwaysOnTop() const;
    void setKeepCriticalAlwaysOnTop(bool enable);

    bool lowPriorityPopups() const;
    void setLowPriorityPopups(bool enable);

    bool lowPriorityHistory() const;
    void setLowPriorityHistory(bool enable);

    PopupPosition popupPosition() const;
    void setPopupPosition(PopupPosition popupPosition);

    int popupTimeout() const;
    void setPopupTimeout(int popupTimeout);
    void resetPopupTimeout();

    bool jobsInTaskManager() const;
    void setJobsInTaskManager(bool enable);

    bool jobsInNotifications() const;
    void setJobsInNotifications(bool enable);

    bool permanentJobPopups() const;
    void setPermanentJobPopups(bool enable);

    bool badgesInTaskManager() const;
    void setBadgesInTaskManager(bool enable);

    QStringList knownApplications() const;

    QStringList popupBlacklistedApplications() const;
    QStringList popupBlacklistedServices() const;

    QStringList doNotDisturbPopupWhitelistedApplications() const;
    QStringList doNotDisturbPopupWhitelistedServices() const;

    QStringList historyBlacklistedApplications() const;
    QStringList historyBlacklistedServices() const;

    QDateTime notificationsInhibitedUntil() const;
    void setNotificationsInhibitedUntil(const QDateTime &time);
    void resetNotificationsInhibitedUntil();

    bool notificationsInhibitedByApplication() const;
    QStringList notificationInhibitionApplications() const;
    QStringList notificationInhibitionReasons() const;

    bool notificationSoundsInhibited() const;
    void setNotificationSoundsInhibited(bool inhibited);

    bool notificationSoundsMuted() const;
    void setNotificationSoundsMuted(bool muted);

    Q_INVOKABLE void revokeApplicationInhibitions();

signals:
    void settingsChanged();

    void liveChanged();
    void dirtyChanged();

    void knownApplicationsChanged();

    void notificationsInhibitedByApplicationChanged(bool notificationsInhibitedByApplication);
    void notificationInhibitionApplicationsChanged();
    void notificationSoundsInhibitedChanged();
    void notificationSoundsMutedChanged();

private:
    class Private;
    QScopedPointer<Private> d;

};

} // namespace NotificationManager

Q_DECLARE_OPERATORS_FOR_FLAGS(NotificationManager::Settings::NotificationBehaviors)