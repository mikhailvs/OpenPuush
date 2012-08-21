#include "tray_menu.hpp"
#include "ui_tray_menu.h"

tray_menu::tray_menu(QWidget * parent) :
    QMenu(parent),
    ui(new Ui::tray_menu)
{
    ui->setupUi(this);
}

tray_menu::~tray_menu()
{
    delete ui;
}
