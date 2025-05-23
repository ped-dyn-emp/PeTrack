/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2025 Forschungszentrum Jülich GmbH, IAS-7
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
#include "openMoCapDialog.h"

#include "analysePlot.h"
#include "moCapPersonMetadata.h"
#include "moCapSelectionWidget.h"
#include "ui_openMoCapDialog.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QScrollBar>

OpenMoCapDialog::OpenMoCapDialog(QWidget *parent, MoCapController &controller) :
    QDialog(parent), mUi(new Ui::OpenMoCapDialog), mController(controller), mParent(parent)
{
    mUi->setupUi(this);


    mMoCapSystems = QMap<QString, MoCapSystem>();
    setWindowFlags(windowFlags().setFlag(Qt::WindowContextHelpButtonHint, false));
    mMoCapSystems.insert("XSensC3D", MoCapSystem::XSensC3D);

    const auto loadedMetadata = mController.getAllMoCapPersonMetadata();
    for(const auto &metadata : loadedMetadata)
    {
        mUi->moCapSelections->layout()->addWidget(new MoCapSelectionWidget(this, mMoCapSystems, metadata));
    }

    connect(mUi->pushButtonOK, &QPushButton::clicked, this, &OpenMoCapDialog::clickedOk);
    connect(mUi->pushButtonCancel, &QPushButton::clicked, this, &OpenMoCapDialog::close);
    connect(mUi->btnAddSelection, &QPushButton::clicked, this, &OpenMoCapDialog::onBtnAddSelectionClicked);
    mUi->moCapSelections->setMinimumSize(mUi->moCapSelections->minimumSizeHint());
    // Scroll Area needs to be set manually; should respect minimum size of widget, but does not
    // add some extra width for the frame and scrollbar
    mUi->scrollArea->setMinimumWidth(
        mUi->moCapSelections->sizeHint().width() + 2 * mUi->scrollArea->frameWidth() +
        mUi->scrollArea->verticalScrollBar()->sizeHint().width());
    resize(mUi->mainLayout->sizeHint());
}


/**
 * @brief Sets FileAttributes of MoCapController and calls readMoCapFile
 *
 * This method is called by a Signal(Ok-Button) and closes the window if no errors occur.
 */
void OpenMoCapDialog::clickedOk()
{
    bool                             allDataIsValid = true;
    std::vector<MoCapPersonMetadata> metadata;

    for(int i = 0; i < mUi->moCapSelections->layout()->count(); ++i)
    {
        auto selectionWidget =
            dynamic_cast<const MoCapSelectionWidget *>(mUi->moCapSelections->layout()->itemAt(i)->widget());
        if(selectionWidget == nullptr || !selectionWidget->isFilledOut())
        {
            continue;
        }

        try
        {
            MoCapPersonMetadata currentMetadata = selectionWidget->getMetadata();
            metadata.push_back(currentMetadata);
        }
        catch(std::exception &e)
        {
            allDataIsValid = false;
            std::stringstream errormsg;
            errormsg << e.what() << "\n";
            QMessageBox::critical(mParent, "PeTrack", QString::fromStdString(errormsg.str()));
        }
    }


    if(allDataIsValid)
    {
        try
        {
            mController.readMoCapFiles(metadata);
            this->close();
        }
        catch(std::exception &e)
        {
            std::stringstream errormsg;
            errormsg << e.what() << "\n";
            QMessageBox::critical(mParent, "PeTrack", QString::fromStdString(errormsg.str()));
        }
    }
}


OpenMoCapDialog::~OpenMoCapDialog()
{
    delete mUi;
}

void OpenMoCapDialog::onBtnAddSelectionClicked()
{
    static QString lastSelectedDir = QDir::currentPath();

    std::stringstream extensions;
    extensions << "All MoCap File Types (";
    for(const auto &extension : moCapFileExtensions)
    {
        extensions << " *." << extension.second;
    }
    extensions << ")";
    auto selectedFiles = QFileDialog::getOpenFileNames(
        this, tr("Open C3D File"), lastSelectedDir, QString::fromStdString(extensions.str()));
    for(const auto &file : selectedFiles)
    {
        auto *widget = new MoCapSelectionWidget(this, mMoCapSystems);
        widget->setFileName(file);
        mUi->moCapSelections->layout()->addWidget(widget);
    }

    if(!selectedFiles.isEmpty())
    {
        lastSelectedDir = QFileInfo(selectedFiles[0]).absolutePath();
    }
}
