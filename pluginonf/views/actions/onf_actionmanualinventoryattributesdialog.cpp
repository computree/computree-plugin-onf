/****************************************************************************
 Copyright (C) 2010-2012 the Office National des Forêts (ONF), France
                         All rights reserved.

 Contact : alexandre.piboule@onf.fr

 Developers : Alexandre PIBOULE (ONF)

 This file is part of PluginONF library.

 PluginONF is free library: you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 PluginONF is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with PluginONF.  If not, see <http://www.gnu.org/licenses/lgpl.html>.
*****************************************************************************/

#include "onf_actionmanualinventoryattributesdialog.h"
#include "ui_onf_actionmanualinventoryattributesdialog.h"

#include <QLabel>
#include <QComboBox>
#include <QLineEdit>

ONF_ActionManualInventoryAttributesDialog::ONF_ActionManualInventoryAttributesDialog(QMap<QString, QStringList> *paramData,
                                                                                   QMap<QString, QString> &attrValues,
                                                                                   QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ONF_ActionManualInventoryAttributesDialog)
{
    ui->setupUi(this);

    _paramData = paramData;
    _attrValues = &attrValues;

    int cpt = 2;
    QMapIterator<QString, QStringList> it(*paramData);
    while (it.hasNext())
    {
        it.next();
        const QString &name = it.key();
        const QStringList &list = it.value();

        const QString &value = _attrValues->value(name);

        QWidget *wid = NULL;

        if (list.size() > 0)
        {
            wid = new QComboBox(ui->wid_attr);
            ((QComboBox*) wid)->addItems(list);

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
            ((QComboBox*) wid)->setCurrentIndex(((QComboBox*) wid)->findText(value););
#else
            ((QComboBox*) wid)->setCurrentText(value);
#endif

        } else {
            wid = new QLineEdit(value, ui->wid_attr);
        }

        _widgets.insert(name, wid);

        ((QGridLayout*) ui->wid_attr->layout())->addWidget(new QLabel(name, ui->wid_attr), cpt, 0);
        ((QGridLayout*) ui->wid_attr->layout())->addWidget(wid, cpt++, 1);

        if (cpt == 3) {wid->setFocus();}
    }
}

ONF_ActionManualInventoryAttributesDialog::~ONF_ActionManualInventoryAttributesDialog()
{
    delete ui;
}

QString ONF_ActionManualInventoryAttributesDialog::getValueForAttr(QString name) const
{
    QWidget* wid = _widgets.value(name);
    if (wid == NULL) {return "";}

    QComboBox* cb = dynamic_cast<QComboBox*>(wid);
    if (cb != NULL)
    {
        return cb->currentText();
    } else {
        QLineEdit* le = dynamic_cast<QLineEdit*>(wid);
        if (le != NULL)
        {
            return le->text();
        }
    }
    return "";
}

