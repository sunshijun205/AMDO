#include "mainwindow.h"
#include "theme.h"

#include <QApplication>
#include <QFont>

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication a(argc, argv);
    a.setOrganizationName(QStringLiteral("AMDO"));
    a.setApplicationName(QString::fromUtf8("飞机概念设计平台"));
    a.setApplicationDisplayName(QString::fromUtf8("飞机概念设计平台"));
    QFont font(QStringLiteral("Microsoft YaHei"));
    font.setPixelSize(13);
    a.setFont(font);
    a.setStyleSheet(Theme::styleSheet());

    MainWindow w;
    w.show();
    return QApplication::exec();
}
