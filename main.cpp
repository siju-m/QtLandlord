#include "loading.h"
#include "Cards.h"

#include <QApplication>
// #include <QResource>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qRegisterMetaType<Cards>("Cards&");
    qRegisterMetaType<Cards>("Cards");
    // 命令行 rcc -binary .\res.qrc -o resource.rcc
    // 根据qrc文件生成rcc文件，避免exe文件过大
    // QResource::registerResource("./resource.rcc");
    Loading w;
    w.show();
    return a.exec();
}
