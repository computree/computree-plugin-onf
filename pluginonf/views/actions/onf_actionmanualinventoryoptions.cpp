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

#include "onf_actionmanualinventoryoptions.h"
#include "ui_onf_actionmanualinventoryoptions.h"

#include "actions/onf_actionmanualinventory.h"

#include <QColorDialog>

ONF_ActionManualInventoryOptions::ONF_ActionManualInventoryOptions(const ONF_ActionManualInventory *action) :
    CT_GAbstractActionOptions(action),
    ui(new Ui::ONF_ActionManualInventoryOptions())
{
    ui->setupUi(this);

    ui->rb_select->setToolTip(tr("Séléction de la scène active, sans modification [S]"));
    ui->rb_dbh->setToolTip(tr("Choix d'un cercle pour une scène [D]"));
    ui->rb_attributes->setToolTip(tr("Ouverture de la saisie des attributs pour une scène [F] ou [C] pour la scène active"));
    ui->pb_upper->setToolTip(tr("Séléctionne le cercle supérieur (CTRL MOLETTE +)"));
    ui->pb_lower->setToolTip(tr("Séléctionne le cercle inférieur (CTRL MOLETTE -)"));
    ui->pb_toValidated->setToolTip(tr("Valide la scène active [V] ou [Fin]"));
}

ONF_ActionManualInventoryOptions::~ONF_ActionManualInventoryOptions()
{
    delete ui;
}

bool ONF_ActionManualInventoryOptions::shouldAutoCenterCamera()
{
    return ui->cb_centerCamera->isChecked();
}

void ONF_ActionManualInventoryOptions::chooseSelectMode()
{
    ui->rb_select->setChecked(true);
}

void ONF_ActionManualInventoryOptions::chooseDbhMode()
{
    ui->rb_dbh->setChecked(true);
}

void ONF_ActionManualInventoryOptions::chooseAttributesMode()
{
    ui->rb_attributes->setChecked(true);
}

bool ONF_ActionManualInventoryOptions::isSelectModeSelected()
{
    return ui->rb_select->isChecked();
}

bool ONF_ActionManualInventoryOptions::isDbhModeSelected()
{
    return ui->rb_dbh->isChecked();
}

bool ONF_ActionManualInventoryOptions::isAttributesModeSelected()
{
    return ui->rb_attributes->isChecked();
}

bool ONF_ActionManualInventoryOptions::isShowDataChecked()
{
    return ui->cb_showData->isChecked();
}

bool ONF_ActionManualInventoryOptions::isShowActiveCirclesChecked()
{
    return ui->cb_activeCircles->isChecked();
}

bool ONF_ActionManualInventoryOptions::isShowOtherCirclesChecked()
{
    return ui->cb_otherCircles->isChecked();
}

bool ONF_ActionManualInventoryOptions::isShowActiveSceneChecked()
{
    return ui->cb_activeScene->isChecked();
}

bool ONF_ActionManualInventoryOptions::isShowOtherScenesChecked()
{
    return ui->cb_otherScenes->isChecked();
}

bool ONF_ActionManualInventoryOptions::isShowTrashChecked()
{
    return ui->cb_trash->isChecked();
}

bool ONF_ActionManualInventoryOptions::isShowTrashedScenesChecked()
{
    return ui->cb_trashScenes->isChecked();
}

bool ONF_ActionManualInventoryOptions::isAutoValidateChecked()
{
    return ui->cb_autoValidate->isChecked();
}

void ONF_ActionManualInventoryOptions::on_rb_select_toggled(bool checked)
{
    Q_UNUSED(checked);
    emit modeChanged();
}

void ONF_ActionManualInventoryOptions::on_rb_dbh_toggled(bool checked)
{
    Q_UNUSED(checked);
    emit modeChanged();
}

void ONF_ActionManualInventoryOptions::on_rb_attributes_toggled(bool checked)
{
    Q_UNUSED(checked);
    emit modeChanged();
}

void ONF_ActionManualInventoryOptions::on_cb_showData_toggled(bool checked)
{
    Q_UNUSED(checked);
    emit visibilityChanged();
}

void ONF_ActionManualInventoryOptions::on_cb_activeCircles_toggled(bool checked)
{
    Q_UNUSED(checked);
    emit visibilityChanged();
}

void ONF_ActionManualInventoryOptions::on_cb_otherCircles_toggled(bool checked)
{
    Q_UNUSED(checked);
    emit visibilityChanged();
}

void ONF_ActionManualInventoryOptions::on_cb_activeScene_toggled(bool checked)
{
    Q_UNUSED(checked);
    emit visibilityChanged();
}

void ONF_ActionManualInventoryOptions::on_cb_otherScenes_toggled(bool checked)
{
    Q_UNUSED(checked);
    emit visibilityChanged();
}

void ONF_ActionManualInventoryOptions::on_onf_upper_clicked()
{
    emit chooseUpperCircle();
}

void ONF_ActionManualInventoryOptions::on_onf_lower_clicked()
{
    emit chooseLowerCircle();
}

void ONF_ActionManualInventoryOptions::on_onf_toTrash_clicked()
{
    emit sendToTrash();
}

void ONF_ActionManualInventoryOptions::on_onf_fromTrash_clicked()
{
    emit retrieveFromTrash();
}

void ONF_ActionManualInventoryOptions::on_onf_toValidated_clicked()
{
    emit sendToValidated();
}

void ONF_ActionManualInventoryOptions::on_cb_trash_toggled(bool checked)
{
    Q_UNUSED(checked);
    emit visibilityChanged();
}

void ONF_ActionManualInventoryOptions::on_cb_trashScenes_toggled(bool checked)
{
    Q_UNUSED(checked);
    emit visibilityChanged();
}
