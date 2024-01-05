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

#ifndef FILTERBEFOREBOX_H
#define FILTERBEFOREBOX_H

#include <QWidget>

namespace Ui
{
class FilterBeforeBox;
}
class QDomElement;
class BackgroundFilter;
class BrightContrastFilter;
class BorderFilter;
class SwapFilter;
class BackgroundItem;

struct FilterSettings
{
    bool useBrightContrast = false;
    bool useBorder         = false;
    bool useBackground     = false;
    bool useSwap           = false;
    bool useSwapH          = false;
    bool useSwapV          = false;
};

class FilterBeforeBox : public QWidget
{
    Q_OBJECT

public:
    explicit FilterBeforeBox(
        QWidget              *parent,
        BackgroundFilter     &bgFilter,
        BrightContrastFilter &brightContrastFilter,
        BorderFilter         &borderFilter,
        SwapFilter           &swapFilter,
        std::function<void()> updateImageCallback);
    FilterBeforeBox(const FilterBeforeBox &)            = delete;
    FilterBeforeBox(FilterBeforeBox &&)                 = delete;
    FilterBeforeBox &operator=(const FilterBeforeBox &) = delete;
    FilterBeforeBox &operator=(FilterBeforeBox &&)      = delete;
    ~FilterBeforeBox() override;

    void setBackgroundItem(BackgroundItem *item);

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

private slots:
    void on_filterBrightContrast_stateChanged(int i);
    void on_filterContrastParam_valueChanged(int i);
    void on_filterBrightParam_valueChanged(int i);
    void on_filterBorder_stateChanged(int i);
    void on_filterBorderParamSize_valueChanged(int i);
    void on_filterBorderParamCol_clicked();
    void on_filterBg_stateChanged(int i);
    void on_filterBgShow_stateChanged(int i);
    void on_filterBgUpdate_stateChanged(int i);
    void on_filterBgReset_clicked();
    void on_filterBgSave_clicked();
    void on_filterBgLoad_clicked();
    void on_filterSwap_stateChanged(int i);
    void on_filterSwapH_stateChanged(int i);
    void on_filterSwapV_stateChanged(int i);

private:
    Ui::FilterBeforeBox  *mUi;
    bool                  mShowBackgroundCache;
    std::function<void()> mUpdateImageCallback;
    BackgroundFilter     &mBgFilter;
    BrightContrastFilter &mBrightContrastFilter;
    BorderFilter         &mBorderFilter;
    SwapFilter           &mSwapFilter;
    BackgroundItem       *mBgItem;
};

#endif // FILTERBEFOREBOX_H
