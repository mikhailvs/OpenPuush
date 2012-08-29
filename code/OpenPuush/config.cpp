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

#include <cstdlib>
#include <QSettings>
#include <QString>

#include "config.hpp"

static const char * names[] = {
    "dropbox/oauth/access_token_secret",
    "dropbox/oauth/access_token",
    "dropbox/user/display_name",
    "dropbox/user/uid",
    "dropbox/user/used",
    "general/general_settings/start_on_boot",
    "general/on_upload/sound_notification",
    "general/on_upload/copy_link",
    "general/on_upload/open_link",
    "general/on_upload/save_copy",
    "general/on_upload/local_copy_path",
    "general/icon_onclick/show_settings",
    "general/icon_onclick/begin_screen_capture",
    "general/icon_onclick/open_file_dialog",
    "shortcuts/capture_fullscreen",
    "shortcuts/capture_current_window",
    "shortcuts/capture_area",
    "shortcuts/upload_file",
    "shortcuts/upload_clipboard",
    "shortcuts/toggle_functionality",
    "updates/last_update_check",
    "advanced/compression/no_compression",
    "advanced/compression/smart_compression",
    "advanced/desktop_integration/show_explorer_menu",
    "advanced/multiple_monitors/capture_all_screens",
    "advanced/multiple_monitors/capture_screen_with_cursor",
    "advanced/multiple_monitors/capture_primary_screen",
    "advanced/experimental/enable_experimental"
};

static QSettings * _instance;

config::config()
{
    std::atexit(&cleanup);
}

void config::cleanup()
{
    if (_instance)
    {
        delete _instance;
        _instance = 0;
    }
}

QSettings & config::instance()
{
    static QSettings * _instance = 0;
    if (!_instance)
    {
        _instance = new QSettings(QSettings::IniFormat, QSettings::UserScope, "opuu.sh", "OpenPuush");
    }
    return *_instance;
}

void config::save(setting setting, const QVariant & val)
{
    instance().setValue(names[setting], val);
}

QVariant config::load(config::setting setting, const QVariant & def)
{
    return instance().value(names[setting], def);
}

void config::defaults()
{
    save(START_ON_BOOT,              true);
    save(SOUND_NOTIFICATION,         true);
    save(COPY_LINK,                  true);
    save(OPEN_LINK,                  false);
    save(SAVE_COPY,                  false);
    save(LOCAL_COPY_PATH,            "");
    save(SHOW_SETTINGS,              true);
    save(BEGIN_SCREEN_CAPTURE,       false);
    save(OPEN_FILE_DIALOG,           false);
#if defined(Q_OS_MAC)
    save(CAPTURE_FULLSCREEN,         "Meta+Shift+3");
    save(CAPTURE_CURRENT_WINDOW,     "Meta+Shift+2");
    save(CAPTURE_AREA,               "Meta+Shift+4");
    save(UPLOAD_FILE,                "Meta+Shift+U");
    save(UPLOAD_CLIPBOARD,           "Meta+Shift+5");
    save(TOGGLE_FUNCTIONALITY,       "Meta+Alt+P");
#else
    save(CAPTURE_FULLSCREEN,         "Ctrl+Shift+3");
    save(CAPTURE_CURRENT_WINDOW,     "Ctrl+Shift+2");
    save(CAPTURE_AREA,               "Ctrl+Shift+4");
    save(UPLOAD_FILE,                "Ctrl+Shift+U");
    save(UPLOAD_CLIPBOARD,           "Ctrl+Shift+5");
    save(TOGGLE_FUNCTIONALITY,       "Ctrl+Alt+P");
#endif
    save(LAST_UPDATE_CHECK,          0);
    save(NO_COMPRESSION,             true);
    save(SMART_COMPRESSION,          false);
    save(SHOW_EXPLORER_MENU,         true);
    save(CAPTURE_ALL_SCREENS,        true);
    save(CAPTURE_SCREEN_WITH_CURSOR, false);
    save(CAPTURE_PRIMARY_SCREEN,     false);
    save(ENABLE_EXPERIMENTAL,        false);
}

bool config::first_run()
{
    bool r = 0;
    for (int i = 5; i < 28; ++i)
    {
        r |= instance().contains(names[i]);
    }
    return !r;
}
