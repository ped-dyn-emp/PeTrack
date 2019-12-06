#ifndef LOGOITEM_H
#define LOGOITEM_H

#include <QGraphicsItem>
//#include <QtWidgets>
//#include <QDragEnterEvent>
//#include <QDropEvent>

class Petrack;
class LogoItem;

class Fader: public QObject
{
    Q_OBJECT
public:
    Fader();
    void fadeOut(LogoItem* gi, int frames);
private slots:
    void fadeOutStep();
public:
    LogoItem* mLogoItem;
    int mFrames;
    double mStep;
    QTimer *mTimer;
};

class LogoItem: public QGraphicsItem
{
public:
    LogoItem(QWidget *wParent, QGraphicsItem * parent = 0);

    QRectF boundingRect() const;

//    void dragEnterEvent(QDragEnterEvent *event);
//    void dropEvent(QDropEvent *event);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    inline float getOpacity()
    {
        return mOpacity;
    }
    inline void setOpacity(float o)
    {
        mOpacity = o;
    }
    void fadeOut(int frames=100);

private:
    Petrack *mMainWindow;
    //QGraphicsScene *mScene; erhaelt man mit scene()
    QImage *mImage;
    float mOpacity;
    Fader fader;
};

#endif
