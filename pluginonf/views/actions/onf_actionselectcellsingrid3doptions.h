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

#ifndef ONF_ACTIONSELECTCELLSINGRID3DOPTIONS_H
#define ONF_ACTIONSELECTCELLSINGRID3DOPTIONS_H

#include "ct_view/actions/abstract/ct_gabstractactionoptions.h"
#include "views/actions/onf_actionselectcellsingrid3dcolonizedialog.h"

class ONF_ActionSelectCellsInGrid3D;

namespace Ui {
class ONF_ActionSelectCellsInGrid3DOptions;
}

class ONF_ActionSelectCellsInGrid3DOptions : public CT_GAbstractActionOptions
{
    Q_OBJECT

public:

    enum GridSelectionMode {
        FREEMOVE,
        ADD,
        REMOVE,
        EXTENDS,
        COLONIZE,
        CHANGECENTERCELL
    };

    enum CumulateMode {
        MEAN,
        MAX,
        SUM,
        NUMBER
    };

    explicit ONF_ActionSelectCellsInGrid3DOptions(const ONF_ActionSelectCellsInGrid3D *action, int maxlevel);
    ~ONF_ActionSelectCellsInGrid3DOptions();

    ONF_ActionSelectCellsInGrid3DOptions::GridSelectionMode selectionMode() const;

    int level() const;
    int thickness() const;
    QColor gridColor() const;
    bool showSeletedOnly() const;
    bool isGridOn() const;
    bool relativeRepresentation() const;

    inline bool isXpColonizationAllowed() {return _colonizeDialog.getXp();}
    inline bool isXmColonizationAllowed() {return _colonizeDialog.getXm();}
    inline bool isYpColonizationAllowed() {return _colonizeDialog.getYp();}
    inline bool isYmColonizationAllowed() {return _colonizeDialog.getYm();}
    inline bool isZpColonizationAllowed() {return _colonizeDialog.getZp();}
    inline bool isZmColonizationAllowed() {return _colonizeDialog.getZm();}

private:
    Ui::ONF_ActionSelectCellsInGrid3DOptions *ui;

    int             _maxLevel;
    CumulateMode    _mode;
    QColor          _gridColor;
    ONF_ActionSelectCellsInGrid3DColonizeDialog _colonizeDialog;

public slots:

    void setSelectionMode(ONF_ActionSelectCellsInGrid3DOptions::GridSelectionMode mode);
    void modifyThickness(int modifier);
    void modifyLevel(int modifier);
    void modify2D3DMode();

signals:

    void selectionModeChanged(ONF_ActionSelectCellsInGrid3DOptions::GridSelectionMode mode);
    void levelChanged(int level);
    void thicknessChanged(int thickness);
    void showModeChanged();
    void gridModeChanged();
    void relativeRepresentationChanged();
    void cumulModeChanged(ONF_ActionSelectCellsInGrid3DOptions::CumulateMode);
    void askForUpwardsCopy();
    void askForFullCopy();
    void mode2D3DChanged(bool mode3Dselected);
    void coefChanged(double coef);
    void gridColorChanged(QColor color);
    void askForSelectionOfNotEmptyCells();
    void askForConvexHull();

private slots:
    void on_sb_level_valueChanged(int arg1);
    void on_pb_all_clicked();
    void on_cb_cumulMode_currentTextChanged(const QString &arg1);
    void on_pb_copyupwards_clicked();
    void on_pb_copyfull_clicked();
    void on_pb_2d3d_clicked(bool checked);
    void on_pb_gridColor_clicked();
    void on_pb_centerOnCell_clicked();
    void on_pb_add_clicked();
    void on_pb_colonize_clicked();
    void on_pb_extends_clicked();
    void on_pb_remove_clicked();
    void on_pb_freemove_clicked();
    void on_pb_colonizeParam_clicked();
    void on_pb_selectNotEmptyCells_clicked();
    void on_pb_convexHull_clicked();
};

#endif // ONF_ACTIONSELECTCELLSINGRID3DOPTIONS_H
