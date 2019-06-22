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

#include "appletslayout.h"
#include "appletcontainer.h"
#include "gridlayoutmanager.h"

#include <QQmlEngine>
#include <QQmlContext>
#include <QTimer>
#include <QStyleHints>

// Plasma
#include <Containment>
#include <PlasmaQuick/AppletQuickItem>

AppletsLayout::AppletsLayout(QQuickItem *parent)
    : QQuickItem(parent)
{
    m_layoutManager = new GridLayoutManager(this);

    setFlags(QQuickItem::ItemIsFocusScope);
    setAcceptedMouseButtons(Qt::LeftButton);

    m_saveLayoutTimer = new QTimer(this);
    m_saveLayoutTimer->setSingleShot(true);
    m_saveLayoutTimer->setInterval(100);
    connect(m_layoutManager, &AbstractLayoutManager::layoutNeedsSaving, m_saveLayoutTimer, QOverload<>::of(&QTimer::start));
    connect(m_saveLayoutTimer, &QTimer::timeout, this, [this] () {
        if (!m_configKey.isEmpty()) {
            const QString serializedConfig = m_layoutManager->serializeLayout();
            m_containment->config().writeEntry(m_configKey, serializedConfig);
            //FIXME: something more efficient
            m_layoutManager->parseLayout(serializedConfig);
            m_savedSize = size();
        }
    });

    m_pressAndHoldTimer = new QTimer(this);
    m_pressAndHoldTimer->setSingleShot(true);
    connect(m_pressAndHoldTimer, &QTimer::timeout, this, [this]() {
        setEditMode(true);
    });
    connect(this, &QQuickItem::focusChanged, this, [this]() {
        if (!hasFocus()) {
            setEditMode(false);
        }
    });
}

AppletsLayout::~AppletsLayout()
{
}

PlasmaQuick::AppletQuickItem *AppletsLayout::containment() const
{
    return m_containmentItem;
}

void AppletsLayout::setContainment(PlasmaQuick::AppletQuickItem *containmentItem)
{
    // Forbid changing containmentItem at runtime
    if (m_containmentItem
        || containmentItem == m_containmentItem
        || !containmentItem->applet()
        || !containmentItem->applet()->isContainment()) {
        qWarning() << "Error: cannot change the containment to AppletsLayout";
        return;
    }

    // Can't assign containments that aren't parents
    QQuickItem *candidate = parentItem();
    while (candidate) {
        if (candidate == m_containmentItem) {
            break;
        }
        candidate = candidate->parentItem();
    }
    if (candidate != m_containmentItem) {
        return;
    }

    m_containmentItem = containmentItem;
    m_containment = static_cast<Plasma::Containment *>(m_containmentItem->applet());

    connect(m_containmentItem, SIGNAL(appletAdded(QObject *, int, int)), 
            this, SLOT(appletAdded(QObject *, int, int)));

    connect(m_containmentItem, SIGNAL(appletRemoved(QObject *)), 
            this, SLOT(appletRemoved(QObject *)));

    emit containmentChanged();
}

QString AppletsLayout::configKey() const
{
    return m_configKey;
}

void AppletsLayout::setConfigKey(const QString &key)
{
    if (m_configKey == key) {
        return;
    }

    m_configKey = key;

    if (!m_configKey.isEmpty() && m_containment) {
        m_layoutManager->parseLayout(m_containment->config().readEntry(m_configKey, ""));

        if (width() > 0 && height() > 0) {
            m_layoutManager->resetLayoutFromConfig();
        }
    }

    emit configKeyChanged();
}

QJSValue AppletsLayout::acceptsAppletCallback() const
{
    return m_acceptsAppletCallback;
}

qreal AppletsLayout::minimumItemWidth() const
{
    return m_minimumItemSize.width();
}

void AppletsLayout::setMinimumItemWidth(qreal width)
{
    if (qFuzzyCompare(width, m_minimumItemSize.width())) {
        return;
    }

    m_minimumItemSize.setWidth(width);

    emit minimumItemWidthChanged();
}

qreal AppletsLayout::minimumItemHeight() const
{
    return m_minimumItemSize.height();
}

void AppletsLayout::setMinimumItemHeight(qreal height)
{
    if (qFuzzyCompare(height, m_minimumItemSize.height())) {
        return;
    }

    m_minimumItemSize.setHeight(height);

    emit minimumItemHeightChanged();
}

qreal AppletsLayout::defaultItemWidth() const
{
    return m_defaultItemSize.width();
}

void AppletsLayout::setDefaultItemWidth(qreal width)
{
    if (qFuzzyCompare(width, m_defaultItemSize.width())) {
        return;
    }

    m_defaultItemSize.setWidth(width);

    emit defaultItemWidthChanged();
}

qreal AppletsLayout::defaultItemHeight() const
{
    return m_defaultItemSize.height();
}

void AppletsLayout::setDefaultItemHeight(qreal height)
{
    if (qFuzzyCompare(height, m_defaultItemSize.height())) {
        return;
    }

    m_defaultItemSize.setHeight(height);

    emit defaultItemHeightChanged();
}

qreal AppletsLayout::cellWidth() const
{
    return m_layoutManager->cellSize().width();
}

void AppletsLayout::setCellWidth(qreal width)
{
    if (qFuzzyCompare(width, m_layoutManager->cellSize().width())) {
        return;
    }

    m_layoutManager->setCellSize(QSizeF(width, m_layoutManager->cellSize().height()));

    emit cellWidthChanged();
}

qreal AppletsLayout::cellHeight() const
{
    return m_layoutManager->cellSize().height();
}

void AppletsLayout::setCellHeight(qreal height)
{
    if (qFuzzyCompare(height, m_layoutManager->cellSize().height())) {
        return;
    }

    m_layoutManager->setCellSize(QSizeF(m_layoutManager->cellSize().width(), height));

    emit cellHeightChanged();
}

void AppletsLayout::setAcceptsAppletCallback(const QJSValue& callback)
{
    if (m_acceptsAppletCallback.strictlyEquals(callback)) {
        return;
    }

    if (!callback.isNull() && !callback.isCallable()) {
        return;
    }

    m_acceptsAppletCallback = callback;

    Q_EMIT acceptsAppletCallbackChanged();
}

QQmlComponent *AppletsLayout::appletContainerComponent() const
{
    return m_appletContainerComponent;
}

void AppletsLayout::setAppletContainerComponent(QQmlComponent *component)
{
    if (m_appletContainerComponent == component) {
        return;
    }

    m_appletContainerComponent = component;

    emit appletContainerComponentChanged();
}

bool AppletsLayout::editMode() const
{
    return m_editMode;
}

void AppletsLayout::setEditMode(bool editMode)
{
    if (m_editMode == editMode) {
        return;
    }

    m_editMode = editMode;

    emit editModeChanged();
}

ItemContainer *AppletsLayout::placeHolder() const
{
    return m_placeHolder;
}

void AppletsLayout::setPlaceHolder(ItemContainer *placeHolder)
{
    if (m_placeHolder == placeHolder) {
        return;
    }

    m_placeHolder = placeHolder;
    m_placeHolder->setParentItem(this);
    m_placeHolder->setZ(9999);
    m_placeHolder->setOpacity(false);

    emit placeHolderChanged();
}

void AppletsLayout::save()
{
    m_saveLayoutTimer->start();
}

void AppletsLayout::showPlaceHolderAt(const QRectF &geom)
{
    if (!m_placeHolder) {
        return;
    }

    m_placeHolder->setX(geom.x());
    m_placeHolder->setY(geom.y());
    m_placeHolder->setWidth(geom.width());
    m_placeHolder->setHeight(geom.height());

    m_layoutManager->positionItem(m_placeHolder);

    m_placeHolder->setProperty("opacity", 1);
}

void AppletsLayout::showPlaceHolderForItem(QQuickItem *item)
{
    if (!m_placeHolder) {
        return;
    }

    m_placeHolder->setX(item->x());
    m_placeHolder->setY(item->y());
    m_placeHolder->setWidth(item->width());
    m_placeHolder->setHeight(item->height());

    m_layoutManager->positionItem(m_placeHolder);

    m_placeHolder->setProperty("opacity", 1);
}

void AppletsLayout::hidePlaceHolder()
{
    if (!m_placeHolder) {
        return;
    }

    m_placeHolder->setProperty("opacity", 0);
}

bool AppletsLayout::isRectAvailable(qreal x, qreal y, qreal width, qreal height)
{
    return m_layoutManager->isRectAvailable(QRectF(x, y, width, height));
}

bool AppletsLayout::itemIsManaged(ItemContainer *item)
{
    return m_layoutManager->itemIsManaged(item);
}

void AppletsLayout::positionItem(ItemContainer *item)
{
    item->setParent(this);
    m_layoutManager->positionItemAndAssign(item);
}

void AppletsLayout::releaseSpace(ItemContainer *item)
{
    m_layoutManager->releaseSpace(item);
}

void AppletsLayout::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    if (newGeometry.size() == oldGeometry.size()) {
        QQuickItem::geometryChanged(newGeometry, oldGeometry);
        return;
    }

    if (oldGeometry.isEmpty() && !newGeometry.isEmpty()) {
        m_layoutManager->resetLayoutFromConfig();
    } else if (!oldGeometry.isEmpty() && !newGeometry.isEmpty()) {
        if (newGeometry.size() == m_savedSize) {
            m_layoutManager->resetLayoutFromConfig();
        } else {
            m_layoutManager->layoutGeometryChanged(newGeometry, oldGeometry);
            polish();
        }
    }

    QQuickItem::geometryChanged(newGeometry, oldGeometry);
}

void AppletsLayout::updatePolish()
{
    m_layoutManager->resetLayout();
}

void AppletsLayout::componentComplete()
{
    if (!m_containment || !m_containmentItem) {
        return;
    }

    if (!m_configKey.isEmpty()) {
        m_layoutManager->parseLayout(m_containment->config().readEntry(m_configKey, ""));
    }

    QList <QObject*> appletObjects = m_containmentItem->property("applets").value<QList <QObject*> >();

    for (auto *obj : appletObjects) {
        PlasmaQuick::AppletQuickItem *appletItem = qobject_cast<PlasmaQuick::AppletQuickItem *>(obj);

        if (!obj) {
            continue;
        }

        AppletContainer *container = createContainerForApplet(appletItem);
        if (width() > 0 && height() > 0) {
            m_layoutManager->positionItemAndAssign(container);
        }
    }

    //layout all extra non applet items
    if (width() > 0 && height() > 0) {
        for (auto *child : childItems()) {
            ItemContainer *item = qobject_cast<ItemContainer *>(child);
            if (item && item != m_placeHolder && !m_layoutManager->itemIsManaged(item)) {
                m_layoutManager->positionItemAndAssign(item);
            }
        }
    }

    QQuickItem::componentComplete();
}



void AppletsLayout::mousePressEvent(QMouseEvent *event)
{
    forceActiveFocus(Qt::MouseFocusReason);

    if (!m_editMode && m_editModeCondition == AppletsLayout::Manual) {
        return;
    }

    if (!m_editMode && m_editModeCondition == AppletsLayout::AfterPressAndHold) {
        m_pressAndHoldTimer->start(QGuiApplication::styleHints()->mousePressAndHoldInterval());
    }

    m_mouseDownWasEditMode = m_editMode;
    m_mouseDownPosition = event->windowPos();

    //event->setAccepted(false);
}

void AppletsLayout::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_editMode && m_editModeCondition == AppletsLayout::Manual) {
        return;
    }

    if (!m_editMode
        && QPointF(event->windowPos() - m_mouseDownPosition).manhattanLength() >= QGuiApplication::styleHints()->startDragDistance()) {
        m_pressAndHoldTimer->stop();
    }
}

void AppletsLayout::mouseReleaseEvent(QMouseEvent *event)
{
    if (!m_editMode && m_editModeCondition == AppletsLayout::Manual) {
        return;
    }

    if (m_editMode
        && m_mouseDownWasEditMode
        && QPointF(event->windowPos() - m_mouseDownPosition).manhattanLength() < QGuiApplication::styleHints()->startDragDistance()) {
        setEditMode(false);
    }

    m_pressAndHoldTimer->stop();

    if (!m_editMode) {
        for (auto *child : childItems()) {
            ItemContainer *item = qobject_cast<ItemContainer *>(child);
            if (item && item != m_placeHolder) {
                item->setEditMode(false);
            }
        }
    }
}

void AppletsLayout::appletAdded(QObject *applet, int x, int y)
{
    PlasmaQuick::AppletQuickItem *appletItem = qobject_cast<PlasmaQuick::AppletQuickItem *>(applet);

    //maybe even an assert?
    if (!appletItem) {
        return;
    }

    if (m_acceptsAppletCallback.isCallable()) {
        QQmlEngine *engine = QQmlEngine::contextForObject(this)->engine();
        Q_ASSERT(engine);
        QJSValueList args;
        args << engine->newQObject(applet) << QJSValue(x) << QJSValue(y);

        if (!m_acceptsAppletCallback.call(args).toBool()) {
            emit appletRefused(applet, x, y);
            return;
        }
    }

    AppletContainer *container = createContainerForApplet(appletItem);
    container->setX(x);
    container->setY(y);
    container->setVisible(true);

    m_layoutManager->positionItemAndAssign(container);
}

void AppletsLayout::appletRemoved(QObject *applet)
{
    PlasmaQuick::AppletQuickItem *appletItem = qobject_cast<PlasmaQuick::AppletQuickItem *>(applet);

    //maybe even an assert?
    if (!appletItem) {
        return;
    }

    AppletContainer *container = m_containerForApplet.value(appletItem);
    if (!container) {
        return;
    }

    m_layoutManager->releaseSpace(container);
    m_containerForApplet.remove(appletItem);
    appletItem->setParentItem(this);
    container->deleteLater();
}

AppletContainer *AppletsLayout::createContainerForApplet(PlasmaQuick::AppletQuickItem *appletItem)
{
    AppletContainer *container = m_containerForApplet.value(appletItem);
qWarning()<<appletItem<<container;
    if (container) {
        return container;
    }

    bool createdFromQml = true;

    if (m_appletContainerComponent) {
        QQmlContext *context = QQmlEngine::contextForObject(this);
        Q_ASSERT(context);
        QObject *instance = m_appletContainerComponent->beginCreate(context);
        container = qobject_cast<AppletContainer *>(instance);
        if (container) {
            container->setParentItem(this);
        } else {
            qWarning() << "Error: provided component not an AppletContainer instance";
            if (instance) {
                instance->deleteLater();
            }
            createdFromQml = false;
        }
    }

    if (!container) {
        container = new AppletContainer(this);
    }

    m_containerForApplet[appletItem] = container;
    container->setLayout(this);
    container->setKey(QLatin1String("Applet-") + QString::number(appletItem->applet()->id()));
    container->setX(appletItem->x() - container->leftPadding());
    container->setY(appletItem->y() - container->topPadding());
    if (appletItem->width() > 0 && appletItem->height() > 0) {
        container->setWidth(qMax(m_minimumItemSize.width(), appletItem->width() + container->leftPadding() + container->rightPadding()));
        container->setHeight(qMax(m_minimumItemSize.height(), appletItem->height() + container->topPadding() + container->bottomPadding()));
    } else {
        container->setWidth(qMax(m_minimumItemSize.width(), m_defaultItemSize.width()));
        container->setHeight(qMax(m_minimumItemSize.height(), m_defaultItemSize.height()));
    }
    container->setContentItem(appletItem);
    container->setVisible(true);
    appletItem->setVisible(true);

    if (m_appletContainerComponent && createdFromQml) {
        m_appletContainerComponent->completeCreate();
    }

    return container;
}

#include "moc_appletslayout.cpp"
