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

#include "onf_actionslicepointcloudoptions.h"
#include "ui_onf_actionslicepointcloudoptions.h"

#include "actions/onf_actionslicepointcloud.h"

#include <QColorDialog>

ONF_ActionSlicePointCloudOptions::ONF_ActionSlicePointCloudOptions(const ONF_ActionSlicePointCloud *action) :
    CT_GAbstractActionForGraphicsViewOptions(action),
    ui(new Ui::ONF_ActionSlicePointCloudOptions())
{
    ui->setupUi(this);

    connect(ui->dsb_thickness, SIGNAL(valueChanged(double)), this, SIGNAL(parametersChanged()));
    connect(ui->dsb_space, SIGNAL(valueChanged(double)), this, SIGNAL(parametersChanged()));

    ui->dsb_thickness->setToolTip(tr("CTRL Molette"));
    ui->lb_thickness->setToolTip(tr("CTRL Molette"));
    ui->dsb_space->setToolTip(tr("SHIFT Molette"));
    ui->lb_spacing->setToolTip(tr("SHIFT Molette"));


    connect(ui->bg_singleStep, SIGNAL(buttonClicked(int)), this, SLOT(changeSingleStep(int)));
    ui->rb_10cm->setChecked(true);

}

ONF_ActionSlicePointCloudOptions::~ONF_ActionSlicePointCloudOptions()
{
    delete ui;
}

double ONF_ActionSlicePointCloudOptions::getThickness() const
{
    return ui->dsb_thickness->value();
}

double ONF_ActionSlicePointCloudOptions::getSpacing() const
{
    return ui->dsb_space->value();
}

double ONF_ActionSlicePointCloudOptions::getIncrement() const
{
    if (ui->rb_1mm->isChecked()) {return 0.001;}
    if (ui->rb_1cm->isChecked()) {return 0.01;}
    if (ui->rb_10cm->isChecked()) {return 0.1;}
    if (ui->rb_1m->isChecked()) {return 1;}

    return 0.1;
}

void ONF_ActionSlicePointCloudOptions::setThickness(double t) const
{
    ui->dsb_thickness->setValue(t);
}

void ONF_ActionSlicePointCloudOptions::setSpacing(double s) const
{
    ui->dsb_space->setValue(s);
}

void ONF_ActionSlicePointCloudOptions::increaseIncrement()
{
    if (ui->rb_1mm->isChecked()) {ui->rb_1cm->setChecked(true);}
    else if (ui->rb_1cm->isChecked()) {ui->rb_10cm->setChecked(true);}
    else if (ui->rb_10cm->isChecked()) {ui->rb_1m->setChecked(true);}
}

void ONF_ActionSlicePointCloudOptions::decreaseIncrement()
{
    if (ui->rb_1m->isChecked()) {ui->rb_10cm->setChecked(true);}
    else if (ui->rb_10cm->isChecked()) {ui->rb_1cm->setChecked(true);}
    else if (ui->rb_1cm->isChecked()) {ui->rb_1mm->setChecked(true);}

}

void ONF_ActionSlicePointCloudOptions::changeSingleStep(int button)
{
    Q_UNUSED(button);

    if (ui->rb_1mm->isChecked()) {ui->dsb_thickness->setSingleStep(0.001);ui->dsb_space->setSingleStep(0.001);}
    else if (ui->rb_1cm->isChecked()) {ui->dsb_thickness->setSingleStep(0.01);ui->dsb_space->setSingleStep(0.01);}
    else if (ui->rb_10cm->isChecked()) {ui->dsb_thickness->setSingleStep(0.1);ui->dsb_space->setSingleStep(0.1);}
    else if (ui->rb_1m->isChecked()) {ui->dsb_thickness->setSingleStep(1);ui->dsb_space->setSingleStep(1);}

}

