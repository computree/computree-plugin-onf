#ifndef ONF_ACTIONAGGREGATEITEMSOPTIONS_H
#define ONF_ACTIONAGGREGATEITEMSOPTIONS_H

#include "ct_view/actions/abstract/ct_gabstractactionoptions.h"

#include <QPushButton>
#include <QButtonGroup>

class ONF_ActionAggregateItems;

namespace Ui {
class ONF_ActionAggregateItemsOptions;
}

class ONF_ActionAggregateItemsOptions : public CT_GAbstractActionOptions
{
    Q_OBJECT

public:

    explicit ONF_ActionAggregateItemsOptions(const ONF_ActionAggregateItems *action);
    ~ONF_ActionAggregateItemsOptions();

    void addModality(QString modality);

private:
    Ui::ONF_ActionAggregateItemsOptions *ui;

    QButtonGroup        _btGroup;

signals:
    void currentModalityChanged(QString modality);

private slots:
    void changeCurrentModality(QAbstractButton *button);
    void on_pb_items_clicked();
};

#endif // ONF_ACTIONAGGREGATEITEMSOPTIONS_H
