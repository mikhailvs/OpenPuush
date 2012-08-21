#ifndef DROPBOX_HPP
#define DROPBOX_HPP

#include <QObject>
#include <QMap>

class QString;
class QNetworkReply;
class QNetworkAccessManager;
class QTcpServer;
class QTcpSocket;
class QFile;
class QNetworkReply;

class dropbox : public QObject
{
    Q_OBJECT

public:
    explicit dropbox(QObject * parent = 0);
    ~dropbox();

    void file_put(const QString &);
    void file_put(QDataStream &, QString &);
    void file_put(QByteArray &, QString &);

public slots:
    void check_authentication();
    void logout();

private slots:
    void network_reply(QNetworkReply *);
    void http_connection();
    void file_upload_progress_slot(qint64, qint64);
    void file_upload_finished_slot();
    void file_upload_error();
    void account_info();

signals:
    void got_authorized();
    void got_account_info(QMap<QString, QString>);
    void upload_progress(qint64, qint64);
    void upload_finished();
    void upload_error();
    void link_ready(QString);
    void error_occurred(QString);

private:
    enum action
    {
        REQUEST_TOKEN,
        ACCESS_TOKEN,
        ACCOUNT_INFO,
        FILES_GET,
        FILES_PUT,
        FILES_POST,
        METADATA,
        DELTA,
        REVISIONS,
        RESTORE,
        SEARCH,
        SHARES,
        MEDIA,
        COPY_REF,
        THUMBNAILS,
        FILEOPS_COPY,
        FILEOPS_CREATE_FOLDER,
        FILEOPS_DELETE,
        FILEOPS_MOVE,
        OTHER
    };

    const QString app_key;
    const QString app_secret;
    const QString base_url;
    const QString conf_location;

    QString oauth_token_secret;
    QString oauth_access_token_secret;
    QString oauth_access_token;
    QString uid;

    action current_action;

    QNetworkAccessManager * manager;
    QNetworkReply         * reply;
    QTcpServer            * server;
    QTcpSocket            * client;

    void authenticate();
    void save_config();
    bool load_config();
    void reply_sent_common(const QString &);

    QString timestamp();
    QString nonce();
    QString encode(const QString &);
    QByteArray oauth_authorization_header(const QString &,
                                          const QString &,
                                          const QString &,
                                          const QString &,
                                          const QString &);
};

#endif // DROPBOX_HPP
