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

#include "onf_actionsegmentcrownsoptions.h"
#include "ui_onf_actionsegmentcrownsoptions.h"

#include "actions/onf_actionsegmentcrowns.h"

#include <QColorDialog>

ONF_ActionSegmentCrownsOptions::ONF_ActionSegmentCrownsOptions(const ONF_ActionSegmentCrowns *action, float initZvalue) :
    CT_GAbstractActionOptions(action),
    ui(new Ui::ONF_ActionSegmentCrownsOptions())
{
    ui->setupUi(this);
    ui->dsb_height->setValue(initZvalue);

    ui->pb_pickup->setToolTip(tr("Choisir le cluster actif (S = Select)"));
    ui->pb_drawLimit->setToolTip(tr("Dessiner une limite (D = Draw)"));
    ui->pb_fill->setToolTip(tr("Inclure une zone dans le cluster actif (F = Fill)"));
    ui->pb_freemove->setToolTip(tr("Naviguer en 3D (G)"));
    ui->pb_centerOnCell->setToolTip(tr("Recentrer la vue sur la case sélectionnée"));
    ui->pb_addCluster->setToolTip(tr("Ajouter un nouveau cluster et le rendre actif"));
    ui->pb_activeClusterColor->setToolTip(tr("Couleur du cluster actif (attention les couleurs sont recyclées)\n"
                                             "Si enfoncé : le cluster actif est tracé en rouge, même en mode cluster"));
    ui->pb_showClusters->setToolTip(tr("Afficher / masquer les cellules vides"));
    ui->pb_fillMode->setToolTip(tr("Choisir le mode de remplissage (pot de peinture) :\n"
                                   "- Remplir uniquement la zone sélectionnée\n"
                                   "- Remplir tous les pixels du cluster"));
    ui->pb_merge->setToolTip(tr("Agrèges chaque petit cluster (< n pixels) avec le grand cluster le plus proche"));
    ui->sb_merge->setToolTip(tr("Taille maximale d'un petit cluster (en pixels)"));

    ui->pb_undo->setToolTip(tr("Annuler (CTRL+Z)"));
    ui->pb_redo->setToolTip(tr("Rétablir (CTRL+Y)"));

    ui->rb_clusters->setToolTip(tr("Afficher les clusters"));
    ui->rb_density->setToolTip(tr("Afficher la carte de densité"));
    ui->rb_mns->setToolTip(tr("Afficher le Modèle Numérique de Surface"));

    ui->rb_m->setToolTip(tr("Ajuster le plan de visualisation par étapes de 1 mètre"));
    ui->rb_dm->setToolTip(tr("Ajuster le plan de visualisation par étapes de 10 cm"));
    ui->rb_cm->setToolTip(tr("Ajuster le plan de visualisation par étapes de 1 cm"));


    ui->sb_activeCluster->setToolTip(tr("Changer de cluster actif"));
    ui->lb_activeCluster->setToolTip(tr("Changer de cluster actif"));
    ui->dsb_height->setToolTip(tr("Choisir la hauteur du plan de visualisation (CTRL + molette)"));
    ui->lb_height->setToolTip(tr("Choisir la hauteur du plan de visualisation (CTRL + molette)"));
    ui->sb_pencilSize->setToolTip(tr("Changer la taille de pinceau (SHIFT + molette)"));
    ui->lb_pencilSize->setToolTip(tr("Changer la taille de pinceau (SHIFT + molette)"));

    ui->pb_activeClusterColor->setChecked(false);

    connect(ui->bg_singleStep, SIGNAL(buttonClicked(int)), this, SLOT(changeSingleStep(int)));
    connect(ui->pb_undo, SIGNAL(clicked()), this, SIGNAL(undo()));
    connect(ui->pb_redo, SIGNAL(clicked()), this, SIGNAL(redo()));
    setUndoRedo(false, false);
}

ONF_ActionSegmentCrownsOptions::~ONF_ActionSegmentCrownsOptions()
{
    delete ui;
}

float ONF_ActionSegmentCrownsOptions::getHeight()
{
    return ui->dsb_height->value();
}

int ONF_ActionSegmentCrownsOptions::getActiveCluster()
{
    return ui->sb_activeCluster->value();
}

bool ONF_ActionSegmentCrownsOptions::getShowClustersOnly()
{
    return ui->pb_showClusters->isChecked();
}


ONF_ActionSegmentCrownsOptions::Mode ONF_ActionSegmentCrownsOptions::getMode()
{
    return _mode;
}

ONF_ActionSegmentCrownsOptions::DrawingGrid ONF_ActionSegmentCrownsOptions::getDrawingGrid()
{
    return _drawingGrid;
}

void ONF_ActionSegmentCrownsOptions::setHeight(float h)
{
    ui->dsb_height->setValue(h);
    emit parametersChanged();
}

void ONF_ActionSegmentCrownsOptions::setActiveCluster(int cluster)
{
    ui->sb_activeCluster->setValue(cluster);
    emit parametersChanged();
}

void ONF_ActionSegmentCrownsOptions::setClusterNumber(int nb)
{
    ui->sb_activeCluster->setMaximum(nb);
    emit parametersChanged();
}

void ONF_ActionSegmentCrownsOptions::setMode(Mode mode)
{
    _mode = mode;

    switch (_mode) {
    case CHOOSECLUSTER: ui->pb_pickup->setChecked(true); break;
    case DRAWLIMITS: ui->pb_drawLimit->setChecked(true); break;
    case FILLAREA: ui->pb_fill->setChecked(true); break;
    case FREEMOVE: ui->pb_freemove->setChecked(true); break;
    case CHANGECENTERCELL: ui->pb_centerOnCell->setChecked(true); break;
    };
}

void ONF_ActionSegmentCrownsOptions::setDrawingGrid(DrawingGrid drawingGrid)
{
    _drawingGrid = drawingGrid;

    switch (_drawingGrid) {
    case DENSITY: ui->rb_density->setChecked(true); break;
    case HEIGHT: ui->rb_mns->setChecked(true); break;
    case CLUSTERS: ui->rb_clusters->setChecked(true); break;
    };

    emit parametersChanged();
}

double ONF_ActionSegmentCrownsOptions::getHSingleStep()
{
    if (ui->rb_m->isChecked()) {return 1;}
    if (ui->rb_dm->isChecked()) {return 0.10;}
    if (ui->rb_cm->isChecked()) {return 0.01;}

    return 1;
}

int ONF_ActionSegmentCrownsOptions::getPencilSize()
{
    return ui->sb_pencilSize->value();
}

bool ONF_ActionSegmentCrownsOptions::redForSelection()
{
    return ui->pb_activeClusterColor->isChecked();
}

bool ONF_ActionSegmentCrownsOptions::isFillModeFull()
{
    return ui->pb_fillMode->isChecked();
}

void ONF_ActionSegmentCrownsOptions::increaseHValue()
{
    ui->dsb_height->setValue(ui->dsb_height->value() + getHSingleStep());
    emit parametersChanged();
}

void ONF_ActionSegmentCrownsOptions::decreaseHValue()
{
    ui->dsb_height->setValue(ui->dsb_height->value() - getHSingleStep());
    emit parametersChanged();
}

void ONF_ActionSegmentCrownsOptions::increasePencilSizeValue()
{
    ui->sb_pencilSize->setValue(ui->sb_pencilSize->value() + 1);
    emit parametersChanged();
}

void ONF_ActionSegmentCrownsOptions::decreasePencilSizeValue()
{
    ui->sb_pencilSize->setValue(ui->sb_pencilSize->value() - 1);
    emit parametersChanged();
}

void ONF_ActionSegmentCrownsOptions::setActiveClusterColor(const QColor* color)
{  
    if (redForSelection())
    {
        ui->pb_activeClusterColor->setStyleSheet("QPushButton { background-color: transparent; }");
    } else {
        ui->pb_activeClusterColor->setStyleSheet("QPushButton { background-color: " + color->name() + "; }");
    }
}

void ONF_ActionSegmentCrownsOptions::setUndoRedo(bool undo, bool redo)
{
    ui->pb_undo->setEnabled(undo);
    ui->pb_redo->setEnabled(redo);
}

void ONF_ActionSegmentCrownsOptions::on_onf_pickup_clicked()
{
    setMode(CHOOSECLUSTER);
}

void ONF_ActionSegmentCrownsOptions::on_onf_addCluster_clicked()
{
    setMode(DRAWLIMITS);
    emit askForClusterCreation();
}

void ONF_ActionSegmentCrownsOptions::on_onf_drawLimit_clicked()
{
    setMode(DRAWLIMITS);
}

void ONF_ActionSegmentCrownsOptions::on_onf_fill_clicked()
{
    setMode(FILLAREA);
}

void ONF_ActionSegmentCrownsOptions::on_onf_freemove_clicked()
{
    setMode(FREEMOVE);
}

void ONF_ActionSegmentCrownsOptions::on_onf_centerOnCell_clicked()
{
    setMode(CHANGECENTERCELL);
}

void ONF_ActionSegmentCrownsOptions::on_rb_clusters_clicked()
{
    setDrawingGrid(CLUSTERS);
}

void ONF_ActionSegmentCrownsOptions::on_rb_mns_clicked()
{
    setDrawingGrid(HEIGHT);
}

void ONF_ActionSegmentCrownsOptions::on_rb_density_clicked()
{
    setDrawingGrid(DENSITY);
}

void ONF_ActionSegmentCrownsOptions::on_sb_activeCluster_valueChanged(int arg1)
{
    Q_UNUSED(arg1)
    emit activeClusterChanged();
    emit parametersChanged();
}

void ONF_ActionSegmentCrownsOptions::on_dsb_height_valueChanged(double arg1)
{
    Q_UNUSED(arg1)
    emit parametersChanged();
}

void ONF_ActionSegmentCrownsOptions::on_onf_showClusters_clicked()
{
    emit parametersChanged();
}

void ONF_ActionSegmentCrownsOptions::on_sb_pencilSize_valueChanged(int arg1)
{
    emit parametersChanged();
}

void ONF_ActionSegmentCrownsOptions::on_onf_activeClusterColor_toggled(bool checked)
{
    emit activeClusterChanged();
    emit parametersChanged();
}

void ONF_ActionSegmentCrownsOptions::changeSingleStep(int button)
{
    Q_UNUSED(button);
    if (ui->rb_m->isChecked()) {ui->dsb_height->setSingleStep(1);}
    else if (ui->rb_dm->isChecked()) {ui->dsb_height->setSingleStep(0.1);}
    else if (ui->rb_cm->isChecked()) {ui->dsb_height->setSingleStep(0.01);}
}

void ONF_ActionSegmentCrownsOptions::on_onf_merge_clicked()
{
    emit askForMerging(ui->sb_merge->value());
}
