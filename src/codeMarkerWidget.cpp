#include "codeMarkerWidget.h"

CodeMarkerWidget::CodeMarkerWidget(QWidget *parent)
    : QWidget(parent)
{
    mMainWindow = (class Petrack*) parent;

    setupUi(this);

    dictList->addItem("DICT_4X4_50");  // 0
    dictList->addItem("DICT_4X4_100"); // 1
    dictList->addItem("DICT_4X4_250"); // 2
    dictList->addItem("DICT_4X4_1000");// 3

    dictList->addItem("DICT_5X5_50");  // 4
    dictList->addItem("DICT_5X5_100"); // 5
    dictList->addItem("DICT_5X5_250"); // 6
    dictList->addItem("DICT_5X5_1000");// 7

    dictList->addItem("DICT_6X6_50");  // 8
    dictList->addItem("DICT_6X6_100"); // 9
    dictList->addItem("DICT_6X6_250"); // 10
    dictList->addItem("DICT_6X6_1000");// 11

    dictList->addItem("DICT_7X7_50");  // 12
    dictList->addItem("DICT_7X7_100"); // 13
    dictList->addItem("DICT_7X7_250"); // 14
    dictList->addItem("DICT_7X7_1000");// 15

    dictList->addItem("DICT_ARUCO_ORGINAL"); // 16
    dictList->addItem("DICT_mip_36h12");//17

    // Add some other marker dictionaries
    //dictList->addItem("ARTOOLKIT");

    dictList->setCurrentIndex(16);

}
//<COLOR_MARKER>
//    <MASK SHOW="0" OPACITY="100" MASK="1"/>
//    <FROM_COLOR HUE="0" SATURATION="0" VALUE="0"/>
//    <TO_COLOR HUE="0" SATURATION="0" VALUE="0"/>
//    <PARAM CLOSE_RADIUS="0" CLOSE_USED="0" OPEN_RADIUS="0" OPEN_USED="0" MIN_AREA="0" MAX_AREA="0" MAX_RATIO="0.0"/>
//</COLOR_MARKER>

// store data in xml node
void CodeMarkerWidget::setXml(QDomElement &elem)
{
    QDomElement subElem;

    subElem = (elem.ownerDocument()).createElement("DICTIONARY");
    subElem.setAttribute("ID", dictList->currentIndex());
    elem.appendChild(subElem);

    subElem = (elem.ownerDocument()).createElement("PARAM");
    subElem.setAttribute("ADAPTIVE_THRESH_WIN_SIZE_MIN",adaptiveThreshWinSizeMin->value());
    subElem.setAttribute("ADAPTIVE_THRESH_WIN_SIZE_MAX",adaptiveThreshWinSizeMax->value());
    subElem.setAttribute("ADAPTIVE_THRESH_WIN_SIZE_STEP",adaptiveThreshWinSizeStep->value());
    subElem.setAttribute("ADAPTIVE_THRESH_CONSTANT",adaptiveThreshConstant->value());
    subElem.setAttribute("MIN_MARKER_PERIMETER",minMarkerPerimeter->value());
    subElem.setAttribute("MAX_MARKER_PERIMETER",maxMarkerPerimeter->value());
    subElem.setAttribute("MAX_RATIO_ERROR",polygonalApproxAccuracyRate->value());
    subElem.setAttribute("MIN_CORNER_DISTANCE",minCornerDistance->value());
    subElem.setAttribute("MIN_DISTANCE_TO_BORDER",minDistanceToBorder->value());
    subElem.setAttribute("MIN_MARKER_DISTANCE",minMarkerDistance->value());
    subElem.setAttribute("CORNER_REFINEMENT", doCornerRefinement->isChecked());
    subElem.setAttribute("CORNER_REFINEMENT_WIN_SIZE",cornerRefinementWinSize->value());
    subElem.setAttribute("CORNER_REFINEMENT_MAX_ITERATIONS",cornerRefinementMaxIterations->value());
    subElem.setAttribute("CORNER_REFINEMENT_MIN_ACCURACY",cornerRefinementMinAccuracy->value());
    subElem.setAttribute("MARKER_BORDER_BITS",markerBorderBits->value());
    subElem.setAttribute("PERSPECTIVE_REMOVE_PIXEL_PER_CELL",perspectiveRemovePixelPerCell->value());
    subElem.setAttribute("PERSPECTIVE_REMOVE_IGNORED_MARGIN_PER_CELL",perspectiveRemoveIgnoredMarginPerCell->value());
    subElem.setAttribute("MAX_ERRONEOUS_BITS_IN_BORDER_RATE",maxErroneousBitsInBorderRate->value());
    subElem.setAttribute("MIN_OTSU_STD_DEV",minOtsuStdDev->value());
    subElem.setAttribute("ERROR_CORRECTION_RATE",errorCorrectionRate->value());
    subElem.setAttribute("SHOW_DETECTED_CANDIDATES", showDetectedCandidates->isChecked());
    elem.appendChild(subElem);

}

// read data from xml node
void CodeMarkerWidget::getXml(QDomElement &elem)
{
    QDomElement subElem;
    QString styleString;

    for(subElem = elem.firstChildElement(); !subElem.isNull(); subElem = subElem.nextSiblingElement())
    {

        if (subElem.tagName() == "DICTIONARY")
        {
            if (subElem.hasAttribute("ID"))
                dictList->setCurrentIndex(subElem.attribute("ID").toInt());
        }

        if (subElem.tagName() == "PARAM")
        {
            if (subElem.hasAttribute("ADAPTIVE_TRHESH_WIN_SIZE_MIN"))
                adaptiveThreshWinSizeMin->setValue(subElem.attribute("ADAPTIVE_THRESH_WIN_SIZE_MIN").toInt());
            if (subElem.hasAttribute("ADAPTIVE_TRHESH_WIN_SIZE_MAX"))
                adaptiveThreshWinSizeMax->setValue(subElem.attribute("ADAPTIVE_THRESH_WIN_SIZE_MAX").toInt());
            if (subElem.hasAttribute("ADAPTIVE_TRHESH_WIN_SIZE_STEP"))
                adaptiveThreshWinSizeStep->setValue(subElem.attribute("ADAPTIVE_THRESH_WIN_SIZE_STEP").toInt());
            if (subElem.hasAttribute("ADAPTIVE_TRHESH_CONSTANT"))
                adaptiveThreshConstant->setValue(subElem.attribute("ADAPTIVE_THRESH_CONSTANT").toInt());
            if (subElem.hasAttribute("MIN_MARKER_PERIMETER"))
                minMarkerPerimeter->setValue(subElem.attribute("MIN_MARKER_PERIMETER").toDouble());
            if (subElem.hasAttribute("MAX_MARKER_PERIMETER"))
                maxMarkerPerimeter->setValue(subElem.attribute("MAX_MARKER_PERIMETER").toDouble());
            if (subElem.hasAttribute("MAX_RATIO_ERROR"))
                polygonalApproxAccuracyRate->setValue(subElem.attribute("MAX_RATIO_ERROR").toDouble());
            if (subElem.hasAttribute("MIN_CORNER_DISTANCE"))
                minCornerDistance->setValue(subElem.attribute("MIN_CORNER_DISTANCE").toDouble());
            if (subElem.hasAttribute("MIN_DISTANCE_TO_BORDER"))
                minDistanceToBorder->setValue(subElem.attribute("MIN_DISTANCE_TO_BORDER").toInt());
            if (subElem.hasAttribute("MIN_MARKER_DISTANCE"))
                minMarkerDistance->setValue(subElem.attribute("MIN_MARKER_DISTANCE").toDouble());
            if (subElem.hasAttribute("CORNER_REFINEMENT"))
                doCornerRefinement->setChecked(subElem.attribute("CORNER_REFINEMENT").toInt());
          if (subElem.hasAttribute("CORNER_REFINEMENT_WIN_SIZE"))
                cornerRefinementWinSize->setValue(subElem.attribute("CORNER_REFINEMENT_WIN_SIZE").toInt());
            if (subElem.hasAttribute("CORNER_REFINEMENT_MAX_ITERATIONS"))
                cornerRefinementMaxIterations->setValue(subElem.attribute("CORNER_REFINEMENT_MAX_ITERATIONS").toInt());
            if (subElem.hasAttribute("CORNER_REFINEMENT_MIN_ACCURACY"))
                cornerRefinementMinAccuracy->setValue(subElem.attribute("CORNER_REFINEMENT_MIN_ACCURACY").toDouble());
            if (subElem.hasAttribute("MARKER_BORDER_BITS"))
                markerBorderBits->setValue(subElem.attribute("MARKER_BORDER_BITS").toInt());
            if (subElem.hasAttribute("PERSPECTIVE_REMOVE_PIXEL_PER_CELL"))
                perspectiveRemovePixelPerCell->setValue(subElem.attribute("PERSPECTIVE_REMOVE_PIXEL_PER_CELL").toInt());
            if (subElem.hasAttribute("PERSPECTIVE_REMOVE_IGNORED_MARGIN_PER_CELL"))
                perspectiveRemoveIgnoredMarginPerCell->setValue(subElem.attribute("PERSPECTIVE_REMOVE_IGNORED_MARGIN_PER_CELL").toDouble());
            if (subElem.hasAttribute("MAX_ERRONEOUS_BITS_IN_BORDER_RATE"))
                maxErroneousBitsInBorderRate->setValue(subElem.attribute("MAX_ERRONEOUS_BITS_IN_BORDER_RATE").toDouble());
            if (subElem.hasAttribute("MIN_OTSU_STD_DEV"))
                minOtsuStdDev->setValue(subElem.attribute("MIN_OTSU_STD_DEV").toDouble());
            if (subElem.hasAttribute("ERROR_CORRECTION_RATE"))
                errorCorrectionRate->setValue(subElem.attribute("ERROR_CORRECTION_RATE").toDouble());
            if (subElem.hasAttribute("SHOW_DETECTED_CANDIDATES"))
                showDetectedCandidates->setCheckState(subElem.attribute("SHOW_DETECTED_CANDIDATES").toInt() ? Qt::Checked : Qt::Unchecked);


        }
    }
}

#include "moc_codeMarkerWidget.cpp"
