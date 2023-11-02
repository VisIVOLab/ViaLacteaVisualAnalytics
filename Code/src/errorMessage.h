#ifndef ERRORMESSAGE_H
#define ERRORMESSAGE_H

#include <QMessageBox>

/**
 * @brief pqWindowImage::throwError
 * A standardised error message that can be thrown from anywhere in the class.
 * @param text The error message
 * @param info More detailed information, such as the next step to take or
 *             steps that could resolve the issue.
 */
static void throwError(const QString &text, const QString &info)
{
    QMessageBox e;
    e.setIcon(QMessageBox::Critical);
    e.setWindowTitle("Error!");
    e.setText(text);
    e.setInformativeText(info);
    e.setStandardButtons(QMessageBox::Ok);
    e.exec();
}

#endif // ERRORMESSAGE_H
