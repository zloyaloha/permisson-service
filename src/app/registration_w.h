#pragma once
#include <QDialog>
#include "hello_w.h"

class RegistrationDialog : public QDialog
{
    Q_OBJECT
public:
    explicit RegistrationDialog(Ui::HelloWindow* main);
    ~RegistrationDialog();
public slots:
        // void RegistrationButtonClicked();
private:
    Ui::HelloWindow *ui;
};
