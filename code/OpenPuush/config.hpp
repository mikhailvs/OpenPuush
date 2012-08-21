#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <QVariant>
#define UNUSED(x) (void)(x)

class QString;
class QSettings;

class config
{
public:
    enum setting
    {
        DROPBOX_OAUTH_ACCESS_TOKEN_SECRET = 0,
        DROPBOX_OAUTH_ACCESS_TOKEN,
        DROPBOX_DISPLAY_NAME,
        DROPBOX_UID,
        DROPBOX_USED,
        START_ON_BOOT,
        SOUND_NOTIFICATION,
        COPY_LINK,
        OPEN_LINK,
        SAVE_COPY,
        LOCAL_COPY_PATH,
        SHOW_SETTINGS,
        BEGIN_SCREEN_CAPTURE,
        OPEN_FILE_DIALOG,
        CAPTURE_FULLSCREEN,
        CAPTURE_CURRENT_WINDOW,
        CAPTURE_AREA,
        UPLOAD_FILE,
        UPLOAD_CLIPBOARD,
        TOGGLE_FUNCTIONALITY,
        LAST_UPDATE_CHECK,
        NO_COMPRESSION,
        SMART_COMPRESSION,
        SHOW_EXPLORER_MENU,
        CAPTURE_ALL_SCREENS,
        CAPTURE_SCREEN_WITH_CURSOR,
        CAPTURE_PRIMARY_SCREEN,
        ENABLE_EXPERIMENTAL,
        NONE
    };

    static void save(setting, const QVariant &);
    static QVariant load(setting, const QVariant & def = QVariant());
    static void defaults();
    static bool first_run();

private:
    config();
    static void cleanup();
    static QSettings & instance();
};

#endif // CONFIG_HPP
