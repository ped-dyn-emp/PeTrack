#include <algorithm>
#include <math.h>
#include <QtWidgets>

#include "petrack.h"
#include "control.h"
#include "view.h"
#include "coordItem.h"
#include "extrCalibration.h"

using namespace::cv;

// in x und y gleichermassen skaliertes koordinatensystem,
// da von einer vorherigen intrinsischen kamerakalibrierung ausgegenagen wird,
// so dass pixel quadratisch 
CoordItem::CoordItem(QWidget *wParent,/* QGraphicsScene * scene,*/ QGraphicsItem * parent)
    : QGraphicsItem(parent/*, scene*/)
{
    mMainWindow = (class Petrack*) wParent;
    extCalib = mMainWindow->getExtrCalibration();
    mControlWidget = mMainWindow->getControlWidget();
    //mScene = scene;

    //Set Min and Max
    calibPointsMin.x = 50000;
    calibPointsMax.x = 0;
    calibPointsMin.y = 50000;
    calibPointsMax.y = 0;

    updateData(); // um zB setFlags(ItemIsMovable) je nach anzeige zu aendern
    //setEnabled(false); // all mouse events connot access this item, but it will be seen
    //einzig move koennte interessant sein, um grid zu verschieben?!

    //setAcceptsHoverEvents(true);
}
// bounding box wird durch linke obere ecke und breite/hoehe angegeben
// wenn an den rand gescrollt wurde im view, dann wird durch das dynamische anpassen
// bei trans und scale zwar zuerst alles neu gezeichnet durch update, 
// aber beim verkleinern des scrollbereichs nur der teil von coord neu gezeichnet
QRectF CoordItem::boundingRect() const
{
   /* double sc = mControlWidget->getCalibCoordScale()/10;
    double tX = mControlWidget->getCalibCoordTransX();
    double tY = mControlWidget->getCalibCoordTransY();
    double tZ = mControlWidget->getCalibCoordRotate();
    double cU = mControlWidget->getCalibCoordUnit();

    Point2f p[4];
    p[0] = extCalib->getImagePoint(Point3f(tX+cU,tY,tZ));
    p[1] = extCalib->getImagePoint(Point3f(tX,tY+cU,tZ));
    p[2] = extCalib->getImagePoint(Point3f(tX,tY,tZ+cU));
    p[3] = extCalib->getImagePoint(Point3f(0,0,0));
    double min_x = 1000, min_y = 1000, max_x = -1000, max_y = -1000;

    double x,y;

    for(int i=0;i<4;i++)
    {
        x = p[i].x;
        y = p[i].y;
        max_x = x > max_x ? x : max_x;
        min_x = x < min_x ? x : min_x;
        max_y = y > max_y ? y : max_y;
        min_y = y < min_y ? y : min_y;
    }*/

    // bounding box wird in lokalen koordinaten angegeben!!! (+-10 wegen zahl "1")

    if (mControlWidget->getCalibCoordShow() /*|| mControlWidget->getCalibExtrCalibPointsShow()*/)
    {
        //debout << "Items pos: " << this->pos() << " scenePos(): " << this->scenePos() << endl;

        if( !mControlWidget->getCalibCoordDimension() == 0 ) // 2D view
        {
            return QRectF(-110., -110., 220., 220.);
        }
        else // 3D view
        {
            //debout << x.x << " " << y.x << " " << z.x << " " << ursprung.x << " " << calibPointsMin.x << " " << calibPointsMax.x << endl;
            double min_x = min(min(x.x,y.x),min(z.x,ursprung.x));
            double max_x = max(max(x.x,y.x),max(z.x,ursprung.x));

            //debout << x.y << " " << y.y << " " << z.y << " " << ursprung.y << " " << calibPointsMin.y << " " << calibPointsMax.y << endl;
            double min_y = min(min(x.y,y.y),min(z.y,ursprung.y));
            double max_y = max(max(x.y,y.y),max(z.y,ursprung.y));

            if( mControlWidget->getCalibExtrCalibPointsShow() )
            {
                min_x = min(float(min_x),calibPointsMin.x);
                max_x = max(float(max_x),calibPointsMax.x);

                min_y = min(float(min_y),calibPointsMin.y);
                max_y = max(float(max_y),calibPointsMax.y);
            }
            //debout << "Bounding Rect: x: " << min_x << " y: " << min_y << ", width: " << max_x << " height: " << max_y << endl;
            //debout << "Bounding Rect: " << mapToScene(min_x,min_y) << ", " << mapToItem(this,max_x-min_x,max_y-min_y) << endl;
            //return QRectF(mapToItem( this, min_x, min_y),mapToItem( this,max_x-min_x,max_y-min_y));
            //double delta_x = max_x-min_x, delta_y = max_y-min_y;
            //return QRectF(-0.5*delta_x,-0.5*delta_y,delta_x, delta_y);
            return QRectF(min_x-25, min_y-25, max_x-min_x+50, max_y-min_y+50);
//            return QRectF(-mMainWindow->getImageBorderSize(),
//                          -mMainWindow->getImageBorderSize(),
//                          mControlWidget->getMainWindow()->getImage()->width()
//                          + 2*mMainWindow->getImageBorderSize(),
//                          mControlWidget->getMainWindow()->getImage()->height()
//                          + 2*mMainWindow->getImageBorderSize());
            //return QRectF(-img->width(),-img->height(),img->width()*2,img->height()*2);
        }
    }
    else
    {
        return QRectF(0., 0., 0., 0.);
    }

    // sicher ware diese boundingbox, da alles
    //     return QRectF(xMin, yMin, xMax-xMin, yMax-yMin);
    // eigentlich muesste folgende Zeile reichen, aber beim ranzoomen verschwindet dann koord.sys.
    //     return QRectF(mControlWidget->getCalibCoordTransX()/10.-scale, mControlWidget->getCalibCoordTransY()/10.-scale, 2*scale, 2*scale);
}

// event, of moving mouse while button is pressed
void CoordItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{

    // if coordinate system position is not fixed
    if (!mControlWidget->getCalibCoordFix())
    {

        setCursor(Qt::ClosedHandCursor);

        QPointF diff = event->scenePos()-event->lastScenePos(); //screenPos()-buttonDownScreenPos(Qt::RightButton) also interesting
        if (event->buttons() == Qt::RightButton) // event->button() doesnt work
        {
            mControlWidget->setCalibCoordRotate(mControlWidget->getCalibCoordRotate()-(int)(3.*(diff.x()+diff.y()))); //10* nicht noetig, da eh nur relativ
                //cout << diff.x()-diff.y() <<endl; //globalPos()
        }
        else if (event->buttons() == Qt::LeftButton)
        {
            if( mControlWidget->getCalibCoordDimension() == 0 ) // 3D
            {

                //debout << "P_cur: " << event->scenePos().x() << "," << event->scenePos().y() << endl;
                //debout << "P_last: " << event->lastScenePos().x() << "," << event->lastScenePos().y() << endl;

                Point3f p_cur = extCalib->get3DPoint(Point2f(event->scenePos().x(),
                                                             event->scenePos().y()),
                                                     mControlWidget->getCalibCoord3DTransZ());
                Point3f p_last = extCalib->get3DPoint(Point2f(mouse_x/*event->lastScenePos().x()*/,
                                                              mouse_y/*event->lastScenePos().y()*/),
                                                      mControlWidget->getCalibCoord3DTransZ());
                // ToDo:
                // Problem: Die Mouse Bewegungen, die erfasst werden sind zu gering, sodass viele Werte als 0 gewertet werden
                // und nicht als Bewegung bercksichtigt werden, obwohl man die Maus bewegt. D.h. die Maus bewegt sich
                // der Abstand zwischen dem Start und dem End-punkt ist aber sehr gering und wird als 0 gewertet und
                // das Kooridnatensystem bewegt sich nicht. Effekt wird noch verstrkt, da das stndig passiert
                // Besonders schnelle Mausbewegungen lindern den Effekt

                mControlWidget->setCalibCoord3DTransX(coordTrans_x/*mControlWidget->getCalibCoord3DTransX()*/-(mControlWidget->getCalibCoord3DSwapX() ? -1 : 1)*round(p_last.x-p_cur.x));
                mControlWidget->setCalibCoord3DTransY(coordTrans_y/*mControlWidget->getCalibCoord3DTransY()*/-(mControlWidget->getCalibCoord3DSwapY() ? -1 : 1)*round(p_last.y-p_cur.y));

//                mControlWidget->setCalibExtrTrans1(mControlWidget->getCalibExtrTrans1()+round(p_cur.x-p_last.x));
//                mControlWidget->setCalibExtrTrans2(mControlWidget->getCalibExtrTrans2()+round(p_cur.y-p_last.y));

            }else
            {
                mControlWidget->setCalibCoordTransX(mControlWidget->getCalibCoordTransX()+(int)(10.*diff.x()));
                mControlWidget->setCalibCoordTransY(mControlWidget->getCalibCoordTransY()+(int)(10.*diff.y()));
            }
        }
        else if (event->buttons() == Qt::MidButton)
        {
            if( mControlWidget->getCalibCoordDimension() == 0 )
            {
                mControlWidget->setCalibCoord3DAxeLen(mControlWidget->getCalibCoord3DAxeLen()+(int)(10.*(diff.x()-diff.y())));
            }else
            {
                mControlWidget->setCalibCoordScale(mControlWidget->getCalibCoordScale()+(int)(10.*(diff.x()-diff.y())));
            }
        }
    }
    else
    {
        QGraphicsItem::mouseMoveEvent(event); // drag mach ich selber
    }
    //mMainWindow->updateSceneRect();

}

void CoordItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!mControlWidget->getCalibCoordFix())
    {
        if (event->button() == Qt::LeftButton)
        {
            mouse_x = event->scenePos().x();
            mouse_y = event->scenePos().y();

            coordTrans_x = mControlWidget->getCalibCoord3DTransX();
            coordTrans_y = mControlWidget->getCalibCoord3DTransY();

           //debout << ">>>>>>>>MousePressEvent: " << mouse_x << ", " << mouse_y << endl;
        }
    }else
    {
        QGraphicsItem::mousePressEvent(event);
    }
}


// event, of moving mouse
//void CoordItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
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

//    QGraphicsItem::hoverMoveEvent(event);
//}

void CoordItem::updateData()
{
    if (!mControlWidget->getCalibCoordFix())
         setFlag(ItemIsMovable); // noetig, damit mouseEvent leftmousebutton weitergegeben wird, aber drag mach ich selber
    else
       setFlag(ItemIsMovable, false); // noetig, damit mouseEvent leftmousebutton weitergegeben wird, aber drag mach ich selber

    if( mControlWidget->getCalibCoordDimension() == 1 ) // 2D
    {
        double sc = mControlWidget->getCalibCoordScale()/10.;
        double tX = mControlWidget->getCalibCoordTransX()/10.;
        double tY = mControlWidget->getCalibCoordTransY()/10.;
        double ro = mControlWidget->getCalibCoordRotate()/10.;

        // aktualisierung der transformationsmatrix
        QTransform matrix;
        // matrix wird nur bei aenderungen neu bestimmt
        matrix.translate(tX, tY);
        matrix.rotate(ro);
        matrix.scale(sc/100., sc/100.);
        //matrix.shear(tX,tY);
        setTransform(matrix);

    }else // 3D
    {
        ////////////////////////////////////////
        //     3D World-Coordinate-System     //
        ////////////////////////////////////////
        if( mMainWindow->getImage())
        {

        //prepareGeometryChange();
        // Reset Matrix - No Matrix Transformations for 3D Coordsystem
        // aktualisierung der transformationsmatrix
        QTransform matrix;
        // matrix wird nur bei aenderungen neu bestimmt
        matrix.translate(0, 0);
        matrix.rotate(0);
        matrix.scale(1, 1);
        //matrix.shear(tX,tY);
        setTransform(matrix);

//        double tX3D = mControlWidget->getCalibCoord3DTransX();
//        double tY3D = mControlWidget->getCalibCoord3DTransY();
//        double tZ3D = mControlWidget->getCalibCoord3DTransZ();
        double axeLen = mControlWidget->getCalibCoord3DAxeLen();
        int bS = mMainWindow->getImageBorderSize();
//        int swapX = 1;//mControlWidget->getCalibCoord3DSwapX() ? -1 : 1;
//        int swapY = 1;//mControlWidget->getCalibCoord3DSwapY() ? -1 : 1;
//        int swapZ = 1;//mControlWidget->getCalibCoord3DSwapZ() ? -1 : 1;


        //debout << "Before isSetExtrCalib..." << endl;
        // Coordinate-system origin at (tX,tY,tZ)
        if( extCalib->isSetExtrCalib() ){

            //debout << "X: swap: " << swapX << " T1: " << mControlWidget->getCalibExtrTrans1() << " tX3D: " << tX3D << endl;
            //debout << "Y: swap: " << swapY << " T2: " << mControlWidget->getCalibExtrTrans2() << " tY3D: " << tY3D << endl;
            //debout << "Z: swap: " << swapZ << " T3: " << mControlWidget->getCalibExtrTrans3() << " tZ3D: " << tZ3D << endl;

            ursprung = extCalib->getImagePoint(Point3f(0,0,0));

            bool ursprungOutside = false;

            x3D = Point3f(axeLen,0,0);
            y3D = Point3f(0,axeLen,0);
            z3D = Point3f(0,0,axeLen);

            // Tests if the origin-point of the coordinate-system is outside the image
            if( extCalib->isOutsideImage(ursprung) )
            {
                //debout << "Ursprung: " << ursprung.x << ", " <<ursprung.y << endl;
                ursprungOutside = true;
                return;
            }
            //debout << "Test" << endl;
//            x3D.x += swapX;
//            y3D.y += swapY;
//            z3D.z += swapZ;
            x3D.x++;
            y3D.y++;
            z3D.z++;

            // Kuerzt die Koordinaten-Achsen, falls sie aus dem angezeigten Bild raus laufen wuerden
            do{
                x3D.x--;
                x = extCalib->getImagePoint(x3D);
                //if( ursprungOutside ){ break; }
                //debout << x.x << ", " << x.y << endl;
                // tests if the coord system axis are inside the view or outside, if outside short them till they are inside the image
            }while( x.x < -bS || x.x > mMainWindow->getImage()->width()-bS || x.y < -bS || x.y > mMainWindow->getImage()->height()-bS );
            do{
                y3D.y--;
                y = extCalib->getImagePoint(y3D);
                //if( ursprungOutside ){ break; }
                //debout << y.x << ", " << y.y << endl;
            }while( y.x < -bS || y.x > mMainWindow->getImage()->width()-bS || y.y < -bS || y.y > mMainWindow->getImage()->height()-bS );
            do{
                z3D.z--;
                z = extCalib->getImagePoint(z3D);
                //if( ursprungOutside ){ break; }
                //debout << z.x << ", " << z.y << endl;
            }while( z.x < -bS || z.x > mMainWindow->getImage()->width()-bS || z.y < -bS || z.y > mMainWindow->getImage()->height()-bS );
        }
        }
    //}

        prepareGeometryChange();

    }
    if (!mMainWindow->isLoading()){
        mMainWindow->updateImage();
    }
}

void CoordItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    bool debug = false;


    int bS = mMainWindow->getImageBorderSize();

    if( debug ) debout << "**************** CoordItem:paint() aufgerufen" << endl;
    if( debug ) debout << "show coord: " << mControlWidget->getCalibCoordShow() << endl;
    ////////////////////////////////
    // Drawing Calibration Points //
    ////////////////////////////////
    if( mControlWidget->getCalibExtrCalibPointsShow() && mControlWidget->getCalibCoordDimension() == 0 )
    {

        if( extCalib->isSetExtrCalib() )
        {

            QFont font;
            font.setBold(mControlWidget->trackNumberBold->checkState() == Qt::Checked);
            font.setPixelSize(mControlWidget->trackNumberSize->value());

            painter->setFont(font);

            for(int i=0; i< extCalib->get2DList().size(); i++)
            {
                painter->setPen(Qt::red);
                painter->setBrush(Qt::NoBrush);
                // Original 2D-Pixel-Points
                Point2f p2 = extCalib->get2DList().at(i);
                painter->drawEllipse(p2.x-8,p2.y-8,16,16);

                // general configuration
                painter->setPen(Qt::blue);
                painter->setBrush(Qt::blue);

                // Projected 3D-Points
                Point3f p3d = extCalib->get3DList().at(i);
                p3d.x -= mControlWidget->getCalibCoord3DTransX();
                p3d.y -= mControlWidget->getCalibCoord3DTransY();
                p3d.z -= mControlWidget->getCalibCoord3DTransZ();

                p3d.x *= (mControlWidget->getCalibCoord3DSwapX() ? -1 : 1);
                p3d.y *= (mControlWidget->getCalibCoord3DSwapY() ? -1 : 1);
                p3d.z *= (mControlWidget->getCalibCoord3DSwapZ() ? -1 : 1);

                Point2f p3 = extCalib->getImagePoint(p3d);

                //QPointF axis = mMainWindow->getImageItem()->getCmPerPixel(p3.x,p3.y,p3d.z);

                painter->drawEllipse(p3.x-4,p3.y-4,8,8);

                // Connecting-line Pixel-3D-Points
                painter->drawLine(QPointF(p2.x,p2.y),QPointF(p3.x,p3.y));

                // Show point number
                painter->setPen(Qt::black);
                painter->setBrush(Qt::black);
                painter->drawText(QPointF(p2.x+10,p2.y+font.pixelSize()), QObject::tr("%1").arg((i+1)));

                //painter->setPen(Qt::blue);
                //painter->setBrush(Qt::blue);

                // min/max of calibration points for bounding box
//                if( i == 0)
//                {
//                    calibPointsMin.x = p2.x;
//                    calibPointsMin.y = p2.y;
//                    calibPointsMax.x = p2.x;
//                    calibPointsMax.y = p2.y;
//                }
                if( p2.x < calibPointsMin.x ) calibPointsMin.x = p2.x;
                if( p2.x > calibPointsMax.x ) calibPointsMax.x = p2.x;
                if( p3.x < calibPointsMin.x ) calibPointsMin.x = p3.x;
                if( p3.x > calibPointsMax.x ) calibPointsMax.x = p3.x;

                if( p2.y < calibPointsMin.y ) calibPointsMin.y = p2.y;
                if( p2.y > calibPointsMax.y ) calibPointsMax.y = p2.y;
                if( p3.y < calibPointsMin.y ) calibPointsMin.y = p3.y;
                if( p3.y > calibPointsMax.y ) calibPointsMax.y = p3.y;

            }
        }
    }

    if (mControlWidget->getCalibCoordShow())
    {
        // general configuration
        painter->setPen(Qt::blue);
        painter->setBrush(QBrush(Qt::blue,Qt::SolidPattern));

        if ( mControlWidget->getCalibCoordDimension() == 1 ) // 2D
        {
            //////////////////////////
            // 2D Coordinate-System //
            //////////////////////////

            painter->setPen(QPen(QBrush(Qt::blue),0));
//            double sc = mControlWidget->getCalibCoordScale();
//            double tX = mControlWidget->getCalibCoordTransX();
//            double tY = mControlWidget->getCalibCoordTransY();
//            double tZ = mControlWidget->getCalibCoordRotate();
//            double cU = mControlWidget->getCalibCoordUnit();

            static QPointF points[3];

            //         //eigentliche BoundingBox: painter->drawRect(QRectF(mControlWidget->getCalibCoordTransX()/10.-scale, mControlWidget->getCalibCoordTransY()/10.-scale, 2*scale, 2*scale));
            //         matrix.translate(mControlWidget->getCalibCoordTransX()/10., mControlWidget->getCalibCoordTransY()/10.);
            //         matrix.rotate((mControlWidget->getCalibCoordRotate())/10.);
            // mit folgender zeile wuerde nur der painter bewegt, nicht aber das koord des items!!!
            //         painter->setWorldMatrix(matrix, true); // true sagt, dass relativ und nicht absolut (also zusaetzlichj zur uebergeordneten matrizen)

            // Koordinatenachsen
            points[0].setX(-10.); points[0].setY(0.);
            points[1].setX(100.);     points[1].setY(0.);
            painter->drawLine(points[0], points[1]);

            //debout << "1. Points[0]: " << points[0].x() << " " << points[0].y() << endl;
            //debout << "1. Points[1]: " << points[1].x() << " " << points[1].y() << endl;

            points[0].setX(0.); points[0].setY(10.);
            points[1].setX(0.); points[1].setY(-100.);
            painter->drawLine(points[0], points[1]);

            //debout << "2. Points[0]: " << points[0].x() << " " << points[0].y() << endl;
            //debout << "2. Points[1]: " << points[1].x() << " " << points[1].y() << endl;

            // Ticks
            for (int i=1; i<11; i++) // i=10 zeichnen sieht ungewoehnlich aus, laeest sich aber besser mit messen
            {
                points[0].setX(2.);  points[0].setY(-i*10.);
                points[1].setX(-2.); points[1].setY(-i*10.);
                painter->drawLine(points[0], points[1]);
                points[0].setX(i*10.); points[0].setY(2.);
                points[1].setX(i*10.); points[1].setY(-2.);
                painter->drawLine(points[0], points[1]);
            }

            // Beschriftung
            points[0].setX(97.); points[0].setY(12.);
            painter->drawText(points[0], QObject::tr("1"));
            points[0].setX(-8.); points[0].setY(-97.);
            painter->drawText(points[0], QObject::tr("1"));

            // Pfeilspitzen
            painter->setPen(Qt::NoPen);
            painter->setBrush(Qt::blue);

            points[0].setX(100.);points[0].setY(0.);
            points[1].setX(95.); points[1].setY(2.);
            points[2].setX(95.); points[2].setY(-2.);
            painter->drawPolygon(points, 3);

            points[0].setX(0.);  points[0].setY(-100.);
            points[1].setX(2.);  points[1].setY(-95.);
            points[2].setX(-2.); points[2].setY(-95.);
            painter->drawPolygon(points, 3);

            /*
            //         // Koordinatenachsen
            //         painter->setPen(Qt::blue);

            //         points[0].setX(-.1*scale); points[0].setY(0.);
            //         points[1].setX(scale);     points[1].setY(0.);
            //         painter->drawLine(points[0], points[1]);

            //         points[0].setX(0.); points[0].setY(.1*scale);
            //         points[1].setX(0.); points[1].setY(-scale);
            //         painter->drawLine(points[0], points[1]);

            //         // Ticks
            //         for (int i=1; i<10; i++)
            //         {
            //             points[0].setX(0.02*scale);  points[0].setY(-i*scale/10.);
            //             points[1].setX(-0.02*scale); points[1].setY(-i*scale/10.);
            //             painter->drawLine(points[0], points[1]);
            //             points[0].setX(i*scale/10.); points[0].setY(0.02*scale);
            //             points[1].setX(i*scale/10.); points[1].setY(-0.02*scale);
            //             painter->drawLine(points[0], points[1]);
            //         }

            //         // Beschriftung
            //         points[0].setX(0.97*scale); points[0].setY(0.12*scale);
            //         painter->drawText(points[0], QObject::tr("1"));
            //         points[0].setX(-0.08*scale); points[0].setY(-0.97*scale);
            //         painter->drawText(points[0], QObject::tr("1"));

            //         // Pfeilspitzen
            //         painter->setPen(Qt::NoPen);
            //         painter->setBrush(Qt::blue);

            //         points[0].setX(scale);      points[0].setY(0.);
            //         points[1].setX(0.95*scale); points[1].setY(0.02*scale);
            //         points[2].setX(0.95*scale); points[2].setY(-0.02*scale);
            //         painter->drawPolygon(points, 3);

            //         points[0].setX(0.);           points[0].setY(-scale);
            //         points[1].setX(0.02*scale);  points[1].setY(-0.95*scale);
            //         points[2].setX(-0.02*scale); points[2].setY(-0.95*scale);
            //         painter->drawPolygon(points, 3);
            */

        }else
        {
            double tX3D = mControlWidget->getCalibCoord3DTransX();
            double tY3D = mControlWidget->getCalibCoord3DTransY();
            double tZ3D = mControlWidget->getCalibCoord3DTransZ();
            double axeLen = mControlWidget->getCalibCoord3DAxeLen();

//            int swapX = mControlWidget->getCalibCoord3DSwapX() ? -1 : 1;
//            int swapY = mControlWidget->getCalibCoord3DSwapY() ? -1 : 1;
//            int swapZ = mControlWidget->getCalibCoord3DSwapZ() ? -1 : 1;

            qreal coordLineWidth = 2.0;


            if( extCalib->isSetExtrCalib() ){

            QPointF points[4];
            Point2f p[4];
            char coordinaten[50];

            /////////////////////////////////////////////////////
            // Draw a cube (Quader) near coord system          //
            /////////////////////////////////////////////////////

            bool drawCube = false;

            if( drawCube ){
                painter->setPen(Qt::green);
                // Boden
                p[0] = extCalib->getImagePoint(Point3f(100,100,0));
                p[1] = extCalib->getImagePoint(Point3f(200,100,0));
                painter->drawLine(QPointF(p[0].x,p[0].y), QPointF(p[1].x, p[1].y));
                p[0] = extCalib->getImagePoint(Point3f(200,100,0));
                p[1] = extCalib->getImagePoint(Point3f(200,200,0));
                painter->drawLine(QPointF(p[0].x,p[0].y), QPointF(p[1].x, p[1].y));
                p[0] = extCalib->getImagePoint(Point3f(200,200,0));
                p[1] = extCalib->getImagePoint(Point3f(100,200,0));
                painter->drawLine(QPointF(p[0].x,p[0].y), QPointF(p[1].x, p[1].y));
                p[0] = extCalib->getImagePoint(Point3f(100,200,0));
                p[1] = extCalib->getImagePoint(Point3f(100,100,0));
                painter->drawLine(QPointF(p[0].x,p[0].y), QPointF(p[1].x, p[1].y));
                // Seiten
                p[0] = extCalib->getImagePoint(Point3f(100,100,0));
                p[1] = extCalib->getImagePoint(Point3f(100,100,100));
                painter->drawLine(QPointF(p[0].x,p[0].y), QPointF(p[1].x, p[1].y));
                p[0] = extCalib->getImagePoint(Point3f(200,100,0));
                p[1] = extCalib->getImagePoint(Point3f(200,100,100));
                painter->drawLine(QPointF(p[0].x,p[0].y), QPointF(p[1].x, p[1].y));
                p[0] = extCalib->getImagePoint(Point3f(100,200,0));
                p[1] = extCalib->getImagePoint(Point3f(100,200,100));
                painter->drawLine(QPointF(p[0].x,p[0].y), QPointF(p[1].x, p[1].y));
                p[0] = extCalib->getImagePoint(Point3f(200,200,0));
                p[1] = extCalib->getImagePoint(Point3f(200,200,100));
                painter->drawLine(QPointF(p[0].x,p[0].y), QPointF(p[1].x, p[1].y));
                // Oben
                p[0] = extCalib->getImagePoint(Point3f(100,100,100));
                p[1] = extCalib->getImagePoint(Point3f(200,100,100));
                painter->drawLine(QPointF(p[0].x,p[0].y), QPointF(p[1].x, p[1].y));
                p[0] = extCalib->getImagePoint(Point3f(200,100,100));
                p[1] = extCalib->getImagePoint(Point3f(200,200,100));
                painter->drawLine(QPointF(p[0].x,p[0].y), QPointF(p[1].x, p[1].y));
                p[0] = extCalib->getImagePoint(Point3f(200,200,100));
                p[1] = extCalib->getImagePoint(Point3f(100,200,100));
                painter->drawLine(QPointF(p[0].x,p[0].y), QPointF(p[1].x, p[1].y));
                p[0] = extCalib->getImagePoint(Point3f(100,200,100));
                p[1] = extCalib->getImagePoint(Point3f(100,100,100));
                painter->drawLine(QPointF(p[0].x,p[0].y), QPointF(p[1].x, p[1].y));


            }

            /////////////////////////////////////////////////////
            // Draw the bounding rect                          //
            /////////////////////////////////////////////////////

            bool drawBoundingRect = false;

            if( drawBoundingRect )
            {
                QRectF boundingBox = boundingRect();
                double min_x = boundingBox.x(), max_x = min_x+boundingBox.width();
                double min_y = boundingBox.y(), max_y = min_y+boundingBox.height();

                painter->setPen(Qt::cyan);
                painter->drawLine(QPointF(min_x, min_y), QPointF(min_x, max_y));
                painter->drawLine(QPointF(min_x, max_y), QPointF(max_x, max_y));
                painter->drawLine(QPointF(max_x, max_y), QPointF(max_x, min_y));
                painter->drawLine(QPointF(max_x, min_y), QPointF(min_x, min_y));
            }

            /////////////////////////////////////////////////////
            // Draw a rect around the whole coordinate system //
            /////////////////////////////////////////////////////

            bool paintRectCoordArea = false;

            if( paintRectCoordArea )
            {
                int min_x = min(min(x.x,y.x),min(z.x,ursprung.x));
                int max_x = max(max(x.x,y.x),max(z.x,ursprung.x));

                int min_y = min(min(x.y,y.y),min(z.y,ursprung.y));
                int max_y = max(max(x.y,y.y),max(z.y,ursprung.y));

                painter->setFont(QFont("Arial", 20));

                painter->setPen(Qt::green);

                painter->drawLine(QPointF(min_x, min_y), QPointF(min_x, max_y));
                painter->drawLine(QPointF(min_x, max_y), QPointF(max_x, max_y));
                painter->drawLine(QPointF(max_x, max_y), QPointF(max_x, min_y));
                painter->drawLine(QPointF(max_x, min_y), QPointF(min_x, min_y));

                sprintf (coordinaten, "(%d, %d)", min_x, min_y);
                painter->drawText(QPoint(min_x-45, min_y-10), QString(coordinaten));

                sprintf (coordinaten, "%d", max_y-min_y);
                painter->drawText(QPoint(min_x+10,min_y+(max_y-min_y)/2.0), QString(coordinaten));

                sprintf (coordinaten, "%d", max_x-min_x);
                painter->drawText(QPoint(min_x+(max_x-min_x)/2.0,min_y+30), QString(coordinaten));
            }


            //////////////////////////////
            // Drawing the X,Y,Z - Axis //
            //////////////////////////////

            painter->setPen(QPen(QBrush(Qt::blue),coordLineWidth));
            p[0] = extCalib->getImagePoint(Point3f(0,0,0));
            p[1] = extCalib->getImagePoint(Point3f(x3D.x,x3D.y,x3D.z));
            painter->drawLine(QPointF(p[0].x,p[0].y),QPointF(p[1].x,p[1].y));
            p[1] = extCalib->getImagePoint(Point3f(y3D.x,y3D.y,y3D.z));
            painter->drawLine(QPointF(p[0].x,p[0].y),QPointF(p[1].x,p[1].y));
            p[1] = extCalib->getImagePoint(Point3f(z3D.x,z3D.y,z3D.z));
            painter->drawLine(QPointF(p[0].x,p[0].y),QPointF(p[1].x,p[1].y));
//            painter->drawLine(QPointF(ursprung.x,ursprung.y),QPointF(x.x,x.y));
//            painter->drawLine(QPointF(ursprung.x,ursprung.y),QPointF(y.x,y.y));
//            painter->drawLine(QPointF(ursprung.x,ursprung.y),QPointF(z.x,z.y));

            /////////////////////////////////
            // Drawing the X,Y,Z - Symbols //
            /////////////////////////////////

            painter->setPen(QPen(QBrush(Qt::black),coordLineWidth));
            painter->setFont(QFont("Arial", 15 ));

            p[0] = extCalib->getImagePoint(Point3f(x3D.x+10, x3D.y, x3D.z));
            painter->drawText(QPointF(p[0].x-5,p[0].y+5), QString("X"));
            p[0] = extCalib->getImagePoint(Point3f(y3D.x, y3D.y+10, y3D.z));
            painter->drawText(QPointF(p[0].x-5,p[0].y+5), QString("Y"));
            p[0] = extCalib->getImagePoint(Point3f(z3D.x, z3D.y, z3D.z+10));
            painter->drawText(QPointF(p[0].x-5,p[0].y+5), QString("Z"));


            if( debug ) debout << "Ursprungskoordinaten: " << ursprung.x << ", " << ursprung.y << endl;
            if( debug ) debout << "Bildsize: " << mMainWindow->getImage()->width() << "x" << mMainWindow->getImage()->height() << endl;
            if( debug ) sprintf (coordinaten, "(%d  %d %d)", tX3D, tY3D, tZ3D);
            //painter->drawText(QPoint(ursprung.x+25,ursprung.y),QString(coordinaten));

            //////////////////////////////
            // Drawing the tick-markers //
            //////////////////////////////

            int tickLength = AXIS_MARKERS_LENGTH;
            painter->setPen(QPen(QBrush(Qt::blue),coordLineWidth));

            // Start bei 100cm bis Achsen-Laenge-Pfeilspitzenlaenge alle 100cm
            for(int i=100;i<axeLen-tickLength;i+=100)
            {
                // Solange Achsen-Ende noch nicht erreicht: Markierung zeichnen
                if( i+tickLength < x3D.x )
                {
                    p[0] = extCalib->getImagePoint(Point3f(i,-tickLength,0));
                    p[1] = extCalib->getImagePoint(Point3f(i, tickLength,0));
                    painter->drawLine(QPointF(p[0].x,p[0].y),QPointF(p[1].x,p[1].y));
                    p[0] = extCalib->getImagePoint(Point3f(i,0,-tickLength));
                    p[1] = extCalib->getImagePoint(Point3f(i,0, tickLength));
                    painter->drawLine(QPointF(p[0].x,p[0].y),QPointF(p[1].x,p[1].y));
                    p[0] = extCalib->getImagePoint(Point3f(i,0,0));
                }
                if( i+tickLength < y3D.y )
                {
                    p[0] = extCalib->getImagePoint(Point3f(-tickLength,i,0));
                    p[1] = extCalib->getImagePoint(Point3f( tickLength,i,0));
                    painter->drawLine(QPointF(p[0].x,p[0].y),QPointF(p[1].x,p[1].y));
                    p[0] = extCalib->getImagePoint(Point3f(0,i,-tickLength));
                    p[1] = extCalib->getImagePoint(Point3f(0,i, tickLength));
                    painter->drawLine(QPointF(p[0].x,p[0].y),QPointF(p[1].x,p[1].y));
                }
                if( i+tickLength < z3D.z )
                {
                    p[0] = extCalib->getImagePoint(Point3f(-tickLength,0,i));
                    p[1] = extCalib->getImagePoint(Point3f( tickLength,0,i));
                    painter->drawLine(QPointF(p[0].x,p[0].y),QPointF(p[1].x,p[1].y));
                    p[0] = extCalib->getImagePoint(Point3f(0,-tickLength,i));
                    p[1] = extCalib->getImagePoint(Point3f(0, tickLength,i));
                    painter->drawLine(QPointF(p[0].x,p[0].y),QPointF(p[1].x,p[1].y));
                }
            }

            QFont font("Arial", tickLength*0.5 );
            painter->setFont(font);

            painter->setPen(Qt::green);
            painter->setBrush(Qt::green);
            painter->setFont(QFont("Arial", 20));

            if( debug ) sprintf (coordinaten, "(%.2f, %.2f)", ursprung.x,ursprung.y);
            if( debug ) painter->drawText(QPoint(ursprung.x-100,ursprung.y),QString(coordinaten));

            //////////////////////////////////////////////
            // Drawing the peaks at the end of the axis //
            //////////////////////////////////////////////            

            int peakSize = AXIS_MARKERS_LENGTH;

            ///////
            // X //
            ///////

            p[0] = extCalib->getImagePoint(Point3f(x3D.x,0,0));
            p[1] = extCalib->getImagePoint(Point3f(x3D.x-peakSize,-peakSize,0));
            p[2] = extCalib->getImagePoint(Point3f(x3D.x-peakSize, peakSize,0));
            points[0] = QPointF(p[0].x,p[0].y);
            points[1] = QPointF(p[1].x,p[1].y);
            points[2] = QPointF(p[2].x,p[2].y);
            painter->setPen(Qt::green);
//            painter->drawPolygon(points,3,Qt::WindingFill);
            painter->setPen(QPen(QBrush(Qt::blue),coordLineWidth));
            painter->drawLine(points[0],points[1]);
            painter->drawLine(points[0],points[2]);
            painter->drawLine(points[2],points[1]);
            p[0] = extCalib->getImagePoint(Point3f(x3D.x,0,0));
            p[1] = extCalib->getImagePoint(Point3f(x3D.x-peakSize,0,-peakSize));
            p[2] = extCalib->getImagePoint(Point3f(x3D.x-peakSize,0, peakSize));
            p[3] = extCalib->getImagePoint(Point3f(x3D.x+peakSize,0,0));
            points[0] = QPointF(p[0].x,p[0].y);
            points[1] = QPointF(p[1].x,p[1].y);
            points[2] = QPointF(p[2].x,p[2].y);
            painter->setPen(Qt::green);
//            painter->drawPolygon(points,3,Qt::WindingFill);
            painter->setPen(QPen(QBrush(Qt::blue),coordLineWidth));
            painter->drawLine(points[0],points[1]);
            painter->drawLine(points[0],points[2]);
            painter->drawLine(points[2],points[1]);

            if( debug ) sprintf (coordinaten, "(%.2f, %.2f)", p[1].x,p[1].y);
            if( debug ) painter->drawText(QPoint(p[3].x,p[3].y),QString(coordinaten));

            if( debug ) sprintf(coordinaten, "X");
            if( debug ) painter->drawText(QPoint(p[1].x,p[1].y),QString(coordinaten));

            ///////
            // Y //
            ///////

            p[0] = extCalib->getImagePoint(Point3f(0,y3D.y,0));
            p[1] = extCalib->getImagePoint(Point3f(-peakSize,y3D.y-peakSize,0));
            p[2] = extCalib->getImagePoint(Point3f( peakSize,y3D.y-peakSize,0));
            points[0] = QPointF(p[0].x,p[0].y);
            points[1] = QPointF(p[1].x,p[1].y);
            points[2] = QPointF(p[2].x,p[2].y);
            painter->setPen(Qt::green);
//            painter->drawPolygon(points,3,Qt::WindingFill);
            painter->setPen(QPen(QBrush(Qt::blue),coordLineWidth));
            painter->drawLine(points[0],points[1]);
            painter->drawLine(points[0],points[2]);
            painter->drawLine(points[2],points[1]);
            p[0] = extCalib->getImagePoint(Point3f(0,y3D.y,0));
            p[1] = extCalib->getImagePoint(Point3f(0,y3D.y-peakSize,-peakSize));
            p[2] = extCalib->getImagePoint(Point3f(0,y3D.y-peakSize, peakSize));
            p[3] = extCalib->getImagePoint(Point3f(0,y3D.y+peakSize,0));
            points[0] = QPointF(p[0].x,p[0].y);
            points[1] = QPointF(p[1].x,p[1].y);
            points[2] = QPointF(p[2].x,p[2].y);
            painter->setPen(Qt::green);
//            painter->drawPolygon(points,3,Qt::WindingFill);
            painter->setPen(QPen(QBrush(Qt::blue),coordLineWidth));
            painter->drawLine(points[0],points[1]);
            painter->drawLine(points[0],points[2]);
            painter->drawLine(points[2],points[1]);

            if( debug ) sprintf (coordinaten, "(%.2f, %.2f)", p[1].x,p[1].y);
            if( debug ) painter->drawText(QPoint(p[3].x,p[3].y),QString(coordinaten));

            if( debug ) sprintf(coordinaten, "Y");
            if( debug ) painter->drawText(QPoint(p[1].x,p[1].y),QString(coordinaten));

            ///////
            // Z //
            ///////

            p[0] = extCalib->getImagePoint(Point3f(0,0,z3D.z));
            p[1] = extCalib->getImagePoint(Point3f(0,-peakSize,z3D.z-peakSize));
            p[2] = extCalib->getImagePoint(Point3f(0, peakSize,z3D.z-peakSize));
            points[0] = QPointF(p[0].x,p[0].y);
            points[1] = QPointF(p[1].x,p[1].y);
            points[2] = QPointF(p[2].x,p[2].y);
            painter->setPen(Qt::green);
//            painter->drawPolygon(points,3,Qt::WindingFill);
            painter->setPen(QPen(QBrush(Qt::blue),coordLineWidth));
            painter->drawLine(points[0],points[1]);
            painter->drawLine(points[0],points[2]);
            painter->drawLine(points[2],points[1]);
            p[0] = extCalib->getImagePoint(Point3f(0,0,z3D.z));
            p[1] = extCalib->getImagePoint(Point3f(-peakSize,0,z3D.z-peakSize));
            p[2] = extCalib->getImagePoint(Point3f( peakSize,0,z3D.z-peakSize));
            p[3] = extCalib->getImagePoint(Point3f(0,0,z3D.z+peakSize));
            points[0] = QPointF(p[0].x,p[0].y);
            points[1] = QPointF(p[1].x,p[1].y);
            points[2] = QPointF(p[2].x,p[2].y);
            painter->setPen(Qt::green);
//            painter->drawPolygon(points,3,Qt::WindingFill);
            painter->setPen(QPen(QBrush(Qt::blue),coordLineWidth));
            painter->drawLine(points[0],points[1]);
            painter->drawLine(points[0],points[2]);
            painter->drawLine(points[2],points[1]);

            if( debug )  sprintf (coordinaten, "(%.2f, %.2f)", p[1].x,p[1].y);
            if( debug ) painter->drawText(QPoint(p[3].x,p[3].y),QString(coordinaten));

            if( debug ) sprintf(coordinaten, "Z");
            if( debug ) painter->drawText(QPoint(p[1].x,p[1].y),QString(coordinaten));

            }
        }

    }
}
