#include "forms/headers/tutorialdialog.h"
#include "ui_tutorialdialog.h"

tutorialDialog::tutorialDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::tutorialDialog)
{
    ui->setupUi(this);
}

tutorialDialog::~tutorialDialog()
{
    delete ui;
}
