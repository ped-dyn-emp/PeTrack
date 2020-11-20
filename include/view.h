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

#ifndef VIEW_H
#define VIEW_H

#include <QFrame>
#include <QGraphicsView>
#include <QSlider>
#include <QKeyEvent>


class Petrack;
class QLabel;
class QToolButton;
class ViewWidget;

class GraphicsView : public QGraphicsView
{
    Q_OBJECT // here: for emitting a signal (you need it also for connecting signals with slots)
             // must stand in .h file so qmake generates makefile where moc uses header

private:
    ViewWidget *mViewWidget;

public:
    GraphicsView(ViewWidget *viewWidget);

    void wheelEvent(QWheelEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    //void mouseReleaseEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

signals:
    void mouseDoubleClick();
    void mouseShiftControlDoubleClick(QPointF pos);
    void mouseShiftDoubleClick(QPointF pos);
    void mouseControlDoubleClick(QPointF pos);
    void mouseRightDoubleClick(QPointF pos, int direction);
    void mouseMiddleDoubleClick(int direction);
    void mouseShiftWheel(int delta);
    void colorSelected();
    void setColorEvent();
    //void mouseRightClick(QPointF pos);
};

// -------------------------------

class ViewWidget : public QFrame
{
    Q_OBJECT
public:
    ViewWidget(QWidget *parent = 0);

    GraphicsView *view() const;

    void resetView();

    void fitInView();

    void fitInROI(QRectF rect);

    void hideControls(bool hide);

    inline int getZoomLevel()
    {
        return mZoomSlider->value();
    }
    inline void setZoomLevel(int l)
    {
        mZoomSlider->setValue(l);
    }
    inline int getRotateLevel()
    {
        return mRotateSlider->value();
    }
    inline void setRotateLevel(int l)
    {
        mRotateSlider->setValue(l);
    }

public slots:
    void zoomIn(int i = 1);
    void zoomOut(int i = 1);

private slots:
    void setupMatrix();

    void rotateLeft();
    void rotateRight();
    void hideShowControls();

private:
    Petrack *mMainWindow;
    GraphicsView *mGraphicsView;
    QToolButton *hideShowControlsButton;
    QSlider *mZoomSlider;
    QSlider *mRotateSlider;
};

#endif
