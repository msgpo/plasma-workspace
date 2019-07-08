/*
 *  Copyright 2019 Marco Martin <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.12
import QtQuick.Layouts 1.2

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents
import org.kde.plasma.private.containmentlayoutmanager 1.0 as ContainmentLayoutManager

ContainmentLayoutManager.AppletContainer {
    id: appletContainer
    editModeCondition: plasmoid.immutable
        ? ContainmentLayoutManager.ItemContainer.Manual
        : ContainmentLayoutManager.ItemContainer.AfterPressAndHold

    Layout.minimumWidth: (applet && applet.compactRepresentationItem ? applet.compactRepresentationItem.Layout.minimumWidth : 0) + background.margins.left + background.margins.right
    Layout.minimumHeight: (applet && applet.compactRepresentationItem ? applet.compactRepresentationItem.Layout.minimumHeight : 0) + background.margins.top + background.margins.bottom

    Layout.preferredWidth: (applet ? Math.max(applet.Layout.minimumWidth, applet.Layout.preferredWidth) : 0) + background.margins.left + background.margins.right
    Layout.preferredHeight: (applet ? Math.max(applet.Layout.minimumHeight, applet.Layout.preferredHeight) : 0) + background.margins.top + background.margins.bottom

    Layout.maximumWidth: applet && applet.fullRepresentationItem ? applet.fullRepresentationItem.Layout.maximumWidth + background.margins.left + background.margins.right : Number.POSITIVE_INFINITY
    Layout.maximumHeight: applet && applet.fullRepresentationItem ? applet.fullRepresentationItem.Layout.maximumHeight + background.margins.top + background.margins.bottom : Number.POSITIVE_INFINITY

    leftPadding: background.margins.left
    topPadding: background.margins.top
    rightPadding: background.margins.right
    bottomPadding: background.margins.bottom

    background: PlasmaCore.FrameSvgItem {
        imagePath: contentItem && contentItem.backgroundHints == PlasmaCore.Types.StandardBackground ? "widgets/background" : ""
    }

    busyIndicatorComponent: PlasmaComponents.BusyIndicator {
        anchors.centerIn: parent
        visible: applet.busy
        running: visible
    }
} 
