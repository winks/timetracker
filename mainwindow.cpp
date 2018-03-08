#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    trayMenu(new QMenu),
    iconDefault(":/images/icon_default.png"),
    iconGreen(":/images/icon_green.png"),
    isTracking(false),
    timeElapsed(0),
    dbId(0)
{
    tickTimer = new QTimer(this);
    tickTimer->start(1000);
    backupTimer = new QTimer(this);
    ui->setupUi(this);
    ui->label->setText("Elapsed time (s):");
    ui->lcdNumber->setDigitCount(5);
    ui->lcdNumber->setDecMode();

    trayIcon = new QSystemTrayIcon(iconDefault);

    aStart = trayMenu->addAction(QString("St&art"));
    aStop  = trayMenu->addAction(QString("St&op"));
    aSep1  = trayMenu->addSeparator();
    aQuit  = trayMenu->addAction(QString("&Quit"));

    aSep2  = new QAction;
    aElapsed = new QAction;
    aStop->setEnabled(false);

    connect(aStart,     SIGNAL(triggered()), this, SLOT(startTracking()));
    connect(aStop,      SIGNAL(triggered()), this, SLOT(stopTracking()));
    connect(aQuit,      SIGNAL(triggered()), this, SLOT(close()));
    connect(tickTimer,  SIGNAL(timeout()),   this, SLOT(tick()));
    connect(backupTimer,SIGNAL(timeout()),   this, SLOT(backup()));
    connect(trayIcon,   SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(toggleWindow(QSystemTrayIcon::ActivationReason)));

    trayIcon->setContextMenu(trayMenu);

    if (QSystemTrayIcon::isSystemTrayAvailable()) {
        trayIcon->setIcon(iconDefault);
        trayIcon->show();

        if (QSystemTrayIcon::supportsMessages()) {
            trayIcon->showMessage(QString("started timetracker"), QString(),
                                 QSystemTrayIcon::Information, 1000);
        }
    } else {
        qDebug() << QString("no tray");
        this->show();
    }
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(QDir::homePath() + QDir::separator() + "timetracker.sqlite");
    db.open();
    QSqlQuery sql;
    bool rv = sql.exec("CREATE TABLE IF NOT EXISTS timetracker "
                       "(id integer primary key,"
                       "elapsed int,"
                       "created datetime,"
                       "updated datetime)");
    qDebug() << QString("SQL CREATE [r:%1]").arg(rv);
    rv = sql.exec("INSERT INTO timetracker(elapsed, created, updated) "
                  "VALUES(0, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP)");
    qDebug() << QString("SQL NEWROW [r:%1]").arg(rv);
    sql.exec("SELECT last_insert_rowid() FROM timetracker");
    while (sql.next()) {
        dbId = sql.value(0).toInt();
        qDebug() << QString("SQL ROWID: %1").arg(dbId);
        break;
    }
}

MainWindow::~MainWindow()
{
    updateDB();
    db.close();
    delete aElapsed;
    delete aQuit;
    delete aSep1;
    delete aSep2;
    delete aStart;
    delete aStop;
    delete trayIcon;
    delete trayMenu;
    delete ui;
}

void MainWindow::startTracking()
{
    if (isTracking) return;
    isTracking = true;

    QDateTime now = QDateTime::currentDateTimeUtc();
    qDebug() << QString("startTracking @ %1").arg(now.toString());
    lastStarted = now;
    lastTick = now;
    updateStuff();
}

void MainWindow::stopTracking()
{
    if (!isTracking) return;
    isTracking = false;

    QDateTime now = QDateTime::currentDateTimeUtc();
    qDebug() << QString("stopTracking @ %1").arg(now.toString());
    qint64 diff = lastTick.msecsTo(now);
    timeElapsed += diff;
    qDebug() << QString("stopTracking += %1 (%2)").arg(diff).arg(getElapsedSeconds());
    updateStuff();
    updateDB();
}

void MainWindow::updateDB()
{
    QSqlQuery sql;
    sql.prepare("UPDATE timetracker SET elapsed = :elapsed, updated = CURRENT_TIMESTAMP WHERE id = :id");
    sql.bindValue(":id", dbId);
    sql.bindValue(":elapsed", getElapsedSeconds());
    bool rv = sql.exec();
    qDebug() << QString("SQL UPDATE: [r:%1] %2 @ %3").arg(rv).arg(getElapsedSeconds()).arg(QDateTime::currentDateTimeUtc().toString());
}

void MainWindow::tick()
{
    if (!isTracking) {
        return;
    }
    QDateTime now = QDateTime::currentDateTimeUtc();
    qint64 diff = lastTick.msecsTo(now);
    lastTick = now;
    timeElapsed += diff;
    qDebug() << QString("tick %1").arg(getElapsedSeconds());
    updateStuff();
}

void MainWindow::backup()
{
    if (!isTracking) {
        return;
    }
    qDebug() << QString("tickBackup");
    updateDB();
}

void MainWindow::toggleWindow(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Context) {
        return;
    } else if (reason != QSystemTrayIcon::Trigger) {
        qDebug() << QString("received %1").arg(QString::number(reason));
        return;
    }
    if(this->isVisible()) {
        this->hide();
    } else {
        this->show();
    }
}

void MainWindow::quitApp()
{
    this->close();
}

void MainWindow::updateStuff()
{
    // set icon based on current tracking status
    if (isTracking) {
        trayIcon->setIcon(iconGreen);
        aStop->setEnabled(true);
        aStart->setEnabled(false);
        if(!backupTimer->isActive()) {
            backupTimer->start(10000);
        }
    } else {
        trayIcon->setIcon(iconDefault);
        aStop->setEnabled(false);
        aStart->setEnabled(true);
        backupTimer->stop();
    }
    // update "elapsed" context menu item
    if (timeElapsed > 0) {
        QString elapsed = QString("elapsed: %1").arg(formatTime(getElapsedSeconds()));
        aElapsed->setText(elapsed);
        if (trayMenu->actions().size() == 4) {
            trayMenu->insertAction(aSep1, aElapsed);
            aSep2 = trayMenu->insertSeparator(aElapsed);
        }
        trayMenu->update();
    }
    ui->lcdNumber->display(QString::number(getElapsedSeconds()));
}

QString MainWindow::formatTime(const qint64 & seconds_)
{
    QString rv;
    qint64 seconds = seconds_;
    qint64 tmp;
    if (seconds >= SECS_PER_DAY) {
        tmp = (seconds - (seconds % SECS_PER_DAY)) / SECS_PER_DAY;
        rv += QString::number(tmp);
        rv += " d";
        seconds -= (tmp * SECS_PER_DAY);
    }
    if (seconds >= SECS_PER_HOUR) {
        tmp = (seconds - (seconds % SECS_PER_HOUR)) / SECS_PER_HOUR;
        rv += QString::number(tmp);
        rv += " h";
        seconds -= (tmp * SECS_PER_HOUR);
    }
    if (seconds >= SECS_PER_MIN) {
        tmp = (seconds - (seconds % SECS_PER_MIN)) / SECS_PER_MIN;
        rv += QString::number(tmp);
        rv += "m";
        seconds -= (tmp * SECS_PER_MIN);
    }
    rv += QString::number(seconds);
    rv += "s";
    return rv;
}

qint64 MainWindow::getElapsedSeconds()
{
    return timeElapsed / 1000;
}
