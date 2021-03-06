#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "src/ui/aboutdialog.h"
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QIcon>
#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlQueryModel>
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
    void resetTracking();
    void toggleWindow();
    int getElapsedSeconds();

private slots:
    void tick();
    void backup();
    void trayClicked(QSystemTrayIcon::ActivationReason reason);
    void changeEvent(QEvent * event);
    void quitApp();
    void toggleTracking();
    void showAboutDialog();

private:
    void newEntry();
    void setupDB();
    void updateDB();
    void updateGUI();
    void updateModel();
    QString formatTime(const qint64 & seconds);

private:
    int tickTimerDuration = 1000;
    int backupTimerDuration = 60000;
    int startupTooltipDuration = 1000;
    QString defaultProject = QString("default");

    Ui::MainWindow *ui;

    QIcon iconDefault;
    QIcon iconGreen;
    QSystemTrayIcon * trayIcon;

    QMenu * trayMenu;
    QAction * aTrayElapsed;
    QAction * aTrayQuit;
    QAction * aTraySep1;
    QAction * aTraySep2;
    QAction * aTrayShow;
    QAction * aTrayStart;
    QAction * aTrayStop;

    QMenu * mainMenu;
    QAction * aTopMainQuit;
    QAction * aTopMainSep1;
    QAction * aTopMainShow;
    QAction * aTopMainStart;
    QAction * aTopMainStop;

    QMenu * helpMenu;
    QAction * aTopHelpAbout;

    QTimer * tickTimer;
    QTimer * backupTimer;

    QSqlDatabase db;
    QSqlQueryModel * model;
    int dbId;

    bool isTracking;
    qint64 timeElapsed;
    QDateTime lastStarted;
    QDateTime lastTick;

    AboutDialog winAbout;
};

#endif // MAINWINDOW_H
