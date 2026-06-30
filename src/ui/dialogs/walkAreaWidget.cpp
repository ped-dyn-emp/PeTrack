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

#include "walkAreaWidget.h"

#include "calibration/walkAreaManager.h"
#include "petrack.h"
#include "ui/dialogs/pMessageBox.h"
#include "ui/helper/pdoublespinbox.h"
#include "ui/main-windows/view.h"
#include "util/wktParser.h"

#include <QApplication>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QTextEdit>
#include <QTimer>

WalkAreaWidget::WalkAreaWidget(QWidget *parent) : QWidget(parent), mUi(new Ui::WalkAreaWidget())
{
    mUi->setupUi(this);

    setFocusProxy(mUi->mVisibility);
    mUi->mPolygonTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    mConfirmationTimer = new QTimer(this);
    mConfirmationTimer->setInterval(3000); // reset deletion-confirmation after interval
    connect(mConfirmationTimer, &QTimer::timeout, this, &WalkAreaWidget::onConfirmationTimeout);

    connect(mUi->mWalkareaVisibilityToggle, &QCheckBox::toggled, this, &WalkAreaWidget::onWalkareaVisibilityToggled);
    connect(mUi->mObstacleVisibilityToggle, &QCheckBox::toggled, this, &WalkAreaWidget::onObstacleVisibilityToggled);
    connect(mUi->mVisibility, &QCheckBox::toggled, this, &WalkAreaWidget::onVisibilityToggled);
    connect(mUi->mEditModeToggle, &QCheckBox::toggled, this, &WalkAreaWidget::onEditModeToggled);
    connect(mUi->mExportButton, &QPushButton::clicked, this, &WalkAreaWidget::onExportWKT);
    connect(mUi->mImportButton, &QPushButton::clicked, this, &WalkAreaWidget::onImportWKT);
}

WalkAreaWidget::~WalkAreaWidget()
{
    if(mMainWindow)
    {
        mMainWindow->removeEventFilter(this);
    }
}

void WalkAreaWidget::setManager(WalkAreaManager *mgr)
{
    if(mManager)
    {
        disconnect(mManager, nullptr, this, nullptr);
    }
    mManager = mgr;
    if(!mManager)
    {
        return;
    }

    connect(mManager, &WalkAreaManager::validationChanged, this, &WalkAreaWidget::onValidationChanged);
    connect(mManager, &WalkAreaManager::geometryChanged, this, &WalkAreaWidget::onGeometryChanged);
    connect(
        mManager,
        &WalkAreaManager::visibilityChanged,
        this,
        [this](bool visible, bool walkableVisible, bool obstacleVisible)
        {
            mUi->mVisibility->blockSignals(true);
            mUi->mWalkareaVisibilityToggle->blockSignals(true);
            mUi->mObstacleVisibilityToggle->blockSignals(true);

            mUi->mVisibility->setChecked(visible);
            mUi->mWalkareaVisibilityToggle->setChecked(walkableVisible);
            mUi->mObstacleVisibilityToggle->setChecked(obstacleVisible);

            mUi->mVisibility->blockSignals(false);
            mUi->mWalkareaVisibilityToggle->blockSignals(false);
            mUi->mObstacleVisibilityToggle->blockSignals(false);
        });
    connect(
        mManager,
        &WalkAreaManager::editModeChanged,
        this,
        [this](bool v)
        {
            mUi->mEditModeToggle->setChecked(v);
            mUi->mStatusLabel->setText(v ? "edit mode: double-click to add points" : "paused: not in edit mode");
        });
    connect(mManager, &WalkAreaManager::drawingTypeChanged, this, &WalkAreaWidget::onDrawingTypeChanged);

    if(mMainWindow)
    {
        mMainWindow->installEventFilter(this);
    }

    mUi->mVisibility->setChecked(mManager->isVisible());
    mUi->mWalkareaVisibilityToggle->setChecked(mManager->isWalkableVisible());
    mUi->mObstacleVisibilityToggle->setChecked(mManager->isObstacleVisible());
    mUi->mEditModeToggle->setChecked(mManager->isEditMode());
    mUi->mWalkareaRadio->setChecked(mManager->getDrawingType() == walkarea::PolygonType::Walkable);
    mUi->mObstacleRadio->setChecked(mManager->getDrawingType() == walkarea::PolygonType::Obstacle);

    onGeometryChanged(); // initialize
}

void WalkAreaWidget::onWalkareaVisibilityToggled(bool checked)
{
    if(mManager)
    {
        mManager->setWalkableVisible(checked);
    }
}

void WalkAreaWidget::onObstacleVisibilityToggled(bool checked)
{
    if(mManager)
    {
        mManager->setObstacleVisible(checked);
    }
}

void WalkAreaWidget::onVisibilityToggled(bool checked)
{
    if(mManager)
    {
        mManager->setVisible(checked);
    }
}

void WalkAreaWidget::onEditModeToggled(bool checked)
{
    if(mManager)
    {
        mManager->setEditMode(checked);
    }
}

void WalkAreaWidget::onDrawingTypeChanged(walkarea::PolygonType type)
{
    mUi->mWalkareaRadio->setChecked(type == walkarea::PolygonType::Walkable);
    mUi->mObstacleRadio->setChecked(type == walkarea::PolygonType::Obstacle);
}

void WalkAreaWidget::updatePolygonList()
{
    if(!mManager)
    {
        return;
    }

    mUi->mPolygonTable->blockSignals(true);
    mPolygonIdToRow.clear();

    std::vector<std::pair<int, walkarea::Polygon>> allPolys;
    if(mManager->getWalkableArea().has_value())
    {
        allPolys.push_back({0, *mManager->getWalkableArea()});
    }
    for(const auto &kv : mManager->getObstacles())
    {
        allPolys.emplace_back(kv);
    }

    mUi->mPolygonTable->setRowCount(allPolys.size());
    mUi->mPolygonTable->setColumnCount(5);

    int row = 0;
    for(const auto &[id, poly] : allPolys)
    {
        mPolygonIdToRow[id] = row;

        auto *idItem = new QTableWidgetItem(QString::number(id));
        idItem->setFlags(idItem->flags() & ~Qt::ItemIsEditable);
        mUi->mPolygonTable->setItem(row, 0, idItem);

        auto *typeItem = new QTableWidgetItem(poly.type() == walkarea::PolygonType::Walkable ? "walkable" : "obstacle");
        typeItem->setFlags(typeItem->flags() & ~Qt::ItemIsEditable);
        mUi->mPolygonTable->setItem(row, 1, typeItem);

        auto *heightSpin = new PDoubleSpinBox(this);
        heightSpin->setDecimals(2);
        heightSpin->setSingleStep(0.01);
        heightSpin->setProperty("polygonId", id);
        heightSpin->setValue(poly.height());
        connect(
            heightSpin, QOverload<double>::of(&PDoubleSpinBox::valueChanged), this, &WalkAreaWidget::onHeightChanged);
        mUi->mPolygonTable->setCellWidget(row, 2, heightSpin);

        auto *checkboxContainer = new QWidget();
        auto *checkboxLayout    = new QHBoxLayout(checkboxContainer);
        auto *enabledCheckbox   = new QCheckBox(checkboxContainer);
        enabledCheckbox->setChecked(!poly.disabled());
        enabledCheckbox->setProperty("polygonId", id);
        connect(enabledCheckbox, &QCheckBox::toggled, this, &WalkAreaWidget::onTogglePolygonEnabled);
        checkboxLayout->addWidget(enabledCheckbox);
        checkboxLayout->setAlignment(Qt::AlignCenter);
        checkboxLayout->setContentsMargins(0, 0, 0, 0);
        mUi->mPolygonTable->setCellWidget(row, 3, checkboxContainer);

        auto *btnContainer = new QWidget();
        auto *btnLayout    = new QHBoxLayout(btnContainer);
        auto *delBtn       = new QPushButton("delete", btnContainer);
        delBtn->setMaximumWidth(80);
        delBtn->setProperty("polygonId", id);
        delBtn->setProperty("confirmMode", false);
        connect(delBtn, &QPushButton::clicked, this, &WalkAreaWidget::onRowDeleteClicked);
        btnLayout->addWidget(delBtn);
        btnLayout->setAlignment(Qt::AlignCenter);
        btnLayout->setContentsMargins(0, 0, 0, 0);
        mUi->mPolygonTable->setCellWidget(row, 4, btnContainer);

        row++;
    }

    mUi->mPolygonTable->blockSignals(false);

    // Dynamic height adjustment
    int rows = mUi->mPolygonTable->rowCount();
    int rowH = rows > 0 ? mUi->mPolygonTable->rowHeight(0) : mUi->mPolygonTable->verticalHeader()->defaultSectionSize();
    int hdrH = mUi->mPolygonTable->horizontalHeader()->height();
    int totalH = hdrH + rows * rowH + 2 * mUi->mPolygonTable->frameWidth();
    mUi->mPolygonTable->setMinimumHeight(totalH);
    mUi->mPolygonTable->setMaximumHeight(totalH);

    highlightInvalidRows();
}

void WalkAreaWidget::onValidationChanged()
{
    highlightInvalidRows();
    updateStatusLabel();
}

void WalkAreaWidget::highlightInvalidRows()
{
    if(!mManager)
    {
        return;
    }

    const auto invalidIds = mManager->getInvalidObstacleIds();

    // Update all rows
    for(int row = 0; row < mUi->mPolygonTable->rowCount(); ++row)
    {
        QTableWidgetItem *idItem = mUi->mPolygonTable->item(row, 0);
        if(!idItem)
        {
            continue;
        }

        int  polygonId = idItem->text().toInt();
        bool isInvalid = invalidIds.find(polygonId) != invalidIds.end();

        for(int col = 0; col < mUi->mPolygonTable->columnCount(); ++col)
        {
            QTableWidgetItem *item = mUi->mPolygonTable->item(row, col);
            if(!item)
            {
                continue;
            }
            if(isInvalid)
            {
                static const QColor invalidTint = QColor(255, 0, 0, 40); // semi-transparent red tint
                item->setBackground(QBrush(invalidTint));
            }
            else
            {
                item->setBackground(QBrush());
            }
        }
    }
}

void WalkAreaWidget::updateStatusLabel()
{
    const auto invalidIds = mManager->getInvalidObstacleIds();
    const bool isValid    = mManager->isValid();

    if(!isValid)
    {
        const int count = static_cast<int>(invalidIds.size());
        mUi->mStatusLabel->setText(
            QString("validation failed: %1 invalid polygon%2 highlighted.").arg(count).arg(count == 1 ? "" : "s"));
        mUi->mStatusLabel->setStyleSheet("QLabel { color: #cc0000; }");
    }
    else
    {
        mUi->mStatusLabel->setText("validation ok.");
        mUi->mStatusLabel->setStyleSheet("QLabel { color: #008800; }");
    }
}

void WalkAreaWidget::onExportWKT()
{
    if(!mManager || !mMainWindow)
    {
        return;
    }

    if(!mManager->isValid())
    {
        PCritical(
            this,
            "validation error",
            "cannot export: polygon validation failed.\n\ncheck the highlighted entries in red.");
        return;
    }

    // Build polygon list: walkable area + obstacles
    QVector<walkarea::Polygon> polygons;

    // Add walkable area
    if(mManager->getWalkableArea().has_value())
    {
        polygons.append(*mManager->getWalkableArea());
    }

    // Add obstacles
    for(const auto &kv : mManager->getObstacles())
    {
        if(!kv.second.disabled())
        {
            polygons.append(kv.second);
        }
    }

    auto    calib = mMainWindow->getExtrCalibration();
    QString wkt   = walkarea::WKTParser::generateWorldWKT(polygons, calib);

    if(wkt.isEmpty())
    {
        PCritical(this, "export error", "failed to generate wkt string.");
        return;
    }

    // Create dialog for displaying WKT
    QDialog dialog(this);
    dialog.setWindowTitle("wkt string - copy to clipboard");
    dialog.setModal(true);
    dialog.resize(600, 300);

    QVBoxLayout layout(&dialog);

    QLabel label("wkt representation of the walk area (polygon in world coord from pixel-transformation):", &dialog);
    layout.addWidget(&label);

    QTextEdit textEdit(&dialog);
    textEdit.setPlainText(wkt);
    textEdit.setReadOnly(true);
    textEdit.setLineWrapMode(QTextEdit::WidgetWidth);
    textEdit.selectAll();
    layout.addWidget(&textEdit);

    QHBoxLayout buttonLayout;

    QPushButton closeBtn("close", &dialog);
    connect(&closeBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    buttonLayout.addWidget(&closeBtn);

    layout.addLayout(&buttonLayout);

    dialog.exec();
}

void WalkAreaWidget::onImportWKT()
{
    if(!mManager || !mMainWindow)
    {
        return;
    }

    // Create import dialog
    QDialog dialog(this);
    dialog.setWindowTitle("import wkt string");
    dialog.setModal(true);
    dialog.resize(600, 350);

    QVBoxLayout layout(&dialog);

    QLabel warningLabel(
        "<b>warning:</b> importing will <b>overwrite all existing polygons</b> (walkable area and obstacles).",
        &dialog);
    warningLabel.setStyleSheet("QLabel { color: #cc6600; }");
    warningLabel.setWordWrap(true);
    layout.addWidget(&warningLabel);

    QLabel instructionLabel(
        "paste the wkt string below (single polygon string in world coordinates (meters)):", &dialog);
    instructionLabel.setWordWrap(true);
    layout.addWidget(&instructionLabel);

    QTextEdit textEdit(&dialog);
    textEdit.setPlaceholderText("paste wkt string here...");
    textEdit.setLineWrapMode(QTextEdit::WidgetWidth);
    layout.addWidget(&textEdit);

    QHBoxLayout buttonLayout;

    QPushButton cancelBtn("cancel", &dialog);
    connect(&cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);
    buttonLayout.addWidget(&cancelBtn);

    QPushButton importBtn("import && overwrite", &dialog);
    connect(
        &importBtn,
        &QPushButton::clicked,
        [this, &dialog, &textEdit]()
        {
            QString wktInput = textEdit.toPlainText().trimmed();
            if(wktInput.isEmpty())
            {
                PWarning(&dialog, "empty input", "please paste a wkt string before importing.");
                return;
            }

            // Parse WKT
            auto                       corr = mMainWindow->getWorldImageCorrespondencePtr();
            QVector<walkarea::Polygon> polygons;
            polygons = walkarea::WKTParser::parseWorldWKT(wktInput, corr);

            if(polygons.isEmpty())
            {
                PWarning(&dialog, "no polygons found", "the wkt string contained no or incorrectly formatted polygons");
                return;
            }

            // Reset existing polygons
            mManager->reset();

            // Import polygons: first one is walkable, rest are obstacles
            bool first = true;
            for(const auto &poly : polygons)
            {
                if(first)
                {
                    mManager->addPolygon(poly, walkarea::PolygonType::Walkable);
                    first = false;
                }
                else
                {
                    mManager->addPolygon(poly, walkarea::PolygonType::Obstacle);
                }
            }
            dialog.accept();
        });
    buttonLayout.addWidget(&importBtn);

    layout.addLayout(&buttonLayout);

    dialog.exec();
}

void WalkAreaWidget::onGeometryChanged()
{
    bool hasWalkable = mManager->hasWalkableArea();
    mUi->mWalkareaRadio->setEnabled(!hasWalkable);
    mUi->mObstacleRadio->setEnabled(hasWalkable);

    if(hasWalkable && mUi->mWalkareaRadio->isChecked())
    {
        mUi->mObstacleRadio->setChecked(true);
    }
    updatePolygonList();
}

void WalkAreaWidget::onTogglePolygonEnabled(bool checked)
{
    if(!mManager)
    {
        return;
    }

    auto *checkbox = qobject_cast<QCheckBox *>(sender());
    if(!checkbox)
    {
        return;
    }
    int id = checkbox->property("polygonId").toInt();

    if(mManager->getWalkableArea().has_value() && mManager->getWalkableArea()->id() == id)
    {
        auto poly = *mManager->getWalkableArea();
        poly.setDisabled(!checked);
        mManager->updatePolygon(poly, true);
    }
    else // item is obstacle
    {
        const auto &obstacles = mManager->getObstacles();
        if(obstacles.find(id) != obstacles.end())
        {
            auto poly = obstacles.at(id);
            poly.setDisabled(!checked);
            mManager->updatePolygon(poly, true);
        }
    }
}

void WalkAreaWidget::onHeightChanged(double value)
{
    if(!mManager)
    {
        return;
    }
    auto *spin = qobject_cast<QDoubleSpinBox *>(sender());
    if(!spin)
    {
        return;
    }
    int id = spin->property("polygonId").toInt();

    if(mManager->getWalkableArea().has_value() && mManager->getWalkableArea()->id() == id)
    {
        auto poly = *mManager->getWalkableArea();
        poly.setHeight(value);
        mManager->updatePolygon(poly);
    }
    else // item is obstacle
    {
        const auto &obstacles = mManager->getObstacles();
        if(obstacles.find(id) != obstacles.end())
        {
            auto poly = obstacles.at(id);
            poly.setHeight(value);
            mManager->updatePolygon(poly);
        }
    }
}

void WalkAreaWidget::resetDeleteConfirmation()
{
    if(mConfirmationButton)
    {
        mConfirmationButton->setText("delete");
        mConfirmationButton->setStyleSheet("");
        mConfirmationButton->setProperty("confirmMode", false);
        mConfirmationButton = nullptr;
    }
    mConfirmationTimer->stop();
}

void WalkAreaWidget::onConfirmationTimeout()
{
    resetDeleteConfirmation();
}

void WalkAreaWidget::onRowDeleteClicked()
{
    if(!mManager)
    {
        return;
    }

    auto *btn = qobject_cast<QPushButton *>(sender());
    if(!btn)
    {
        return;
    }
    int  id          = btn->property("polygonId").toInt();
    bool confirmMode = btn->property("confirmMode").toBool();

    if(!confirmMode)
    {
        resetDeleteConfirmation();

        // First click: enter confirmation mode
        btn->setText("confirm?");
        btn->setStyleSheet("background-color: rgba(255, 0, 0, 40);"); // semi-transparent red tint
        btn->setProperty("confirmMode", true);
        mConfirmationButton = btn;
        mConfirmationTimer->start();
    }
    else
    {
        // Second click: actually delete
        mManager->removePolygon(id);
        mConfirmationTimer->stop();
        mConfirmationButton = nullptr;
        // Button will be recreated when table refreshes
    }
}

bool WalkAreaWidget::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj);
    if(!mManager || !mManager->isEditMode())
    {
        return false;
    }

    if(event->type() == QEvent::KeyPress)
    {
        // Only intercept keys if the view has focus
        if(!mMainWindow->getView()->hasFocus())
        {
            return false;
        }

        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        switch(keyEvent->key())
        {
            case Qt::Key_Backspace:
                mManager->selection().removeLastPoint();
                mMainWindow->updateImage();
                return true;

            case Qt::Key_Escape:
                mManager->selection().reset();
                mMainWindow->updateImage();
                return true;

            default:
                break;
        }
    }
    return false;
}
