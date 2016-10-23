#ifndef ONF_ACTIONAGGREGATEITEMSOPTIONSSELECTIONDIALOG_H
#define ONF_ACTIONAGGREGATEITEMSOPTIONSSELECTIONDIALOG_H

#include "ct_itemdrawable/abstract/ct_abstractsingularitemdrawable.h"

#include <QDialog>
#include <QCheckBox>
#include <QLabel>


namespace Ui {
class ONF_ActionAggregateItemsOptionsSelectionDialog;
}

class ONF_ActionAggregateItemsOptionsSelectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ONF_ActionAggregateItemsOptionsSelectionDialog(QList<CT_AbstractSingularItemDrawable*> &items,
                                                            QList<QString> &modalities,
                                                            QWidget *parent = 0);
    ~ONF_ActionAggregateItemsOptionsSelectionDialog();

    void updateModalities(const QList<QString> &modalities);


signals:
    void visibleItemsChanged();


private slots:
    void checkBoxesChanged(bool checked);

private:
    Ui::ONF_ActionAggregateItemsOptionsSelectionDialog *ui;

    QList<CT_AbstractSingularItemDrawable*>* _items;
    QList<QString>*                          _modalities;
    QList<QCheckBox*>                        _checkBoxes;
    QList<QLabel*>                           _labelsMod;
    QList<bool>                              _activeItems;
};

#endif // ONF_ACTIONAGGREGATEITEMSOPTIONSSELECTIONDIALOG_H
