#ifndef ERRORDIALOG_H
#define ERRORDIALOG_H

#include <QMessageBox>
#include <QString>

class ErrorDialog {
public:
    static void showError(const QString& message);
};

#endif // ERRORDIALOG_H
