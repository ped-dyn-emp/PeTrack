/********************************************************************************
** Form generated from reading UI file 'stereo.ui'
**
** Created by: Qt User Interface Compiler version 5.5.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_STEREO_H
#define UI_STEREO_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Stereo
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QCheckBox *stereoShowDisparity;
    QComboBox *stereoColor;
    QHBoxLayout *horizontalLayout_4;
    QCheckBox *stereoUseForReco;
    QComboBox *stereoDispAlgo;
    QHBoxLayout *horizontalLayout_2;
    QCheckBox *stereoUseForHeight;
    QCheckBox *stereoUseForHeightEver;
    QHBoxLayout *horizontalLayout_3;
    QCheckBox *stereoUseForExport;
    QCheckBox *stereoUseCalibrationCenter;
    QGridLayout *gridLayout;
    QLabel *label;
    QLabel *label_2;
    QLabel *label_4;
    QSpinBox *stereoMaskSize;
    QSpinBox *edgeMaskSize;
    QSpinBox *maxDisparity;
    QSpinBox *minDisparity;
    QLabel *label_3;
    QCheckBox *useEdge;
    QLabel *label_5;
    QSpinBox *opacity;
    QCheckBox *hideWrong;
    QPushButton *stereoExport;

    void setupUi(QWidget *Stereo)
    {
        if (Stereo->objectName().isEmpty())
            Stereo->setObjectName(QStringLiteral("Stereo"));
        Stereo->resize(267, 274);
        verticalLayout = new QVBoxLayout(Stereo);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(2);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        stereoShowDisparity = new QCheckBox(Stereo);
        stereoShowDisparity->setObjectName(QStringLiteral("stereoShowDisparity"));

        horizontalLayout->addWidget(stereoShowDisparity);

        stereoColor = new QComboBox(Stereo);
        stereoColor->setObjectName(QStringLiteral("stereoColor"));

        horizontalLayout->addWidget(stereoColor);


        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(2);
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        stereoUseForReco = new QCheckBox(Stereo);
        stereoUseForReco->setObjectName(QStringLiteral("stereoUseForReco"));

        horizontalLayout_4->addWidget(stereoUseForReco);

        stereoDispAlgo = new QComboBox(Stereo);
        stereoDispAlgo->setObjectName(QStringLiteral("stereoDispAlgo"));

        horizontalLayout_4->addWidget(stereoDispAlgo);


        verticalLayout->addLayout(horizontalLayout_4);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        stereoUseForHeight = new QCheckBox(Stereo);
        stereoUseForHeight->setObjectName(QStringLiteral("stereoUseForHeight"));

        horizontalLayout_2->addWidget(stereoUseForHeight);

        stereoUseForHeightEver = new QCheckBox(Stereo);
        stereoUseForHeightEver->setObjectName(QStringLiteral("stereoUseForHeightEver"));
        stereoUseForHeightEver->setEnabled(true);
        stereoUseForHeightEver->setChecked(true);

        horizontalLayout_2->addWidget(stereoUseForHeightEver);


        verticalLayout->addLayout(horizontalLayout_2);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        stereoUseForExport = new QCheckBox(Stereo);
        stereoUseForExport->setObjectName(QStringLiteral("stereoUseForExport"));

        horizontalLayout_3->addWidget(stereoUseForExport);

        stereoUseCalibrationCenter = new QCheckBox(Stereo);
        stereoUseCalibrationCenter->setObjectName(QStringLiteral("stereoUseCalibrationCenter"));
        stereoUseCalibrationCenter->setChecked(true);

        horizontalLayout_3->addWidget(stereoUseCalibrationCenter);


        verticalLayout->addLayout(horizontalLayout_3);

        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        label = new QLabel(Stereo);
        label->setObjectName(QStringLiteral("label"));

        gridLayout->addWidget(label, 1, 0, 1, 1);

        label_2 = new QLabel(Stereo);
        label_2->setObjectName(QStringLiteral("label_2"));

        gridLayout->addWidget(label_2, 2, 0, 1, 1);

        label_4 = new QLabel(Stereo);
        label_4->setObjectName(QStringLiteral("label_4"));

        gridLayout->addWidget(label_4, 4, 0, 1, 1);

        stereoMaskSize = new QSpinBox(Stereo);
        stereoMaskSize->setObjectName(QStringLiteral("stereoMaskSize"));
        stereoMaskSize->setKeyboardTracking(false);
        stereoMaskSize->setMinimum(1);
        stereoMaskSize->setMaximum(23);
        stereoMaskSize->setSingleStep(2);
        stereoMaskSize->setValue(7);

        gridLayout->addWidget(stereoMaskSize, 1, 1, 1, 1);

        edgeMaskSize = new QSpinBox(Stereo);
        edgeMaskSize->setObjectName(QStringLiteral("edgeMaskSize"));
        edgeMaskSize->setKeyboardTracking(false);
        edgeMaskSize->setMinimum(3);
        edgeMaskSize->setMaximum(11);
        edgeMaskSize->setSingleStep(2);
        edgeMaskSize->setValue(5);

        gridLayout->addWidget(edgeMaskSize, 2, 1, 1, 1);

        maxDisparity = new QSpinBox(Stereo);
        maxDisparity->setObjectName(QStringLiteral("maxDisparity"));
        maxDisparity->setKeyboardTracking(false);
        maxDisparity->setMaximum(999);
        maxDisparity->setValue(100);

        gridLayout->addWidget(maxDisparity, 3, 1, 1, 1);

        minDisparity = new QSpinBox(Stereo);
        minDisparity->setObjectName(QStringLiteral("minDisparity"));
        minDisparity->setKeyboardTracking(false);
        minDisparity->setMaximum(999);

        gridLayout->addWidget(minDisparity, 4, 1, 1, 1);

        label_3 = new QLabel(Stereo);
        label_3->setObjectName(QStringLiteral("label_3"));

        gridLayout->addWidget(label_3, 3, 0, 1, 1);

        useEdge = new QCheckBox(Stereo);
        useEdge->setObjectName(QStringLiteral("useEdge"));

        gridLayout->addWidget(useEdge, 2, 2, 1, 1);

        label_5 = new QLabel(Stereo);
        label_5->setObjectName(QStringLiteral("label_5"));

        gridLayout->addWidget(label_5, 5, 0, 1, 1);

        opacity = new QSpinBox(Stereo);
        opacity->setObjectName(QStringLiteral("opacity"));
        opacity->setMaximum(100);
        opacity->setValue(100);

        gridLayout->addWidget(opacity, 5, 1, 1, 1);

        hideWrong = new QCheckBox(Stereo);
        hideWrong->setObjectName(QStringLiteral("hideWrong"));
        hideWrong->setChecked(true);

        gridLayout->addWidget(hideWrong, 5, 2, 1, 1);


        verticalLayout->addLayout(gridLayout);

        stereoExport = new QPushButton(Stereo);
        stereoExport->setObjectName(QStringLiteral("stereoExport"));

        verticalLayout->addWidget(stereoExport);


        retranslateUi(Stereo);

        QMetaObject::connectSlotsByName(Stereo);
    } // setupUi

    void retranslateUi(QWidget *Stereo)
    {
        stereoShowDisparity->setText(QApplication::translate("Stereo", "show disparity", 0));
#ifndef QT_NO_TOOLTIP
        stereoUseForReco->setToolTip(QApplication::translate("Stereo", "the recognition is not only done by markers; the height profile of a person is searched inside the disparity map", 0));
#endif // QT_NO_TOOLTIP
        stereoUseForReco->setText(QApplication::translate("Stereo", "use for recognition", 0));
#ifndef QT_NO_TOOLTIP
        stereoUseForHeight->setToolTip(QApplication::translate("Stereo", "enable, if the disparity should be used to measure the position in 3D space and use it for height measurement", 0));
#endif // QT_NO_TOOLTIP
        stereoUseForHeight->setText(QApplication::translate("Stereo", "use for height/pos measurement", 0));
        stereoUseForHeightEver->setText(QApplication::translate("Stereo", "ever", 0));
#ifndef QT_NO_TOOLTIP
        stereoUseForExport->setToolTip(QApplication::translate("Stereo", "the calculated 3D data is directly used for trajectory export; values in the calibration tab are ignored (for uneven videos)", 0));
#endif // QT_NO_TOOLTIP
        stereoUseForExport->setText(QApplication::translate("Stereo", "use for trajectory export", 0));
#ifndef QT_NO_TOOLTIP
        stereoUseCalibrationCenter->setToolTip(QApplication::translate("Stereo", "while exporting pointGrey data directly, the coordinate center of calibration tab is used", 0));
#endif // QT_NO_TOOLTIP
        stereoUseCalibrationCenter->setText(QApplication::translate("Stereo", "use calib center", 0));
        label->setText(QApplication::translate("Stereo", "stereo mask size: ", 0));
        label_2->setText(QApplication::translate("Stereo", "edge mask size: ", 0));
        label_4->setText(QApplication::translate("Stereo", "min disparity", 0));
        label_3->setText(QApplication::translate("Stereo", "max disparity", 0));
        useEdge->setText(QApplication::translate("Stereo", "use", 0));
        label_5->setText(QApplication::translate("Stereo", "opacity", 0));
        hideWrong->setText(QApplication::translate("Stereo", "hide", 0));
        stereoExport->setText(QApplication::translate("Stereo", "export point cloud", 0));
        Q_UNUSED(Stereo);
    } // retranslateUi

};

namespace Ui {
    class Stereo: public Ui_Stereo {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_STEREO_H
