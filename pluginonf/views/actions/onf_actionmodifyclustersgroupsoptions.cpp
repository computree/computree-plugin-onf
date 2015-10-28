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

#include "onf_actionmodifyclustersgroupsoptions.h"
#include "ui_onf_actionmodifyclustersgroupsoptions.h"
#include "ct_global/ct_context.h"

#include <QColorDialog>

ONF_ActionModifyClustersGroupsOptions::ONF_ActionModifyClustersGroupsOptions(const ONF_ActionModifyClustersGroups *action) :
    CT_GAbstractActionOptions(action),
    ui(new Ui::ONF_ActionModifyClustersGroupsOptions)
{
    ui->setupUi(this);

    connect(ui->buttonGroupType, SIGNAL(buttonClicked(int)), this, SLOT(modeChanged(int)));
    connect(ui->buttonGroupBy, SIGNAL(buttonClicked(int)), this, SLOT(byChanged(int)));
}

ONF_ActionModifyClustersGroupsOptions::~ONF_ActionModifyClustersGroupsOptions()
{
    delete ui;
}

GraphicsViewInterface::SelectionMode ONF_ActionModifyClustersGroupsOptions::selectionMode() const
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

    if(ui->pb_SplitMode->isChecked()) {
        mode--;
        mode += GraphicsViewInterface::SELECT_POINTS;
    }

    return (GraphicsViewInterface::SelectionMode)mode;
}

bool ONF_ActionModifyClustersGroupsOptions::isAVisible() const
{
    return ui->cb_Avisible->isChecked();
}

bool ONF_ActionModifyClustersGroupsOptions::isBVisible() const
{
    return ui->cb_Bvisible->isChecked();
}

bool ONF_ActionModifyClustersGroupsOptions::isTMPVisible() const
{
    return ui->cb_tmpVisible->isChecked();
}

bool ONF_ActionModifyClustersGroupsOptions::isTrashVisible() const
{
    return ui->cb_trashVisible->isChecked();
}

bool ONF_ActionModifyClustersGroupsOptions::isOthersVisible() const
{
    return ui->cb_othersVisible->isChecked();
}

bool ONF_ActionModifyClustersGroupsOptions::isInSceneMode() const
{
    return ui->pb_SceneMode->isChecked();
}

bool ONF_ActionModifyClustersGroupsOptions::isInLimitMode() const
{
    return ui->pb_LimitMode->isChecked();
}

bool ONF_ActionModifyClustersGroupsOptions::isInSplitMode() const
{
    return ui->pb_SplitMode->isChecked();
}

void ONF_ActionModifyClustersGroupsOptions::on_buttonGroupType_buttonReleased(int id)
{
    Q_UNUSED(id)

    (dynamic_cast<ONF_ActionModifyClustersGroups*>(action()))->setSelectionMode(selectionMode());
}

void ONF_ActionModifyClustersGroupsOptions::on_buttonGroupMode_buttonReleased(int id)
{
    Q_UNUSED(id)

    (dynamic_cast<ONF_ActionModifyClustersGroups*>(action()))->setSelectionMode(selectionMode());
}

void ONF_ActionModifyClustersGroupsOptions::on_buttonGroupSelection_buttonReleased(int id)
{
    Q_UNUSED(id)

    (dynamic_cast<ONF_ActionModifyClustersGroups*>(action()))->setSelectionMode(selectionMode());
}

void ONF_ActionModifyClustersGroupsOptions::setSelectionMode(GraphicsViewInterface::SelectionMode mode)
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

void ONF_ActionModifyClustersGroupsOptions::on_onf_SetSceneA_clicked()
{
    emit selectPositionA();
}

void ONF_ActionModifyClustersGroupsOptions::on_onf_SetSceneB_clicked()
{
    emit selectPositionB();
}

void ONF_ActionModifyClustersGroupsOptions::on_onf_ColorA_clicked()
{
    QColor tmp = QColorDialog::getColor(_colorA);
    if (!tmp.isValid()) {return;}
    selectColorA(tmp);
    emit setColorA(tmp);
}

void ONF_ActionModifyClustersGroupsOptions::selectColorA(QColor color)
{
    _colorA = color;
    ui->pb_ColorA->setStyleSheet("QPushButton { background-color: " + _colorA.name() + "; }");
    ui->pb_toA->setStyleSheet("QToolButton { background-color: " + _colorA.name() + "; }");
    ui->pb_frontierA->setStyleSheet("QPushButton { background-color: " + _colorA.name() + "; }");
}

void ONF_ActionModifyClustersGroupsOptions::on_onf_ColorB_clicked()
{
    QColor tmp = QColorDialog::getColor(_colorB);
    if (!tmp.isValid()) {return;}
    selectColorB(tmp);
    emit setColorB(tmp);
}

void ONF_ActionModifyClustersGroupsOptions::selectColorB(QColor color)
{
    _colorB = color;
    ui->pb_ColorB->setStyleSheet("QPushButton { background-color: " + _colorB.name() + "; }");
    ui->pb_toB->setStyleSheet("QToolButton { background-color: " + _colorB.name() + "; }");
    ui->pb_frontierB->setStyleSheet("QPushButton { background-color: " + _colorB.name() + "; }");
}

void ONF_ActionModifyClustersGroupsOptions::selectSceneMode()
{
    ui->pb_SceneMode->setChecked(true);
    modeChanged(0);
}

void ONF_ActionModifyClustersGroupsOptions::selectLimitMode()
{
    ui->pb_LimitMode->setChecked(true);
    modeChanged(0);
}

void ONF_ActionModifyClustersGroupsOptions::selectSplitMode()
{
    ui->pb_SplitMode->setChecked(true);
    modeChanged(0);
}

void ONF_ActionModifyClustersGroupsOptions::setMultiSelect(bool multi)
{
    if (multi)
    {
        ui->toolButtonSelectMulti->setChecked(true);
    } else {
        ui->toolButtonSelectOne->setChecked(true);
    }
}

void ONF_ActionModifyClustersGroupsOptions::setDistance(int val)
{
    ui->sb_LengthOnAxis->setValue(val);
}

void ONF_ActionModifyClustersGroupsOptions::setMaxDistance(int val)
{
    ui->sb_LengthOnAxis->setMaximum(val);
    ui->sb_LengthOnAxis->setSuffix(QString("/ %1").arg(val));
}

void ONF_ActionModifyClustersGroupsOptions::on_onf_toA_clicked()
{
    emit affectClusterToA();
}

void ONF_ActionModifyClustersGroupsOptions::on_onf_toB_clicked()
{
    emit affectClusterToB();
}

void ONF_ActionModifyClustersGroupsOptions::on_onf_toTmp_clicked()
{
    emit affectClusterToTMP();
}

void ONF_ActionModifyClustersGroupsOptions::on_onf_toTrash_clicked()
{
    emit affectClusterToTrash();
}

void ONF_ActionModifyClustersGroupsOptions::on_cb_Avisible_toggled(bool checked)
{
    Q_UNUSED(checked);
    emit visibilityChanged();
}

void ONF_ActionModifyClustersGroupsOptions::on_cb_Bvisible_toggled(bool checked)
{
    Q_UNUSED(checked);
    emit visibilityChanged();
}

void ONF_ActionModifyClustersGroupsOptions::on_cb_othersVisible_toggled(bool checked)
{
    Q_UNUSED(checked);
    emit visibilityChanged();
}

void ONF_ActionModifyClustersGroupsOptions::on_cb_tmpVisible_toggled(bool checked)
{
    Q_UNUSED(checked);
    emit visibilityChanged();
}

void ONF_ActionModifyClustersGroupsOptions::on_cb_trashVisible_toggled(bool checked)
{
    Q_UNUSED(checked);
    emit visibilityChanged();
}

void ONF_ActionModifyClustersGroupsOptions::modeChanged(int button)
{
    Q_UNUSED(button);

    if (ui->pb_SceneMode->isChecked())
    {
        ui->wid_sceneMode->setVisible(true);
        ui->wid_limitMode->setVisible(false);
    } else if (ui->pb_LimitMode->isChecked())
    {
        ui->wid_sceneMode->setVisible(false);
        ui->wid_limitMode->setVisible(true);

        setSelectionMode(GraphicsViewInterface::SELECT_ONE);

        emit enterLimitMode();

    } else if (ui->pb_SplitMode->isChecked())
    {
        ui->wid_sceneMode->setVisible(false);
        ui->wid_limitMode->setVisible(false);
    }
}

void ONF_ActionModifyClustersGroupsOptions::byChanged(int button)
{
    Q_UNUSED(button);
    if (ui->rb_001->isChecked())
    {
        ui->sb_LengthOnAxis->setSingleStep(1);
    } else if (ui->rb_010->isChecked())
    {
        ui->sb_LengthOnAxis->setSingleStep(10);
    } else  {
        ui->sb_LengthOnAxis->setSingleStep(100);
    }
}

void ONF_ActionModifyClustersGroupsOptions::on_sb_LengthOnAxis_valueChanged(int arg1)
{
    emit distanceChanged(arg1);
}
