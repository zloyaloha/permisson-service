#pragma once
#include <QDialog>
#include "hello_w.h"

class RegistrationDialog : public QDialog
{
    Q_OBJECT
public:
    explicit RegistrationDialog(QWidget *parent = nullptr);
    ~RegistrationDialog();
};
