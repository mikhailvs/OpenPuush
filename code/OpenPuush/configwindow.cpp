#include <QFileDialog>
#include <QDir>
#include <QDebug>
#include <QKeyEvent>
#include <QDesktopServices>
#include <QUrl>

#include "configwindow.hpp"
#include "ui_configwindow.h"

#define TOGGLE_SETTING(m, s) void configwindow::m(bool _) { \
    config::save(config::s, _); \
}

#define SET_SHORTCUT(m, s) void configwindow::m() { \
    current_shortcut = config::s; \
    n_keys_pressed = 0; \
    emit set_shortcuts_enabled(false, false); \
}

#define LOAD_CHECKBOX(m, s) ui->m->setChecked(config::load(config::s).toBool())

#define LOAD_SHORTCUT(m, s) ui->m->setText(config::load(config::s).toString())

configwindow::configwindow(QWidget *parent) :
    QTabWidget(parent),
    ui(new Ui::configwindow),
    n_keys_pressed(0),
    logged_in(true)
{
    ui->setupUi(this);

    current_shortcut = config::NONE;

    init_settings();
    init_account();
}

configwindow::~configwindow()
{
    delete ui;
}

void configwindow::init_settings()
{
    LOAD_CHECKBOX(start_on_startup,              START_ON_BOOT);
    LOAD_CHECKBOX(play_notification_sound,       SOUND_NOTIFICATION);
    LOAD_CHECKBOX(copy_link_to_clipboard,        COPY_LINK);
    LOAD_CHECKBOX(open_link_in_browser,          OPEN_LINK);
    LOAD_CHECKBOX(save_a_local_copy_of_image,    SAVE_COPY);
    LOAD_CHECKBOX(show_settings_dialog,          SHOW_SETTINGS);
    LOAD_CHECKBOX(begin_screen_capture_mode,     BEGIN_SCREEN_CAPTURE);
    LOAD_CHECKBOX(open_upload_file_dialog,       OPEN_FILE_DIALOG);
    LOAD_CHECKBOX(no_compression,                NO_COMPRESSION);
    LOAD_CHECKBOX(smart_compression,             SMART_COMPRESSION);
    LOAD_CHECKBOX(show_context_menu,             SHOW_EXPLORER_MENU);
    LOAD_CHECKBOX(capture_all_screens,           CAPTURE_ALL_SCREENS);
    LOAD_CHECKBOX(capture_primary_screen,        CAPTURE_PRIMARY_SCREEN);
    LOAD_CHECKBOX(capture_screen_with_cursor,    CAPTURE_SCREEN_WITH_CURSOR);
    LOAD_CHECKBOX(enable_experimental_features,  ENABLE_EXPERIMENTAL);

    LOAD_SHORTCUT(capture_fullscreen_button,     CAPTURE_FULLSCREEN);
    LOAD_SHORTCUT(capture_current_window_button, CAPTURE_CURRENT_WINDOW);
    LOAD_SHORTCUT(capture_area_button,           CAPTURE_AREA);
    LOAD_SHORTCUT(upload_file_button,            UPLOAD_FILE);
    LOAD_SHORTCUT(upload_clipboard_button,       UPLOAD_CLIPBOARD);
    LOAD_SHORTCUT(toggle_functionality_button,   TOGGLE_FUNCTIONALITY);

    ui->save_a_local_copy_of_image->setEnabled(config::load(config::SAVE_COPY).toBool());
    ui->path_to_local_copy->setText(config::load(config::LOCAL_COPY_PATH).toString());
}

TOGGLE_SETTING(on_play_notification_sound_toggled,      SOUND_NOTIFICATION)
TOGGLE_SETTING(on_copy_link_to_clipboard_toggled,       COPY_LINK)
TOGGLE_SETTING(on_open_link_in_browser_toggled,         OPEN_LINK)
TOGGLE_SETTING(on_show_settings_dialog_toggled,         SHOW_SETTINGS)
TOGGLE_SETTING(on_begin_screen_capture_mode_toggled,    BEGIN_SCREEN_CAPTURE)
TOGGLE_SETTING(on_open_upload_file_dialog_toggled,      OPEN_FILE_DIALOG)
TOGGLE_SETTING(on_no_compression_toggled,               NO_COMPRESSION)
TOGGLE_SETTING(on_smart_compression_toggled,            SMART_COMPRESSION)
TOGGLE_SETTING(on_show_context_menu_toggled,            SHOW_EXPLORER_MENU)
TOGGLE_SETTING(on_capture_all_screens_toggled,          CAPTURE_ALL_SCREENS)
TOGGLE_SETTING(on_capture_primary_screen_toggled,       CAPTURE_PRIMARY_SCREEN)
TOGGLE_SETTING(on_capture_screen_with_cursor_toggled,   CAPTURE_SCREEN_WITH_CURSOR)
TOGGLE_SETTING(on_enable_experimental_features_toggled, ENABLE_EXPERIMENTAL)

void configwindow::on_start_on_startup_toggled(bool checked)
{
    config::save(config::START_ON_BOOT, checked);

    emit toggle_start_on_boot();
}

void configwindow::on_save_a_local_copy_of_image_toggled(bool checked)
{
    if (!checked)
    {
        ui->save_a_local_copy_of_image->setChecked(checked);
        ui->save_a_local_copy_of_image->setEnabled(false);
        ui->path_to_local_copy->setText("");
        config::save(config::SAVE_COPY, false);
        qDebug() << "here i am, in on_save_a_local_copy_of_image_toggled";
    }
}

void configwindow::on_browse_for_local_path_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Choose where to store local copies",
                                                    QDir::homePath());

    if (QDir(dir).exists())
    {
        ui->path_to_local_copy->setText(dir);

        config::save(config::LOCAL_COPY_PATH, dir);
        config::save(config::SAVE_COPY, true);
        ui->save_a_local_copy_of_image->setChecked(true);
        ui->save_a_local_copy_of_image->setEnabled(true);
    }
    else
    {
        emit error_occurred("The directory you selected doesnt exist!");
    }
}

QString configwindow::lookup(int key)
{
    QString str;
    switch ((Qt::Key) key)
    {
#define _case(key) case Qt::Key_##key:{str=#key;break;}
#define __case(key, val) case Qt::Key_##key:{str=#val;break;}
    _case(Shift)
    __case(Control, Ctrl)
    _case(Meta)
    _case(Alt)
    _case(CapsLock)
    _case(NumLock)
    _case(ScrollLock)
    _case(Return)
    _case(Enter)
    _case(Insert)
    _case(Delete)
    _case(Left)
    _case(Up)
    _case(Right)
    _case(Down)
    _case(Escape)
    _case(A)
    _case(B)
    _case(C)
    _case(D)
    _case(E)
    _case(F)
    _case(G)
    _case(H)
    _case(I)
    _case(J)
    _case(K)
    _case(L)
    _case(M)
    _case(N)
    _case(O)
    _case(P)
    _case(Q)
    _case(R)
    _case(S)
    _case(T)
    _case(U)
    _case(V)
    _case(W)
    _case(X)
    _case(Y)
    _case(Z)
    case Qt::Key_ParenRight:
    _case(0)
    case Qt::Key_Exclam:
    _case(1)
    case Qt::Key_At:
    _case(2)
    case Qt::Key_NumberSign:
    _case(3)
    case Qt::Key_Dollar:
    _case(4)
    case Qt::Key_Percent:
    _case(5)
    case Qt::Key_AsciiCircum: // "caret"
    _case(6)
    case Qt::Key_Ampersand:
    _case(7)
    case Qt::Key_Asterisk:
    _case(8)
    case Qt::Key_ParenLeft:
    _case(9)
#undef _case
#undef __case
    default:
        break;
    }
    return str;
}

void configwindow::keyPressEvent(QKeyEvent * e)
{
    QTabWidget::keyPressEvent(e);
    if (current_shortcut != config::NONE)
    {
        ++n_keys_pressed;
        keys_pressed.append(lookup(e->key()));
    }
}

void configwindow::keyReleaseEvent(QKeyEvent * e)
{
    QTabWidget::keyReleaseEvent(e);
    if (current_shortcut != config::NONE && --n_keys_pressed == 0)
    {
        QString shortcut = keys_pressed.join("+");

        config::setting test[] = {
            config::CAPTURE_FULLSCREEN,
            config::CAPTURE_CURRENT_WINDOW,
            config::CAPTURE_AREA,
            config::UPLOAD_FILE,
            config::UPLOAD_CLIPBOARD,
            config::TOGGLE_FUNCTIONALITY
        };

        QPushButton * button;
        switch (current_shortcut)
        {
        case config::CAPTURE_FULLSCREEN:
            button = ui->capture_fullscreen_button;
            break;
        case config::CAPTURE_CURRENT_WINDOW:
            button = ui->capture_current_window_button;
            break;
        case config::CAPTURE_AREA:
            button = ui->capture_area_button;
            break;
        case config::UPLOAD_FILE:
            button = ui->upload_file_button;
            break;
        case config::UPLOAD_CLIPBOARD:
            button = ui->upload_clipboard_button;
            break;
        case config::TOGGLE_FUNCTIONALITY:
            button = ui->toggle_functionality_button;
            break;
        default:
            break;
        }

        bool okay = true;
        for (int i = 0; i < 6; ++i)
        {
            if (config::load(test[i]).toString() == shortcut && button->text() != shortcut)
            {
                okay = false;
            }
        }

        if (okay)
        {
            config::save(current_shortcut, shortcut);
            button->setText(shortcut);
        }
        else
        {
            emit error_occurred(QString("The shortcut \"%1\" is already being used!").arg(shortcut));
        }

        keys_pressed.clear();
        current_shortcut = config::NONE;
        emit set_shortcuts_enabled(true, false);
    }
}

SET_SHORTCUT(on_capture_fullscreen_button_clicked, CAPTURE_FULLSCREEN)
SET_SHORTCUT(on_capture_current_window_button_clicked, CAPTURE_CURRENT_WINDOW)
SET_SHORTCUT(on_capture_area_button_clicked, CAPTURE_AREA)
SET_SHORTCUT(on_upload_file_button_clicked, UPLOAD_FILE)
SET_SHORTCUT(on_upload_clipboard_button_clicked, UPLOAD_CLIPBOARD)
SET_SHORTCUT(on_toggle_functionality_button_clicked, TOGGLE_FUNCTIONALITY)

void configwindow::init_account()
{
    ui->username_label->setText(config::load(config::DROPBOX_DISPLAY_NAME, "hmbl9r").toString());
    ui->used_info_label->setText(config::load(config::DROPBOX_USED, "0%").toString());
}

void configwindow::update_account_info(QString name, QString used)
{
    ui->username_label->setText(name);
    ui->used_info_label->setText(used);
}

void configwindow::on_go_to_dropbox_button_clicked()
{
    QDesktopServices::openUrl(QUrl("https://www.dropbox.com/home/Public/OpenPuush"));
}

void configwindow::on_logout_button_clicked()
{
    if (!logged_in)
    {
        emit login_pressed();
    }
    else
    {
        emit logout_pressed();
    }

    logged_in = !logged_in;
}

void configwindow::set_logged_in(login_state state)
{
    ui->logout_button->setEnabled(true);
    switch (state)
    {
    case LOGGED_IN:
        ui->logout_button->setText("Logout");
        break;
    case LOGGED_OUT:
        ui->logout_button->setText("Login");
        ui->username_label->setText("Unknown");
        ui->used_info_label->setText("Unknown");
        break;
    }
}

void configwindow::on_check_for_updates_button_clicked()
{
    emit error_occurred("This feature has not yet been implemented.");
}
