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

#ifndef ONF_ACTIONMODIFYCLUSTERSGROUPSOPTIONS02_H
#define ONF_ACTIONMODIFYCLUSTERSGROUPSOPTIONS02_H

#include "ct_view/actions/abstract/ct_gabstractactionoptions.h"
#include "actions/onf_actionmodifyclustersgroups02.h"

namespace Ui {
class ONF_ActionModifyClustersGroupsOptions02;
}

class ONF_ActionModifyClustersGroupsOptions02 : public CT_GAbstractActionOptions
{
    Q_OBJECT

public:

    explicit ONF_ActionModifyClustersGroupsOptions02(const ONF_ActionModifyClustersGroups02 *action);
    ~ONF_ActionModifyClustersGroupsOptions02();

    GraphicsViewInterface::SelectionMode selectionMode() const;

    bool isAVisible() const;
    bool isBVisible() const;
    bool isTMPVisible() const;
    bool isTrashVisible() const;
    bool isOthersVisible() const;
    bool isValidatedVisible() const;

    void selectColorA(QColor color);
    void selectColorB(QColor color);
    void setMultiSelect(bool multi);

    void toggleOthersVisible();
protected:
    QColor  _colorA;
    QColor  _colorB;

private:
    Ui::ONF_ActionModifyClustersGroupsOptions02 *ui;

private slots:
    void on_buttonGroupSelection_buttonReleased(int id);
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

    void on_onf_extend_clicked();

    void on_onf_validate_clicked();

    void on_cb_showValidated_toggled(bool checked);

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

    void extend();
    void validatePosition();

    void visibilityChanged();

    void enterLimitMode();
    void distanceChanged(int val);

};

#endif // ONF_ACTIONMODIFYCLUSTERSGROUPSOPTIONS02_H
