#include "aboutdialog.h"
#include "ui_aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    ui->lblName->setText(APP_NAME);
    ui->lblVersion->setText("Version: ");
    ui->lblVersionText->setText(APP_VERSION);

    ui->btnClose->setText("Close");

    connect(ui->btnClose,  SIGNAL(clicked()), this, SLOT(close()));
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
