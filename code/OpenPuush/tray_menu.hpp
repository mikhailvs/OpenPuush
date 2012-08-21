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
