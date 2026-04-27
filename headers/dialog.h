#ifndef DIALOG_H
#define DIALOG_H

#include <QWidget>

class Dialog : public QWidget
{
    Q_OBJECT
public:
    explicit Dialog(QWidget *parent = nullptr);

signals:
    void dialogFinished();
};

#endif // DIALOG_H
