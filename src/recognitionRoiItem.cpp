/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2010-2021 Forschungszentrum Jülich GmbH,
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

#include "recognitionRoiItem.h"

#include "control.h"
#include "petrack.h"
#include "view.h"

#include <QtWidgets>

RecognitionRoiItem::RecognitionRoiItem(QWidget *wParent, QGraphicsItem *parent) : QGraphicsRectItem(parent)
{
    mMainWindow    = (class Petrack *) wParent;
    mControlWidget = mMainWindow->getControlWidget();
    setRect(0, 0, 0, 0); // qreal x, qreal y, qreal width, qreal height
    QPen pen(Qt::green);
    setPen(pen);
    setAcceptHoverEvents(true);
    setFlags(ItemIsMovable); // default in control
    hide();                  // default in control
}

void RecognitionRoiItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(!mControlWidget->getRecoRoiFix())
    {
        mPressRect =
            QRect(myRound(rect().left()), myRound(rect().top()), myRound(rect().width()), myRound(rect().height()));
        mPressPos = event->pos();
        if((event->pos()).x() < DISTANCE_TO_BORDER + mPressRect.x())
        {
            if((event->pos()).y() < DISTANCE_TO_BORDER + mPressRect.y())
            {
                mPressLocation = topLeft;
                setCursor(Qt::SizeFDiagCursor);
            }
            else if((event->pos()).y() > mPressRect.height() + mPressRect.y() - DISTANCE_TO_BORDER)
            {
                mPressLocation = bottomLeft;
                setCursor(Qt::SizeBDiagCursor);
            }
            else
            {
                mPressLocation = left;
                setCursor(Qt::SizeHorCursor);
            }
        }
        else if((event->pos()).x() > mPressRect.width() + mPressRect.x() - DISTANCE_TO_BORDER)
        {
            if((event->pos()).y() < DISTANCE_TO_BORDER + mPressRect.y())
            {
                mPressLocation = topRight;
                setCursor(Qt::SizeBDiagCursor);
            }
            else if((event->pos()).y() > mPressRect.height() + mPressRect.y() - DISTANCE_TO_BORDER)
            {
                mPressLocation = bottomRight;
                setCursor(Qt::SizeFDiagCursor);
            }
            else
            {
                mPressLocation = right;
                setCursor(Qt::SizeHorCursor);
            }
        }
        else if((event->pos()).y() < DISTANCE_TO_BORDER + mPressRect.y())
        {
            mPressLocation = top;
            setCursor(Qt::SizeVerCursor);
        }
        else if((event->pos()).y() > mPressRect.height() + mPressRect.y() - DISTANCE_TO_BORDER)
        {
            mPressLocation = bottom;
            setCursor(Qt::SizeVerCursor);
        }
        else
        {
            mPressLocation = inside;
            setCursor(Qt::ClosedHandCursor);
        }
    }

    QGraphicsRectItem::mousePressEvent(event);
}

void RecognitionRoiItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    mMainWindow->setRecognitionChanged(true);
    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }

    QGraphicsRectItem::mouseReleaseEvent(event);
}

// event, of moving mouse while mouse button is pressed
void RecognitionRoiItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if(!mControlWidget->getRecoRoiFix())
    {
        QImage *img  = mMainWindow->getImage();
        QPoint  diff = QPoint(myRound((event->pos() - mPressPos).x()), myRound((event->pos() - mPressPos).y()));
        // raender des bildes nicht ueberscheiten
        // swappen des rechtecks vermeiden, damit keine negativen width...
        if(img != nullptr)
        {
            if(mPressLocation == inside || mPressLocation == topLeft || mPressLocation == left ||
               mPressLocation == bottomLeft)
            {
                if(mPressRect.x() + diff.x() < -mMainWindow->getImageBorderSize())
                {
                    diff.setX(-mPressRect.x() - mMainWindow->getImageBorderSize());
                }
                if(mPressLocation != inside && mPressRect.width() - diff.x() < MIN_SIZE)
                {
                    diff.setX(mPressRect.width() - MIN_SIZE);
                }
            }
            if(mPressLocation == inside || mPressLocation == topLeft || mPressLocation == top ||
               mPressLocation == topRight)
            {
                if(mPressRect.y() + diff.y() < -mMainWindow->getImageBorderSize())
                {
                    diff.setY(-mPressRect.y() - mMainWindow->getImageBorderSize());
                }
                if(mPressLocation != inside && mPressRect.height() - diff.y() < MIN_SIZE)
                {
                    diff.setY(mPressRect.height() - MIN_SIZE);
                }
            }
            if(mPressLocation == inside || mPressLocation == topRight || mPressLocation == right ||
               mPressLocation == bottomRight)
            {
                if(mPressRect.x() + diff.x() + mPressRect.width() > img->width() - mMainWindow->getImageBorderSize())
                {
                    diff.setX(img->width() - mPressRect.x() - mPressRect.width() - mMainWindow->getImageBorderSize());
                }
                if(mPressLocation != inside && mPressRect.width() + diff.x() < MIN_SIZE)
                {
                    diff.setX(-mPressRect.width() + MIN_SIZE);
                }
            }
            if(mPressLocation == inside || mPressLocation == bottomLeft || mPressLocation == bottom ||
               mPressLocation == bottomRight)
            {
                if(mPressRect.y() + diff.y() + mPressRect.height() > img->height() - mMainWindow->getImageBorderSize())
                {
                    diff.setY(img->height() - mPressRect.y() - mPressRect.height() - mMainWindow->getImageBorderSize());
                }
                if(mPressLocation != inside && mPressRect.height() + diff.y() < MIN_SIZE)
                {
                    diff.setY(-mPressRect.height() + MIN_SIZE);
                }
            }
        }
        if(mPressLocation == topLeft)
        {
            setRect(
                mPressRect.x() + diff.x(),
                mPressRect.y() + diff.y(),
                mPressRect.width() - diff.x(),
                mPressRect.height() - diff.y());
        }
        else if(mPressLocation == topRight)
        {
            setRect(
                mPressRect.x(),
                mPressRect.y() + diff.y(),
                mPressRect.width() + diff.x(),
                mPressRect.height() - diff.y());
        }
        else if(mPressLocation == bottomLeft)
        {
            setRect(
                mPressRect.x() + diff.x(),
                mPressRect.y(),
                mPressRect.width() - diff.x(),
                mPressRect.height() + diff.y());
        }
        else if(mPressLocation == bottomRight)
        {
            setRect(mPressRect.x(), mPressRect.y(), mPressRect.width() + diff.x(), mPressRect.height() + diff.y());
        }
        else if(mPressLocation == left)
        {
            setRect(mPressRect.x() + diff.x(), mPressRect.y(), mPressRect.width() - diff.x(), mPressRect.height());
        }
        else if(mPressLocation == right)
        {
            setRect(mPressRect.x(), mPressRect.y(), mPressRect.width() + diff.x(), mPressRect.height());
        }
        else if(mPressLocation == top)
        {
            setRect(mPressRect.x(), mPressRect.y() + diff.y(), mPressRect.width(), mPressRect.height() - diff.y());
        }
        else if(mPressLocation == bottom)
        {
            setRect(mPressRect.x(), mPressRect.y(), mPressRect.width(), mPressRect.height() + diff.y());
        }
        else // entspricht: if (mPressLocation == inside)
        {
            setRect(mPressRect.x() + diff.x(), mPressRect.y() + diff.y(), mPressRect.width(), mPressRect.height());
        }
        // nicht, da sonst koordinatensystem verschoben wird: QGraphicsRectItem::mouseMoveEvent(event); // drag
    }
    else // drag mach ich selber
    {
        QGraphicsRectItem::mouseMoveEvent(event);
    }
}

// event, of moving mouse
void RecognitionRoiItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    QPointF pos = event->scenePos();
    pos.setX(pos.x() + mMainWindow->getImageBorderSize());
    pos.setY(pos.y() + mMainWindow->getImageBorderSize());
    // abfrage auf width() ..., da durch rectLinie die recoBox etwas groesser ist als das Bild und
    // es bei mMainWindow->setMousePosOnImage(pos); zum fehler beim bildzugriff kommen kann!!!
    if(mMainWindow->getImage() && //(pos.x() > 0) && (pos.y() > 0) &&
       (pos.x() < mMainWindow->getImage()->width()) && (pos.y() < mMainWindow->getImage()->height()))
    {
        mMainWindow->setMousePosOnImage(pos);

        if(!mControlWidget->getRecoRoiFix())
        {
            QRect r =
                QRect(myRound(rect().left()), myRound(rect().top()), myRound(rect().width()), myRound(rect().height()));
            if((event->pos()).x() < DISTANCE_TO_BORDER + r.x())
            {
                if((event->pos()).y() < DISTANCE_TO_BORDER + r.y())
                {
                    setCursor(Qt::SizeFDiagCursor);
                }
                else if((event->pos()).y() > r.height() + r.y() - DISTANCE_TO_BORDER)
                {
                    setCursor(Qt::SizeBDiagCursor);
                }
                else
                {
                    setCursor(Qt::SizeHorCursor);
                }
            }
            else if((event->pos()).x() > r.width() + r.x() - DISTANCE_TO_BORDER)
            {
                if((event->pos()).y() < DISTANCE_TO_BORDER + r.y())
                {
                    setCursor(Qt::SizeBDiagCursor);
                }
                else if((event->pos()).y() > r.height() + r.y() - DISTANCE_TO_BORDER)
                {
                    setCursor(Qt::SizeFDiagCursor);
                }
                else
                {
                    setCursor(Qt::SizeHorCursor);
                }
            }
            else if(
                ((event->pos()).y() < DISTANCE_TO_BORDER + r.y()) ||
                ((event->pos()).y() > r.height() + r.y() - DISTANCE_TO_BORDER))
            {
                setCursor(Qt::SizeVerCursor);
            }
            else
            {
                setCursor(Qt::OpenHandCursor);
            }
        }
        else // wird nur einmal durchaufen - ruecksetzen in control.cpp
        {
            setAcceptHoverEvents(false); // verhoindert nicht, dass wenn objekt darunter liegt, was andereen cursor
                                         // haette - cursor wird weiterhin beim drueberfahren auf cross gesetzt
            setCursor(Qt::CrossCursor);
        }
    }

    QGraphicsRectItem::hoverMoveEvent(event);
}

// check rect because bordersize changes and without mouse event nothing changes the rect
void RecognitionRoiItem::checkRect()
{
    cv::Mat img = mMainWindow->getImageFiltered();
    if(!img.empty())
    {
        // not QImage *img = mMainWindow->getImage(); as size is not adapted yet
        QRect r =
            QRect(myRound(rect().left()), myRound(rect().top()), myRound(rect().width()), myRound(rect().height()));
        if(r.x() > img.cols - mMainWindow->getImageBorderSize() - MIN_SIZE ||
           r.y() > img.rows - mMainWindow->getImageBorderSize() - MIN_SIZE ||
           r.x() + r.width() < -mMainWindow->getImageBorderSize() + MIN_SIZE ||
           r.y() + r.height() < -mMainWindow->getImageBorderSize() + MIN_SIZE)
        {
            setRect(-mMainWindow->getImageBorderSize(), -mMainWindow->getImageBorderSize(), img.cols, img.rows);
        }
        else
        {
            if(r.x() < -mMainWindow->getImageBorderSize())
            {
                setRect(
                    -mMainWindow->getImageBorderSize(),
                    r.y(),
                    r.width() + (mMainWindow->getImageBorderSize() + r.x()),
                    r.height());
            }
            if(r.y() < -mMainWindow->getImageBorderSize())
            {
                setRect(
                    r.x(),
                    -mMainWindow->getImageBorderSize(),
                    r.width(),
                    r.height() + (mMainWindow->getImageBorderSize() + r.y()));
            }
            if(r.x() + mMainWindow->getImageBorderSize() + r.width() > img.cols)
            {
                setRect(r.x(), r.y(), img.cols - r.x() - mMainWindow->getImageBorderSize(), r.height());
            }
            if(r.y() + mMainWindow->getImageBorderSize() + r.height() > img.rows)
            {
                setRect(r.x(), r.y(), r.width(), img.rows - r.y() - mMainWindow->getImageBorderSize());
            }
        }
    }
    else
    {
        setRect(0, 0, 0, 0);
    }
}
