/********************************************************************************
** Form generated from reading UI file 'control.ui'
**
** Created by: Qt User Interface Compiler version 5.14.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CONTROL_H
#define UI_CONTROL_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "analysePlot.h"
#include "colorPlot.h"

QT_BEGIN_NAMESPACE

class Ui_Control
{
public:
    QHBoxLayout *horizontalLayout_19;
    QTabWidget *tabs;
    QWidget *calib;
    QHBoxLayout *horizontalLayout_11;
    QScrollArea *scrollArea;
    QWidget *scrollAreaWidgetContents;
    QVBoxLayout *verticalLayout_13;
    QGroupBox *groupBox;
    QGridLayout *gridLayout;
    QHBoxLayout *horizontalLayout_8;
    QScrollBar *filterBorderParamSize;
    QPushButton *filterBorderParamCol;
    QVBoxLayout *verticalLayout_8;
    QHBoxLayout *horizontalLayout_15;
    QCheckBox *filterBgShow;
    QCheckBox *filterBgUpdate;
    QSpacerItem *horizontalSpacer_11;
    QHBoxLayout *horizontalLayout_13;
    QPushButton *filterBgReset;
    QPushButton *filterBgLoad;
    QPushButton *filterBgSave;
    QSpacerItem *horizontalSpacer_7;
    QHBoxLayout *horizontalLayout_14;
    QCheckBox *filterBgDeleteTrj;
    QLabel *label_63;
    QSpinBox *filterBgDeleteNumber;
    QSpacerItem *horizontalSpacer_12;
    QHBoxLayout *horizontalLayout_3;
    QCheckBox *filterSwapH;
    QCheckBox *filterSwapV;
    QSpacerItem *horizontalSpacer_13;
    QCheckBox *filterSwap;
    QCheckBox *filterBorder;
    QScrollBar *filterContrastParam;
    QCheckBox *filterBrightContrast;
    QScrollBar *filterBrightParam;
    QCheckBox *filterBg;
    QLabel *label_18;
    QGroupBox *intr;
    QVBoxLayout *verticalLayout_12;
    QCheckBox *apply;
    QGridLayout *gridLayout1;
    QDoubleSpinBox *fx;
    QDoubleSpinBox *ty;
    QDoubleSpinBox *r2;
    QLabel *label_7;
    QDoubleSpinBox *tx;
    QLabel *label_3;
    QLabel *label_6;
    QLabel *label_8;
    QDoubleSpinBox *r4;
    QDoubleSpinBox *cx;
    QLabel *label;
    QLabel *label_4;
    QDoubleSpinBox *cy;
    QDoubleSpinBox *fy;
    QLabel *label_2;
    QLabel *label_5;
    QDoubleSpinBox *r6;
    QLabel *label_64;
    QGridLayout *gridLayout2;
    QCheckBox *fixCenter;
    QCheckBox *quadAspectRatio;
    QCheckBox *tangDist;
    QGridLayout *gridLayout3;
    QPushButton *autoCalib;
    QPushButton *calibFiles;
    QGroupBox *extr;
    QVBoxLayout *verticalLayout_7;
    QGridLayout *gridLayout_6;
    QLabel *label_58;
    QLabel *label_59;
    QDoubleSpinBox *trans1;
    QDoubleSpinBox *rot1;
    QDoubleSpinBox *trans2;
    QDoubleSpinBox *trans3;
    QDoubleSpinBox *rot3;
    QDoubleSpinBox *rot2;
    QHBoxLayout *horizontalLayout_10;
    QLabel *label_62;
    QPushButton *coordLoad3DCalibPoints;
    QPushButton *extrCalibFetch;
    QPushButton *extrCalibSave;
    QPushButton *extrCalibShowPoints;
    QPushButton *extrCalibShowError;
    QGroupBox *align_2;
    QVBoxLayout *verticalLayout_14;
    QGridLayout *gridLayout_7;
    QCheckBox *coordShow;
    QCheckBox *coordFix;
    QSpacerItem *horizontalSpacer_9;
    QTabWidget *coordTab;
    QWidget *coordTab3D;
    QVBoxLayout *verticalLayout_5;
    QGridLayout *gridLayout_5;
    QLabel *label_54;
    QScrollBar *coord3DTransX;
    QSpinBox *coord3DTransX_spin;
    QLabel *label_55;
    QScrollBar *coord3DTransY;
    QSpinBox *coord3DTransY_spin;
    QLabel *label_57;
    QScrollBar *coord3DTransZ;
    QSpinBox *coord3DTransZ_spin;
    QLabel *label_56;
    QScrollBar *coord3DAxeLen;
    QSpinBox *coord3DAxeLen_spin;
    QHBoxLayout *horizontalLayout_6;
    QLabel *label_60;
    QCheckBox *coord3DSwapX;
    QCheckBox *coord3DSwapY;
    QCheckBox *coord3DSwapZ;
    QSpacerItem *horizontalSpacer_5;
    QHBoxLayout *horizontalLayout_7;
    QLabel *label_61;
    QCheckBox *extCalibPointsShow;
    QCheckBox *extVanishPointsShow;
    QSpacerItem *horizontalSpacer_6;
    QSpacerItem *verticalSpacer_2;
    QWidget *coordTab2D;
    QVBoxLayout *verticalLayout_4;
    QGridLayout *gridLayout4;
    QSpinBox *coordTransX_spin;
    QLabel *label_34;
    QScrollBar *coordTransX;
    QLabel *label_32;
    QScrollBar *coordTransY;
    QLabel *label_33;
    QScrollBar *coordScale;
    QSpinBox *coordTransY_spin;
    QSpinBox *coordScale_spin;
    QLabel *label_35;
    QScrollBar *coordRotate;
    QSpinBox *coordRotate_spin;
    QGridLayout *gridLayout5;
    QLabel *label_14;
    QDoubleSpinBox *coordAltitude;
    QLabel *coordAltitudeMeasured;
    QSpacerItem *horizontalSpacer_8;
    QGridLayout *gridLayout6;
    QLabel *label_37;
    QDoubleSpinBox *coordUnit;
    QSpacerItem *spacerItem;
    QCheckBox *coordUseIntrinsic;
    QGroupBox *align;
    QVBoxLayout *verticalLayout_15;
    QHBoxLayout *horizontalLayout_12;
    QCheckBox *gridShow;
    QCheckBox *gridFix;
    QSpacerItem *horizontalSpacer_10;
    QTabWidget *gridTab;
    QWidget *gridTab3D;
    QVBoxLayout *verticalLayout_11;
    QGridLayout *gridLayout_3;
    QLabel *label_15;
    QScrollBar *grid3DTransX;
    QSpinBox *grid3DTransX_spin;
    QLabel *label_49;
    QScrollBar *grid3DTransY;
    QSpinBox *grid3DTransY_spin;
    QLabel *label_50;
    QScrollBar *grid3DTransZ;
    QSpinBox *grid3DTransZ_spin;
    QLabel *label_51;
    QScrollBar *grid3DResolution;
    QSpinBox *grid3DResolution_spin;
    QWidget *gridTab2D;
    QVBoxLayout *verticalLayout_10;
    QGridLayout *gridLayout_100;
    QLabel *label_10;
    QScrollBar *gridTransX;
    QSpinBox *gridTransX_spin;
    QLabel *label_11;
    QScrollBar *gridTransY;
    QSpinBox *gridTransY_spin;
    QLabel *label_9;
    QScrollBar *gridRotate;
    QSpinBox *gridRot_spin;
    QLabel *label_12;
    QScrollBar *gridScale;
    QSpinBox *gridScale_spin;
    QWidget *rec;
    QHBoxLayout *horizontalLayout_16;
    QScrollArea *scrollArea_2;
    QWidget *scrollAreaWidgetContents_2;
    QVBoxLayout *verticalLayout_16;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *hboxLayout;
    QCheckBox *performRecognition;
    QGridLayout *gridLayout7;
    QSpinBox *recoStep;
    QLabel *label_27;
    QSpacerItem *spacerItem1;
    QSpacerItem *horizontalSpacer_3;
    QGridLayout *gridLayout_2;
    QLabel *label_26;
    QLabel *recoNumberNow;
    QSpacerItem *horizontalSpacer_2;
    QHBoxLayout *horizontalLayout_4;
    QComboBox *recoMethod;
    QPushButton *recoStereoShow;
    QGroupBox *groupBox_2;
    QCheckBox *roiShow;
    QCheckBox *roiFix;
    QGroupBox *groupBox_5;
    QVBoxLayout *verticalLayout_19;
    QVBoxLayout *vboxLayout;
    QHBoxLayout *hboxLayout1;
    QLabel *label_19;
    QScrollBar *markerBrightness;
    QCheckBox *markerIgnoreWithout;
    QGroupBox *colorBox;
    QVBoxLayout *verticalLayout_9;
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout_2;
    QCheckBox *recoShowColor;
    QPushButton *recoOptimizeColor;
    QSpacerItem *horizontalSpacer_4;
    QHBoxLayout *hboxLayout2;
    QLabel *label_28;
    QComboBox *recoColorModel;
    QCheckBox *recoAutoWB;
    QSpacerItem *spacerItem2;
    QHBoxLayout *hboxLayout3;
    QLabel *label_29;
    QComboBox *recoColorX;
    QLabel *label_30;
    QComboBox *recoColorY;
    QLabel *label_31;
    QScrollBar *recoColorZ;
    QHBoxLayout *hboxLayout4;
    QLabel *label_38;
    QScrollBar *recoGreyLevel;
    QLabel *label_39;
    QScrollBar *recoSymbolSize;
    ColorPlot *colorPlot;
    QGroupBox *groupBox_4;
    QWidget *layoutWidget11;
    QVBoxLayout *vboxLayout1;
    QGridLayout *gridLayout8;
    QLabel *label_44;
    QLabel *label_40;
    QScrollBar *mapX;
    QScrollBar *mapH;
    QScrollBar *mapW;
    QScrollBar *mapY;
    QLabel *label_43;
    QLabel *label_42;
    QSpinBox *mapNr;
    QCheckBox *mapColor;
    QHBoxLayout *hboxLayout5;
    QLabel *label_45;
    QDoubleSpinBox *mapHeight;
    QPushButton *mapAdd;
    QPushButton *mapDel;
    QPushButton *mapColorRange;
    QHBoxLayout *hboxLayout6;
    QLabel *label_46;
    QDoubleSpinBox *mapDefaultHeight;
    QPushButton *mapDistribution;
    QPushButton *mapResetHeight;
    QPushButton *mapResetPos;
    QSpacerItem *verticalSpacer_3;
    QWidget *track;
    QHBoxLayout *horizontalLayout_17;
    QScrollArea *scrollArea_3;
    QWidget *scrollAreaWidgetContents_3;
    QVBoxLayout *verticalLayout_17;
    QHBoxLayout *hboxLayout7;
    QCheckBox *trackOnlineCalc;
    QCheckBox *trackRepeat;
    QSpinBox *trackRepeatQual;
    QSpacerItem *spacerItem3;
    QHBoxLayout *horizontalLayout_5;
    QCheckBox *trackExtrapolation;
    QCheckBox *trackMerge;
    QCheckBox *trackOnlyVisible;
    QSpacerItem *horizontalSpacer_15;
    QGridLayout *gridLayout9;
    QLabel *trackNumberNow;
    QLabel *label_22;
    QLabel *label_23;
    QLabel *trackNumberAll;
    QSpacerItem *spacerItem4;
    QLabel *label_65;
    QLabel *trackNumberVisible;
    QGridLayout *gridLayout10;
    QPushButton *trackExport;
    QPushButton *trackCalc;
    QPushButton *trackImport;
    QPushButton *trackReset;
    QGroupBox *groupBox_8;
    QCheckBox *trackRoiShow;
    QCheckBox *trackRoiFix;
    QGroupBox *groupBox_7;
    QVBoxLayout *verticalLayout_6;
    QGridLayout *gridLayout_4;
    QCheckBox *trackMissingFrames;
    QCheckBox *trackRecalcHeight;
    QCheckBox *trackAlternateHeight;
    QCheckBox *exportElimTp;
    QCheckBox *exportElimTrj;
    QCheckBox *exportSmooth;
    QCheckBox *exportViewDir;
    QCheckBox *exportAngleOfView;
    QCheckBox *exportUseM;
    QCheckBox *exportComment;
    QCheckBox *exportMarkerID;
    QHBoxLayout *horizontalLayout_9;
    QPushButton *trackTest;
    QCheckBox *testEqual;
    QCheckBox *testVelocity;
    QCheckBox *testInside;
    QCheckBox *testLength;
    QSpacerItem *horizontalSpacer_14;
    QGroupBox *groupBox_6;
    QVBoxLayout *verticalLayout_20;
    QVBoxLayout *verticalLayout;
    QGridLayout *gridLayout_17;
    QLabel *label_52;
    QScrollBar *trackRegionScale;
    QLabel *label_53;
    QScrollBar *trackRegionLevels;
    QScrollBar *trackErrorExponent;
    QLabel *label_66;
    QCheckBox *trackShowSearchSize;
    QGroupBox *groupBox_3;
    QVBoxLayout *verticalLayout_21;
    QVBoxLayout *vboxLayout2;
    QHBoxLayout *hboxLayout8;
    QCheckBox *trackShow;
    QCheckBox *trackFix;
    QSpacerItem *spacerItem5;
    QCheckBox *trackShowOnlyVisible;
    QHBoxLayout *horizontalLayout;
    QCheckBox *trackShowOnly;
    QSpinBox *trackShowOnlyNr;
    QPushButton *trackGotoNr;
    QPushButton *trackGotoStartNr;
    QPushButton *trackGotoEndNr;
    QSpacerItem *horizontalSpacer;
    QHBoxLayout *horizontalLayout_20;
    QCheckBox *trackShowOnlyList;
    QLineEdit *trackShowOnlyNrList;
    QPushButton *trackShowOnlyListButton;
    QSpacerItem *spacerItem6;
    QGridLayout *gridLayout11;
    QCheckBox *trackShowPath;
    QSpinBox *trackCurrentPointSize;
    QSpinBox *trackColColorSize;
    QCheckBox *trackShowCurrentPoint;
    QLabel *label_25;
    QLabel *label_36;
    QCheckBox *trackShowColColor;
    QCheckBox *trackShowColorMarker;
    QCheckBox *trackShowNumber;
    QLabel *label_48;
    QSpinBox *trackNumberSize;
    QCheckBox *trackNumberBold;
    QCheckBox *trackHeadSized;
    QCheckBox *trackShowHeightIndividual;
    QPushButton *trackPathColorButton;
    QSpinBox *trackPathWidth;
    QCheckBox *trackShowPoints;
    QCheckBox *trackShowPointsColored;
    QLabel *label_13;
    QLabel *label_41;
    QSpinBox *trackColorMarkerSize;
    QSpinBox *trackPointSize;
    QLabel *label_47;
    QCheckBox *trackShowGroundPosition;
    QLabel *label_67;
    QSpinBox *trackGroundPositionSize;
    QCheckBox *trackShowGroundPath;
    QLabel *label_68;
    QSpinBox *trackGroundPathSize;
    QPushButton *trackGroundPathColorButton;
    QSpacerItem *spacerItem7;
    QGridLayout *gridLayout12;
    QLabel *label_17;
    QSpinBox *trackShowAfter;
    QLabel *label_20;
    QSpinBox *trackShowBefore;
    QSpacerItem *spacerItem8;
    QSpacerItem *verticalSpacer_4;
    QWidget *ana;
    QHBoxLayout *horizontalLayout_18;
    QScrollArea *scrollArea_4;
    QWidget *scrollAreaWidgetContents_4;
    QVBoxLayout *verticalLayout_18;
    AnalysePlot *analysePlot;
    QHBoxLayout *hboxLayout9;
    QPushButton *anaCalculate;
    QCheckBox *anaMissingFrames;
    QHBoxLayout *hboxLayout10;
    QHBoxLayout *hboxLayout11;
    QLabel *label_21;
    QSpinBox *anaStep;
    QSpacerItem *spacerItem9;
    QCheckBox *anaMarkAct;
    QHBoxLayout *hboxLayout12;
    QLabel *label_16;
    QCheckBox *anaConsiderX;
    QCheckBox *anaConsiderY;
    QCheckBox *anaConsiderAbs;
    QCheckBox *anaConsiderRev;
    QCheckBox *showVoronoiCells;
    QSpacerItem *verticalSpacer;

    void setupUi(QWidget *Control)
    {
        if (Control->objectName().isEmpty())
            Control->setObjectName(QString::fromUtf8("Control"));
        Control->resize(380, 357);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(1);
        sizePolicy.setVerticalStretch(1);
        sizePolicy.setHeightForWidth(Control->sizePolicy().hasHeightForWidth());
        Control->setSizePolicy(sizePolicy);
        Control->setMinimumSize(QSize(375, 350));
        Control->setMaximumSize(QSize(380, 16777215));
        horizontalLayout_19 = new QHBoxLayout(Control);
        horizontalLayout_19->setObjectName(QString::fromUtf8("horizontalLayout_19"));
        horizontalLayout_19->setContentsMargins(0, 0, 0, 0);
        tabs = new QTabWidget(Control);
        tabs->setObjectName(QString::fromUtf8("tabs"));
        tabs->setEnabled(true);
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy1.setHorizontalStretch(1);
        sizePolicy1.setVerticalStretch(1);
        sizePolicy1.setHeightForWidth(tabs->sizePolicy().hasHeightForWidth());
        tabs->setSizePolicy(sizePolicy1);
        tabs->setMinimumSize(QSize(370, 350));
        tabs->setMaximumSize(QSize(15777215, 16777215));
        tabs->setLayoutDirection(Qt::LeftToRight);
        tabs->setTabsClosable(false);
        tabs->setMovable(false);
        calib = new QWidget();
        calib->setObjectName(QString::fromUtf8("calib"));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Expanding);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(calib->sizePolicy().hasHeightForWidth());
        calib->setSizePolicy(sizePolicy2);
        horizontalLayout_11 = new QHBoxLayout(calib);
        horizontalLayout_11->setObjectName(QString::fromUtf8("horizontalLayout_11"));
        scrollArea = new QScrollArea(calib);
        scrollArea->setObjectName(QString::fromUtf8("scrollArea"));
        scrollArea->setMaximumSize(QSize(16777215, 16777215));
        scrollArea->setAutoFillBackground(true);
        scrollArea->setStyleSheet(QString::fromUtf8(""));
        scrollArea->setFrameShape(QFrame::StyledPanel);
        scrollArea->setLineWidth(1);
        scrollArea->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
        scrollArea->setWidgetResizable(true);
        scrollAreaWidgetContents = new QWidget();
        scrollAreaWidgetContents->setObjectName(QString::fromUtf8("scrollAreaWidgetContents"));
        scrollAreaWidgetContents->setGeometry(QRect(0, 0, 337, 1028));
        verticalLayout_13 = new QVBoxLayout(scrollAreaWidgetContents);
        verticalLayout_13->setObjectName(QString::fromUtf8("verticalLayout_13"));
        groupBox = new QGroupBox(scrollAreaWidgetContents);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        QSizePolicy sizePolicy3(QSizePolicy::Expanding, QSizePolicy::Minimum);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(groupBox->sizePolicy().hasHeightForWidth());
        groupBox->setSizePolicy(sizePolicy3);
        groupBox->setMinimumSize(QSize(300, 0));
        groupBox->setMaximumSize(QSize(16777215, 16777215));
        groupBox->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        groupBox->setFlat(false);
        groupBox->setCheckable(false);
        gridLayout = new QGridLayout(groupBox);
        gridLayout->setSpacing(4);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(5, 5, 5, 5);
        horizontalLayout_8 = new QHBoxLayout();
        horizontalLayout_8->setObjectName(QString::fromUtf8("horizontalLayout_8"));
        filterBorderParamSize = new QScrollBar(groupBox);
        filterBorderParamSize->setObjectName(QString::fromUtf8("filterBorderParamSize"));
        filterBorderParamSize->setMinimumSize(QSize(100, 0));
        filterBorderParamSize->setMaximum(500);
        filterBorderParamSize->setSingleStep(1);
        filterBorderParamSize->setOrientation(Qt::Horizontal);

        horizontalLayout_8->addWidget(filterBorderParamSize);

        filterBorderParamCol = new QPushButton(groupBox);
        filterBorderParamCol->setObjectName(QString::fromUtf8("filterBorderParamCol"));
        filterBorderParamCol->setMaximumSize(QSize(40, 18));

        horizontalLayout_8->addWidget(filterBorderParamCol);


        gridLayout->addLayout(horizontalLayout_8, 2, 1, 1, 1);

        verticalLayout_8 = new QVBoxLayout();
        verticalLayout_8->setObjectName(QString::fromUtf8("verticalLayout_8"));
        horizontalLayout_15 = new QHBoxLayout();
        horizontalLayout_15->setObjectName(QString::fromUtf8("horizontalLayout_15"));
        filterBgShow = new QCheckBox(groupBox);
        filterBgShow->setObjectName(QString::fromUtf8("filterBgShow"));

        horizontalLayout_15->addWidget(filterBgShow);

        filterBgUpdate = new QCheckBox(groupBox);
        filterBgUpdate->setObjectName(QString::fromUtf8("filterBgUpdate"));

        horizontalLayout_15->addWidget(filterBgUpdate);

        horizontalSpacer_11 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_15->addItem(horizontalSpacer_11);


        verticalLayout_8->addLayout(horizontalLayout_15);

        horizontalLayout_13 = new QHBoxLayout();
        horizontalLayout_13->setSpacing(0);
        horizontalLayout_13->setObjectName(QString::fromUtf8("horizontalLayout_13"));
        filterBgReset = new QPushButton(groupBox);
        filterBgReset->setObjectName(QString::fromUtf8("filterBgReset"));
        filterBgReset->setEnabled(true);
        QSizePolicy sizePolicy4(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(filterBgReset->sizePolicy().hasHeightForWidth());
        filterBgReset->setSizePolicy(sizePolicy4);
        filterBgReset->setMaximumSize(QSize(40, 18));

        horizontalLayout_13->addWidget(filterBgReset);

        filterBgLoad = new QPushButton(groupBox);
        filterBgLoad->setObjectName(QString::fromUtf8("filterBgLoad"));
        filterBgLoad->setEnabled(true);
        filterBgLoad->setMaximumSize(QSize(36, 18));

        horizontalLayout_13->addWidget(filterBgLoad);

        filterBgSave = new QPushButton(groupBox);
        filterBgSave->setObjectName(QString::fromUtf8("filterBgSave"));
        filterBgSave->setEnabled(true);
        filterBgSave->setMaximumSize(QSize(36, 18));

        horizontalLayout_13->addWidget(filterBgSave);

        horizontalSpacer_7 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_13->addItem(horizontalSpacer_7);


        verticalLayout_8->addLayout(horizontalLayout_13);

        horizontalLayout_14 = new QHBoxLayout();
        horizontalLayout_14->setObjectName(QString::fromUtf8("horizontalLayout_14"));
        filterBgDeleteTrj = new QCheckBox(groupBox);
        filterBgDeleteTrj->setObjectName(QString::fromUtf8("filterBgDeleteTrj"));
        filterBgDeleteTrj->setChecked(true);

        horizontalLayout_14->addWidget(filterBgDeleteTrj);

        label_63 = new QLabel(groupBox);
        label_63->setObjectName(QString::fromUtf8("label_63"));
        sizePolicy4.setHeightForWidth(label_63->sizePolicy().hasHeightForWidth());
        label_63->setSizePolicy(sizePolicy4);

        horizontalLayout_14->addWidget(label_63);

        filterBgDeleteNumber = new QSpinBox(groupBox);
        filterBgDeleteNumber->setObjectName(QString::fromUtf8("filterBgDeleteNumber"));
        filterBgDeleteNumber->setMinimum(1);
        filterBgDeleteNumber->setValue(3);

        horizontalLayout_14->addWidget(filterBgDeleteNumber);

        horizontalSpacer_12 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_14->addItem(horizontalSpacer_12);


        verticalLayout_8->addLayout(horizontalLayout_14);


        gridLayout->addLayout(verticalLayout_8, 3, 1, 1, 1);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        filterSwapH = new QCheckBox(groupBox);
        filterSwapH->setObjectName(QString::fromUtf8("filterSwapH"));

        horizontalLayout_3->addWidget(filterSwapH);

        filterSwapV = new QCheckBox(groupBox);
        filterSwapV->setObjectName(QString::fromUtf8("filterSwapV"));
        filterSwapV->setChecked(true);

        horizontalLayout_3->addWidget(filterSwapV);

        horizontalSpacer_13 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_13);


        gridLayout->addLayout(horizontalLayout_3, 4, 1, 1, 1);

        filterSwap = new QCheckBox(groupBox);
        filterSwap->setObjectName(QString::fromUtf8("filterSwap"));

        gridLayout->addWidget(filterSwap, 4, 0, 1, 1);

        filterBorder = new QCheckBox(groupBox);
        filterBorder->setObjectName(QString::fromUtf8("filterBorder"));

        gridLayout->addWidget(filterBorder, 2, 0, 1, 1);

        filterContrastParam = new QScrollBar(groupBox);
        filterContrastParam->setObjectName(QString::fromUtf8("filterContrastParam"));
        filterContrastParam->setMinimumSize(QSize(100, 0));
        filterContrastParam->setMinimum(-100);
        filterContrastParam->setMaximum(100);
        filterContrastParam->setOrientation(Qt::Horizontal);

        gridLayout->addWidget(filterContrastParam, 1, 1, 1, 1);

        filterBrightContrast = new QCheckBox(groupBox);
        filterBrightContrast->setObjectName(QString::fromUtf8("filterBrightContrast"));

        gridLayout->addWidget(filterBrightContrast, 0, 0, 1, 1);

        filterBrightParam = new QScrollBar(groupBox);
        filterBrightParam->setObjectName(QString::fromUtf8("filterBrightParam"));
        filterBrightParam->setMinimumSize(QSize(100, 0));
        filterBrightParam->setMinimum(-100);
        filterBrightParam->setMaximum(100);
        filterBrightParam->setOrientation(Qt::Horizontal);

        gridLayout->addWidget(filterBrightParam, 0, 1, 1, 1);

        filterBg = new QCheckBox(groupBox);
        filterBg->setObjectName(QString::fromUtf8("filterBg"));
        filterBg->setChecked(false);

        gridLayout->addWidget(filterBg, 3, 0, 1, 1);

        label_18 = new QLabel(groupBox);
        label_18->setObjectName(QString::fromUtf8("label_18"));
        sizePolicy4.setHeightForWidth(label_18->sizePolicy().hasHeightForWidth());
        label_18->setSizePolicy(sizePolicy4);

        gridLayout->addWidget(label_18, 1, 0, 1, 1);

        gridLayout->setColumnStretch(1, 1);

        verticalLayout_13->addWidget(groupBox);

        intr = new QGroupBox(scrollAreaWidgetContents);
        intr->setObjectName(QString::fromUtf8("intr"));
        sizePolicy3.setHeightForWidth(intr->sizePolicy().hasHeightForWidth());
        intr->setSizePolicy(sizePolicy3);
        intr->setMinimumSize(QSize(300, 0));
        intr->setMaximumSize(QSize(16777215, 16777215));
        verticalLayout_12 = new QVBoxLayout(intr);
        verticalLayout_12->setObjectName(QString::fromUtf8("verticalLayout_12"));
        verticalLayout_12->setContentsMargins(5, 5, 5, 5);
        apply = new QCheckBox(intr);
        apply->setObjectName(QString::fromUtf8("apply"));

        verticalLayout_12->addWidget(apply);

        gridLayout1 = new QGridLayout();
        gridLayout1->setSpacing(4);
        gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
        gridLayout1->setContentsMargins(0, 0, 0, 0);
        fx = new QDoubleSpinBox(intr);
        fx->setObjectName(QString::fromUtf8("fx"));
        fx->setAlignment(Qt::AlignRight);
        fx->setDecimals(2);
        fx->setMinimum(500.000000000000000);
        fx->setMaximum(5000.000000000000000);
        fx->setValue(1000.000000000000000);

        gridLayout1->addWidget(fx, 0, 1, 1, 1);

        ty = new QDoubleSpinBox(intr);
        ty->setObjectName(QString::fromUtf8("ty"));
        ty->setAlignment(Qt::AlignRight);
        ty->setDecimals(6);
        ty->setMinimum(-5.000000000000000);
        ty->setMaximum(5.000000000000000);
        ty->setSingleStep(0.001000000000000);

        gridLayout1->addWidget(ty, 4, 3, 1, 1);

        r2 = new QDoubleSpinBox(intr);
        r2->setObjectName(QString::fromUtf8("r2"));
        r2->setAlignment(Qt::AlignRight);
        r2->setDecimals(6);
        r2->setMinimum(-5.000000000000000);
        r2->setMaximum(5.000000000000000);
        r2->setSingleStep(0.010000000000000);

        gridLayout1->addWidget(r2, 2, 1, 1, 1);

        label_7 = new QLabel(intr);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        QFont font;
        font.setPointSize(8);
        label_7->setFont(font);
        label_7->setLayoutDirection(Qt::LeftToRight);
        label_7->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout1->addWidget(label_7, 4, 0, 1, 1);

        tx = new QDoubleSpinBox(intr);
        tx->setObjectName(QString::fromUtf8("tx"));
        tx->setAlignment(Qt::AlignRight);
        tx->setDecimals(6);
        tx->setMinimum(-5.000000000000000);
        tx->setMaximum(5.000000000000000);
        tx->setSingleStep(0.001000000000000);

        gridLayout1->addWidget(tx, 4, 1, 1, 1);

        label_3 = new QLabel(intr);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setLayoutDirection(Qt::LeftToRight);
        label_3->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout1->addWidget(label_3, 1, 0, 1, 1);

        label_6 = new QLabel(intr);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout1->addWidget(label_6, 2, 2, 1, 1);

        label_8 = new QLabel(intr);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        label_8->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout1->addWidget(label_8, 4, 2, 1, 1);

        r4 = new QDoubleSpinBox(intr);
        r4->setObjectName(QString::fromUtf8("r4"));
        r4->setAlignment(Qt::AlignRight);
        r4->setDecimals(6);
        r4->setMinimum(-5.000000000000000);
        r4->setMaximum(5.000000000000000);
        r4->setSingleStep(0.010000000000000);

        gridLayout1->addWidget(r4, 2, 3, 1, 1);

        cx = new QDoubleSpinBox(intr);
        cx->setObjectName(QString::fromUtf8("cx"));
        cx->setAlignment(Qt::AlignRight);
        cx->setDecimals(2);
        cx->setMinimum(1.000000000000000);
        cx->setMaximum(1920.000000000000000);
        cx->setValue(960.000000000000000);

        gridLayout1->addWidget(cx, 1, 1, 1, 1);

        label = new QLabel(intr);
        label->setObjectName(QString::fromUtf8("label"));
        label->setFont(font);
        label->setLayoutDirection(Qt::LeftToRight);
        label->setScaledContents(false);
        label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout1->addWidget(label, 0, 0, 1, 1);

        label_4 = new QLabel(intr);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout1->addWidget(label_4, 1, 2, 1, 1);

        cy = new QDoubleSpinBox(intr);
        cy->setObjectName(QString::fromUtf8("cy"));
        cy->setAlignment(Qt::AlignRight);
        cy->setDecimals(2);
        cy->setMinimum(1.000000000000000);
        cy->setMaximum(1080.000000000000000);
        cy->setValue(540.000000000000000);

        gridLayout1->addWidget(cy, 1, 3, 1, 1);

        fy = new QDoubleSpinBox(intr);
        fy->setObjectName(QString::fromUtf8("fy"));
        fy->setAlignment(Qt::AlignRight);
        fy->setDecimals(2);
        fy->setMinimum(500.000000000000000);
        fy->setMaximum(5000.000000000000000);
        fy->setValue(1000.000000000000000);

        gridLayout1->addWidget(fy, 0, 3, 1, 1);

        label_2 = new QLabel(intr);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout1->addWidget(label_2, 0, 2, 1, 1);

        label_5 = new QLabel(intr);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setFont(font);
        label_5->setLayoutDirection(Qt::LeftToRight);
        label_5->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout1->addWidget(label_5, 2, 0, 1, 1);

        r6 = new QDoubleSpinBox(intr);
        r6->setObjectName(QString::fromUtf8("r6"));
        r6->setAlignment(Qt::AlignRight);
        r6->setDecimals(6);
        r6->setMinimum(-5.000000000000000);
        r6->setMaximum(5.000000000000000);
        r6->setSingleStep(0.010000000000000);

        gridLayout1->addWidget(r6, 3, 1, 1, 1);

        label_64 = new QLabel(intr);
        label_64->setObjectName(QString::fromUtf8("label_64"));
        label_64->setLayoutDirection(Qt::LeftToRight);
        label_64->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout1->addWidget(label_64, 3, 0, 1, 1, Qt::AlignRight);

        gridLayout1->setColumnStretch(1, 1);
        gridLayout1->setColumnStretch(3, 1);

        verticalLayout_12->addLayout(gridLayout1);

        gridLayout2 = new QGridLayout();
        gridLayout2->setSpacing(4);
        gridLayout2->setObjectName(QString::fromUtf8("gridLayout2"));
        gridLayout2->setContentsMargins(0, 0, 0, 0);
        fixCenter = new QCheckBox(intr);
        fixCenter->setObjectName(QString::fromUtf8("fixCenter"));

        gridLayout2->addWidget(fixCenter, 0, 1, 1, 1);

        quadAspectRatio = new QCheckBox(intr);
        quadAspectRatio->setObjectName(QString::fromUtf8("quadAspectRatio"));

        gridLayout2->addWidget(quadAspectRatio, 0, 0, 1, 1);

        tangDist = new QCheckBox(intr);
        tangDist->setObjectName(QString::fromUtf8("tangDist"));
        tangDist->setChecked(true);

        gridLayout2->addWidget(tangDist, 0, 2, 1, 1);


        verticalLayout_12->addLayout(gridLayout2);

        gridLayout3 = new QGridLayout();
        gridLayout3->setSpacing(4);
        gridLayout3->setObjectName(QString::fromUtf8("gridLayout3"));
        gridLayout3->setContentsMargins(0, 0, 0, 0);
        autoCalib = new QPushButton(intr);
        autoCalib->setObjectName(QString::fromUtf8("autoCalib"));
        autoCalib->setEnabled(false);

        gridLayout3->addWidget(autoCalib, 0, 0, 1, 1);

        calibFiles = new QPushButton(intr);
        calibFiles->setObjectName(QString::fromUtf8("calibFiles"));

        gridLayout3->addWidget(calibFiles, 0, 1, 1, 1);


        verticalLayout_12->addLayout(gridLayout3);


        verticalLayout_13->addWidget(intr);

        extr = new QGroupBox(scrollAreaWidgetContents);
        extr->setObjectName(QString::fromUtf8("extr"));
        sizePolicy3.setHeightForWidth(extr->sizePolicy().hasHeightForWidth());
        extr->setSizePolicy(sizePolicy3);
        extr->setMinimumSize(QSize(300, 0));
        extr->setMaximumSize(QSize(16777215, 16777215));
        verticalLayout_7 = new QVBoxLayout(extr);
        verticalLayout_7->setObjectName(QString::fromUtf8("verticalLayout_7"));
        verticalLayout_7->setContentsMargins(5, 5, 5, 5);
        gridLayout_6 = new QGridLayout();
        gridLayout_6->setObjectName(QString::fromUtf8("gridLayout_6"));
        label_58 = new QLabel(extr);
        label_58->setObjectName(QString::fromUtf8("label_58"));
        label_58->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_6->addWidget(label_58, 0, 0, 1, 1);

        label_59 = new QLabel(extr);
        label_59->setObjectName(QString::fromUtf8("label_59"));
        label_59->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_6->addWidget(label_59, 1, 0, 1, 1);

        trans1 = new QDoubleSpinBox(extr);
        trans1->setObjectName(QString::fromUtf8("trans1"));
        trans1->setDecimals(3);
        trans1->setMinimum(-10000.000000000000000);
        trans1->setMaximum(10000.000000000000000);
        trans1->setSingleStep(0.100000000000000);

        gridLayout_6->addWidget(trans1, 1, 1, 1, 1);

        rot1 = new QDoubleSpinBox(extr);
        rot1->setObjectName(QString::fromUtf8("rot1"));
        rot1->setDecimals(3);
        rot1->setMinimum(-100.000000000000000);
        rot1->setSingleStep(0.001000000000000);

        gridLayout_6->addWidget(rot1, 0, 1, 1, 1);

        trans2 = new QDoubleSpinBox(extr);
        trans2->setObjectName(QString::fromUtf8("trans2"));
        trans2->setDecimals(3);
        trans2->setMinimum(-10000.000000000000000);
        trans2->setMaximum(10000.000000000000000);
        trans2->setSingleStep(0.100000000000000);

        gridLayout_6->addWidget(trans2, 1, 2, 1, 1);

        trans3 = new QDoubleSpinBox(extr);
        trans3->setObjectName(QString::fromUtf8("trans3"));
        trans3->setDecimals(3);
        trans3->setMinimum(-10000.000000000000000);
        trans3->setMaximum(10000.000000000000000);
        trans3->setSingleStep(0.100000000000000);
        trans3->setValue(-500.000000000000000);

        gridLayout_6->addWidget(trans3, 1, 3, 1, 1);

        rot3 = new QDoubleSpinBox(extr);
        rot3->setObjectName(QString::fromUtf8("rot3"));
        rot3->setDecimals(3);
        rot3->setMinimum(-100.000000000000000);
        rot3->setSingleStep(0.001000000000000);

        gridLayout_6->addWidget(rot3, 0, 3, 1, 1);

        rot2 = new QDoubleSpinBox(extr);
        rot2->setObjectName(QString::fromUtf8("rot2"));
        rot2->setDecimals(3);
        rot2->setMinimum(-100.000000000000000);
        rot2->setSingleStep(0.001000000000000);

        gridLayout_6->addWidget(rot2, 0, 2, 1, 1);


        verticalLayout_7->addLayout(gridLayout_6);

        horizontalLayout_10 = new QHBoxLayout();
        horizontalLayout_10->setObjectName(QString::fromUtf8("horizontalLayout_10"));
        label_62 = new QLabel(extr);
        label_62->setObjectName(QString::fromUtf8("label_62"));
        QSizePolicy sizePolicy5(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy5.setHorizontalStretch(0);
        sizePolicy5.setVerticalStretch(0);
        sizePolicy5.setHeightForWidth(label_62->sizePolicy().hasHeightForWidth());
        label_62->setSizePolicy(sizePolicy5);
        label_62->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_10->addWidget(label_62);

        coordLoad3DCalibPoints = new QPushButton(extr);
        coordLoad3DCalibPoints->setObjectName(QString::fromUtf8("coordLoad3DCalibPoints"));
        QSizePolicy sizePolicy6(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy6.setHorizontalStretch(0);
        sizePolicy6.setVerticalStretch(0);
        sizePolicy6.setHeightForWidth(coordLoad3DCalibPoints->sizePolicy().hasHeightForWidth());
        coordLoad3DCalibPoints->setSizePolicy(sizePolicy6);
        coordLoad3DCalibPoints->setMinimumSize(QSize(30, 18));
        coordLoad3DCalibPoints->setMaximumSize(QSize(40, 18));

        horizontalLayout_10->addWidget(coordLoad3DCalibPoints);

        extrCalibFetch = new QPushButton(extr);
        extrCalibFetch->setObjectName(QString::fromUtf8("extrCalibFetch"));
        sizePolicy6.setHeightForWidth(extrCalibFetch->sizePolicy().hasHeightForWidth());
        extrCalibFetch->setSizePolicy(sizePolicy6);
        extrCalibFetch->setMinimumSize(QSize(30, 18));
        extrCalibFetch->setMaximumSize(QSize(40, 18));

        horizontalLayout_10->addWidget(extrCalibFetch);

        extrCalibSave = new QPushButton(extr);
        extrCalibSave->setObjectName(QString::fromUtf8("extrCalibSave"));
        sizePolicy6.setHeightForWidth(extrCalibSave->sizePolicy().hasHeightForWidth());
        extrCalibSave->setSizePolicy(sizePolicy6);
        extrCalibSave->setMinimumSize(QSize(30, 18));
        extrCalibSave->setMaximumSize(QSize(40, 18));

        horizontalLayout_10->addWidget(extrCalibSave);

        extrCalibShowPoints = new QPushButton(extr);
        extrCalibShowPoints->setObjectName(QString::fromUtf8("extrCalibShowPoints"));
        sizePolicy6.setHeightForWidth(extrCalibShowPoints->sizePolicy().hasHeightForWidth());
        extrCalibShowPoints->setSizePolicy(sizePolicy6);
        extrCalibShowPoints->setMinimumSize(QSize(30, 18));
        extrCalibShowPoints->setMaximumSize(QSize(40, 18));

        horizontalLayout_10->addWidget(extrCalibShowPoints);

        extrCalibShowError = new QPushButton(extr);
        extrCalibShowError->setObjectName(QString::fromUtf8("extrCalibShowError"));
        sizePolicy6.setHeightForWidth(extrCalibShowError->sizePolicy().hasHeightForWidth());
        extrCalibShowError->setSizePolicy(sizePolicy6);
        extrCalibShowError->setMinimumSize(QSize(30, 18));
        extrCalibShowError->setMaximumSize(QSize(40, 18));

        horizontalLayout_10->addWidget(extrCalibShowError);


        verticalLayout_7->addLayout(horizontalLayout_10);


        verticalLayout_13->addWidget(extr);

        align_2 = new QGroupBox(scrollAreaWidgetContents);
        align_2->setObjectName(QString::fromUtf8("align_2"));
        sizePolicy3.setHeightForWidth(align_2->sizePolicy().hasHeightForWidth());
        align_2->setSizePolicy(sizePolicy3);
        align_2->setMinimumSize(QSize(300, 271));
        align_2->setMaximumSize(QSize(16777215, 16777215));
        verticalLayout_14 = new QVBoxLayout(align_2);
        verticalLayout_14->setObjectName(QString::fromUtf8("verticalLayout_14"));
        verticalLayout_14->setContentsMargins(5, 5, 5, 5);
        gridLayout_7 = new QGridLayout();
        gridLayout_7->setObjectName(QString::fromUtf8("gridLayout_7"));
        coordShow = new QCheckBox(align_2);
        coordShow->setObjectName(QString::fromUtf8("coordShow"));
        coordShow->setMinimumSize(QSize(57, 17));
        coordShow->setMaximumSize(QSize(16777215, 17));

        gridLayout_7->addWidget(coordShow, 0, 0, 1, 1);

        coordFix = new QCheckBox(align_2);
        coordFix->setObjectName(QString::fromUtf8("coordFix"));
        coordFix->setMinimumSize(QSize(0, 17));
        coordFix->setMaximumSize(QSize(16777215, 17));

        gridLayout_7->addWidget(coordFix, 0, 1, 1, 1);

        horizontalSpacer_9 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_7->addItem(horizontalSpacer_9, 0, 2, 1, 1);

        gridLayout_7->setColumnStretch(2, 1);

        verticalLayout_14->addLayout(gridLayout_7);

        coordTab = new QTabWidget(align_2);
        coordTab->setObjectName(QString::fromUtf8("coordTab"));
        sizePolicy5.setHeightForWidth(coordTab->sizePolicy().hasHeightForWidth());
        coordTab->setSizePolicy(sizePolicy5);
        coordTab->setFocusPolicy(Qt::StrongFocus);
        coordTab->setLayoutDirection(Qt::LeftToRight);
        coordTab->setTabPosition(QTabWidget::North);
        coordTab->setTabShape(QTabWidget::Rounded);
        coordTab->setElideMode(Qt::ElideNone);
        coordTab->setUsesScrollButtons(true);
        coordTab->setTabsClosable(false);
        coordTab3D = new QWidget();
        coordTab3D->setObjectName(QString::fromUtf8("coordTab3D"));
        verticalLayout_5 = new QVBoxLayout(coordTab3D);
        verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
        gridLayout_5 = new QGridLayout();
        gridLayout_5->setSpacing(4);
        gridLayout_5->setObjectName(QString::fromUtf8("gridLayout_5"));
        label_54 = new QLabel(coordTab3D);
        label_54->setObjectName(QString::fromUtf8("label_54"));
        label_54->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_5->addWidget(label_54, 0, 0, 1, 1);

        coord3DTransX = new QScrollBar(coordTab3D);
        coord3DTransX->setObjectName(QString::fromUtf8("coord3DTransX"));
        sizePolicy5.setHeightForWidth(coord3DTransX->sizePolicy().hasHeightForWidth());
        coord3DTransX->setSizePolicy(sizePolicy5);
        coord3DTransX->setMinimumSize(QSize(50, 17));
        coord3DTransX->setMaximumSize(QSize(16777215, 17));
        coord3DTransX->setMinimum(-10000);
        coord3DTransX->setMaximum(10000);
        coord3DTransX->setPageStep(10);
        coord3DTransX->setValue(0);
        coord3DTransX->setSliderPosition(0);
        coord3DTransX->setOrientation(Qt::Horizontal);

        gridLayout_5->addWidget(coord3DTransX, 0, 1, 1, 1);

        coord3DTransX_spin = new QSpinBox(coordTab3D);
        coord3DTransX_spin->setObjectName(QString::fromUtf8("coord3DTransX_spin"));
        coord3DTransX_spin->setMinimumSize(QSize(0, 17));
        coord3DTransX_spin->setMaximumSize(QSize(16777215, 17));
        coord3DTransX_spin->setMinimum(-10000);
        coord3DTransX_spin->setMaximum(10000);

        gridLayout_5->addWidget(coord3DTransX_spin, 0, 2, 1, 1);

        label_55 = new QLabel(coordTab3D);
        label_55->setObjectName(QString::fromUtf8("label_55"));
        label_55->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_5->addWidget(label_55, 1, 0, 1, 1);

        coord3DTransY = new QScrollBar(coordTab3D);
        coord3DTransY->setObjectName(QString::fromUtf8("coord3DTransY"));
        coord3DTransY->setMinimumSize(QSize(0, 17));
        coord3DTransY->setMaximumSize(QSize(16777215, 17));
        coord3DTransY->setMinimum(-10000);
        coord3DTransY->setMaximum(10000);
        coord3DTransY->setValue(0);
        coord3DTransY->setSliderPosition(0);
        coord3DTransY->setOrientation(Qt::Horizontal);

        gridLayout_5->addWidget(coord3DTransY, 1, 1, 1, 1);

        coord3DTransY_spin = new QSpinBox(coordTab3D);
        coord3DTransY_spin->setObjectName(QString::fromUtf8("coord3DTransY_spin"));
        coord3DTransY_spin->setMinimumSize(QSize(0, 17));
        coord3DTransY_spin->setMaximumSize(QSize(16777215, 17));
        coord3DTransY_spin->setMinimum(-10000);
        coord3DTransY_spin->setMaximum(10000);

        gridLayout_5->addWidget(coord3DTransY_spin, 1, 2, 1, 1);

        label_57 = new QLabel(coordTab3D);
        label_57->setObjectName(QString::fromUtf8("label_57"));
        label_57->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_5->addWidget(label_57, 2, 0, 1, 1);

        coord3DTransZ = new QScrollBar(coordTab3D);
        coord3DTransZ->setObjectName(QString::fromUtf8("coord3DTransZ"));
        coord3DTransZ->setMinimumSize(QSize(0, 17));
        coord3DTransZ->setMaximumSize(QSize(16777215, 17));
        coord3DTransZ->setMinimum(-10000);
        coord3DTransZ->setMaximum(10000);
        coord3DTransZ->setPageStep(10);
        coord3DTransZ->setSliderPosition(0);
        coord3DTransZ->setOrientation(Qt::Horizontal);

        gridLayout_5->addWidget(coord3DTransZ, 2, 1, 1, 1);

        coord3DTransZ_spin = new QSpinBox(coordTab3D);
        coord3DTransZ_spin->setObjectName(QString::fromUtf8("coord3DTransZ_spin"));
        coord3DTransZ_spin->setMinimumSize(QSize(0, 17));
        coord3DTransZ_spin->setMaximumSize(QSize(16777215, 17));
        coord3DTransZ_spin->setMinimum(-10000);
        coord3DTransZ_spin->setMaximum(10000);

        gridLayout_5->addWidget(coord3DTransZ_spin, 2, 2, 1, 1);

        label_56 = new QLabel(coordTab3D);
        label_56->setObjectName(QString::fromUtf8("label_56"));
        label_56->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_5->addWidget(label_56, 3, 0, 1, 1);

        coord3DAxeLen = new QScrollBar(coordTab3D);
        coord3DAxeLen->setObjectName(QString::fromUtf8("coord3DAxeLen"));
        coord3DAxeLen->setMinimumSize(QSize(0, 17));
        coord3DAxeLen->setMaximumSize(QSize(16777215, 17));
        coord3DAxeLen->setMinimum(1);
        coord3DAxeLen->setMaximum(10000);
        coord3DAxeLen->setValue(200);
        coord3DAxeLen->setSliderPosition(200);
        coord3DAxeLen->setOrientation(Qt::Horizontal);

        gridLayout_5->addWidget(coord3DAxeLen, 3, 1, 1, 1);

        coord3DAxeLen_spin = new QSpinBox(coordTab3D);
        coord3DAxeLen_spin->setObjectName(QString::fromUtf8("coord3DAxeLen_spin"));
        coord3DAxeLen_spin->setMinimumSize(QSize(0, 17));
        coord3DAxeLen_spin->setMaximumSize(QSize(16777215, 17));
        coord3DAxeLen_spin->setMinimum(1);
        coord3DAxeLen_spin->setMaximum(10000);
        coord3DAxeLen_spin->setValue(200);

        gridLayout_5->addWidget(coord3DAxeLen_spin, 3, 2, 1, 1);


        verticalLayout_5->addLayout(gridLayout_5);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        label_60 = new QLabel(coordTab3D);
        label_60->setObjectName(QString::fromUtf8("label_60"));
        label_60->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_6->addWidget(label_60);

        coord3DSwapX = new QCheckBox(coordTab3D);
        coord3DSwapX->setObjectName(QString::fromUtf8("coord3DSwapX"));
        coord3DSwapX->setMinimumSize(QSize(0, 17));
        coord3DSwapX->setMaximumSize(QSize(16777215, 17));

        horizontalLayout_6->addWidget(coord3DSwapX);

        coord3DSwapY = new QCheckBox(coordTab3D);
        coord3DSwapY->setObjectName(QString::fromUtf8("coord3DSwapY"));
        coord3DSwapY->setMinimumSize(QSize(0, 17));
        coord3DSwapY->setMaximumSize(QSize(16777215, 17));

        horizontalLayout_6->addWidget(coord3DSwapY);

        coord3DSwapZ = new QCheckBox(coordTab3D);
        coord3DSwapZ->setObjectName(QString::fromUtf8("coord3DSwapZ"));
        coord3DSwapZ->setMinimumSize(QSize(0, 17));
        coord3DSwapZ->setMaximumSize(QSize(16777215, 17));

        horizontalLayout_6->addWidget(coord3DSwapZ);

        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_6->addItem(horizontalSpacer_5);


        verticalLayout_5->addLayout(horizontalLayout_6);

        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        label_61 = new QLabel(coordTab3D);
        label_61->setObjectName(QString::fromUtf8("label_61"));
        label_61->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_7->addWidget(label_61);

        extCalibPointsShow = new QCheckBox(coordTab3D);
        extCalibPointsShow->setObjectName(QString::fromUtf8("extCalibPointsShow"));
        extCalibPointsShow->setMinimumSize(QSize(0, 17));
        extCalibPointsShow->setMaximumSize(QSize(16777215, 17));

        horizontalLayout_7->addWidget(extCalibPointsShow);

        extVanishPointsShow = new QCheckBox(coordTab3D);
        extVanishPointsShow->setObjectName(QString::fromUtf8("extVanishPointsShow"));
        extVanishPointsShow->setMinimumSize(QSize(0, 17));
        extVanishPointsShow->setMaximumSize(QSize(16777215, 17));

        horizontalLayout_7->addWidget(extVanishPointsShow);

        horizontalSpacer_6 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_7->addItem(horizontalSpacer_6);


        verticalLayout_5->addLayout(horizontalLayout_7);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_5->addItem(verticalSpacer_2);

        coordTab->addTab(coordTab3D, QString());
        coordTab2D = new QWidget();
        coordTab2D->setObjectName(QString::fromUtf8("coordTab2D"));
        verticalLayout_4 = new QVBoxLayout(coordTab2D);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        gridLayout4 = new QGridLayout();
        gridLayout4->setSpacing(4);
        gridLayout4->setObjectName(QString::fromUtf8("gridLayout4"));
        gridLayout4->setContentsMargins(0, 0, 0, 0);
        coordTransX_spin = new QSpinBox(coordTab2D);
        coordTransX_spin->setObjectName(QString::fromUtf8("coordTransX_spin"));
        coordTransX_spin->setMinimumSize(QSize(0, 18));
        coordTransX_spin->setMaximumSize(QSize(16777215, 18));
        coordTransX_spin->setMinimum(0);
        coordTransX_spin->setMaximum(10000);

        gridLayout4->addWidget(coordTransX_spin, 1, 2, 1, 1);

        label_34 = new QLabel(coordTab2D);
        label_34->setObjectName(QString::fromUtf8("label_34"));
        label_34->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout4->addWidget(label_34, 1, 0, 1, 1);

        coordTransX = new QScrollBar(coordTab2D);
        coordTransX->setObjectName(QString::fromUtf8("coordTransX"));
        coordTransX->setMinimumSize(QSize(0, 17));
        coordTransX->setMaximumSize(QSize(16777215, 17));
        coordTransX->setMinimum(0);
        coordTransX->setMaximum(10000);
        coordTransX->setPageStep(10);
        coordTransX->setValue(0);
        coordTransX->setSliderPosition(0);
        coordTransX->setOrientation(Qt::Horizontal);

        gridLayout4->addWidget(coordTransX, 1, 1, 1, 1);

        label_32 = new QLabel(coordTab2D);
        label_32->setObjectName(QString::fromUtf8("label_32"));
        label_32->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout4->addWidget(label_32, 2, 0, 1, 1);

        coordTransY = new QScrollBar(coordTab2D);
        coordTransY->setObjectName(QString::fromUtf8("coordTransY"));
        coordTransY->setMinimumSize(QSize(0, 17));
        coordTransY->setMaximumSize(QSize(16777215, 17));
        coordTransY->setMinimum(0);
        coordTransY->setMaximum(10000);
        coordTransY->setValue(0);
        coordTransY->setSliderPosition(0);
        coordTransY->setOrientation(Qt::Horizontal);

        gridLayout4->addWidget(coordTransY, 2, 1, 1, 1);

        label_33 = new QLabel(coordTab2D);
        label_33->setObjectName(QString::fromUtf8("label_33"));
        label_33->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout4->addWidget(label_33, 4, 0, 1, 1);

        coordScale = new QScrollBar(coordTab2D);
        coordScale->setObjectName(QString::fromUtf8("coordScale"));
        coordScale->setMinimumSize(QSize(0, 17));
        coordScale->setMaximumSize(QSize(16777215, 17));
        coordScale->setMinimum(1);
        coordScale->setMaximum(100000);
        coordScale->setValue(100);
        coordScale->setSliderPosition(100);
        coordScale->setOrientation(Qt::Horizontal);

        gridLayout4->addWidget(coordScale, 4, 1, 1, 1);

        coordTransY_spin = new QSpinBox(coordTab2D);
        coordTransY_spin->setObjectName(QString::fromUtf8("coordTransY_spin"));
        coordTransY_spin->setMinimumSize(QSize(0, 18));
        coordTransY_spin->setMaximumSize(QSize(16777215, 18));
        coordTransY_spin->setMinimum(0);
        coordTransY_spin->setMaximum(10000);

        gridLayout4->addWidget(coordTransY_spin, 2, 2, 1, 1);

        coordScale_spin = new QSpinBox(coordTab2D);
        coordScale_spin->setObjectName(QString::fromUtf8("coordScale_spin"));
        coordScale_spin->setMinimumSize(QSize(0, 18));
        coordScale_spin->setMaximumSize(QSize(16777215, 18));
        coordScale_spin->setMinimum(1);
        coordScale_spin->setMaximum(100000);
        coordScale_spin->setValue(100);

        gridLayout4->addWidget(coordScale_spin, 4, 2, 1, 1);

        label_35 = new QLabel(coordTab2D);
        label_35->setObjectName(QString::fromUtf8("label_35"));
        label_35->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout4->addWidget(label_35, 3, 0, 1, 1);

        coordRotate = new QScrollBar(coordTab2D);
        coordRotate->setObjectName(QString::fromUtf8("coordRotate"));
        sizePolicy5.setHeightForWidth(coordRotate->sizePolicy().hasHeightForWidth());
        coordRotate->setSizePolicy(sizePolicy5);
        coordRotate->setMinimumSize(QSize(100, 17));
        coordRotate->setMaximumSize(QSize(16777215, 17));
        coordRotate->setMinimum(0);
        coordRotate->setMaximum(3600);
        coordRotate->setSliderPosition(0);
        coordRotate->setOrientation(Qt::Horizontal);

        gridLayout4->addWidget(coordRotate, 3, 1, 1, 1);

        coordRotate_spin = new QSpinBox(coordTab2D);
        coordRotate_spin->setObjectName(QString::fromUtf8("coordRotate_spin"));
        coordRotate_spin->setMinimumSize(QSize(0, 18));
        coordRotate_spin->setMaximumSize(QSize(16777215, 18));
        coordRotate_spin->setMinimum(0);
        coordRotate_spin->setMaximum(3600);

        gridLayout4->addWidget(coordRotate_spin, 3, 2, 1, 1);


        verticalLayout_4->addLayout(gridLayout4);

        gridLayout5 = new QGridLayout();
        gridLayout5->setSpacing(6);
        gridLayout5->setObjectName(QString::fromUtf8("gridLayout5"));
        gridLayout5->setContentsMargins(0, 0, 0, 0);
        label_14 = new QLabel(coordTab2D);
        label_14->setObjectName(QString::fromUtf8("label_14"));

        gridLayout5->addWidget(label_14, 0, 0, 1, 1);

        coordAltitude = new QDoubleSpinBox(coordTab2D);
        coordAltitude->setObjectName(QString::fromUtf8("coordAltitude"));
        sizePolicy6.setHeightForWidth(coordAltitude->sizePolicy().hasHeightForWidth());
        coordAltitude->setSizePolicy(sizePolicy6);
        coordAltitude->setMinimumSize(QSize(0, 18));
        coordAltitude->setMaximumSize(QSize(16777215, 18));
        coordAltitude->setAlignment(Qt::AlignRight);
        coordAltitude->setDecimals(1);
        coordAltitude->setMinimum(0.100000000000000);
        coordAltitude->setMaximum(9999.000000000000000);
        coordAltitude->setValue(535.000000000000000);

        gridLayout5->addWidget(coordAltitude, 0, 1, 1, 1);

        coordAltitudeMeasured = new QLabel(coordTab2D);
        coordAltitudeMeasured->setObjectName(QString::fromUtf8("coordAltitudeMeasured"));
        coordAltitudeMeasured->setMinimumSize(QSize(90, 0));

        gridLayout5->addWidget(coordAltitudeMeasured, 0, 2, 1, 1);

        horizontalSpacer_8 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout5->addItem(horizontalSpacer_8, 0, 3, 1, 1);


        verticalLayout_4->addLayout(gridLayout5);

        gridLayout6 = new QGridLayout();
        gridLayout6->setSpacing(6);
        gridLayout6->setObjectName(QString::fromUtf8("gridLayout6"));
        gridLayout6->setContentsMargins(0, 0, 0, 0);
        label_37 = new QLabel(coordTab2D);
        label_37->setObjectName(QString::fromUtf8("label_37"));
        label_37->setMinimumSize(QSize(57, 0));

        gridLayout6->addWidget(label_37, 0, 0, 1, 1);

        coordUnit = new QDoubleSpinBox(coordTab2D);
        coordUnit->setObjectName(QString::fromUtf8("coordUnit"));
        coordUnit->setMinimumSize(QSize(0, 18));
        coordUnit->setMaximumSize(QSize(16777215, 18));
        coordUnit->setAlignment(Qt::AlignRight);
        coordUnit->setDecimals(0);
        coordUnit->setMinimum(1.000000000000000);
        coordUnit->setMaximum(9999.000000000000000);
        coordUnit->setSingleStep(1.000000000000000);
        coordUnit->setValue(100.000000000000000);

        gridLayout6->addWidget(coordUnit, 0, 1, 1, 1);

        spacerItem = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout6->addItem(spacerItem, 0, 2, 1, 1);


        verticalLayout_4->addLayout(gridLayout6);

        coordUseIntrinsic = new QCheckBox(coordTab2D);
        coordUseIntrinsic->setObjectName(QString::fromUtf8("coordUseIntrinsic"));
        coordUseIntrinsic->setMinimumSize(QSize(0, 17));
        coordUseIntrinsic->setMaximumSize(QSize(16777215, 17));

        verticalLayout_4->addWidget(coordUseIntrinsic);

        coordTab->addTab(coordTab2D, QString());

        verticalLayout_14->addWidget(coordTab);


        verticalLayout_13->addWidget(align_2);

        align = new QGroupBox(scrollAreaWidgetContents);
        align->setObjectName(QString::fromUtf8("align"));
        sizePolicy3.setHeightForWidth(align->sizePolicy().hasHeightForWidth());
        align->setSizePolicy(sizePolicy3);
        align->setMinimumSize(QSize(300, 197));
        align->setMaximumSize(QSize(16777215, 16777215));
        verticalLayout_15 = new QVBoxLayout(align);
        verticalLayout_15->setObjectName(QString::fromUtf8("verticalLayout_15"));
        verticalLayout_15->setContentsMargins(5, 5, 5, 5);
        horizontalLayout_12 = new QHBoxLayout();
        horizontalLayout_12->setObjectName(QString::fromUtf8("horizontalLayout_12"));
        gridShow = new QCheckBox(align);
        gridShow->setObjectName(QString::fromUtf8("gridShow"));
        gridShow->setMinimumSize(QSize(57, 0));

        horizontalLayout_12->addWidget(gridShow);

        gridFix = new QCheckBox(align);
        gridFix->setObjectName(QString::fromUtf8("gridFix"));
        gridFix->setMaximumSize(QSize(16777215, 25));

        horizontalLayout_12->addWidget(gridFix);

        horizontalSpacer_10 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_12->addItem(horizontalSpacer_10);


        verticalLayout_15->addLayout(horizontalLayout_12);

        gridTab = new QTabWidget(align);
        gridTab->setObjectName(QString::fromUtf8("gridTab"));
        gridTab->setEnabled(true);
        gridTab->setBaseSize(QSize(0, 0));
        gridTab->setLayoutDirection(Qt::LeftToRight);
        gridTab->setAutoFillBackground(false);
        gridTab->setTabPosition(QTabWidget::North);
        gridTab->setTabShape(QTabWidget::Rounded);
        gridTab->setElideMode(Qt::ElideNone);
        gridTab->setUsesScrollButtons(false);
        gridTab->setDocumentMode(false);
        gridTab->setTabsClosable(false);
        gridTab->setMovable(false);
        gridTab3D = new QWidget();
        gridTab3D->setObjectName(QString::fromUtf8("gridTab3D"));
        verticalLayout_11 = new QVBoxLayout(gridTab3D);
        verticalLayout_11->setObjectName(QString::fromUtf8("verticalLayout_11"));
        gridLayout_3 = new QGridLayout();
        gridLayout_3->setSpacing(4);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        gridLayout_3->setSizeConstraint(QLayout::SetDefaultConstraint);
        label_15 = new QLabel(gridTab3D);
        label_15->setObjectName(QString::fromUtf8("label_15"));
        label_15->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_3->addWidget(label_15, 0, 0, 1, 1);

        grid3DTransX = new QScrollBar(gridTab3D);
        grid3DTransX->setObjectName(QString::fromUtf8("grid3DTransX"));
        sizePolicy4.setHeightForWidth(grid3DTransX->sizePolicy().hasHeightForWidth());
        grid3DTransX->setSizePolicy(sizePolicy4);
        grid3DTransX->setMinimumSize(QSize(50, 17));
        grid3DTransX->setMaximumSize(QSize(16777215, 17));
        grid3DTransX->setMinimum(-10000);
        grid3DTransX->setMaximum(10000);
        grid3DTransX->setValue(0);
        grid3DTransX->setOrientation(Qt::Horizontal);

        gridLayout_3->addWidget(grid3DTransX, 0, 1, 1, 1);

        grid3DTransX_spin = new QSpinBox(gridTab3D);
        grid3DTransX_spin->setObjectName(QString::fromUtf8("grid3DTransX_spin"));
        grid3DTransX_spin->setMaximumSize(QSize(16777215, 22));
        grid3DTransX_spin->setMinimum(-10000);
        grid3DTransX_spin->setMaximum(10000);

        gridLayout_3->addWidget(grid3DTransX_spin, 0, 2, 1, 1);

        label_49 = new QLabel(gridTab3D);
        label_49->setObjectName(QString::fromUtf8("label_49"));
        label_49->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_3->addWidget(label_49, 1, 0, 1, 1);

        grid3DTransY = new QScrollBar(gridTab3D);
        grid3DTransY->setObjectName(QString::fromUtf8("grid3DTransY"));
        grid3DTransY->setMinimumSize(QSize(0, 17));
        grid3DTransY->setMaximumSize(QSize(16777215, 17));
        grid3DTransY->setMinimum(-10000);
        grid3DTransY->setMaximum(10000);
        grid3DTransY->setOrientation(Qt::Horizontal);

        gridLayout_3->addWidget(grid3DTransY, 1, 1, 1, 1);

        grid3DTransY_spin = new QSpinBox(gridTab3D);
        grid3DTransY_spin->setObjectName(QString::fromUtf8("grid3DTransY_spin"));
        grid3DTransY_spin->setMaximumSize(QSize(16777215, 22));
        grid3DTransY_spin->setMinimum(-10000);
        grid3DTransY_spin->setMaximum(10000);

        gridLayout_3->addWidget(grid3DTransY_spin, 1, 2, 1, 1);

        label_50 = new QLabel(gridTab3D);
        label_50->setObjectName(QString::fromUtf8("label_50"));
        label_50->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_3->addWidget(label_50, 2, 0, 1, 1);

        grid3DTransZ = new QScrollBar(gridTab3D);
        grid3DTransZ->setObjectName(QString::fromUtf8("grid3DTransZ"));
        sizePolicy4.setHeightForWidth(grid3DTransZ->sizePolicy().hasHeightForWidth());
        grid3DTransZ->setSizePolicy(sizePolicy4);
        grid3DTransZ->setMinimumSize(QSize(50, 17));
        grid3DTransZ->setMaximumSize(QSize(16777215, 17));
        grid3DTransZ->setMinimum(-10000);
        grid3DTransZ->setMaximum(10000);
        grid3DTransZ->setOrientation(Qt::Horizontal);

        gridLayout_3->addWidget(grid3DTransZ, 2, 1, 1, 1);

        grid3DTransZ_spin = new QSpinBox(gridTab3D);
        grid3DTransZ_spin->setObjectName(QString::fromUtf8("grid3DTransZ_spin"));
        grid3DTransZ_spin->setEnabled(true);
        grid3DTransZ_spin->setMaximumSize(QSize(16777215, 22));
        grid3DTransZ_spin->setMinimum(-1000);
        grid3DTransZ_spin->setMaximum(1000);

        gridLayout_3->addWidget(grid3DTransZ_spin, 2, 2, 1, 1);

        label_51 = new QLabel(gridTab3D);
        label_51->setObjectName(QString::fromUtf8("label_51"));
        label_51->setLayoutDirection(Qt::LeftToRight);
        label_51->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_3->addWidget(label_51, 3, 0, 1, 1);

        grid3DResolution = new QScrollBar(gridTab3D);
        grid3DResolution->setObjectName(QString::fromUtf8("grid3DResolution"));
        grid3DResolution->setMinimumSize(QSize(0, 17));
        grid3DResolution->setMaximumSize(QSize(16777215, 17));
        grid3DResolution->setMinimum(1);
        grid3DResolution->setMaximum(10000);
        grid3DResolution->setSingleStep(1);
        grid3DResolution->setPageStep(10);
        grid3DResolution->setValue(100);
        grid3DResolution->setSliderPosition(100);
        grid3DResolution->setOrientation(Qt::Horizontal);
        grid3DResolution->setInvertedAppearance(false);
        grid3DResolution->setInvertedControls(true);

        gridLayout_3->addWidget(grid3DResolution, 3, 1, 1, 1);

        grid3DResolution_spin = new QSpinBox(gridTab3D);
        grid3DResolution_spin->setObjectName(QString::fromUtf8("grid3DResolution_spin"));
        grid3DResolution_spin->setMaximumSize(QSize(16777215, 22));
        grid3DResolution_spin->setMinimum(1);
        grid3DResolution_spin->setMaximum(10000);
        grid3DResolution_spin->setSingleStep(10);
        grid3DResolution_spin->setValue(100);

        gridLayout_3->addWidget(grid3DResolution_spin, 3, 2, 1, 1);

        gridLayout_3->setColumnStretch(1, 1);

        verticalLayout_11->addLayout(gridLayout_3);

        gridTab->addTab(gridTab3D, QString());
        gridTab2D = new QWidget();
        gridTab2D->setObjectName(QString::fromUtf8("gridTab2D"));
        verticalLayout_10 = new QVBoxLayout(gridTab2D);
        verticalLayout_10->setObjectName(QString::fromUtf8("verticalLayout_10"));
        gridLayout_100 = new QGridLayout();
        gridLayout_100->setSpacing(6);
        gridLayout_100->setObjectName(QString::fromUtf8("gridLayout_100"));
        label_10 = new QLabel(gridTab2D);
        label_10->setObjectName(QString::fromUtf8("label_10"));
        label_10->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_100->addWidget(label_10, 0, 0, 1, 1);

        gridTransX = new QScrollBar(gridTab2D);
        gridTransX->setObjectName(QString::fromUtf8("gridTransX"));
        sizePolicy5.setHeightForWidth(gridTransX->sizePolicy().hasHeightForWidth());
        gridTransX->setSizePolicy(sizePolicy5);
        gridTransX->setMinimumSize(QSize(100, 17));
        gridTransX->setMaximumSize(QSize(16777215, 17));
        gridTransX->setMinimum(0);
        gridTransX->setMaximum(10000);
        gridTransX->setValue(0);
        gridTransX->setOrientation(Qt::Horizontal);

        gridLayout_100->addWidget(gridTransX, 0, 1, 1, 1);

        gridTransX_spin = new QSpinBox(gridTab2D);
        gridTransX_spin->setObjectName(QString::fromUtf8("gridTransX_spin"));
        gridTransX_spin->setMinimumSize(QSize(0, 17));
        gridTransX_spin->setMaximumSize(QSize(16777215, 17));
        gridTransX_spin->setMinimum(0);
        gridTransX_spin->setMaximum(10000);

        gridLayout_100->addWidget(gridTransX_spin, 0, 2, 1, 1);

        label_11 = new QLabel(gridTab2D);
        label_11->setObjectName(QString::fromUtf8("label_11"));
        label_11->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_100->addWidget(label_11, 1, 0, 1, 1);

        gridTransY = new QScrollBar(gridTab2D);
        gridTransY->setObjectName(QString::fromUtf8("gridTransY"));
        gridTransY->setMinimumSize(QSize(0, 17));
        gridTransY->setMaximumSize(QSize(16777215, 17));
        gridTransY->setMinimum(0);
        gridTransY->setMaximum(10000);
        gridTransY->setOrientation(Qt::Horizontal);

        gridLayout_100->addWidget(gridTransY, 1, 1, 1, 1);

        gridTransY_spin = new QSpinBox(gridTab2D);
        gridTransY_spin->setObjectName(QString::fromUtf8("gridTransY_spin"));
        gridTransY_spin->setMinimumSize(QSize(0, 17));
        gridTransY_spin->setMaximumSize(QSize(16777215, 17));
        gridTransY_spin->setMinimum(0);
        gridTransY_spin->setMaximum(10000);

        gridLayout_100->addWidget(gridTransY_spin, 1, 2, 1, 1);

        label_9 = new QLabel(gridTab2D);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        label_9->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_100->addWidget(label_9, 2, 0, 1, 1);

        gridRotate = new QScrollBar(gridTab2D);
        gridRotate->setObjectName(QString::fromUtf8("gridRotate"));
        sizePolicy4.setHeightForWidth(gridRotate->sizePolicy().hasHeightForWidth());
        gridRotate->setSizePolicy(sizePolicy4);
        gridRotate->setMinimumSize(QSize(50, 17));
        gridRotate->setMaximumSize(QSize(16777215, 17));
        gridRotate->setMinimum(0);
        gridRotate->setMaximum(3600);
        gridRotate->setOrientation(Qt::Horizontal);

        gridLayout_100->addWidget(gridRotate, 2, 1, 1, 1);

        gridRot_spin = new QSpinBox(gridTab2D);
        gridRot_spin->setObjectName(QString::fromUtf8("gridRot_spin"));
        gridRot_spin->setEnabled(true);
        gridRot_spin->setMinimumSize(QSize(0, 17));
        gridRot_spin->setMaximumSize(QSize(16777215, 17));
        gridRot_spin->setMinimum(0);
        gridRot_spin->setMaximum(3600);

        gridLayout_100->addWidget(gridRot_spin, 2, 2, 1, 1);

        label_12 = new QLabel(gridTab2D);
        label_12->setObjectName(QString::fromUtf8("label_12"));
        QSizePolicy sizePolicy7(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy7.setHorizontalStretch(0);
        sizePolicy7.setVerticalStretch(0);
        sizePolicy7.setHeightForWidth(label_12->sizePolicy().hasHeightForWidth());
        label_12->setSizePolicy(sizePolicy7);
        label_12->setMaximumSize(QSize(16777215, 17));
        label_12->setLayoutDirection(Qt::LeftToRight);
        label_12->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_100->addWidget(label_12, 3, 0, 1, 1);

        gridScale = new QScrollBar(gridTab2D);
        gridScale->setObjectName(QString::fromUtf8("gridScale"));
        gridScale->setMinimumSize(QSize(0, 17));
        gridScale->setMaximumSize(QSize(16777215, 17));
        gridScale->setMinimum(1);
        gridScale->setMaximum(10000);
        gridScale->setValue(100);
        gridScale->setSliderPosition(100);
        gridScale->setOrientation(Qt::Horizontal);

        gridLayout_100->addWidget(gridScale, 3, 1, 1, 1);

        gridScale_spin = new QSpinBox(gridTab2D);
        gridScale_spin->setObjectName(QString::fromUtf8("gridScale_spin"));
        gridScale_spin->setMinimumSize(QSize(0, 17));
        gridScale_spin->setMaximumSize(QSize(16777215, 17));
        gridScale_spin->setMinimum(1);
        gridScale_spin->setMaximum(10000);
        gridScale_spin->setValue(100);

        gridLayout_100->addWidget(gridScale_spin, 3, 2, 1, 1);

        gridLayout_100->setColumnStretch(1, 1);

        verticalLayout_10->addLayout(gridLayout_100);

        gridTab->addTab(gridTab2D, QString());

        verticalLayout_15->addWidget(gridTab);


        verticalLayout_13->addWidget(align);

        scrollArea->setWidget(scrollAreaWidgetContents);

        horizontalLayout_11->addWidget(scrollArea);

        tabs->addTab(calib, QString());
        rec = new QWidget();
        rec->setObjectName(QString::fromUtf8("rec"));
        QSizePolicy sizePolicy8(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy8.setHorizontalStretch(0);
        sizePolicy8.setVerticalStretch(0);
        sizePolicy8.setHeightForWidth(rec->sizePolicy().hasHeightForWidth());
        rec->setSizePolicy(sizePolicy8);
        horizontalLayout_16 = new QHBoxLayout(rec);
        horizontalLayout_16->setObjectName(QString::fromUtf8("horizontalLayout_16"));
        scrollArea_2 = new QScrollArea(rec);
        scrollArea_2->setObjectName(QString::fromUtf8("scrollArea_2"));
        scrollArea_2->setWidgetResizable(true);
        scrollAreaWidgetContents_2 = new QWidget();
        scrollAreaWidgetContents_2->setObjectName(QString::fromUtf8("scrollAreaWidgetContents_2"));
        scrollAreaWidgetContents_2->setGeometry(QRect(0, 0, 337, 874));
        verticalLayout_16 = new QVBoxLayout(scrollAreaWidgetContents_2);
        verticalLayout_16->setObjectName(QString::fromUtf8("verticalLayout_16"));
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setSpacing(2);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        hboxLayout = new QHBoxLayout();
        hboxLayout->setSpacing(6);
        hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
        hboxLayout->setContentsMargins(0, 0, 0, 0);
        performRecognition = new QCheckBox(scrollAreaWidgetContents_2);
        performRecognition->setObjectName(QString::fromUtf8("performRecognition"));
        performRecognition->setEnabled(true);
        performRecognition->setChecked(false);

        hboxLayout->addWidget(performRecognition);

        gridLayout7 = new QGridLayout();
        gridLayout7->setSpacing(6);
        gridLayout7->setObjectName(QString::fromUtf8("gridLayout7"));
        gridLayout7->setContentsMargins(0, 0, 0, 0);
        recoStep = new QSpinBox(scrollAreaWidgetContents_2);
        recoStep->setObjectName(QString::fromUtf8("recoStep"));
        sizePolicy6.setHeightForWidth(recoStep->sizePolicy().hasHeightForWidth());
        recoStep->setSizePolicy(sizePolicy6);
        recoStep->setMinimumSize(QSize(20, 0));
        recoStep->setAlignment(Qt::AlignRight);
        recoStep->setMinimum(1);
        recoStep->setMaximum(9999);
        recoStep->setValue(1);

        gridLayout7->addWidget(recoStep, 0, 2, 1, 1);

        label_27 = new QLabel(scrollAreaWidgetContents_2);
        label_27->setObjectName(QString::fromUtf8("label_27"));

        gridLayout7->addWidget(label_27, 0, 1, 1, 1);

        spacerItem1 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout7->addItem(spacerItem1, 0, 0, 1, 1);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout7->addItem(horizontalSpacer_3, 0, 4, 1, 1);


        hboxLayout->addLayout(gridLayout7);


        verticalLayout_2->addLayout(hboxLayout);

        gridLayout_2 = new QGridLayout();
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        label_26 = new QLabel(scrollAreaWidgetContents_2);
        label_26->setObjectName(QString::fromUtf8("label_26"));

        gridLayout_2->addWidget(label_26, 0, 0, 1, 1);

        recoNumberNow = new QLabel(scrollAreaWidgetContents_2);
        recoNumberNow->setObjectName(QString::fromUtf8("recoNumberNow"));
        QFont font1;
        font1.setBold(true);
        font1.setWeight(75);
        recoNumberNow->setFont(font1);
        recoNumberNow->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_2->addWidget(recoNumberNow, 0, 1, 1, 1);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_2->addItem(horizontalSpacer_2, 0, 2, 1, 1);


        verticalLayout_2->addLayout(gridLayout_2);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        recoMethod = new QComboBox(scrollAreaWidgetContents_2);
        recoMethod->setObjectName(QString::fromUtf8("recoMethod"));
        sizePolicy5.setHeightForWidth(recoMethod->sizePolicy().hasHeightForWidth());
        recoMethod->setSizePolicy(sizePolicy5);

        horizontalLayout_4->addWidget(recoMethod);

        recoStereoShow = new QPushButton(scrollAreaWidgetContents_2);
        recoStereoShow->setObjectName(QString::fromUtf8("recoStereoShow"));

        horizontalLayout_4->addWidget(recoStereoShow);


        verticalLayout_2->addLayout(horizontalLayout_4);


        verticalLayout_16->addLayout(verticalLayout_2);

        groupBox_2 = new QGroupBox(scrollAreaWidgetContents_2);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        sizePolicy8.setHeightForWidth(groupBox_2->sizePolicy().hasHeightForWidth());
        groupBox_2->setSizePolicy(sizePolicy8);
        groupBox_2->setMinimumSize(QSize(0, 44));
        groupBox_2->setMaximumSize(QSize(350, 16777215));
        groupBox_2->setCheckable(false);
        roiShow = new QCheckBox(groupBox_2);
        roiShow->setObjectName(QString::fromUtf8("roiShow"));
        roiShow->setGeometry(QRect(10, 20, 49, 18));
        roiFix = new QCheckBox(groupBox_2);
        roiFix->setObjectName(QString::fromUtf8("roiFix"));
        roiFix->setGeometry(QRect(80, 20, 36, 18));

        verticalLayout_16->addWidget(groupBox_2);

        groupBox_5 = new QGroupBox(scrollAreaWidgetContents_2);
        groupBox_5->setObjectName(QString::fromUtf8("groupBox_5"));
        groupBox_5->setMinimumSize(QSize(0, 80));
        groupBox_5->setMaximumSize(QSize(350, 16777215));
        groupBox_5->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        verticalLayout_19 = new QVBoxLayout(groupBox_5);
        verticalLayout_19->setObjectName(QString::fromUtf8("verticalLayout_19"));
        vboxLayout = new QVBoxLayout();
        vboxLayout->setSpacing(6);
        vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
        vboxLayout->setContentsMargins(0, 0, 0, 0);
        hboxLayout1 = new QHBoxLayout();
        hboxLayout1->setSpacing(6);
        hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
        hboxLayout1->setContentsMargins(0, 0, 0, 0);
        label_19 = new QLabel(groupBox_5);
        label_19->setObjectName(QString::fromUtf8("label_19"));
        QSizePolicy sizePolicy9(QSizePolicy::Fixed, QSizePolicy::Preferred);
        sizePolicy9.setHorizontalStretch(0);
        sizePolicy9.setVerticalStretch(0);
        sizePolicy9.setHeightForWidth(label_19->sizePolicy().hasHeightForWidth());
        label_19->setSizePolicy(sizePolicy9);

        hboxLayout1->addWidget(label_19);

        markerBrightness = new QScrollBar(groupBox_5);
        markerBrightness->setObjectName(QString::fromUtf8("markerBrightness"));
        sizePolicy4.setHeightForWidth(markerBrightness->sizePolicy().hasHeightForWidth());
        markerBrightness->setSizePolicy(sizePolicy4);
        markerBrightness->setMinimumSize(QSize(50, 0));
        markerBrightness->setValue(50);
        markerBrightness->setOrientation(Qt::Horizontal);

        hboxLayout1->addWidget(markerBrightness);

        hboxLayout1->setStretch(1, 1);

        vboxLayout->addLayout(hboxLayout1);

        markerIgnoreWithout = new QCheckBox(groupBox_5);
        markerIgnoreWithout->setObjectName(QString::fromUtf8("markerIgnoreWithout"));
        markerIgnoreWithout->setChecked(true);

        vboxLayout->addWidget(markerIgnoreWithout);


        verticalLayout_19->addLayout(vboxLayout);


        verticalLayout_16->addWidget(groupBox_5);

        colorBox = new QGroupBox(scrollAreaWidgetContents_2);
        colorBox->setObjectName(QString::fromUtf8("colorBox"));
        sizePolicy6.setHeightForWidth(colorBox->sizePolicy().hasHeightForWidth());
        colorBox->setSizePolicy(sizePolicy6);
        colorBox->setMinimumSize(QSize(309, 631));
        colorBox->setMaximumSize(QSize(350, 631));
        verticalLayout_9 = new QVBoxLayout(colorBox);
        verticalLayout_9->setObjectName(QString::fromUtf8("verticalLayout_9"));
        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setSpacing(2);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        verticalLayout_3->setSizeConstraint(QLayout::SetDefaultConstraint);
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        recoShowColor = new QCheckBox(colorBox);
        recoShowColor->setObjectName(QString::fromUtf8("recoShowColor"));
        sizePolicy6.setHeightForWidth(recoShowColor->sizePolicy().hasHeightForWidth());
        recoShowColor->setSizePolicy(sizePolicy6);
        recoShowColor->setChecked(true);

        horizontalLayout_2->addWidget(recoShowColor);

        recoOptimizeColor = new QPushButton(colorBox);
        recoOptimizeColor->setObjectName(QString::fromUtf8("recoOptimizeColor"));
        recoOptimizeColor->setMaximumSize(QSize(50, 16777215));

        horizontalLayout_2->addWidget(recoOptimizeColor);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_4);


        verticalLayout_3->addLayout(horizontalLayout_2);

        hboxLayout2 = new QHBoxLayout();
        hboxLayout2->setSpacing(6);
        hboxLayout2->setObjectName(QString::fromUtf8("hboxLayout2"));
        hboxLayout2->setContentsMargins(0, 0, 0, 0);
        label_28 = new QLabel(colorBox);
        label_28->setObjectName(QString::fromUtf8("label_28"));
        sizePolicy9.setHeightForWidth(label_28->sizePolicy().hasHeightForWidth());
        label_28->setSizePolicy(sizePolicy9);
        label_28->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        hboxLayout2->addWidget(label_28);

        recoColorModel = new QComboBox(colorBox);
        recoColorModel->setObjectName(QString::fromUtf8("recoColorModel"));
        sizePolicy4.setHeightForWidth(recoColorModel->sizePolicy().hasHeightForWidth());
        recoColorModel->setSizePolicy(sizePolicy4);
        recoColorModel->setMinimumSize(QSize(50, 0));
        recoColorModel->setMaximumSize(QSize(16777215, 20));

        hboxLayout2->addWidget(recoColorModel);

        recoAutoWB = new QCheckBox(colorBox);
        recoAutoWB->setObjectName(QString::fromUtf8("recoAutoWB"));

        hboxLayout2->addWidget(recoAutoWB);

        spacerItem2 = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout2->addItem(spacerItem2);


        verticalLayout_3->addLayout(hboxLayout2);

        hboxLayout3 = new QHBoxLayout();
        hboxLayout3->setSpacing(6);
        hboxLayout3->setObjectName(QString::fromUtf8("hboxLayout3"));
        hboxLayout3->setContentsMargins(0, 0, 0, 0);
        label_29 = new QLabel(colorBox);
        label_29->setObjectName(QString::fromUtf8("label_29"));
        sizePolicy9.setHeightForWidth(label_29->sizePolicy().hasHeightForWidth());
        label_29->setSizePolicy(sizePolicy9);
        label_29->setMinimumSize(QSize(32, 0));
        label_29->setLayoutDirection(Qt::LeftToRight);
        label_29->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        hboxLayout3->addWidget(label_29);

        recoColorX = new QComboBox(colorBox);
        recoColorX->setObjectName(QString::fromUtf8("recoColorX"));
        sizePolicy4.setHeightForWidth(recoColorX->sizePolicy().hasHeightForWidth());
        recoColorX->setSizePolicy(sizePolicy4);
        recoColorX->setMinimumSize(QSize(35, 0));
        recoColorX->setMaximumSize(QSize(35, 20));

        hboxLayout3->addWidget(recoColorX);

        label_30 = new QLabel(colorBox);
        label_30->setObjectName(QString::fromUtf8("label_30"));
        label_30->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        hboxLayout3->addWidget(label_30);

        recoColorY = new QComboBox(colorBox);
        recoColorY->setObjectName(QString::fromUtf8("recoColorY"));
        recoColorY->setMinimumSize(QSize(35, 0));
        recoColorY->setMaximumSize(QSize(35, 20));

        hboxLayout3->addWidget(recoColorY);

        label_31 = new QLabel(colorBox);
        label_31->setObjectName(QString::fromUtf8("label_31"));
        label_31->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        hboxLayout3->addWidget(label_31);

        recoColorZ = new QScrollBar(colorBox);
        recoColorZ->setObjectName(QString::fromUtf8("recoColorZ"));
        recoColorZ->setMinimumSize(QSize(100, 0));
        recoColorZ->setMaximum(255);
        recoColorZ->setValue(255);
        recoColorZ->setOrientation(Qt::Horizontal);

        hboxLayout3->addWidget(recoColorZ);

        hboxLayout3->setStretch(5, 1);

        verticalLayout_3->addLayout(hboxLayout3);

        hboxLayout4 = new QHBoxLayout();
        hboxLayout4->setSpacing(6);
        hboxLayout4->setObjectName(QString::fromUtf8("hboxLayout4"));
        hboxLayout4->setContentsMargins(0, 0, 0, 0);
        label_38 = new QLabel(colorBox);
        label_38->setObjectName(QString::fromUtf8("label_38"));
        sizePolicy6.setHeightForWidth(label_38->sizePolicy().hasHeightForWidth());
        label_38->setSizePolicy(sizePolicy6);

        hboxLayout4->addWidget(label_38);

        recoGreyLevel = new QScrollBar(colorBox);
        recoGreyLevel->setObjectName(QString::fromUtf8("recoGreyLevel"));
        recoGreyLevel->setValue(50);
        recoGreyLevel->setOrientation(Qt::Horizontal);

        hboxLayout4->addWidget(recoGreyLevel);

        label_39 = new QLabel(colorBox);
        label_39->setObjectName(QString::fromUtf8("label_39"));
        sizePolicy6.setHeightForWidth(label_39->sizePolicy().hasHeightForWidth());
        label_39->setSizePolicy(sizePolicy6);

        hboxLayout4->addWidget(label_39);

        recoSymbolSize = new QScrollBar(colorBox);
        recoSymbolSize->setObjectName(QString::fromUtf8("recoSymbolSize"));
        recoSymbolSize->setMinimum(1);
        recoSymbolSize->setMaximum(100);
        recoSymbolSize->setValue(10);
        recoSymbolSize->setOrientation(Qt::Horizontal);

        hboxLayout4->addWidget(recoSymbolSize);


        verticalLayout_3->addLayout(hboxLayout4);

        colorPlot = new ColorPlot(colorBox);
        colorPlot->setObjectName(QString::fromUtf8("colorPlot"));
        sizePolicy5.setHeightForWidth(colorPlot->sizePolicy().hasHeightForWidth());
        colorPlot->setSizePolicy(sizePolicy5);
        colorPlot->setMinimumSize(QSize(288, 350));
        colorPlot->setMaximumSize(QSize(340, 350));
        colorPlot->setFrameShape(QFrame::StyledPanel);
        colorPlot->setFrameShadow(QFrame::Raised);

        verticalLayout_3->addWidget(colorPlot);

        groupBox_4 = new QGroupBox(colorBox);
        groupBox_4->setObjectName(QString::fromUtf8("groupBox_4"));
        QSizePolicy sizePolicy10(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy10.setHorizontalStretch(0);
        sizePolicy10.setVerticalStretch(0);
        sizePolicy10.setHeightForWidth(groupBox_4->sizePolicy().hasHeightForWidth());
        groupBox_4->setSizePolicy(sizePolicy10);
        groupBox_4->setMinimumSize(QSize(250, 110));
        groupBox_4->setMaximumSize(QSize(350, 110));
        layoutWidget11 = new QWidget(groupBox_4);
        layoutWidget11->setObjectName(QString::fromUtf8("layoutWidget11"));
        layoutWidget11->setGeometry(QRect(10, 20, 271, 95));
        vboxLayout1 = new QVBoxLayout(layoutWidget11);
        vboxLayout1->setSpacing(6);
        vboxLayout1->setObjectName(QString::fromUtf8("vboxLayout1"));
        vboxLayout1->setContentsMargins(0, 0, 0, 0);
        gridLayout8 = new QGridLayout();
        gridLayout8->setSpacing(6);
        gridLayout8->setObjectName(QString::fromUtf8("gridLayout8"));
        gridLayout8->setContentsMargins(0, 0, 0, 0);
        label_44 = new QLabel(layoutWidget11);
        label_44->setObjectName(QString::fromUtf8("label_44"));
        sizePolicy9.setHeightForWidth(label_44->sizePolicy().hasHeightForWidth());
        label_44->setSizePolicy(sizePolicy9);

        gridLayout8->addWidget(label_44, 1, 3, 1, 1);

        label_40 = new QLabel(layoutWidget11);
        label_40->setObjectName(QString::fromUtf8("label_40"));
        sizePolicy9.setHeightForWidth(label_40->sizePolicy().hasHeightForWidth());
        label_40->setSizePolicy(sizePolicy9);

        gridLayout8->addWidget(label_40, 0, 1, 1, 1);

        mapX = new QScrollBar(layoutWidget11);
        mapX->setObjectName(QString::fromUtf8("mapX"));
        mapX->setMaximum(718);
        mapX->setOrientation(Qt::Horizontal);

        gridLayout8->addWidget(mapX, 0, 2, 1, 1);

        mapH = new QScrollBar(layoutWidget11);
        mapH->setObjectName(QString::fromUtf8("mapH"));
        mapH->setMaximum(395);
        mapH->setOrientation(Qt::Horizontal);

        gridLayout8->addWidget(mapH, 1, 4, 1, 1);

        mapW = new QScrollBar(layoutWidget11);
        mapW->setObjectName(QString::fromUtf8("mapW"));
        mapW->setMaximum(395);
        mapW->setOrientation(Qt::Horizontal);

        gridLayout8->addWidget(mapW, 1, 2, 1, 1);

        mapY = new QScrollBar(layoutWidget11);
        mapY->setObjectName(QString::fromUtf8("mapY"));
        mapY->setMaximum(718);
        mapY->setOrientation(Qt::Horizontal);

        gridLayout8->addWidget(mapY, 0, 4, 1, 1);

        label_43 = new QLabel(layoutWidget11);
        label_43->setObjectName(QString::fromUtf8("label_43"));
        sizePolicy9.setHeightForWidth(label_43->sizePolicy().hasHeightForWidth());
        label_43->setSizePolicy(sizePolicy9);

        gridLayout8->addWidget(label_43, 1, 1, 1, 1);

        label_42 = new QLabel(layoutWidget11);
        label_42->setObjectName(QString::fromUtf8("label_42"));
        sizePolicy9.setHeightForWidth(label_42->sizePolicy().hasHeightForWidth());
        label_42->setSizePolicy(sizePolicy9);

        gridLayout8->addWidget(label_42, 0, 3, 1, 1);

        mapNr = new QSpinBox(layoutWidget11);
        mapNr->setObjectName(QString::fromUtf8("mapNr"));
        sizePolicy6.setHeightForWidth(mapNr->sizePolicy().hasHeightForWidth());
        mapNr->setSizePolicy(sizePolicy6);
        mapNr->setAlignment(Qt::AlignRight);
        mapNr->setMaximum(0);

        gridLayout8->addWidget(mapNr, 0, 0, 1, 1);

        mapColor = new QCheckBox(layoutWidget11);
        mapColor->setObjectName(QString::fromUtf8("mapColor"));
        sizePolicy6.setHeightForWidth(mapColor->sizePolicy().hasHeightForWidth());
        mapColor->setSizePolicy(sizePolicy6);
        mapColor->setChecked(true);

        gridLayout8->addWidget(mapColor, 1, 0, 1, 1);


        vboxLayout1->addLayout(gridLayout8);

        hboxLayout5 = new QHBoxLayout();
        hboxLayout5->setSpacing(6);
        hboxLayout5->setObjectName(QString::fromUtf8("hboxLayout5"));
        hboxLayout5->setContentsMargins(0, 0, 0, 0);
        label_45 = new QLabel(layoutWidget11);
        label_45->setObjectName(QString::fromUtf8("label_45"));
        sizePolicy9.setHeightForWidth(label_45->sizePolicy().hasHeightForWidth());
        label_45->setSizePolicy(sizePolicy9);

        hboxLayout5->addWidget(label_45);

        mapHeight = new QDoubleSpinBox(layoutWidget11);
        mapHeight->setObjectName(QString::fromUtf8("mapHeight"));
        sizePolicy6.setHeightForWidth(mapHeight->sizePolicy().hasHeightForWidth());
        mapHeight->setSizePolicy(sizePolicy6);
        mapHeight->setAlignment(Qt::AlignRight);
        mapHeight->setDecimals(1);
        mapHeight->setMinimum(50.000000000000000);
        mapHeight->setMaximum(250.000000000000000);
        mapHeight->setValue(180.000000000000000);

        hboxLayout5->addWidget(mapHeight);

        mapAdd = new QPushButton(layoutWidget11);
        mapAdd->setObjectName(QString::fromUtf8("mapAdd"));
        sizePolicy4.setHeightForWidth(mapAdd->sizePolicy().hasHeightForWidth());
        mapAdd->setSizePolicy(sizePolicy4);
        mapAdd->setMaximumSize(QSize(40, 16777215));

        hboxLayout5->addWidget(mapAdd);

        mapDel = new QPushButton(layoutWidget11);
        mapDel->setObjectName(QString::fromUtf8("mapDel"));
        mapDel->setMaximumSize(QSize(40, 16777215));

        hboxLayout5->addWidget(mapDel);

        mapColorRange = new QPushButton(layoutWidget11);
        mapColorRange->setObjectName(QString::fromUtf8("mapColorRange"));
        mapColorRange->setMaximumSize(QSize(80, 16777215));

        hboxLayout5->addWidget(mapColorRange);


        vboxLayout1->addLayout(hboxLayout5);


        verticalLayout_3->addWidget(groupBox_4);

        hboxLayout6 = new QHBoxLayout();
        hboxLayout6->setSpacing(0);
        hboxLayout6->setObjectName(QString::fromUtf8("hboxLayout6"));
        hboxLayout6->setContentsMargins(0, 0, 0, 0);
        label_46 = new QLabel(colorBox);
        label_46->setObjectName(QString::fromUtf8("label_46"));
        sizePolicy9.setHeightForWidth(label_46->sizePolicy().hasHeightForWidth());
        label_46->setSizePolicy(sizePolicy9);

        hboxLayout6->addWidget(label_46);

        mapDefaultHeight = new QDoubleSpinBox(colorBox);
        mapDefaultHeight->setObjectName(QString::fromUtf8("mapDefaultHeight"));
        sizePolicy6.setHeightForWidth(mapDefaultHeight->sizePolicy().hasHeightForWidth());
        mapDefaultHeight->setSizePolicy(sizePolicy6);
        mapDefaultHeight->setAlignment(Qt::AlignRight);
        mapDefaultHeight->setDecimals(1);
        mapDefaultHeight->setMinimum(0.000000000000000);
        mapDefaultHeight->setMaximum(250.000000000000000);
        mapDefaultHeight->setValue(180.000000000000000);

        hboxLayout6->addWidget(mapDefaultHeight);

        mapDistribution = new QPushButton(colorBox);
        mapDistribution->setObjectName(QString::fromUtf8("mapDistribution"));
        sizePolicy6.setHeightForWidth(mapDistribution->sizePolicy().hasHeightForWidth());
        mapDistribution->setSizePolicy(sizePolicy6);
        mapDistribution->setMaximumSize(QSize(40, 16777215));

        hboxLayout6->addWidget(mapDistribution);

        mapResetHeight = new QPushButton(colorBox);
        mapResetHeight->setObjectName(QString::fromUtf8("mapResetHeight"));
        sizePolicy6.setHeightForWidth(mapResetHeight->sizePolicy().hasHeightForWidth());
        mapResetHeight->setSizePolicy(sizePolicy6);

        hboxLayout6->addWidget(mapResetHeight);

        mapResetPos = new QPushButton(colorBox);
        mapResetPos->setObjectName(QString::fromUtf8("mapResetPos"));
        sizePolicy6.setHeightForWidth(mapResetPos->sizePolicy().hasHeightForWidth());
        mapResetPos->setSizePolicy(sizePolicy6);

        hboxLayout6->addWidget(mapResetPos);


        verticalLayout_3->addLayout(hboxLayout6);


        verticalLayout_9->addLayout(verticalLayout_3);


        verticalLayout_16->addWidget(colorBox);

        verticalSpacer_3 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_16->addItem(verticalSpacer_3);

        scrollArea_2->setWidget(scrollAreaWidgetContents_2);

        horizontalLayout_16->addWidget(scrollArea_2);

        tabs->addTab(rec, QString());
        track = new QWidget();
        track->setObjectName(QString::fromUtf8("track"));
        horizontalLayout_17 = new QHBoxLayout(track);
        horizontalLayout_17->setObjectName(QString::fromUtf8("horizontalLayout_17"));
        scrollArea_3 = new QScrollArea(track);
        scrollArea_3->setObjectName(QString::fromUtf8("scrollArea_3"));
        scrollArea_3->setWidgetResizable(true);
        scrollAreaWidgetContents_3 = new QWidget();
        scrollAreaWidgetContents_3->setObjectName(QString::fromUtf8("scrollAreaWidgetContents_3"));
        scrollAreaWidgetContents_3->setGeometry(QRect(0, 0, 337, 843));
        verticalLayout_17 = new QVBoxLayout(scrollAreaWidgetContents_3);
        verticalLayout_17->setObjectName(QString::fromUtf8("verticalLayout_17"));
        hboxLayout7 = new QHBoxLayout();
        hboxLayout7->setSpacing(4);
        hboxLayout7->setObjectName(QString::fromUtf8("hboxLayout7"));
        hboxLayout7->setContentsMargins(0, 0, 0, 0);
        trackOnlineCalc = new QCheckBox(scrollAreaWidgetContents_3);
        trackOnlineCalc->setObjectName(QString::fromUtf8("trackOnlineCalc"));
        trackOnlineCalc->setMaximumSize(QSize(120, 16777215));
        trackOnlineCalc->setChecked(false);

        hboxLayout7->addWidget(trackOnlineCalc);

        trackRepeat = new QCheckBox(scrollAreaWidgetContents_3);
        trackRepeat->setObjectName(QString::fromUtf8("trackRepeat"));
        trackRepeat->setMaximumSize(QSize(130, 16777215));
        trackRepeat->setChecked(true);

        hboxLayout7->addWidget(trackRepeat);

        trackRepeatQual = new QSpinBox(scrollAreaWidgetContents_3);
        trackRepeatQual->setObjectName(QString::fromUtf8("trackRepeatQual"));
        sizePolicy6.setHeightForWidth(trackRepeatQual->sizePolicy().hasHeightForWidth());
        trackRepeatQual->setSizePolicy(sizePolicy6);
        trackRepeatQual->setMinimumSize(QSize(40, 0));
        trackRepeatQual->setLayoutDirection(Qt::LeftToRight);
        trackRepeatQual->setAlignment(Qt::AlignRight);
        trackRepeatQual->setMaximum(100);
        trackRepeatQual->setValue(50);

        hboxLayout7->addWidget(trackRepeatQual);

        spacerItem3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout7->addItem(spacerItem3);


        verticalLayout_17->addLayout(hboxLayout7);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        trackExtrapolation = new QCheckBox(scrollAreaWidgetContents_3);
        trackExtrapolation->setObjectName(QString::fromUtf8("trackExtrapolation"));
        trackExtrapolation->setMaximumSize(QSize(150, 16777215));
        trackExtrapolation->setChecked(true);

        horizontalLayout_5->addWidget(trackExtrapolation);

        trackMerge = new QCheckBox(scrollAreaWidgetContents_3);
        trackMerge->setObjectName(QString::fromUtf8("trackMerge"));
        trackMerge->setMaximumSize(QSize(75, 16777215));

        horizontalLayout_5->addWidget(trackMerge);

        trackOnlyVisible = new QCheckBox(scrollAreaWidgetContents_3);
        trackOnlyVisible->setObjectName(QString::fromUtf8("trackOnlyVisible"));
        trackOnlyVisible->setMaximumSize(QSize(75, 16777215));
        trackOnlyVisible->setChecked(true);

        horizontalLayout_5->addWidget(trackOnlyVisible);

        horizontalSpacer_15 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_5->addItem(horizontalSpacer_15);


        verticalLayout_17->addLayout(horizontalLayout_5);

        gridLayout9 = new QGridLayout();
        gridLayout9->setSpacing(2);
        gridLayout9->setObjectName(QString::fromUtf8("gridLayout9"));
        gridLayout9->setContentsMargins(0, 0, 0, 0);
        trackNumberNow = new QLabel(scrollAreaWidgetContents_3);
        trackNumberNow->setObjectName(QString::fromUtf8("trackNumberNow"));
        trackNumberNow->setFont(font1);
        trackNumberNow->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout9->addWidget(trackNumberNow, 3, 1, 1, 1);

        label_22 = new QLabel(scrollAreaWidgetContents_3);
        label_22->setObjectName(QString::fromUtf8("label_22"));
        sizePolicy4.setHeightForWidth(label_22->sizePolicy().hasHeightForWidth());
        label_22->setSizePolicy(sizePolicy4);
        label_22->setMinimumSize(QSize(0, 16));
        label_22->setMaximumSize(QSize(300, 16777215));

        gridLayout9->addWidget(label_22, 1, 0, 1, 1);

        label_23 = new QLabel(scrollAreaWidgetContents_3);
        label_23->setObjectName(QString::fromUtf8("label_23"));
        sizePolicy4.setHeightForWidth(label_23->sizePolicy().hasHeightForWidth());
        label_23->setSizePolicy(sizePolicy4);
        label_23->setMinimumSize(QSize(0, 16));
        label_23->setMaximumSize(QSize(300, 16777215));

        gridLayout9->addWidget(label_23, 3, 0, 1, 1);

        trackNumberAll = new QLabel(scrollAreaWidgetContents_3);
        trackNumberAll->setObjectName(QString::fromUtf8("trackNumberAll"));
        trackNumberAll->setFont(font1);
        trackNumberAll->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout9->addWidget(trackNumberAll, 1, 1, 1, 1);

        spacerItem4 = new QSpacerItem(40, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout9->addItem(spacerItem4, 1, 2, 1, 1);

        label_65 = new QLabel(scrollAreaWidgetContents_3);
        label_65->setObjectName(QString::fromUtf8("label_65"));
        sizePolicy4.setHeightForWidth(label_65->sizePolicy().hasHeightForWidth());
        label_65->setSizePolicy(sizePolicy4);
        label_65->setMinimumSize(QSize(0, 16));
        label_65->setMaximumSize(QSize(300, 16777215));

        gridLayout9->addWidget(label_65, 2, 0, 1, 1);

        trackNumberVisible = new QLabel(scrollAreaWidgetContents_3);
        trackNumberVisible->setObjectName(QString::fromUtf8("trackNumberVisible"));
        trackNumberVisible->setFont(font1);
        trackNumberVisible->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout9->addWidget(trackNumberVisible, 2, 1, 1, 1);


        verticalLayout_17->addLayout(gridLayout9);

        gridLayout10 = new QGridLayout();
        gridLayout10->setSpacing(2);
        gridLayout10->setObjectName(QString::fromUtf8("gridLayout10"));
        gridLayout10->setContentsMargins(0, 0, 0, 0);
        trackExport = new QPushButton(scrollAreaWidgetContents_3);
        trackExport->setObjectName(QString::fromUtf8("trackExport"));
        trackExport->setMaximumSize(QSize(16777215, 16777215));

        gridLayout10->addWidget(trackExport, 1, 0, 1, 1);

        trackCalc = new QPushButton(scrollAreaWidgetContents_3);
        trackCalc->setObjectName(QString::fromUtf8("trackCalc"));
        trackCalc->setMaximumSize(QSize(16777215, 16777215));

        gridLayout10->addWidget(trackCalc, 0, 0, 1, 1);

        trackImport = new QPushButton(scrollAreaWidgetContents_3);
        trackImport->setObjectName(QString::fromUtf8("trackImport"));
        trackImport->setMaximumSize(QSize(16777215, 16777215));

        gridLayout10->addWidget(trackImport, 1, 1, 1, 1);

        trackReset = new QPushButton(scrollAreaWidgetContents_3);
        trackReset->setObjectName(QString::fromUtf8("trackReset"));
        trackReset->setMaximumSize(QSize(16777215, 16777215));

        gridLayout10->addWidget(trackReset, 0, 1, 1, 1);


        verticalLayout_17->addLayout(gridLayout10);

        groupBox_8 = new QGroupBox(scrollAreaWidgetContents_3);
        groupBox_8->setObjectName(QString::fromUtf8("groupBox_8"));
        groupBox_8->setMinimumSize(QSize(0, 44));
        trackRoiShow = new QCheckBox(groupBox_8);
        trackRoiShow->setObjectName(QString::fromUtf8("trackRoiShow"));
        trackRoiShow->setGeometry(QRect(10, 20, 49, 18));
        trackRoiFix = new QCheckBox(groupBox_8);
        trackRoiFix->setObjectName(QString::fromUtf8("trackRoiFix"));
        trackRoiFix->setGeometry(QRect(70, 20, 36, 18));

        verticalLayout_17->addWidget(groupBox_8);

        groupBox_7 = new QGroupBox(scrollAreaWidgetContents_3);
        groupBox_7->setObjectName(QString::fromUtf8("groupBox_7"));
        sizePolicy10.setHeightForWidth(groupBox_7->sizePolicy().hasHeightForWidth());
        groupBox_7->setSizePolicy(sizePolicy10);
        groupBox_7->setMaximumSize(QSize(324, 16777215));
        verticalLayout_6 = new QVBoxLayout(groupBox_7);
        verticalLayout_6->setSpacing(2);
        verticalLayout_6->setObjectName(QString::fromUtf8("verticalLayout_6"));
        verticalLayout_6->setContentsMargins(2, 6, 2, 6);
        gridLayout_4 = new QGridLayout();
        gridLayout_4->setSpacing(4);
        gridLayout_4->setObjectName(QString::fromUtf8("gridLayout_4"));
        trackMissingFrames = new QCheckBox(groupBox_7);
        trackMissingFrames->setObjectName(QString::fromUtf8("trackMissingFrames"));
        sizePolicy5.setHeightForWidth(trackMissingFrames->sizePolicy().hasHeightForWidth());
        trackMissingFrames->setSizePolicy(sizePolicy5);
        trackMissingFrames->setMaximumSize(QSize(100, 16777215));
        trackMissingFrames->setChecked(true);

        gridLayout_4->addWidget(trackMissingFrames, 0, 0, 1, 1);

        trackRecalcHeight = new QCheckBox(groupBox_7);
        trackRecalcHeight->setObjectName(QString::fromUtf8("trackRecalcHeight"));
        sizePolicy5.setHeightForWidth(trackRecalcHeight->sizePolicy().hasHeightForWidth());
        trackRecalcHeight->setSizePolicy(sizePolicy5);
        trackRecalcHeight->setMaximumSize(QSize(100, 16777215));
        trackRecalcHeight->setChecked(true);

        gridLayout_4->addWidget(trackRecalcHeight, 0, 1, 1, 1);

        trackAlternateHeight = new QCheckBox(groupBox_7);
        trackAlternateHeight->setObjectName(QString::fromUtf8("trackAlternateHeight"));
        sizePolicy5.setHeightForWidth(trackAlternateHeight->sizePolicy().hasHeightForWidth());
        trackAlternateHeight->setSizePolicy(sizePolicy5);
        trackAlternateHeight->setMaximumSize(QSize(100, 16777215));

        gridLayout_4->addWidget(trackAlternateHeight, 0, 2, 1, 1);

        exportElimTp = new QCheckBox(groupBox_7);
        exportElimTp->setObjectName(QString::fromUtf8("exportElimTp"));
        sizePolicy5.setHeightForWidth(exportElimTp->sizePolicy().hasHeightForWidth());
        exportElimTp->setSizePolicy(sizePolicy5);
        exportElimTp->setMaximumSize(QSize(120, 16777215));
        exportElimTp->setChecked(false);

        gridLayout_4->addWidget(exportElimTp, 1, 0, 1, 1);

        exportElimTrj = new QCheckBox(groupBox_7);
        exportElimTrj->setObjectName(QString::fromUtf8("exportElimTrj"));
        sizePolicy5.setHeightForWidth(exportElimTrj->sizePolicy().hasHeightForWidth());
        exportElimTrj->setSizePolicy(sizePolicy5);
        exportElimTrj->setMaximumSize(QSize(120, 16777215));
        exportElimTrj->setChecked(false);

        gridLayout_4->addWidget(exportElimTrj, 1, 1, 1, 1);

        exportSmooth = new QCheckBox(groupBox_7);
        exportSmooth->setObjectName(QString::fromUtf8("exportSmooth"));
        sizePolicy5.setHeightForWidth(exportSmooth->sizePolicy().hasHeightForWidth());
        exportSmooth->setSizePolicy(sizePolicy5);
        exportSmooth->setMaximumSize(QSize(120, 16777215));
        exportSmooth->setChecked(true);

        gridLayout_4->addWidget(exportSmooth, 1, 2, 1, 1);

        exportViewDir = new QCheckBox(groupBox_7);
        exportViewDir->setObjectName(QString::fromUtf8("exportViewDir"));
        sizePolicy5.setHeightForWidth(exportViewDir->sizePolicy().hasHeightForWidth());
        exportViewDir->setSizePolicy(sizePolicy5);
        exportViewDir->setMaximumSize(QSize(120, 16777215));

        gridLayout_4->addWidget(exportViewDir, 2, 0, 1, 1);

        exportAngleOfView = new QCheckBox(groupBox_7);
        exportAngleOfView->setObjectName(QString::fromUtf8("exportAngleOfView"));
        sizePolicy5.setHeightForWidth(exportAngleOfView->sizePolicy().hasHeightForWidth());
        exportAngleOfView->setSizePolicy(sizePolicy5);
        exportAngleOfView->setMaximumSize(QSize(120, 16777215));

        gridLayout_4->addWidget(exportAngleOfView, 2, 1, 1, 1);

        exportUseM = new QCheckBox(groupBox_7);
        exportUseM->setObjectName(QString::fromUtf8("exportUseM"));
        sizePolicy5.setHeightForWidth(exportUseM->sizePolicy().hasHeightForWidth());
        exportUseM->setSizePolicy(sizePolicy5);
        exportUseM->setMaximumSize(QSize(120, 16777215));

        gridLayout_4->addWidget(exportUseM, 2, 2, 1, 1);

        exportComment = new QCheckBox(groupBox_7);
        exportComment->setObjectName(QString::fromUtf8("exportComment"));
        sizePolicy5.setHeightForWidth(exportComment->sizePolicy().hasHeightForWidth());
        exportComment->setSizePolicy(sizePolicy5);
        exportComment->setMaximumSize(QSize(120, 16777215));

        gridLayout_4->addWidget(exportComment, 3, 0, 1, 1);

        exportMarkerID = new QCheckBox(groupBox_7);
        exportMarkerID->setObjectName(QString::fromUtf8("exportMarkerID"));

        gridLayout_4->addWidget(exportMarkerID, 3, 1, 1, 1);

        gridLayout_4->setColumnStretch(0, 1);
        gridLayout_4->setColumnStretch(1, 1);
        gridLayout_4->setColumnStretch(2, 1);

        verticalLayout_6->addLayout(gridLayout_4);


        verticalLayout_17->addWidget(groupBox_7);

        horizontalLayout_9 = new QHBoxLayout();
        horizontalLayout_9->setObjectName(QString::fromUtf8("horizontalLayout_9"));
        trackTest = new QPushButton(scrollAreaWidgetContents_3);
        trackTest->setObjectName(QString::fromUtf8("trackTest"));
        trackTest->setMaximumSize(QSize(60, 16777215));

        horizontalLayout_9->addWidget(trackTest);

        testEqual = new QCheckBox(scrollAreaWidgetContents_3);
        testEqual->setObjectName(QString::fromUtf8("testEqual"));
        testEqual->setMaximumSize(QSize(60, 16777215));
        testEqual->setChecked(true);

        horizontalLayout_9->addWidget(testEqual);

        testVelocity = new QCheckBox(scrollAreaWidgetContents_3);
        testVelocity->setObjectName(QString::fromUtf8("testVelocity"));
        testVelocity->setMaximumSize(QSize(60, 16777215));
        testVelocity->setChecked(true);

        horizontalLayout_9->addWidget(testVelocity);

        testInside = new QCheckBox(scrollAreaWidgetContents_3);
        testInside->setObjectName(QString::fromUtf8("testInside"));
        testInside->setMaximumSize(QSize(60, 16777215));
        testInside->setChecked(true);

        horizontalLayout_9->addWidget(testInside);

        testLength = new QCheckBox(scrollAreaWidgetContents_3);
        testLength->setObjectName(QString::fromUtf8("testLength"));
        testLength->setMaximumSize(QSize(60, 16777215));
        testLength->setChecked(true);

        horizontalLayout_9->addWidget(testLength);

        horizontalSpacer_14 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_9->addItem(horizontalSpacer_14);

        horizontalLayout_9->setStretch(0, 1);

        verticalLayout_17->addLayout(horizontalLayout_9);

        groupBox_6 = new QGroupBox(scrollAreaWidgetContents_3);
        groupBox_6->setObjectName(QString::fromUtf8("groupBox_6"));
        sizePolicy8.setHeightForWidth(groupBox_6->sizePolicy().hasHeightForWidth());
        groupBox_6->setSizePolicy(sizePolicy8);
        groupBox_6->setMinimumSize(QSize(0, 120));
        groupBox_6->setMaximumSize(QSize(324, 16777215));
        verticalLayout_20 = new QVBoxLayout(groupBox_6);
        verticalLayout_20->setObjectName(QString::fromUtf8("verticalLayout_20"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        gridLayout_17 = new QGridLayout();
        gridLayout_17->setObjectName(QString::fromUtf8("gridLayout_17"));
        gridLayout_17->setVerticalSpacing(0);
        label_52 = new QLabel(groupBox_6);
        label_52->setObjectName(QString::fromUtf8("label_52"));
        label_52->setMaximumSize(QSize(100, 16777215));

        gridLayout_17->addWidget(label_52, 0, 0, 1, 1);

        trackRegionScale = new QScrollBar(groupBox_6);
        trackRegionScale->setObjectName(QString::fromUtf8("trackRegionScale"));
        trackRegionScale->setMinimum(1);
        trackRegionScale->setValue(16);
        trackRegionScale->setOrientation(Qt::Horizontal);

        gridLayout_17->addWidget(trackRegionScale, 0, 1, 1, 1);

        label_53 = new QLabel(groupBox_6);
        label_53->setObjectName(QString::fromUtf8("label_53"));
        label_53->setMaximumSize(QSize(100, 16777215));

        gridLayout_17->addWidget(label_53, 1, 0, 1, 1);

        trackRegionLevels = new QScrollBar(groupBox_6);
        trackRegionLevels->setObjectName(QString::fromUtf8("trackRegionLevels"));
        trackRegionLevels->setMinimum(0);
        trackRegionLevels->setMaximum(8);
        trackRegionLevels->setValue(3);
        trackRegionLevels->setOrientation(Qt::Horizontal);

        gridLayout_17->addWidget(trackRegionLevels, 1, 1, 1, 1);

        trackErrorExponent = new QScrollBar(groupBox_6);
        trackErrorExponent->setObjectName(QString::fromUtf8("trackErrorExponent"));
        trackErrorExponent->setMinimum(-10);
        trackErrorExponent->setMaximum(10);
        trackErrorExponent->setPageStep(3);
        trackErrorExponent->setOrientation(Qt::Horizontal);

        gridLayout_17->addWidget(trackErrorExponent, 2, 1, 1, 1);

        label_66 = new QLabel(groupBox_6);
        label_66->setObjectName(QString::fromUtf8("label_66"));
        label_66->setMaximumSize(QSize(100, 16777215));

        gridLayout_17->addWidget(label_66, 2, 0, 1, 1);

        gridLayout_17->setColumnStretch(1, 1);

        verticalLayout->addLayout(gridLayout_17);

        trackShowSearchSize = new QCheckBox(groupBox_6);
        trackShowSearchSize->setObjectName(QString::fromUtf8("trackShowSearchSize"));
        trackShowSearchSize->setMaximumSize(QSize(300, 16777215));

        verticalLayout->addWidget(trackShowSearchSize);


        verticalLayout_20->addLayout(verticalLayout);


        verticalLayout_17->addWidget(groupBox_6);

        groupBox_3 = new QGroupBox(scrollAreaWidgetContents_3);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        groupBox_3->setMinimumSize(QSize(0, 325));
        groupBox_3->setMaximumSize(QSize(324, 16777215));
        verticalLayout_21 = new QVBoxLayout(groupBox_3);
        verticalLayout_21->setSpacing(6);
        verticalLayout_21->setObjectName(QString::fromUtf8("verticalLayout_21"));
        verticalLayout_21->setContentsMargins(2, 2, 2, 2);
        vboxLayout2 = new QVBoxLayout();
        vboxLayout2->setSpacing(4);
        vboxLayout2->setObjectName(QString::fromUtf8("vboxLayout2"));
        vboxLayout2->setContentsMargins(4, 4, 4, 4);
        hboxLayout8 = new QHBoxLayout();
        hboxLayout8->setSpacing(2);
        hboxLayout8->setObjectName(QString::fromUtf8("hboxLayout8"));
        hboxLayout8->setContentsMargins(0, 0, 0, 0);
        trackShow = new QCheckBox(groupBox_3);
        trackShow->setObjectName(QString::fromUtf8("trackShow"));
        trackShow->setChecked(true);

        hboxLayout8->addWidget(trackShow);

        trackFix = new QCheckBox(groupBox_3);
        trackFix->setObjectName(QString::fromUtf8("trackFix"));

        hboxLayout8->addWidget(trackFix);

        spacerItem5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout8->addItem(spacerItem5);


        vboxLayout2->addLayout(hboxLayout8);

        trackShowOnlyVisible = new QCheckBox(groupBox_3);
        trackShowOnlyVisible->setObjectName(QString::fromUtf8("trackShowOnlyVisible"));
        trackShowOnlyVisible->setMaximumSize(QSize(300, 16777215));

        vboxLayout2->addWidget(trackShowOnlyVisible);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        trackShowOnly = new QCheckBox(groupBox_3);
        trackShowOnly->setObjectName(QString::fromUtf8("trackShowOnly"));
        trackShowOnly->setMaximumSize(QSize(160, 16777215));

        horizontalLayout->addWidget(trackShowOnly);

        trackShowOnlyNr = new QSpinBox(groupBox_3);
        trackShowOnlyNr->setObjectName(QString::fromUtf8("trackShowOnlyNr"));
        trackShowOnlyNr->setMinimum(1);

        horizontalLayout->addWidget(trackShowOnlyNr);

        trackGotoNr = new QPushButton(groupBox_3);
        trackGotoNr->setObjectName(QString::fromUtf8("trackGotoNr"));
        trackGotoNr->setMaximumSize(QSize(40, 16777215));

        horizontalLayout->addWidget(trackGotoNr);

        trackGotoStartNr = new QPushButton(groupBox_3);
        trackGotoStartNr->setObjectName(QString::fromUtf8("trackGotoStartNr"));
        trackGotoStartNr->setMaximumSize(QSize(40, 16777215));

        horizontalLayout->addWidget(trackGotoStartNr);

        trackGotoEndNr = new QPushButton(groupBox_3);
        trackGotoEndNr->setObjectName(QString::fromUtf8("trackGotoEndNr"));
        trackGotoEndNr->setMaximumSize(QSize(40, 16777215));

        horizontalLayout->addWidget(trackGotoEndNr);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);


        vboxLayout2->addLayout(horizontalLayout);

        horizontalLayout_20 = new QHBoxLayout();
        horizontalLayout_20->setObjectName(QString::fromUtf8("horizontalLayout_20"));
        trackShowOnlyList = new QCheckBox(groupBox_3);
        trackShowOnlyList->setObjectName(QString::fromUtf8("trackShowOnlyList"));

        horizontalLayout_20->addWidget(trackShowOnlyList);

        trackShowOnlyNrList = new QLineEdit(groupBox_3);
        trackShowOnlyNrList->setObjectName(QString::fromUtf8("trackShowOnlyNrList"));
        trackShowOnlyNrList->setEnabled(false);

        horizontalLayout_20->addWidget(trackShowOnlyNrList);

        trackShowOnlyListButton = new QPushButton(groupBox_3);
        trackShowOnlyListButton->setObjectName(QString::fromUtf8("trackShowOnlyListButton"));
        trackShowOnlyListButton->setEnabled(false);
        trackShowOnlyListButton->setMaximumSize(QSize(40, 16777215));

        horizontalLayout_20->addWidget(trackShowOnlyListButton);


        vboxLayout2->addLayout(horizontalLayout_20);

        spacerItem6 = new QSpacerItem(275, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        vboxLayout2->addItem(spacerItem6);

        gridLayout11 = new QGridLayout();
        gridLayout11->setObjectName(QString::fromUtf8("gridLayout11"));
        gridLayout11->setHorizontalSpacing(2);
        gridLayout11->setVerticalSpacing(0);
        gridLayout11->setContentsMargins(0, 0, 0, 0);
        trackShowPath = new QCheckBox(groupBox_3);
        trackShowPath->setObjectName(QString::fromUtf8("trackShowPath"));
        trackShowPath->setMaximumSize(QSize(170, 16777215));
        trackShowPath->setChecked(true);

        gridLayout11->addWidget(trackShowPath, 2, 0, 1, 1);

        trackCurrentPointSize = new QSpinBox(groupBox_3);
        trackCurrentPointSize->setObjectName(QString::fromUtf8("trackCurrentPointSize"));
        trackCurrentPointSize->setEnabled(false);
        trackCurrentPointSize->setMaximumSize(QSize(16777215, 16777215));
        trackCurrentPointSize->setAlignment(Qt::AlignRight);
        trackCurrentPointSize->setMinimum(1);
        trackCurrentPointSize->setValue(60);

        gridLayout11->addWidget(trackCurrentPointSize, 0, 2, 1, 1);

        trackColColorSize = new QSpinBox(groupBox_3);
        trackColColorSize->setObjectName(QString::fromUtf8("trackColColorSize"));
        trackColColorSize->setAlignment(Qt::AlignRight);
        trackColColorSize->setMinimum(1);
        trackColColorSize->setValue(11);

        gridLayout11->addWidget(trackColColorSize, 3, 2, 1, 1);

        trackShowCurrentPoint = new QCheckBox(groupBox_3);
        trackShowCurrentPoint->setObjectName(QString::fromUtf8("trackShowCurrentPoint"));
        trackShowCurrentPoint->setMaximumSize(QSize(150, 16777215));
        trackShowCurrentPoint->setChecked(true);

        gridLayout11->addWidget(trackShowCurrentPoint, 0, 0, 1, 1);

        label_25 = new QLabel(groupBox_3);
        label_25->setObjectName(QString::fromUtf8("label_25"));
        label_25->setMaximumSize(QSize(30, 16777215));

        gridLayout11->addWidget(label_25, 2, 1, 1, 1);

        label_36 = new QLabel(groupBox_3);
        label_36->setObjectName(QString::fromUtf8("label_36"));
        label_36->setMaximumSize(QSize(30, 16777215));

        gridLayout11->addWidget(label_36, 3, 1, 1, 1);

        trackShowColColor = new QCheckBox(groupBox_3);
        trackShowColColor->setObjectName(QString::fromUtf8("trackShowColColor"));
        trackShowColColor->setMinimumSize(QSize(124, 0));
        trackShowColColor->setMaximumSize(QSize(170, 16777215));
        trackShowColColor->setChecked(true);

        gridLayout11->addWidget(trackShowColColor, 3, 0, 1, 1);

        trackShowColorMarker = new QCheckBox(groupBox_3);
        trackShowColorMarker->setObjectName(QString::fromUtf8("trackShowColorMarker"));
        trackShowColorMarker->setMaximumSize(QSize(170, 16777215));
        trackShowColorMarker->setChecked(true);

        gridLayout11->addWidget(trackShowColorMarker, 4, 0, 1, 1);

        trackShowNumber = new QCheckBox(groupBox_3);
        trackShowNumber->setObjectName(QString::fromUtf8("trackShowNumber"));
        trackShowNumber->setMaximumSize(QSize(170, 16777215));
        trackShowNumber->setChecked(true);

        gridLayout11->addWidget(trackShowNumber, 5, 0, 1, 1);

        label_48 = new QLabel(groupBox_3);
        label_48->setObjectName(QString::fromUtf8("label_48"));
        label_48->setMaximumSize(QSize(30, 16777215));

        gridLayout11->addWidget(label_48, 5, 1, 1, 1);

        trackNumberSize = new QSpinBox(groupBox_3);
        trackNumberSize->setObjectName(QString::fromUtf8("trackNumberSize"));
        trackNumberSize->setAlignment(Qt::AlignRight);
        trackNumberSize->setMinimum(2);
        trackNumberSize->setValue(14);

        gridLayout11->addWidget(trackNumberSize, 5, 2, 1, 1);

        trackNumberBold = new QCheckBox(groupBox_3);
        trackNumberBold->setObjectName(QString::fromUtf8("trackNumberBold"));
        trackNumberBold->setMaximumSize(QSize(70, 16777215));
        trackNumberBold->setChecked(true);

        gridLayout11->addWidget(trackNumberBold, 5, 3, 1, 1);

        trackHeadSized = new QCheckBox(groupBox_3);
        trackHeadSized->setObjectName(QString::fromUtf8("trackHeadSized"));
        trackHeadSized->setMaximumSize(QSize(70, 16777215));
        trackHeadSized->setChecked(true);

        gridLayout11->addWidget(trackHeadSized, 0, 3, 1, 1);

        trackShowHeightIndividual = new QCheckBox(groupBox_3);
        trackShowHeightIndividual->setObjectName(QString::fromUtf8("trackShowHeightIndividual"));
        trackShowHeightIndividual->setMaximumSize(QSize(70, 16777215));
        trackShowHeightIndividual->setChecked(true);

        gridLayout11->addWidget(trackShowHeightIndividual, 3, 3, 1, 1);

        trackPathColorButton = new QPushButton(groupBox_3);
        trackPathColorButton->setObjectName(QString::fromUtf8("trackPathColorButton"));
        sizePolicy5.setHeightForWidth(trackPathColorButton->sizePolicy().hasHeightForWidth());
        trackPathColorButton->setSizePolicy(sizePolicy5);
        trackPathColorButton->setMaximumSize(QSize(60, 20));
        trackPathColorButton->setBaseSize(QSize(0, 0));
        QPalette palette;
        QBrush brush(QColor(0, 0, 0, 255));
        brush.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::WindowText, brush);
        QBrush brush1(QColor(255, 0, 0, 255));
        brush1.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Button, brush1);
        QBrush brush2(QColor(255, 127, 127, 255));
        brush2.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Light, brush2);
        QBrush brush3(QColor(255, 63, 63, 255));
        brush3.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Midlight, brush3);
        QBrush brush4(QColor(127, 0, 0, 255));
        brush4.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Dark, brush4);
        QBrush brush5(QColor(170, 0, 0, 255));
        brush5.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Mid, brush5);
        palette.setBrush(QPalette::Active, QPalette::Text, brush);
        QBrush brush6(QColor(255, 255, 255, 255));
        brush6.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::BrightText, brush6);
        palette.setBrush(QPalette::Active, QPalette::ButtonText, brush);
        palette.setBrush(QPalette::Active, QPalette::Base, brush6);
        palette.setBrush(QPalette::Active, QPalette::Window, brush1);
        palette.setBrush(QPalette::Active, QPalette::Shadow, brush);
        palette.setBrush(QPalette::Active, QPalette::AlternateBase, brush2);
        QBrush brush7(QColor(255, 255, 220, 255));
        brush7.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::ToolTipBase, brush7);
        palette.setBrush(QPalette::Active, QPalette::ToolTipText, brush);
        palette.setBrush(QPalette::Inactive, QPalette::WindowText, brush);
        palette.setBrush(QPalette::Inactive, QPalette::Button, brush1);
        palette.setBrush(QPalette::Inactive, QPalette::Light, brush2);
        palette.setBrush(QPalette::Inactive, QPalette::Midlight, brush3);
        palette.setBrush(QPalette::Inactive, QPalette::Dark, brush4);
        palette.setBrush(QPalette::Inactive, QPalette::Mid, brush5);
        palette.setBrush(QPalette::Inactive, QPalette::Text, brush);
        palette.setBrush(QPalette::Inactive, QPalette::BrightText, brush6);
        palette.setBrush(QPalette::Inactive, QPalette::ButtonText, brush);
        palette.setBrush(QPalette::Inactive, QPalette::Base, brush6);
        palette.setBrush(QPalette::Inactive, QPalette::Window, brush1);
        palette.setBrush(QPalette::Inactive, QPalette::Shadow, brush);
        palette.setBrush(QPalette::Inactive, QPalette::AlternateBase, brush2);
        palette.setBrush(QPalette::Inactive, QPalette::ToolTipBase, brush7);
        palette.setBrush(QPalette::Inactive, QPalette::ToolTipText, brush);
        palette.setBrush(QPalette::Disabled, QPalette::WindowText, brush4);
        palette.setBrush(QPalette::Disabled, QPalette::Button, brush1);
        palette.setBrush(QPalette::Disabled, QPalette::Light, brush2);
        palette.setBrush(QPalette::Disabled, QPalette::Midlight, brush3);
        palette.setBrush(QPalette::Disabled, QPalette::Dark, brush4);
        palette.setBrush(QPalette::Disabled, QPalette::Mid, brush5);
        palette.setBrush(QPalette::Disabled, QPalette::Text, brush4);
        palette.setBrush(QPalette::Disabled, QPalette::BrightText, brush6);
        palette.setBrush(QPalette::Disabled, QPalette::ButtonText, brush4);
        palette.setBrush(QPalette::Disabled, QPalette::Base, brush1);
        palette.setBrush(QPalette::Disabled, QPalette::Window, brush1);
        palette.setBrush(QPalette::Disabled, QPalette::Shadow, brush);
        palette.setBrush(QPalette::Disabled, QPalette::AlternateBase, brush1);
        palette.setBrush(QPalette::Disabled, QPalette::ToolTipBase, brush7);
        palette.setBrush(QPalette::Disabled, QPalette::ToolTipText, brush);
        trackPathColorButton->setPalette(palette);

        gridLayout11->addWidget(trackPathColorButton, 2, 3, 1, 1);

        trackPathWidth = new QSpinBox(groupBox_3);
        trackPathWidth->setObjectName(QString::fromUtf8("trackPathWidth"));
        trackPathWidth->setAlignment(Qt::AlignRight);
        trackPathWidth->setMinimum(1);
        trackPathWidth->setMaximum(20);
        trackPathWidth->setValue(2);

        gridLayout11->addWidget(trackPathWidth, 2, 2, 1, 1);

        trackShowPoints = new QCheckBox(groupBox_3);
        trackShowPoints->setObjectName(QString::fromUtf8("trackShowPoints"));
        trackShowPoints->setMinimumSize(QSize(90, 0));
        trackShowPoints->setMaximumSize(QSize(170, 16777215));
        trackShowPoints->setChecked(false);

        gridLayout11->addWidget(trackShowPoints, 1, 0, 1, 1);

        trackShowPointsColored = new QCheckBox(groupBox_3);
        trackShowPointsColored->setObjectName(QString::fromUtf8("trackShowPointsColored"));
        trackShowPointsColored->setMaximumSize(QSize(70, 16777215));
        trackShowPointsColored->setChecked(true);

        gridLayout11->addWidget(trackShowPointsColored, 1, 3, 1, 1);

        label_13 = new QLabel(groupBox_3);
        label_13->setObjectName(QString::fromUtf8("label_13"));
        label_13->setMaximumSize(QSize(30, 16777215));

        gridLayout11->addWidget(label_13, 1, 1, 1, 1);

        label_41 = new QLabel(groupBox_3);
        label_41->setObjectName(QString::fromUtf8("label_41"));
        label_41->setMaximumSize(QSize(30, 16777215));

        gridLayout11->addWidget(label_41, 4, 1, 1, 1);

        trackColorMarkerSize = new QSpinBox(groupBox_3);
        trackColorMarkerSize->setObjectName(QString::fromUtf8("trackColorMarkerSize"));
        trackColorMarkerSize->setAlignment(Qt::AlignRight);
        trackColorMarkerSize->setMinimum(1);
        trackColorMarkerSize->setValue(14);

        gridLayout11->addWidget(trackColorMarkerSize, 4, 2, 1, 1);

        trackPointSize = new QSpinBox(groupBox_3);
        trackPointSize->setObjectName(QString::fromUtf8("trackPointSize"));
        trackPointSize->setAlignment(Qt::AlignRight);
        trackPointSize->setMinimum(1);
        trackPointSize->setValue(7);

        gridLayout11->addWidget(trackPointSize, 1, 2, 1, 1);

        label_47 = new QLabel(groupBox_3);
        label_47->setObjectName(QString::fromUtf8("label_47"));
        label_47->setMaximumSize(QSize(30, 16777215));

        gridLayout11->addWidget(label_47, 0, 1, 1, 1);

        trackShowGroundPosition = new QCheckBox(groupBox_3);
        trackShowGroundPosition->setObjectName(QString::fromUtf8("trackShowGroundPosition"));
        trackShowGroundPosition->setMaximumSize(QSize(170, 16777215));
        trackShowGroundPosition->setChecked(false);

        gridLayout11->addWidget(trackShowGroundPosition, 6, 0, 1, 1);

        label_67 = new QLabel(groupBox_3);
        label_67->setObjectName(QString::fromUtf8("label_67"));
        label_67->setMaximumSize(QSize(30, 16777215));

        gridLayout11->addWidget(label_67, 6, 1, 1, 1);

        trackGroundPositionSize = new QSpinBox(groupBox_3);
        trackGroundPositionSize->setObjectName(QString::fromUtf8("trackGroundPositionSize"));
        trackGroundPositionSize->setAlignment(Qt::AlignRight);
        trackGroundPositionSize->setMinimum(1);
        trackGroundPositionSize->setValue(1);

        gridLayout11->addWidget(trackGroundPositionSize, 6, 2, 1, 1);

        trackShowGroundPath = new QCheckBox(groupBox_3);
        trackShowGroundPath->setObjectName(QString::fromUtf8("trackShowGroundPath"));
        trackShowGroundPath->setMaximumSize(QSize(170, 16777215));
        trackShowGroundPath->setChecked(false);

        gridLayout11->addWidget(trackShowGroundPath, 7, 0, 1, 1);

        label_68 = new QLabel(groupBox_3);
        label_68->setObjectName(QString::fromUtf8("label_68"));
        label_68->setMaximumSize(QSize(30, 16777215));

        gridLayout11->addWidget(label_68, 7, 1, 1, 1);

        trackGroundPathSize = new QSpinBox(groupBox_3);
        trackGroundPathSize->setObjectName(QString::fromUtf8("trackGroundPathSize"));
        trackGroundPathSize->setAlignment(Qt::AlignRight);
        trackGroundPathSize->setMinimum(1);
        trackGroundPathSize->setValue(1);

        gridLayout11->addWidget(trackGroundPathSize, 7, 2, 1, 1);

        trackGroundPathColorButton = new QPushButton(groupBox_3);
        trackGroundPathColorButton->setObjectName(QString::fromUtf8("trackGroundPathColorButton"));
        sizePolicy5.setHeightForWidth(trackGroundPathColorButton->sizePolicy().hasHeightForWidth());
        trackGroundPathColorButton->setSizePolicy(sizePolicy5);
        trackGroundPathColorButton->setMaximumSize(QSize(60, 20));
        trackGroundPathColorButton->setBaseSize(QSize(0, 0));
        QPalette palette1;
        palette1.setBrush(QPalette::Active, QPalette::WindowText, brush);
        QBrush brush8(QColor(0, 255, 0, 255));
        brush8.setStyle(Qt::SolidPattern);
        palette1.setBrush(QPalette::Active, QPalette::Button, brush8);
        QBrush brush9(QColor(127, 255, 127, 255));
        brush9.setStyle(Qt::SolidPattern);
        palette1.setBrush(QPalette::Active, QPalette::Light, brush9);
        QBrush brush10(QColor(63, 255, 63, 255));
        brush10.setStyle(Qt::SolidPattern);
        palette1.setBrush(QPalette::Active, QPalette::Midlight, brush10);
        QBrush brush11(QColor(0, 127, 0, 255));
        brush11.setStyle(Qt::SolidPattern);
        palette1.setBrush(QPalette::Active, QPalette::Dark, brush11);
        QBrush brush12(QColor(0, 170, 0, 255));
        brush12.setStyle(Qt::SolidPattern);
        palette1.setBrush(QPalette::Active, QPalette::Mid, brush12);
        palette1.setBrush(QPalette::Active, QPalette::Text, brush);
        palette1.setBrush(QPalette::Active, QPalette::BrightText, brush6);
        palette1.setBrush(QPalette::Active, QPalette::ButtonText, brush);
        palette1.setBrush(QPalette::Active, QPalette::Base, brush6);
        palette1.setBrush(QPalette::Active, QPalette::Window, brush8);
        palette1.setBrush(QPalette::Active, QPalette::Shadow, brush);
        palette1.setBrush(QPalette::Active, QPalette::AlternateBase, brush9);
        palette1.setBrush(QPalette::Active, QPalette::ToolTipBase, brush7);
        palette1.setBrush(QPalette::Active, QPalette::ToolTipText, brush);
        palette1.setBrush(QPalette::Inactive, QPalette::WindowText, brush);
        palette1.setBrush(QPalette::Inactive, QPalette::Button, brush8);
        palette1.setBrush(QPalette::Inactive, QPalette::Light, brush9);
        palette1.setBrush(QPalette::Inactive, QPalette::Midlight, brush10);
        palette1.setBrush(QPalette::Inactive, QPalette::Dark, brush11);
        palette1.setBrush(QPalette::Inactive, QPalette::Mid, brush12);
        palette1.setBrush(QPalette::Inactive, QPalette::Text, brush);
        palette1.setBrush(QPalette::Inactive, QPalette::BrightText, brush6);
        palette1.setBrush(QPalette::Inactive, QPalette::ButtonText, brush);
        palette1.setBrush(QPalette::Inactive, QPalette::Base, brush6);
        palette1.setBrush(QPalette::Inactive, QPalette::Window, brush8);
        palette1.setBrush(QPalette::Inactive, QPalette::Shadow, brush);
        palette1.setBrush(QPalette::Inactive, QPalette::AlternateBase, brush9);
        palette1.setBrush(QPalette::Inactive, QPalette::ToolTipBase, brush7);
        palette1.setBrush(QPalette::Inactive, QPalette::ToolTipText, brush);
        palette1.setBrush(QPalette::Disabled, QPalette::WindowText, brush11);
        palette1.setBrush(QPalette::Disabled, QPalette::Button, brush8);
        palette1.setBrush(QPalette::Disabled, QPalette::Light, brush9);
        palette1.setBrush(QPalette::Disabled, QPalette::Midlight, brush10);
        palette1.setBrush(QPalette::Disabled, QPalette::Dark, brush11);
        palette1.setBrush(QPalette::Disabled, QPalette::Mid, brush12);
        palette1.setBrush(QPalette::Disabled, QPalette::Text, brush11);
        palette1.setBrush(QPalette::Disabled, QPalette::BrightText, brush6);
        palette1.setBrush(QPalette::Disabled, QPalette::ButtonText, brush11);
        palette1.setBrush(QPalette::Disabled, QPalette::Base, brush8);
        palette1.setBrush(QPalette::Disabled, QPalette::Window, brush8);
        palette1.setBrush(QPalette::Disabled, QPalette::Shadow, brush);
        palette1.setBrush(QPalette::Disabled, QPalette::AlternateBase, brush8);
        palette1.setBrush(QPalette::Disabled, QPalette::ToolTipBase, brush7);
        palette1.setBrush(QPalette::Disabled, QPalette::ToolTipText, brush);
        trackGroundPathColorButton->setPalette(palette1);

        gridLayout11->addWidget(trackGroundPathColorButton, 7, 3, 1, 1);

        gridLayout11->setColumnStretch(0, 5);
        gridLayout11->setColumnStretch(1, 1);
        gridLayout11->setColumnStretch(2, 1);
        gridLayout11->setColumnStretch(3, 2);

        vboxLayout2->addLayout(gridLayout11);

        spacerItem7 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        vboxLayout2->addItem(spacerItem7);

        gridLayout12 = new QGridLayout();
        gridLayout12->setSpacing(0);
        gridLayout12->setObjectName(QString::fromUtf8("gridLayout12"));
        gridLayout12->setContentsMargins(0, 0, 0, 0);
        label_17 = new QLabel(groupBox_3);
        label_17->setObjectName(QString::fromUtf8("label_17"));
        label_17->setMinimumSize(QSize(150, 0));
        label_17->setMaximumSize(QSize(250, 16777215));

        gridLayout12->addWidget(label_17, 0, 0, 1, 1);

        trackShowAfter = new QSpinBox(groupBox_3);
        trackShowAfter->setObjectName(QString::fromUtf8("trackShowAfter"));
        trackShowAfter->setMaximumSize(QSize(80, 16777215));
        trackShowAfter->setAlignment(Qt::AlignRight);
        trackShowAfter->setMinimum(-1);
        trackShowAfter->setMaximum(99999);
        trackShowAfter->setValue(15);

        gridLayout12->addWidget(trackShowAfter, 1, 1, 1, 1);

        label_20 = new QLabel(groupBox_3);
        label_20->setObjectName(QString::fromUtf8("label_20"));
        label_20->setMaximumSize(QSize(250, 16777215));

        gridLayout12->addWidget(label_20, 1, 0, 1, 1);

        trackShowBefore = new QSpinBox(groupBox_3);
        trackShowBefore->setObjectName(QString::fromUtf8("trackShowBefore"));
        trackShowBefore->setMaximumSize(QSize(80, 16777215));
        trackShowBefore->setAlignment(Qt::AlignRight);
        trackShowBefore->setMinimum(-1);
        trackShowBefore->setMaximum(99999);
        trackShowBefore->setValue(15);

        gridLayout12->addWidget(trackShowBefore, 0, 1, 1, 1);

        spacerItem8 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout12->addItem(spacerItem8, 0, 2, 1, 1);


        vboxLayout2->addLayout(gridLayout12);


        verticalLayout_21->addLayout(vboxLayout2);


        verticalLayout_17->addWidget(groupBox_3);

        verticalSpacer_4 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_17->addItem(verticalSpacer_4);

        scrollArea_3->setWidget(scrollAreaWidgetContents_3);

        horizontalLayout_17->addWidget(scrollArea_3);

        tabs->addTab(track, QString());
        ana = new QWidget();
        ana->setObjectName(QString::fromUtf8("ana"));
        horizontalLayout_18 = new QHBoxLayout(ana);
        horizontalLayout_18->setObjectName(QString::fromUtf8("horizontalLayout_18"));
        scrollArea_4 = new QScrollArea(ana);
        scrollArea_4->setObjectName(QString::fromUtf8("scrollArea_4"));
        scrollArea_4->setWidgetResizable(true);
        scrollAreaWidgetContents_4 = new QWidget();
        scrollAreaWidgetContents_4->setObjectName(QString::fromUtf8("scrollAreaWidgetContents_4"));
        scrollAreaWidgetContents_4->setGeometry(QRect(0, 0, 354, 311));
        verticalLayout_18 = new QVBoxLayout(scrollAreaWidgetContents_4);
        verticalLayout_18->setObjectName(QString::fromUtf8("verticalLayout_18"));
        analysePlot = new AnalysePlot(scrollAreaWidgetContents_4);
        analysePlot->setObjectName(QString::fromUtf8("analysePlot"));
        QSizePolicy sizePolicy11(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy11.setHorizontalStretch(0);
        sizePolicy11.setVerticalStretch(0);
        sizePolicy11.setHeightForWidth(analysePlot->sizePolicy().hasHeightForWidth());
        analysePlot->setSizePolicy(sizePolicy11);
        analysePlot->setMaximumSize(QSize(16777215, 400));
        analysePlot->setFrameShape(QFrame::StyledPanel);
        analysePlot->setFrameShadow(QFrame::Raised);

        verticalLayout_18->addWidget(analysePlot);

        hboxLayout9 = new QHBoxLayout();
        hboxLayout9->setObjectName(QString::fromUtf8("hboxLayout9"));
        anaCalculate = new QPushButton(scrollAreaWidgetContents_4);
        anaCalculate->setObjectName(QString::fromUtf8("anaCalculate"));

        hboxLayout9->addWidget(anaCalculate);

        anaMissingFrames = new QCheckBox(scrollAreaWidgetContents_4);
        anaMissingFrames->setObjectName(QString::fromUtf8("anaMissingFrames"));
        anaMissingFrames->setChecked(true);

        hboxLayout9->addWidget(anaMissingFrames);


        verticalLayout_18->addLayout(hboxLayout9);

        hboxLayout10 = new QHBoxLayout();
        hboxLayout10->setObjectName(QString::fromUtf8("hboxLayout10"));
        hboxLayout11 = new QHBoxLayout();
        hboxLayout11->setObjectName(QString::fromUtf8("hboxLayout11"));
        label_21 = new QLabel(scrollAreaWidgetContents_4);
        label_21->setObjectName(QString::fromUtf8("label_21"));

        hboxLayout11->addWidget(label_21);

        anaStep = new QSpinBox(scrollAreaWidgetContents_4);
        anaStep->setObjectName(QString::fromUtf8("anaStep"));
        anaStep->setMinimum(1);
        anaStep->setValue(25);

        hboxLayout11->addWidget(anaStep);


        hboxLayout10->addLayout(hboxLayout11);

        spacerItem9 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout10->addItem(spacerItem9);

        anaMarkAct = new QCheckBox(scrollAreaWidgetContents_4);
        anaMarkAct->setObjectName(QString::fromUtf8("anaMarkAct"));

        hboxLayout10->addWidget(anaMarkAct);


        verticalLayout_18->addLayout(hboxLayout10);

        hboxLayout12 = new QHBoxLayout();
        hboxLayout12->setObjectName(QString::fromUtf8("hboxLayout12"));
        label_16 = new QLabel(scrollAreaWidgetContents_4);
        label_16->setObjectName(QString::fromUtf8("label_16"));

        hboxLayout12->addWidget(label_16);

        anaConsiderX = new QCheckBox(scrollAreaWidgetContents_4);
        anaConsiderX->setObjectName(QString::fromUtf8("anaConsiderX"));

        hboxLayout12->addWidget(anaConsiderX);

        anaConsiderY = new QCheckBox(scrollAreaWidgetContents_4);
        anaConsiderY->setObjectName(QString::fromUtf8("anaConsiderY"));
        anaConsiderY->setChecked(true);

        hboxLayout12->addWidget(anaConsiderY);

        anaConsiderAbs = new QCheckBox(scrollAreaWidgetContents_4);
        anaConsiderAbs->setObjectName(QString::fromUtf8("anaConsiderAbs"));

        hboxLayout12->addWidget(anaConsiderAbs);

        anaConsiderRev = new QCheckBox(scrollAreaWidgetContents_4);
        anaConsiderRev->setObjectName(QString::fromUtf8("anaConsiderRev"));

        hboxLayout12->addWidget(anaConsiderRev);


        verticalLayout_18->addLayout(hboxLayout12);

        showVoronoiCells = new QCheckBox(scrollAreaWidgetContents_4);
        showVoronoiCells->setObjectName(QString::fromUtf8("showVoronoiCells"));

        verticalLayout_18->addWidget(showVoronoiCells);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_18->addItem(verticalSpacer);

        scrollArea_4->setWidget(scrollAreaWidgetContents_4);

        horizontalLayout_18->addWidget(scrollArea_4);

        tabs->addTab(ana, QString());

        horizontalLayout_19->addWidget(tabs);

#if QT_CONFIG(shortcut)
        label_7->setBuddy(tx);
        label_3->setBuddy(cx);
        label_6->setBuddy(r4);
        label_8->setBuddy(ty);
        label->setBuddy(fx);
        label_4->setBuddy(cy);
        label_2->setBuddy(fy);
        label_5->setBuddy(r2);
        label_64->setBuddy(r2);
#endif // QT_CONFIG(shortcut)
        QWidget::setTabOrder(fx, fy);
        QWidget::setTabOrder(fy, cx);
        QWidget::setTabOrder(cx, cy);
        QWidget::setTabOrder(cy, r2);
        QWidget::setTabOrder(r2, r4);
        QWidget::setTabOrder(r4, tx);
        QWidget::setTabOrder(tx, ty);
        QWidget::setTabOrder(ty, gridTransY_spin);
        QWidget::setTabOrder(gridTransY_spin, gridScale_spin);
        QWidget::setTabOrder(gridScale_spin, gridTransX_spin);
        QWidget::setTabOrder(gridTransX_spin, gridRot_spin);
        QWidget::setTabOrder(gridRot_spin, coordTransY_spin);
        QWidget::setTabOrder(coordTransY_spin, coordScale_spin);
        QWidget::setTabOrder(coordScale_spin, coordTransX_spin);
        QWidget::setTabOrder(coordTransX_spin, coordRotate_spin);
        QWidget::setTabOrder(coordRotate_spin, coordAltitude);
        QWidget::setTabOrder(coordAltitude, coordUnit);
        QWidget::setTabOrder(coordUnit, coordUseIntrinsic);
        QWidget::setTabOrder(coordUseIntrinsic, filterBrightContrast);
        QWidget::setTabOrder(filterBrightContrast, filterBorder);
        QWidget::setTabOrder(filterBorder, filterBorderParamCol);
        QWidget::setTabOrder(filterBorderParamCol, filterBg);
        QWidget::setTabOrder(filterBg, filterBgReset);
        QWidget::setTabOrder(filterBgReset, filterBgLoad);
        QWidget::setTabOrder(filterBgLoad, filterBgSave);
        QWidget::setTabOrder(filterBgSave, filterSwap);
        QWidget::setTabOrder(filterSwap, filterSwapH);
        QWidget::setTabOrder(filterSwapH, filterSwapV);
        QWidget::setTabOrder(filterSwapV, autoCalib);
        QWidget::setTabOrder(autoCalib, calibFiles);
        QWidget::setTabOrder(calibFiles, fixCenter);
        QWidget::setTabOrder(fixCenter, quadAspectRatio);
        QWidget::setTabOrder(quadAspectRatio, tangDist);
        QWidget::setTabOrder(tangDist, apply);
        QWidget::setTabOrder(apply, markerIgnoreWithout);
        QWidget::setTabOrder(markerIgnoreWithout, roiShow);
        QWidget::setTabOrder(roiShow, roiFix);
        QWidget::setTabOrder(roiFix, recoShowColor);
        QWidget::setTabOrder(recoShowColor, recoOptimizeColor);
        QWidget::setTabOrder(recoOptimizeColor, recoColorModel);
        QWidget::setTabOrder(recoColorModel, recoAutoWB);
        QWidget::setTabOrder(recoAutoWB, recoColorX);
        QWidget::setTabOrder(recoColorX, recoColorY);
        QWidget::setTabOrder(recoColorY, mapNr);
        QWidget::setTabOrder(mapNr, mapColor);
        QWidget::setTabOrder(mapColor, mapHeight);
        QWidget::setTabOrder(mapHeight, mapAdd);
        QWidget::setTabOrder(mapAdd, mapDel);
        QWidget::setTabOrder(mapDel, mapDefaultHeight);
        QWidget::setTabOrder(mapDefaultHeight, mapDistribution);
        QWidget::setTabOrder(mapDistribution, mapResetHeight);
        QWidget::setTabOrder(mapResetHeight, mapResetPos);
        QWidget::setTabOrder(mapResetPos, performRecognition);
        QWidget::setTabOrder(performRecognition, recoStep);
        QWidget::setTabOrder(recoStep, recoMethod);
        QWidget::setTabOrder(recoMethod, recoStereoShow);
        QWidget::setTabOrder(recoStereoShow, trackOnlineCalc);
        QWidget::setTabOrder(trackOnlineCalc, trackRepeat);
        QWidget::setTabOrder(trackRepeat, trackRepeatQual);
        QWidget::setTabOrder(trackRepeatQual, trackExtrapolation);
        QWidget::setTabOrder(trackExtrapolation, trackMerge);
        QWidget::setTabOrder(trackMerge, trackOnlyVisible);
        QWidget::setTabOrder(trackOnlyVisible, trackExport);
        QWidget::setTabOrder(trackExport, trackCalc);
        QWidget::setTabOrder(trackCalc, trackImport);
        QWidget::setTabOrder(trackImport, trackReset);
        QWidget::setTabOrder(trackReset, trackMissingFrames);
        QWidget::setTabOrder(trackMissingFrames, trackRecalcHeight);
        QWidget::setTabOrder(trackRecalcHeight, trackAlternateHeight);
        QWidget::setTabOrder(trackAlternateHeight, exportElimTp);
        QWidget::setTabOrder(exportElimTp, exportElimTrj);
        QWidget::setTabOrder(exportElimTrj, exportSmooth);
        QWidget::setTabOrder(exportSmooth, trackTest);
        QWidget::setTabOrder(trackTest, testEqual);
        QWidget::setTabOrder(testEqual, testVelocity);
        QWidget::setTabOrder(testVelocity, testInside);
        QWidget::setTabOrder(testInside, testLength);
        QWidget::setTabOrder(testLength, trackShowSearchSize);
        QWidget::setTabOrder(trackShowSearchSize, trackShow);
        QWidget::setTabOrder(trackShow, trackFix);
        QWidget::setTabOrder(trackFix, trackShowOnlyVisible);
        QWidget::setTabOrder(trackShowOnlyVisible, trackShowOnly);
        QWidget::setTabOrder(trackShowOnly, trackGotoNr);
        QWidget::setTabOrder(trackGotoNr, trackPathWidth);
        QWidget::setTabOrder(trackPathWidth, trackShowPoints);
        QWidget::setTabOrder(trackShowPoints, trackShowPointsColored);
        QWidget::setTabOrder(trackShowPointsColored, trackColorMarkerSize);
        QWidget::setTabOrder(trackColorMarkerSize, trackPointSize);
        QWidget::setTabOrder(trackPointSize, trackShowPath);
        QWidget::setTabOrder(trackShowPath, trackCurrentPointSize);
        QWidget::setTabOrder(trackCurrentPointSize, trackColColorSize);
        QWidget::setTabOrder(trackColColorSize, trackShowCurrentPoint);
        QWidget::setTabOrder(trackShowCurrentPoint, trackShowColColor);
        QWidget::setTabOrder(trackShowColColor, trackShowColorMarker);
        QWidget::setTabOrder(trackShowColorMarker, trackShowNumber);
        QWidget::setTabOrder(trackShowNumber, trackNumberSize);
        QWidget::setTabOrder(trackNumberSize, trackNumberBold);
        QWidget::setTabOrder(trackNumberBold, trackHeadSized);
        QWidget::setTabOrder(trackHeadSized, trackShowHeightIndividual);
        QWidget::setTabOrder(trackShowHeightIndividual, trackShowAfter);
        QWidget::setTabOrder(trackShowAfter, trackShowBefore);
        QWidget::setTabOrder(trackShowBefore, anaStep);
        QWidget::setTabOrder(anaStep, anaMarkAct);
        QWidget::setTabOrder(anaMarkAct, anaCalculate);
        QWidget::setTabOrder(anaCalculate, anaMissingFrames);
        QWidget::setTabOrder(anaMissingFrames, anaConsiderX);
        QWidget::setTabOrder(anaConsiderX, anaConsiderY);
        QWidget::setTabOrder(anaConsiderY, anaConsiderAbs);
        QWidget::setTabOrder(anaConsiderAbs, anaConsiderRev);

        retranslateUi(Control);
        QObject::connect(gridTransY, SIGNAL(valueChanged(int)), gridTransY_spin, SLOT(setValue(int)));
        QObject::connect(gridTransY_spin, SIGNAL(valueChanged(int)), gridTransY, SLOT(setValue(int)));
        QObject::connect(gridScale, SIGNAL(valueChanged(int)), gridScale_spin, SLOT(setValue(int)));
        QObject::connect(gridTransX, SIGNAL(valueChanged(int)), gridTransX_spin, SLOT(setValue(int)));
        QObject::connect(gridRot_spin, SIGNAL(valueChanged(int)), gridRotate, SLOT(setValue(int)));
        QObject::connect(coordTransY, SIGNAL(valueChanged(int)), coordTransY_spin, SLOT(setValue(int)));
        QObject::connect(gridRotate, SIGNAL(valueChanged(int)), gridRot_spin, SLOT(setValue(int)));
        QObject::connect(coordTransX, SIGNAL(valueChanged(int)), coordTransX_spin, SLOT(setValue(int)));
        QObject::connect(gridScale_spin, SIGNAL(valueChanged(int)), gridScale, SLOT(setValue(int)));
        QObject::connect(gridTransX_spin, SIGNAL(valueChanged(int)), gridTransX, SLOT(setValue(int)));
        QObject::connect(coordScale, SIGNAL(valueChanged(int)), coordScale_spin, SLOT(setValue(int)));
        QObject::connect(coordRotate, SIGNAL(valueChanged(int)), coordRotate_spin, SLOT(setValue(int)));
        QObject::connect(coordTransY_spin, SIGNAL(valueChanged(int)), coordTransY, SLOT(setValue(int)));
        QObject::connect(coordTransX_spin, SIGNAL(valueChanged(int)), coordTransX, SLOT(setValue(int)));
        QObject::connect(grid3DTransY, SIGNAL(valueChanged(int)), grid3DTransY_spin, SLOT(setValue(int)));
        QObject::connect(coord3DTransZ_spin, SIGNAL(valueChanged(int)), coord3DTransZ, SLOT(setValue(int)));
        QObject::connect(coordRotate_spin, SIGNAL(valueChanged(int)), coordRotate, SLOT(setValue(int)));
        QObject::connect(coord3DAxeLen_spin, SIGNAL(valueChanged(int)), coord3DAxeLen, SLOT(setValue(int)));
        QObject::connect(coordScale_spin, SIGNAL(valueChanged(int)), coordScale, SLOT(setValue(int)));
        QObject::connect(grid3DTransX, SIGNAL(valueChanged(int)), grid3DTransX_spin, SLOT(setValue(int)));
        QObject::connect(grid3DResolution, SIGNAL(valueChanged(int)), grid3DResolution_spin, SLOT(setValue(int)));
        QObject::connect(grid3DTransZ, SIGNAL(valueChanged(int)), grid3DTransZ_spin, SLOT(setValue(int)));
        QObject::connect(grid3DResolution_spin, SIGNAL(valueChanged(int)), grid3DResolution, SLOT(setValue(int)));
        QObject::connect(grid3DTransY_spin, SIGNAL(valueChanged(int)), grid3DTransY, SLOT(setValue(int)));
        QObject::connect(grid3DTransZ_spin, SIGNAL(valueChanged(int)), grid3DTransZ, SLOT(setValue(int)));
        QObject::connect(coord3DTransY, SIGNAL(valueChanged(int)), coord3DTransY_spin, SLOT(setValue(int)));
        QObject::connect(coord3DTransZ, SIGNAL(valueChanged(int)), coord3DTransZ_spin, SLOT(setValue(int)));
        QObject::connect(coord3DAxeLen, SIGNAL(valueChanged(int)), coord3DAxeLen_spin, SLOT(setValue(int)));
        QObject::connect(grid3DTransX_spin, SIGNAL(valueChanged(int)), grid3DTransX, SLOT(setValue(int)));
        QObject::connect(coord3DTransX_spin, SIGNAL(valueChanged(int)), coord3DTransX, SLOT(setValue(int)));
        QObject::connect(coord3DTransY_spin, SIGNAL(valueChanged(int)), coord3DTransY, SLOT(setValue(int)));
        QObject::connect(coord3DTransX, SIGNAL(valueChanged(int)), coord3DTransX_spin, SLOT(setValue(int)));

        tabs->setCurrentIndex(2);
        coordTab->setCurrentIndex(0);
        gridTab->setCurrentIndex(0);
        recoMethod->setCurrentIndex(-1);


        QMetaObject::connectSlotsByName(Control);
    } // setupUi

    void retranslateUi(QWidget *Control)
    {
        Control->setWindowTitle(QCoreApplication::translate("Control", "Form", nullptr));
        groupBox->setTitle(QCoreApplication::translate("Control", "filter before", nullptr));
        filterBorderParamCol->setText(QCoreApplication::translate("Control", "color", nullptr));
        filterBgShow->setText(QCoreApplication::translate("Control", "show", nullptr));
        filterBgUpdate->setText(QCoreApplication::translate("Control", "update", nullptr));
        filterBgReset->setText(QCoreApplication::translate("Control", "reset", nullptr));
        filterBgLoad->setText(QCoreApplication::translate("Control", "load", nullptr));
        filterBgSave->setText(QCoreApplication::translate("Control", "save", nullptr));
#if QT_CONFIG(tooltip)
        filterBgDeleteTrj->setToolTip(QCoreApplication::translate("Control", "delete trajectory after a certain number of trackpoints outside foreground", nullptr));
#endif // QT_CONFIG(tooltip)
        filterBgDeleteTrj->setText(QCoreApplication::translate("Control", "delete", nullptr));
        label_63->setText(QCoreApplication::translate("Control", "trj. after:", nullptr));
        filterSwapH->setText(QCoreApplication::translate("Control", "horizontally", nullptr));
        filterSwapV->setText(QCoreApplication::translate("Control", "vertically", nullptr));
        filterSwap->setText(QCoreApplication::translate("Control", "swap", nullptr));
        filterBorder->setText(QCoreApplication::translate("Control", "border:", nullptr));
        filterBrightContrast->setText(QCoreApplication::translate("Control", "brightness:", nullptr));
        filterBg->setText(QCoreApplication::translate("Control", "bg subtr.:", nullptr));
        label_18->setText(QCoreApplication::translate("Control", "         contrast:", nullptr));
        intr->setTitle(QCoreApplication::translate("Control", "intrinsic parameters", nullptr));
        apply->setText(QCoreApplication::translate("Control", "apply", nullptr));
        label_7->setText(QCoreApplication::translate("Control", "tangential distortion: x:", nullptr));
        label_3->setText(QCoreApplication::translate("Control", "center [px]: x:", nullptr));
        label_6->setText(QCoreApplication::translate("Control", "4th:", nullptr));
        label_8->setText(QCoreApplication::translate("Control", "y:", nullptr));
        label->setText(QCoreApplication::translate("Control", "focal length [px]: x:", nullptr));
        label_4->setText(QCoreApplication::translate("Control", "y:", nullptr));
        label_2->setText(QCoreApplication::translate("Control", "y:", nullptr));
        label_5->setText(QCoreApplication::translate("Control", "radial distortion: 2nd:", nullptr));
        label_64->setText(QCoreApplication::translate("Control", "6th:", nullptr));
        fixCenter->setText(QCoreApplication::translate("Control", "fix center", nullptr));
        quadAspectRatio->setText(QCoreApplication::translate("Control", "quad. aspect ratio", nullptr));
        tangDist->setText(QCoreApplication::translate("Control", "tang. dist.", nullptr));
        autoCalib->setText(QCoreApplication::translate("Control", "auto", nullptr));
        calibFiles->setText(QCoreApplication::translate("Control", "files", nullptr));
        extr->setTitle(QCoreApplication::translate("Control", "extrinsic parameters", nullptr));
#if QT_CONFIG(tooltip)
        label_58->setToolTip(QCoreApplication::translate("Control", "Translate the coordinate system in x-direction.", nullptr));
#endif // QT_CONFIG(tooltip)
        label_58->setText(QCoreApplication::translate("Control", "rotation:", nullptr));
#if QT_CONFIG(tooltip)
        label_59->setToolTip(QCoreApplication::translate("Control", "Translate the coordinate system in x-direction.", nullptr));
#endif // QT_CONFIG(tooltip)
        label_59->setText(QCoreApplication::translate("Control", "translation:", nullptr));
        label_62->setText(QCoreApplication::translate("Control", "3D/2D Points:", nullptr));
#if QT_CONFIG(tooltip)
        coordLoad3DCalibPoints->setToolTip(QCoreApplication::translate("Control", "Load 3D points to corresponding 2D image points", nullptr));
#endif // QT_CONFIG(tooltip)
        coordLoad3DCalibPoints->setText(QCoreApplication::translate("Control", "load", nullptr));
#if QT_CONFIG(tooltip)
        extrCalibFetch->setToolTip(QCoreApplication::translate("Control", "Fetch marked 2D Points to loaded 3D points", nullptr));
#endif // QT_CONFIG(tooltip)
        extrCalibFetch->setText(QCoreApplication::translate("Control", "fetch", nullptr));
#if QT_CONFIG(tooltip)
        extrCalibSave->setToolTip(QCoreApplication::translate("Control", "Save 3D and/or 2D points to extrinisc calib file", nullptr));
#endif // QT_CONFIG(tooltip)
        extrCalibSave->setText(QCoreApplication::translate("Control", "save", nullptr));
#if QT_CONFIG(tooltip)
        extrCalibShowPoints->setToolTip(QCoreApplication::translate("Control", "Show the saved 2D/3D point correspondences in the file", nullptr));
#endif // QT_CONFIG(tooltip)
        extrCalibShowPoints->setText(QCoreApplication::translate("Control", "show", nullptr));
#if QT_CONFIG(tooltip)
        extrCalibShowError->setToolTip(QCoreApplication::translate("Control", "Show the reprojection error of extrinsic calibration", nullptr));
#endif // QT_CONFIG(tooltip)
        extrCalibShowError->setText(QCoreApplication::translate("Control", "error", nullptr));
        align_2->setTitle(QCoreApplication::translate("Control", "coordinate system", nullptr));
#if QT_CONFIG(tooltip)
        coordShow->setToolTip(QCoreApplication::translate("Control", "Show the coordinate system.", nullptr));
#endif // QT_CONFIG(tooltip)
        coordShow->setText(QCoreApplication::translate("Control", "show", nullptr));
#if QT_CONFIG(tooltip)
        coordFix->setToolTip(QCoreApplication::translate("Control", "Fix the coordinate system", nullptr));
#endif // QT_CONFIG(tooltip)
        coordFix->setText(QCoreApplication::translate("Control", "fix", nullptr));
#if QT_CONFIG(tooltip)
        label_54->setToolTip(QCoreApplication::translate("Control", "Translate the coordinate system in x-direction.", nullptr));
#endif // QT_CONFIG(tooltip)
        label_54->setText(QCoreApplication::translate("Control", "translate [cm] x:", nullptr));
#if QT_CONFIG(tooltip)
        coord3DTransX->setToolTip(QCoreApplication::translate("Control", "Translate the coordinate system in x-direction.", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        coord3DTransX_spin->setToolTip(QCoreApplication::translate("Control", "Translate the coordinate system in x-direction.", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        label_55->setToolTip(QCoreApplication::translate("Control", "Translate the coordinate system in y-direction.", nullptr));
#endif // QT_CONFIG(tooltip)
        label_55->setText(QCoreApplication::translate("Control", "y:", nullptr));
#if QT_CONFIG(tooltip)
        coord3DTransY->setToolTip(QCoreApplication::translate("Control", "Translate the coordinate system in y-direction.", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        coord3DTransY_spin->setToolTip(QCoreApplication::translate("Control", "Translate the coordinate system in y-direction.", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        label_57->setToolTip(QCoreApplication::translate("Control", "2D: Rotate the coordinate system clockwise. 3D: Translate the coordinate system in z-direction.", nullptr));
#endif // QT_CONFIG(tooltip)
        label_57->setText(QCoreApplication::translate("Control", "z:", nullptr));
#if QT_CONFIG(tooltip)
        coord3DTransZ->setToolTip(QCoreApplication::translate("Control", "2D: Rotate the coordinate system clockwise. 3D: Translate the coordinate system in z-direction.", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        coord3DTransZ_spin->setToolTip(QCoreApplication::translate("Control", "2D: Rotate the coordinate system clockwise. 3D: Translate the coordinate system in z-direction.", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        label_56->setToolTip(QCoreApplication::translate("Control", "Scale the coordinate system.", nullptr));
#endif // QT_CONFIG(tooltip)
        label_56->setText(QCoreApplication::translate("Control", "axis length:", nullptr));
#if QT_CONFIG(tooltip)
        coord3DAxeLen->setToolTip(QCoreApplication::translate("Control", "Scale the coordinate system.", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        coord3DAxeLen_spin->setToolTip(QCoreApplication::translate("Control", "Scale the coordinate system.", nullptr));
#endif // QT_CONFIG(tooltip)
        coord3DAxeLen_spin->setSuffix(QCoreApplication::translate("Control", " cm", nullptr));
#if QT_CONFIG(tooltip)
        label_60->setToolTip(QCoreApplication::translate("Control", "Translate the coordinate system in x-direction.", nullptr));
#endif // QT_CONFIG(tooltip)
        label_60->setText(QCoreApplication::translate("Control", "swap axis:", nullptr));
        coord3DSwapX->setText(QCoreApplication::translate("Control", "X-axis", nullptr));
        coord3DSwapY->setText(QCoreApplication::translate("Control", "Y-axis", nullptr));
        coord3DSwapZ->setText(QCoreApplication::translate("Control", "Z-axis", nullptr));
#if QT_CONFIG(tooltip)
        label_61->setToolTip(QCoreApplication::translate("Control", "Translate the coordinate system in x-direction.", nullptr));
#endif // QT_CONFIG(tooltip)
        label_61->setText(QCoreApplication::translate("Control", "        show:", nullptr));
        extCalibPointsShow->setText(QCoreApplication::translate("Control", "calibration points", nullptr));
        extVanishPointsShow->setText(QCoreApplication::translate("Control", "vanish points", nullptr));
        coordTab->setTabText(coordTab->indexOf(coordTab3D), QCoreApplication::translate("Control", "     3D     ", nullptr));
#if QT_CONFIG(tooltip)
        coordTab->setTabToolTip(coordTab->indexOf(coordTab3D), QCoreApplication::translate("Control", "Select for 3D coordinatesystem", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        coordTransX_spin->setToolTip(QCoreApplication::translate("Control", "Translate the coordinate system in x-direction.", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        label_34->setToolTip(QCoreApplication::translate("Control", "Translate the coordinate system in x-direction.", nullptr));
#endif // QT_CONFIG(tooltip)
        label_34->setText(QCoreApplication::translate("Control", "translate x:", nullptr));
#if QT_CONFIG(tooltip)
        coordTransX->setToolTip(QCoreApplication::translate("Control", "Translate the coordinate system in x-direction.", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        label_32->setToolTip(QCoreApplication::translate("Control", "Translate the coordinate system in y-direction.", nullptr));
#endif // QT_CONFIG(tooltip)
        label_32->setText(QCoreApplication::translate("Control", "y:", nullptr));
#if QT_CONFIG(tooltip)
        coordTransY->setToolTip(QCoreApplication::translate("Control", "Translate the coordinate system in y-direction.", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        label_33->setToolTip(QCoreApplication::translate("Control", "Scale the coordinate system.", nullptr));
#endif // QT_CONFIG(tooltip)
        label_33->setText(QCoreApplication::translate("Control", "scale:", nullptr));
#if QT_CONFIG(tooltip)
        coordScale->setToolTip(QCoreApplication::translate("Control", "Scale the coordinate system.", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        coordTransY_spin->setToolTip(QCoreApplication::translate("Control", "Translate the coordinate system in y-direction.", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        coordScale_spin->setToolTip(QCoreApplication::translate("Control", "Scale the coordinate system.", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        label_35->setToolTip(QCoreApplication::translate("Control", "2D: Rotate the coordinate system clockwise. 3D: Translate the coordinate system in z-direction.", nullptr));
#endif // QT_CONFIG(tooltip)
        label_35->setText(QCoreApplication::translate("Control", "rotate:", nullptr));
#if QT_CONFIG(tooltip)
        coordRotate->setToolTip(QCoreApplication::translate("Control", "2D: Rotate the coordinate system clockwise. 3D: Translate the coordinate system in z-direction.", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        coordRotate_spin->setToolTip(QCoreApplication::translate("Control", "2D: Rotate the coordinate system clockwise. 3D: Translate the coordinate system in z-direction.", nullptr));
#endif // QT_CONFIG(tooltip)
        label_14->setText(QCoreApplication::translate("Control", "camera altitude [cm]:", nullptr));
        coordAltitude->setSuffix(QString());
        coordAltitudeMeasured->setText(QCoreApplication::translate("Control", "(measured: 535.0)", nullptr));
        label_37->setText(QCoreApplication::translate("Control", "unit [cm]:", nullptr));
        coordUnit->setSuffix(QString());
        coordUseIntrinsic->setText(QCoreApplication::translate("Control", "use intrinsic center for calculating real position", nullptr));
        coordTab->setTabText(coordTab->indexOf(coordTab2D), QCoreApplication::translate("Control", "     2D     ", nullptr));
#if QT_CONFIG(tooltip)
        coordTab->setTabToolTip(coordTab->indexOf(coordTab2D), QCoreApplication::translate("Control", "Select for 2D coordinatesystem", nullptr));
#endif // QT_CONFIG(tooltip)
        align->setTitle(QCoreApplication::translate("Control", "alignment grid", nullptr));
#if QT_CONFIG(tooltip)
        gridShow->setToolTip(QCoreApplication::translate("Control", "Show the grid.", nullptr));
#endif // QT_CONFIG(tooltip)
        gridShow->setText(QCoreApplication::translate("Control", "show", nullptr));
#if QT_CONFIG(tooltip)
        gridFix->setToolTip(QCoreApplication::translate("Control", "Fix the grid.", nullptr));
#endif // QT_CONFIG(tooltip)
        gridFix->setText(QCoreApplication::translate("Control", "fix", nullptr));
#if QT_CONFIG(tooltip)
        label_15->setToolTip(QCoreApplication::translate("Control", "Translate the grid in x-direction", nullptr));
#endif // QT_CONFIG(tooltip)
        label_15->setText(QCoreApplication::translate("Control", "translate [cm] x:", nullptr));
#if QT_CONFIG(tooltip)
        grid3DTransX->setToolTip(QCoreApplication::translate("Control", "Translate the grid in x-direction", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        grid3DTransX_spin->setToolTip(QCoreApplication::translate("Control", "Translate the grid in x-direction", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        label_49->setToolTip(QCoreApplication::translate("Control", "Translate the grid in y-direction.", nullptr));
#endif // QT_CONFIG(tooltip)
        label_49->setText(QCoreApplication::translate("Control", "y:", nullptr));
#if QT_CONFIG(tooltip)
        grid3DTransY->setToolTip(QCoreApplication::translate("Control", "Translate the grid in y-direction.", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        grid3DTransY_spin->setToolTip(QCoreApplication::translate("Control", "Translate the grid in y-direction.", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        label_50->setToolTip(QCoreApplication::translate("Control", "2D: Rotate the grid clockwise. 3D: Translate the grid in z-direction.", nullptr));
#endif // QT_CONFIG(tooltip)
        label_50->setText(QCoreApplication::translate("Control", "z:", nullptr));
#if QT_CONFIG(tooltip)
        grid3DTransZ->setToolTip(QCoreApplication::translate("Control", "2D: Rotate the grid clockwise. 3D: Translate the grid in z-direction.", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        grid3DTransZ_spin->setToolTip(QCoreApplication::translate("Control", "2D: Rotate the grid clockwise. 3D: Translate the grid in z-direction.", nullptr));
#endif // QT_CONFIG(tooltip)
        grid3DTransZ_spin->setSuffix(QString());
#if QT_CONFIG(tooltip)
        label_51->setToolTip(QCoreApplication::translate("Control", "Scale the gridcells", nullptr));
#endif // QT_CONFIG(tooltip)
        label_51->setText(QCoreApplication::translate("Control", "resolution [cm]:", nullptr));
#if QT_CONFIG(tooltip)
        grid3DResolution->setToolTip(QCoreApplication::translate("Control", "Scale the gridcells", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        grid3DResolution_spin->setToolTip(QCoreApplication::translate("Control", "Scale the gridcells", nullptr));
#endif // QT_CONFIG(tooltip)
        grid3DResolution_spin->setSuffix(QCoreApplication::translate("Control", " cm", nullptr));
        grid3DResolution_spin->setPrefix(QString());
        gridTab->setTabText(gridTab->indexOf(gridTab3D), QCoreApplication::translate("Control", "     3D     ", nullptr));
#if QT_CONFIG(tooltip)
        gridTab->setTabToolTip(gridTab->indexOf(gridTab3D), QCoreApplication::translate("Control", "Select for 3D grid", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        label_10->setToolTip(QCoreApplication::translate("Control", "Translate the grid in x-direction", nullptr));
#endif // QT_CONFIG(tooltip)
        label_10->setText(QCoreApplication::translate("Control", "translate x:", nullptr));
#if QT_CONFIG(tooltip)
        gridTransX->setToolTip(QCoreApplication::translate("Control", "Translate the grid in x-direction", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        gridTransX_spin->setToolTip(QCoreApplication::translate("Control", "Translate the grid in x-direction", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        label_11->setToolTip(QCoreApplication::translate("Control", "Translate the grid in y-direction.", nullptr));
#endif // QT_CONFIG(tooltip)
        label_11->setText(QCoreApplication::translate("Control", "y:", nullptr));
#if QT_CONFIG(tooltip)
        gridTransY->setToolTip(QCoreApplication::translate("Control", "Translate the grid in y-direction.", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        gridTransY_spin->setToolTip(QCoreApplication::translate("Control", "Translate the grid in y-direction.", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        label_9->setToolTip(QCoreApplication::translate("Control", "2D: Rotate the grid clockwise. 3D: Translate the grid in z-direction.", nullptr));
#endif // QT_CONFIG(tooltip)
        label_9->setText(QCoreApplication::translate("Control", "rotate:", nullptr));
#if QT_CONFIG(tooltip)
        gridRotate->setToolTip(QCoreApplication::translate("Control", "2D: Rotate the grid clockwise. 3D: Translate the grid in z-direction.", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        gridRot_spin->setToolTip(QCoreApplication::translate("Control", "2D: Rotate the grid clockwise. 3D: Translate the grid in z-direction.", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        label_12->setToolTip(QCoreApplication::translate("Control", "Scale the gridcells", nullptr));
#endif // QT_CONFIG(tooltip)
        label_12->setText(QCoreApplication::translate("Control", "scale:", nullptr));
#if QT_CONFIG(tooltip)
        gridScale->setToolTip(QCoreApplication::translate("Control", "Scale the gridcells", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        gridScale_spin->setToolTip(QCoreApplication::translate("Control", "Scale the gridcells", nullptr));
#endif // QT_CONFIG(tooltip)
        gridTab->setTabText(gridTab->indexOf(gridTab2D), QCoreApplication::translate("Control", "     2D     ", nullptr));
#if QT_CONFIG(tooltip)
        gridTab->setTabToolTip(gridTab->indexOf(gridTab2D), QCoreApplication::translate("Control", "Select for 2D grid", nullptr));
#endif // QT_CONFIG(tooltip)
        tabs->setTabText(tabs->indexOf(calib), QCoreApplication::translate("Control", "calibration", nullptr));
        performRecognition->setText(QCoreApplication::translate("Control", "perform", nullptr));
        label_27->setText(QCoreApplication::translate("Control", "step:", nullptr));
        label_26->setText(QCoreApplication::translate("Control", "number of recognized people by now:", nullptr));
        recoNumberNow->setText(QCoreApplication::translate("Control", "0", nullptr));
        recoStereoShow->setText(QCoreApplication::translate("Control", "parameter", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("Control", "region of interest", nullptr));
        roiShow->setText(QCoreApplication::translate("Control", "show", nullptr));
        roiFix->setText(QCoreApplication::translate("Control", "fix", nullptr));
        groupBox_5->setTitle(QCoreApplication::translate("Control", "marker", nullptr));
        label_19->setText(QCoreApplication::translate("Control", "marker brightness: ", nullptr));
        markerIgnoreWithout->setText(QCoreApplication::translate("Control", "ignore head without marker", nullptr));
        colorBox->setTitle(QCoreApplication::translate("Control", "size and color", nullptr));
        recoShowColor->setText(QCoreApplication::translate("Control", "show recognition color", nullptr));
        recoOptimizeColor->setText(QCoreApplication::translate("Control", "optimize", nullptr));
        label_28->setText(QCoreApplication::translate("Control", "model:", nullptr));
        recoAutoWB->setText(QCoreApplication::translate("Control", "auto white balance and brigtness", nullptr));
        label_29->setText(QCoreApplication::translate("Control", "x:", nullptr));
        label_30->setText(QCoreApplication::translate("Control", "y:", nullptr));
        label_31->setText(QCoreApplication::translate("Control", "z:", nullptr));
        label_38->setText(QCoreApplication::translate("Control", "grey level:", nullptr));
        label_39->setText(QCoreApplication::translate("Control", "symbol size:", nullptr));
        groupBox_4->setTitle(QCoreApplication::translate("Control", "map", nullptr));
        label_44->setText(QCoreApplication::translate("Control", "h:", nullptr));
        label_40->setText(QCoreApplication::translate("Control", "x:", nullptr));
        label_43->setText(QCoreApplication::translate("Control", "w:", nullptr));
        label_42->setText(QCoreApplication::translate("Control", "y:", nullptr));
        mapColor->setText(QCoreApplication::translate("Control", "color", nullptr));
        label_45->setText(QCoreApplication::translate("Control", "height:", nullptr));
        mapAdd->setText(QCoreApplication::translate("Control", "add", nullptr));
        mapDel->setText(QCoreApplication::translate("Control", "delete", nullptr));
        mapColorRange->setText(QCoreApplication::translate("Control", "color range", nullptr));
#if QT_CONFIG(tooltip)
        label_46->setToolTip(QCoreApplication::translate("Control", "default height, if no height is measured through color marker or disparity", nullptr));
#endif // QT_CONFIG(tooltip)
        label_46->setText(QCoreApplication::translate("Control", "def. height:", nullptr));
#if QT_CONFIG(tooltip)
        mapDistribution->setToolTip(QCoreApplication::translate("Control", "print out distribution of height", nullptr));
#endif // QT_CONFIG(tooltip)
        mapDistribution->setText(QCoreApplication::translate("Control", "dist.", nullptr));
#if QT_CONFIG(tooltip)
        mapResetHeight->setToolTip(QCoreApplication::translate("Control", "reset height of every person measured over time, so that default value will be choosen", nullptr));
#endif // QT_CONFIG(tooltip)
        mapResetHeight->setText(QCoreApplication::translate("Control", "res. height", nullptr));
#if QT_CONFIG(tooltip)
        mapResetPos->setToolTip(QCoreApplication::translate("Control", "reset position of every trackpoint (from disparity)", nullptr));
#endif // QT_CONFIG(tooltip)
        mapResetPos->setText(QCoreApplication::translate("Control", "res. pos.", nullptr));
        tabs->setTabText(tabs->indexOf(rec), QCoreApplication::translate("Control", "recognition", nullptr));
        trackOnlineCalc->setText(QCoreApplication::translate("Control", "online calculation", nullptr));
#if QT_CONFIG(shortcut)
        trackOnlineCalc->setShortcut(QCoreApplication::translate("Control", "Shift+T", nullptr));
#endif // QT_CONFIG(shortcut)
#if QT_CONFIG(tooltip)
        trackRepeat->setToolTip(QCoreApplication::translate("Control", "repeat tracking below existing quality of trackpoint ", nullptr));
#endif // QT_CONFIG(tooltip)
        trackRepeat->setText(QCoreApplication::translate("Control", "repeat below quality:", nullptr));
        trackExtrapolation->setText(QCoreApplication::translate("Control", "extrapolation for big diff.", nullptr));
        trackMerge->setText(QCoreApplication::translate("Control", "merge", nullptr));
#if QT_CONFIG(tooltip)
        trackOnlyVisible->setToolTip(QCoreApplication::translate("Control", "online tracking and manual deletion and moving only for trajectories, which are visable (see \"show only people\")", nullptr));
#endif // QT_CONFIG(tooltip)
        trackOnlyVisible->setText(QCoreApplication::translate("Control", "only visible", nullptr));
        trackNumberNow->setText(QCoreApplication::translate("Control", "0", nullptr));
        label_22->setText(QCoreApplication::translate("Control", "number of all tracked and recognized people:", nullptr));
        label_23->setText(QCoreApplication::translate("Control", "number of tracked people by now:", nullptr));
        trackNumberAll->setText(QCoreApplication::translate("Control", "0", nullptr));
        label_65->setText(QCoreApplication::translate("Control", "number of visible people:", nullptr));
        trackNumberVisible->setText(QCoreApplication::translate("Control", "0", nullptr));
        trackExport->setText(QCoreApplication::translate("Control", "export", nullptr));
        trackCalc->setText(QCoreApplication::translate("Control", "calculate all", nullptr));
        trackImport->setText(QCoreApplication::translate("Control", "import", nullptr));
        trackReset->setText(QCoreApplication::translate("Control", "reset", nullptr));
        groupBox_8->setTitle(QCoreApplication::translate("Control", "Region of interest", nullptr));
        trackRoiShow->setText(QCoreApplication::translate("Control", "show", nullptr));
        trackRoiFix->setText(QCoreApplication::translate("Control", "fix", nullptr));
        groupBox_7->setTitle(QCoreApplication::translate("Control", "export options", nullptr));
#if QT_CONFIG(tooltip)
        trackMissingFrames->setToolTip(QCoreApplication::translate("Control", "search and insert missing frames", nullptr));
#endif // QT_CONFIG(tooltip)
        trackMissingFrames->setText(QCoreApplication::translate("Control", "insert miss. frames", nullptr));
#if QT_CONFIG(tooltip)
        trackRecalcHeight->setToolTip(QCoreApplication::translate("Control", "recalculate median height", nullptr));
#endif // QT_CONFIG(tooltip)
        trackRecalcHeight->setText(QCoreApplication::translate("Control", "recalc. height", nullptr));
#if QT_CONFIG(tooltip)
        trackAlternateHeight->setToolTip(QCoreApplication::translate("Control", "allow alternate height", nullptr));
#endif // QT_CONFIG(tooltip)
        trackAlternateHeight->setText(QCoreApplication::translate("Control", "alt. height", nullptr));
#if QT_CONFIG(tooltip)
        exportElimTp->setToolTip(QCoreApplication::translate("Control", "eliminate trackpoints at outer margins without calculated height", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(statustip)
        exportElimTp->setStatusTip(QString());
#endif // QT_CONFIG(statustip)
        exportElimTp->setText(QCoreApplication::translate("Control", "elim. tp. wo. hgt.", nullptr));
#if QT_CONFIG(tooltip)
        exportElimTrj->setToolTip(QCoreApplication::translate("Control", "eliminate trajectories without calculated height", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(statustip)
        exportElimTrj->setStatusTip(QString());
#endif // QT_CONFIG(statustip)
        exportElimTrj->setText(QCoreApplication::translate("Control", "elim. trj. wo. hgt.", nullptr));
#if QT_CONFIG(tooltip)
        exportSmooth->setToolTip(QCoreApplication::translate("Control", "smooth trajectories (Attention: original trajectories are changed)", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(statustip)
        exportSmooth->setStatusTip(QString());
#endif // QT_CONFIG(statustip)
        exportSmooth->setText(QCoreApplication::translate("Control", "smooth", nullptr));
#if QT_CONFIG(tooltip)
        exportViewDir->setToolTip(QCoreApplication::translate("Control", "add direction of head (corresponding to view direction; possible with Japan and casern marker)", nullptr));
#endif // QT_CONFIG(tooltip)
        exportViewDir->setText(QCoreApplication::translate("Control", "add head direction", nullptr));
#if QT_CONFIG(tooltip)
        exportAngleOfView->setToolTip(QCoreApplication::translate("Control", "add angle of view of camera to person", nullptr));
#endif // QT_CONFIG(tooltip)
        exportAngleOfView->setText(QCoreApplication::translate("Control", "add angle of view", nullptr));
#if QT_CONFIG(tooltip)
        exportUseM->setToolTip(QCoreApplication::translate("Control", "use meter instead of cm", nullptr));
#endif // QT_CONFIG(tooltip)
        exportUseM->setText(QCoreApplication::translate("Control", "use meter", nullptr));
        exportComment->setText(QCoreApplication::translate("Control", "add comment", nullptr));
        exportMarkerID->setText(QCoreApplication::translate("Control", "add marker ID", nullptr));
        trackTest->setText(QCoreApplication::translate("Control", "test", nullptr));
#if QT_CONFIG(tooltip)
        testEqual->setToolTip(QCoreApplication::translate("Control", "search for trajectories with similar trackpoints", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(whatsthis)
        testEqual->setWhatsThis(QString());
#endif // QT_CONFIG(whatsthis)
        testEqual->setText(QCoreApplication::translate("Control", "equal", nullptr));
#if QT_CONFIG(tooltip)
        testVelocity->setToolTip(QCoreApplication::translate("Control", "search for hight velocity variations", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(whatsthis)
        testVelocity->setWhatsThis(QString());
#endif // QT_CONFIG(whatsthis)
        testVelocity->setText(QCoreApplication::translate("Control", "velocity", nullptr));
#if QT_CONFIG(tooltip)
        testInside->setToolTip(QCoreApplication::translate("Control", "search for trajectories which start or end outside recognition area", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(whatsthis)
        testInside->setWhatsThis(QString());
#endif // QT_CONFIG(whatsthis)
        testInside->setText(QCoreApplication::translate("Control", "inside", nullptr));
#if QT_CONFIG(tooltip)
        testLength->setToolTip(QCoreApplication::translate("Control", "search for short trajectories", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(whatsthis)
        testLength->setWhatsThis(QString());
#endif // QT_CONFIG(whatsthis)
        testLength->setText(QCoreApplication::translate("Control", "length", nullptr));
        groupBox_6->setTitle(QCoreApplication::translate("Control", "search region", nullptr));
        label_52->setText(QCoreApplication::translate("Control", "scale:", nullptr));
        label_53->setText(QCoreApplication::translate("Control", "levels:", nullptr));
        label_66->setText(QCoreApplication::translate("Control", "max. error:", nullptr));
        trackShowSearchSize->setText(QCoreApplication::translate("Control", "show pyramidal search size", nullptr));
        groupBox_3->setTitle(QCoreApplication::translate("Control", "path", nullptr));
        trackShow->setText(QCoreApplication::translate("Control", "show", nullptr));
        trackFix->setText(QCoreApplication::translate("Control", "fix", nullptr));
        trackShowOnlyVisible->setText(QCoreApplication::translate("Control", "only for visible people", nullptr));
        trackShowOnly->setText(QCoreApplication::translate("Control", "show only people:", nullptr));
        trackGotoNr->setText(QCoreApplication::translate("Control", "goto", nullptr));
        trackGotoStartNr->setText(QCoreApplication::translate("Control", "start", nullptr));
        trackGotoEndNr->setText(QCoreApplication::translate("Control", "end", nullptr));
        trackShowOnlyList->setText(QCoreApplication::translate("Control", "show only people list: ", nullptr));
        trackShowOnlyListButton->setText(QCoreApplication::translate("Control", "list", nullptr));
        trackShowPath->setText(QCoreApplication::translate("Control", "show path", nullptr));
        trackShowCurrentPoint->setText(QCoreApplication::translate("Control", "show current point", nullptr));
        label_25->setText(QCoreApplication::translate("Control", "size:", nullptr));
        label_36->setText(QCoreApplication::translate("Control", "size:", nullptr));
        trackShowColColor->setText(QCoreApplication::translate("Control", "show height/col. color", nullptr));
        trackShowColorMarker->setText(QCoreApplication::translate("Control", "show color marker", nullptr));
        trackShowNumber->setText(QCoreApplication::translate("Control", "show number", nullptr));
        label_48->setText(QCoreApplication::translate("Control", "size:", nullptr));
        trackNumberBold->setText(QCoreApplication::translate("Control", "bold", nullptr));
        trackHeadSized->setText(QCoreApplication::translate("Control", "head size", nullptr));
        trackShowHeightIndividual->setText(QCoreApplication::translate("Control", "individual", nullptr));
        trackPathColorButton->setText(QCoreApplication::translate("Control", "color", nullptr));
        trackShowPoints->setText(QCoreApplication::translate("Control", "show points", nullptr));
        trackShowPointsColored->setText(QCoreApplication::translate("Control", "colored", nullptr));
        label_13->setText(QCoreApplication::translate("Control", "size:", nullptr));
        label_41->setText(QCoreApplication::translate("Control", "size:", nullptr));
        label_47->setText(QCoreApplication::translate("Control", "size:", nullptr));
        trackShowGroundPosition->setText(QCoreApplication::translate("Control", "show ground position", nullptr));
        label_67->setText(QCoreApplication::translate("Control", "size:", nullptr));
        trackShowGroundPath->setText(QCoreApplication::translate("Control", "show ground path", nullptr));
        label_68->setText(QCoreApplication::translate("Control", "size:", nullptr));
        trackGroundPathColorButton->setText(QCoreApplication::translate("Control", "color", nullptr));
        label_17->setText(QCoreApplication::translate("Control", "frames before actual position:", nullptr));
        label_20->setText(QCoreApplication::translate("Control", "frames after actual position:", nullptr));
        tabs->setTabText(tabs->indexOf(track), QCoreApplication::translate("Control", "tracking", nullptr));
        anaCalculate->setText(QCoreApplication::translate("Control", "calculate", nullptr));
        anaMissingFrames->setText(QCoreApplication::translate("Control", "search and  insert missing frames", nullptr));
        label_21->setText(QCoreApplication::translate("Control", "averaging step size:", nullptr));
        anaMarkAct->setText(QCoreApplication::translate("Control", "mark actual frame", nullptr));
        label_16->setText(QCoreApplication::translate("Control", "consider:", nullptr));
        anaConsiderX->setText(QCoreApplication::translate("Control", "x", nullptr));
        anaConsiderY->setText(QCoreApplication::translate("Control", "y", nullptr));
        anaConsiderAbs->setText(QCoreApplication::translate("Control", "absolute", nullptr));
        anaConsiderRev->setText(QCoreApplication::translate("Control", "reverse", nullptr));
        showVoronoiCells->setText(QCoreApplication::translate("Control", "show voronoi cells", nullptr));
        tabs->setTabText(tabs->indexOf(ana), QCoreApplication::translate("Control", "analysis", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Control: public Ui_Control {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CONTROL_H
