#include "registration_w.h"
#include <QLabel>
#include <QVBoxLayout>

RegistrationDialog::RegistrationDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Регистрация");

    QLabel *label = new QLabel("Окно регистрации", this);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(label);
    setLayout(layout);
}

RegistrationDialog::~RegistrationDialog()
{
}