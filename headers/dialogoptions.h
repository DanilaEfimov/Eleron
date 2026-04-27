#ifndef DIALOGOPTIONS_H
#define DIALOGOPTIONS_H

#include <QWidget>

namespace Ui {
class DialogOptions;
}

class DialogOptions : public QWidget
{
    Q_OBJECT

public:
    explicit DialogOptions(QWidget *parent = nullptr);
    ~DialogOptions();

private:
    Ui::DialogOptions *ui;
};

#endif // DIALOGOPTIONS_H
