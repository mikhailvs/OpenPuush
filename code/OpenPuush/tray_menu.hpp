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

#ifndef TRAY_MENU_HPP
#define TRAY_MENU_HPP

#include <QMenu>

namespace Ui {
    class tray_menu;
}

class tray_menu : public QMenu
{
    Q_OBJECT
    
public:
    explicit tray_menu(QWidget * parent = 0);
    ~tray_menu();
    
private:
    Ui::tray_menu * ui;
};

#endif // TRAY_MENU_HPP
