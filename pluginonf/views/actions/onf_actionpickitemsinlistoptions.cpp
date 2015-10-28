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

#include "onf_actionpickitemsinlistoptions.h"
#include "ui_onf_actionpickitemsinlistoptions.h"

#include "actions/onf_actionpickitemsinlist.h"

#include <QColorDialog>

ONF_ActionPickItemsInListOptions::ONF_ActionPickItemsInListOptions(const ONF_ActionPickItemsInList *action) :
    CT_GAbstractActionOptions(action),
    ui(new Ui::ONF_ActionPickItemsInListOptions())
{
    ui->setupUi(this);
    _itemColor = QColor(0, 250, 0);
    ui->pb_color->setStyleSheet("QPushButton { background-color: " + _itemColor.name() + "; }");
}

ONF_ActionPickItemsInListOptions::~ONF_ActionPickItemsInListOptions()
{
    delete ui;
}

QColor ONF_ActionPickItemsInListOptions::getColor()
{
    return _itemColor;
}

float ONF_ActionPickItemsInListOptions::getMaxDist()
{
    return (float) ui->dsb_maxDist->value();
}

bool ONF_ActionPickItemsInListOptions::souldAutoCenterCamera()
{
    return ui->cb_centerCamera->isChecked();
}

void ONF_ActionPickItemsInListOptions::on_onf_color_clicked()
{
    _itemColor = QColorDialog::getColor(_itemColor);
    if (!_itemColor.isValid()) {_itemColor = QColor(0, 250, 0);}
    ui->pb_color->setStyleSheet("QPushButton { background-color: " + _itemColor.name() + "; }");

    emit parametersChanged();
}


void ONF_ActionPickItemsInListOptions::on_dsb_maxDist_valueChanged(double arg1)
{
    emit parametersChanged();
}

void ONF_ActionPickItemsInListOptions::on_onf_all_clicked()
{
    emit allClicked();
}

void ONF_ActionPickItemsInListOptions::on_onf_none_clicked()
{
    emit noneClicked();
}
