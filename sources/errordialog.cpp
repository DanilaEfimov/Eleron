#include "ErrorDialog.h"

#include <QMessageBox>

void ErrorDialog::showError(const QString& message) {
    QMessageBox::critical(nullptr, "Ошибка", message, QMessageBox::Ok);
}
