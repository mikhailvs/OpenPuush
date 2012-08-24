#ifndef SIGNAL_PROXY_H
#define SIGNAL_PROXY_H

#include <QObject>

class signal_proxy : public QObject
{
    Q_OBJECT
public:
    explicit signal_proxy(QObject *parent = 0);
    
signals:
    
public slots:
    
};

#endif // SIGNAL_PROXY_H
