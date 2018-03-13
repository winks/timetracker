#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    iconDefault(":/images/icon_default.png"),
    iconGreen(":/images/icon_green.png"),
    trayIcon(new QSystemTrayIcon(iconDefault)),
    trayMenu(new QMenu),
    mainMenu(new QMenu("&Main")),
    helpMenu(new QMenu("&Help")),
    db(QSqlDatabase::addDatabase("QSQLITE")),
    model(new QSqlQueryModel),
    dbId(0),
    isTracking(false),
    timeElapsed(0)
{
    ui->setupUi(this);
    this->setWindowIcon(iconDefault);

    ui->lblElapsed->setText("Elapsed time (s):");
    ui->lcdElapsed->setDigitCount(5);
    ui->lcdElapsed->setDecMode();
    ui->btnStartStop->setText("Start");
    ui->btnReset->setText("Reset");
    ui->btnReset->setEnabled(false);
    ui->lblProject->setText("Project:");
    ui->inputProject->setText(defaultProject);
    ui->tblHistory->setSortingEnabled(true);
    ui->tblHistory->setCornerButtonEnabled(false);

    ui->topMenu->addMenu(mainMenu);
    aTopMainStart = mainMenu->addAction(iconGreen, QString("St&art"));
    aTopMainStop  = mainMenu->addAction(iconDefault, QString("St&op"));
    aTopMainSep1  = mainMenu->addSeparator();
    aTopMainShow  = mainMenu->addAction(QString("&Show"));
    aTopMainQuit  = mainMenu->addAction(QString("&Quit"));

    ui->topMenu->addMenu(helpMenu);
    aTopHelpAbout = helpMenu->addAction(QString("&About"));

    aTrayStart = trayMenu->addAction(iconGreen, QString("St&art"));
    aTrayStop  = trayMenu->addAction(iconDefault, QString("St&op"));
    aTraySep1  = trayMenu->addSeparator();
    aTrayShow  = trayMenu->addAction(QString("&Show"));
    aTrayQuit  = trayMenu->addAction(QString("&Quit"));

    aTraySep2  = new QAction;
    aTrayElapsed = new QAction;
    aTrayStop->setEnabled(false);
    aTopMainStop->setEnabled(false);

    tickTimer = new QTimer(this);
    tickTimer->start(tickTimerDuration);
    backupTimer = new QTimer(this);

    connect(tickTimer,  SIGNAL(timeout()),   this, SLOT(tick()));
    connect(backupTimer,SIGNAL(timeout()),   this, SLOT(backup()));

    connect(aTrayStart,    SIGNAL(triggered()), this, SLOT(startTracking()));
    connect(aTrayStop,     SIGNAL(triggered()), this, SLOT(stopTracking()));
    connect(aTrayShow,     SIGNAL(triggered()), this, SLOT(toggleWindow()));
    connect(aTrayQuit,     SIGNAL(triggered()), this, SLOT(close()));

    connect(aTopMainStart, SIGNAL(triggered()), this, SLOT(startTracking()));
    connect(aTopMainStop,  SIGNAL(triggered()), this, SLOT(stopTracking()));
    connect(aTopMainShow,  SIGNAL(triggered()), this, SLOT(toggleWindow()));
    connect(aTopMainQuit,  SIGNAL(triggered()), this, SLOT(close()));

    connect(aTopHelpAbout, SIGNAL(triggered()), this, SLOT(showAboutDialog()));

    connect(ui->btnStartStop, SIGNAL(clicked(bool)), this, SLOT(toggleTracking()));
    connect(ui->btnReset,     SIGNAL(clicked(bool)), this, SLOT(resetTracking()));
    connect(trayIcon,         SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(trayClicked(QSystemTrayIcon::ActivationReason)));

    trayIcon->setContextMenu(trayMenu);
    trayIcon->setToolTip(QString("TimeTracker"));

    if (QSystemTrayIcon::isSystemTrayAvailable()) {
        trayIcon->setIcon(iconDefault);
        trayIcon->show();

        if (QSystemTrayIcon::supportsMessages()) {
            trayIcon->showMessage(QString("started timetracker"), QString(),
                                  QSystemTrayIcon::Information, startupTooltipDuration);
        }
    } else {
        qDebug() << QString("no tray");
        toggleWindow();
    }
    setupDB();
    updateModel();
}

MainWindow::~MainWindow()
{
    updateDB();
    delete model;
    db.close();
    delete backupTimer;
    delete tickTimer;
    delete aTopHelpAbout;
    delete helpMenu;
    delete aTopMainQuit;
    delete aTopMainSep1;
    delete aTopMainShow;
    delete aTopMainStart;
    delete aTopMainStop;
    delete mainMenu;
    delete aTrayElapsed;
    delete aTrayQuit;
    delete aTraySep1;
    delete aTraySep2;
    delete aTrayShow;
    delete aTrayStart;
    delete aTrayStop;
    delete trayMenu;
    delete trayIcon;
    delete ui;
}

void MainWindow::startTracking()
{
    if (isTracking) return;
    isTracking = true;
    if (timeElapsed <= 0) {
        newEntry();
    } else {
        updateDB();
    }

    QDateTime now = QDateTime::currentDateTimeUtc();
    qDebug() << QString("startTracking @ %1").arg(now.toString());
    lastStarted = now;
    lastTick = now;
    updateModel();
    updateGUI();
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
    updateDB();
    updateGUI();
}

void MainWindow::resetTracking()
{
    if (isTracking) return;
    if (timeElapsed <= 0) return;
    updateDB();
    timeElapsed = 0;
    updateGUI();
}

void MainWindow::updateModel()
{
    model->setQuery("SELECT project, elapsed, created, updated "
                    "FROM timetracker ORDER BY created ASC");
    //model->setHeaderData(0, Qt::Horizontal, "ID");
    model->setHeaderData(0, Qt::Horizontal, "Project");
    model->setHeaderData(1, Qt::Horizontal, "Elapsed");
    model->setHeaderData(2, Qt::Horizontal, "Created");
    model->setHeaderData(3, Qt::Horizontal, "Updated");
    ui->tblHistory->setModel(model);
    ui->tblHistory->scrollToBottom();
}

void MainWindow::updateDB()
{
    QString projectName = ui->inputProject->text().length() > 0 ? ui->inputProject->text() : defaultProject;
    QSqlQuery sql;
    sql.prepare("UPDATE timetracker SET elapsed = :elapsed, project = :project, updated = CURRENT_TIMESTAMP WHERE id = :id");
    sql.bindValue(":id", dbId);
    sql.bindValue(":project", projectName);
    sql.bindValue(":elapsed", getElapsedSeconds());
    bool rv = sql.exec();
    qDebug() << QString("SQL UPDATE: [r:%1] %2 @ %3").arg(rv).arg(getElapsedSeconds()).arg(QDateTime::currentDateTimeUtc().toString());
    updateModel();
}

void MainWindow::newEntry()
{
    QString projectName = ui->inputProject->text().length() > 0 ? ui->inputProject->text() : defaultProject;
    QSqlQuery sql;
    sql.prepare("INSERT INTO timetracker(project, elapsed, created, updated) "
                "VALUES(:project, 0, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP)");
    sql.bindValue(":project", projectName);
    bool rv = sql.exec();
    qDebug() << QString("SQL NEWROW [r:%1]").arg(rv);
    sql.exec("SELECT last_insert_rowid() FROM timetracker");
    while (sql.next()) {
        dbId = sql.value(0).toInt();
        qDebug() << QString("SQL ROWID: %1").arg(dbId);
        break;
    }
    updateModel();
}

void MainWindow::setupDB()
{
    db.setDatabaseName(QDir::homePath() + QDir::separator() + "timetracker.sqlite");
    db.open();
    QSqlQuery sql;
    bool rv = sql.exec("CREATE TABLE IF NOT EXISTS timetracker "
                       "(id integer primary key,"
                       "project text,"
                       "elapsed int,"
                       "created datetime,"
                       "updated datetime)");
    qDebug() << QString("SQL CREATE [r:%1]").arg(rv);
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
    if (getElapsedSeconds() % 10 == 0) qDebug() << QString("tick %1").arg(getElapsedSeconds());
    updateGUI();
}

void MainWindow::backup()
{
    if (!isTracking) {
        return;
    }
    qDebug() << QString("tickBackup");
    updateDB();
}

void MainWindow::trayClicked(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Context) {
        return;
    } else if (reason != QSystemTrayIcon::Trigger) {
        qDebug() << QString("received %1").arg(QString::number(reason));
        return;
    }
    if(this->isVisible()) {
        toggleWindow();
    } else {
        toggleWindow();
    }
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->QEvent::WindowStateChange) {
        if (isMinimized()) {
            toggleWindow();
        } else {
            //if (aShow) aShow->setText("&Hide");
        }
    }
}

void MainWindow::quitApp()
{
    this->close();
}

void MainWindow::toggleWindow()
{
    if (isMinimized()) {
        this->setWindowState(Qt::WindowNoState);
    }
    if (isVisible()) {
        this->hide();
        aTrayShow->setText("&Show");
        aTopMainShow->setText("&Show");
    } else {
        this->show();
        aTrayShow->setText("&Hide");
        aTopMainShow->setText("&Hide");
    }
}

void MainWindow::toggleTracking()
{
    if (isTracking) {
        stopTracking();
    } else {
        startTracking();
    }
}

void MainWindow::updateGUI()
{
    // based on current tracking status:
    // - set correct icons
    // - set start/stop menu items to enabled/disabled
    // - set button to enabled/disabled
    // - start backup timer
    if (isTracking) {
        trayIcon->setIcon(iconGreen);
        this->setWindowIcon(iconGreen);
        aTrayStop->setEnabled(true);
        aTrayStart->setEnabled(false);
        aTopMainStop->setEnabled(true);
        aTopMainStart->setEnabled(false);
        ui->btnStartStop->setText("Stop");
        ui->btnReset->setEnabled(false);
        ui->inputProject->setEnabled(false);
        if(!backupTimer->isActive()) {
            backupTimer->start(backupTimerDuration);
        }
    } else {
        trayIcon->setIcon(iconDefault);
        this->setWindowIcon(iconDefault);
        aTrayStop->setEnabled(false);
        aTrayStart->setEnabled(true);
        aTopMainStop->setEnabled(false);
        aTopMainStart->setEnabled(true);
        ui->btnStartStop->setText("Start");
        ui->btnReset->setEnabled(true);
        ui->inputProject->setEnabled(true);
        backupTimer->stop();
    }
    // update "elapsed" context menu item
    if (timeElapsed > 0) {
        QString elapsed = QString("elapsed: %1").arg(formatTime(getElapsedSeconds()));
        aTrayElapsed->setText(elapsed);
        if (trayMenu->actions().size() == 5) {
            trayMenu->insertAction(aTraySep1, aTrayElapsed);
            aTraySep2 = trayMenu->insertSeparator(aTrayElapsed);
        }
        trayMenu->update();

        ui->statusBar->showMessage(QString("Elapsed: %1").arg(formatTime(getElapsedSeconds())));
    }
    // update lcd display
    ui->lcdElapsed->display(QString::number(getElapsedSeconds()));
}

void MainWindow::showAboutDialog()
{
    winAbout.show();
}

QString MainWindow::formatTime(const qint64 & seconds_)
{
    QString rv;
    qint64 seconds = seconds_;
    qint64 tmp;
    bool needSpace = false;
    if (seconds >= SECS_PER_DAY) {
        tmp = (seconds - (seconds % SECS_PER_DAY)) / SECS_PER_DAY;
        rv += QString::number(tmp);
        rv += " d";
        seconds -= (tmp * SECS_PER_DAY);
        needSpace = true;
    }
    if (seconds >= SECS_PER_HOUR) {
        tmp = (seconds - (seconds % SECS_PER_HOUR)) / SECS_PER_HOUR;
        if (needSpace) {
            rv += " ";
            needSpace = false;
        }
        rv += QString::number(tmp);
        rv += " h";
        seconds -= (tmp * SECS_PER_HOUR);
    }
    if (seconds >= SECS_PER_MIN) {
        tmp = (seconds - (seconds % SECS_PER_MIN)) / SECS_PER_MIN;
        if (needSpace) {
            rv += " ";
            needSpace = false;
        }
        rv += QString::number(tmp);
        rv += "m";
        seconds -= (tmp * SECS_PER_MIN);
    }
    rv += QString::number(seconds);
    rv += "s";
    return rv;
}

int MainWindow::getElapsedSeconds()
{
    return timeElapsed / 1000;
}
