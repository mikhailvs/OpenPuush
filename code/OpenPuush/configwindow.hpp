#ifndef CONFIGWINDOW_HPP
#define CONFIGWINDOW_HPP

#include <QTabWidget>
#include "config.hpp"

namespace Ui {
    class configwindow;
}

class QStringList;

class configwindow : public QTabWidget
{
    Q_OBJECT
    
public:
    enum login_state
    {
        LOGGED_IN,
        LOGGED_OUT
    };

    explicit configwindow(QWidget *parent = 0);
    ~configwindow();
    void update_account_info(QString, QString);

public slots:
    void set_logged_in(login_state);
    
private slots:
    void on_start_on_startup_toggled(bool checked);
    void on_play_notification_sound_toggled(bool checked);
    void on_copy_link_to_clipboard_toggled(bool checked);
    void on_open_link_in_browser_toggled(bool checked);
    void on_save_a_local_copy_of_image_toggled(bool checked);
    void on_show_settings_dialog_toggled(bool checked);
    void on_begin_screen_capture_mode_toggled(bool checked);
    void on_open_upload_file_dialog_toggled(bool checked);
    void on_no_compression_toggled(bool checked);
    void on_smart_compression_toggled(bool checked);
    void on_show_context_menu_toggled(bool checked);
    void on_capture_all_screens_toggled(bool checked);
    void on_capture_primary_screen_toggled(bool checked);
    void on_capture_screen_with_cursor_toggled(bool checked);
    void on_enable_experimental_features_toggled(bool checked);

    void on_browse_for_local_path_clicked();

    void on_capture_fullscreen_button_clicked();
    void on_capture_current_window_button_clicked();
    void on_capture_area_button_clicked();
    void on_upload_file_button_clicked();
    void on_upload_clipboard_button_clicked();
    void on_toggle_functionality_button_clicked();

    void on_go_to_dropbox_button_clicked();

    void on_logout_button_clicked();

    void on_check_for_updates_button_clicked();

signals:
    void set_shortcuts_enabled(bool, bool);
    void toggle_start_on_boot();
    void error_occurred(QString);
    void logout_pressed();
    void login_pressed();

protected:
    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);

private:
    Ui::configwindow * ui;

    QString lookup(int);

    void init_settings();
    void init_account();

    config::setting current_shortcut;
    short n_keys_pressed;
    QStringList keys_pressed;
    bool logged_in;
};

#endif // CONFIGWINDOW_HPP
