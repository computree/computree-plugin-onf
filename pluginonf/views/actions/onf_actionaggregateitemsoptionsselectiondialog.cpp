#include "onf_actionaggregateitemsoptionsselectiondialog.h"
#include "ui_onf_actionaggregateitemsoptionsselectiondialog.h"

#include <QGridLayout>

ONF_ActionAggregateItemsOptionsSelectionDialog::ONF_ActionAggregateItemsOptionsSelectionDialog(QList<CT_AbstractSingularItemDrawable*> &items,
                                                                                               QList<QString> &modalities,
                                                                                               QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ONF_ActionAggregateItemsOptionsSelectionDialog)
{
    ui->setupUi(this);

    _items = &items;
    _modalities = &modalities;

    QGridLayout* layout = (QGridLayout*) ui->wid_itemsModalities->layout();
    for (int i = 0 ; i < _items->size() ; i++)
    {
        if (i == 0)
        {
            _activeItems.append(true);
        } else {
            _activeItems.append(false);
        }

        QCheckBox* cb = new QCheckBox(items.at(i)->name());
        QLabel* lb = new QLabel(_modalities.at(i));
        _checkBoxes.append(cb);
        _labelsMod.append(lb);
        layout->addWidget(cb, 0, 2 + i);
        layout->addWidget(lb, 1, 2 + i);

        connect(cb, SIGNAL(toggled(bool)), this, SLOT(checkBoxesChanged(bool)));
    }
}

ONF_ActionAggregateItemsOptionsSelectionDialog::updateModalities(QList<QString> &modalities)
{
    for (int i = 0 ; i < _labelsMod->size() ; i++)
    {
        _labelsMod[i]->setText(modalities.at(i));
    }
}

void ONF_ActionAggregateItemsOptionsSelectionDialog::checkBoxesChanged(bool checked)
{
    Q_UNUSED(checked);

    for (int i = 0 ; i < _checkBoxes->size() ; i++)
    {
        _activeItems[i] = _checkBoxes.at(i)->isChecked();
    }
    emit visibleItemsChanged();
}

ONF_ActionAggregateItemsOptionsSelectionDialog::~ONF_ActionAggregateItemsOptionsSelectionDialog()
{
    delete ui;
}
