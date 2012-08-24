/*   OpenPuush, an open-source clone of Puush
 *   Copyright (C) 2012  Mikhail Slyusarev
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QKeyEvent>
#include <QMouseEvent>
#include <QDesktopWidget>
#include <QPixmap>
#include <QApplication>
#include <QTimer>
#include <QDebug>

#include "screenshot_overlay.hpp"

#if defined(Q_OS_UNIX)
#   include <QxtGui/QxtWindowSystem>
#else
#   include <QxtWidgets/QxtWindowSystem>
#endif

screenshot_overlay::screenshot_overlay(QWidget *parent) :
    QMainWindow(parent)
{
    setCursor(Qt::CrossCursor);
    QDesktopWidget dw;

    int w = 0, h = dw.screenGeometry().height();
    for (int i = 0; i < dw.screenCount(); ++i)
    {
        w += dw.screenGeometry(i).width();
#if defined(Q_WS_X11)
        w += 5;
#endif
    }

    setGeometry(0, 0, w, h);
    setWindowFlags(Qt::FramelessWindowHint |
                   Qt::CustomizeWindowHint |
                   Qt::WindowStaysOnTopHint |
                   Qt::Tool);
    setStyleSheet("background:gray");

    selected_area = new QWidget(this);
    selected_area->setStyleSheet("background:black");
    selected_area->setWindowOpacity(1);
}

void screenshot_overlay::showEvent(QShowEvent * e)
{
    QMainWindow::showEvent(e);
    qDebug() << geometry();
#if defined(Q_WS_X11)
    move(-1000, -1000); // I don't know why I must do this...
#endif
    setWindowOpacity(0.1);
    grabbing_window = false;
    selected_area->setGeometry(0, 0, 0, 0);
}

screenshot_overlay::~screenshot_overlay()
{
    delete selected_area;
}

void screenshot_overlay::keyPressEvent(QKeyEvent * e)
{
    QMainWindow::keyPressEvent(e);
    switch (e->key())
    {
    case Qt::Key_Escape:
        close();
        break;
    case Qt::Key_Space:
        grabbing_window = !grabbing_window;
        if (grabbing_window)
        {
            setCursor(Qt::OpenHandCursor);
        }
        else
        {
            setCursor(Qt::CrossCursor);
        }
    default:
        break;
    }
}

void screenshot_overlay::update_selected()
{
    if (initial_selected != QPoint(-1, -1))
    {
        int x, y, w, h;
        x = qMin(initial_selected.x(), current_selected.x());
        y = qMin(initial_selected.y(), current_selected.y());
        w = abs(initial_selected.x() - current_selected.x());
        h = abs(initial_selected.y() - current_selected.y());

        selected_area->setGeometry(x, y, w, h);
    }
}

void screenshot_overlay::mousePressEvent(QMouseEvent * e)
{
    QMainWindow::mousePressEvent(e);
    if (grabbing_window)
    {
        setCursor(Qt::ClosedHandCursor);
    }
    else
    {
        initial_selected = e->pos();
        current_selected = initial_selected;
        selected_area->show();
    }
}

void screenshot_overlay::mouseReleaseEvent(QMouseEvent * e)
{
    QMainWindow::mouseReleaseEvent(e);
    if (grabbing_window)
    {
        setCursor(Qt::OpenHandCursor);
        selected_area->close();
    }
    else
    {
        current_selected = QPoint(-1, -1);
        initial_selected = QPoint(-1, -1);
    }

    setWindowOpacity(0);
    QTimer::singleShot(100, this, SLOT(get_screenshot()));
    selected_area->hide();
}

void screenshot_overlay::mouseMoveEvent(QMouseEvent * e)
{
    QMainWindow::mouseMoveEvent(e);
    if (grabbing_window)
    {

    }
    else
    {
        current_selected = e->pos();
        update_selected();
    }
}

class NativeWindow : public QWidget {
public:
    NativeWindow(WId wid) {
        QWidget::create(wid, false, false); // window, initializeWindow, destroyOldWindow
    }
    ~NativeWindow() {
        QWidget::destroy(false, false); // destroyWindow, destroySubWindows
    }
};

#if defined(Q_WS_X11)

#include <X11/Xlib.h>

#endif

void screenshot_overlay::get_screenshot()
{
    if (grabbing_window)
    {
    }
    else
    {
        int x, y, w, h;
        QRect frame = selected_area->geometry();
        x = frame.left();
        y = frame.top();
        w = frame.width();
        h = frame.height();

        QPixmap p = QPixmap::grabWindow(QApplication::desktop()->winId(), x, y, w, h);
        emit got_screenshot(p);
    }
    selected_area->setGeometry(0, 0, 0, 0);
}
