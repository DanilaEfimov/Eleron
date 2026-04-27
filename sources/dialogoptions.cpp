#include "dialogoptions.h"
#include "ui_dialogoptions.h"

DialogOptions::DialogOptions(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DialogOptions)
{
    ui->setupUi(this);
}

DialogOptions::~DialogOptions()
{
    delete ui;
}
