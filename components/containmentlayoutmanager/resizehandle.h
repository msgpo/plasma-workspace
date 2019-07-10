/*
 *   Copyright 2019 by Marco Martin <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
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
 *
 */

#pragma once

#include <QQuickItem>

#include "configoverlay.h"

class ResizeHandle: public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(Corner resizeCorner MEMBER m_resizeCorner NOTIFY resizeCornerChanged)

public:
    enum Corner {
        Left = 0,
        TopLeft,
        Top,
        TopRight,
        Right,
        BottomRight,
        Bottom,
        BottomLeft        
    };
    Q_ENUMS(Corner)

    ResizeHandle(QQuickItem *parent = nullptr);
    ~ResizeHandle();


protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

Q_SIGNALS:
    void resizeCornerChanged();

private:
    void setConfigOverlay(ConfigOverlay *configOverlay);

    inline bool resizeLeft() const;
    inline bool resizeTop() const;
    inline bool resizeRight() const;
    inline bool resizeBottom() const;


    QPointF m_lastMousePosition;
    QPointer<ConfigOverlay> m_configOverlay;
    Corner m_resizeCorner = Left;
    bool m_shouldResizeWidth = false;
    bool m_shouldResizeHeight = false;
};

