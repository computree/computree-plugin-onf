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

#ifndef ONF_ACTIONMODIFYPOSITIONS2DOPTIONS_H
#define ONF_ACTIONMODIFYPOSITIONS2DOPTIONS_H

#include "ct_view/actions/abstract/ct_gabstractactionoptions.h"

class ONF_ActionModifyPositions2D;

namespace Ui {
class ONF_ActionModifyPositions2DOptions;
}

class ONF_ActionModifyPositions2DOptions : public CT_GAbstractActionOptions
{
    Q_OBJECT

public:

    explicit ONF_ActionModifyPositions2DOptions(const ONF_ActionModifyPositions2D *action);
    ~ONF_ActionModifyPositions2DOptions();

    bool isFreeMoveSelected();
    bool isMovePositionSelected();
    bool isAddPositionSelected();
    bool isRemovePositionSelected();

    double getZValue();
    bool isDrawPlaneSelected();
    bool isDrawLinesSelected();
    bool isUpdateLinesSelected();

    void selectFreeMove();
    void selectMovePosition();
    void selectAddPosition();
    void selectRemovePosition();

    void increaseZValue();
    void decreaseZValue();

private:
    Ui::ONF_ActionModifyPositions2DOptions *ui;

signals:
    void parametersChanged();

private slots:
    void on_pb_freeMove_clicked();
    void on_pb_move_clicked();
    void on_pb_add_clicked();
    void on_pb_remove_clicked();
    void on_dsb_zval_valueChanged(double arg1);
    void singleStepChanged(int button);
    void on_cb_drawPlane_toggled(bool checked);
    void on_cb_drawLines_toggled(bool checked);

};

#endif // ONF_ACTIONMODIFYPOSITIONS2DOPTIONS_H
