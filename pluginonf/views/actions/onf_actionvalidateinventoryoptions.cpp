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

#include "onf_actionvalidateinventoryoptions.h"
#include "ui_onf_actionvalidateinventoryoptions.h"

#include "actions/onf_actionvalidateinventory.h"

#include <QColorDialog>

ONF_ActionValidateInventoryOptions::ONF_ActionValidateInventoryOptions(const ONF_ActionValidateInventory *action) :
    CT_GAbstractActionOptions(action),
    ui(new Ui::ONF_ActionValidateInventoryOptions())
{
    ui->setupUi(this);

    connect(ui->pb_reset, SIGNAL(clicked()), this, SIGNAL(askForCurrentItemReset()));
}

ONF_ActionValidateInventoryOptions::~ONF_ActionValidateInventoryOptions()
{
    delete ui;
}

bool ONF_ActionValidateInventoryOptions::shouldAutoCenterCamera()
{
    return ui->cb_centerCamera->isChecked();
}

bool ONF_ActionValidateInventoryOptions::shouldShowData()
{
    return ui->cb_showData->isChecked();
}

bool ONF_ActionValidateInventoryOptions::shouldShowCandidateItems()
{
    return ui->cb_showCandidateItems->isChecked();
}

ONF_ActionValidateInventoryOptions::Mode ONF_ActionValidateInventoryOptions::getMode()
{
    if (ui->rb_attributes->isChecked())
    {
        return Mode_attributes;
    }
    return Mode_Item;
}

void ONF_ActionValidateInventoryOptions::toggleMode()
{
    if (ui->rb_dbh->isChecked())
    {
        ui->rb_attributes->setChecked(true);
    } else {
        ui->rb_dbh->setChecked(true);
    }
}

void ONF_ActionValidateInventoryOptions::on_rb_dbh_toggled(bool checked)
{
    Q_UNUSED(checked);
    emit parametersChanged();
}

void ONF_ActionValidateInventoryOptions::on_cb_showData_toggled(bool checked)
{
    Q_UNUSED(checked);
    emit parametersChanged();
}



void ONF_ActionValidateInventoryOptions::on_cb_showCandidateItems_toggled(bool checked)
{
    Q_UNUSED(checked);
    emit parametersChanged();
}

