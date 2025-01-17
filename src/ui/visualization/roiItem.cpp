/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) 2025 Forschungszentrum Jülich GmbH, IAS-7
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

#include "roiItem.h"

#include "petrack.h"

#include <QGraphicsSceneMouseEvent>

RoiItem::RoiItem(QWidget *wParent, const QColor &color) : QObject(wParent)
{
    mMainWindow = dynamic_cast<class Petrack *>(wParent);
    setRect(0, 0, 0, 0);
    QPen pen(color);
    setPen(pen);
    setAcceptHoverEvents(!mIsFixed);
    setFlag(ItemIsMovable, !mIsFixed);
    hide(); // default in control
}

void RoiItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(!mIsFixed)
    {
        mPressRect =
            QRect(myRound(rect().left()), myRound(rect().top()), myRound(rect().width()), myRound(rect().height()));
        mPressPos = event->pos();
        if((event->pos()).x() < DISTANCE_TO_BORDER + mPressRect.x())
        {
            if((event->pos()).y() < DISTANCE_TO_BORDER + mPressRect.y())
            {
                mPressLocation = PressLocation::topLeft;
                setCursor(Qt::SizeFDiagCursor);
            }
            else if((event->pos()).y() > mPressRect.height() + mPressRect.y() - DISTANCE_TO_BORDER)
            {
                mPressLocation = PressLocation::bottomLeft;
                setCursor(Qt::SizeBDiagCursor);
            }
            else
            {
                mPressLocation = PressLocation::left;
                setCursor(Qt::SizeHorCursor);
            }
        }
        else if((event->pos()).x() > mPressRect.width() + mPressRect.x() - DISTANCE_TO_BORDER)
        {
            if((event->pos()).y() < DISTANCE_TO_BORDER + mPressRect.y())
            {
                mPressLocation = PressLocation::topRight;
                setCursor(Qt::SizeBDiagCursor);
            }
            else if((event->pos()).y() > mPressRect.height() + mPressRect.y() - DISTANCE_TO_BORDER)
            {
                mPressLocation = PressLocation::bottomRight;
                setCursor(Qt::SizeFDiagCursor);
            }
            else
            {
                mPressLocation = PressLocation::right;
                setCursor(Qt::SizeHorCursor);
            }
        }
        else if((event->pos()).y() < DISTANCE_TO_BORDER + mPressRect.y())
        {
            mPressLocation = PressLocation::top;
            setCursor(Qt::SizeVerCursor);
        }
        else if((event->pos()).y() > mPressRect.height() + mPressRect.y() - DISTANCE_TO_BORDER)
        {
            mPressLocation = PressLocation::bottom;
            setCursor(Qt::SizeVerCursor);
        }
        else
        {
            mPressLocation = PressLocation::inside;
            setCursor(Qt::ClosedHandCursor);
        }
    }

    QGraphicsRectItem::mousePressEvent(event);
}

void RoiItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    emit changed();

    if(!mMainWindow->isLoading())
    {
        mMainWindow->updateImage();
    }

    QGraphicsRectItem::mouseReleaseEvent(event);
}

void RoiItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    // event, of moving mouse while mouse button is pressed
    if(!mIsFixed)
    {
        QImage *img  = mMainWindow->getImage();
        QPoint  diff = QPoint(myRound((event->pos() - mPressPos).x()), myRound((event->pos() - mPressPos).y()));
        // do not extend over the border of the image
        // do not swap the rectangle, to avoid negative width
        if(img != nullptr)
        {
            if(mPressLocation == PressLocation::inside || mPressLocation == PressLocation::topLeft ||
               mPressLocation == PressLocation::left || mPressLocation == PressLocation::bottomLeft)
            {
                if(mPressRect.x() + diff.x() < -mMainWindow->getImageBorderSize())
                {
                    diff.setX(-mPressRect.x() - mMainWindow->getImageBorderSize());
                }
                if(mPressLocation != PressLocation::inside && mPressRect.width() - diff.x() < MIN_SIZE)
                {
                    diff.setX(mPressRect.width() - MIN_SIZE);
                }
            }
            if(mPressLocation == PressLocation::inside || mPressLocation == PressLocation::topLeft ||
               mPressLocation == PressLocation::top || mPressLocation == PressLocation::topRight)
            {
                if(mPressRect.y() + diff.y() < -mMainWindow->getImageBorderSize())
                {
                    diff.setY(-mPressRect.y() - mMainWindow->getImageBorderSize());
                }
                if(mPressLocation != PressLocation::inside && mPressRect.height() - diff.y() < MIN_SIZE)
                {
                    diff.setY(mPressRect.height() - MIN_SIZE);
                }
            }
            if(mPressLocation == PressLocation::inside || mPressLocation == PressLocation::topRight ||
               mPressLocation == PressLocation::right || mPressLocation == PressLocation::bottomRight)
            {
                if(mPressRect.x() + diff.x() + mPressRect.width() > img->width() - mMainWindow->getImageBorderSize())
                {
                    diff.setX(img->width() - mPressRect.x() - mPressRect.width() - mMainWindow->getImageBorderSize());
                }
                if(mPressLocation != PressLocation::inside && mPressRect.width() + diff.x() < MIN_SIZE)
                {
                    diff.setX(-mPressRect.width() + MIN_SIZE);
                }
            }
            if(mPressLocation == PressLocation::inside || mPressLocation == PressLocation::bottomLeft ||
               mPressLocation == PressLocation::bottom || mPressLocation == PressLocation::bottomRight)
            {
                if(mPressRect.y() + diff.y() + mPressRect.height() > img->height() - mMainWindow->getImageBorderSize())
                {
                    diff.setY(img->height() - mPressRect.y() - mPressRect.height() - mMainWindow->getImageBorderSize());
                }
                if(mPressLocation != PressLocation::inside && mPressRect.height() + diff.y() < MIN_SIZE)
                {
                    diff.setY(-mPressRect.height() + MIN_SIZE);
                }
            }
        }

        switch(mPressLocation)
        {
            case PressLocation::topLeft:
                setRect(
                    mPressRect.x() + diff.x(),
                    mPressRect.y() + diff.y(),
                    mPressRect.width() - diff.x(),
                    mPressRect.height() - diff.y());
                break;
            case PressLocation::topRight:
                setRect(
                    mPressRect.x(),
                    mPressRect.y() + diff.y(),
                    mPressRect.width() + diff.x(),
                    mPressRect.height() - diff.y());
                break;
            case PressLocation::bottomLeft:
                setRect(
                    mPressRect.x() + diff.x(),
                    mPressRect.y(),
                    mPressRect.width() - diff.x(),
                    mPressRect.height() + diff.y());
                break;
            case PressLocation::bottomRight:
                setRect(mPressRect.x(), mPressRect.y(), mPressRect.width() + diff.x(), mPressRect.height() + diff.y());
                break;
            case PressLocation::left:
                setRect(mPressRect.x() + diff.x(), mPressRect.y(), mPressRect.width() - diff.x(), mPressRect.height());
                break;
            case PressLocation::right:
                setRect(mPressRect.x(), mPressRect.y(), mPressRect.width() + diff.x(), mPressRect.height());
                break;
            case PressLocation::top:
                setRect(mPressRect.x(), mPressRect.y() + diff.y(), mPressRect.width(), mPressRect.height() - diff.y());
                break;
            case PressLocation::bottom:
                setRect(mPressRect.x(), mPressRect.y(), mPressRect.width(), mPressRect.height() + diff.y());
                break;
            case PressLocation::inside:
                setRect(mPressRect.x() + diff.x(), mPressRect.y() + diff.y(), mPressRect.width(), mPressRect.height());
                break;
                // do not use QGraphicsRectItem::mouseMoveEvent(event) as it also moves the coordinate system
        }
    }
    else
    {
        QGraphicsRectItem::mouseMoveEvent(event);
    }
}

void RoiItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    QPointF pos = event->scenePos();
    pos.setX(pos.x() + mMainWindow->getImageBorderSize());
    pos.setY(pos.y() + mMainWindow->getImageBorderSize());

    // due to the line width of the QRect, the QRect might get slightly larger than the image. To avoid errors then
    // accessing the image mMainWindow->setMousePosOnImage(pos) also width() is checked here
    if((mMainWindow->getImage() != nullptr) && (pos.x() < mMainWindow->getImage()->width()) &&
       (pos.y() < mMainWindow->getImage()->height()))
    {
        mMainWindow->setMousePosOnImage(pos);

        if(!mIsFixed)
        {
            QRectF r = rect().marginsRemoved(
                QMarginsF(DISTANCE_TO_BORDER, DISTANCE_TO_BORDER, DISTANCE_TO_BORDER, DISTANCE_TO_BORDER));
            auto pos = event->pos();

            const bool topEdge    = pos.y() < r.top();
            const bool bottomEdge = pos.y() > r.bottom();
            const bool leftEdge   = pos.x() < r.left();
            const bool rightEdge  = pos.x() > r.right();

            const bool xEdge = leftEdge || rightEdge;
            const bool yEdge = topEdge || bottomEdge;

            if(xEdge && !yEdge)
            {
                setCursor(Qt::SizeHorCursor);
            }
            else if(yEdge && !xEdge)
            {
                setCursor(Qt::SizeVerCursor);
            }
            else if(topEdge && xEdge)
            {
                if(leftEdge)
                {
                    setCursor(Qt::SizeFDiagCursor);
                }
                else
                {
                    setCursor(Qt::SizeBDiagCursor);
                }
            }
            else if(bottomEdge && xEdge)
            {
                if(leftEdge)
                {
                    setCursor(Qt::SizeBDiagCursor);
                }
                else
                {
                    setCursor(Qt::SizeFDiagCursor);
                }
            }
            else
            {
                setCursor(Qt::OpenHandCursor);
            }
        }
    }
    QGraphicsRectItem::hoverMoveEvent(event);
}

/**
 * @brief Restores the size of the ROI if the image size changed
 */
void RoiItem::restoreSize()
{
    cv::Mat img = mMainWindow->getImageFiltered();

    if(!img.empty())
    {
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

void RoiItem::setFixed(bool fixed)
{
    mIsFixed = fixed;
    setAcceptHoverEvents(!mIsFixed);
    setFlag(ItemIsMovable, !mIsFixed);
    if(mIsFixed)
    {
        unsetCursor();
    }
}

/**
 * @brief Sets the rect to the top left corner of the image and width/height such that it includes the full
 * image.
 */
void RoiItem::setToFullImageSize()
{
    setRect(
        -mMainWindow->getImageBorderSize(),
        -mMainWindow->getImageBorderSize(),
        mMainWindow->getImage()->width(),
        mMainWindow->getImage()->height());

    emit changed();
}
