#include "pMessageBox.h"

#include <QGridLayout>
#include <QLabel>
#include <QStyle>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QApplication>
#include <QTextDocument>

#include "helper.h"

/**
 * @brief Constructs a PMessageBox
 *
 * @param parent pointer to parent widget
 * @param title title of the dialog window
 * @param msg message to display on the dialog
 * @param icon icon to display (e.g. yellow warning triangle)
 * @param informativeText longer text (or table via html/rich text) to show
 * @param buttons buttons for the user to click default: only Ok)
 * @param defaultButton button used when pressing enter
 */
PMessageBox::PMessageBox(QWidget *parent, const QString& title, const QString& msg, const QIcon& icon, const QString& informativeText, StandardButtons buttons, StandardButton defaultButton) : QDialog(parent, Qt::MSWindowsFixedSizeDialogHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint)
{
    QGridLayout* layout = new QGridLayout();
    setLayout(layout);

    QFont f = QApplication::font("QMessageBox");
    Qt::TextInteractionFlags flags(style()->styleHint(QStyle::SH_MessageBox_TextInteractionFlags, nullptr, this));

    QLabel *detailedText = new QLabel(this);
    if(!informativeText.isEmpty())
    {
        detailedText->setWordWrap(true);
        detailedText->setTextInteractionFlags(flags);
        detailedText->setFont(f);
        detailedText->setText(informativeText);
        layout->addWidget(detailedText, 1, 1);
    }

    QLabel* text = new QLabel(this);
    text->setWordWrap(true);
    text->setTextInteractionFlags(flags);
    text->setFont(f);
    text->setText(msg);
    layout->addWidget(text, 0,1);

    setWindowTitle(title);
    QLabel* infoIcon = new QLabel(this);
    int iconSize = style()->pixelMetric(QStyle::PM_MessageBoxIconSize, nullptr, this);
    infoIcon->setPixmap(icon.pixmap(iconSize, iconSize));
    layout->addWidget(infoIcon, 0, 0);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(buttons, this);
    layout->addWidget(buttonBox, 2, 1);
    if(defaultButton != StandardButton::NoButton){
        QPushButton *def = buttonBox->button(defaultButton);
        if(def){
            def->setAutoDefault(false);
            def->setDefault(true);
        }else{
            debout << "Warning: Given default button does is non-specified button" << std::endl;
        }

    }

    // return the clicked button; -1 if none was clicked
    connect(buttonBox, &QDialogButtonBox::clicked, this, [=](QAbstractButton *button){
        int ret = buttonBox->standardButton(button);
        if(ret == StandardButton::NoButton){
            this->done(-1);
        }else{
            this->done(ret);
        }
    });

    layout->setSpacing(20);

    setMinimumWidth(text);
    setMinimumWidth(detailedText);

    setFixedSize(sizeHint());
}

/**
 * @brief Opens a dialog with given title, text and buttons and also logs the message.
 *
 * This Method opens a modal dialog with given text and buttons. The
 * dialogue has an information-icon and is logged with info-level.
 *
 * The pressed button is returned, i.e. PMessageBox::StandardButton::Yes.
 *
 * @param parent Pointer to parent
 * @param title title of dialog window
 * @param text message to the user
 * @param buttons buttons to use (default: OK)
 * @param defaultButton button to press, when pressing enter
 * @return clicked button
 */
int PMessageBox::information(const char*file,
                             const char*func,
                             int line, QWidget *parent,
                             const QString &title,
                             const QString &text,
                             PMessageBox::StandardButtons buttons,
                             PMessageBox::StandardButton defaultButton)
{
    std::cout << func << " in " << file_name(file) << " line " << line << ": Info: " << text << std::endl;

    // if no GUI gets displayed and no button except OK is used (no return Value used)
    // Then only log it
    if(QGuiApplication::platformName() == "offscreen")
    {
        if(buttons & (~StandardButton::Ok))
        {
            throw std::runtime_error(
                QString("user-interaction demanded during offscreen mode with message\n%1")
                    .arg(text).toStdString()
            );
        }

        return StandardButton::NoButton;
    }

    QIcon icon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation);
    PMessageBox msg = PMessageBox(parent, title, text, icon, QString(), buttons, defaultButton);
    return msg.exec();
}

/**
 * @brief Opens a dialog with given title, text and buttons and also logs the message.
 *
 * This Method opens a modal dialog with given text and buttons. The
 * dialogue has an warning-icon and is logged with warning-level.
 *
 * The pressed button is returned, i.e. PMessageBox::StandardButton::Yes.
 *
 * @param parent Pointer to parent
 * @param title title of dialog window
 * @param text message to the user
 * @param buttons buttons to use (default: OK)
 * @param defaultButton button to press, when pressing enter
 * @return clicked button
 */
int PMessageBox::warning(const char* file,
                         const char* func,
                         int line,
                         QWidget *parent,
                         const QString &title,
                         const QString &text,
                         PMessageBox::StandardButtons buttons,
                         PMessageBox::StandardButton defaultButton)
{
    std::cout << func << " in " << file_name(file) << " line " << line << ": Warning: " << text << std::endl;

    // if no GUI gets displayed and no button except OK is used (no return Value used)
    // Then only log it
    if(QGuiApplication::platformName() == "offscreen")
    {
        if(buttons & (~StandardButton::Ok))
        {
            throw std::runtime_error(
                QString("user-interaction demanded during offscreen mode with message\n%1")
                    .arg(text).toStdString()
            );
        }

        return StandardButton::NoButton;
    }

    QIcon icon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxWarning);
    PMessageBox msg = PMessageBox(parent, title, text, icon, QString(), buttons, defaultButton);
    return msg.exec();
}

/**
 * @brief Opens a dialog with given title, text and buttons and also logs the message.
 *
 * This Method opens a modal dialog with given text and buttons. The
 * dialogue has an critical-icon and is logged with critical(error?)-level.
 *
 * The pressed button is returned, i.e. PMessageBox::StandardButton::Yes.
 *
 * @param parent Pointer to parent
 * @param title title of dialog window
 * @param text message to the user
 * @param buttons buttons to use (default: OK)
 * @param defaultButton button to press, when pressing enter
 * @return clicked button
 */
int PMessageBox::critical(const char*file,
                          const char*func,
                          int line,
                          QWidget *parent,
                          const QString &title,
                          const QString &text,
                          PMessageBox::StandardButtons buttons,
                          PMessageBox::StandardButton defaultButton)
{
    std::cout << func << " in " << file_name(file) << " line " << line << ": Critical: " << text << std::endl;

    // if no GUI gets displayed and no button except OK is used (no return Value used)
    // Then only log it
    if(QGuiApplication::platformName() == "offscreen")
    {
        if(buttons & (~StandardButton::Ok))
        {
            throw std::runtime_error(
                QString("user-interaction demanded during offscreen mode with message\n%1")
                    .arg(text).toStdString()
            );
        }

        return StandardButton::NoButton;
    }

    QIcon icon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxCritical);
    PMessageBox msg = PMessageBox(parent, title, text, icon, QString(), buttons, defaultButton);
    return msg.exec();
}

/**
 * @brief Opens a dialog with given title, text and buttons and returns chosen action.
 *
 * This Method opens a modal dialog with given text and buttons. The
 * Dialog defaults to a yes and no button and returns accordingly.
 *
 * The pressed button is returned, i.e. PMessageBox::StandardButton::Yes.
 *
 * @param parent Pointer to parent
 * @param title title of dialog window
 * @param text message to the user
 * @param buttons buttons to use (default: OK)
 * @param defaultButton button to press, when pressing enter
 * @return clicked button
 */
int PMessageBox::question(const char* /*file*/,
                          const char* /*func*/,
                          int /*line*/,
                          QWidget *parent,
                          const QString &title,
                          const QString &text,
                          PMessageBox::StandardButtons buttons,
                          PMessageBox::StandardButton defaultButton)
{
    QIcon icon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxQuestion);
    PMessageBox msg = PMessageBox(parent, title, text, icon, QString(), buttons, defaultButton);

    // no debout, since question **demands** user interaction; also no support for scripting to come
    if(QGuiApplication::platformName() == "offscreen")
    {
        throw std::runtime_error(
            QString("user-interaction demanded during offscreen mode with message\n%1")
                .arg(text).toStdString()
        );
    }

    return msg.exec();
}

void PMessageBox::setMinimumWidth(QLabel *textLabel)
{
    QTextDocument doc;
    if(Qt::mightBeRichText(textLabel->text())){
        doc.setHtml(textLabel->text());
    }else{
        doc.setPlainText(textLabel->text());
    }
    doc.setDefaultFont(textLabel->font());
    doc.setTextWidth(textLabel->fontMetrics().averageCharWidth()*120);

    textLabel->setMinimumWidth(static_cast<int>(doc.idealWidth()));
}
