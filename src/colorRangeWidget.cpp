#include "colorRangeWidget.h"
#include "control.h"

ColorRangeWidget::ColorRangeWidget(QWidget *parent)
    : QWidget(parent)
{
    mMainWindow = (class Petrack*) parent;
    mColorPlot = mMainWindow->getControlWidget()->getColorPlot();

    fromHue = 0;
    fromSat = 0;
    fromVal = 128;
    toHue = 359;
    toSat = 255;
    toVal = 255;

    setupUi(this);

    QString styleString = QString("background-color: hsv(%1,%2,%3)").arg(fromHue).arg(fromSat).arg(fromVal);
    fromColor->setStyleSheet(styleString);
    styleString = QString("background-color: hsv(%1,%2,%3)").arg(toHue).arg(toSat).arg(toVal);
    toColor->setStyleSheet(styleString);

    QColor col;
    col.convertTo(QColor::Hsv);
    col.setHsv(fromHue, fromSat, fromVal);
    fromTriangle->setColor(col);
    col.setHsv(toHue, toSat, toVal);
    toTriangle->setColor(col);
}

// functions which force a new recognition
void ColorRangeWidget::on_inversHue_stateChanged(int i)
{
    mColorPlot->getMapItem()->changeActMapInvHue(i == Qt::Checked);
    mMainWindow->setRecognitionChanged(true);// flag indicates that changes of recognition parameters happens
    if( !mMainWindow->isLoading() )
        mMainWindow->updateImage();
    mColorPlot->replot();
}

void ColorRangeWidget::setControlWidget()
{

    int fH, tH, fS, tS;
    if (toHue < fromHue)
    {
        fH = toHue;
        tH = fromHue;
    }
    else
    {
        tH = toHue;
        fH = fromHue;
    }
    if (toSat < fromSat)
    {
        fS = toSat;
        tS = fromSat;
    }
    else
    {
        tS = toSat;
        fS = fromSat;
    }
    mMainWindow->getControlWidget()->mapX->setValue(fH*2);
    mMainWindow->getControlWidget()->mapW->setValue((tH-fH));
    mMainWindow->getControlWidget()->mapY->setValue(fS*2);
    mMainWindow->getControlWidget()->mapH->setValue((tS-fS));
}

void ColorRangeWidget::on_fromTriangle_colorChanged(const QColor &col)
{
    fromHue = col.hue(); fromSat = col.saturation(); fromVal = col.value();
    QString styleString = QString("background-color: hsv(%1,%2,%3)").arg(fromHue).arg(fromSat).arg(fromVal);
    //QString styleString = QString("background-color: rgb(%1,%2,%3)").arg(col.red()).arg(col.green()).arg(col.blue());
    fromColor->setStyleSheet(styleString);

    mColorPlot->getMapItem()->changeActMapFromColor(col);
    setControlWidget();

    mMainWindow->setRecognitionChanged(true);// flag indicates that changes of recognition parameters happens
    if( !mMainWindow->isLoading() )
        mMainWindow->updateImage();
}
void ColorRangeWidget::on_toTriangle_colorChanged(const QColor &col)
{
    toHue = col.hue(); toSat = col.saturation(); toVal = col.value();
    QString styleString = QString("background-color: hsv(%1,%2,%3)").arg(toHue).arg(toSat).arg(toVal);
    //QString styleString = QString("background-color: rgb(%1,%2,%3)").arg(col.red()).arg(col.green()).arg(col.blue());
    toColor->setStyleSheet(styleString);

    mColorPlot->getMapItem()->changeActMapToColor(col);
    setControlWidget();

    mMainWindow->setRecognitionChanged(true);// flag indicates that changes of recognition parameters happens
    if( !mMainWindow->isLoading() )
        mMainWindow->updateImage();
}

void ColorRangeWidget::on_fromColor_clicked()
{
    // QWindowsXpStyle uses native theming engine which causes some palette modifications not to have any effect.
    // ueber palette war der button ausser initial nicht zu aendern!!!

    QColor colBefore;//fromColor->palette().color(QPalette::Button);
    colBefore.convertTo(QColor::Hsv);
    colBefore.setHsv(fromHue, fromSat, fromVal);
    QColor col = (QColorDialog::getColor(colBefore, this, "Select color from which value a pixel belongs to marker")).convertTo(QColor::Hsv);
    if (col.isValid() && col != colBefore)
    {
        on_fromTriangle_colorChanged(col);
        fromTriangle->setColor(col);
    }
}

void ColorRangeWidget::on_toColor_clicked()
{
    // QWindowsXpStyle uses native theming engine which causes some palette modifications not to have any effect.
    // ueber palette war der button ausser initial nicht zu aendern!!!
    QColor colBefore;//toColor->palette().color(QPalette::Button);
    colBefore.convertTo(QColor::Hsv);
    colBefore.setHsv(toHue, toSat, toVal);
    QColor col = (QColorDialog::getColor(colBefore, this, "Select color to which value a pixel belongs to marker")).convertTo(QColor::Hsv);
    //QPalette palette = toColor->palette();
    //palette.setColor(QPalette::Button, col);
    //toColor->setPalette(palette);
    //toColor->setAutoFillBackground(true);
    if (col.isValid() && col != colBefore)
    {
        on_toTriangle_colorChanged(col);
        toTriangle->setColor(col);
    }
// Versuch mit hsv stabil werte einzugeben - wandern in hsv und rgb zur folge, wenn auch wenig
//        // QWindowsXpStyle uses native theming engine which causes some palette modifications not to have any effect.
//        // ueber palette war der button ausser initial nicht zu aendern!!!
//        QColor col = QColorDialog::getColor(toColor->palette().color(QPalette::Button).toHsv(), this, "").toHsv();
//        //debout << col.hue()<<endl;
//        //debout << col.saturation()<<endl;
//        //debout << col.value()<<endl;
//        //QPalette palette = toColor->palette();
//        //palette.setColor(QPalette::Button, col);
//        //toColor->setPalette(palette);
//        //toColor->setAutoFillBackground(true);
//        if (col.isValid())
//        {
//            QString styleString = QString("background-color: hsv(%1,%2,%3)").arg(col.hue()).arg(col.saturation()).arg(col.value());
//            toColor->setStyleSheet(styleString);
//        }
}

void ColorRangeWidget::setInvHue(bool b)
{
    inversHue->setChecked(b);//setCheckState();
}

void ColorRangeWidget::setFromColor(const QColor &col)
{
    if (col.isValid())
    {
        fromHue = col.hue(); fromSat = col.saturation(); fromVal = col.value();
        QString styleString = QString("background-color: hsv(%1,%2,%3)").arg(fromHue).arg(fromSat).arg(fromVal);
        fromColor->setStyleSheet(styleString);
        fromTriangle->setColor(col);
    }
}

void ColorRangeWidget::setToColor(const QColor &col)
{
    if (col.isValid())
    {
        toHue = col.hue(); toSat = col.saturation(); toVal = col.value();
        QString styleString = QString("background-color: hsv(%1,%2,%3)").arg(toHue).arg(toSat).arg(toVal);
        toColor->setStyleSheet(styleString);
        toTriangle->setColor(col);
    }
}
