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

#ifndef VIEW_H
#define VIEW_H

#include "pSlider.h"
#include "personStorage.h"

#include <QFrame>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QSlider>

class Petrack;
class QLabel;
class QToolButton;
class ViewWidget;

class GraphicsView : public QGraphicsView
{
    // here: for emitting a signal (you need it also for connecting signals with slots)
    // must stand in .h file so qmake generates makefile where moc uses header
    Q_OBJECT

private:
    ViewWidget *mViewWidget;

public:
    GraphicsView(ViewWidget *viewWidget);

    void wheelEvent(QWheelEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override { emit dropped(event); }

signals:
    void mouseDoubleClick();
    void mouseCtrlAltDoubleClick(QPointF pos);
    void mouseShiftControlDoubleClick(QPointF pos);
    void mouseShiftDoubleClick(QPointF pos);
    void mouseControlDoubleClick(QPointF pos);
    void mouseRightDoubleClick(QPointF pos, PersonStorage::TrajectorySegment direction);
    void mouseMiddleDoubleClick(PersonStorage::TrajectorySegment direction);
    void mouseShiftWheel(int delta);
    void mouseCtrlWheel(int delta);
    void mouseAltPressed(QPointF pos);
    void mouseAltReleased(QPointF pos);
    void altReleased();
    void mouseAltMoved(QPointF pos);
    void colorSelected();
    void setColorEvent();
    void dropped(QDropEvent *event);
};

// -------------------------------

class ViewWidget : public QFrame
{
    Q_OBJECT
public:
    ViewWidget(QWidget *parent = nullptr);

    GraphicsView *view() const;

    void resetView();

    void fitInView();

    void fitInROI(QRectF rect);

    void hideControls(bool hide);

    inline int  getZoomLevel() { return mZoomSlider->value(); }
    inline void setZoomLevel(int l) { mZoomSlider->setValue(l); }
    inline int  getRotateLevel() { return mRotateSlider->value(); }
    inline void setRotateLevel(int l) { mRotateSlider->setValue(l); }
    void        setZoomLevelBlocked(int l);

public slots:
    void zoomIn();
    void zoomOut();

private slots:
    void setupMatrix();

    void rotateLeft();
    void rotateRight();
    void hideShowControls();

private:
    Petrack      *mMainWindow;
    GraphicsView *mGraphicsView;
    QToolButton  *hideShowControlsButton;
    PSlider      *mZoomSlider;
    PSlider      *mRotateSlider;
};

#endif
