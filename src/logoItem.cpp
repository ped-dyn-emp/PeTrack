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

#include <QGraphicsScene>
#include <QPainter>
#include <QApplication>
#include <QTimer>

#include "logoItem.h"
#include "petrack.h"

#include <time.h>

// minimale Zeit zwischen zwei Blendenstufen in Millisekunden
#define FRAME_INTERVAL 40

Fader::Fader()
{
    mTimer = nullptr;
}

void Fader::fadeOut(LogoItem* lI, int frames)
{
    mLogoItem = lI;
    if (frames < 1)
        mFrames = 1;
    else
        mFrames = frames;
    mStep = 1./mFrames;
    if (mTimer) // wenn fadeOut schon mal aufgerufen wurde; so wird nur ein QTimer maximal angelegt
        delete mTimer;
    mTimer = new QTimer(this);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(fadeOutStep()));
    mTimer->start(FRAME_INTERVAL); // fuer 25fps
    //QTimer::singleShot(10, this, SLOT(fadeOutSlot()));    
}

void Fader::fadeOutStep()
{
    static clock_t lastTime = clock();

    if (((double)(clock()-lastTime))/CLOCKS_PER_SEC > FRAME_INTERVAL*.001) // beschleunigen, wenn zu langsam ausgeblendet wird
        mStep*=2;
    lastTime = clock();

    if (mLogoItem->getOpacity()>0.)
    {
        mLogoItem->setOpacity(mLogoItem->getOpacity()-mStep);
        mLogoItem->scene()->update(); // ein neuzeichnen wird erwuenscht und irgendwann bei processEvents gemacht
        qApp->processEvents();
    }
    else
    {
        mLogoItem->setVisible(false);
        mTimer->stop();
    }
}

// void Fader::fadeOutStep()
// {
//     double step = 1./mFrames;
//     for (mLogoItem->setOpacity(1.0); mLogoItem->getOpacity()>0.; mLogoItem->setOpacity(mLogoItem->getOpacity()-step))
//     {
//         debout << mLogoItem->getOpacity()<<endl;
// //         mLogoItem->scene()->update(); // ein neuzeichnen wird erwuenscht und irgendwann bei processEvents gemacht
// //         qApp->processEvents();
//     }
//     mLogoItem->setVisible(false);
// }

// --------------------------------------------------

LogoItem::LogoItem(QWidget *wParent, QGraphicsItem * parent)
    : QGraphicsItem(parent)
{
    mOpacity = 1.0;
    mMainWindow = (class Petrack*) wParent;
    //mScene = scene; erhaelt man mit scene()
    mImage = new QImage(":/logo"); // in icons.qrc definiert

    //setAcceptDrops(true);

}

QRectF LogoItem::boundingRect() const
{
    return QRectF(0, 0, mImage->width(), mImage->height());
}

//void LogoItem::dragEnterEvent(QDragEnterEvent *event)
//{
//    debout << "DragEnterEvent(LogoItem)" << endl;
//    mMainWindow->dragEnterEvent(event);
//}
//void LogoItem::dropEvent(QDropEvent *event)
//{
//    debout << "DropEvent(LogoItem): " << endl;
//    mMainWindow->dropEvent(event);
//}
    
void LogoItem::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    int w;
    int h;
    if (mMainWindow->getImage())
    {
        w = (mMainWindow->getImage()->width())/2 - (mImage->width())/2 - mMainWindow->getImageBorderSize();
        h = (mMainWindow->getImage()->height())/2 - (mImage->height())/2 - mMainWindow->getImageBorderSize();
    }
    else
    {
        w = 0; 
        h = 0;
    }
    painter->setOpacity(mOpacity);
    painter->drawImage(w,h,*mImage);
}

// logo wird langsam ausgeblendet
// blendgeschw ist rechnergeschwindigkeitsabhaengig
// qtimer waere auch eine mgl
// so wie hier, wird fkt, die nicht uber events/slots abgewickelt wird,
//    wird zB bei projekt import die titelleiste erst nach dem ausblenden aktualisiert
// frame = anzahl der bildupdates, bis logo verschwunden ist
// mit opengl fkt fade schlecht
void LogoItem::fadeOut(int frames)
{
    fader.fadeOut(this, frames);
//     if (frames < 1)
//         frames = 1;
//     double step = 1./frames;
//     for (mOpacity=1.0; mOpacity>0.; mOpacity-=step)
//     {
//         scene()->update(); // ein neuzeichnen wird erwuenscht und irgendwann bei processEvents gemacht
//         qApp->processEvents();
//     }
//     setVisible(false);
}

#include "moc_logoItem.cpp"
