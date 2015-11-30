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

#include "onf_actionmodifypositions2doptions.h"
#include "ui_onf_actionmodifypositions2doptions.h"

#include "actions/onf_actionmodifypositions2d.h"

#include <QColorDialog>

ONF_ActionModifyPositions2DOptions::ONF_ActionModifyPositions2DOptions(const ONF_ActionModifyPositions2D *action) :
    CT_GAbstractActionOptions(action),
    ui(new Ui::ONF_ActionModifyPositions2DOptions())
{
    ui->setupUi(this);

    ui->pb_freeMove->setToolTip(tr("Déplacement normal dans la vue [F]"));
    ui->pb_move->setToolTip(tr("Modification d'une position [D]"));
    ui->pb_add->setToolTip(tr("Ajout d'une position [A]"));
    ui->pb_remove->setToolTip(tr("Suppression d'une position [S]"));

    connect(ui->buttonGroup_step, SIGNAL(buttonClicked(int)), this, SLOT(singleStepChanged(int)));
}

ONF_ActionModifyPositions2DOptions::~ONF_ActionModifyPositions2DOptions()
{
    delete ui;
}

bool ONF_ActionModifyPositions2DOptions::isFreeMoveSelected()
{
    return ui->pb_freeMove->isChecked();
}

bool ONF_ActionModifyPositions2DOptions::isMovePositionSelected()
{
    return ui->pb_move->isChecked();
}

bool ONF_ActionModifyPositions2DOptions::isAddPositionSelected()
{
    return ui->pb_add->isChecked();
}

bool ONF_ActionModifyPositions2DOptions::isRemovePositionSelected()
{
    return ui->pb_remove->isChecked();
}

double ONF_ActionModifyPositions2DOptions::getZValue()
{
    return ui->dsb_zval->value();
}

bool ONF_ActionModifyPositions2DOptions::isDrawPlaneSelected()
{
    return ui->cb_drawPlane->isChecked();
}

bool ONF_ActionModifyPositions2DOptions::isDrawLinesSelected()
{
    return ui->cb_drawLines->isChecked();
}

bool ONF_ActionModifyPositions2DOptions::isUpdateLinesSelected()
{
    return ui->cb_updateLines->isChecked();
}

void ONF_ActionModifyPositions2DOptions::selectFreeMove()
{
    ui->pb_freeMove->setChecked(true);
}

void ONF_ActionModifyPositions2DOptions::selectMovePosition()
{
    ui->pb_move->setChecked(true);
}

void ONF_ActionModifyPositions2DOptions::selectAddPosition()
{
    ui->pb_add->setChecked(true);
}

void ONF_ActionModifyPositions2DOptions::selectRemovePosition()
{
    ui->pb_remove->setChecked(true);
}

void ONF_ActionModifyPositions2DOptions::decreaseZValue()
{
    ui->dsb_zval->stepDown();
}

void ONF_ActionModifyPositions2DOptions::increaseZValue()
{
    ui->dsb_zval->stepUp();
}

void ONF_ActionModifyPositions2DOptions::on_pb_freeMove_clicked()
{

}

void ONF_ActionModifyPositions2DOptions::on_pb_move_clicked()
{

}

void ONF_ActionModifyPositions2DOptions::on_pb_add_clicked()
{

}

void ONF_ActionModifyPositions2DOptions::on_pb_remove_clicked()
{

}

void ONF_ActionModifyPositions2DOptions::on_dsb_zval_valueChanged(double arg1)
{
    emit parametersChanged();
}

void ONF_ActionModifyPositions2DOptions::singleStepChanged(int button)
{
    Q_UNUSED(button);

    if (ui->rb_1m->isChecked())
    {
        ui->dsb_zval->setSingleStep(1.0);
    } else if (ui->rb_10cm->isChecked())
    {
        ui->dsb_zval->setSingleStep(0.10);
    } else  {
        ui->dsb_zval->setSingleStep(0.01);
    }
}

void ONF_ActionModifyPositions2DOptions::on_cb_drawPlane_toggled(bool checked)
{
    emit parametersChanged();
}

void ONF_ActionModifyPositions2DOptions::on_cb_drawLines_toggled(bool checked)
{
    emit parametersChanged();
}
