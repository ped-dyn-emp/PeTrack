#ifndef PMESSAGEBOX_H
#define PMESSAGEBOX_H

#include <QDialog>
#include <QDialogButtonBox>

class QLabel;

// need to define these macros to get the file/line/function of caller
// Should be replaced with std::source_location once C++20 is used
#define PInformation(...) PMessageBox::information(__FILE__, __func__, __LINE__, __VA_ARGS__)
#define PWarning(...) PMessageBox::warning(__FILE__, __func__, __LINE__, __VA_ARGS__)
#define PCritical(...) PMessageBox::critical(__FILE__, __func__, __LINE__, __VA_ARGS__)
#define PQuestion(...) PMessageBox::question(__FILE__, __func__, __LINE__, __VA_ARGS__)


/**
 * @brief The PMessageBox class is similar to QMessageBox, but also allows usage in unsupervied scripts.
 *
 * The PMessageBox can be constructed like a QMessageBox. If the user is using PeTrack interactively,
 * it also opens a messagebox, like QMessageBox. But it also logs the message. If the user uses
 * PeTrack with an auto-option, the graphical boxes are not displayed anymore and only logging remains (TO BE DONE).
 */
class PMessageBox final : public QDialog
{
    Q_OBJECT

public:
    using StandardButton = QDialogButtonBox::StandardButton;
    using StandardButtons = QDialogButtonBox::StandardButtons;

    PMessageBox(QWidget *parent, const QString& title, const QString& msg, const QIcon& icon, const QString& informativeText = QString(), StandardButtons buttons = StandardButton::Ok, StandardButton defaultButton = StandardButton::NoButton);

    static int information(const char*file, const char*func, int line, QWidget *parent, const QString& title, const QString& text, StandardButtons buttons = StandardButton::Ok, StandardButton defaultButton = StandardButton::NoButton);
    static int warning(const char*file, const char*func, int line, QWidget *parent, const QString& title, const QString& text, StandardButtons buttons = StandardButton::Ok, StandardButton defaultButton = StandardButton::NoButton);
    static int critical(const char*file, const char*func, int line, QWidget *parent, const QString& title, const QString& text, StandardButtons buttons = StandardButton::Ok, StandardButton defaultButton = StandardButton::NoButton);
    [[nodiscard]] static int question(const char*file, const char*func, int line, QWidget *parent, const QString& title, const QString& text, StandardButtons buttons = (StandardButton::Yes | StandardButton::No), StandardButton defaultButton = StandardButton::NoButton);

private:
    static void setMinimumWidth(QLabel *textLabel);
};

#endif // PMESSAGEBOX_H
