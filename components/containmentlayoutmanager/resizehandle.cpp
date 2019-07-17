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

#include "resizehandle.h"

#include <QCursor>
#include <cmath>

ResizeHandle::ResizeHandle(QQuickItem *parent)
    : QQuickItem(parent)
{
    setAcceptedMouseButtons(Qt::LeftButton);

    QQuickItem *candidate = parent;
    while (candidate) {
        ConfigOverlay *overlay = qobject_cast<ConfigOverlay *>(candidate);
        if (overlay) {
            setConfigOverlay(overlay);
            break;
        }

        candidate = candidate->parentItem();
    }

    connect(this, &QQuickItem::parentChanged, this, [this]() {
        QQuickItem *candidate = parentItem();
        while (candidate) {
            ConfigOverlay *overlay = qobject_cast<ConfigOverlay *>(candidate);
            if (overlay) {
                setConfigOverlay(overlay);
                break;
            }

            candidate = candidate->parentItem();
        }
    });

    auto syncCursor = [this] () {
        switch (m_resizeCorner) {
        case Left:
        case Right:
            setCursor(QCursor(Qt::SizeHorCursor));
            break;
        case Top:
        case Bottom:
            setCursor(QCursor(Qt::SizeVerCursor));
            break;
        case TopLeft:
        case BottomRight:
            setCursor(QCursor(Qt::SizeFDiagCursor));
            break;
        case TopRight:
        case BottomLeft:
        default:
            setCursor(Qt::SizeBDiagCursor);
        }
    };

    syncCursor();
    connect(this, &ResizeHandle::resizeCornerChanged, this, syncCursor);
}

ResizeHandle::~ResizeHandle()
{
}

bool ResizeHandle::resizeBlocked() const
{
    return !m_shouldResizeWidth || !m_shouldResizeHeight;
}

bool ResizeHandle::resizeLeft() const
{
    return m_resizeCorner == Left || m_resizeCorner == TopLeft || m_resizeCorner == BottomLeft;
}

bool ResizeHandle::resizeTop() const
{
    return m_resizeCorner == Top || m_resizeCorner == TopLeft || m_resizeCorner == TopRight;
}

bool ResizeHandle::resizeRight() const
{
    return m_resizeCorner == Right || m_resizeCorner == TopRight ||m_resizeCorner == BottomRight;
}

bool ResizeHandle::resizeBottom() const
{
    return m_resizeCorner == Bottom || m_resizeCorner == BottomLeft || m_resizeCorner == BottomRight;
}



void ResizeHandle::mousePressEvent(QMouseEvent *event)
{
    m_lastMousePosition = event->windowPos();
    m_shouldResizeWidth = true;
    m_shouldResizeHeight = true;
    event->accept();
}

void ResizeHandle::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_configOverlay || !m_configOverlay->itemContainer()) {
        return;
    }

    ItemContainer *itemContainer = m_configOverlay->itemContainer();
    AppletsLayout *layout = itemContainer->layout();

    if (!layout) {
        return;
    }

    layout->releaseSpace(itemContainer);
    const QPointF difference = m_lastMousePosition - event->windowPos();
    
    QSizeF minimumSize = QSize(layout->minimumItemWidth(), layout->minimumItemHeight());
    if (itemContainer->layoutAttached()) {
        minimumSize.setWidth(qMax(minimumSize.width(), itemContainer->layoutAttached()->property("minimumWidth").toReal()));
        minimumSize.setHeight(qMax(minimumSize.height(), itemContainer->layoutAttached()->property("minimumHeight").toReal()));
    }

    //Now make minimumSize an integer number of cells
    minimumSize.setWidth(ceil(minimumSize.width() / layout->cellWidth()) * layout->cellWidth());
    minimumSize.setHeight(ceil(minimumSize.height() / layout->cellWidth()) * layout->cellHeight());

    if (m_shouldResizeWidth) {
        if (resizeLeft()) {
            const qreal x = itemContainer->x() - difference.x();
            const qreal width = itemContainer->width() + difference.x();
            // -1 to have a bit of margins around
            if (layout->isRectAvailable(x - 1, itemContainer->y(), width, itemContainer->height()) && width >= minimumSize.width()) {
                itemContainer->setX(x);
                itemContainer->setWidth(qMax(width, minimumSize.width()));

            } else {
                m_shouldResizeWidth = false;
                emit resizeBlockedChanged();
            }
        } else if (resizeRight()) {
            const qreal width = itemContainer->width() - difference.x();
            if (layout->isRectAvailable(itemContainer->x(), itemContainer->y(), width, itemContainer->height()) && width >= minimumSize.width()) {
                itemContainer->setWidth(qMax(width, minimumSize.width()));
            } else {
                m_shouldResizeWidth = false;
                emit resizeBlockedChanged();
            }
        }
    } else {
        m_shouldResizeWidth = event->pos().x() >= width()/3 && event->pos().x() <= width()/3 * 2;
        emit resizeBlockedChanged();
    }

    if (m_shouldResizeHeight) {
        if (resizeTop()) {
            const qreal y = itemContainer->y() - difference.y();
            const qreal height = itemContainer->height() + difference.y();
            // -1 to have a bit of margins around
            if (layout->isRectAvailable(itemContainer->x(), y - 1, itemContainer->width(), itemContainer->height()) &&height >= minimumSize.height()) {
                itemContainer->setY(y);
                itemContainer->setHeight(qMax(height, minimumSize.height()));
            } else {
                m_shouldResizeHeight = false;
                emit resizeBlockedChanged();
            }
        } else if (resizeBottom()) {
            const qreal height = itemContainer->height() - difference.y();
            if (layout->isRectAvailable(itemContainer->x(), itemContainer->y(), itemContainer->width(), height) && height >= minimumSize.height()) {
                itemContainer->setHeight(qMax(height, minimumSize.height()));
            } else {
                m_shouldResizeHeight = false;
                emit resizeBlockedChanged();
            }
        }

    } else {
        m_shouldResizeHeight = event->pos().y() >= height()/3 && event->pos().y() <= height()/3 * 2;
        emit resizeBlockedChanged();
    }

    m_lastMousePosition = event->windowPos();
    event->accept();
}

void ResizeHandle::mouseReleaseEvent(QMouseEvent *event)
{
    if (!m_configOverlay || !m_configOverlay->itemContainer()) {
        return;
    }

    ItemContainer *itemContainer = m_configOverlay->itemContainer();
    AppletsLayout *layout = itemContainer->layout();

    if (!layout) {
        return;
    }

    layout->positionItem(itemContainer);

    event->accept();

    m_shouldResizeWidth = true;
    m_shouldResizeHeight = true;
    emit resizeBlockedChanged();
}

void ResizeHandle::setConfigOverlay(ConfigOverlay *handle)
{
    if (handle == m_configOverlay) {
        return;
    }

    m_configOverlay = handle;    
}

#include "moc_resizehandle.cpp"
