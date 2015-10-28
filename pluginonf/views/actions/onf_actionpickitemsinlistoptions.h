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

#ifndef ONF_ACTIONPICKITEMSINLISTOPTIONS_H
#define ONF_ACTIONPICKITEMSINLISTOPTIONS_H

#include "ct_view/actions/abstract/ct_gabstractactionoptions.h"

class ONF_ActionPickItemsInList;

namespace Ui {
class ONF_ActionPickItemsInListOptions;
}

class ONF_ActionPickItemsInListOptions : public CT_GAbstractActionOptions
{
    Q_OBJECT

public:

    explicit ONF_ActionPickItemsInListOptions(const ONF_ActionPickItemsInList *action);
    ~ONF_ActionPickItemsInListOptions();

    QColor getColor();
    float getMaxDist();
    bool souldAutoCenterCamera();

private:
    Ui::ONF_ActionPickItemsInListOptions *ui;

signals:
    void parametersChanged();
    void allClicked();
    void noneClicked();

private slots:
    void on_onf_color_clicked();

    void on_dsb_maxDist_valueChanged(double arg1);

    void on_onf_all_clicked();

    void on_onf_none_clicked();

private:
    QColor _itemColor;
};

#endif // ONF_ACTIONPICKITEMSINLISTOPTIONS_H
