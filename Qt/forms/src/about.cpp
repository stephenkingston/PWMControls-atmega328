#include "forms/headers/about.h"
#include "ui_about.h"

About::About(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::About)
{
    ui->setupUi(this);
    ui->myLabel->setTextFormat(Qt::RichText);
    ui->myLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    ui->myLabel->setOpenExternalLinks(true);

    QPixmap pix(":/images/copyleft.png");
    ui->logo->setScaledContents(true);
    ui->logo->setPixmap(pix);
}

About::~About()
{
    delete ui;
}
