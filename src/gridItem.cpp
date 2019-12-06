#include <QtWidgets>
#include <math.h>
#include <stdlib.h>

#include "petrack.h"
#include "control.h"
#include "view.h"
#include "gridItem.h"

GridItem::GridItem(QWidget *wParent, QGraphicsItem * parent)
    : QGraphicsItem(parent)
{
    mMainWindow = (class Petrack*) wParent;
    extCalib = mMainWindow->getExtrCalibration();
    mControlWidget = mMainWindow->getControlWidget();
    //setEnabled(false); // all mouse events connot access this item, but it will be seen
    // einzig move koennte interessant sein, um grid zu verschieben?!
}
QRectF GridItem::boundingRect() const
{
    // -mMainWindow->getImageBorderSize(), -mMainWindow->getImageBorderSize() TUTS NICHT !!!!!
    // aber setMatrix istr hier nicht so schoen
    if (mMainWindow->getImage())
        return QRectF(-mMainWindow->getImageBorderSize(), -mMainWindow->getImageBorderSize(), mMainWindow->getImage()->width(), mMainWindow->getImage()->height());
    else
        return QRectF(0, 0, 0, 0);
    // FOLGENDE ZEILE HAT FUER ENDLOSSCHLEIFE GESORGT; DA QT INTERN SICH 2 FKT GEGENSEITIG AUFGERUFEN HABEN!!!!!!!!!!
    //return QRectF(0, 0, mMainWindow->getScene()->width(), mMainWindow->getScene()->height());
}

// event, of moving mouse while pressing a mouse button
void GridItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{

    //debout << ">>>>>>>>MouseMoveEvent: " << mouse_x << ", " << mouse_y << endl;

    if (!mControlWidget->getCalibGridFix())
    {
        setCursor(Qt::SizeBDiagCursor);
        //setCursor(QPixmap("/home/zam/salden/Desktop/grid_small.png"));
        //         setCursor(Qt::ClosedHandCursor);
        QPointF diff = event->scenePos()-event->lastScenePos(); //screenPos()-buttonDownScreenPos(Qt::RightButton) also interesting
        if (event->buttons() == Qt::RightButton) // event->button() doesnt work
        {

            mControlWidget->setCalibGridRotate(mControlWidget->getCalibGridRotate()+(int)(3.*(diff.x()+diff.y())));
            //cout << diff.x()-diff.y() <<endl; //globalPos()
        }
        else if (event->buttons() == Qt::LeftButton)
        {
            //debout << "Grid Dimension: " << mControlWidget->getCalibGridDimension() << endl;
            if( mControlWidget->getCalibGridDimension() == 0 )
            {
                Point3f p3d = extCalib->get3DPoint(Point2f(event->scenePos().x(),
                                                           event->scenePos().y()),
                                                   mControlWidget->getCalibGrid3DTransZ());
                Point3f p3d_last = extCalib->get3DPoint(Point2f(mouse_x/*event->lastScenePos().x()*/,
                                                                mouse_y/*event->lastScenePos().y()*/),
                                                        mControlWidget->getCalibGrid3DTransZ());
                mControlWidget->setCalibGrid3DTransX(gridTrans_x+(mControlWidget->getCalibCoord3DSwapX() > 0 ? -1 : 1)*round(p3d.x-p3d_last.x));
                mControlWidget->setCalibGrid3DTransY(gridTrans_y+(mControlWidget->getCalibCoord3DSwapY() > 0 ? -1 : 1)*round(p3d.y-p3d_last.y));

            }else
            {
                cout << "Grid Move 2D: "<<diff.x() << ", " << diff.y() << endl;
                mControlWidget->setCalibGridTransX(mControlWidget->getCalibGridTransX()+(int)(10.*diff.x()));
                mControlWidget->setCalibGridTransY(mControlWidget->getCalibGridTransY()+(int)(10.*diff.y()));
            }
        }
        else if (event->buttons() == Qt::MidButton)
        {
            if( mControlWidget->getCalibGridDimension() == 0)
            {
                mControlWidget->setCalibGrid3DResolution(mControlWidget->getCalibGrid3DResolution()+(int)(10.*(diff.x()-diff.y())));
            }else
            {
                mControlWidget->setCalibGridScale(mControlWidget->getCalibGridScale()+(int)(10.*(diff.x()-diff.y())));
            }
        }
    }
    else
        QGraphicsItem::mouseMoveEvent(event); // drag mach ich selber
}

void GridItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!mControlWidget->getCalibGridFix())
    {
        if (event->button() == Qt::LeftButton)
        {
            mouse_x = event->scenePos().x();
            mouse_y = event->scenePos().y();

            gridTrans_x = mControlWidget->getCalibGrid3DTransX();
            gridTrans_y = mControlWidget->getCalibGrid3DTransY();

            //debout << ">>>>>>>>MousePressEvent: " << mouse_x << ", " << mouse_y << endl;
        }
    }else
    {
        QGraphicsItem::mousePressEvent(event);
    }
}

//// event, of moving mouse
//void GridItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
//{
//    QPointF pos = event->scenePos();
//    pos.setX3D(pos.x() + mMainWindow->getImageBorderSize());
//    pos.setY3D(pos.y() + mMainWindow->getImageBorderSize());
//    mMainWindow->setMousePosOnImage(pos);
//
//    // cursor doesnt change like coord and roi because complex handling if fixed!!!
////     if (!mControlWidget->getCalibGridFix())
////         setCursor(Qt::OpenHandCursor);
////     else
////         setCursor(Qt::CrossCursor);
//
//    QGraphicsItem::hoverMoveEvent(event);


int GridItem::drawLine(QPainter *painter, Point2f *p, int y_offset){

    int bS = mMainWindow->getImageBorderSize();
    QImage *img = mMainWindow->getImage();

    int iW, iH;

    if (img)
    {
        iW = img->width();
        iH = img->height();
    }
    else
    {
        iW = (int) mMainWindow->getScene()->width();
        iH = (int) mMainWindow->getScene()->height();
    }


    double x1_d, y1_d, x2_d, y2_d;

    // Steigung m und Achsenabschnitt b der Geraden berechnen: g(x) = m * x + b
    double m = (p[1].y-p[0].y)/(p[1].x-p[0].x), // Steigung
            b = p[1].y-m*p[1].x;                // Achsenabschnitt

    //debout << "P1(" << p[0].x << "," << p[0].y << "), P2(" << p[1].x << "," << p[1].y << ")" << endl;
    //debout << "Steigung m=" << m << " Achsenabschnitt b=" << b << endl;

    // boolean Werte fuer beide Endpunkte der Geraden, am Ende schauen, ob beide gesetzt wurden
    bool p1IsSet = false, p2IsSet = false;

    // Sonderfall falls Steigung unendlich ist (horizontale Linie)
    if( isinf(m) )
    {
        if( p[0].x >= 0-bS && p[0].x <= iW-bS )
        {
            painter->drawLine(QPointF(p[0].x,-bS),QPointF(p[1].x,iH-bS));
            return 1;
        }
    }

    // Schnittpunkt mit oberer Bildkante? ( y = y_offset )
    if( (y_offset-b)/m >= 0-bS && (y_offset-b)/m <= iW-bS ){
        x1_d = (y_offset-b)/m;
        y1_d = y_offset;
        p1IsSet = true;
    }

    // Schnittpunkt mit rechter Bildkante? ( x = iW-bS )
    if( m*(iW-bS)+b >= y_offset && m*(iW-bS)+b <= iH-bS ){
        if( p1IsSet ){
            x2_d = iW-bS;
            y2_d = m*(iW-bS)+b;
            p2IsSet = true;
        }else{
            x1_d = iW-bS;
            y1_d = m*(iW-bS)+b;
            p1IsSet = true;
        }
    }

    // Schnittpunkt mit unterer Bildkante? ( y = iH-bS )
    if( (iH-bS-b)/m >= 0-bS && (iH-bS-b)/m <= iW-bS ){
        if( p1IsSet ){
            x2_d = (iH-bS-b)/m;
            y2_d = iH-bS;
            p2IsSet = true;
        }else{
            x1_d = (iH-bS-b)/m;
            y1_d = iH-bS;
            p1IsSet = true;
        }
    }

    // Schnittpunkt mit linker Bildkante? ( x = 0-bS )
    if( m*(-bS)+b >= y_offset && m*(-bS)+b <= iH-bS ){
        if( p1IsSet ){
            x2_d = -bS;
            y2_d = m*(-bS)+b;
            p2IsSet = true;
        }else{
            x1_d = -bS;
            y1_d = m*(-bS)+b;
            p1IsSet = true;
        }
    }
    //debout << "p1: " << p1IsSet << " p2: " << p2IsSet << endl;
    // Schauen, ob beide Endpunkte der Geraden gesetzt wurden
    if( p1IsSet && p2IsSet )
    {
        painter->drawLine(QPointF(x1_d,y1_d),QPointF(x2_d,y2_d));
        return 1;
    }
    return 0;
}

void GridItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{

    bool debug = false;
//    bool debug = true;

    //     QMatrix matrix;
    //     matrix.translate(-mMainWindow->getImageBorderSize(), -mMainWindow->getImageBorderSize());
    //     setMatrix(matrix);

    // folgende zeilen waren mal noetig und haben 50% CPU load verursacht,
    // weil wahrscheinlich mit setmatrix ein neues paint angestossen wurde
    // cpu load 0 war alleine mit grid.setvisible(false) zu erzeugen
    //     //FAKE!!!!!!!!!!!!!!!!!!!!!!!!
    //     QMatrix matrix;
    //     matrix.translate(1, 1); // FEHLER IN QT ????? noetig, damit trotz recognitionroiitem auch image auch ohne border komplett neu gezeichnet wird // wird 2 zeilen weiter zurueckgesetzt, aber mit 0, 0 geht es nicht
    //     setMatrix(matrix);
    //     matrix.translate(-1, -1);
    //     setMatrix(matrix);

    if (!mControlWidget->getCalibGridFix() && mControlWidget->getCalibGridShow())
    {
        //setFlags(ItemIsMovable); // noetig, damit mouseEvent leftmousebutton weitergegeben wird, aber drag mach ich selber
        setFlag(ItemIsMovable); // noetig, damit mouseEvent leftmousebutton weitergegeben wird, aber drag mach ich selber
    }
    else
    {
        //setFlags(!ItemIsMovable);
        setFlag(ItemIsMovable, false);
    }

    int tX3D = mControlWidget->getCalibGrid3DTransX();
    int tY3D = mControlWidget->getCalibGrid3DTransY();
    int tZ3D = mControlWidget->getCalibGrid3DTransZ();
    int resolution = mControlWidget->getCalibGrid3DResolution();

    // confirmation prompt if the vanish points are inside the image
    bool vanishPointIsInsideImage = false;
    bool vanishPointYIsInsideImage = false;
    bool vanishPointXIsInsideImage = false;
    Point2f vanishPointY, vanishPointX;
    double x,y;
    if( mMainWindow->getImage() && extCalib->isSetExtrCalib() )
    {

        // create 2 parallel lines in x-direction
        Point3f a3d = Point3f(-500,-500,0),
                b3d = Point3f(500,-500,0),
                c3d = Point3f(-500,500,0),
                d3d = Point3f(500,500,0);
        Point2f a2d = extCalib->getImagePoint(a3d),
                b2d = extCalib->getImagePoint(b3d),
                c2d = extCalib->getImagePoint(c3d),
                d2d = extCalib->getImagePoint(d3d);

        if( debug )
        {
            debout << "A3d x=" << a3d.x << ", y=" << a3d.y << ", z=" << a3d.z << endl;
            debout << "B3d x=" << b3d.x << ", y=" << b3d.y << ", z=" << b3d.z << endl;
            debout << "C3d x=" << c3d.x << ", y=" << c3d.y << ", z=" << c3d.z << endl;
            debout << "D3d x=" << d3d.x << ", y=" << d3d.y << ", z=" << d3d.z << endl;

            debout << "A2d x=" << a2d.x << ", y=" << a2d.y << endl;
            debout << "B2d x=" << b2d.x << ", y=" << b2d.y << endl;
            debout << "C2d x=" << c2d.x << ", y=" << c2d.y << endl;
            //            debout << "D2d x=" << d2d.x << ", y=" << d2d.y << endl;
        }
        // y = m*x+n
        float m1 = (b2d.y-a2d.y)/(b2d.x-a2d.x),
                m2 = (d2d.y-c2d.y)/(d2d.x-c2d.x),
                n1 = a2d.y-m1*a2d.x,
                n2 = c2d.y-m2*c2d.x;
        if( debug ) debout << "m1=" << m1 << ", m2=" << m2 << ", n1=" << n1 << ", n2=" << n2 << endl;

        x = (n2-n1)/(m1-m2);
        y = ((m1*x+n1)+(m2*x+n2))/2.0;

        vanishPointY = Point2f(x,y);


        if( debug ) debout << "Vanish Point (x): (x=" << x << ", y=" << y << ")" << endl;

        // create 2 parallel lines in y-direction
        a3d = Point3f(-500,-500,0),
                b3d = Point3f(-500,500,0),
                c3d = Point3f(500,-500,0),
                d3d = Point3f(500,500,0);
        a2d = extCalib->getImagePoint(a3d),
                b2d = extCalib->getImagePoint(b3d),
                c2d = extCalib->getImagePoint(c3d),
                d2d = extCalib->getImagePoint(d3d);

        if( debug )
        {
            debout << "A3d x=" << a3d.x << ", y=" << a3d.y << ", z=" << a3d.z << endl;
            debout << "B3d x=" << b3d.x << ", y=" << b3d.y << ", z=" << b3d.z << endl;
            debout << "C3d x=" << c3d.x << ", y=" << c3d.y << ", z=" << c3d.z << endl;
            debout << "D3d x=" << d3d.x << ", y=" << d3d.y << ", z=" << d3d.z << endl;

            debout << "A2d x=" << a2d.x << ", y=" << a2d.y << endl;
            debout << "B2d x=" << b2d.x << ", y=" << b2d.y << endl;
            debout << "C2d x=" << c2d.x << ", y=" << c2d.y << endl;
            debout << "D2d x=" << d2d.x << ", y=" << d2d.y << endl;
        }
        // y = m*x+n
        m1 = (b2d.y-a2d.y)/(b2d.x-a2d.x),
                m2 = (d2d.y-c2d.y)/(d2d.x-c2d.x),
                n1 = a2d.y-m1*a2d.x,
                n2 = c2d.y-m2*c2d.x;

        if( debug ) debout << "m1=" << m1 << ", m2=" << m2 << ", n1=" << n1 << ", n2=" << n2 << endl;

        x = (n2-n1)/(m1-m2);
        y = ((m1*x+n1)+(m2*x+n2))/2.0;
        vanishPointX = Point2f(x,y);


        if( debug ) debout << "Vanish Point: (x=" << x << ", y=" << y << ")" << endl;

        vanishPointYIsInsideImage = !extCalib->isOutsideImage(vanishPointY);
        vanishPointXIsInsideImage = !extCalib->isOutsideImage(vanishPointX);

        if( vanishPointYIsInsideImage )
        {
            if( debug ) debout << "Vanish Point 1 is inside the image!" << endl;
        }
        if( vanishPointXIsInsideImage )
        {
            if( debug ) debout << "Vanish Point 2 is inside the image!" << endl;
        }


        ////////////////////////////////
        // Drawing Vanish Points      //
        ////////////////////////////////
        if( mControlWidget->getCalibExtrVanishPointsShow() )
        {
            painter->setPen(Qt::yellow);
            painter->drawLine(QPointF(vanishPointY.x-10,vanishPointY.y-10),QPointF(vanishPointY.x+10,vanishPointY.y+10));
            painter->drawLine(QPointF(vanishPointY.x-10,vanishPointY.y+10),QPointF(vanishPointY.x+10,vanishPointY.y-10));
            painter->setPen(Qt::red);
            painter->drawPoint(vanishPointY.x,vanishPointY.y);
            painter->drawText(vanishPointY.x-20,vanishPointY.y-10,"Vanishing Point (y)");

            painter->setPen(Qt::yellow);
            painter->drawLine(QPointF(vanishPointX.x-10,vanishPointX.y-10),QPointF(vanishPointX.x+10,vanishPointX.y+10));
            painter->drawLine(QPointF(vanishPointX.x-10,vanishPointX.y+10),QPointF(vanishPointX.x+10,vanishPointX.y-10));

            painter->setPen(Qt::red);
            painter->drawPoint(vanishPointX.x,vanishPointX.y);
            painter->drawText(vanishPointX.x-20,vanishPointX.y-10,"Vanishing Point (x)");

        }
    }

    if (mControlWidget->getCalibGridShow())
    {
        QImage *img = mMainWindow->getImage();
        QMatrix matrixPaint;
        QPointF pnt1, pnt2; // floating point
        int bS = mMainWindow->getImageBorderSize();
        if( debug ) debout << "Border-Size: " << bS << endl;
        int iW, iH;
        int maxExp;

        if (img)
        {
            iW = img->width();
            iH = img->height();
        }
        else
        {
            iW = (int) mMainWindow->getScene()->width();
            iH = (int) mMainWindow->getScene()->height();
        }
        maxExp = iH>iW?iH:iW;

        if( debug ) debout << "Image-Size: " << iW << "x" << iH << endl;


        if( mControlWidget->getCalibGridDimension() == 1 )
        {
            double sc = mControlWidget->getCalibGridScale()/10.;
            double tX = mControlWidget->getCalibGridTransX()/10.;
            double tY = mControlWidget->getCalibGridTransY()/10.;
            double ro = mControlWidget->getCalibGridRotate()/10.;

            // transformation nicht des koordinatensystems wie bei coorditem sondern bei grid der painter

            //         // aktualisierung der transformationsmatrix
            //         QMatrix matrix;
            //         // matrix wird nur bei aenderungen neu bestimmt
            //         matrix.translate(mControlWidget->getCalibGridTransX()/10., mControlWidget->getCalibGridTransY()/10.);
            //         matrix.rotate(mControlWidget->getCalibGridRotate()/10.);
            //         //matrix.scale(sc/100., sc/100.);
            //         setMatrix(matrix);

            // grid
            painter->save(); // wie opengl matrix aber auch pen etc auf stack
            matrixPaint.translate(tX, tY); // hier ist translate zusaetzlich auf matrix, bei translate-fkt von item ist absolut
            //matrixPaint.translate(iW/2.-bS, iH/2.-bS);
            matrixPaint.rotate(ro);
            //matrixPaint.translate(-iW/2.+bS, -iH/2.+bS);
            painter->setWorldMatrix(matrixPaint, true); // true sagt, dass relativ und nicht absolut (also zusaetzlich zur uebergeordneten matrizen)
            painter->setPen(Qt::red);

            for (int i=(int)-((maxExp+100)/sc); i<2*(maxExp/sc); i++)
            {
                pnt1.setX(i*sc);pnt1.setY(-maxExp-100);
                pnt2.setX(i*sc);pnt2.setY(2*maxExp);
                painter->drawLine(pnt1, pnt2);
                pnt1.setX(-maxExp-100);pnt1.setY(i*sc);
                pnt2.setX(2*maxExp);pnt2.setY(i*sc);
                painter->drawLine(pnt1, pnt2);
            }
            painter->restore(); // matr etc von stack

            // weisses polygon, was ueberragende bereiche von grid ueberdeckt
            QPointF points[] = {
                QPointF(-bS, -bS),
                QPointF(iW-bS,-bS),
                QPointF(iW-bS,iH-bS),
                QPointF(iW+3*(maxExp+200)-bS,iH-bS),
                QPointF(iW+3*(maxExp+200)-bS,-2*maxExp-100-bS),
                QPointF(-2*maxExp-400-bS,-2*maxExp-100-bS),
                QPointF(-2*maxExp-400-bS,iH+3*(maxExp+200)-bS),
                QPointF(iW+3*(maxExp+200)-bS,iH+3*(maxExp+200)-bS),
                QPointF(iW+3*(maxExp+200)-bS,iH-bS),
                QPointF(-bS,iH-bS)
            };
            painter->setPen(Qt::NoPen);
            painter->setBrush(Qt::white);
            //painter->setBrush(Qt::green);

            bool drawPolygon = true;

            if( drawPolygon )
            {
                painter->drawPolygon(points, 10);
            }

        }else
        {
            if( extCalib->isSetExtrCalib() ){

                /*char text[100];

            painter->setPen(Qt::blue);
            Point3f mousePoint = extCalib->get3DPoint(Point2f(mouse_x,mouse_y),tZ3D);

            debout << ">>>>>>>>Mouse Point: " << mousePoint.x << ", " << mousePoint.y << ", " << mousePoint.z << endl;

            sprintf (text, "3D: (%.2f, %.2f, %.2f)", mousePoint.x, mousePoint.y, mousePoint.z);
            painter->drawText(mouse_x, mouse_y, QString(text));

            painter->setPen(Qt::red);
            */
                double min_x = INT_MAX, min_y = INT_MAX, max_x = INT_MIN, max_y = INT_MIN;



                /*
            Point2f p2f[4];
            p2f[0] = Point2f(0-bS,0-bS);
            p2f[0] = Point2f(0-bS,0-bS);
            p2f[0] = Point2f(0-bS,0-bS);
            p2f[0] = Point2f(0-bS,0-bS);
            */
                int y_offset = -bS;

                if( vanishPointXIsInsideImage ){
                    y_offset = vanishPointX.y+100;
                }
                if( vanishPointYIsInsideImage ){
                    y_offset = vanishPointY.y+100;
                }

                if( debug ) cout << "y_offset: " << y_offset << endl;
                Point3f points[4];
                // top left corner
                points[0] = extCalib->get3DPoint(Point2f(0-bS,y_offset),tZ3D);
                // top right corner
                points[1] = extCalib->get3DPoint(Point2f(iW,y_offset),tZ3D);
                // bottom left corner
                points[2] = extCalib->get3DPoint(Point2f(0-bS,iH),tZ3D);
                // bottom right corner
                points[3] = extCalib->get3DPoint(Point2f(iW,iH),tZ3D);

                if( debug ) debout << "iW: " << iW << " iH: " << iH << " bS: " << bS << " y_offset: " << y_offset << " tZ3D: " << tZ3D << endl;


                painter->setPen(Qt::green);

                //            painter->drawLine(QPointF(0-bS+2,y_offset+2),QPointF(iW-bS-2,y_offset+2));
                //            painter->drawLine(QPointF(iW-bS-2,y_offset+2),QPointF(iW-bS-2,iH-bS-2));
                //            painter->drawLine(QPointF(iW-bS-2,iH-bS-2),QPointF(0-bS+2,iH-bS-2));
                //            painter->drawLine(QPointF(0-bS+2,iH-bS-2),QPointF(0-bS+2,y_offset+2));

                //debout << "Width: " << iW << ", Height: " << iH << endl;

                //            painter->setPen(Qt::green);
                //            painter->drawLine(QPointF(points[0].x,points[0].y),QPointF(points[1].x,points[1].y));
                //            painter->drawLine(QPointF(points[3].x,points[3].y),QPointF(points[1].x,points[1].y));
                //            painter->drawLine(QPointF(points[2].x,points[2].y),QPointF(points[3].x,points[3].y));
                //            painter->drawLine(QPointF(points[0].x,points[0].y),QPointF(points[2].x,points[2].y));
                //            painter->setPen(Qt::red);

                //debout << ">>>>>>>>>Eckpunkte: " << endl;

                for(int i=0;i<4;i++)
                {
                    x = points[i].x;
                    y = points[i].y;

                    if (debug) debout << "x: " << x << " y: " << y << endl;

                    max_x = x > max_x ? x : max_x;
                    min_x = x < min_x ? x : min_x;
                    max_y = y > max_y ? y : max_y;
                    min_y = y < min_y ? y : min_y;
                    if( debug ) cout << ">>>" << points[i].x << ", " << points[i].y << ", " << points[i].z << endl;
                }


                if( debug ) cout << "X: min=" << min_x << " min:" << max_x << endl;
                if( debug ) cout << "Y: min=" << min_y << " max:" << max_y << endl;

                //mControlWidget->setGridMinMaxTranslation(min_x, max_x, min_y, max_y);

                Point3f ursprung(0,0,0);// = extCalib->getImagePoint(Point3f(tX3D,tY3D,tZ3D));
                double abs = 0;
                int res = -1;

                Point3f xl, xr, yo, yu;
                Point2f p[4];
                QPointF point[4];

                painter->setPen(Qt::red);

                if( false && vanishPointIsInsideImage )
                {
                    bool simpleMethod = true;
                    if( simpleMethod ){
                        // Draw simple Grid around origin point
                        for(int i=0; i<=2500; i+=resolution)
                        {
                            p[0] = extCalib->getImagePoint(Point3f(tX3D+2500,tY3D+i,tZ3D));
                            p[1] = extCalib->getImagePoint(Point3f(tX3D,tY3D+i,tZ3D));
                            painter->drawLine(QPointF(p[0].x,p[0].y),QPointF(p[1].x,p[1].y));
                            p[0] = extCalib->getImagePoint(Point3f(tX3D+i,tY3D+2500,tZ3D));
                            p[1] = extCalib->getImagePoint(Point3f(tX3D+i,tY3D,tZ3D));
                            painter->drawLine(QPointF(p[0].x,p[0].y),QPointF(p[1].x,p[1].y));
                        }
                    }else
                    {
                        // Draw line by line
                        for(int i=min_y*5; i<max_y*5; i+=resolution)
                        {
                            for(int j=min_x*5; j<max_x*5; j+=resolution)
                            {
                                p[0] = extCalib->getImagePoint(Point3f(tX3D+i,tY3D+j,tZ3D));
                                p[1] = extCalib->getImagePoint(Point3f(tX3D+i+resolution,tY3D+j,tZ3D));

                                point[0] = QPointF(p[0].x,p[0].y);
                                point[1] = QPointF(p[1].x,p[1].y);

                                p[0] = extCalib->getImagePoint(Point3f(tX3D+i,tY3D+j,tZ3D));
                                p[1] = extCalib->getImagePoint(Point3f(tX3D+i,tY3D+j+resolution,tZ3D));

                                point[2] = QPointF(p[0].x,p[0].y);
                                point[3] = QPointF(p[1].x,p[1].y);

                                if( ( point[0].x() > -bS && point[0].x() < (iW+bS) && point[0].y() > -bS && point[0].y() < (iH+bS) ) ||
                                        ( point[1].x() > -bS && point[1].x() < (iW+bS) && point[1].y() > -bS && point[1].y() < (iH+bS) ) ||
                                        ( point[3].x() > -bS && point[3].x() < (iW+bS) && point[2].y() > -bS && point[2].y() < (iH+bS) ) ||
                                        ( point[2].x() > -bS && point[2].x() < (iW+bS) && point[3].y() > -bS && point[3].y() < (iH+bS) ) )

                                {
                                    float a = sqrt( pow(point[1].x()-point[0].x(), 2) + pow(point[1].y()-point[0].y(), 2) ),
                                            b = sqrt( pow(point[3].x()-point[2].x(), 2) + pow(point[3].y()-point[2].y(), 2) ),
                                            c = sqrt( pow(point[1].x()-point[3].x(), 2) + pow(point[1].y()-point[3].y(), 2) ),
                                            s = (a+b+c) / 2.0;
                                    if( sqrt(s*(s-a)*(s-b)*(s-c)) > 15 )
                                    {
                                        painter->drawLine(point[0],point[1]);
                                        painter->drawLine(point[2],point[3]);
                                    }
                                }
                            }
                        }
                    }

                }else
                {


                    int swapX = mControlWidget->getCalibCoord3DSwapX() ? -1 : 1;
                    int swapY = mControlWidget->getCalibCoord3DSwapY() ? -1 : 1;
                    int grid_height = tZ3D-mControlWidget->getCalibCoord3DTransZ(); // Da extCalibration immer vom Koordinatensystemursprung ausgeht (Das Grid soll aber unabhngig davon gezeichnet werden)

                    //int vorzeichenX = 1, vorzeichenY = 1;

                    if( debug ) debout << "Grid x: " << tX3D << " y: " << tY3D << " z: " << tZ3D <<  " y_offset: "
                           << y_offset << " bS: " << bS << " iW: " << iW << " iH: " << iH
                           << " min_x: " << min_x << " max_x: " << max_x << " min_y: " << min_y << " max_y: " << max_y << endl;

                    // horizontal lines from the left to the right on height tZ3D the lines start from origin point (tY3D)
                    // until max_y or if tY3D < min_y it starts with min_y because otherwise it is outside the image

                    //Point3f p3d_center = extCalib->get3DPoint(Point2f(iW/2,iH/2),tZ3D);

                    //if( debug ) debout << "Centerpoint 2D: (" << iW/2 << "," << iH/2 << "), 3D: (" << p3d_center.x << "," << p3d_center.y << "," << p3d_center.z << ")" << endl;
                    // y+
//                    for(int i = p3d_center.y+tY3D; i<max_y; i+=resolution)
                    for(int i = -mControlWidget->getCalibCoord3DTransY()+tY3D; i < (swapY > 0 ? max_y : -min_y); i+=resolution)
                    {

                        //debout << "i=" << -mControlWidget->getCalibCoord3DTransY()+tY3D << "; i<" << (swapY > 0 ? max_y : min_y) << "; i+=" << resolution << endl;
                        // Bildpunkte zu den Endpunkten der Linie holen
                        p[0] = extCalib->getImagePoint(Point3f(min_x,swapY*i,grid_height));
                        //if( debug ) debout << "p[0] x: " << p[0].x << " y: " << p[0].y << endl;
                        p[1] = extCalib->getImagePoint(Point3f(max_x,swapY*i,grid_height));
                        //if( debug ) debout << "p[1] x: " << p[1].x << " y: " << p[1].y << endl;
                        res = drawLine(painter,p,y_offset);
                        if (debug) debout << "Zeichnung? " << res << endl;
                        //if( debug ) debout << "i: " << i << endl;

                    }
                    // see above but now the lines start from origin point (tY3D) until min_y
                    // y-
//                    for(int i = p3d_center.y+tY3D-resolution; i>min_y; i-=resolution)
                    for(int i = -mControlWidget->getCalibCoord3DTransY()+tY3D-resolution; i > (swapY > 0 ? min_y : -max_y); i-=resolution)
                    {
                        //debout << "i=" << -mControlWidget->getCalibCoord3DTransY()+tY3D-resolution << "; i<" << (swapY > 0 ? min_y : -max_y) << "; i-=" << resolution << endl;
                        // Bildpunkte zu den Endpunkten der Linie holen
                        p[0] = extCalib->getImagePoint(Point3f(min_x,swapY*i,grid_height));
                        p[1] = extCalib->getImagePoint(Point3f(max_x,swapY*i,grid_height));
                        res = drawLine(painter,p,y_offset);
                        if (debug) debout << "Zeichnung? " << res << endl;


                    }
                    // vertical lines from the top to the bottom on height tZ3D the lines start from origin point(tX3D)
                    // until max_x of if tX3D < minx it starts with min_x because otherwise the lines are outside the image
                    // x+
//                    for(int i = p3d_center.x+tX3D; i<max_x; i+=resolution)
                    for(int i = -mControlWidget->getCalibCoord3DTransX()+tX3D; i < (swapX > 0 ? max_x : -min_x); i+=resolution)
                    {
                        //debout << "i=" << -mControlWidget->getCalibCoord3DTransX()+tX3D << "; i<" << (swapX > 0 ? max_x : -min_x) << "; i+=" << resolution << endl;
                        //debout << "x+ " << i << endl;
                        // Bildpunkte zu den Endpunkten der Linie holen
                        p[0] = extCalib->getImagePoint(Point3f(swapX*i,min_y,grid_height));
                        p[1] = extCalib->getImagePoint(Point3f(swapX*i,max_y,grid_height));
                        res = drawLine(painter,p,y_offset);
                        if (debug) debout << "Zeichnung? " << res << endl;

                    }
                    // see above but now the lines start from origin point until min_x
                    // x-
//                    for(int i = p3d_center.x+tX3D-resolution; i>min_x; i-=resolution)
                    for(int i = -mControlWidget->getCalibCoord3DTransX()+tX3D-resolution; i > (swapX > 0 ? min_x : -max_x); i-=resolution)
                    {
                        //debout << "i=" << -mControlWidget->getCalibCoord3DTransX()+tX3D-resolution << "; i<" << (swapX > 0 ? min_x : -max_x) << "; i-=" << resolution << endl;
                        //debout << "x- " << i << endl;
                        // Bildpunkte zu den Endpunkten der Linie holen
                        p[0] = extCalib->getImagePoint(Point3f(swapX*i,min_y,grid_height));
                        p[1] = extCalib->getImagePoint(Point3f(swapX*i,max_y,grid_height));
                        res = drawLine(painter,p,y_offset);
                        if (debug) debout << "Zeichnung? " << res << endl;

                    }


                }
            }

        }

    }
    //setVisible(false);
}
