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

#ifndef CODEMARKERWIDGET_H
#define CODEMARKERWIDGET_H

#include <QtWidgets>
#include <opencv2/aruco.hpp>



#include "petrack.h"
#include "codeMarkerItem.h"
#include "recognition.h"
namespace Ui{
    class CodeMarker;
}



class CodeMarkerWidget: public QWidget
{
    Q_OBJECT

public:
    CodeMarkerWidget(QWidget *parent, reco::CodeMarkerOptions& opt, Ui::CodeMarker *mUi);

    // store data in xml node
    void setXml(QDomElement &elem);

    // read data from xml node
    void getXml(QDomElement &elem);

    bool showDetectedCandidates();

private slots:

    void on_showDetectedCandidates_stateChanged(int i);

    void on_moreInfosButton_clicked();
    void on_dictList_currentIndexChanged(int /*index*/);

private:
    void notifyChanged();
    void sendDetectorParams(reco::ArucoCodeParams params);

private slots:
    void readDetectorParams();
    void readDictListIndex();

private:
    Ui::CodeMarker *mUi;

    Petrack *mMainWindow;
    reco::CodeMarkerOptions &mCodeMarkerOpt;
};

reco::ArucoCodeParams packDetectorParams(const Ui::CodeMarker *ui);

#endif
