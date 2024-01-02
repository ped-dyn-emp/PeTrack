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

#include "view.h"

#include "control.h"
#include "petrack.h"
#include "vector.h"

#include <QtWidgets>
#include <cmath>

GraphicsView::GraphicsView(ViewWidget *viewWidget) : QGraphicsView()
{
    mViewWidget = viewWidget;

    setTransformationAnchor(AnchorUnderMouse); // for wheel
    setResizeAnchor(AnchorUnderMouse);         // for scroll?
}


void GraphicsView::wheelEvent(QWheelEvent *event)
{
    QPoint numDegrees = event->angleDelta() / 8;

    if(event->modifiers() == Qt::ShiftModifier)
    {
        // Check if horizontal scroll
        if(event->angleDelta().x() != 0) // warum orienttion?
        {
            emit mouseShiftWheel(numDegrees.x() / 15);
        }
        else
        {
            emit mouseShiftWheel(-numDegrees.y() / 15);
        }
    }
    else if(event->modifiers() == Qt::ControlModifier)
    {
        if(event->angleDelta().x() != 0)
        {
            emit mouseCtrlWheel(numDegrees.x() / 15);
        }
        else
        {
            emit mouseCtrlWheel(numDegrees.y() / 15);
        }
    }
    else
    {
        // Check if horizontal scroll
        if(event->angleDelta().x() != 0) // warum orienttion?
        {
            mViewWidget->zoomIn(numDegrees.x() / 2);
        }
        else
        {
            mViewWidget->zoomOut(numDegrees.y() / 2);
        }
    }
}

void GraphicsView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        if(event->modifiers() &
           Qt::ShiftModifier) // mit & genuegt, dass unter anderem shift gedrueckt wird (zb mit conrol)
        {
            if(event->modifiers() & Qt::ControlModifier) // mindestens shift und control gedrueckt
            {
                emit mouseShiftControlDoubleClick(mapToScene(event->pos()));
            }
            else
            {
                emit mouseShiftDoubleClick(mapToScene(event->pos())); // const QPoint & //const QPointF &pos
            }
        }
        else if(event->modifiers() & Qt::ControlModifier) // mit & genuegt, dass unter anderem control gedrueckt wird
                                                          // (zb mit shift)
        {
            if(event->modifiers().testFlag(Qt::AltModifier))
            {
                emit mouseCtrlAltDoubleClick(mapToScene(event->pos()));
            }
            else
            {
                emit mouseControlDoubleClick(mapToScene(event->pos()));
            }
        }
        else
        {
            emit mouseDoubleClick();
        }
    }
    else if(event->button() == Qt::RightButton)
    {
        if(event->modifiers() == Qt::ShiftModifier) // nur shift zugelassen ...
        {
            emit mouseRightDoubleClick(mapToScene(event->pos()), -1);
        }
        else if(event->modifiers() == Qt::ControlModifier)
        {
            emit mouseRightDoubleClick(mapToScene(event->pos()), 0);
        }
        else if(event->modifiers() == Qt::AltModifier)
        {
            emit mouseRightDoubleClick(mapToScene(event->pos()), 1);
        }
    }
    else if(event->button() == Qt::MiddleButton)
    {
        if(event->modifiers() == Qt::ShiftModifier) // nur shift zugelassen ...
        {
            emit mouseMiddleDoubleClick(PersonStorage::Direction::Previous);
        }
        else if(event->modifiers() == Qt::ControlModifier)
        {
            emit mouseMiddleDoubleClick(PersonStorage::Direction::Whole);
        }
        else if(event->modifiers() == Qt::AltModifier)
        {
            emit mouseMiddleDoubleClick(PersonStorage::Direction::Following);
        }
    }

    scene()->update(); // damit neu hinzugekommende trackpoint oder das loeschen von trackpoints sichtbar wird

    QGraphicsView::mouseDoubleClickEvent(event);
}

void GraphicsView::keyPressEvent(QKeyEvent *event)
{
    // forward the arrow keys for handling in petrack player
    switch(event->key())
    {
        case Qt::Key_Up:
            mViewWidget->parent()->event(event);
            break;
        case Qt::Key_Down:
            mViewWidget->parent()->event(event);
            break;
        case Qt::Key_Left:
            mViewWidget->parent()->event(event);
            break;
        case Qt::Key_Right:
            mViewWidget->parent()->event(event);
            break;
        default:
            QGraphicsView::keyPressEvent(event);
    }
}

void GraphicsView::keyReleaseEvent(QKeyEvent *event)
{
    switch(event->key())
    {
        case Qt::Key_Alt:
            emit altReleased();
            break;
    }
}

void GraphicsView::mousePressEvent(QMouseEvent *event)
{
    if(event->modifiers() & Qt::ShiftModifier)
    {
        emit setColorEvent();
    }
    else
    {
        emit colorSelected();
    }

    if(event->modifiers().testFlag(Qt::AltModifier) && !event->modifiers().testFlag(Qt::ControlModifier) &&
       event->button() == Qt::LeftButton)
    {
        emit mouseAltPressed(mapToScene(event->pos()));
    }
    QGraphicsView::mousePressEvent(event);
}

void GraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->modifiers().testFlag(Qt::AltModifier) && !event->modifiers().testFlag(Qt::ControlModifier))
    {
        emit mouseAltReleased(mapToScene(event->pos()));
    }
    QGraphicsView::mouseReleaseEvent(event);
}

void GraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    if(event->modifiers().testFlag(Qt::AltModifier))
    {
        emit mouseAltMoved(mapToScene(event->pos()));
    }
    else
    {
        QGraphicsView::mouseMoveEvent(event);
    }
}

//---------------------------------------------------------------------


ViewWidget::ViewWidget(QWidget *parent) : QFrame(parent)
{
    mMainWindow = (class Petrack *) parent;
    setContentsMargins(0, 0, 0, 0);
    mGraphicsView = new GraphicsView(this);
    mGraphicsView->setRenderHint(QPainter::Antialiasing, false);
    mGraphicsView->setDragMode(QGraphicsView::ScrollHandDrag); // RubberBandDrag

    int   size = style()->pixelMetric(QStyle::PM_ToolBarIconSize);
    QSize iconSize(size, size);

    // Hide|Show Button
    hideShowControlsButton = new QToolButton;
    hideShowControlsButton->setIcon(QPixmap(":/arrowRight"));
    hideShowControlsButton->setIconSize(iconSize);

    QVBoxLayout *sliderLayout = new QVBoxLayout;
    sliderLayout->addWidget(hideShowControlsButton);

    // Rotate slider
    QToolButton *rotateLeftIcon = new QToolButton;
    rotateLeftIcon->setAutoRepeat(true);
    rotateLeftIcon->setAutoRepeatInterval(33);
    rotateLeftIcon->setAutoRepeatDelay(100); // damit kurzes drucken nicht sofort mehrmals anschlaegt
    rotateLeftIcon->setIcon(QPixmap(":/rotleft"));
    rotateLeftIcon->setIconSize(iconSize);
    QToolButton *rotateRightIcon = new QToolButton;
    rotateRightIcon->setAutoRepeat(true);
    rotateRightIcon->setAutoRepeatInterval(33);
    rotateRightIcon->setAutoRepeatDelay(100); // damit kurzes drucken nicht sofort mehrmals anschlaegt
    rotateRightIcon->setIcon(QPixmap(":/rotright"));
    rotateRightIcon->setIconSize(iconSize);
    mRotateSlider = new QSlider;
    mRotateSlider->setMinimum(-180); //-360
    mRotateSlider->setMaximum(180);  // 360
    mRotateSlider->setValue(0);
    mRotateSlider->setTickPosition(QSlider::TicksBelow);

    // Rotate slider layout
    sliderLayout->addWidget(rotateRightIcon);
    sliderLayout->addWidget(mRotateSlider);
    sliderLayout->addWidget(rotateLeftIcon);

    // Zoom slider
    QToolButton *zoomInIcon = new QToolButton;
    zoomInIcon->setAutoRepeat(true);
    zoomInIcon->setAutoRepeatInterval(33);
    zoomInIcon->setAutoRepeatDelay(100); // damit kurzes drucken nicht sofort mehrmals anschlaegt
    zoomInIcon->setIcon(QPixmap(":/zoomin"));
    zoomInIcon->setIconSize(iconSize);
    QToolButton *zoomOutIcon = new QToolButton;
    zoomOutIcon->setAutoRepeat(true);
    zoomOutIcon->setAutoRepeatInterval(33);
    zoomOutIcon->setAutoRepeatDelay(100);
    zoomOutIcon->setIcon(QPixmap(":/zoomout"));
    zoomOutIcon->setIconSize(iconSize);
    mZoomSlider = new QSlider;
    mZoomSlider->setMinimum(0);
    mZoomSlider->setMaximum(500);
    mZoomSlider->setValue(250);
    mZoomSlider->setTickPosition(QSlider::TicksRight);

    // Zoom slider layout
    sliderLayout->addWidget(zoomInIcon);
    sliderLayout->addWidget(mZoomSlider);
    sliderLayout->addWidget(zoomOutIcon);

    QGridLayout *topLayout = new QGridLayout;
    topLayout->setMargin(0);
    topLayout->setSpacing(2);
    topLayout->addWidget(mGraphicsView, 0, 0);
    topLayout->addLayout(sliderLayout, 0, 1);
    setLayout(topLayout);

    connect(mZoomSlider, SIGNAL(valueChanged(int)), this, SLOT(setupMatrix()));
    connect(mRotateSlider, SIGNAL(valueChanged(int)), this, SLOT(setupMatrix()));
    connect(hideShowControlsButton, SIGNAL(clicked()), this, SLOT(hideShowControls()));
    connect(rotateLeftIcon, SIGNAL(clicked()), this, SLOT(rotateLeft()));
    connect(rotateRightIcon, SIGNAL(clicked()), this, SLOT(rotateRight()));
    connect(zoomInIcon, SIGNAL(clicked()), this, SLOT(zoomIn()));
    connect(zoomOutIcon, SIGNAL(clicked()), this, SLOT(zoomOut()));

    setupMatrix();
}

GraphicsView *ViewWidget::view() const
{
    return mGraphicsView;
}

void ViewWidget::resetView()
{
    mZoomSlider->setValue(250);
    mRotateSlider->setValue(0);
    setupMatrix();
    mGraphicsView->ensureVisible(QRectF(0, 0, 0, 0));
}

void ViewWidget::fitInView()
{
    mGraphicsView->fitInView(
        mGraphicsView->sceneRect(),
        Qt::KeepAspectRatio); // Qt::KeepAspectRatioByExpanding wuerde nur in eine dimension passend machen
    // doesnt work: mGraphicsView->fitInView(mGraphicsView->sceneRect(), Qt::KeepAspectRatio); // two times, while in
    // the first run possibly the scrollbars have been there for calculation
    QTransform matrix = mGraphicsView->transform();
    // calculates the sclaing of matrix
    // see http://www.robertblum.com/articles/2005/02/14/decomposing-matrices for rotation out of matrix
    double scale = Vec2F(matrix.m11(), matrix.m21()).length();
    mZoomSlider->setValue((int) (250. + 50. * log(scale) / log(2.)));
}

void ViewWidget::fitInROI(QRectF rect)
{
    mGraphicsView->fitInView(
        rect, Qt::KeepAspectRatio); // Qt::KeepAspectRatioByExpanding wuerde nur in eine dimension passend machen
    // doesnt work: mGraphicsView->fitInView(mGraphicsView->sceneRect(), Qt::KeepAspectRatio); // two times, while in
    // the first run possibly the scrollbars have been there for calculation
    QTransform matrix = mGraphicsView->transform();
    // calculates the sclaing of matrix
    // see http://www.robertblum.com/articles/2005/02/14/decomposing-matrices for rotation out of matrix
    double scale = Vec2F(matrix.m11(), matrix.m21()).length();
    mZoomSlider->setValue((int) (250. + 50. * log(scale) / log(2.)));
}

void ViewWidget::setupMatrix()
{
    qreal scale = ::pow(2.0, (mZoomSlider->value() - 250) / 50.0);

    QTransform matrix;
    matrix.scale(scale, scale);
    matrix.rotate(mRotateSlider->value());

    mGraphicsView->setTransform(matrix);
}

void ViewWidget::zoomIn(int i) // default i = 1
{
    mZoomSlider->setValue(mZoomSlider->value() + i);
}

void ViewWidget::zoomOut(int i) // default i = 1
{
    mZoomSlider->setValue(mZoomSlider->value() - i);
}

void ViewWidget::rotateLeft()
{
    mRotateSlider->setValue(mRotateSlider->value() - 1); // war 10
}

void ViewWidget::rotateRight()
{
    mRotateSlider->setValue(mRotateSlider->value() + 1); // war 10
}

void ViewWidget::hideControls(bool hide)
{
    if(hide)
    {
        mMainWindow->getControlWidget()->setVisible(false);
        hideShowControlsButton->setIcon(QPixmap(":/arrowLeft"));
    }
    else
    {
        mMainWindow->getControlWidget()->setVisible(true);
        hideShowControlsButton->setIcon(QPixmap(":/arrowRight"));
    }
}
void ViewWidget::hideShowControls()
{
    mMainWindow->getHideControlActor()->setChecked(mMainWindow->getControlWidget()->isVisible());
}


#include "moc_view.cpp"
