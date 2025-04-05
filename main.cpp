#include "gamepanel.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qRegisterMetaType<Cards>("Cards&");
    qRegisterMetaType<Cards>("Cards");
    GamePanel w;
    w.show();
    return a.exec();
}
