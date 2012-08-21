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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QMap>

class QPaintEvent;
class QPushButton;
class QClipboard;
class QxtGlobalShortcut;
class QShowEvent;
class QFileDialog;
class QTimer;

class dropbox;
class configwindow;
class screenshot_overlay;
class tray_menu;

class openpuush : public QObject
{
    Q_OBJECT
    
public:
    explicit openpuush(QObject *parent = 0);
    ~openpuush();
    
private slots:
    void show_ss_overlay();
    void got_screenshot(QPixmap);
    void fullscreen_ss();
    void upload_file(QString);
    void upload_current_window();
    void upload_clipboard();
    void toggle_functionality();

    void db_authorized();
    void db_upload_progress(qint64, qint64);
    void db_upload_finished();
    void db_upload_error();
    void db_link_ready(QString);
    void db_info_received(QMap<QString, QString>);
    void db_login();
    void db_logout();

    void update_db_info();

    void tray_icon_activated(QSystemTrayIcon::ActivationReason);
    void tray_icon_normal();

    void set_shortcuts_enabled(bool, bool);

    void follow_link();
    void error_occurred(QString);

    void load_start_on_boot();

private:
    const QString base_path;

    dropbox            * db;
    configwindow       * conf_win;
    screenshot_overlay * ss_overlay;
    tray_menu          * menu;

    QSystemTrayIcon * tray_icon;
    QFileDialog     * file_dialog;
    QTimer          * info_update_timer;

    QxtGlobalShortcut * fullscreen_shortcut;
    QxtGlobalShortcut * current_window_shortcut;
    QxtGlobalShortcut * ss_overlay_shortcut;
    QxtGlobalShortcut * file_shortcut;
    QxtGlobalShortcut * upload_clipboard_shortcut;
    QxtGlobalShortcut * toggle_functionality_shortcut;

    QString link;
    bool dropbox_authenticated;
    bool shortcuts_enabled;
    bool follow_tray_icon_link;

    void init_tray_icon_context_menu();
    void init_shortcuts();
    void init_tray_icon();
    void init_dropbox();
    void init_file_dialog();
    void init_ss_overlay();
    void init_conf_win();

    void update_shortcuts();
};

#endif // MAINWINDOW_H
