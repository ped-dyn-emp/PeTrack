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

#include "YOLOMarkerWidget.h"

#include "importHelper.h"
#include "recognition.h"
#include "ui_YOLOMarkerWidget.h"

#include <QFileDialog>

YOLOMarkerWidget::YOLOMarkerWidget(QWidget *parent, Ui::YOLOMarkerWidget *ui) : QWidget(parent)
{
    mMainWindow = (class Petrack *) parent;
    if(!ui)
    {
        mUi = new Ui::YOLOMarkerWidget();
    }
    else
    {
        mUi = ui;
    }
    mUi->setupUi(this);
    setWindowFlags(Qt::Window);
    setWindowTitle("YOLO marker parameter");
    connect(
        mUi->confThreshold,
        qOverload<double>(&PDoubleSpinBox::valueChanged),
        this,
        &YOLOMarkerWidget::onConfThresholdValueChanged);
    connect(
        mUi->nmsThreshold,
        qOverload<double>(&PDoubleSpinBox::valueChanged),
        this,
        &YOLOMarkerWidget::onNmsThresholdValueChanged);
    connect(
        mUi->scoreThreshold,
        qOverload<double>(&PDoubleSpinBox::valueChanged),
        this,
        &YOLOMarkerWidget::onScoreThresholdValueChanged);
    connect(mUi->imageSize, qOverload<int>(&PSpinBox::valueChanged), this, &YOLOMarkerWidget::onImageSizeValueChanged);
    connect(mUi->selectModelButton, &QPushButton::clicked, this, &YOLOMarkerWidget::selectModelFile);
    connect(mUi->selectNamesButton, &QPushButton::clicked, this, &YOLOMarkerWidget::selectNamesFile);
}

/// store data in xml node
void YOLOMarkerWidget::setXML(QDomElement &elem)
{
    QDomElement subElem;

    subElem = (elem.ownerDocument()).createElement("PARAMS");
    subElem.setAttribute("CONF_THRESHOLD", mUi->confThreshold->value());
    subElem.setAttribute("NMS_THRESHOLD", mUi->nmsThreshold->value());
    subElem.setAttribute("SCORE_THRESHOLD", mUi->scoreThreshold->value());
    subElem.setAttribute("IMAGE_SIZE", mUi->imageSize->value());
    subElem.setAttribute("MODEL_FILE", getFileList(mYOLOMarkerOptions.modelFile));
    subElem.setAttribute("NAMES_FILE", getFileList(mYOLOMarkerOptions.namesFile));
    elem.appendChild(subElem);
}

/// read data from xml node
void YOLOMarkerWidget::getXML(QDomElement &elem)
{
    QDomElement subElem;

    subElem = elem.firstChildElement();
    if(subElem.tagName() == "PARAMS")
    {
        loadDoubleValue(subElem, "CONF_THRESHOLD", mUi->confThreshold);
        loadDoubleValue(subElem, "NMS_THRESHOLD", mUi->nmsThreshold);
        loadDoubleValue(subElem, "SCORE_THRESHOLD", mUi->scoreThreshold);
        loadIntValue(subElem, "IMAGE_SIZE", mUi->imageSize);
        setModelFile(getExistingFile(readQString(subElem, "MODEL_FILE")));
        setNamesFile(getExistingFile(readQString(subElem, "NAMES_FILE")));
    }
}

void YOLOMarkerWidget::setModelFile(QString filename)
{
    if(!filename.isEmpty())
    {
        mYOLOMarkerOptions.modelFile = filename;
        mUi->modelLabel->setText(QString(QFileInfo(filename).fileName()));
        initialize();
        notifyChanged();
    }
}

void YOLOMarkerWidget::setNamesFile(QString filename)
{
    if(!filename.isEmpty())
    {
        mYOLOMarkerOptions.namesFile = filename;
        mUi->namesLabel->setText(QString(QFileInfo(filename).fileName()));
    }
}

void YOLOMarkerWidget::selectModelFile()
{
    QString filename = QFileDialog::getOpenFileName(
        mMainWindow, tr("Select onnx file"), "", tr("onnx file (*.onnx);;All files (*.*)"));
    setModelFile(filename);
}

void YOLOMarkerWidget::selectNamesFile()
{
    QString filename = QFileDialog::getOpenFileName(
        mMainWindow, tr("Select names file"), "", tr("names file (*.names);;All files (*.*)"));
    setNamesFile(filename);
}

void YOLOMarkerWidget::initialize()
{
    QFile                    namesFile(mYOLOMarkerOptions.namesFile);
    std::vector<std::string> classes;
    if(namesFile.exists())
    {
        if(namesFile.open(QIODevice::ReadOnly))
        {
            QTextStream in(&namesFile);
            while(!in.atEnd())
            {
                QString line = in.readLine();
                classes.push_back(line.toStdString());
            }
            namesFile.close();
        }
        else
        {
            throw std::invalid_argument(
                QString("%1 is not a valid class names file").arg(namesFile.fileName()).toStdString());
        }
    }
    else
    {
        classes.push_back("Pedestrian");
    }
    mYOLOMarkerOptions.classList = classes;


    if(mYOLOMarkerOptions.modelFile.isEmpty())
    {
        throw std::invalid_argument("No model file provided");
    }
    QFile modelFile(mYOLOMarkerOptions.modelFile);
    if(!modelFile.exists())
    {
        throw std::invalid_argument(QString("File %1 does not exist").arg(modelFile.fileName()).toStdString());
    }
    try
    {
        mYOLOMarkerOptions.network = cv::dnn::readNet(mYOLOMarkerOptions.modelFile.toStdString());
    }
    catch(cv::Exception &e)
    {
        throw std::invalid_argument(
            QString("Provided model file %1 is not valid: %2").arg(modelFile.fileName(), e.what()).toStdString());
    }
}

void YOLOMarkerWidget::onConfThresholdValueChanged(double d)
{
    mYOLOMarkerOptions.confThreshold = d;
    notifyChanged();
}

void YOLOMarkerWidget::onNmsThresholdValueChanged(double d)
{
    mYOLOMarkerOptions.nmsThreshold = d;
    notifyChanged();
}

void YOLOMarkerWidget::onScoreThresholdValueChanged(double d)
{
    mYOLOMarkerOptions.scoreThreshold = d;
    notifyChanged();
}

void YOLOMarkerWidget::onImageSizeValueChanged(int i)
{
    mYOLOMarkerOptions.imageSize = i;
}

void YOLOMarkerWidget::notifyChanged()
{
    mMainWindow->setRecognitionChanged(true); // flag indicates that changes of recognition parameters happens
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }
}

YOLOMarkerWidget::~YOLOMarkerWidget()
{
    delete mUi;
}
