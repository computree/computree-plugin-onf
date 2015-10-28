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

#ifndef ONF_ACTIONMODIFYCLUSTERSGROUPSOPTIONS_H
#define ONF_ACTIONMODIFYCLUSTERSGROUPSOPTIONS_H

#include "ct_view/actions/abstract/ct_gabstractactionoptions.h"
#include "actions/onf_actionmodifyclustersgroups.h"

namespace Ui {
class ONF_ActionModifyClustersGroupsOptions;
}

class ONF_ActionModifyClustersGroupsOptions : public CT_GAbstractActionOptions
{
    Q_OBJECT

public:

    explicit ONF_ActionModifyClustersGroupsOptions(const ONF_ActionModifyClustersGroups *action);
    ~ONF_ActionModifyClustersGroupsOptions();

    GraphicsViewInterface::SelectionMode selectionMode() const;

    bool isAVisible() const;
    bool isBVisible() const;
    bool isTMPVisible() const;
    bool isTrashVisible() const;
    bool isOthersVisible() const;

    bool isInSceneMode() const;
    bool isInLimitMode() const;
    bool isInSplitMode() const;

    void selectColorA(QColor color);
    void selectColorB(QColor color);

    void selectSceneMode();
    void selectLimitMode();
    void selectSplitMode();

    void setMultiSelect(bool multi);
    void setDistance(int val);
    void setMaxDistance(int val);
protected:
    QColor  _colorA;
    QColor  _colorB;

private:
    Ui::ONF_ActionModifyClustersGroupsOptions *ui;

private slots:
    void on_buttonGroupSelection_buttonReleased(int id);
    void on_buttonGroupType_buttonReleased(int id);
    void on_buttonGroupMode_buttonReleased(int id);

    void on_onf_SetSceneA_clicked();
    void on_onf_SetSceneB_clicked();

    void on_onf_ColorA_clicked();
    void on_onf_ColorB_clicked();

    void on_onf_toA_clicked();
    void on_onf_toB_clicked();
    void on_onf_toTmp_clicked();
    void on_onf_toTrash_clicked();

    void on_cb_Avisible_toggled(bool checked);
    void on_cb_Bvisible_toggled(bool checked);
    void on_cb_othersVisible_toggled(bool checked);
    void on_cb_tmpVisible_toggled(bool checked);
    void on_cb_trashVisible_toggled(bool checked);

    void modeChanged(int button);
    void byChanged(int button);

    void on_sb_LengthOnAxis_valueChanged(int arg1);

public slots:

    void setSelectionMode(GraphicsViewInterface::SelectionMode mode);

signals:

    void selectionModeChanged(GraphicsViewInterface::SelectionMode mode);
    void selectPositionA();
    void selectPositionB();
    void setColorA(QColor color);
    void setColorB(QColor color);

    void affectClusterToA();
    void affectClusterToB();
    void affectClusterToTMP();
    void affectClusterToTrash();

    void visibilityChanged();

    void enterLimitMode();
    void distanceChanged(int val);

};

#endif // ONF_ACTIONMODIFYCLUSTERSGROUPSOPTIONS_H
