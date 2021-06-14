/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2010-2020 Forschungszentrum Jülich GmbH,
 * Maik Boltes, Juliane Adrian, Ricardo Martin Brualla, Arne Graf, Paul Häger, Daniel Hillebrand,
 * Deniz Kilic, Paul Lieberenz, Daniel Salden, Tobias Schrödter, Ann Katrin Seemann
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <QFileDialog>

#include "analysePlot.h"
#include "openMoCapDialog.h"
#include "ui_openMoCapDialog.h"
#include "moCapPersonMetadata.h"

OpenMoCapDialog::OpenMoCapDialog(QWidget *parent, MoCapController &controller) :
        QDialog(parent),
        mUi(new Ui::OpenMoCapDialog),
        mController(controller),
        mParent(parent)
{
    constexpr int defaultSampleRate = 60;
    constexpr double offsetRange = 5;

    mUi->setupUi(this);
    mMoCapSystems = QMap<QString, MoCapSystem>();
    setWindowFlags(windowFlags().setFlag(Qt::WindowContextHelpButtonHint, false));
    mMoCapSystems.insert("XSensC3D", MoCapSystem::XSensC3D);
    mUi->cbInputSystem->addItems(QStringList(mMoCapSystems.keys()));
    mUi->cbInputSystem->setCurrentIndex(0);
    mUi->sampleRateSpinBox->setRange(1, 300);
    mUi->sampleRateSpinBox->setValue(defaultSampleRate);
    mUi->offSetSpinBox->setRange(-offsetRange, offsetRange);
    mUi->offSetSpinBox->setSingleStep(0.01);
    mUi->filePathLineEdit->setReadOnly(true);
    mUi->pushButtonOK->setDisabled(true);
    connect(mUi->pushButtonOK, &QPushButton::clicked, this, &OpenMoCapDialog::clickedOk);
    connect(mUi->pushButtonCancel, &QPushButton::clicked, this, &OpenMoCapDialog::close);
    connect(mUi->browseFileButton, &QPushButton::clicked, this, &OpenMoCapDialog::setFileName);
}

/**
 * @brief Opens QFileDialog and enables ok-Button if the selected filename isn't empty.
 *
 * This method is called by a Signal('browse File'-Button).
 * The selected Filename is shown(read-only) in the LineEdit-Field.
 */

void OpenMoCapDialog::setFileName(){
    std::stringstream extensionsString;
    extensionsString << "All MoCap File Types (";
    for(const auto &extension: moCapFileExtensions){
        extensionsString << " *." << extension.second;
    }
    extensionsString << ")";
    QString filename = QFileDialog::getOpenFileName(this, tr("Open C3D File"), QDir::currentPath(),
                                                                 QString::fromStdString(extensionsString.str()));
    mUi->filePathLineEdit->clear();
    mUi->filePathLineEdit->insert(filename);
    mUi->pushButtonOK->setDisabled(filename.isEmpty());
}

/**
 * @brief Sets FileAttributes of MoCapController and calls readMoCapFile
 *
 * This method is called by a Signal(Ok-Button) and closes the window if no errors occur.
 */
void OpenMoCapDialog::clickedOk() {
    QString filePath = mUi->filePathLineEdit->text();
    int samplerate = mUi->sampleRateSpinBox->value();
    double offset = mUi->offSetSpinBox->value();
    MoCapSystem system = mMoCapSystems.value(mUi->cbInputSystem->currentText());
    std::stringstream errormsg;
    try{
        std::vector<MoCapPersonMetadata> metadata;
        MoCapPersonMetadata currentMetadata(filePath.toStdString(), system, samplerate, offset);
        metadata.push_back(currentMetadata);
        mController.readMoCapFiles(metadata);
        this->close();
    }catch(std::exception &e){
        errormsg << e.what() << "\n";
        QMessageBox::critical(mParent, "PeTrack", QString::fromStdString(errormsg.str()));
    }
}


OpenMoCapDialog::~OpenMoCapDialog()
{
    delete mUi;
}
