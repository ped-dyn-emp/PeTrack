/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2024 Forschungszentrum Jülich GmbH, IAS-7
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

#include "extrinsicBox.h"

#include "extrCalibration.h"
#include "helper.h"
#include "importHelper.h"
#include "pGroupBox.h"
#include "ui_extrinsicBox.h"

#include <QDialogButtonBox>
#include <QDomElement>
#include <QMessageBox>
#include <QStyle>
#include <utility>

ExtrinsicBox::ExtrinsicBox(QWidget *parent, Ui::extr *ui, ExtrCalibration &extrCalib) :
    QWidget(parent), mUi(ui), mExtrCalibration(extrCalib)
{
    mUi->setupUi(this);
    setFocusProxy(mUi->rot1);
    // use default values from struct as default for UI
    setExtrinsicParameters(mParams);
}

ExtrinsicBox::ExtrinsicBox(QWidget *parent, ExtrCalibration &extrCalib) : ExtrinsicBox(parent, new Ui::extr, extrCalib)
{
}

void ExtrinsicBox::setEnabledExtrParams(bool enable)
{
    mUi->rot1->setEnabled(enable);
    mUi->rot2->setEnabled(enable);
    mUi->rot3->setEnabled(enable);
    mUi->trans1->setEnabled(enable);
    mUi->trans2->setEnabled(enable);
    mUi->trans3->setEnabled(enable);
    emit enabledChanged(enable);
}

const ExtrinsicParameters &ExtrinsicBox::getExtrinsicParameters() const
{
    return mParams;
}

void ExtrinsicBox::setExtrinsicParameters(const ExtrinsicParameters &params)
{
    setValue(mUi->trans1, params.trans1);
    setValue(mUi->trans2, params.trans2);
    setValue(mUi->trans3, params.trans3);
    setValue(mUi->rot1, params.rot1);
    setValue(mUi->rot2, params.rot2);
    setValue(mUi->rot3, params.rot3);
}

void ExtrinsicBox::on_extrCalibFetch_clicked()
{
    auto newCalib = mExtrCalibration.fetch2DPoints();
    if(newCalib)
    {
        setExtrinsicParameters(*newCalib);
    }
}

void ExtrinsicBox::on_coordLoad3DCalibPoints_clicked()
{
    auto newCalib = mExtrCalibration.openExtrCalibFile();
    if(newCalib)
    {
        setExtrinsicParameters(*newCalib);
    }
    emit extrinsicChanged();
}

void ExtrinsicBox::on_rot1_valueChanged(double newVal)
{
    mParams.rot1 = newVal;
    emit extrinsicChanged();
}

void ExtrinsicBox::on_rot2_valueChanged(double newVal)
{
    mParams.rot2 = newVal;
    emit extrinsicChanged();
}

void ExtrinsicBox::on_rot3_valueChanged(double newVal)
{
    mParams.rot3 = newVal;
    emit extrinsicChanged();
}

void ExtrinsicBox::on_trans1_valueChanged(double newVal)
{
    mParams.trans1 = newVal;
    emit extrinsicChanged();
}

void ExtrinsicBox::on_trans2_valueChanged(double newVal)
{
    mParams.trans2 = newVal;
    emit extrinsicChanged();
}

void ExtrinsicBox::on_trans3_valueChanged(double newVal)
{
    mParams.trans3 = newVal;
    emit extrinsicChanged();
}

void ExtrinsicBox::on_extrCalibSave_clicked()
{
    mExtrCalibration.saveExtrCalibPoints();
}

void ExtrinsicBox::on_extrCalibShowError_clicked()
{
    QString      out;
    QDialog      msgBox;
    QGridLayout *layout = new QGridLayout();
    msgBox.setLayout(layout);
    QLabel *tableView = new QLabel(&msgBox);
    layout->addWidget(tableView, 1, 1);
    QLabel *titel = new QLabel(&msgBox);
    titel->setText("<b>Reprojection error for extrinsic calibration:</b>");
    layout->addWidget(titel, 0, 1);

    if(!mExtrCalibration.getReprojectionError().isValid())
    {
        out = QString("No File for extrinsic calibration found!");
        tableView->setText(out);
    }
    else
    {
        out                    = QString("<table>"
                                         "<tr><th></th>"
                                         "<th>average   &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</th>"
                                         "<th>std. deviation                          &nbsp;</th>"
                                         "<th>variance  &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</th>"
                                         "<th>max       &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</th></tr>"
                                         "<tr><td>Point   height: &nbsp;&nbsp;            </td><td> %0 cm</td><td> %1 cm</td><td> %2 "
                                         "cm</td><td> %3 cm</td></tr>"
                                         "<tr><td>Default height: <small>[%12 cm]</small> </td><td> %4 cm</td><td> %5 cm</td><td> %6 "
                                         "cm</td><td> %7 cm</td></tr>"
                                         "<tr><td>Pixel    error: &nbsp;&nbsp;            </td><td> %8 px</td><td> %9 px</td><td> %10 "
                                         "px</td><td> %11 px</td></tr>"
                                         "</table>");
        const auto &reproError = mExtrCalibration.getReprojectionError().getData();
        for(double value : reproError)
        {
            if(value < 0)
            {
                out = out.arg("-");
            }
            else
            {
                out = out.arg(value);
            }
        }
        tableView->setText(out);
    }

    msgBox.setWindowTitle("PeTrack");
    QIcon   icon     = QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation);
    QLabel *infoIcon = new QLabel(&msgBox);
    int     iconSize = msgBox.style()->pixelMetric(QStyle::PM_MessageBoxIconSize, nullptr, &msgBox);
    infoIcon->setPixmap(icon.pixmap(iconSize, iconSize));
    layout->addWidget(infoIcon, 0, 0);
    QDialogButtonBox *ok = new QDialogButtonBox(QDialogButtonBox::Ok);
    layout->addWidget(ok, 2, 1);
    connect(ok, &QDialogButtonBox::clicked, &msgBox, &QDialog::close);
    msgBox.setFixedSize(msgBox.sizeHint());
    msgBox.exec();
}

void ExtrinsicBox::on_extrCalibShowPoints_clicked()
{
    QString     out_str;
    QTextStream out(&out_str);

    unsigned int i;

    out << "<table><tr><th>Nr.</th><th>3D.x</th><th>3D.y</th><th>3D.z</th><th>2D.x</th><th>2D.y</th></tr>" << Qt::endl;


    for(i = 0; i < std::max(mExtrCalibration.get3DList().size(), mExtrCalibration.get2DList().size()); ++i)
    {
        out << "<tr>";
        if(i < mExtrCalibration.get3DList().size())
        {
            out << "<td>[" << QString::number(i + 1, 'i', 0) << "]: </td><td>"
                << QString::number(mExtrCalibration.get3DList().at(i).x, 'f', 1) << "</td><td>"
                << QString::number(mExtrCalibration.get3DList().at(i).y, 'f', 1) << "</td><td>"
                << QString::number(mExtrCalibration.get3DList().at(i).z, 'f', 1) << "</td><td>";
        }
        else
        {
            out << "<td>-</td><td>-</td><td>-</td>";
        }
        if(i < mExtrCalibration.get2DList().size())
        {
            out << QString::number(mExtrCalibration.get2DList().at(i).x, 'f', 3) << "</td><td>"
                << QString::number(mExtrCalibration.get2DList().at(i).y, 'f', 3) << "</td>";
        }
        else
        {
            out << "<td>-</td><td>-</td>";
        }
        out << "</tr>" << Qt::endl;
    }
    out << "</table>" << Qt::endl;

    QMessageBox msgBox;
    msgBox.setWindowTitle("PeTrack");
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText("Currently loaded point correspondences<br />for extrinsic calibration:");
    msgBox.setInformativeText(out_str);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec();
}

/// returns whether the all attributes were consumed
bool ExtrinsicBox::getXml(QDomElement &subSubElem)
{
    if(subSubElem.tagName() == "EXTRINSIC_PARAMETERS")
    {
        ExtrinsicParameters params;

        params.rot1   = readDouble(subSubElem, "EXTR_ROT_1", 0);
        params.rot2   = readDouble(subSubElem, "EXTR_ROT_2", 0);
        params.rot3   = readDouble(subSubElem, "EXTR_ROT_3", 0);
        params.trans1 = readDouble(subSubElem, "EXTR_TRANS_1", 0);
        params.trans2 = readDouble(subSubElem, "EXTR_TRANS_2", 0);
        params.trans3 = readDouble(subSubElem, "EXTR_TRANS_3", -500);
        setExtrinsicParameters(params);

        if(subSubElem.hasAttribute("EXTERNAL_CALIB_FILE"))
        {
            mExtrCalibration.setExtrCalibFile(getExistingFile(readQString(subSubElem, "EXTERNAL_CALIB_FILE")));
            // mMainWindow->isLoading() is true; doesn't perform calibration -> ignore return
            mExtrCalibration.loadExtrCalibFile();
        }

        if(subSubElem.hasAttribute("IMMUTABLE_EXTRINSIC_BOX"))
        {
            if(this->parent())
            {
                auto *parent = dynamic_cast<PGroupBox *>(this->parent()->parent());
                if(parent)
                {
                    parent->setImmutable(readBool(subSubElem, "IMMUTABLE_EXTRINSIC_BOX", false));
                }
            }
        }
    }
    return false;
}

void ExtrinsicBox::setXml(QDomElement &subSubElem) const
{
    subSubElem.setAttribute("EXTR_ROT_1", mUi->rot1->value());
    subSubElem.setAttribute("EXTR_ROT_2", mUi->rot2->value());
    subSubElem.setAttribute("EXTR_ROT_3", mUi->rot3->value());
    subSubElem.setAttribute("EXTR_TRANS_1", mUi->trans1->value());
    subSubElem.setAttribute("EXTR_TRANS_2", mUi->trans2->value());
    subSubElem.setAttribute("EXTR_TRANS_3", mUi->trans3->value());

    if(this->parent())
    {
        auto *parent = dynamic_cast<PGroupBox *>(this->parent()->parent());
        if(parent)
        {
            subSubElem.setAttribute("IMMUTABLE_EXTRINSIC_BOX", parent->isImmutable());
        }
    }
}

void ExtrinsicBox::loadCalibFile()
{
    auto newCalib = mExtrCalibration.loadExtrCalibFile();
    if(newCalib)
    {
        setExtrinsicParameters(*newCalib);
    }
    emit extrinsicChanged();
}


ExtrinsicBox::~ExtrinsicBox()
{
    delete mUi;
}
