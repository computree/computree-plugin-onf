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

#include "onf_actionselectcellsingrid3doptions.h"
#include "ui_onf_actionselectcellsingrid3doptions.h"

#include "actions/onf_actionselectcellsingrid3d.h"

#include <QColorDialog>

ONF_ActionSelectCellsInGrid3DOptions::ONF_ActionSelectCellsInGrid3DOptions(const ONF_ActionSelectCellsInGrid3D *action, int maxlevel) :
    CT_GAbstractActionOptions(action),
    ui(new Ui::ONF_ActionSelectCellsInGrid3DOptions)
{
    ui->setupUi(this);

    _maxLevel = maxlevel;
    _gridColor = QColor(125, 125, 125);
    ui->pb_gridColor->setStyleSheet("QPushButton { background-color: " + _gridColor.name() + "; }");

    ui->pb_add->setToolTip(tr("Change de mode : 2D / 3D (D = 2D / 3D)"));

    ui->pb_add->setToolTip(tr("Ajouter des cellules (A = Add)"));
    ui->pb_remove->setToolTip(tr("Supprimer des cellules (R = Remove)"));
    ui->pb_freemove->setToolTip(tr("Déplacement de la vue (F = Free Move)"));
    ui->pb_extends->setToolTip(tr("Extension jusqu'aux limites (E = Extends)\nEn mode 3D, cet outil ne fonctionne que sur des cellules séléctionnées"));
    ui->pb_colonize->setToolTip(tr("Colonization des valeurs non vides (Z = coloniZe)"));
    ui->pb_centerOnCell->setToolTip(tr("Sélection d'une case pour centrer la vue"));

    ui->pb_showselected->setToolTip(tr("Monter toute les cellules / les cellules sélectionnées"));
    ui->pb_gridon->setToolTip(tr("Grille visible / masquée"));
    ui->pb_all->setToolTip(tr("Cumuler tous les niveaux"));
    ui->pb_copyfull->setToolTip(tr("Copier le niveau actuel à toute la grille (ATTENTION écrasement des valeurs)"));
    ui->pb_copyupwards->setToolTip(tr("Copier le niveau actuel aux niveaux actuellement cumulés (ATTENTION écrasement des valeurs)"));

    ui->pb_gridColor->setToolTip(tr("Couleur des voxels non vides et non séléctionnés"));

    ui->cb_cumulMode->setToolTip(tr("Mode de cumul des niveaux"));
    ui->cb_relative->setToolTip(tr("Echelle de couleur calée sur le niveau en cours"));

    ui->lb_level->setToolTip(tr("Niveau Z affiché, ou le plus bas si épaisseur > 1 (CTRL MOLETTE)"));
    ui->lb_thickness->setToolTip(tr("Nombre de niveaux cumulés = épaisseur (SHIFT MOLETTE)"));
    ui->lb_coef->setToolTip(tr("Facteur de réduction de la taille des cellules 3D"));

    ui->sb_level->setMaximum(_maxLevel);
    ui->sb_level->setValue(_maxLevel / 2);
    ui->sb_thickness->setMinimum(1);
    ui->sb_thickness->setMaximum(_maxLevel - ui->sb_level->value() + 1);
    ui->sb_thickness->setValue(1);

    ui->pb_freemove->setChecked(true);
    ui->pb_add->setChecked(false);
    ui->pb_remove->setChecked(false);
    ui->pb_colonize->setChecked(false);
    ui->pb_extends->setChecked(false);
    ui->pb_centerOnCell->setChecked(false);

    ui->cb_cumulMode->addItem("Mean");
    ui->cb_cumulMode->addItem("Max");
    ui->cb_cumulMode->addItem("Sum");
    ui->cb_cumulMode->addItem("Number");

    _colonizeDialog.setXm(true);
    _colonizeDialog.setXp(true);
    _colonizeDialog.setYm(true);
    _colonizeDialog.setYp(true);
    _colonizeDialog.setZm(true);
    _colonizeDialog.setZp(true);

    _mode = MAX;

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
        ui->cb_cumulMode->setCurrentIndex(ui->cb_cumulMode->findText("Max"));
#else
        ui->cb_cumulMode->setCurrentText("Max");
#endif

    ui->wd_3D->hide();

    connect(ui->sb_level, SIGNAL(valueChanged(int)), this, SIGNAL(levelChanged(int)));
    connect(ui->sb_thickness, SIGNAL(valueChanged(int)), this, SIGNAL(thicknessChanged(int)));
    connect(ui->pb_showselected, SIGNAL(clicked()), this, SIGNAL(showModeChanged()));
    connect(ui->pb_gridon, SIGNAL(clicked()), this, SIGNAL(gridModeChanged()));
    connect(ui->cb_relative, SIGNAL(clicked()), this, SIGNAL(relativeRepresentationChanged()));
    connect(ui->sbd_coef, SIGNAL(valueChanged(double)), this, SIGNAL(coefChanged(double)));
}

ONF_ActionSelectCellsInGrid3DOptions::~ONF_ActionSelectCellsInGrid3DOptions()
{
    delete ui;
}

int ONF_ActionSelectCellsInGrid3DOptions::level() const
{
    return ui->sb_level->value();
}

int ONF_ActionSelectCellsInGrid3DOptions::thickness() const
{
    return ui->sb_thickness->value();
}

QColor ONF_ActionSelectCellsInGrid3DOptions::gridColor() const
{
    return _gridColor;
}

bool ONF_ActionSelectCellsInGrid3DOptions::showSeletedOnly() const
{
    return ui->pb_showselected->isChecked();
}

bool ONF_ActionSelectCellsInGrid3DOptions::isGridOn() const
{
    return !ui->pb_gridon->isChecked();
}


bool ONF_ActionSelectCellsInGrid3DOptions::relativeRepresentation() const
{
    return ui->cb_relative->isChecked();
}


ONF_ActionSelectCellsInGrid3DOptions::GridSelectionMode ONF_ActionSelectCellsInGrid3DOptions::selectionMode() const
{
    ONF_ActionSelectCellsInGrid3DOptions::GridSelectionMode mode = ONF_ActionSelectCellsInGrid3DOptions::FREEMOVE;

    if(ui->pb_add->isChecked())
    {
        mode = ADD;
    }
    else if(ui->pb_remove->isChecked())
    {
        mode = REMOVE;
    }
    else if(ui->pb_colonize->isChecked())
    {
        mode = COLONIZE;
    }
    else if(ui->pb_extends->isChecked())
    {
        mode = EXTENDS;
    }
    else if(ui->pb_centerOnCell->isChecked())
    {
        mode = CHANGECENTERCELL;
    }

    return mode;
}

void ONF_ActionSelectCellsInGrid3DOptions::setSelectionMode(GridSelectionMode mode)
{
    switch(mode)
    {
    case FREEMOVE:
        ui->pb_freemove->setChecked(true);
        ui->pb_add->setChecked(false);
        ui->pb_remove->setChecked(false);
        ui->pb_colonize->setChecked(false);
        ui->pb_extends->setChecked(false);
        ui->pb_centerOnCell->setChecked(false);
        break;
    case ADD:
        ui->pb_freemove->setChecked(false);
        ui->pb_add->setChecked(true);
        ui->pb_remove->setChecked(false);
        ui->pb_colonize->setChecked(false);
        ui->pb_extends->setChecked(false);
        ui->pb_centerOnCell->setChecked(false);
        break;

    case REMOVE:
        ui->pb_freemove->setChecked(false);
        ui->pb_add->setChecked(false);
        ui->pb_remove->setChecked(true);
        ui->pb_colonize->setChecked(false);
        ui->pb_extends->setChecked(false);
        ui->pb_centerOnCell->setChecked(false);
        break;

    case COLONIZE:
        ui->pb_freemove->setChecked(false);
        ui->pb_add->setChecked(false);
        ui->pb_remove->setChecked(false);
        ui->pb_colonize->setChecked(true);
        ui->pb_extends->setChecked(false);
        ui->pb_centerOnCell->setChecked(false);
        break;

    case EXTENDS:
        ui->pb_freemove->setChecked(false);
        ui->pb_add->setChecked(false);
        ui->pb_remove->setChecked(false);
        ui->pb_colonize->setChecked(false);
        ui->pb_extends->setChecked(true);
        ui->pb_centerOnCell->setChecked(false);
        break;

    case CHANGECENTERCELL:
        ui->pb_freemove->setChecked(false);
        ui->pb_add->setChecked(false);
        ui->pb_remove->setChecked(false);
        ui->pb_colonize->setChecked(false);
        ui->pb_extends->setChecked(false);
        ui->pb_centerOnCell->setChecked(true);
        break;

    default:
        ui->pb_freemove->setChecked(true);
        ui->pb_add->setChecked(false);
        ui->pb_remove->setChecked(false);
        ui->pb_colonize->setChecked(false);
        ui->pb_extends->setChecked(false);
        ui->pb_centerOnCell->setChecked(false);
        break;
    }
    emit selectionModeChanged(mode);
}

void ONF_ActionSelectCellsInGrid3DOptions::modifyThickness(int modifier)
{
    ui->sb_thickness->setValue(ui->sb_thickness->value() + modifier);
}

void ONF_ActionSelectCellsInGrid3DOptions::modifyLevel(int modifier)
{
    ui->sb_level->setValue(ui->sb_level->value() + modifier);

}

void ONF_ActionSelectCellsInGrid3DOptions::modify2D3DMode()
{
    ui->pb_2d3d->setChecked(!ui->pb_2d3d->isChecked());
    on_pb_2d3d_clicked(ui->pb_2d3d->isChecked());
}



void ONF_ActionSelectCellsInGrid3DOptions::on_sb_level_valueChanged(int arg1)
{
    ui->sb_thickness->setMaximum(_maxLevel - arg1 + 1);
}

void ONF_ActionSelectCellsInGrid3DOptions::on_pb_all_clicked()
{
    ui->sb_level->setValue(0);
    ui->sb_thickness->setValue(_maxLevel + 1);
}


void ONF_ActionSelectCellsInGrid3DOptions::on_cb_cumulMode_currentTextChanged(const QString &arg1)
{
    if (arg1 == "Mean") {_mode = MEAN;}
    else if (arg1 == "Max") {_mode = MAX;}
    else if (arg1 == "Sum") {_mode = SUM;}
    else if (arg1 == "Number") {_mode = NUMBER;}

    emit cumulModeChanged(_mode);
}

void ONF_ActionSelectCellsInGrid3DOptions::on_pb_copyupwards_clicked()
{
    setSelectionMode(FREEMOVE);
    emit askForUpwardsCopy();
}

void ONF_ActionSelectCellsInGrid3DOptions::on_pb_copyfull_clicked()
{
    setSelectionMode(FREEMOVE);
    emit askForFullCopy();
}



void ONF_ActionSelectCellsInGrid3DOptions::on_pb_2d3d_clicked(bool checked)
{
    setSelectionMode(FREEMOVE);
    if (checked)
    {
        ui->wd_2D->hide();
        ui->wd_3D->show();
    } else {
        ui->wd_2D->show();
        ui->wd_3D->hide();
    }
    emit mode2D3DChanged(checked);
}


void ONF_ActionSelectCellsInGrid3DOptions::on_pb_gridColor_clicked()
{
    _gridColor = QColorDialog::getColor(_gridColor);
    if (!_gridColor.isValid()) {_gridColor = QColor(125, 125, 125);}
    ui->pb_gridColor->setStyleSheet("QPushButton { background-color: " + _gridColor.name() + "; }");
    emit gridColorChanged(_gridColor);
}

void ONF_ActionSelectCellsInGrid3DOptions::on_pb_centerOnCell_clicked()
{
    emit selectionModeChanged(ONF_ActionSelectCellsInGrid3DOptions::CHANGECENTERCELL);
}

void ONF_ActionSelectCellsInGrid3DOptions::on_pb_add_clicked()
{
    emit selectionModeChanged(ONF_ActionSelectCellsInGrid3DOptions::ADD);
}

void ONF_ActionSelectCellsInGrid3DOptions::on_pb_colonize_clicked()
{

    emit selectionModeChanged(ONF_ActionSelectCellsInGrid3DOptions::COLONIZE);

}

void ONF_ActionSelectCellsInGrid3DOptions::on_pb_extends_clicked()
{
    emit selectionModeChanged(ONF_ActionSelectCellsInGrid3DOptions::EXTENDS);
}

void ONF_ActionSelectCellsInGrid3DOptions::on_pb_remove_clicked()
{
    emit selectionModeChanged(ONF_ActionSelectCellsInGrid3DOptions::REMOVE);
}

void ONF_ActionSelectCellsInGrid3DOptions::on_pb_freemove_clicked()
{
    emit selectionModeChanged(ONF_ActionSelectCellsInGrid3DOptions::FREEMOVE);
}

void ONF_ActionSelectCellsInGrid3DOptions::on_pb_colonizeParam_clicked()
{
    _colonizeDialog.exec();
}

void ONF_ActionSelectCellsInGrid3DOptions::on_pb_selectNotEmptyCells_clicked()
{
    setSelectionMode(FREEMOVE);
    emit askForSelectionOfNotEmptyCells();
}

void ONF_ActionSelectCellsInGrid3DOptions::on_pb_convexHull_clicked()
{
    setSelectionMode(FREEMOVE);
    emit askForConvexHull();
}
