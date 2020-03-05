/********************************************************************************
** Form generated from reading UI file 'stereo.ui'
**
** Created by: Qt User Interface Compiler version 5.14.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_STEREO_H
#define UI_STEREO_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
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
            Stereo->setObjectName(QString::fromUtf8("Stereo"));
        Stereo->resize(267, 274);
        verticalLayout = new QVBoxLayout(Stereo);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(2);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        stereoShowDisparity = new QCheckBox(Stereo);
        stereoShowDisparity->setObjectName(QString::fromUtf8("stereoShowDisparity"));

        horizontalLayout->addWidget(stereoShowDisparity);

        stereoColor = new QComboBox(Stereo);
        stereoColor->setObjectName(QString::fromUtf8("stereoColor"));

        horizontalLayout->addWidget(stereoColor);


        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(2);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        stereoUseForReco = new QCheckBox(Stereo);
        stereoUseForReco->setObjectName(QString::fromUtf8("stereoUseForReco"));

        horizontalLayout_4->addWidget(stereoUseForReco);

        stereoDispAlgo = new QComboBox(Stereo);
        stereoDispAlgo->setObjectName(QString::fromUtf8("stereoDispAlgo"));

        horizontalLayout_4->addWidget(stereoDispAlgo);


        verticalLayout->addLayout(horizontalLayout_4);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        stereoUseForHeight = new QCheckBox(Stereo);
        stereoUseForHeight->setObjectName(QString::fromUtf8("stereoUseForHeight"));

        horizontalLayout_2->addWidget(stereoUseForHeight);

        stereoUseForHeightEver = new QCheckBox(Stereo);
        stereoUseForHeightEver->setObjectName(QString::fromUtf8("stereoUseForHeightEver"));
        stereoUseForHeightEver->setEnabled(true);
        stereoUseForHeightEver->setChecked(true);

        horizontalLayout_2->addWidget(stereoUseForHeightEver);


        verticalLayout->addLayout(horizontalLayout_2);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        stereoUseForExport = new QCheckBox(Stereo);
        stereoUseForExport->setObjectName(QString::fromUtf8("stereoUseForExport"));

        horizontalLayout_3->addWidget(stereoUseForExport);

        stereoUseCalibrationCenter = new QCheckBox(Stereo);
        stereoUseCalibrationCenter->setObjectName(QString::fromUtf8("stereoUseCalibrationCenter"));
        stereoUseCalibrationCenter->setChecked(true);

        horizontalLayout_3->addWidget(stereoUseCalibrationCenter);


        verticalLayout->addLayout(horizontalLayout_3);

        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label = new QLabel(Stereo);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout->addWidget(label, 1, 0, 1, 1);

        label_2 = new QLabel(Stereo);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout->addWidget(label_2, 2, 0, 1, 1);

        label_4 = new QLabel(Stereo);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        gridLayout->addWidget(label_4, 4, 0, 1, 1);

        stereoMaskSize = new QSpinBox(Stereo);
        stereoMaskSize->setObjectName(QString::fromUtf8("stereoMaskSize"));
        stereoMaskSize->setKeyboardTracking(false);
        stereoMaskSize->setMinimum(1);
        stereoMaskSize->setMaximum(23);
        stereoMaskSize->setSingleStep(2);
        stereoMaskSize->setValue(7);

        gridLayout->addWidget(stereoMaskSize, 1, 1, 1, 1);

        edgeMaskSize = new QSpinBox(Stereo);
        edgeMaskSize->setObjectName(QString::fromUtf8("edgeMaskSize"));
        edgeMaskSize->setKeyboardTracking(false);
        edgeMaskSize->setMinimum(3);
        edgeMaskSize->setMaximum(11);
        edgeMaskSize->setSingleStep(2);
        edgeMaskSize->setValue(5);

        gridLayout->addWidget(edgeMaskSize, 2, 1, 1, 1);

        maxDisparity = new QSpinBox(Stereo);
        maxDisparity->setObjectName(QString::fromUtf8("maxDisparity"));
        maxDisparity->setKeyboardTracking(false);
        maxDisparity->setMaximum(999);
        maxDisparity->setValue(100);

        gridLayout->addWidget(maxDisparity, 3, 1, 1, 1);

        minDisparity = new QSpinBox(Stereo);
        minDisparity->setObjectName(QString::fromUtf8("minDisparity"));
        minDisparity->setKeyboardTracking(false);
        minDisparity->setMaximum(999);

        gridLayout->addWidget(minDisparity, 4, 1, 1, 1);

        label_3 = new QLabel(Stereo);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        gridLayout->addWidget(label_3, 3, 0, 1, 1);

        useEdge = new QCheckBox(Stereo);
        useEdge->setObjectName(QString::fromUtf8("useEdge"));

        gridLayout->addWidget(useEdge, 2, 2, 1, 1);

        label_5 = new QLabel(Stereo);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        gridLayout->addWidget(label_5, 5, 0, 1, 1);

        opacity = new QSpinBox(Stereo);
        opacity->setObjectName(QString::fromUtf8("opacity"));
        opacity->setMaximum(100);
        opacity->setValue(100);

        gridLayout->addWidget(opacity, 5, 1, 1, 1);

        hideWrong = new QCheckBox(Stereo);
        hideWrong->setObjectName(QString::fromUtf8("hideWrong"));
        hideWrong->setChecked(true);

        gridLayout->addWidget(hideWrong, 5, 2, 1, 1);


        verticalLayout->addLayout(gridLayout);

        stereoExport = new QPushButton(Stereo);
        stereoExport->setObjectName(QString::fromUtf8("stereoExport"));

        verticalLayout->addWidget(stereoExport);


        retranslateUi(Stereo);

        QMetaObject::connectSlotsByName(Stereo);
    } // setupUi

    void retranslateUi(QWidget *Stereo)
    {
        stereoShowDisparity->setText(QCoreApplication::translate("Stereo", "show disparity", nullptr));
#if QT_CONFIG(tooltip)
        stereoUseForReco->setToolTip(QCoreApplication::translate("Stereo", "the recognition is not only done by markers; the height profile of a person is searched inside the disparity map", nullptr));
#endif // QT_CONFIG(tooltip)
        stereoUseForReco->setText(QCoreApplication::translate("Stereo", "use for recognition", nullptr));
#if QT_CONFIG(tooltip)
        stereoUseForHeight->setToolTip(QCoreApplication::translate("Stereo", "enable, if the disparity should be used to measure the position in 3D space and use it for height measurement", nullptr));
#endif // QT_CONFIG(tooltip)
        stereoUseForHeight->setText(QCoreApplication::translate("Stereo", "use for height/pos measurement", nullptr));
        stereoUseForHeightEver->setText(QCoreApplication::translate("Stereo", "ever", nullptr));
#if QT_CONFIG(tooltip)
        stereoUseForExport->setToolTip(QCoreApplication::translate("Stereo", "the calculated 3D data is directly used for trajectory export; values in the calibration tab are ignored (for uneven videos)", nullptr));
#endif // QT_CONFIG(tooltip)
        stereoUseForExport->setText(QCoreApplication::translate("Stereo", "use for trajectory export", nullptr));
#if QT_CONFIG(tooltip)
        stereoUseCalibrationCenter->setToolTip(QCoreApplication::translate("Stereo", "while exporting pointGrey data directly, the coordinate center of calibration tab is used", nullptr));
#endif // QT_CONFIG(tooltip)
        stereoUseCalibrationCenter->setText(QCoreApplication::translate("Stereo", "use calib center", nullptr));
        label->setText(QCoreApplication::translate("Stereo", "stereo mask size: ", nullptr));
        label_2->setText(QCoreApplication::translate("Stereo", "edge mask size: ", nullptr));
        label_4->setText(QCoreApplication::translate("Stereo", "min disparity", nullptr));
        label_3->setText(QCoreApplication::translate("Stereo", "max disparity", nullptr));
        useEdge->setText(QCoreApplication::translate("Stereo", "use", nullptr));
        label_5->setText(QCoreApplication::translate("Stereo", "opacity", nullptr));
        hideWrong->setText(QCoreApplication::translate("Stereo", "hide", nullptr));
        stereoExport->setText(QCoreApplication::translate("Stereo", "export point cloud", nullptr));
        (void)Stereo;
    } // retranslateUi

};

namespace Ui {
    class Stereo: public Ui_Stereo {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_STEREO_H
