#ifndef TUTORIALDIALOG_H
#define TUTORIALDIALOG_H

#include <QDialog>

namespace Ui {
class tutorialDialog;
}

class tutorialDialog : public QDialog
{
    Q_OBJECT

public:
    explicit tutorialDialog(QWidget *parent = nullptr);
    ~tutorialDialog();

private slots:

private:
    Ui::tutorialDialog *ui;
};

#endif // TUTORIALDIALOG_H
