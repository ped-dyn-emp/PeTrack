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

    void wheelEvent(QWheelEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    //void mouseReleaseEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);

signals:
    void mouseDoubleClick();
    void mouseShiftControlDoubleClick(QPointF pos);
    void mouseShiftDoubleClick(QPointF pos);
    void mouseControlDoubleClick(QPointF pos);
    void mouseRightDoubleClick(QPointF pos, int direction);
    void mouseMiddleDoubleClick(int direction);
    void mouseShiftWheel(int delta);
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
