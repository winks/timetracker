#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDateTime>
#include <QDebug>
#include <QIcon>
#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QTimer>

#define SECS_PER_DAY 86400
#define SECS_PER_HOUR 3600
#define SECS_PER_MIN 60

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void startTracking();
    void stopTracking();

private slots:
    void tick();
    void toggleWindow(QSystemTrayIcon::ActivationReason reason);
    void quitApp();

private:
    void updateStuff();
    QString formatTime(const qint64 & seconds);
    qint64 getElapsedSeconds();

private:
    Ui::MainWindow *ui;
    QSystemTrayIcon * trayIcon;
    QMenu * trayMenu;
    QAction * aElapsed;
    QAction * aQuit;
    QAction * aSep1;
    QAction * aSep2;
    QAction * aStart;
    QAction * aStop;
    QTimer * timer;

    QIcon iconDefault;
    QIcon iconGreen;
    bool isTracking;
    QDateTime lastStarted;
    QDateTime lastTick;
    qint64 timeElapsed;
};

#endif // MAINWINDOW_H
