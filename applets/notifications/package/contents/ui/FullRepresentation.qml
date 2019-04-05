/*
 * Copyright 2018-2019 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

import QtQuick 2.8
import QtQuick.Layouts 1.1

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.extras 2.0 as PlasmaExtras

import org.kde.kcoreaddons 1.0 as KCoreAddons

import org.kde.notificationmanager 1.0 as NotificationManager

import "global"

ColumnLayout {
    Layout.preferredWidth: units.gridUnit * 18
    Layout.preferredHeight: units.gridUnit * 24
    Layout.fillHeight: plasmoid.formFactor === PlasmaCore.Types.Vertical
    spacing: units.smallSpacing

    // TODO these should be configurable in the future
    readonly property int dndMorningHour: 6
    readonly property int dndEveningHour: 20

    Connections {
        target: plasmoid
        onExpandedChanged: {
            if (plasmoid.expanded) {
                list.positionViewAtBeginning();
            }
        }
    }

    // header
    ColumnLayout {
        Layout.fillWidth: true
        spacing: 0

        RowLayout {
            Layout.fillWidth: true

            RowLayout {
                id: dndRow
                spacing: units.smallSpacing

                PlasmaComponents3.CheckBox {
                    id: dndCheck
                    text: i18n("Do not disturb")
                    spacing: units.smallSpacing
                    checkable: true
                    checked: Globals.inhibited

                    // Let the menu open on press
                    onPressed: {
                        if (!Globals.inhibited) {
                            dndMenu.date = new Date();
                            // shows ontop of CheckBox to hide the fact that it's unchecked
                            // until you actually select something :)
                            dndMenu.open(0, 0);
                        }
                    }
                    // but disable only on click
                    onClicked: {
                        if (Globals.inhibited) {
                            notificationSettings.notificationsInhibitedUntil = undefined;
                            notificationSettings.revokeApplicationInhibitions();

                            notificationSettings.save();
                        }
                    }

                    contentItem: RowLayout {
                        spacing: dndCheck.spacing

                        PlasmaCore.IconItem {
                            Layout.leftMargin: dndCheck.mirrored ? 0 : dndCheck.indicator.width + dndCheck.spacing
                            Layout.rightMargin: dndCheck.mirrored ? dndCheck.indicator.width + dndCheck.spacing : 0
                            source: "notifications-disabled"
                            Layout.preferredWidth: units.iconSizes.smallMedium
                            Layout.preferredHeight: units.iconSizes.smallMedium
                        }

                        PlasmaComponents.Label {
                            text: i18n("Do not disturb")
                        }
                    }

                    PlasmaComponents.ContextMenu {
                        id: dndMenu
                        property date date
                        visualParent: dndCheck
                        onTriggered: {
                            notificationSettings.notificationsInhibitedUntil = item.date;
                            notificationSettings.save();
                        }

                        PlasmaComponents.MenuItem {
                            section: true
                            text: i18n("Do not disturb")
                        }

                        PlasmaComponents.MenuItem {
                            text: i18n("For 1 hour")
                            readonly property date date: {
                                var d = dndMenu.date;
                                d.setHours(d.getHours() + 1);
                                d.setSeconds(0);
                                return d;
                            }
                        }
                        PlasmaComponents.MenuItem {
                            text: i18n("Until this evening")
                            // TODO make the user's preferred time schedule configurable
                            visible: dndMenu.date.getHours() < dndEveningHour
                            readonly property date date: {
                                var d = dndMenu.date;
                                d.setHours(dndEveningHour);
                                d.setMinutes(0);
                                d.setSeconds(0);
                                return d;
                            }
                        }
                        PlasmaComponents.MenuItem {
                            text: i18n("Until tomorrow morning")
                            visible: dndMenu.date.getHours() > dndMorningHour
                            readonly property date date: {
                                var d = dndMenu.date;
                                d.setDate(d.getDate() + 1);
                                d.setHours(dndMorningHour);
                                d.setMinutes(0);
                                d.setSeconds(0);
                                return d;
                            }
                        }
                        PlasmaComponents.MenuItem {
                            text: i18n("Until Monday")
                            // show Friday and Saturday, Sunday is "0" but for that you can use "until tomorrow morning"
                            visible: dndMenu.date.getDay() >= 5
                            readonly property date date: {
                                var d = dndMenu.date;
                                d.setHours(dndMorningHour);
                                // wraps around if neccessary
                                d.setDate(d.getDate() + (7 - d.getDay() + 1));
                                d.setMinutes(0);
                                d.setSeconds(0);
                                return d;
                            }
                        }
                    }
                }
            }

            Item {
                Layout.fillWidth: true
            }

            PlasmaComponents.ToolButton {
                iconName: "configure"
                tooltip: plasmoid.action("openKcm").text
                visible: plasmoid.action("openKcm").enabled
                onClicked: plasmoid.action("openKcm").trigger()
            }
        }

        PlasmaExtras.DescriptiveLabel {
            Layout.leftMargin: dndCheck.mirrored ? 0 : dndCheck.indicator.width + 2 * dndCheck.spacing + units.iconSizes.smallMedium
            Layout.rightMargin: dndCheck.mirrored ? dndCheck.indicator.width + 2 * dndCheck.spacing + units.iconSizes.smallMedium : 0
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            textFormat: Text.PlainText
            text: {
                if (!Globals.inhibited) {
                    return "";
                }

                var inhibitedUntil = notificationSettings.notificationsInhibitedUntil;
                var inhibitedByApp = notificationSettings.notificationsInhibitedByApplication;

                var sections = [];

                if (!isNaN(inhibitedUntil.getTime())) {
                    sections.push(i18nc("Do not disturb until date", "Until %1",
                                        KCoreAddons.Format.formatRelativeDateTime(inhibitedUntil, Locale.ShortFormat)));
                }

                if (inhibitedByApp) {
                    var inhibitionAppNames = notificationSettings.notificationInhibitionApplications;
                    var inhibitionAppReasons = notificationSettings.notificationInhibitionReasons;

                    for (var i = 0, length = inhibitionAppNames.length; i < length; ++i) {
                        var name = inhibitionAppNames[i];
                        var reason = inhibitionAppReasons[i];

                        if (reason) {
                            sections.push(i18nc("Do not disturb until app has finished (reason)", "While %1 is active (%2)", name, reason));
                        } else {
                            sections.push(i18nc("Do not disturb until app has finished", "While %1 is active", name));
                        }
                    }
                }

                return sections.join(" · ");
            }
            visible:  text !== ""
        }
    }

    PlasmaCore.SvgItem {
        elementId: "horizontal-line"
        Layout.fillWidth: true
        // why is this needed here but not in the delegate?
        Layout.preferredHeight: naturalSize.height
        svg: PlasmaCore.Svg {
            id: lineSvg
            imagePath: "widgets/line"
        }
    }

    RowLayout {
        Layout.fillWidth: true

        PlasmaExtras.Heading {
            Layout.fillWidth: true
            level: 3
            opacity: 0.6
            text: list.count === 0 ? i18n("No unread notifications.") : i18n("Notifications")
        }

        PlasmaComponents.ToolButton {
            iconName: "edit-clear-history"
            tooltip: i18n("Clear History")
            visible: plasmoid.action("clearHistory").visible
            onClicked: action_clearHistory()
        }
    }

    // actual notifications
    PlasmaExtras.ScrollArea {
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.preferredWidth: units.gridUnit * 18
        Layout.preferredHeight: units.gridUnit * 24

        ListView {
            id: list
            model: historyModel

            add: Transition {
                SequentialAnimation {
                    PauseAnimation { duration: units.longDuration }
                    ParallelAnimation {
                        NumberAnimation { property: "opacity"; from: 0; duration: units.longDuration }
                        NumberAnimation { property: "height"; from: 0; duration: units.longDuration }
                    }
                }
            }
            addDisplaced: Transition {
                NumberAnimation { properties: "y"; duration:  units.longDuration }
            }

            remove: Transition {
                ParallelAnimation {
                    NumberAnimation { property: "opacity"; to: 0; duration: units.longDuration }
                    NumberAnimation { property: "x"; to: list.width; duration: units.longDuration }
                }
            }
            removeDisplaced: Transition {
                SequentialAnimation {
                    PauseAnimation { duration: units.longDuration }
                    NumberAnimation { properties: "y"; duration:  units.longDuration }
                }
            }

            // This is so the delegates can detect the change in "isInGroup" and show a separator
            section {
                property: "isInGroup"
                criteria: ViewSection.FullString
            }

            delegate: Loader {
                id: delegateLoader
                width: list.width
                sourceComponent: model.isGroup ? groupDelegate : notificationDelegate

                Component {
                    id: groupDelegate
                    NotificationHeader {
                        applicationName: model.applicationName
                        applicationIconSource: model.applicationIconName

                        // don't show timestamp for group

                        configurable: model.configurable
                        closable: model.closable
                        closeButtonTooltip: i18n("Close Group")

                        onCloseClicked: historyModel.close(historyModel.index(index, 0))

                        onConfigureClicked: historyModel.configure(historyModel.index(index, 0))
                    }
                }

                Component {
                    id: notificationDelegate
                    ColumnLayout {
                        spacing: units.smallSpacing

                        RowLayout {
                            Item {
                                id: groupLineContainer
                                Layout.fillHeight: true
                                Layout.topMargin: units.smallSpacing
                                width: units.iconSizes.small
                                visible: model.isInGroup

                                PlasmaCore.SvgItem {
                                    elementId: "vertical-line"
                                    svg: lineSvg
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    width: units.iconSizes.small
                                    height: parent.height
                                }
                            }

                            NotificationItem {
                                Layout.fillWidth: true

                                notificationType: model.type

                                inGroup: model.isInGroup

                                applicationName: model.applicationName
                                applicationIconSource: model.applicationIconName
                                deviceName: model.deviceName || ""

                                time: model.updated || model.created

                                // configure button on every single notifications is bit overwhelming
                                configurable: !inGroup && model.configurable

                                dismissable: model.type === NotificationManager.Notifications.JobType
                                    && model.jobState !== NotificationManager.Notifications.JobStateStopped
                                    && model.dismissed
                                dismissed: model.dismissed || false
                                closable: model.closable

                                summary: model.summary
                                body: model.body || ""
                                icon: model.image || model.iconName

                                urls: model.urls || []

                                jobState: model.jobState || 0
                                percentage: model.percentage || 0
                                error: model.error || 0
                                errorText: model.errorText || ""
                                suspendable: !!model.suspendable
                                killable: !!model.killable
                                jobDetails: model.jobDetails || null

                                configureActionLabel: model.configureActionLabel || ""
                                // In the popup the default action is triggered by clicking on the popup
                                // however in the list this is undesirable, so instead show a clickable button
                                // in case you have a non-expired notification in history (do not disturb mode)
                                // unless it has the same label as an action
                                readonly property bool addDefaultAction: (model.hasDefaultAction
                                                                         && model.defaultActionLabel
                                                                         && (model.actionLabels || []).indexOf(model.defaultActionLabel) === -1) ? true : false
                                actionNames: {
                                    var actions = (model.actionNames || []);
                                    if (addDefaultAction) {
                                        actions.unshift("default"); // prepend
                                    }
                                    return actions;
                                }
                                actionLabels: {
                                    var labels = (model.actionLabels || []);
                                    if (addDefaultAction) {
                                        labels.unshift(model.defaultActionLabel);
                                    }
                                    return labels;
                                }

                                onCloseClicked: historyModel.close(historyModel.index(index, 0))
                                onDismissClicked: model.dismissed = false
                                onConfigureClicked: historyModel.configure(historyModel.index(index, 0))

                                onActionInvoked: {
                                    if (actionName === "default") {
                                        historyModel.invokeDefaultAction(historyModel.index(index, 0));
                                    } else {
                                        historyModel.invokeAction(historyModel.index(index, 0), actionName);
                                    }
                                    // Keep it in the history
                                    historyModel.expire(historyModel.index(index, 0));
                                }
                                onOpenUrl: {
                                    Qt.openUrlExternally(url);
                                    historyModel.expire(historyModel.index(index, 0));
                                }
                                onFileActionInvoked: historyModel.expire(historyModel.index(index, 0))

                                onSuspendJobClicked: historyModel.suspendJob(historyModel.index(index, 0))
                                onResumeJobClicked: historyModel.resumeJob(historyModel.index(index, 0))
                                onKillJobClicked: historyModel.killJob(historyModel.index(index, 0))
                            }
                        }

                        PlasmaComponents.ToolButton {
                            Layout.preferredWidth: minimumWidth
                            iconName: model.isGroupExpanded ? "arrow-up" : "arrow-down"
                            text: model.isGroupExpanded ? i18n("Show Fewer")
                                                        : i18nc("Expand to show n more notifications",
                                                                "Show %1 More", (model.groupChildrenCount - historyModel.groupLimit))
                            visible: model.groupChildrenCount > historyModel.groupLimit && delegateLoader.ListView.nextSection !== delegateLoader.ListView.section
                            onClicked: model.isGroupExpanded = !model.isGroupExpanded
                        }

                        PlasmaCore.SvgItem {
                            Layout.fillWidth: true
                            Layout.bottomMargin: units.smallSpacing
                            elementId: "horizontal-line"
                            svg: lineSvg

                            // property is only atached to the delegate itself (the Loader in our case)
                            visible: (!model.isInGroup || delegateLoader.ListView.nextSection !== delegateLoader.ListView.section)
                                             && delegateLoader.ListView.nextSection !== "" // don't show after last item
                        }
                    }
                }
            }
        }
    }
}