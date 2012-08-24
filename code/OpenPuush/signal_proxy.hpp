#ifndef SIGNAL_PROXY_H
#define SIGNAL_PROXY_H

class signal_proxy
{
public:
    void send(const QString &);
    void recv(const QString &, const char *, const QObject *);

private:
    signal_proxy();

    static signal_proxy _instance;
};

#endif // SIGNAL_PROXY_H
