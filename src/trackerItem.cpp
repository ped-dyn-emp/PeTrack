#include <QtWidgets>
#include <QInputDialog>

#include "petrack.h"
#include "control.h"
#include "view.h"
#include "trackerItem.h"
#include "tracker.h"
#include "animation.h"

using namespace::cv;
using namespace std;

// in x und y gleichermassen skaliertes koordinatensystem,
// da von einer vorherigen intrinsischen kamerakalibrierung ausgegenagen wird,
// so dass pixel quadratisch 
TrackerItem::TrackerItem(QWidget *wParent, Tracker *tracker, QGraphicsItem * parent)
    : QGraphicsItem(parent)
{
    mMainWindow = (class Petrack*) wParent;
    mControlWidget = mMainWindow->getControlWidget();
    mTracker = tracker;

    //acceptHoverEvents();

    updateData(); // um zB setFlags(ItemIsMovable) je nach anzeige zu aendern
    //    setEnabled(false); // all mouse events connot access this item, but it will be seen
    // einzig move koennte interessant sein, um grid zu verschieben?!
//     setAcceptsHoverEvents(true);
}
// // bounding box wird durch linke obere ecke und breite/hoehe angegeben
// // wenn an den rand gescrollt wurde im view, dann wird durch das dynamische anpassen
// // bei trans und scale zwar zuerst alles neu gezeichnet durch update, 
// // aber beim verkleinern des scrollbereichs nur der teil von tracker neu gezeichnet
QRectF TrackerItem::boundingRect() const
{
    if (mMainWindow->getImage())
        return QRectF(-mMainWindow->getImageBorderSize(), -mMainWindow->getImageBorderSize(), mMainWindow->getImage()->width(), mMainWindow->getImage()->height());
//         return QRectF(-mMainWindow->getImageBorderSize(), -mMainWindow->getImageBorderSize(), mImage->width(), mImage->height());
    else
        return QRectF(0, 0, 0, 0);
//     // bounding box wird in lokalen koordinaten angegeben!!! (+-10 wegen zahl "1")
//     if (mControlWidget->getCalibCoordShow())
//         return QRectF(-110., -110., 220., 220.);
//     else                    ;

//         return QRectF(0., 0., 0., 0.);

//     // sicher ware diese boundingbox, da alles
//     //     return QRectF(xMin, yMin, xMax-xMin, yMax-yMin);
//     // eigentlich muesste folgende Zeile reichen, aber beim ranzoomen verschwindet dann koord.sys.
//     //     return QRectF(mControlWidget->getCalibCoordTransX()/10.-scale, mControlWidget->getCalibCoordTransY()/10.-scale, 2*scale, 2*scale);
}
void TrackerItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    if( !(event->modifiers() & Qt::ShiftModifier || event->modifiers() & Qt::ControlModifier || event->modifiers() & Qt::AltModifier) )
    {
    //debout << "start contextMenuEvent..." << endl;
    TrackPoint p((Vec2F) event->pos(), 110); // 110 ist ueber 100 (hoechste Qualitaetsstufe) und wird nach einfuegen auf 100 gesetzt
    //debout << "p: " << p.x() << ", " << p.y() << endl;
    bool found = false;
    int i, iNearest = -1;
    float dist, minDist = 1000000.;

    QSet<int> onlyVisible = mMainWindow->getOnlyVisible();
    int frame = mMainWindow->getAnimation()->getCurrentFrameNum();

    for (i = 0; i < mTracker->size(); ++i) // !found &&  // ueber TrackPerson
    {
        if (((onlyVisible.empty()) || (onlyVisible.contains(i))) && mTracker->at(i).trackPointExist(frame))
        {
            dist = mTracker->at(i).trackPointAt(frame).distanceToPoint(p);
            if (( dist < mMainWindow->getHeadSize(NULL, i, frame)/2.) ||
                ( (mTracker->at(i).trackPointAt(frame).distanceToPoint(p.colPoint()) < mMainWindow->getHeadSize(NULL, i, frame)/2.)))
            {
                if (found)
                {
                    debout << "Warning: more possible trackpoints for point" << endl;
                    debout << "         " << p << " in frame " << frame << " with low distance:" << endl;
                    debout << "         person " << i+1 << " (distance: " << dist << "), " << endl;
                    debout << "         person " << iNearest+1 << " (distance: " << minDist << "), " << endl;
                    if (minDist > dist)
                    {
                        minDist = dist;
                        iNearest = i;
                    }
                }
                else
                {
                    minDist = dist;
                    iNearest = i;
                    // WAR: break inner loop
                    found = true;
                }
            }
        }
    }
    QMenu menu;
    TrackPerson tp;
    float height;
    bool height_set_by_user = false;
    QAction *delTrj, *delFutureTrj, *delPastTrj, *creTrj, *infoTrj, *addComment, *setHeight, *resetHeight;

    //debout << "mTracker->size(): " << mTracker->size() << endl;
    if(found)
    {
        i = iNearest;
        //debout << "found: " << found << " iNearest: " << iNearest << endl;
        tp = mTracker->at(i);
        //debout << "test" << endl;
        height = tp.height();
        //debout << "height: " << height << endl;
        //debout << "height<MIN_HEIGHT: " << height << endl;
        //debout << "mTracker.color: valid? " << tp.color().isValid() << endl;
        //debout << "tp.color: " << tp.color() << endl;
        //QRgb col = mMainWindow->getImage()->pixel(QPoint(event->scenePos().x(),event->scenePos().y()));
        //QColor color(qRed(col), qGreen(col), qBlue(col));

        //debout << "color: " << color << endl;
        if (height < MIN_HEIGHT+1)
        {
            if (tp.color().isValid())
                height = mControlWidget->getColorPlot()->map(tp.color());
            //debout << "height: " << height << endl;
        }else
        {
            //height = mControlWidget->mapDefaultHeight->value();
            //debout << "height(default): " << height << endl;
            height_set_by_user = true;
        }
        //debout << "setHeight" << endl;
//        tp.setHeight(height);
        infoTrj = menu.addAction(QString("PersonNr: %1 height: %2 frames: [%3..%4]").arg(i+1).arg(height).arg(tp.firstFrame()).arg(tp.lastFrame()));
        delTrj = menu.addAction("Delete whole trajectory");
        delFutureTrj = menu.addAction("Delete past part of the trajectory");
        delPastTrj = menu.addAction("Delete future part of the trajectory");
        setHeight = menu.addAction("Set person height");
        if(height_set_by_user) resetHeight = menu.addAction("Reset height");
        addComment = menu.addAction("Edit comment");
    }else
    {
        creTrj = menu.addAction("Create new trajectory");
    }

    //debout << "selectedAction: " << endl;
    QAction *selectedAction = menu.exec(event->screenPos());
    //debout << "screenPos: " << event->scenePos() << endl;
    if( selectedAction == creTrj )
    {
        //debout << "Create new trajectory..." << endl;
        int res = mMainWindow->addManualTrackPoint(event->scenePos());
        //debout << "addManuelTrackPoint: " << res << endl;
    }else if( selectedAction == delTrj )
    {
        //debout << "Delete trajectory..." << endl;
        mMainWindow->deleteTrackPoint(event->scenePos(),0);
    }else if( selectedAction == delFutureTrj )
    {
        //debout << "Delete future part of trajectory..." << endl;
        mMainWindow->deleteTrackPoint(event->scenePos(),-1);
    }else if( selectedAction == delPastTrj )
    {
        //debout << "Delete past part of trajectory..." << endl;
        mMainWindow->deleteTrackPoint(event->scenePos(),1);
    }else if( selectedAction == addComment )
    {
        mMainWindow->editTrackPersonComment(event->scenePos());
    }else if( selectedAction == setHeight )
    {
        mMainWindow->setTrackPersonHeight(event->scenePos());
    }else if( selectedAction == resetHeight )
    {
        mMainWindow->resetTrackPersonHeight(event->scenePos());
    }else if( selectedAction == infoTrj )
    {
        if (found)
        {
        QString out;
        QMessageBox msgBox;
        msgBox.setText(QString("Info for trajectory number %1:").arg(i+1));

        if(height_set_by_user)
            out = QString("<table>"
                      "<tr><td>height:</td><td>%0 cm (edited by user)</td></tr>"
                      "<tr><td>frames:</td><td>[%1...%2]</td></tr>"
                      "<tr><td>color:</td><td><font style='display:inline;background:%3;color:#fff;'>%4</font></td></tr>"
                      "<tr><td>comment:</td><td>%5</td></tr>"
                      "<tr><td></td><td></td></tr>");
        else
            out = QString("<table>"
                      "<tr><td>height:</td><td>%0 cm</td></tr>"
                      "<tr><td>frames:</td><td>[%1...%2]</td></tr>"
                      "<tr><td>color:</td><td><font style='display:inline;background:%3;color:#fff;'>%4</font></td></tr>"
                      "<tr><td>comment:</td><td>%5</td></tr>"
                      "<tr><td></td><td></td></tr>");
        if( tp.lastFrame()-tp.firstFrame() > 5 )
        {
            out.append(QString("<tr><td>frame [%6]:</td><td>[%7, %8]</td></tr>"
                                            "<tr><td>frame [%9]:</td><td>[%10, %11]</td></tr>"
                                            "<tr><td colspan='2'>...</td></tr>"
                                            "<tr><td colspan='2'>...</td></tr>"
                                            "<tr><td>frame [%12]:</td><td>[%13, %14]</td></tr>"
                                            "<tr><td>frame [%15]:</td><td>[%16, %17]]</td></tr>"
                                          "</table>").toLatin1());
            msgBox.setInformativeText( out.arg(height)
                                      .arg(tp.firstFrame())
                                      .arg(tp.lastFrame())
                                      .arg(tp.color().name())
                                      .arg(tp.color().name())
                                      .arg(tp.comment())
                                      .arg(tp.firstFrame())
                                      .arg(tp.at(0).x())
                                      .arg(tp.at(0).y())
                                      .arg(tp.firstFrame()+1)
                                      .arg(tp.at(1).x())
                                      .arg(tp.at(1).y())
                                      .arg(tp.lastFrame()-1)
                                      .arg(tp.at(tp.size()-2).x())
                                      .arg(tp.at(tp.size()-2).y())
                                      .arg(tp.lastFrame())
                                      .arg(tp.at(tp.size()-1).x())
                                      .arg(tp.at(tp.size()-1).y()));
        }else
        {
            out.append(QString("</table>"));
            msgBox.setInformativeText( out.arg(height)
                                       .arg(tp.firstFrame())
                                       .arg(tp.lastFrame())
                                       .arg(tp.color().name())
                                       .arg(tp.color().name())
                                       .arg(tp.comment()));
        }
        out = QString();

        for(int frame=tp.firstFrame(); frame<=tp.lastFrame();frame++)
        {
            out.append(QString("frame [%0]: [%1, %2]\n").arg(frame).arg(tp.at(frame-tp.firstFrame()).x()).arg(tp.at(frame-tp.firstFrame()).y()));
        }

        msgBox.setDetailedText(out);

        QSpacerItem* horizontalSpacer = new QSpacerItem(300, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
        QGridLayout* layout = (QGridLayout*)msgBox.layout();
        msgBox.setWindowTitle("PeTrack");
        msgBox.setIcon(QMessageBox::Information);
        layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.setEscapeButton(QMessageBox::Ok);
        msgBox.exec();
        }
    }
    }
    mMainWindow->getScene()->update();
    //debout << "finished contextMenuEvent!" << endl;
}

//// event, of moving mouse while button is pressed
//void TrackerItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
//{

//        setCursor(Qt::ClosedHandCursor);
//        QPointF diff = event->scenePos()-event->lastScenePos(); //screenPos()-buttonDownScreenPos(Qt::RightButton) also interesting
//        if (event->buttons() == Qt::RightButton) // event->button() doesnt work
//        {
//            debout << "right mouse button" << endl;
//        }
//        else if (event->buttons() == Qt::LeftButton)
//        {
//            debout << "left mouse button" << endl;
//        }
//        else if (event->buttons() == Qt::MidButton)
//        {
//            debout << "mid mouse button" << endl;
//        }
//}

//// event, of moving mouse
//void TrackerItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
//{
//    QPointF pos = event->scenePos();
//    pos.setX(pos.x() + mMainWindow->getImageBorderSize());
//    pos.setY(pos.y() + mMainWindow->getImageBorderSize());
//    if (pos.x() >=0 && pos.x() <= mMainWindow->getImage()->width() && pos.y() >=0 && pos.y() <= mMainWindow->getImage()->height())
//        mMainWindow->setMousePosOnImage(pos);

//    if (!mControlWidget->getCalibCoordFix())
//        setCursor(Qt::OpenHandCursor);
//    else
//        setCursor(Qt::CrossCursor);

    //debout << "hover Move Event" << endl;
    //QGraphicsItem::hoverMoveEvent(event);
//}

void TrackerItem::updateData()
{
//     if (!mControlWidget->getCalibCoordFix())
//         setFlags(ItemIsMovable); // noetig, damit mouseEvent leftmousebutton weitergegeben wird, aber drag mach ich selber
//     else
//         setFlags(!ItemIsMovable);

//     if (mControlWidget->getCalibCoordShow())
//     {
//         double sc=mControlWidget->getCalibCoordScale()/10.;
//         double tX=mControlWidget->getCalibCoordTransX()/10.;
//         double tY=mControlWidget->getCalibCoordTransY()/10.;
//         double ro=mControlWidget->getCalibCoordRotate()/10.;

//         // aktualisierung der transformationsmatrix
//         QMatrix matrix;
//         // matrix wird nur bei aenderungen neu bestimmt
//         matrix.translate(tX, tY);
//         matrix.rotate(ro);
//         matrix.scale(sc/100., sc/100.);
//         setMatrix(matrix);
//     }
    
//     mMainWindow->updateSceneRect();

// //     QGraphicsItem::update();
//     mScene->update();
}
    
void TrackerItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
//    debout << "TrackerItem::paint" << endl;
//         double scale=mControlWidget->getCalibCoordScale()/10.;

//         //eigentliche BoundingBox: painter->drawRect(QRectF(mControlWidget->getCalibCoordTransX()/10.-scale, mControlWidget->getCalibCoordTransY()/10.-scale, 2*scale, 2*scale));
//         matrix.translate(mControlWidget->getCalibCoordTransX()/10., mControlWidget->getCalibCoordTransY()/10.);
//         matrix.rotate((mControlWidget->getCalibCoordRotate())/10.);
// mit folgender zeile wuerde nur der painter bewegt, nicht aber das koord des items!!!
//         painter->setWorldMatrix(matrix, true); // true sagt, dass relativ und nicht absolut (also zusaetzlichj zur uebergeordneten matrizen)

        //cycle on frame
//     painter->setPen(Qt::red);
    int from, to;
    int curFrame = mMainWindow->getAnimation()->getCurrentFrameNum();
    QPen ellipsePen;
    QRectF rect;
    Vec2F n;
    Subdiv2D subdiv;
    int i, j;
    QPen linePen;
    QPen numberPen;
    QPen groundPositionPen;
    QPen groundPathPen;
    double pSP = (double) mControlWidget->trackCurrentPointSize->value();
    double pS  = (double) mControlWidget->trackPointSize->value();
    double pSC = (double) mControlWidget->trackColColorSize->value();
    double pSM = (double) mControlWidget->trackColorMarkerSize->value();
    double pSN = (double) mControlWidget->trackNumberSize->value();
    double pSG = (double) mControlWidget->trackGroundPositionSize->value();
    double pSGP = (double) mControlWidget->trackGroundPathSize->value();

    QColor pGPC = mControlWidget->getTrackGroundPathColor();
    QColor pTPC = mControlWidget->getTrackPathColor();//Qt::red;
    QFont font, heightFont;
    //double headSize = mMainWindow->getHeadSize();
    float x_offset=0, y_offset=0;
    float y_switch=0, x_switch=0;
    double hS;

    painter->drawRect( boundingRect() );

    // die Groesse des kreises fuer die aktuelle position kann auf durchschnittlichen Kopfdurchmessers gestell werden
    // 21cm avg kopflaenge, 173cm avg koerpergroesse mann / frau mit Schuhen (180cm nur Mann)
    //if (mControlWidget->trackHeadSized->checkState() == Qt::Checked)
    //    pSP = headSize;
    //else
    //    pSP = (double) mControlWidget->trackCurrentPointSize->value();

    linePen.setColor(pTPC);
    linePen.setWidth(mControlWidget->trackPathWidth->value());

    ellipsePen.setWidth(3);

    if (mControlWidget->trackNumberBold->checkState() == Qt::Checked)
        font.setBold(true);
    else
        font.setBold(false);
    font.setPixelSize(mControlWidget->trackNumberSize->value());
    heightFont.setPixelSize(mControlWidget->trackColColorSize->value());
    //font.setPointSize(pSN);
    painter->setFont(font);
    numberPen.setColor(Qt::red);
    groundPositionPen.setColor(Qt::green);
    groundPositionPen.setWidth(pSG);
    groundPathPen.setColor(pGPC);
    groundPathPen.setWidth(pSGP);


    if(mControlWidget->showVoronoiCells->isChecked() && !mTracker->isEmpty())
    {
        // ToDo: adjust subdiv rect to correct area
        QRectF qrect = mMainWindow->getRecoRoiItem()->rect();
//        debout << "rect: " << rect.left() << ", " << rect.top() << ", " << rect.right() << ", " << rect.bottom() << endl;

        Point3f leftTop = mMainWindow->getExtrCalibration()->get3DPoint(Point2f(qrect.left(),qrect.top()),0);
        Point3f rightBottom = mMainWindow->getExtrCalibration()->get3DPoint(Point2f(qrect.right(),qrect.bottom()),0);

        //        subdiv.initDelaunay(Rect(mMainWindow->getRecoRoiItem()->rect().left(),mMainWindow->getRecoRoiItem()->rect().top(),mMainWindow->getRecoRoiItem()->rect().width(),mMainWindow->getRecoRoiItem()->rect().height()));
        x_offset = -min(leftTop.x,rightBottom.x);
        y_offset = -min(leftTop.y,rightBottom.y);
        x_switch = rightBottom.x < leftTop.x ? abs(rightBottom.x-leftTop.x) : 0;
        y_switch = rightBottom.y < leftTop.y ? abs(leftTop.y-rightBottom.y) : 0;
        debout << "x_offset: " << x_offset << ", y_offset: " << y_offset << ", x_switch: " << x_switch << ", y_switch: " << y_switch << endl;
//        debout << "leftTop: " << leftTop.x+x_offset << ", " << leftTop.y+y_offset << " rightBottom: " << rightBottom.x+x_offset << ", " << rightBottom.y+y_offset << endl;

        Rect rect(Rect(leftTop.x+x_offset,leftTop.y+y_offset,x_switch>0 ? x_switch : (rightBottom.x-leftTop.x),y_switch>0 ? y_switch : (rightBottom.y-leftTop.y)));
        debout << "Rect size: P(" << rect.x << ", " << rect.y << "), width: " << rect.width << ", height: " << rect.height << endl;

        subdiv.initDelaunay(rect);
    }
    for (i = 0; i < mTracker->size(); ++i) // ueber TrackPerson
    {
        // show current frame
        if (!(mControlWidget->trackShowOnly->checkState() == Qt::Checked) && !(mControlWidget->trackShowOnlyList->checkState() == Qt::Checked) ||
            (((mControlWidget->trackShowOnly->checkState() == Qt::Checked) || (mControlWidget->trackShowOnlyList->checkState() == Qt::Checked)) &&
                mMainWindow->getOnlyVisible().contains((i))))
        {
            if (mTracker->at(i).trackPointExist(curFrame))
            {
                if (mControlWidget->trackHeadSized->checkState() == Qt::Checked)
                    pSP = mMainWindow->getHeadSize(NULL, i, curFrame); //headSize;
                const TrackPoint &tp = (*mTracker)[i][curFrame-mTracker->at(i).firstFrame()];
                if (mControlWidget->trackShowCurrentPoint->checkState() == Qt::Checked) //(mControlWidget->recoShowColor->checkState() == Qt::Checked)
                {
                    painter->setBrush(Qt::NoBrush);
                    if (mTracker->at(i).newReco())
                        painter->setPen(Qt::green);
                    else
                        painter->setPen(Qt::blue);
                    //TrackPoint tp = mTracker->at(i).trackPointAt(curFrame); // kostet mehr zeit
                    //int idx = curFrame-mTracker->at(i).firstFrame();
                    rect.setRect(tp.x()-pSP/2., tp.y()-pSP/2., pSP, pSP); //10 //mTracker->at(i).at(idx)
                    painter->drawEllipse(rect); // direkt waere nur int erlaubt tp.x()-5., tp.y()-5., 10., 10.
                }

                if (mControlWidget->trackShowSearchSize->checkState() == Qt::Checked)
                {
                    painter->setBrush(Qt::NoBrush);
                    painter->setPen(Qt::yellow);
                    hS = mMainWindow->winSize(NULL, i, curFrame); //headSize * 1.6 * (mControlWidget->trackRegionScale->value() / 10.);
                    if (hS < 2) 
                        hS = 2; // entspricht Vorgehen in tracker.cpp
                    for (j = 0; j <= mControlWidget->trackRegionLevels->value(); ++j)
                    {
                        rect.setRect(tp.x()-hS/2., tp.y()-hS/2., hS, hS);
                        painter->drawRect(rect);
                        hS *= 2; //hS / pow(2,j);
                    }
                }

                if (mControlWidget->trackShowColorMarker->checkState() == Qt::Checked) //(mControlWidget->recoShowColor->checkState() == Qt::Checked)
                {
                    // farbe des trackpoints
                    if (tp.color().isValid())
                    {
                        painter->setBrush(Qt::NoBrush);
                        ellipsePen.setColor(tp.color());
                        painter->setPen(ellipsePen); //tp.color()
                        rect.setRect(tp.colPoint().x()-pSM/2., tp.colPoint().y()-pSM/2., pSM, pSM); // 14 //mTracker->at(i).at(idx)
                        painter->drawEllipse(rect);
                    }
                }

                // berechnung der normalen, die zur positionierung der nummerieung und der gesamtfarbe dient
                if (((mControlWidget->trackShowColColor->checkState() == Qt::Checked) && (mTracker->at(i).color().isValid())) ||
                    (mControlWidget->trackShowNumber->checkState() == Qt::Checked) ||
                    ((mControlWidget->trackShowColColor->checkState() == Qt::Checked) &&
                     ((mTracker->at(i).height() > MIN_HEIGHT) || ((tp.sp().z() > 0.) && (mControlWidget->trackShowHeightIndividual->checkState() == Qt::Checked))))) //  && (mTracker->at(i).height() > 0.) Hoehe kann auf Treppen auch negativ werden, wenn koord weiter oben angesetzt wird
                {
                    if (tp.color().isValid())
                    {
                        n = (tp-tp.colPoint()).normal();
                        n.normalize();
                        if (n.length()<.001) // wenn to und colpoint aufeinander liegen z bei colorMarker!
                            n.set(1., 0.);
                        //                         rect.setRect(tp.x()+13*n.x()-5.5, tp.y()+13*n.y()-5.5, 11., 11.);
                        //                         rect.setRect(tp.x()+(tp.x()-tp.colPoint().x())-5, tp.y()+(tp.y()-tp.colPoint().y())-5., 10., 10.);
                    }
                    else
                    {
                        // man koennte auch lastNormal von anderem trackpath nehmen statt 1, 0
                        n.set(1., 0.);
                        // den vorherigen trackpoint finden, wo reco farbe erzeugt hat und somit colpoint vorliegt
                        for (j = curFrame-mTracker->at(i).firstFrame(); j > -1; --j)
                            if (mTracker->at(i).at(j).color().isValid())
                            {
                                n = (mTracker->at(i).at(j)-mTracker->at(i).at(j).colPoint()).normal();
                                n.normalize();
                                break;
                            }
                        // einen nachfolgenden trackpoint suchen, wenn vorher keiner mit farbe war
                        // zB wenn rueckwaerts abgespielt wird
                        if ((n.x()==1.) && (n.y()==0.))
                        {
                            for (j = curFrame-mTracker->at(i).firstFrame()+1; j < mTracker->at(i).size(); ++j)
                                if (mTracker->at(i).at(j).color().isValid())
                                {
                                    n = (mTracker->at(i).at(j)-mTracker->at(i).at(j).colPoint()).normal();
                                    n.normalize();
                                    break;
                                }
                        }
                        //                         rect.setRect(tp.x()+13*n.x()-5.5, tp.y()+13*n.y()-5.5, 11., 11.);
                        //                         mTracker->at(i).getLastPointDif(curFrame) schoener waere eine andere position
                        //                         rect.setRect(tp.x()+7., tp.y()-5., 10., 10.);
                    }
                }

                // farbe der gesamten trackperson
                double height = mTracker->at(i).height();
                if (mControlWidget->trackShowColColor->checkState() == Qt::Checked)
                {
                    painter->setPen(numberPen);
                    painter->setBrush(Qt::NoBrush);
                    rect.setRect(tp.x()+10,tp.y()+10,15*pSC,10*pSC);
                    painter->drawText(rect,mTracker->at(i).comment());
                    rect.setRect(tp.x()-pSC,tp.y()-pSC,50,50);
                    if (tp.markerID()>0)
                    {
                        painter->drawText(rect,QString("id=%1").arg(tp.markerID()));
                    }

                }

                if ((mControlWidget->trackShowColColor->checkState() == Qt::Checked) && (mTracker->at(i).color().isValid()))
                {
                    painter->setPen(Qt::NoPen);
                    painter->setBrush(QBrush(mTracker->at(i).color()));
                    rect.setRect(tp.x()+(pSP+pSC)*0.6*n.x()-pSC/2., tp.y()+(pSP+pSC)*0.6*n.y()-pSC/2., pSC, pSC); // 11
                    painter->drawEllipse(rect);
                }
                else if ((mControlWidget->trackShowColColor->checkState() == Qt::Checked) &&
                         ((height > MIN_HEIGHT) || ((tp.sp().z() > 0.) && (mControlWidget->trackShowHeightIndividual->checkState() == Qt::Checked)))) // Hoehe  && (mTracker->at(i).height() > 0.) Hoehe kann auf Treppen auch negativ werden, wenn koord weiter oben angesetzt wird
                {
                    painter->setFont(heightFont);
                    if ((mControlWidget->trackShowHeightIndividual->checkState() == Qt::Checked) && (tp.sp().z() > 0.)) // Hoehe incl individual fuer jeden trackpoint
                    {
                        painter->setPen(numberPen);
                        painter->setBrush(Qt::NoBrush);
                        rect.setRect(tp.x()+(pSP+pSC)*0.6*n.x()-pSC/2., tp.y()+(pSP+pSC)*0.6*n.y()-pSC/2., 3*pSC, 2.5*pSC); // 11
                        if (height < MIN_HEIGHT+1)
                        {
                            if (mControlWidget->getCalibCoordDimension() == 0) // 3D
                                painter->drawText(rect, Qt::AlignHCenter, QString("-\n%2").arg(-mControlWidget->getCalibExtrTrans3()-tp.sp().z(), 6, 'f', 1));
                            else
                                painter->drawText(rect, Qt::AlignHCenter, QString("-\n%2").arg(mControlWidget->coordAltitude->value()-tp.sp().z(), 6, 'f', 1));

                        }else
                        {
                            if (mControlWidget->getCalibCoordDimension() == 0) // 3D
                                painter->drawText(rect, Qt::AlignHCenter, QString("%1\n%2").arg(height, 6, 'f', 1).arg(-mControlWidget->getCalibExtrTrans3()-tp.sp().z(), 6, 'f', 1));
                            else
                                painter->drawText(rect, Qt::AlignHCenter, QString("%1\n%2").arg(height, 6, 'f', 1).arg(mControlWidget->coordAltitude->value()-tp.sp().z(), 6, 'f', 1));
                        }
                    }
                    else
                    {
                        painter->setPen(numberPen);
                        painter->setBrush(Qt::NoBrush);
                        rect.setRect(tp.x()+(pSP+pSC)*0.6*n.x()-pSC/2., tp.y()+(pSP+pSC)*0.6*n.y()-pSC/2., 3*pSC, 2*pSC); // 11
                        painter->drawText(rect, Qt::AlignHCenter, QString("%1").arg(height, 6, 'f', 1));
                    }
                    painter->setFont(font);
                }
                if (mControlWidget->trackShowNumber->checkState() == Qt::Checked)
                {
                    // listennummer
                    painter->setPen(numberPen);
                    painter->setBrush(Qt::NoBrush);
                    rect.setRect(tp.x()-(pSP+pSN)*0.6*n.x()-pSN, tp.y()-(pSP+pSN)*0.6*n.y()-pSN/2., 2.*pSN, pSN); // 11
                    painter->drawText(rect, Qt::AlignHCenter, QString("%1").arg(i+1));
                }
                if (mControlWidget->trackShowGroundPosition->checkState() == Qt::Checked)
                {
                    // ground position
                    painter->setPen(groundPositionPen);
                    painter->setBrush(Qt::NoBrush);
                    if( mControlWidget->getCalibCoordDimension() == 0) // 3D
                    {
                        double cross_size = 15+pSG*0.25;
                        //debout << "2D Point: (" << tp.x() << ", " << tp.y() << "), height: " << height << endl;
                        //debout << "mTracker.at(i).height(): " << mTracker->at(i).height() << " tp.sp().z(): " << tp.sp().z() << endl;
                        Point3f p3d_height;
                        if (height < MIN_HEIGHT+1)
                        {
                            p3d_height = mMainWindow->getExtrCalibration()->get3DPoint(Point2f(tp.x(),tp.y()),mControlWidget->getColorPlot()->map(mTracker->at(i).color()));
                        }else
                        {
                            if ( tp.sp().z() > 0 )
                                p3d_height = mMainWindow->getExtrCalibration()->get3DPoint(Point2f(tp.x(),tp.y()),-mControlWidget->getCalibExtrTrans3()-tp.sp().z());
                            else
                                p3d_height = mMainWindow->getExtrCalibration()->get3DPoint(Point2f(tp.x(),tp.y()),height/*mControlWidget->mapDefaultHeight->value()*/);
                        }
                        //debout << "3D Point: (" << p3d_height.x << ", " << p3d_height.y << ", " << p3d_height.z << ")" << endl;
                        p3d_height.z = 0;
                        Point2f p2d_ground = mMainWindow->getExtrCalibration()->getImagePoint(p3d_height);
                        QPointF axis = mMainWindow->getImageItem()->getCmPerPixel(p2d_ground.x, p2d_ground.y, 0);
                        //debout << "2D Point: (" << p2d_ground.x << ", " << p2d_ground.y << ")" << endl;
                        //rect.setRect(p2d_ground.x-cross_size/2., p2d_ground.y-cross_size/2., cross_size, cross_size);
                        painter->drawLine(QLineF(p2d_ground.x-cross_size*0.5*pow(axis.x(),-1),p2d_ground.y-cross_size*0.5*pow(axis.y(),-1),p2d_ground.x+cross_size*0.5*pow(axis.x(),-1),p2d_ground.y+cross_size*0.5*pow(axis.y(),-1)));
                        painter->drawLine(QLineF(p2d_ground.x-cross_size*0.5*pow(axis.x(),-1),p2d_ground.y+cross_size*0.5*pow(axis.y(),-1),p2d_ground.x+cross_size*0.5*pow(axis.x(),-1),p2d_ground.y-cross_size*0.5*pow(axis.y(),-1)));
                        //painter->drawEllipse(rect);
                        painter->drawLine(QLineF(p2d_ground.x,p2d_ground.y,tp.x(),tp.y()));



                    }else // 2D
                    {


                    }

                }
                if( mControlWidget->showVoronoiCells->checkState() == Qt::Checked)
                {
                    if( mControlWidget->getCalibCoordDimension() == 0 ) // 3D
                    {
                        Point3f p3d_height;
                        if (height < MIN_HEIGHT+1)
                        {
                            p3d_height = mMainWindow->getExtrCalibration()->get3DPoint(Point2f(tp.x(),tp.y()),mControlWidget->getColorPlot()->map(mTracker->at(i).color()));
                        }else
                        {
                            if ( tp.sp().z() > 0 )
                                p3d_height = mMainWindow->getExtrCalibration()->get3DPoint(Point2f(tp.x(),tp.y()),-mControlWidget->getCalibExtrTrans3()-tp.sp().z());
                            else
                                p3d_height = mMainWindow->getExtrCalibration()->get3DPoint(Point2f(tp.x(),tp.y()),height/*mControlWidget->mapDefaultHeight->value()*/);
                        }

                        debout << "insert P(" << p3d_height.x+x_offset << ", " << p3d_height.y+y_offset << ") to subdiv" << endl;

                        subdiv.insert(Point2f(x_switch>0 ? x_switch-p3d_height.x+x_offset : p3d_height.x+x_offset, y_switch>0 ? y_switch-p3d_height.y+y_offset : p3d_height.y+y_offset));// p2d_ground);

                    }
                }
            }
        
            if (((mControlWidget->trackShowPoints->checkState() == Qt::Checked) || (mControlWidget->trackShowPath->checkState() == Qt::Checked) || (mControlWidget->trackShowGroundPath->checkState() == Qt::Checked)) && ((mTracker->at(i).trackPointExist(curFrame)) || (mControlWidget->trackShowOnlyVisible->checkState() == Qt::Unchecked)))
            {
                if (mControlWidget->trackShowBefore->value() == -1)
                    from = 0;
                else
                {
                    from = curFrame-mTracker->at(i).firstFrame()-mControlWidget->trackShowBefore->value();// mTracker->at(i).firstFrame();
                    if (from < 0)
                        from = 0;
                }
                if (mControlWidget->trackShowAfter->value() == -1)
                    to = mTracker->at(i).size();
                else
                {
                    to = curFrame-mTracker->at(i).firstFrame()+mControlWidget->trackShowAfter->value()+1;// mTracker->at(i).firstFrame();
                    if (to > mTracker->at(i).size())
                        to = mTracker->at(i).size();
                }
                for (j = from; j < to; ++j) // ueber TrackPoint
                {
                    // path
                    if (mControlWidget->trackShowPath->checkState() == Qt::Checked)
                    {
                        if (j != from) // autom. > 0
                        {
                            painter->setPen(linePen);
                            painter->setBrush(Qt::NoBrush);
//                            if (i==45)
//                            {
//                                debout << mTracker->at(i).at(j-1) <<endl;
//                                debout << mTracker->at(i).at(j).toQPointF() <<endl <<endl;
//                            }
                            // nur Linie zeichnen, wenn x oder y sich unterscheidet, sonst Punkt
                            // die Unterscheidung ist noetig, da Qt sonst grosses quadrat beim ranzoomen zeichnet
                            if ((mTracker->at(i).at(j-1).toQPointF().x() != mTracker->at(i).at(j).toQPointF().x()) ||
                                (mTracker->at(i).at(j-1).toQPointF().y() != mTracker->at(i).at(j).toQPointF().y()))
                                painter->drawLine(mTracker->at(i).at(j-1).toQPointF(), mTracker->at(i).at(j).toQPointF());
                            else
                                painter->drawPoint(mTracker->at(i).at(j-1).toQPointF());
                        }
                    }
                    // path on ground
                    if (mControlWidget->trackShowGroundPath->checkState() == Qt::Checked)
                    {
                        if (j != from)
                        {

                            // ground position
                            painter->setPen(groundPathPen);
                            painter->setBrush(Qt::NoBrush);
                            if( mControlWidget->getCalibCoordDimension() == 0) // 3D
                            {
                                //debout << "2D Point: (" << tp.x() << ", " << tp.y() << "), height: " << height << endl;
                                //debout << "mTracker.at(i).height(): " << mTracker->at(i).height() << " tp.sp().z(): " << tp.sp().z() << endl;
                                Point3f p3d_height_p1, p3d_height_p2;
                                if (mTracker->at(i).height() < MIN_HEIGHT+1)
                                {
                                    p3d_height_p1 = mMainWindow->getExtrCalibration()->get3DPoint(Point2f(mTracker->at(i).at(j-1).x(),mTracker->at(i).at(j-1).y()),mControlWidget->getColorPlot()->map(mTracker->at(i).color()));
                                    p3d_height_p2 = mMainWindow->getExtrCalibration()->get3DPoint(Point2f(mTracker->at(i).at(j).x(),mTracker->at(i).at(j).y()),mControlWidget->getColorPlot()->map(mTracker->at(i).color()));
                                }else
                                {
                                    if ( mTracker->at(i).at(j-1).sp().z() > 0 && mTracker->at(i).at(j).sp().z() > 0 )
                                    {
                                        p3d_height_p1 = mMainWindow->getExtrCalibration()->get3DPoint(Point2f(mTracker->at(i).at(j-1).x(),mTracker->at(i).at(j-1).y()),-mControlWidget->getCalibExtrTrans3()-mTracker->at(i).at(j-1).sp().z());
                                        p3d_height_p2 = mMainWindow->getExtrCalibration()->get3DPoint(Point2f(mTracker->at(i).at(j).x(),mTracker->at(i).at(j).y()),-mControlWidget->getCalibExtrTrans3()-mTracker->at(i).at(j).sp().z());
                                    }else
                                    {
                                        p3d_height_p1 = mMainWindow->getExtrCalibration()->get3DPoint(Point2f(mTracker->at(i).at(j-1).x(),mTracker->at(i).at(j-1).y()),mTracker->at(i).height()/*mControlWidget->mapDefaultHeight->value()*/);
                                        p3d_height_p2 = mMainWindow->getExtrCalibration()->get3DPoint(Point2f(mTracker->at(i).at(j).x(),mTracker->at(i).at(j).y()),mTracker->at(i).height()/*mControlWidget->mapDefaultHeight->value()*/);
                                    }
                                }
                                //debout << "3D Point: (" << p3d_height.x << ", " << p3d_height.y << ", " << p3d_height.z << ")" << endl;
                                p3d_height_p1.z = 0;
                                p3d_height_p2.z = 0;
                                Point2f p2d_ground_p1 = mMainWindow->getExtrCalibration()->getImagePoint(p3d_height_p1);
                                Point2f p2d_ground_p2 = mMainWindow->getExtrCalibration()->getImagePoint(p3d_height_p2);
                                //QPointF axis = mMainWindow->getImageItem()->getCmPerPixel(p2d_ground.x, p2d_ground.y, 0);
                                //debout << "2D Point: (" << p2d_ground.x << ", " << p2d_ground.y << ")" << endl;
                                //rect.setRect(p2d_ground.x-pSG/2., p2d_ground.y-pSG/2., pSG, pSG);
                                //painter->drawLine(QLineF(p2d_ground.x-pSG*0.5*pow(axis.x(),-1),p2d_ground.y-pSG*0.5*pow(axis.y(),-1),p2d_ground.x+pSG*0.5*pow(axis.x(),-1),p2d_ground.y+pSG*0.5*pow(axis.y(),-1)));
                                //painter->drawLine(QLineF(p2d_ground.x-pSG*0.5*pow(axis.x(),-1),p2d_ground.y+pSG*0.5*pow(axis.y(),-1),p2d_ground.x+pSG*0.5*pow(axis.x(),-1),p2d_ground.y-pSG*0.5*pow(axis.y(),-1)));
                                //painter->drawEllipse(rect);
                                // nur Linie zeichnen, wenn x oder y sich unterscheidet, sonst Punkt
                                // die Unterscheidung ist noetig, da Qt sonst grosses quadrat beim ranzoomen zeichnet
                                if (p2d_ground_p1.x != p2d_ground_p2.x || p2d_ground_p1.y != p2d_ground_p2.y )
                                    painter->drawLine(QLineF(p2d_ground_p1.x,p2d_ground_p1.y,p2d_ground_p2.x,p2d_ground_p2.y));
                                else
                                    painter->drawPoint(p2d_ground_p1.x,p2d_ground_p1.y);
                            }else // 2D
                            {

                            }
                        }
                    }

                    // points before and after
                    if (mControlWidget->trackShowPoints->checkState() == Qt::Checked)
                    {
                        if (mTracker->at(i).firstFrame()+j != curFrame)
                        {
                            if ((mControlWidget->trackShowPointsColored->checkState() == Qt::Checked) && (mTracker->at(i).at(j).color().isValid()))
                            {
                                //debout << "person: " << i+1 << " frame: " << j << " color: " << mTracker->at(i).at(j).color() <<endl;
                                painter->setPen(Qt::NoPen);
                                painter->setBrush(QBrush(mTracker->at(i).at(j).color()));
                                rect.setRect(mTracker->at(i).at(j).x()-pS/2., mTracker->at(i).at(j).y()-pS/2., pS, pS); // 7
                            }
                            else
                            {
                                painter->setPen(Qt::red);
                                painter->setBrush(Qt::NoBrush);
                                // war noetig fuer alte qt-version: rect.setRect(mTracker->at(i).at(j).x()-(pS-1.)/2., mTracker->at(i).at(j).y()-(pS-1.)/2., pS-1., pS-1.); // 6
                                rect.setRect(mTracker->at(i).at(j).x()-pS/2., mTracker->at(i).at(j).y()-pS/2., pS, pS);
                            }
                            painter->drawEllipse(rect);
                        }
                    }
                }
            }
        }
    }

    // Mat& img, Subdiv2D& subdiv )
    if( mControlWidget->showVoronoiCells->checkState() == Qt::Checked && !mTracker->isEmpty() )
    {

        vector<vector<Point2f> > facets3D;
        vector<Point2f> centers3D;

        // get Voronoi cell info from subDiv in 3D coordinates on ground (z=0)
        subdiv.getVoronoiFacetList(vector<int>(), facets3D, centers3D);

        painter->setClipRect(mMainWindow->getRecoRoiItem()->rect());//0,0,mMainWindow->getImage()->width(),mMainWindow->getImage()->height());

        // cell by cell
        for( size_t i = 0; i < facets3D.size(); i++ )
        {
            centers3D.at(i).x = x_switch>0 ? x_switch-centers3D.at(i).x-x_offset : centers3D.at(i).x-x_offset;
            centers3D.at(i).y = y_switch>0 ? y_switch-centers3D.at(i).y-y_offset : centers3D.at(i).y-y_offset;
            // voronoi cell center in 2D
            Point2f center2D = mMainWindow->getExtrCalibration()->getImagePoint(Point3f(centers3D.at(i).x,
                                                                                        centers3D.at(i).y,0));

            vector<QPointF> ifacet2D;//[facets[i].size()];//.resize(facets[i].size());
            QPointF circleStart, circleEnd;
            float area = 0;
            float r = 50, m = 0, n = 0, s1_x = 0, s2_x = 0, s1_y = 0, s2_y = 0;
            bool circleStarted = false, circleEnded = false;
            for( size_t j = 0; j < facets3D[i].size(); j++ )
            {
                facets3D.at(i).at(j).x = x_switch>0 ? x_switch-facets3D.at(i).at(j).x-x_offset : facets3D.at(i).at(j).x-x_offset;
                facets3D.at(i).at(j).y = y_switch>0 ? y_switch-facets3D.at(i).at(j).y-y_offset : facets3D.at(i).at(j).y-y_offset;

                Point2f point2D = mMainWindow->getExtrCalibration()->getImagePoint(Point3f(facets3D.at(i).at(j).x,facets3D.at(i).at(j).y,0));

                debout << "facets3D.at(" << i << ").at(" << j << ").x = " << facets3D.at(i).at(j).x << ", .y = " << facets3D.at(i).at(j).y << endl;
                debout << "point2D.x = " << point2D.x << " , .y = " << point2D.y << endl;
                //            ifacet[j] = QPointF(facets.at(i).at(j).x,facets.at(i).at(j).y);//facets[i][j];

                if ( false && sqrt(pow((facets3D.at(i).at(j).x - centers3D.at(i).x),2) +
                                   pow((facets3D.at(i).at(j).y - centers3D.at(i).y),2)) > r )
                {
                    if (circleStarted)
                    {

                        m = (facets3D.at(i).at(j).y - facets3D.at(i).at((j-1)%facets3D.at(i).size()).y) /
                                (facets3D.at(i).at(j).x - facets3D.at(i).at((j-1)%facets3D.at(i).size()).x);

                        // End punkt berechnen (Schnittpunkt Gerade-Kreis)
                        // Steigung der Geraden
                        m = (facets3D.at(i).at(j).y - facets3D.at(i).at((j+1)%facets3D.at(i).size()).y) /
                                (facets3D.at(i).at(j).x - facets3D.at(i).at((j+1)%facets3D.at(i).size()).x);
                        // Achsenabschnitt der Geraden
                        n = facets3D.at(i).at(j).y - m * facets3D.at(i).at(j).x;

                        float p = -( (m*n - m*centers3D.at(i).y - centers3D.at(i).x)/(1+pow(m,2)) );
                        float q = sqrt((pow(r,2)-pow(centers3D.at(i).x,2)-pow(centers3D.at(i).y,2)-pow(n,2)-2*n*centers3D.at(i).y) / (1+pow(m,2)) +
                                       pow((m*n-m*centers3D.at(i).y-centers3D.at(i).x)/(1+pow(m,2)),2));
                        // Schnittpunkte mit Kreis
                        s1_x = p + q;
                        s1_y = m*s1_x+n;

                        s2_x = p - q;
                        s2_y = m*s2_x+n;

                        facets3D[i][j] = Point2f(s1_x,s1_y);

                        point2D = mMainWindow->getExtrCalibration()->getImagePoint(Point3f(s1_x,s1_y,0));
                        circleEnd = QPointF(point2D.x,point2D.y);
                        //
                        //                    painter->setBrush(Qt::blue);
                        //                    point2D = mMainWindow->getExtrCalibration()->getImagePoint(Point3f(facets3D.at(i).at(j).x,facets3D.at(i).at(j).y,0));
                        //                    painter->drawEllipse(QPointF(point2D.x,point2D.y),10,10);
                        //                    point2D = mMainWindow->getExtrCalibration()->getImagePoint(Point3f(facets3D.at(i).at((j+1)%facets3D.at(i).size()).x,facets3D.at(i).at((j+1)%facets3D.at(i).size()).y,0));
                        //                    painter->drawEllipse(QPointF(point2D.x,point2D.y),10,10);

                        ifacet2D.push_back(QPointF(center2D.x,center2D.y));
                        ifacet2D.push_back(QPointF(point2D.x,point2D.y));
                        //
                        //                      painter->drawLine(circleEnd,QPointF(center2D.x,center2D.y));
                        debout << "End point: (" << s1_x << ", " << s1_y << ")" << endl;
                        //                    area += (M_PI*pow(r,2)*angle/360);
                        circleEnded = true;

                    }else
                    {
                        // start punkt berechnen

                        m = (facets3D.at(i).at(j).y - facets3D.at(i).at((j-1)%facets3D.at(i).size()).y) /
                                (facets3D.at(i).at(j).x - facets3D.at(i).at((j-1)%facets3D.at(i).size()).x);

                        n = facets3D.at(i).at(j).y - m * facets3D.at(i).at(j).x;

                        float p = -( (m*n - m*centers3D.at(i).y - centers3D.at(i).x)/(1+pow(m,2)) );
                        float q = sqrt((pow(r,2)-pow(centers3D.at(i).x,2)-pow(centers3D.at(i).y,2)-pow(n,2)-2*n*centers3D.at(i).y) / (1+pow(m,2)) +
                                       pow((m*n-m*centers3D.at(i).y-centers3D.at(i).x)/(1+pow(m,2)),2));

                        // Schnittpunkte mit Kreis
                        s1_x = p + q;
                        s1_y = m*s1_x+n;

                        s2_x = p - q;
                        s2_y = m*s2_x+n;

                        debout << "x=" << s1_x << " G(x)=" << (m*s1_x+n) << " K(x)=" << pow(s1_x-centers3D.at(i).x,2)+pow(s1_y-centers3D.at(i).y,2) << " = " << pow(r,2) << endl;
                        debout << "x=" << s2_x << " G(x)=" << (m*s2_x+n) << " K(x)=" << pow(s2_x-centers3D.at(i).x,2)+pow(s2_y-centers3D.at(i).y,2) << " = " << pow(r,2) << endl;

                        facets3D[i][j] = Point2f(s1_x,s1_y);

                        point2D = mMainWindow->getExtrCalibration()->getImagePoint(Point3f(s1_x,s1_y,0));
                        ifacet2D.push_back(QPointF(point2D.x,point2D.y));
                        //                    facets3D[i][j] = Point2f(s1_x,m*s1_x+n);
                        circleStart = QPointF(point2D.x,point2D.y);
                        circleStarted = true;
                        //                    painter->drawLine(circleStart,QPointF(center2D.x,center2D.y));

                        debout << "Start point: (" << s1_x << ", " << s1_y << ")" << endl;
                    }

                }else
                {
                    //                if (facets3D.at(i).at(j).x > -450 && facets3D.at(i).at(j).x < 900 && facets3D.at(i).at(j).y > -450 && facets3D.at(i).at(j).y < 1250 )
                    //                {
                    ifacet2D.push_back(QPointF(point2D.x,point2D.y));
                    area += (facets3D.at(i).at(j).x*facets3D.at(i).at((j+1)%facets3D[i].size()).y);
                    area -= (facets3D.at(i).at((j+1)%facets3D[i].size()).x*facets3D.at(i).at(j).y);
                    //                }
                }
                //            for( size_t k = 0; k < j; k++)
                //            {
                //                if( sqrt(pow((facets.at(i).at(j).x-facets.at(i).at(k).x),2) +
                //                         pow((facets.at(i).at(j).y-facets.at(i).at(k).y),2)) > 100 )
                //                    toBig = true;
                //            }
                //if( toBig )
                //    break;
            }
            //if( toBig )
            //    continue;
            area *= 0.5;
            area = 1.0/area;//*= 0.00002;
            area *= 10000;
            //        debout << "area: " << area << endl;

            QColor color;

            //        color.setRgb(125,125,125,125);
            color.setHsv((255-area*25.5)<0?0:(255-area*25.5),255,255,/*toBig ? 20 : */128);

//            QPointF ifacet_array[ifacet2D.size()];
            QVector<QPointF> ifacet_vec;
            for(int i=0; i<ifacet2D.size(); i++)
            {
//                ifacet_array[i] = ifacet2D.at(i);
                ifacet_vec.append(ifacet2D.at(i));
            }

            //        color[0] = rand() & 255;
            //        color[1] = rand() & 255;
            //        color[2] = rand() & 255;
            painter->setBrush(color);//QColor(rand()&255,rand()&255,rand()&255,128)));
            painter->setPen(/*toBig ? Qt::transparent : */Qt::black);
            if( ifacet2D.size() == 0 )
                painter->drawEllipse(QPointF(center2D.x,center2D.y),100,100);
            //else
            // voronoi cell
            //        if (circleStarted)
            //            painter->drawLine(circleStart,circleEnd);
            //        painter->drawPolygon(ifacet_array,ifacet2D.size());
//            painter->drawConvexPolygon(ifacet_array,ifacet2D.size());
            painter->drawConvexPolygon(QPolygonF(ifacet_vec));
            //        fillConvexPoly(img, ifacet, color, 8, 0);
            //        if (circleStarted)
            QPointF cmPerPixel = mMainWindow->getImageItem()->getCmPerPixel(center2D.x,center2D.y);
            //        painter->drawPie(center2D.x-50/cmPerPixel.x(),center2D.y-50/cmPerPixel.y(),100/cmPerPixel.x(),100/cmPerPixel.y(),0*16,360*16);
            //        ifacets[0] = ifacet;
            //        polylines(img, ifacets, true, Scalar(), 1, CV_AA, 0);
            //        circle(img, centers[i], 3, Scalar(), CV_FILLED, CV_AA, 0);
            //        painter->setPen(Qt::PenStyle);
            //        painter->drawPoint(centers.at(i).x,centers.at(i).y);
            //        fillConvexPoly(cvarrToMat(mMainWindow->getIplImageFiltered()),ifacet_array, Scalar(125,125,125), 8, 0);

            // voronoi cell point
            painter->setBrush(Qt::black);
            painter->setPen(Qt::red);
            painter->drawEllipse(QPointF(center2D.x,center2D.y),5,5);
            //        if( circleStarted && circleEnded)
            //            i = facets3D.size();

        }

    }



}
