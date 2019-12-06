#include <QtWidgets>

#include <cmath>

#include "helper.h"
#include "view.h"
#include "vector.h"
#include "petrack.h"
#include "control.h"

GraphicsView::GraphicsView(ViewWidget *viewWidget)
    : QGraphicsView()
{
    mViewWidget = viewWidget;

    setTransformationAnchor(AnchorUnderMouse); // for wheel
    setResizeAnchor(AnchorUnderMouse); // for scroll?

    // alignment kann man nicht komplett ausstellen!
    //transformationAnchor
// QGraphicsView::NoAnchor
// QGraphicsView::AnchorViewCenter
// QGraphicsView::AnchorUnderMouse
}

//     void mousePressEvent ( QMouseEvent * event )
//     {
//         if (event->button() == Qt::RightButton) {
//             setDragMode(ScrollHandDrag);
//         }
//     }
//     void mouseReleaseEvent ( QMouseEvent * event )
//     {
//         if (event->button() == Qt::RightButton) {
//             setDragMode(RubberBandDrag);
//         }
//     }
//     void mouseMoveEvent(QMouseEvent *event)
//     {
//         if (event->buttons() & Qt::RightButton)
//             setDragMode(QGraphicsView::ScrollHandDrag);
//     }

void GraphicsView::wheelEvent(QWheelEvent * event)
{
    int numDegrees = event->delta() / 8;

    // hier koennte man einstellen, dass das pixel unter cursor stehen bleibt das dort bereits steht
    // (dorhin gezoomt wird, wo mouse steht!!!)
    // nicht noetig, denn es gibt: setTransformationAnchor(AnchorUnderMouse); (siehe constructor)
    //fitInView(rect)
    //ensureVisible(rect)


//     QMatrix mat = matrix();
//     debout << "----------------------" <<endl;
//     debout << mat.m11() << " " << mat.m22() <<endl;
//     QPointF sp = mapToScene(event->pos());
//     debout << sp.x() << " " << sp.y() <<endl;
//     debout << event->pos().x() << " " << event->pos().y() <<endl;
// //     debout << event->globalPos().x() << " " << event->globalPos().x() <<endl;
// //     QRectF sr = sceneRect();
// //     debout << sr.x() << " " << sr.width() <<endl;
//     debout << numDegrees/2 <<endl;
// //     centerOn(mapToScene(event->pos()));

    if (event->modifiers() == Qt::ShiftModifier) // nur shift zugelassen ...
    {
        if (event->orientation() == Qt::Horizontal) // warum orienttion?
            emit mouseShiftWheel(numDegrees/15);
        else
            emit mouseShiftWheel(-numDegrees/15);
    }
    else
    {
        if (event->orientation() == Qt::Horizontal) // warum orienttion?
            mViewWidget->zoomIn(numDegrees/2);
        else
            mViewWidget->zoomOut(numDegrees/2);
    }

//     mat = matrix();
//     debout << mat.m11() << " " << mat.m22() <<endl;

//     qreal factor = matrix().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
//      if (factor < 0.07 || factor > 100)
//          return;

//      scale(scaleFactor, scaleFactor);

//     QGraphicsView::wheelEvent(event); LANGE NACH GESUCHT: UNBEDINGT RAUS, DA NACH ZOOM NOCH EIN SCROLLEN GEMACHT WIRD
}

//void GraphicsView::mouseReleaseEvent(QMouseEvent *event)
//{
//    if (event->button() == Qt::LeftButton)
//    {

//    }
//    else if (event->button() == Qt::RightButton)
//    {
//        emit mouseRightClick(mapToScene(event->pos()));
//    }
//    else if (event->button() == Qt::MiddleButton)
//    {

//    }
//    scene()->update();

//    //QGraphicsScene::mouseReleaseEvent(event);
//}

void GraphicsView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (event->modifiers() & Qt::ShiftModifier) // mit & genuegt, dass unter anderem shift gedrueckt wird (zb mit conrol)
        {
            if (event->modifiers() & Qt::ControlModifier) // mindestens shift und control gedrueckt
                emit mouseShiftControlDoubleClick(mapToScene(event->pos()));
            else
                emit mouseShiftDoubleClick(mapToScene(event->pos())); //const QPoint & //const QPointF &pos
        }
        else if (event->modifiers() & Qt::ControlModifier) // mit & genuegt, dass unter anderem control gedrueckt wird (zb mit shift)
            emit mouseControlDoubleClick(mapToScene(event->pos())); //const QPoint & //const QPointF &pos
        else
            emit mouseDoubleClick();
    }
    else if (event->button() == Qt::RightButton)
    {
        if (event->modifiers() == Qt::ShiftModifier) // nur shift zugelassen ...
            emit mouseRightDoubleClick(mapToScene(event->pos()), -1);
        else if (event->modifiers() == Qt::ControlModifier)
            emit mouseRightDoubleClick(mapToScene(event->pos()), 0);
        else if (event->modifiers() == Qt::AltModifier)
            emit mouseRightDoubleClick(mapToScene(event->pos()), 1);
    }
    else if (event->button() == Qt::MiddleButton)
    {
        if (event->modifiers() == Qt::ShiftModifier) // nur shift zugelassen ...
            emit mouseMiddleDoubleClick(-1);
        else if (event->modifiers() == Qt::ControlModifier)
            emit mouseMiddleDoubleClick(0);
        else if (event->modifiers() == Qt::AltModifier)
            emit mouseMiddleDoubleClick(1);
    }

    scene()->update(); // damit neu hinzugekommende trackpoint oder das loeschen von trackpoints sichtbar wird

    QGraphicsView::mouseDoubleClickEvent(event);
}

void GraphicsView::keyPressEvent(QKeyEvent *event)
{
//    debout << "key: " << event->key() << endl;
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

//---------------------------------------------------------------------


ViewWidget::ViewWidget(QWidget *parent)
    : QFrame(parent)
{
    mMainWindow = (class Petrack*) parent;
    setContentsMargins(0, 0, 0, 0);
    //setFrameStyle(Sunken | StyledPanel);
    mGraphicsView = new GraphicsView(this);
    mGraphicsView->setRenderHint(QPainter::Antialiasing, false);
    mGraphicsView->setDragMode(QGraphicsView::ScrollHandDrag); //RubberBandDrag
//     mGraphicsView->setFrameStyle(QFrame::StyledPanel|QFrame::Plain);
//     mGraphicsView->setLineWidth(0);
//     mGraphicsView->setMidLineWidth(0);

    int size = style()->pixelMetric(QStyle::PM_ToolBarIconSize);
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
    //mRotateSlider->setOrientation(Qt::Horizontal);
    mRotateSlider->setMinimum(-180); //-360
    mRotateSlider->setMaximum(180); // 360
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
    mGraphicsView->fitInView(mGraphicsView->sceneRect(), Qt::KeepAspectRatio); // Qt::KeepAspectRatioByExpanding wuerde nur in eine dimension passend machen
    // doesnt work: mGraphicsView->fitInView(mGraphicsView->sceneRect(), Qt::KeepAspectRatio); // two times, while in the first run possibly the scrollbars have been there for calculation
    QMatrix matrix = mGraphicsView->matrix();
    // calculates the sclaing of matrix
    // see http://www.robertblum.com/articles/2005/02/14/decomposing-matrices for rotation out of matrix
    double scale = Vec2F(matrix.m11(), matrix.m21()).length();
    mZoomSlider->setValue((int) (250.+50.*log(scale)/log(2.)));
}

void ViewWidget::fitInROI(QRectF rect)
{

    //QRect rect(myRound(mRecognitionRoiItem->rect().x()+getImageBorderSize()), myRound(mRecognitionRoiItem->rect().y()+getImageBorderSize()), myRound(mRecognitionRoiItem->rect().width()), myRound(mRecognitionRoiItem->rect().height()));


    mGraphicsView->fitInView(rect, Qt::KeepAspectRatio); // Qt::KeepAspectRatioByExpanding wuerde nur in eine dimension passend machen
    // doesnt work: mGraphicsView->fitInView(mGraphicsView->sceneRect(), Qt::KeepAspectRatio); // two times, while in the first run possibly the scrollbars have been there for calculation
    QMatrix matrix = mGraphicsView->matrix();
    // calculates the sclaing of matrix
    // see http://www.robertblum.com/articles/2005/02/14/decomposing-matrices for rotation out of matrix
    double scale = Vec2F(matrix.m11(), matrix.m21()).length();
    mZoomSlider->setValue((int) (250.+50.*log(scale)/log(2.)));
}

void ViewWidget::setupMatrix()
{
    qreal scale = ::pow(2.0, (mZoomSlider->value() - 250) / 50.0);

    QMatrix matrix;
    matrix.scale(scale, scale);
    matrix.rotate(mRotateSlider->value());

//     debout << matrix.m11() << " " << matrix.m12() <<endl;
//     debout << matrix.m21() << " " << matrix.m22() <<endl;
//     debout << matrix.dx() << " " << matrix.dy() <<endl;
    mGraphicsView->setMatrix(matrix);
}

//  void View::print()
//  {
//      QPrinter printer;
//      QPrintDialog dialog(&printer, this);
//      if (dialog.exec() == QDialog::Accepted) {
//          QPainter painter(&printer);
//          mGraphicsView->render(&painter);
//      }
//  }

void ViewWidget::zoomIn(int i) //default i = 1
{
    mZoomSlider->setValue(mZoomSlider->value() + i);
}

void ViewWidget::zoomOut(int i) //default i = 1
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
    if ( hide )
    {
        mMainWindow->getControlWidget()->setVisible(false);
        hideShowControlsButton->setIcon(QPixmap(":/arrowLeft"));
    }else
    {
        mMainWindow->getControlWidget()->setVisible(true);
        hideShowControlsButton->setIcon(QPixmap(":/arrowRight"));
    }
}
void ViewWidget::hideShowControls()
{
    mMainWindow->getHideControlActor()->setChecked(mMainWindow->getControlWidget()->isVisible());
}
