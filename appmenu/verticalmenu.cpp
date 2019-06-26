/*
  This file is part of the KDE project.

  Copyright (c) 2011 Lionel Chauvin <megabigbug@yahoo.fr>
  Copyright (c) 2011,2012 Cédric Bellegarde <gnumdk@gmail.com>

  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  DEALINGS IN THE SOFTWARE.
*/

#include "verticalmenu.h"

#include <QCoreApplication>
#include <QKeyEvent>
#include <QEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QDebug>
#include <QGuiApplication>

#include <KWayland/Client/compositor.h>
#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/plasmashell.h>
#include <KWayland/Client/plasmashell.h>
#include <KWayland/Client/registry.h>
#include <KWayland/Client/surface.h>
#include <KWayland/Client/surface.h>

#include <kwindowsystem.h>

using namespace KWayland::Client;

VerticalMenu::VerticalMenu(QWidget* parent) : QMenu(parent)
{
    if (KWindowSystem::isPlatformWayland()) {
        // normally inferred from being a menu, but on wayland, we're a toplevel
        setWindowFlags(Qt::FramelessWindowHint);
        ConnectionThread *connection = ConnectionThread::fromApplication(qApp);
        auto registry = new Registry(this);
        registry->create(connection);
        registry->setup();
        connection->roundtrip();

        const KWayland::Client::Registry::AnnouncedInterface wmInterface = registry->interface(KWayland::Client::Registry::Interface::PlasmaShell);

        if (wmInterface.name != 0) {
            m_plasmaShellInterface = registry->createPlasmaShell(wmInterface.name, wmInterface.version, qApp);
        }
    }
    winId(); //to create a window handle
    windowHandle()->installEventFilter(this);
}

bool VerticalMenu::eventFilter(QObject *watched, QEvent *event)
{
     if (event->type() == QEvent::Expose) {
        auto ee = static_cast<QExposeEvent*>(event);

        if (!KWindowSystem::isPlatformWayland() || ee->region().isNull()) {
            return false;
        }
        if (!m_plasmaShellInterface) {
            qDebug() << "booo1";
            return false;
        }
        if (m_plasmaShellSurface) {
            qDebug() << "booo2";
            return false;
        }
        Surface *s = Surface::fromWindow(windowHandle());
        if (!s) {
            qDebug() << "booo3";
            return false;
        }
        m_plasmaShellSurface = m_plasmaShellInterface->createSurface(s, this);
        m_plasmaShellSurface->setPosition(pos());
        qDebug() << "set position" << pos();
     } else if (event->type() == QEvent::Hide) {
         delete m_plasmaShellSurface.data();
     } else if (event->type() == QEvent::Move) {
        if (m_plasmaShellSurface) {
            QMoveEvent *me = static_cast<QMoveEvent *>(event);
            m_plasmaShellSurface->setPosition(me->pos());
            qDebug() << "move position" << pos();

        }
    }
    return false;
}

VerticalMenu::~VerticalMenu()
{
}

QMenu *VerticalMenu::leafMenu()
{
    QMenu *leaf = this;
    while (true) {
        QAction *act = leaf->activeAction();
        if (act && act->menu() && act->menu()->isVisible()) {
            leaf = act->menu();
            continue;
        }
        return leaf == this ? nullptr : leaf;
    }
    return nullptr; // make gcc happy
}

void VerticalMenu::paintEvent(QPaintEvent *pe)
{
    QMenu::paintEvent(pe);
    if (QWidget::mouseGrabber() == this)
        return;
    if (QWidget::mouseGrabber())
        QWidget::mouseGrabber()->releaseMouse();
    grabMouse();
    grabKeyboard();
}

//DAVE FIXME
// #define FORWARD(_EVENT_, _TYPE_) \
// void VerticalMenu::_EVENT_##Event(Q##_TYPE_##Event *e) \
// { \
//     if (QMenu *leaf = leafMenu()) \
//         QCoreApplication::sendEvent(leaf, e); \
//     else \
//         QMenu::_EVENT_##Event(e); \
// } \
//
// FORWARD(keyPress, Key)
// FORWARD(keyRelease, Key)
