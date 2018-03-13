#include <QApplication>
#include <QtDBus/QDBusConnection>

#include "src/ui/mainwindow.h"
#include "timetracker_adaptor.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow win;
    new TimetrackerAdaptor(&win);
    QDBusConnection conn = QDBusConnection::sessionBus();
    conn.registerObject("/TimeTracker", &win);
    conn.registerService("org.f5n.timetracker");

    return app.exec();
}
