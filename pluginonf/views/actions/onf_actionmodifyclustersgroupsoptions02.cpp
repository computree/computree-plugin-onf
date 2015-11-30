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

#include "onf_actionmodifyclustersgroupsoptions02.h"
#include "ui_onf_actionmodifyclustersgroupsoptions02.h"
#include "ct_global/ct_context.h"

#include <QColorDialog>

ONF_ActionModifyClustersGroupsOptions02::ONF_ActionModifyClustersGroupsOptions02(const ONF_ActionModifyClustersGroups02 *action) :
    CT_GAbstractActionOptions(action),
    ui(new Ui::ONF_ActionModifyClustersGroupsOptions02)
{
    ui->setupUi(this);

    ui->pb_SetSceneA->setToolTip(tr("Choisit le groupe A [double-clic GAUCHE]"));
    ui->pb_SetSceneB->setToolTip(tr("Choisit le groupe B [double-clic DROIT]"));
    ui->pb_extend->setToolTip(tr("Etendre la séléction [SHIFT]"));
    ui->toolButtonSelectMulti->setToolTip(tr("Maintenir [CTRL] enfoncé pour activer temporairement"));

    ui->pb_toA->setToolTip(tr("Affecte les points au groupe A [A]"));
    ui->pb_toB->setToolTip(tr("Affecte les points au groupe Z [Z]"));
    ui->pb_toTmp->setToolTip(tr("Affecte les points au groupe ? [E]"));
    ui->pb_toTrash->setToolTip(tr("Affecte les points à la poubelle [R]"));

    ui->cb_othersVisible->setToolTip(tr("Touche [espace] pour changer"));

    ui->pb_validate->setToolTip(tr("Valider le groupe Z [V]"));
}

ONF_ActionModifyClustersGroupsOptions02::~ONF_ActionModifyClustersGroupsOptions02()
{
    delete ui;
}

GraphicsViewInterface::SelectionMode ONF_ActionModifyClustersGroupsOptions02::selectionMode() const
{   
    int mode = GraphicsViewInterface::NONE;

    if (ui->toolButtonSelectOne->isChecked())
    {
        if (ui->toolButtonReplaceMode->isChecked()) {
            mode = GraphicsViewInterface::SELECT_ONE;
        } else if (ui->toolButtonAddMode->isChecked()) {
            mode = GraphicsViewInterface::ADD_ONE;
        } else {
            mode = GraphicsViewInterface::REMOVE_ONE;
        }
    } else if (ui->toolButtonSelectMulti->isChecked()) {
        if (ui->toolButtonReplaceMode->isChecked()) {
            mode = GraphicsViewInterface::SELECT;
        } else if (ui->toolButtonAddMode->isChecked()) {
            mode = GraphicsViewInterface::ADD;
        } else {
            mode = GraphicsViewInterface::REMOVE;
        }
    } else {
        return (GraphicsViewInterface::SelectionMode)mode;
    }

    return (GraphicsViewInterface::SelectionMode)mode;
}

bool ONF_ActionModifyClustersGroupsOptions02::isAVisible() const
{
    return ui->cb_Avisible->isChecked();
}

bool ONF_ActionModifyClustersGroupsOptions02::isBVisible() const
{
    return ui->cb_Bvisible->isChecked();
}

bool ONF_ActionModifyClustersGroupsOptions02::isTMPVisible() const
{
    return ui->cb_tmpVisible->isChecked();
}

bool ONF_ActionModifyClustersGroupsOptions02::isTrashVisible() const
{
    return ui->cb_trashVisible->isChecked();
}

bool ONF_ActionModifyClustersGroupsOptions02::isOthersVisible() const
{
    return ui->cb_othersVisible->isChecked();
}

bool ONF_ActionModifyClustersGroupsOptions02::isValidatedVisible() const
{
    return ui->cb_showValidated->isChecked();
}

void ONF_ActionModifyClustersGroupsOptions02::on_buttonGroupMode_buttonReleased(int id)
{
    Q_UNUSED(id)

    (dynamic_cast<ONF_ActionModifyClustersGroups02*>(action()))->setSelectionMode(selectionMode());
}

void ONF_ActionModifyClustersGroupsOptions02::on_buttonGroupSelection_buttonReleased(int id)
{
    Q_UNUSED(id)

    (dynamic_cast<ONF_ActionModifyClustersGroups02*>(action()))->setSelectionMode(selectionMode());
}

void ONF_ActionModifyClustersGroupsOptions02::setSelectionMode(GraphicsViewInterface::SelectionMode mode)
{
    if(mode != GraphicsViewInterface::NONE)
    {
        int m = mode;

        while(m > GraphicsViewInterface::REMOVE_ONE)
            m -= GraphicsViewInterface::REMOVE_ONE;       

        if (mode == GraphicsViewInterface::SELECT) {
            ui->toolButtonSelectMulti->setChecked(true);
            ui->toolButtonReplaceMode->setChecked(true);
        } else if (mode == GraphicsViewInterface::ADD) {
            ui->toolButtonSelectMulti->setChecked(true);
            ui->toolButtonAddMode->setChecked(true);
        } else if (mode == GraphicsViewInterface::REMOVE) {
            ui->toolButtonSelectMulti->setChecked(true);
            ui->toolButtonRemoveMode->setChecked(true);
        } else if (mode == GraphicsViewInterface::SELECT_ONE) {
            ui->toolButtonSelectOne->setChecked(true);
            ui->toolButtonReplaceMode->setChecked(true);
        } else if (mode == GraphicsViewInterface::ADD_ONE) {
            ui->toolButtonSelectOne->setChecked(true);
            ui->toolButtonAddMode->setChecked(true);
        } else if (mode == GraphicsViewInterface::REMOVE_ONE) {
            ui->toolButtonSelectOne->setChecked(true);
            ui->toolButtonRemoveMode->setChecked(true);
        }
    }
}

void ONF_ActionModifyClustersGroupsOptions02::on_pb_SetSceneA_clicked()
{
    emit selectPositionA();
}

void ONF_ActionModifyClustersGroupsOptions02::on_pb_SetSceneB_clicked()
{
    emit selectPositionB();
}

void ONF_ActionModifyClustersGroupsOptions02::on_pb_ColorA_clicked()
{
    QColor tmp = QColorDialog::getColor(_colorA);
    if (!tmp.isValid()) {return;}
    selectColorA(tmp);
    emit setColorA(tmp);
}

void ONF_ActionModifyClustersGroupsOptions02::selectColorA(QColor color)
{
    _colorA = color;
    ui->pb_SetSceneA->setStyleSheet("QPushButton { background-color: " + _colorA.name() + "; }");
    ui->pb_toA->setStyleSheet("QToolButton { background-color: " + _colorA.name() + "; }");
}

void ONF_ActionModifyClustersGroupsOptions02::on_pb_ColorB_clicked()
{
    QColor tmp = QColorDialog::getColor(_colorB);
    if (!tmp.isValid()) {return;}
    selectColorB(tmp);
    emit setColorB(tmp);
}

void ONF_ActionModifyClustersGroupsOptions02::selectColorB(QColor color)
{
    _colorB = color;
    ui->pb_SetSceneB->setStyleSheet("QPushButton { background-color: " + _colorB.name() + "; }");
    ui->pb_toB->setStyleSheet("QToolButton { background-color: " + _colorB.name() + "; }");
}

void ONF_ActionModifyClustersGroupsOptions02::setMultiSelect(bool multi)
{
    if (multi)
    {
        ui->toolButtonSelectMulti->setChecked(true);
    } else {
        ui->toolButtonSelectOne->setChecked(true);
    }
}

void ONF_ActionModifyClustersGroupsOptions02::toggleOthersVisible()
{
    if (ui->cb_othersVisible->isChecked())
    {
        ui->cb_othersVisible->setChecked(false);
    } else {
        ui->cb_othersVisible->setChecked(true);
    }
}

void ONF_ActionModifyClustersGroupsOptions02::on_pb_toA_clicked()
{
    emit affectClusterToA();
}

void ONF_ActionModifyClustersGroupsOptions02::on_pb_toB_clicked()
{
    emit affectClusterToB();
}

void ONF_ActionModifyClustersGroupsOptions02::on_pb_toTmp_clicked()
{
    emit affectClusterToTMP();
}

void ONF_ActionModifyClustersGroupsOptions02::on_pb_toTrash_clicked()
{
    emit affectClusterToTrash();
}

void ONF_ActionModifyClustersGroupsOptions02::on_cb_Avisible_toggled(bool checked)
{
    Q_UNUSED(checked);
    emit visibilityChanged();
}

void ONF_ActionModifyClustersGroupsOptions02::on_cb_Bvisible_toggled(bool checked)
{
    Q_UNUSED(checked);
    emit visibilityChanged();
}

void ONF_ActionModifyClustersGroupsOptions02::on_cb_othersVisible_toggled(bool checked)
{
    Q_UNUSED(checked);
    emit visibilityChanged();
}

void ONF_ActionModifyClustersGroupsOptions02::on_cb_tmpVisible_toggled(bool checked)
{
    Q_UNUSED(checked);
    emit visibilityChanged();
}

void ONF_ActionModifyClustersGroupsOptions02::on_cb_trashVisible_toggled(bool checked)
{
    Q_UNUSED(checked);
    emit visibilityChanged();
}


void ONF_ActionModifyClustersGroupsOptions02::on_pb_extend_clicked()
{
    emit extend();
}

void ONF_ActionModifyClustersGroupsOptions02::on_pb_validate_clicked()
{
    emit validatePosition();
}

void ONF_ActionModifyClustersGroupsOptions02::on_cb_showValidated_toggled(bool checked)
{
    Q_UNUSED(checked);
    emit visibilityChanged();
}
