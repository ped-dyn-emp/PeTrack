/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2022 Forschungszentrum Jülich GmbH, IAS-7
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
 * along with this program.  If not, see <https://cdwww.gnu.org/licenses/>.
 */

#ifndef FILTERBEFOREBOX_H
#define FILTERBEFOREBOX_H

#include <QGroupBox>

namespace Ui
{
class FilterBeforeBox;
}
class QDomElement;

struct FilterSettings
{
    bool useBrightContrast = false;
    bool useBorder         = false;
    bool useBackground     = false;
    bool useSwap           = false;
    bool useSwapH          = false;
    bool useSwapV          = false;
};

class FilterBeforeBox : public QGroupBox
{
    Q_OBJECT

public:
    explicit FilterBeforeBox(QWidget *parent);
    FilterBeforeBox(const FilterBeforeBox &)            = delete;
    FilterBeforeBox(FilterBeforeBox &&)                 = delete;
    FilterBeforeBox &operator=(const FilterBeforeBox &) = delete;
    FilterBeforeBox &operator=(FilterBeforeBox &&)      = delete;
    ~FilterBeforeBox() override;

    int  getFilterBorderSize() const;
    void setFilterBorderSizeMin(int i);
    void setFilterBorderSizeMax(int i);
    bool isFilterBgChecked() const;
    bool isFilterBgDeleteTrjChecked() const;
    int  getFilterBgDeleteNumber() const;

    void toggleBackgroundUi(Qt::CheckState state);

    void setFilterSettings(const FilterSettings &settings);

    bool getXmlSub(QDomElement &subSubElem);
    void setXml(QDomElement &subElem, QColor bgColor, const QString &bgFilename) const;

private:
    Ui::FilterBeforeBox *mUi;
    bool                 mShowBackgroundCache;
};

#endif // FILTERBEFOREBOX_H
