#include "moCapSelectionWidget.h"

#include "openMoCapDialog.h"
#include "ui_moCapSelectionWidget.h"

#include <QFileDialog>

MoCapSelectionWidget::MoCapSelectionWidget(QWidget *parent, const QMap<QString, MoCapSystem> &moCapSystems) :
    QWidget(parent), mUi(new Ui::MoCapSelectionWidget), mMoCapSystems(moCapSystems)
{
    mUi->setupUi(this);

    constexpr int    defaultSampleRate = 60;
    constexpr double offsetRange       = 5;

    mUi->btnDelete->setIcon(QApplication::style()->standardIcon(QStyle::SP_TrashIcon));
    mUi->cbInputSystem->addItems(QStringList(moCapSystems.keys()));
    mUi->cbInputSystem->setCurrentIndex(0);
    mUi->sampleRateSpinBox->setRange(1, 300);
    mUi->sampleRateSpinBox->setValue(defaultSampleRate);
    mUi->offSetSpinBox->setRange(-offsetRange, offsetRange);
    mUi->offSetSpinBox->setSingleStep(0.01);

    connect(mUi->browseFileButton, &QPushButton::clicked, this, &MoCapSelectionWidget::setFileName);
    connect(mUi->btnDelete, &QPushButton::clicked, this, &MoCapSelectionWidget::deleteLater);
}

/**
 * @brief constructs a widget with the data from a Metadata-Object
 * @param parent Widget parent (Qt)
 * @param moCapSystems Map from QString in Combobox to MoCapSystem-Enum
 * @param metadata Metadata which should be represented by this widget
 */
MoCapSelectionWidget::MoCapSelectionWidget(
    QWidget *                         parent,
    const QMap<QString, MoCapSystem> &moCapSystems,
    const MoCapPersonMetadata &       metadata) :
    MoCapSelectionWidget(parent, moCapSystems)
{
    auto usedMoCapSystem = std::find(moCapSystems.begin(), moCapSystems.end(), metadata.getSystem());
    if(usedMoCapSystem != moCapSystems.end())
    {
        mUi->cbInputSystem->setCurrentText(usedMoCapSystem.key());
    }
    mUi->sampleRateSpinBox->setValue(metadata.getSamplerate());
    mUi->offSetSpinBox->setValue(metadata.getOffset());
    mUi->filePathLabel->setText(QString::fromStdString(metadata.getFilepath()));
    mFilledOut = true;
}

/**
 * @brief Let user select a MoCapFile
 *
 * This method opens a file dialog which enabled the user
 * to select a file fitting to the given MoCapSystem (e.g. c3d)
 *
 * If a file is selected, the widget counts as filled out.
 *
 * This method is called by a Signal('browse File'-Button).
 */
void MoCapSelectionWidget::setFileName()
{
    std::stringstream extensionsString;
    extensionsString << "All MoCap File Types (";
    for(const auto &extension : moCapFileExtensions)
    {
        extensionsString << " *." << extension.second;
    }
    extensionsString << ")";
    QString filename = QFileDialog::getOpenFileName(
        this, tr("Open C3D File"), QDir::currentPath(), QString::fromStdString(extensionsString.str()));
    mUi->filePathLabel->clear();
    mUi->filePathLabel->setText(filename);
    mFilledOut = !filename.isEmpty();
}

/**
 * @brief Returns contained info as Metadata-Object
 *
 * This method returns the values saved in this widget as
 * a MoCapPersonMetadata-Object.
 * @return Metadata with data from this widget
 */
MoCapPersonMetadata MoCapSelectionWidget::getMetadata() const
{
    return MoCapPersonMetadata(
        mUi->filePathLabel->text().toStdString(),
        mMoCapSystems[mUi->cbInputSystem->currentText()],
        mUi->sampleRateSpinBox->value(),
        mUi->offSetSpinBox->value());
}

bool MoCapSelectionWidget::isFilledOut() const
{
    return mFilledOut;
}

MoCapSelectionWidget::~MoCapSelectionWidget()
{
    delete mUi;
}

#include "moc_moCapSelectionWidget.cpp"
