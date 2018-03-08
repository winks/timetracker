#include <QApplication>
#include <QtDBus/QDBusConnection>

#include "mainwindow.h"
#include "timetracker_adaptor.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow win;
    new TimetrackerAdaptor(&win);
    QDBusConnection conn = QDBusConnection::sessionBus();
    conn.registerObject("/org/f5n/timetracker/TimeTracker", &win);
    conn.registerService("org.f5n.timetracker");

    return app.exec();
}
