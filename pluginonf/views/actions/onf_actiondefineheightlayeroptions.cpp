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

#include "onf_actiondefineheightlayeroptions.h"
#include "ui_onf_actiondefineheightlayeroptions.h"

#include "ct_global/ct_context.h"

#include "actions/onf_actiondefineheightlayer.h"

ONF_ActionDefineHeightLayerOptions::ONF_ActionDefineHeightLayerOptions(const ONF_ActionDefineHeightLayer *action, float zmin, float zmax, float res, int threshold) :
    CT_GAbstractActionOptions(action),
    ui(new Ui::ONF_ActionDefineHeightLayerOptions)
{
    ui->setupUi(this);

    ui->dsb_resolution->setValue(res);
    ui->sb_threshold->setValue(threshold);

    ui->rb_m->setChecked(true);
    ui->rb_zmin->setChecked(true);


    ui->dsb_zmin->setMinimum(zmin);
    ui->dsb_zmax->setMinimum(zmin);

    ui->dsb_zmin->setMaximum(zmax);
    ui->dsb_zmax->setMaximum(zmax);

    ui->dsb_zmin->setValue(zmin);
    ui->dsb_zmax->setValue(zmax);

    emit zValuesChanged(zmin, zmax);

    PS_LOG->addMessage(LogInterface::info, LogInterface::action, tr("Zmin= %1").arg(zmin));
    PS_LOG->addMessage(LogInterface::info, LogInterface::action, tr("Zmax= %1").arg(zmax));

    connect(ui->bg_singleStep, SIGNAL(buttonClicked(int)), this, SLOT(changeSingleStep(int)));
    connect(ui->pb_plus, SIGNAL(clicked()), this, SLOT(increaseHValue()));
    connect(ui->pb_minus, SIGNAL(clicked()), this, SLOT(decreaseHValue()));
    connect(ui->pb_sideView, SIGNAL(clicked()), this, SIGNAL(askForSideView()));
    connect(ui->dsb_resolution, SIGNAL(valueChanged(double)), this, SIGNAL(updateResolution(double)));
    connect(ui->sb_threshold, SIGNAL(valueChanged(int)), this, SIGNAL(updateThreshold(int)));

    ui->pb_sideView->hide();
}

ONF_ActionDefineHeightLayerOptions::~ONF_ActionDefineHeightLayerOptions()
{
    delete ui;
}

double ONF_ActionDefineHeightLayerOptions::getZmin()
{
    return ui->dsb_zmin->value();
}

double ONF_ActionDefineHeightLayerOptions::getZmax()
{
    return ui->dsb_zmax->value();
}

void ONF_ActionDefineHeightLayerOptions::chooseZmin()
{
    ui->rb_zmin->setChecked(true);
}

void ONF_ActionDefineHeightLayerOptions::chooseZmax()
{
    ui->rb_zmax->setChecked(true);
}

int ONF_ActionDefineHeightLayerOptions::getThreshold()
{
    return ui->sb_threshold->value();
}

double ONF_ActionDefineHeightLayerOptions::getResolution()
{
    return ui->dsb_resolution->value();
}

void ONF_ActionDefineHeightLayerOptions::changeSingleStep(int button)
{
    Q_UNUSED(button);
    if (ui->rb_m->isChecked()) {ui->dsb_zmin->setSingleStep(1);ui->dsb_zmax->setSingleStep(1);}
    else if (ui->rb_dm->isChecked()) {ui->dsb_zmin->setSingleStep(0.1);ui->dsb_zmax->setSingleStep(0.1);}
    else if (ui->rb_cm->isChecked()) {ui->dsb_zmin->setSingleStep(0.01);ui->dsb_zmax->setSingleStep(0.01);}
    else if (ui->rb_mm->isChecked()) {ui->dsb_zmin->setSingleStep(0.001);ui->dsb_zmax->setSingleStep(0.001);}
}

void ONF_ActionDefineHeightLayerOptions::increaseHValue()
{
    if (ui->rb_zmin->isChecked())
    {
        ui->dsb_zmin->setValue(ui->dsb_zmin->value() + ui->dsb_zmin->singleStep());
    } else {
        ui->dsb_zmax->setValue(ui->dsb_zmax->value() + ui->dsb_zmax->singleStep());
    }
    emit zValuesChanged(ui->dsb_zmin->value(), ui->dsb_zmax->value());
    emit redrawNeeded();
}

void ONF_ActionDefineHeightLayerOptions::decreaseHValue()
{
    if (ui->rb_zmin->isChecked())
    {
        ui->dsb_zmin->setValue(ui->dsb_zmin->value() - ui->dsb_zmin->singleStep());
    } else {
        ui->dsb_zmax->setValue(ui->dsb_zmax->value() - ui->dsb_zmax->singleStep());
    }
    emit zValuesChanged(ui->dsb_zmin->value(), ui->dsb_zmax->value());
    emit redrawNeeded();
}

void ONF_ActionDefineHeightLayerOptions::increaseSingleStep()
{
    if (ui->rb_mm->isChecked()) {ui->rb_cm->setChecked(true);}
    else if (ui->rb_cm->isChecked()) {ui->rb_dm->setChecked(true);}
    else if (ui->rb_dm->isChecked()) {ui->rb_m->setChecked(true);}

    changeSingleStep(0);
}

void ONF_ActionDefineHeightLayerOptions::decreaseSingleStep()
{
    if (ui->rb_m->isChecked()) {ui->rb_dm->setChecked(true);}
    else if (ui->rb_dm->isChecked()) {ui->rb_cm->setChecked(true);}
    else if (ui->rb_cm->isChecked()) {ui->rb_mm->setChecked(true);}

    changeSingleStep(0);
}

void ONF_ActionDefineHeightLayerOptions::on_dsb_zmin_valueChanged(double arg1)
{
    ui->dsb_zmax->setMinimum(arg1);
    emit zValuesChanged(ui->dsb_zmin->value(), ui->dsb_zmax->value());
}
void ONF_ActionDefineHeightLayerOptions::on_onf_computeCrownProjection_clicked()
{
    emit askForCrownProjectionComputing();
}
