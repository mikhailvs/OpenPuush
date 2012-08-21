#include <QString>
#include <QUrl>
#include <QCryptographicHash>
#include <QByteArray>
#include <QDateTime>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDesktopServices>
#include <QRegExp>
#include <QStringList>
#include <QTcpServer>
#include <QTcpSocket>
#include <QApplication>
#include <QClipboard>
#include <QFile>
#include <QFileInfo>

#include <QxtCore/QxtJSON>

#include "dropbox.hpp"
#include "config.hpp"

dropbox::dropbox(QObject *) : app_key("nwy4ujlf00t1cjt"),
                     app_secret("fm4qw5sjgoqqmg5"),
                     base_url("https://api.dropbox.com/1"),
                     conf_location("./openpuush.conf")
{
    manager = new QNetworkAccessManager(this);
    server = new QTcpServer(this);

    connect(server, SIGNAL(newConnection()), this, SLOT(http_connection()));
    connect(manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(network_reply(QNetworkReply *)));

    current_action = OTHER;
}

dropbox::~dropbox()
{
    delete manager;
    delete server;
}

void dropbox::check_authentication()
{
    if (!load_config())
    {
        authenticate();
    }
    else
    {
        emit got_authorized();
    }
}

void dropbox::logout()
{
    config::save(config::DROPBOX_OAUTH_ACCESS_TOKEN_SECRET, "");
    config::save(config::DROPBOX_OAUTH_ACCESS_TOKEN, "");
    config::save(config::DROPBOX_UID, "");
    config::save(config::DROPBOX_DISPLAY_NAME, "");
    config::save(config::DROPBOX_USED, "");

    oauth_access_token = "";
    oauth_access_token_secret = "";
    oauth_token_secret = "";
    uid = "";
}

void dropbox::network_reply(QNetworkReply * reply)
{
    QString raw = QString(reply->readAll());
    QStringList params = raw.split(QString("&"));
    switch (current_action)
    {
    case REQUEST_TOKEN:
    {
        try
        {
            oauth_token_secret = params[0].split("=")[1];
            QString oauth_token = params[1].split("=")[1];

            QString url = "https://www.dropbox.com/1/oauth/authorize?oauth_token=" +
                    oauth_token + "&oauth_callback=http://127.0.0.1:8080";

            QDesktopServices::openUrl(url);

            reply->deleteLater();

            if (!server->listen(QHostAddress::LocalHost, 8080))
            {
                emit error_occurred("Could not start local server");
            }
        }
        catch (...)
        {
            emit error_occurred("Unable to parse request token parameters.");
        }

        break;
    }
    case ACCESS_TOKEN:
    {
        try
        {
            oauth_access_token_secret =  params[0].split("=")[1];
            oauth_access_token =  params[1].split("=")[1];
            uid =  params[2].split("=")[1];
            emit got_authorized();

            save_config();
        }
        catch (...)
        {
            emit error_occurred("Unable to parse access token parameters.");
        }

        break;
    }
    case ACCOUNT_INFO:
    {
        try
        {
            QMap<QString, QVariant> json = QxtJSON::parse(raw).toMap();
            QMap<QString, QVariant> quota_info = json.value("quota_info").toMap();
            QMap<QString, QString> info;

            double percent_used = quota_info.value("normal").toDouble() /
                                  quota_info.value("quota").toDouble();
            percent_used *= 100;

            QString used = QString("%1%").arg(percent_used, 0, 'f', 1);
            QString display_name = json.value("display_name").toString();

            info.insert("display_name", display_name);
            info.insert("used", used);

            config::save(config::DROPBOX_DISPLAY_NAME, display_name);
            config::save(config::DROPBOX_USED, used);

            emit got_account_info(info);
        }
        catch (...)
        {
            emit error_occurred("Unable to parse account info.");
        }

        break;
    }
    case FILES_PUT:
    {
        break;
    }
    case FILEOPS_CREATE_FOLDER:
    {
        break;
    }
    default:
        break;
    }
    current_action = OTHER;
}

void dropbox::account_info()
{
    QNetworkRequest request(QUrl(base_url + "/account/info"));
    QByteArray authorization = oauth_authorization_header(base_url + "/account/info",
                                app_key,
                                oauth_access_token,
                                "PLAINTEXT",
                                encode(app_secret) + "&" + encode(oauth_access_token_secret));
    request.setRawHeader(QByteArray("Authorization"), authorization);

    current_action = ACCOUNT_INFO;
    manager->get(request);
}

void dropbox::file_put(const QString & path)
{
    QFile file(path);
    QFileInfo info(file);
    file.open(QIODevice::ReadOnly);
    QDataStream data(&file);
    QString filename(info.fileName());
    file_put(data, filename);
}

void dropbox::file_put(QDataStream & data, QString & filename)
{
    QUrl url(QString("https://api-content.dropbox.com/1/files_put/dropbox/Public/OpenPuush/%1")
             .arg(filename));
    QNetworkRequest request(url);
    QByteArray authorization = oauth_authorization_header(url.toString(),
                                app_key,
                                oauth_access_token,
                                "PLAINTEXT",
                                encode(app_secret) + "&" + encode(oauth_access_token_secret));
    request.setRawHeader(QByteArray("Authorization"), authorization);

    current_action = FILES_PUT;
    reply = manager->put(request, data.device());
    reply_sent_common(QString("http://dl.dropbox.com/u/%1/OpenPuush/%2").arg(uid, encode(filename)));
}

void dropbox::file_put(QByteArray & data, QString & filename)
{
    QUrl url(QString("https://api-content.dropbox.com/1/files_put/dropbox/Public/OpenPuush/%1")
             .arg(filename));
    QNetworkRequest request(url);
    QByteArray authorization = oauth_authorization_header(url.toString(),
                                app_key,
                                oauth_access_token,
                                "PLAINTEXT",
                                encode(app_secret) + "&" + encode(oauth_access_token_secret));
    request.setRawHeader(QByteArray("Authorization"), authorization);

    current_action = FILES_PUT;
    reply = manager->put(request, data);
    reply_sent_common(QString("http://dl.dropbox.com/u/%1/OpenPuush/%2").arg(uid, encode(filename)));
}

void dropbox::reply_sent_common(const QString & link)
{
    connect(reply, SIGNAL(uploadProgress(qint64, qint64)), this, SLOT(file_upload_progress_slot(qint64,qint64)));
    connect(reply, SIGNAL(finished()), this, SLOT(file_upload_finished_slot()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(file_upload_error()));

    emit link_ready(link);
}

void dropbox::http_connection()
{
    client = server->nextPendingConnection();
    connect(client, SIGNAL(disconnected()), client, SLOT(deleteLater()));

    QByteArray block;
    QDataStream out(&block, QIODevice::ReadWrite);
    out.setVersion(QDataStream::Qt_4_8);

    QString raw_request;
    if (client->waitForReadyRead(1000))
    {
        raw_request = QString(client->readAll());
    }

    QStringList params = raw_request.mid(6, raw_request.indexOf(" HTTP") - 6).split("&");
    QString uid = params[0].split("=")[1];
    QString oauth_token = params[1].split("=")[1];

    out << "HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n"
           "<!doctype html>"
           "<html>"
           "<head>"
           "<title>Authorized OpenPush</title>"
           "</head>"
           "<body>"
           "<h1>Authorized Openpush!</h1>"
           "<h3>You may close this window whenever you feel like it."
           "</body>"
           "</html>";

    client->write(block);
    client->close();
    server->close();

    QNetworkRequest request(QUrl(base_url + "/oauth/access_token"));
    QByteArray authorization = oauth_authorization_header(base_url + "/account/info",
                                app_key,
                                oauth_token,
                                "PLAINTEXT",
                                encode(app_secret) + "&" + encode(oauth_token_secret));
    request.setRawHeader(QByteArray("Authorization"), authorization);

    current_action = ACCESS_TOKEN;
    manager->get(request);
}

QByteArray dropbox::oauth_authorization_header(const QString & realm,
                                               const QString & oauth_consumer_key,
                                               const QString & oauth_token,
                                               const QString & oauth_signature_method,
                                               const QString & oauth_signature)
{
    QByteArray val;
    val.append(QString("OAuth realm=\"%1\","
               "oauth_consumer_key=\"%2\","
               "oauth_token=\"%3\","
               "oauth_nonce=\"%4\","
               "oauth_timestamp=\"%5\","
               "oauth_signature_method=\"%6\","
               "oauth_version=\"%7\","
               "oauth_signature=\"%8\"")
            .arg(realm,
                 oauth_consumer_key,
                 oauth_token,
                 encode(nonce()),
                 timestamp(),
                 oauth_signature_method,
                 "1.0",
                 encode(oauth_signature)));
    return val;
}

void dropbox::authenticate()
{
    QString signature = encode(app_secret) + "&";

    QUrl url;
    url.setUrl(base_url + "/oauth/request_token");
    url.addQueryItem("oauth_consumer_key", app_key);
    url.addQueryItem("oauth_nonce",  encode(nonce()));
    url.addQueryItem("oauth_signature", encode(signature));
    url.addQueryItem("oauth_signature_method", "PLAINTEXT");
    url.addQueryItem("oauth_timestamp", encode(timestamp()));
    url.addQueryItem("oauth_version", "1.0");

    current_action = REQUEST_TOKEN;
    manager->post(QNetworkRequest(url), QByteArray());
}

QString dropbox::timestamp()
{
    return QString("%1").arg(QDateTime::currentDateTime().toTime_t());
}

QString dropbox::nonce()
{
    QByteArray b;
    b.append(timestamp());
    return QString(QCryptographicHash::hash(b, QCryptographicHash::Sha1).toHex());
}

QString dropbox::encode(const QString & str)
{
    return QString(QUrl::toPercentEncoding(str));
}

bool dropbox::load_config()
{
    oauth_access_token_secret = config::load(config::DROPBOX_OAUTH_ACCESS_TOKEN_SECRET).toString();
    oauth_access_token = config::load(config::DROPBOX_OAUTH_ACCESS_TOKEN).toString();
    uid = config::load(config::DROPBOX_UID).toString();

    bool success = true;
    if (oauth_access_token_secret == "" || oauth_access_token == "" || uid == "")
    {
        success = false;
    }

    return success;
}

void dropbox::save_config()
{
    config::save(config::DROPBOX_OAUTH_ACCESS_TOKEN_SECRET, oauth_access_token_secret);
    config::save(config::DROPBOX_OAUTH_ACCESS_TOKEN, oauth_access_token);
    config::save(config::DROPBOX_UID, uid);
}

void dropbox::file_upload_progress_slot(qint64 sent, qint64 total)
{
    emit upload_progress(sent, total);
}

void dropbox::file_upload_finished_slot()
{
    emit upload_finished();
}

void dropbox::file_upload_error()
{
    emit upload_error();
}
