#include "oasisreader.h"
#include "ui_oasisreader.h"

oasisReader::oasisReader(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::oasisReader)
{
    ui->setupUi(this);
}

oasisReader::~oasisReader()
{
    delete ui;
}
