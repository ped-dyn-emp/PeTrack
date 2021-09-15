#ifndef MOCAPSELECTIONWIDGET_H
#define MOCAPSELECTIONWIDGET_H

#include "moCapPersonMetadata.h"

#include <QWidget>

namespace Ui
{
class MoCapSelectionWidget;
}

class OpenMoCapDialog;

class MoCapSelectionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MoCapSelectionWidget(QWidget *parent, const QMap<QString, MoCapSystem> &moCapSystems);
    explicit MoCapSelectionWidget(
        QWidget *                         parent,
        const QMap<QString, MoCapSystem> &moCapSystems,
        const MoCapPersonMetadata &       metadata);
    MoCapSelectionWidget(const MoCapSelectionWidget &) = delete;
    MoCapSelectionWidget(MoCapSelectionWidget &&)      = delete;
    MoCapSelectionWidget &operator=(const MoCapSelectionWidget &) = delete;
    MoCapSelectionWidget &operator=(MoCapSelectionWidget &&) = delete;
    ~MoCapSelectionWidget() override;

    void                setFileName();
    void                setFileName(QString filename);
    MoCapPersonMetadata getMetadata() const;

    bool isFilledOut() const;

private:
    Ui::MoCapSelectionWidget *        mUi;
    const QMap<QString, MoCapSystem> &mMoCapSystems;
    bool                              mFilledOut = false;
};

#endif // MOCAPSELECTIONWIDGET_H
