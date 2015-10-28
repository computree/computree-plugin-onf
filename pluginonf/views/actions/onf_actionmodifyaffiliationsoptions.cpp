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

#include "onf_actionmodifyaffiliationsoptions.h"
#include "ui_onf_actionmodifyaffiliationsoptions.h"
#include "ct_global/ct_context.h"

#include "actions/onf_actionmodifyaffiliations.h"

ONF_ActionModifyAffiliationsOptions::ONF_ActionModifyAffiliationsOptions(const ONF_ActionModifyAffiliations *action) :
    CT_GAbstractActionOptions(action),
    ui(new Ui::ONF_ActionModifyAffiliationsOptions)
{
    ui->setupUi(this);


    connect(ui->cb_source, SIGNAL(clicked()), this, SIGNAL(parametersChanged()));
    connect(ui->cb_target, SIGNAL(clicked()), this, SIGNAL(parametersChanged()));
    connect(ui->cb_affiliations, SIGNAL(clicked()), this, SIGNAL(parametersChanged()));
    connect(ui->cb_showItems, SIGNAL(clicked()), this, SIGNAL(parametersChanged()));
    connect(ui->cb_showCenters, SIGNAL(clicked()), this, SIGNAL(parametersChanged()));
    connect(ui->cb_selectedOnly, SIGNAL(clicked()), this, SIGNAL(parametersChanged()));
    connect(ui->cb_att, SIGNAL(clicked()), this, SIGNAL(parametersChanged()));
    connect(ui->pb_affiliate, SIGNAL(clicked()), this, SIGNAL(askForAffiliation()));
    connect(ui->pb_break, SIGNAL(clicked()), this, SIGNAL(askForBreakingAffiliation()));

    ui->cb_affiliations->setToolTip(tr("Afficher les lignes reliant les items affiliés"));
    ui->cb_selectedOnly->setToolTip(tr("N'afficher que les items sélectionnés (de référence et à affilier"));
    ui->cb_showCenters->setToolTip(tr("Afficher les centres des items"));
    ui->cb_showItems->setToolTip(tr("Afficher les items"));
    ui->cb_source->setToolTip(tr("Afficher les items de référence"));
    ui->cb_target->setToolTip(tr("Afficher les items à affilier"));
    ui->pb_affiliate->setToolTip(tr("Affilier l'item de référence et l'item à affilier sélectionnés (A)"));
    ui->pb_break->setToolTip(tr("Supprimer l'affiliation de l'item à affilier sélectionné (Z)"));

    ui->pb_selection->setToolTip(tr("Activer la selection (S) :\n"
                                    "- Clic gauche = sélectionner un item de référence\n"
                                    "- Clic droit  = sélectionner un item à affilier"));
}

ONF_ActionModifyAffiliationsOptions::~ONF_ActionModifyAffiliationsOptions()
{
    delete ui;
}

bool ONF_ActionModifyAffiliationsOptions::sourceVisible()
{
    return ui->cb_source->isChecked();
}

bool ONF_ActionModifyAffiliationsOptions::targetVisible()
{
    return ui->cb_target->isChecked();
}

bool ONF_ActionModifyAffiliationsOptions::affiliationsLinesVisible()
{
    return ui->cb_affiliations->isChecked();
}

bool ONF_ActionModifyAffiliationsOptions::itemsVisible()
{
    return ui->cb_showItems->isChecked();
}

bool ONF_ActionModifyAffiliationsOptions::centersVisible()
{
    return ui->cb_showCenters->isChecked();
}

bool ONF_ActionModifyAffiliationsOptions::onlySelectedVisible()
{
    return ui->cb_selectedOnly->isChecked();
}

bool ONF_ActionModifyAffiliationsOptions::selectionActivated()
{
    return ui->pb_selection->isChecked();
}

bool ONF_ActionModifyAffiliationsOptions::showAttributes()
{
    return ui->cb_att->isChecked();
}

void ONF_ActionModifyAffiliationsOptions::toggleSelection()
{
    ui->pb_selection->setChecked(!ui->pb_selection->isChecked());
}


