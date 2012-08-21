#include <QApplication>

#include "openpuush.hpp"
#include "dropbox.hpp"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setApplicationName("OpenPuush");

    openpuush m;
    (void)(m);

    return a.exec();
}
