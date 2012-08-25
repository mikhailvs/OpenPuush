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

#ifndef SCREENSHOT_OVERLAY_HPP
#define SCREENSHOT_OVERLAY_HPP

#include <QMainWindow>

class QKeyEvent;
class QMouseEvent;
class QPaintEvent;
class QShowEvent;
class QTimer;

class screenshot_overlay : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit screenshot_overlay(QWidget *parent = 0);
    ~screenshot_overlay();
    
private:
    QWidget * selected_area;
    bool taking_ss;

    QPoint initial_selected;
    QPoint current_selected;

    void update_selected();

private slots:
    void keyPressEvent(QKeyEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void showEvent(QShowEvent *);

    void get_screenshot();

signals:
    void got_screenshot(QPixmap);
};

#endif // SCREENSHOT_OVERLAY_HPP
