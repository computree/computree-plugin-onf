#include "onf_actionaggregateitemsoptions.h"
#include "ui_onf_actionaggregateitemsoptions.h"

#include "actions/onf_actionaggregateitems.h"

#include <QColorDialog>

ONF_ActionAggregateItemsOptions::ONF_ActionAggregateItemsOptions(const ONF_ActionAggregateItems *action) :
    CT_GAbstractActionOptions(action),
    ui(new Ui::ONF_ActionAggregateItemsOptions())
{
    ui->setupUi(this);

    connect(&_btGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(changeCurrentModality(QAbstractButton*)));
}

ONF_ActionAggregateItemsOptions::~ONF_ActionAggregateItemsOptions()
{
    delete ui;
}

void ONF_ActionAggregateItemsOptions::addModality(QString modality)
{
    QPushButton* button = new QPushButton(modality, ui->wid_modalities);
    _btGroup.addButton(button);
}

void ONF_ActionAggregateItemsOptions::changeCurrentModality (QAbstractButton* button)
{
    if (button == NULL)
    {
        emit currentModalityChanged("");
    } else {
        emit currentModalityChanged(button->text());
    }
}

void ONF_ActionAggregateItemsOptions::on_pb_items_clicked()
{

}
