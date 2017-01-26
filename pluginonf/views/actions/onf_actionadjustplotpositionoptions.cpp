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

#include "onf_actionadjustplotpositionoptions.h"
#include "ui_onf_actionadjustplotpositionoptions.h"

#include "actions/onf_actionadjustplotposition.h"

#include <QColorDialog>

ONF_ActionAdjustPlotPositionOptions::ONF_ActionAdjustPlotPositionOptions(const ONF_ActionAdjustPlotPosition *action) :
    CT_GAbstractActionForGraphicsViewOptions(action),
    ui(new Ui::ONF_ActionAdjustPlotPositionOptions())
{
    ui->setupUi(this);
}

ONF_ActionAdjustPlotPositionOptions::~ONF_ActionAdjustPlotPositionOptions()
{
    delete ui;
}



void ONF_ActionAdjustPlotPositionOptions::on_pb_up_clicked()
{
    emit parametersChanged(0, ui->dsb_increment->value(),
                           ui->cb_circles->isChecked(),
                           ui->cb_fixedH->isChecked(),
                           ui->dsb_height->value());
}

void ONF_ActionAdjustPlotPositionOptions::on_pb_down_clicked()
{
    emit parametersChanged(0, -1.0*ui->dsb_increment->value(),
                           ui->cb_circles->isChecked(),
                           ui->cb_fixedH->isChecked(),
                           ui->dsb_height->value());
}

void ONF_ActionAdjustPlotPositionOptions::on_pb_left_clicked()
{
    emit parametersChanged(-1.0*ui->dsb_increment->value(), 0,
                           ui->cb_circles->isChecked(),
                           ui->cb_fixedH->isChecked(),
                           ui->dsb_height->value());
}

void ONF_ActionAdjustPlotPositionOptions::on_pb_right_clicked()
{
    emit parametersChanged(ui->dsb_increment->value(), 0,
                           ui->cb_circles->isChecked(),
                           ui->cb_fixedH->isChecked(),
                           ui->dsb_height->value());
}

void ONF_ActionAdjustPlotPositionOptions::on_cb_circles_toggled(bool checked)
{
    emit parametersChanged(0, 0,
                           ui->cb_circles->isChecked(),
                           ui->cb_fixedH->isChecked(),
                           ui->dsb_height->value());

}

void ONF_ActionAdjustPlotPositionOptions::on_cb_fixedH_toggled(bool checked)
{
    emit parametersChanged(0, 0,
                           ui->cb_circles->isChecked(),
                           ui->cb_fixedH->isChecked(),
                           ui->dsb_height->value());

}

void ONF_ActionAdjustPlotPositionOptions::on_dsb_height_valueChanged(double arg1)
{
    emit parametersChanged(0, 0,
                           ui->cb_circles->isChecked(),
                           ui->cb_fixedH->isChecked(),
                           ui->dsb_height->value());

}

void ONF_ActionAdjustPlotPositionOptions::on_cb_intensity_toggled(bool checked)
{
    emit colorizationChanged(ui->cb_intensity->isChecked(), ui->hs_min->value(), ui->hs_max->value());
}

void ONF_ActionAdjustPlotPositionOptions::on_hs_min_valueChanged(int value)
{
    emit colorizationChanged(ui->cb_intensity->isChecked(), ui->hs_min->value(), ui->hs_max->value());
}

void ONF_ActionAdjustPlotPositionOptions::on_hs_max_valueChanged(int value)
{
    emit colorizationChanged(ui->cb_intensity->isChecked(), ui->hs_min->value(), ui->hs_max->value());
}