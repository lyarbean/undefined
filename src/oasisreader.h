#ifndef OASISREADER_H
#define OASISREADER_H

#include <QMainWindow>

namespace Ui {
class oasisReader;
}

class oasisReader : public QMainWindow
{
    Q_OBJECT

public:
    explicit oasisReader(QWidget *parent = 0);
    ~oasisReader();

private:
    Ui::oasisReader *ui;
};

#endif // OASISREADER_H
