/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2026 Forschungszentrum Jülich GmbH, IAS-7
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

#ifndef YOLOMARKERWIDGET_H
#define YOLOMARKERWIDGET_H

#include <QWidget>
#include <petrack.h>

namespace Ui
{
class YOLOMarkerWidget;
}

class YOLOMarkerWidget : public QWidget
{
    Q_OBJECT

public:
    void                    selectModelFile();
    void                    setModelFile(QString filename);
    void                    setNamesFile(QString filename);
    void                    selectNamesFile();
    reco::YOLOMarkerOptions getYOLOMarkerOptions() { return mYOLOMarkerOptions; }
    void                    initialize();
    void                    setXML(QDomElement &elem);
    void                    getXML(QDomElement &elem);

    explicit YOLOMarkerWidget(QWidget *parent, Ui::YOLOMarkerWidget *mUi);
    ~YOLOMarkerWidget();
    YOLOMarkerWidget(const YOLOMarkerWidget &)       = delete;
    YOLOMarkerWidget &operator=(YOLOMarkerWidget)    = delete;
    YOLOMarkerWidget(const YOLOMarkerWidget &&)      = delete;
    YOLOMarkerWidget &operator=(YOLOMarkerWidget &&) = delete;

private:
    void notifyChanged();

private slots:
    void onConfThresholdValueChanged(double d);
    void onNmsThresholdValueChanged(double d);
    void onScoreThresholdValueChanged(double d);
    void onImageSizeValueChanged(int i);

private:
    Ui::YOLOMarkerWidget   *mUi;
    Petrack                *mMainWindow;
    reco::YOLOMarkerOptions mYOLOMarkerOptions;
};

#endif // YOLOMARKERWIDGET_H
