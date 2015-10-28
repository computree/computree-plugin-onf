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

#include "onf_actionvalidateinventoryattributesdialog.h"
#include "ui_onf_actionvalidateinventoryattributesdialog.h"

ONF_ActionValidateInventoryAttributesDialog::ONF_ActionValidateInventoryAttributesDialog(const QStringList &speciesList, const QString &speciesValue, const QString &idValue, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ONF_ActionValidateInventoryAttributesDialog)
{
    ui->setupUi(this);

    ui->cb_species->addItems(speciesList);

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
        ui->cb_species->setCurrentIndex(ui->cb_species->findText(speciesValue));
#else
        ui->cb_species->setCurrentText(speciesValue);
#endif

    ui->le_id->setText(idValue);
}

ONF_ActionValidateInventoryAttributesDialog::~ONF_ActionValidateInventoryAttributesDialog()
{
    delete ui;
}

QString ONF_ActionValidateInventoryAttributesDialog::getSpecies()
{
    return ui->cb_species->currentText();
}

QString ONF_ActionValidateInventoryAttributesDialog::getId()
{
    return ui->le_id->text();
}
