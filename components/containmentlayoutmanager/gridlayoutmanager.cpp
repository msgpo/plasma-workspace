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

#include "gridlayoutmanager.h"
#include "appletslayout.h"
#include <cmath>

GridLayoutManager::GridLayoutManager(AppletsLayout *layout)
    : AbstractLayoutManager(layout)
{
}

GridLayoutManager::~GridLayoutManager()
{
}

QString GridLayoutManager::serializeLayout() const
{
    QString result;

    for (auto *item : layout()->childItems()) {
        ItemContainer *itemCont = qobject_cast<ItemContainer*>(item);
        if (itemCont && itemCont != layout()->placeHolder()) {
            result += itemCont->key() + QLatin1Char(':')
                + QString::number(itemCont->x()) + QLatin1Char(',')
                + QString::number(itemCont->y()) + QLatin1Char(',')
                + QString::number(itemCont->width()) + QLatin1Char(',')
                + QString::number(itemCont->height()) + QLatin1Char(',')
                + QString::number(itemCont->rotation()) + QLatin1Char(';');
        }
    }

    return result;
}

void GridLayoutManager::parseLayout(const QString &savedLayout)
{
    m_parsedConfig.clear();
    QStringList itemsConfigs = savedLayout.split(QLatin1Char(';'));

    for (const auto &itemString : itemsConfigs) {
        QStringList itemConfig = itemString.split(QLatin1Char(':'));
        if (itemConfig.count() != 2) {
            continue;
        }

        QString id = itemConfig[0];
        QStringList itemGeom = itemConfig[1].split(QLatin1Char(','));
        if (itemGeom.count() != 5) {
            continue;
        }

        m_parsedConfig[id] = {itemGeom[0].toInt(), itemGeom[1].toInt(), itemGeom[2].toInt(), itemGeom[3].toInt(), itemGeom[4].toInt()};
    }
}

bool GridLayoutManager::itemIsManaged(ItemContainer *item)
{
    return m_pointsForItem.contains(item);
}

void GridLayoutManager::resetLayout()
{
    m_grid.clear();
    m_pointsForItem.clear();
    for (auto *item : layout()->childItems()) {
        ItemContainer *itemCont = qobject_cast<ItemContainer*>(item);
        if (itemCont && itemCont != layout()->placeHolder()) {
            positionItemAndAssign(itemCont);
        }
    }
}

void GridLayoutManager::resetLayoutFromConfig()
{
    m_grid.clear();
    m_pointsForItem.clear();
    QList<ItemContainer *> missingItems;

    for (auto *item : layout()->childItems()) {
        ItemContainer *itemCont = qobject_cast<ItemContainer*>(item);
        if (itemCont && itemCont != layout()->placeHolder()) {
            if (!restoreItem(itemCont)) {
                missingItems << itemCont;
            }
        }
    }

    for (auto *item : missingItems) {
        positionItemAndAssign(item);
    }
}

bool GridLayoutManager::restoreItem(ItemContainer *item)
{
    auto it = m_parsedConfig.find(item->key());

    if (it != m_parsedConfig.end()) {
        item->setX(it.value().x);
        item->setY(it.value().y);
        item->setWidth(it.value().width);
        item->setHeight(it.value().height);
        item->setRotation(it.value().rotation);
        positionItemAndAssign(item);
        return true;
    }

    return false;
}

bool GridLayoutManager::isRectAvailable(const QRectF &rect)
{
    //TODO: define directions in which it can grow
    if (rect.x() < 0 || rect.y() < 0 || rect.x() + rect.width() > layout()->width() || rect.y() + rect.height() > layout()->height()) {
        return false;
    }
    
    const QRect cellItemGeom = cellBasedGeometry(rect);

    for (int row = cellItemGeom.top(); row <= cellItemGeom.bottom(); ++row) {
        for (int column = cellItemGeom.left(); column <= cellItemGeom.right(); ++column) {
            if (!isCellAvailable(QPair<int, int>(row, column))) {
                return false;
            }
        }
    }
    return true;
}

bool GridLayoutManager::assignSpaceImpl(ItemContainer *item)
{
    releaseSpace(item);
    if (!isRectAvailable(itemGeometry(item))) {
        qWarning()<<"Trying to take space not available"<<item;
        return false;
    }

    const QRect cellItemGeom = cellBasedGeometry(itemGeometry(item));

    for (int row = cellItemGeom.top(); row <= cellItemGeom.bottom(); ++row) {
        for (int column = cellItemGeom.left(); column <= cellItemGeom.right(); ++column) {
            QPair<int, int> cell(row, column);
            m_grid.insert(cell, item);
            m_pointsForItem[item].insert(cell);
        }
    }

    // Reorder items tab order
    for (auto *i2 : layout()->childItems()) {
        ItemContainer *item2 = qobject_cast<ItemContainer*>(i2);
        if (item2 && item != item2 && item2 != layout()->placeHolder()
            && item->y() < item2->y() + item2->height()
            && item->x() <= item2->x()) {
            item->stackBefore(item2);
            break;
        }
    }

    return true;
}

void GridLayoutManager::releaseSpaceImpl(ItemContainer *item)
{
    auto it = m_pointsForItem.find(item);

    if (it == m_pointsForItem.end()) {
        return;
    }

    for (const auto &point : it.value()) {
        m_grid.remove(point);
    }

    m_pointsForItem.erase(it);
}

int GridLayoutManager::rows() const
{
    return layout()->height() / cellSize().height();
}

int GridLayoutManager::columns() const
{
    return layout()->width() / cellSize().width();
}

QRect GridLayoutManager::cellBasedGeometry(const QRectF &geom) const
{
    return QRect(
        round(qBound(0.0, geom.x(), layout()->width() - geom.width()) / cellSize().width()),
        round(qBound(0.0, geom.y(), layout()->height() - geom.height()) / cellSize().height()),
        round((qreal)geom.width() / cellSize().width()),
        round((qreal)geom.height() / cellSize().height())
    );
}

QRect GridLayoutManager::cellBasedBoundingGeometry(const QRectF &geom) const
{
    return QRect(
        floor(qBound(0.0, geom.x(), layout()->width() - geom.width()) / cellSize().width()),
        floor(qBound(0.0, geom.y(), layout()->height() - geom.height()) / cellSize().height()),
        ceil((qreal)geom.width() / cellSize().width()),
        ceil((qreal)geom.height() / cellSize().height())
    );
}

bool GridLayoutManager::isOutOfBounds(const QPair<int, int> &cell) const
{
    return cell.first < 0 
        || cell.second < 0
        || cell.first >= rows()
        || cell.second >= columns();
}

bool GridLayoutManager::isCellAvailable(const QPair<int, int> &cell) const
{
    return !isOutOfBounds(cell) && !m_grid.contains(cell);
}

QRectF GridLayoutManager::itemGeometry(QQuickItem *item) const
{
    return QRectF(item->x(), item->y(), item->width(), item->height());
}

QPair<int, int> GridLayoutManager::nextCell(const QPair<int, int> &cell, AppletsLayout::PreferredLayoutDirection direction) const
{
    QPair<int, int> nCell = cell;

    switch (direction) {
    case AppletsLayout::AppletsLayout::BottomToTop:
        --nCell.first;
        break;
    case AppletsLayout::AppletsLayout::TopToBottom:
        ++nCell.first;
        break;
    case AppletsLayout::AppletsLayout::RightToLeft:
        --nCell.second;
        break;
    case AppletsLayout::AppletsLayout::LeftToRight:
    default:
        ++nCell.second;
        break;
    }

    return nCell;
}

QPair<int, int> GridLayoutManager::nextAvailableCell(const QPair<int, int> &cell, AppletsLayout::PreferredLayoutDirection direction) const
{
    QPair<int, int> nCell = cell;
    while (!isOutOfBounds(nCell)) {
        nCell = nextCell(nCell, direction);

        if (isOutOfBounds(nCell)) {
            switch (direction) {
            case AppletsLayout::AppletsLayout::BottomToTop:
                nCell.first = rows() - 1;
                --nCell.second;
                break;
            case AppletsLayout::AppletsLayout::TopToBottom:
                nCell.first = 0;
                ++nCell.second;
                break;
            case AppletsLayout::AppletsLayout::RightToLeft:
                --nCell.first;
                nCell.second = columns() - 1;
                break;
            case AppletsLayout::AppletsLayout::LeftToRight:
            default:
                ++nCell.first;
                nCell.second = 0;
                break;
            }
        }

        if (isCellAvailable(nCell)) {
            return nCell;
        }
    }

    return QPair<int, int>(-1, -1);
}

int GridLayoutManager::freeSpaceInDirection(const QPair<int, int> &cell, AppletsLayout::PreferredLayoutDirection direction) const
{
    QPair<int, int> nCell = cell;

    int avail = 0;

    while (isCellAvailable(nCell)) {
        ++avail;
        nCell = nextCell(nCell, direction);
    }

    return avail;
}

QRectF GridLayoutManager::nextAvailableSpace(ItemContainer *item, const QSizeF &minimumSize, AppletsLayout::PreferredLayoutDirection direction) const
{  
    // The mionimum size in grid units
    const QSize minimumGridSize(
        ceil((qreal)minimumSize.width() / cellSize().width()),
        ceil((qreal)minimumSize.height() / cellSize().height())
    );

    QRect itemCellGeom = cellBasedGeometry(itemGeometry(item));
    itemCellGeom.setWidth(qMax(itemCellGeom.width(), minimumGridSize.width()));
    itemCellGeom.setHeight(qMax(itemCellGeom.height(), minimumGridSize.height()));

    QSize partialSize;

    QPair<int, int> cell(itemCellGeom.y(), itemCellGeom.x());

    while (!isOutOfBounds(cell)) {

        if (!isCellAvailable(cell)) {
            cell = nextAvailableCell(cell, direction);
        }

        if (direction == AppletsLayout::LeftToRight || direction == AppletsLayout::RightToLeft) {
            partialSize = QSize(INT_MAX, 0);

            int currentRow = cell.first;
            for (; currentRow < cell.first + itemCellGeom.height(); ++currentRow) {

                const int freeRow = freeSpaceInDirection(QPair<int, int>(currentRow, cell.second), direction);

                partialSize.setWidth(qMin(partialSize.width(), freeRow));

                if (freeRow > 0) {
                    partialSize.setHeight(partialSize.height() + 1);
                } else if (partialSize.height() < minimumGridSize.height()) {
                    break;
                }
        
                if (partialSize.width() >= itemCellGeom.width()
                    && partialSize.height() >= itemCellGeom.height()) {
                    break;
                } else if (partialSize.width() < minimumGridSize.width()) {
                    break;
                }
            }

            if (partialSize.width() >= minimumGridSize.width()
            && partialSize.height() >= minimumGridSize.height()) {

                const int width = qMin(itemCellGeom.width(), partialSize.width()) * cellSize().width();
                const int height = qMin(itemCellGeom.height(), partialSize.height()) * cellSize().height();

                if  (direction == AppletsLayout::RightToLeft) {
                    return QRectF(cell.second * cellSize().width() - width,
                        cell.first * cellSize().height(),
                        width, height);
                // AppletsLayout::LeftToRight
                } else {
                    return QRectF(cell.second * cellSize().width(),
                        cell.first * cellSize().height(),
                        width, height);
                }
            } else {
                cell.first = currentRow + 1;
            }

        } else if (direction == AppletsLayout::TopToBottom || direction == AppletsLayout::BottomToTop) {
            partialSize = QSize(0, INT_MAX);

            int currentColumn = cell.second;
            for (; currentColumn < cell.second + itemCellGeom.width(); ++currentColumn) {

                const int freeColumn = freeSpaceInDirection(QPair<int, int>(cell.first, currentColumn), direction);

                partialSize.setHeight(qMin(partialSize.height(), freeColumn));

                if (freeColumn > 0) {
                    partialSize.setWidth(partialSize.width() + 1);
                } else if (partialSize.width() < minimumGridSize.width()) {
                    break;
                }
        
                if (partialSize.width() >= itemCellGeom.width()
                    && partialSize.height() >= itemCellGeom.height()) {
                    break;
                } else if (partialSize.height() < minimumGridSize.height()) {
                    break;
                }
            }

            if (partialSize.width() >= minimumGridSize.width()
                && partialSize.height() >= minimumGridSize.height()) {

                const int width = qMin(itemCellGeom.width(), partialSize.width()) * cellSize().width();
                const int height = qMin(itemCellGeom.height(), partialSize.height()) * cellSize().height();

                if (direction == AppletsLayout::BottomToTop) {
                    return QRectF(cell.second * cellSize().width(),
                        cell.first * cellSize().height() - height,
                        width, height);
                // AppletsLayout::TopToBottom:
                } else {
                    return QRectF(cell.second * cellSize().width(),
                        cell.first * cellSize().height(),
                        width, height);
                }
            } else {
                cell.second = currentColumn + 1;
            }
        }
    }

    //We didn't manage to find layout space, return invalid geometry
    return QRectF();
}


#include "moc_gridlayoutmanager.cpp"
