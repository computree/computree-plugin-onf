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

#ifndef ONF_ACTIONMANUALINVENTORYOPTIONS_H
#define ONF_ACTIONMANUALINVENTORYOPTIONS_H

#include "ct_view/actions/abstract/ct_gabstractactionoptions.h"

class ONF_ActionManualInventory;

namespace Ui {
class ONF_ActionManualInventoryOptions;
}

class ONF_ActionManualInventoryOptions : public CT_GAbstractActionOptions
{
    Q_OBJECT

public:

    explicit ONF_ActionManualInventoryOptions(const ONF_ActionManualInventory *action);
    ~ONF_ActionManualInventoryOptions();

    bool shouldAutoCenterCamera();

    void chooseSelectMode();
    void chooseDbhMode();
    void chooseAttributesMode();

    bool isSelectModeSelected();
    bool isDbhModeSelected();
    bool isAttributesModeSelected();

    bool isShowDataChecked();
    bool isShowActiveCirclesChecked();
    bool isShowOtherCirclesChecked();
    bool isShowActiveSceneChecked();
    bool isShowOtherScenesChecked();
    bool isShowTrashChecked();
    bool isShowTrashedScenesChecked();
    bool isAutoValidateChecked();

private:
    Ui::ONF_ActionManualInventoryOptions *ui;

signals:
    void modeChanged();
    void visibilityChanged();
    void chooseUpperCircle();
    void chooseLowerCircle();
    void sendToTrash();
    void retrieveFromTrash();
    void sendToValidated();

private slots:

    void on_rb_select_toggled(bool checked);
    void on_rb_dbh_toggled(bool checked);
    void on_rb_attributes_toggled(bool checked);

    void on_cb_showData_toggled(bool checked);
    void on_cb_activeCircles_toggled(bool checked);
    void on_cb_otherCircles_toggled(bool checked);
    void on_cb_activeScene_toggled(bool checked);
    void on_cb_otherScenes_toggled(bool checked);
    void on_pb_upper_clicked();
    void on_pb_lower_clicked();
    void on_pb_toTrash_clicked();
    void on_pb_fromTrash_clicked();
    void on_cb_trash_toggled(bool checked);
    void on_cb_trashScenes_toggled(bool checked);
    void on_pb_toValidated_clicked();
};

#endif // ONF_ACTIONMANUALINVENTORYOPTIONS_H
