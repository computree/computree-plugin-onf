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


