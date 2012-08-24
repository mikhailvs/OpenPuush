#include <QMap>
#include <QObject>

#include "signal_proxy.h"

class internal_signal_proxy : public QObject
{
    Q_OBJECT
public:
    explicit internal_signal_proxy(QObject * parent = 0) : QObject(parent)
    {
    }

    void activate()
    {
        emit activated();
    }

signals:
    void activated();
};

static QMap<QString, internal_signal_proxy> signal_lookup;

signal_proxy::signal_proxy()
{
}
