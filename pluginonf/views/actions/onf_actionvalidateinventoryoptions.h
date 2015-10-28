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

#ifndef ONF_ACTIONVALIDATEINVENTORYOPTIONS_H
#define ONF_ACTIONVALIDATEINVENTORYOPTIONS_H

#include "ct_view/actions/abstract/ct_gabstractactionoptions.h"

class ONF_ActionValidateInventory;

namespace Ui {
class ONF_ActionValidateInventoryOptions;
}

class ONF_ActionValidateInventoryOptions : public CT_GAbstractActionOptions
{
    Q_OBJECT

public:

    enum Mode {
        Mode_Item,
        Mode_attributes
    };

    explicit ONF_ActionValidateInventoryOptions(const ONF_ActionValidateInventory *action);
    ~ONF_ActionValidateInventoryOptions();

    bool shouldAutoCenterCamera();
    bool shouldShowData();
    bool shouldShowCandidateItems();

    ONF_ActionValidateInventoryOptions::Mode getMode();
    void toggleMode();

private:
    Ui::ONF_ActionValidateInventoryOptions *ui;

signals:
    void parametersChanged();
    void pointsSizeChanged(int);

private slots:

    void on_rb_dbh_toggled(bool checked);
    void on_cb_showData_toggled(bool checked);
    void on_cb_showCandidateItems_toggled(bool checked);
};

#endif // ONF_ActionValidateInventoryOptions_H
