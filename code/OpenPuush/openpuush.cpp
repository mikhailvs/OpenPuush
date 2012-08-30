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

#include <QPainter>
#include <QPaintEvent>
#include <QPushButton>
#include <QDebug>
#include <QDataStream>
#include <QBuffer>
#include <QDateTime>
#include <QClipboard>
#include <QApplication>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QTimer>
#include <QAction>
#include <QDir>
#include <QTemporaryFile>
#include <QUrl>
#include <QDesktopServices>
#include <QMenu>
#include <QVector>

#include <cstdlib>

#if defined(Q_OS_LINUX)
#   include <QxtGlobalShortcut>
#   include <QxtGui/QxtWindowSystem>
#else
#   include <QxtWidgets/QxtGlobalShortcut>
#   include <QxtWidgets/QxtWindowSystem>
#endif

#include "dropbox.hpp"
#include "openpuush.hpp"
#include "screenshot_overlay.hpp"
#include "config.hpp"
#include "configwindow.hpp"

#define set_shortcut(s,e) s->setShortcut(QKeySequence(config::load(config::e).toString()))

static const QString NOT_AUTHORIZED_MESSAGE = "Your action could not be completed because\n"
                                              "OpenPuush is not linked with a Dropbox account.";

openpuush::openpuush(QObject *parent) :
    QObject(parent),
    dropbox_authenticated(false),
    shortcuts_enabled(true),
    follow_tray_icon_link(false)
{
    QApplication::setQuitOnLastWindowClosed(false);

    if (config::first_run())
    {
        config::defaults();
    }

    load_start_on_boot();

    // order is important here!
    init_dropbox();
    init_ss_overlay();
    init_file_dialog();
    init_shortcuts();
    init_conf_win();
    init_tray_icon();
    init_tray_icon_context_menu();
}

openpuush::~openpuush()
{
    delete db;
    delete conf_win;
    delete ss_overlay;
    delete menu;
    delete tray_icon;
    delete file_dialog;
    delete info_update_timer;
    delete fullscreen_shortcut;
    delete current_window_shortcut;
    delete ss_overlay_shortcut;
    delete file_shortcut;
    delete upload_clipboard_shortcut;
    delete toggle_functionality_shortcut;
}

void openpuush::init_conf_win()
{
    conf_win = new configwindow();
    connect(conf_win, SIGNAL(set_shortcuts_enabled(bool,bool)),
            this, SLOT(set_shortcuts_enabled(bool,bool)));
    connect(conf_win, SIGNAL(error_occurred(QString)), this, SLOT(error_occurred(QString)));
    connect(conf_win, SIGNAL(toggle_start_on_boot()), this, SLOT(load_start_on_boot()));
    connect(conf_win, SIGNAL(login_pressed()), this, SLOT(db_login()));
    connect(conf_win, SIGNAL(logout_pressed()), this, SLOT(db_logout()));
}

void openpuush::init_ss_overlay()
{
    ss_overlay = new screenshot_overlay();
    connect(ss_overlay, SIGNAL(got_screenshot(QPixmap)), this, SLOT(got_screenshot(QPixmap)));
}

void openpuush::init_file_dialog()
{
    file_dialog = new QFileDialog();
    file_dialog->setFileMode(QFileDialog::AnyFile);

    connect(file_dialog, SIGNAL(fileSelected(QString)), this, SLOT(upload_file(QString)));
}

void openpuush::init_dropbox()
{
    db = new dropbox();

    connect(db, SIGNAL(got_authorized()), this, SLOT(db_authorized()));
    connect(db, SIGNAL(upload_progress(qint64, qint64)),
            this, SLOT(db_upload_progress(qint64,qint64)));
    connect(db, SIGNAL(upload_finished()), this, SLOT(db_upload_finished()));
    connect(db, SIGNAL(link_ready(QString)), this, SLOT(db_link_ready(QString)));
    connect(db, SIGNAL(got_account_info(QMap<QString,QString>)),
            this, SLOT(db_info_received(QMap<QString,QString>)));

    info_update_timer = new QTimer();

    connect(info_update_timer, SIGNAL(timeout()), this, SLOT(update_db_info()));

    QTimer::singleShot(0, db, SLOT(check_authentication()));
}

void openpuush::init_tray_icon()
{
    tray_icon = new QSystemTrayIcon(this);
    tray_icon->setIcon(QIcon(":/icons/0.png"));
    tray_icon->show();
    tray_icon->setToolTip("OpenPuush is ready.");

    connect(tray_icon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(tray_icon_activated(QSystemTrayIcon::ActivationReason)));
    connect(tray_icon, SIGNAL(messageClicked()), this, SLOT(follow_link()));
}

void openpuush::init_shortcuts()
{
    fullscreen_shortcut = new QxtGlobalShortcut(this);
    connect(fullscreen_shortcut, SIGNAL(activated()), this, SLOT(fullscreen_ss()));

    current_window_shortcut = new QxtGlobalShortcut(this);
    connect(current_window_shortcut, SIGNAL(activated()), this, SLOT(upload_current_window()));

    ss_overlay_shortcut = new QxtGlobalShortcut(this);
    connect(ss_overlay_shortcut, SIGNAL(activated()), this, SLOT(show_ss_overlay()));

    file_shortcut = new QxtGlobalShortcut(this);
    connect(file_shortcut, SIGNAL(activated()), file_dialog, SLOT(show()));

    upload_clipboard_shortcut = new QxtGlobalShortcut(this);
    connect(upload_clipboard_shortcut, SIGNAL(activated()), this, SLOT(upload_clipboard()));

    toggle_functionality_shortcut = new QxtGlobalShortcut(this);
    connect(toggle_functionality_shortcut, SIGNAL(activated()), this, SLOT(toggle_functionality()));

    update_shortcuts();
}

void openpuush::init_tray_icon_context_menu()
{
    menu = new QMenu();
    recent_menu = new QMenu("Recent Uploads");

    QAction * build_number = new QAction(QString("OpenPuush build #%1").arg(BUILD_NUMBER), this);
    build_number->setEnabled(false);
    connect(menu, SIGNAL(destroyed()), build_number, SLOT(deleteLater()));

    menu->addAction(build_number);
    menu->addSeparator();
    menu->addAction("My Account", conf_win, SLOT(on_go_to_dropbox_button_clicked()));
    menu->addSeparator();
    menu->addMenu(recent_menu);
    menu->addSeparator();
    menu->addAction("Capture Current Window", this, SLOT(upload_current_window()),
                    QKeySequence(config::load(config::CAPTURE_CURRENT_WINDOW).toString()));
    menu->addAction("Capture Desktop", this, SLOT(fullscreen_ss()),
                    QKeySequence(config::load(config::CAPTURE_FULLSCREEN).toString()));
    menu->addAction("Capture Area", this, SLOT(show_ss_overlay()),
                    QKeySequence(config::load(config::CAPTURE_AREA).toString()));
    menu->addAction("Upload Clipboard", this, SLOT(upload_clipboard()),
                    QKeySequence(config::load(config::UPLOAD_CLIPBOARD).toString()));
    menu->addAction("Upload File", file_dialog, SLOT(show()),
                    QKeySequence(config::load(config::UPLOAD_FILE).toString()));
    menu->addSeparator();
    menu->addAction("Toggle OpenPuush Shortcuts", this, SLOT(toggle_functionality()),
                    QKeySequence(config::load(config::TOGGLE_FUNCTIONALITY).toString()));
    menu->addAction("Settings", conf_win, SLOT(show()));
    menu->addSeparator();
    menu->addAction("Exit", this, SLOT(exit()));

    tray_icon->setContextMenu(menu);
}

void openpuush::update_shortcuts()
{
    set_shortcut(fullscreen_shortcut, CAPTURE_FULLSCREEN);
    set_shortcut(current_window_shortcut, CAPTURE_CURRENT_WINDOW);
    set_shortcut(ss_overlay_shortcut, CAPTURE_AREA);
    set_shortcut(file_shortcut, UPLOAD_FILE);
    set_shortcut(upload_clipboard_shortcut, UPLOAD_CLIPBOARD);
    set_shortcut(toggle_functionality_shortcut, TOGGLE_FUNCTIONALITY);
}

void openpuush::update_db_info()
{
    QTimer::singleShot(0, db, SLOT(account_info()));
}

void openpuush::db_authorized()
{
    dropbox_authenticated = true;
    QTimer::singleShot(200, db, SLOT(account_info()));
    info_update_timer->start(1000 * 60 * 5);
    conf_win->set_logged_in(configwindow::LOGGED_IN);
}

void openpuush::db_upload_progress(qint64 sent, qint64 total)
{
    int img_num = (sent * 10) / total;

    tray_icon->setIcon(QIcon(QString(":/icons/%1.png").arg(img_num)));
    tray_icon->setToolTip(QString("%1%").arg((sent * 100) / total));
}

void openpuush::db_upload_finished()
{
    tray_icon->setIcon(QIcon(":/icons/success.png"));
    QTimer::singleShot(2000, this, SLOT(tray_icon_normal()));
    tray_icon->showMessage("Success", link, QSystemTrayIcon::NoIcon, 2000);
    follow_tray_icon_link = true;

    recent_links.prepend(link);

    if (config::load(config::SOUND_NOTIFICATION).toBool())
    {
        /*Phonon::createPlayer(Phonon::VideoCategory,
                             Phonon::MediaSource(base_path + "/sounds/fin.ogg"))->play();*/
    }

    if (config::load(config::COPY_LINK).toBool())
    {
        QApplication::clipboard()->setText(link);
    }

    if (config::load(config::OPEN_LINK).toBool())
    {
        QDesktopServices::openUrl(link);
    }
}

void openpuush::db_upload_error()
{
    tray_icon->setIcon(QIcon(":/icons/error.png"));
    QTimer::singleShot(2000, this, SLOT(tray_icon_normal()));
    tray_icon->showMessage("Error", "The current action failed.");
    /*Phonon::createPlayer(Phonon::VideoCategory,
                         Phonon::MediaSource(base_path + "/sounds/err.ogg"))->play();*/
}

void openpuush::db_link_ready(QString link)
{
    this->link = link;
    qDebug() << link;
}

void openpuush::db_info_received(QMap<QString, QString> info)
{
    conf_win->update_account_info(info.value("display_name"), info.value("used"));
}

void openpuush::db_login()
{
    db->check_authentication();
}

void openpuush::db_logout()
{
    info_update_timer->stop();
    dropbox_authenticated = false;
    QTimer::singleShot(0, db, SLOT(logout()));
    conf_win->set_logged_in(configwindow::LOGGED_OUT);
}

void openpuush::got_screenshot(QPixmap ss)
{
    ss_overlay->close();

    bool saved_as_jpeg = false;

    QByteArray bytes;
    QBuffer buf(&bytes);
    buf.open(QIODevice::ReadWrite);

    ss.save(&buf, "png");

    if (config::load(config::SMART_COMPRESSION).toBool())
    {
        QByteArray jpeg_bytes;
        QBuffer jpeg_buf(&jpeg_bytes);
        jpeg_buf.open(QIODevice::ReadWrite);
        ss.save(&jpeg_buf, "jpeg");

        if (bytes.length() < jpeg_bytes.length())
        {
            saved_as_jpeg = true;
            bytes.clear();
            bytes = jpeg_bytes;
        }
    }

    QString name = QString("Screenshot-%1.").arg(QDateTime::currentDateTime().toTime_t());
    name += saved_as_jpeg ? "jpeg" : "png";

    if (bytes.length() > 0)
    {
        if (config::load(config::SAVE_COPY).toBool())
        {
            ss.save(config::load(config::LOCAL_COPY_PATH).toString() + "/" + name, "png");
        }
        db->file_put(bytes, name);
    }
    else
    {
        db_upload_error();
    }
}

void openpuush::show_ss_overlay()
{
    if (dropbox_authenticated)
    {
        ss_overlay->show();
    }
    else
    {
        error_occurred(NOT_AUTHORIZED_MESSAGE);
    }
}

void openpuush::fullscreen_ss()
{
    if (dropbox_authenticated)
    {
        QRect f;
        QDesktopWidget dw;
        if (config::load(config::CAPTURE_ALL_SCREENS).toBool())
        {
            f = ss_overlay->geometry();
        }
        else if (config::load(config::CAPTURE_SCREEN_WITH_CURSOR).toBool())
        {
            QPoint p = QCursor::pos();

            for (int i = 0; i < dw.screenCount(); ++i)
            {
                f = dw.screenGeometry(i);

                if (p.x() >= f.left())
                {
                    break;
                }
            }
        }
        else if (config::load(config::CAPTURE_PRIMARY_SCREEN).toBool())
        {
            f = dw.screenGeometry(dw.primaryScreen());
        }

        got_screenshot(QPixmap::grabWindow(QApplication::desktop()->winId(),
                                           f.left(), f.top(), f.width(), f.height()));
    }
    else
    {
        error_occurred(NOT_AUTHORIZED_MESSAGE);
    }
}

void openpuush::upload_file(QString filename)
{
    if (dropbox_authenticated)
    {
        // TODO: at some point revisit this and make it not load
        // the entire file into memory before upload

        QFile file(filename);
        QString name = QFileInfo(file).fileName();
        if (file.open(QIODevice::ReadOnly))
        {
            QByteArray contents = file.readAll();

            db->file_put(contents, name);
        }
        else
        {
            db_upload_error();
        }
    }
    else
    {
        error_occurred(NOT_AUTHORIZED_MESSAGE);
    }
}

void openpuush::upload_current_window()
{
#if defined(Q_OS_LINUX) || defined(Q_OS_WIN32)
    if (dropbox_authenticated)
    {
        WId window = QxtWindowSystem::activeWindow();
#if defined(Q_WS_X11)
        got_screenshot(QPixmap::grabWindow(window));
#else
        QRect f = QxtWindowSystem::windowGeometry(window);
        got_screenshot(QPixmap::grabWindow(QApplication::desktop()->winId(),
                                           f.left(), f.top(), f.width(), f.height()));
#endif
    }
    else
    {
        error_occurred(NOT_AUTHORIZED_MESSAGE);
    }
#else
#warning "Compiling on a platform without support for uploading a picture of the current window."
    error_occurred("The feature \"upload current window\" is not\nyet implemented for this platform.");
#endif
}

void openpuush::upload_clipboard()
{
    if (dropbox_authenticated)
    {
        const QMimeData * mime_data = QApplication::clipboard()->mimeData();

        if (mime_data->hasText())
        {
            QUrl url(mime_data->text().trimmed());
            if (url.isValid() && url.isLocalFile())
            {
                QFileInfo info(url.toLocalFile());
                if (info.isDir())
                {
                    qDebug() << "pretending to upload a directory";
                }
                else
                {
                    QStringList files = url.toString().split("\n");
                    if (files.length() > 1)
                    {
                        qDebug() << "pretending to upload multiple files";
                    }
                    else
                    {
                        upload_file(url.toLocalFile());
                    }
                }
            }
            else
            {
                QByteArray contents(mime_data->text().toAscii());
                QString name = QString("Text-%1.txt").arg(QDateTime::currentDateTime().toTime_t());
                db->file_put(contents, name);
            }
        }
        else if (mime_data->hasImage())
        {
            QImage img = qvariant_cast<QImage>(mime_data->imageData());
            got_screenshot(QPixmap::fromImage(img));
        }
    }
    else
    {
        error_occurred(NOT_AUTHORIZED_MESSAGE);
    }
}

void openpuush::toggle_functionality()
{
    set_shortcuts_enabled((shortcuts_enabled = !shortcuts_enabled), true);
    tray_icon->showMessage("Status", QString("OpenPuush is now %1.")
                           .arg(shortcuts_enabled ? "enabled" : "disabled"),
                           QSystemTrayIcon::Information, 2000);
}

void openpuush::tray_icon_activated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
        if (config::load(config::SHOW_SETTINGS).toBool())
        {
            if (conf_win->isHidden())
            {
                conf_win->show();
            }
            else
            {
                conf_win->hide();
            }
        }
        else if (config::load(config::BEGIN_SCREEN_CAPTURE).toBool())
        {
            show_ss_overlay();
        }
        else if (config::load(config::OPEN_FILE_DIALOG).toBool())
        {
            file_dialog->show();
        }
        break;
    case QSystemTrayIcon::Context:
    {
        recent_menu->clear();

        int size = recent_links.size();

        for (int i = 0; i < size && i < 5; ++i)
        {
            switch (i)
            {
#define case(n) case n: \
    recent_menu->addAction(recent_links[i].split("/").last(), this, SLOT(recent_link_ ## n())); \
    break;
            case(0)
            case(1)
            case(2)
            case(3)
            case(4)
#undef case
            default:
                break;
            }
        }

        break;
    }
    default:
        break;
    }
}

void openpuush::follow_link()
{
    if (follow_tray_icon_link)
    {
        QDesktopServices::openUrl(link);
        follow_tray_icon_link = false;
    }
}

void openpuush::tray_icon_normal()
{
    tray_icon->setIcon(QIcon(":/icons/0.png"));
    tray_icon->setToolTip("OpenPuush is ready.");
    follow_tray_icon_link = false;
}

void openpuush::set_shortcuts_enabled(bool enabled, bool all_but_toggle_functionality)
{
    if (enabled)
    {
        update_shortcuts();
    }

    fullscreen_shortcut->setEnabled(enabled);
    current_window_shortcut->setEnabled(enabled);
    ss_overlay_shortcut->setEnabled(enabled);
    file_shortcut->setEnabled(enabled);
    upload_clipboard_shortcut->setEnabled(enabled);

    if (!all_but_toggle_functionality)
    {
        toggle_functionality_shortcut->setEnabled(enabled);
    }
}

void openpuush::error_occurred(QString description)
{
    tray_icon->showMessage("Error", description, QSystemTrayIcon::Critical, 2000);
}

void openpuush::load_start_on_boot()
{
    // TODO: implement for windows and osx
    if (config::load(config::START_ON_BOOT).toBool())
    {
#if defined(Q_OS_LINUX)
        QFile xsession(QDir::homePath() + "/" + ".xsession");
        bool existed = xsession.exists();
        xsession.open(QIODevice::Append | QIODevice::Text);
        QTextStream out(&xsession);

        if (!existed)
        {
            out << "#!/bin/sh\r\n";
        }

        out << "./.openpuu.sh\r\n";

        QFile opscript(QDir::homePath() + "/" + ".openpuu.sh");
        opscript.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream o(&opscript);
        o <<  QApplication::applicationFilePath() + " &\r\n";
#elif defined(Q_OS_MAC)

#elif defined(Q_OS_WIN32)

#else
#warning "Compiling for a system without support for starting OpenPuush on boot."
        error_occurred("Starting on boot is not yet supported on this platform");
#endif
    }
    else
    {
#if defined(Q_OS_LINUX)
        QDir::home().remove(".openpuu.sh");
#elif defined(Q_OS_MAC)

#elif defined(Q_OS_WIN32)

#else
        error_occurred("Starting on boot is not yet supported on this platform");
#endif
    }
}

void openpuush::exit()
{
    std::exit(0);
}

#define recent_link(n) \
    void openpuush::recent_link_ ## n() \
    { \
        QDesktopServices::openUrl(recent_links[n]); \
    }

recent_link(0)
recent_link(1)
recent_link(2)
recent_link(3)
recent_link(4)
