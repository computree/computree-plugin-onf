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

#include "onf_actionselectcellsingrid3dcolonizedialog.h"
#include "ui_onf_actionselectcellsingrid3dcolonizedialog.h"

ONF_ActionSelectCellsInGrid3DColonizeDialog::ONF_ActionSelectCellsInGrid3DColonizeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ONF_ActionSelectCellsInGrid3DColonizeDialog)
{
    ui->setupUi(this);
}

ONF_ActionSelectCellsInGrid3DColonizeDialog::~ONF_ActionSelectCellsInGrid3DColonizeDialog()
{
    delete ui;
}

void ONF_ActionSelectCellsInGrid3DColonizeDialog::setXp(bool val)
{
    ui->cb_xp->setChecked(val);
}

void ONF_ActionSelectCellsInGrid3DColonizeDialog::setXm(bool val)
{
    ui->cb_xm->setChecked(val);
}

void ONF_ActionSelectCellsInGrid3DColonizeDialog::setYp(bool val)
{
    ui->cb_yp->setChecked(val);
}

void ONF_ActionSelectCellsInGrid3DColonizeDialog::setYm(bool val)
{
    ui->cb_ym->setChecked(val);
}

void ONF_ActionSelectCellsInGrid3DColonizeDialog::setZp(bool val)
{
    ui->cb_zp->setChecked(val);
}

void ONF_ActionSelectCellsInGrid3DColonizeDialog::setZm(bool val)
{
    ui->cb_zm->setChecked(val);
}

bool ONF_ActionSelectCellsInGrid3DColonizeDialog::getXp()
{
    return ui->cb_xp->isChecked();
}

bool ONF_ActionSelectCellsInGrid3DColonizeDialog::getXm()
{
    return ui->cb_xm->isChecked();
}

bool ONF_ActionSelectCellsInGrid3DColonizeDialog::getYp()
{
    return ui->cb_yp->isChecked();
}

bool ONF_ActionSelectCellsInGrid3DColonizeDialog::getYm()
{
    return ui->cb_ym->isChecked();
}
bool ONF_ActionSelectCellsInGrid3DColonizeDialog::getZp()
{
    return ui->cb_zp->isChecked();
}

bool ONF_ActionSelectCellsInGrid3DColonizeDialog::getZm()
{
    return ui->cb_zm->isChecked();
}
