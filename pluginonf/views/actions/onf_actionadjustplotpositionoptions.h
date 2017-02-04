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

#ifndef ONF_ACTIONADJUSTPLOTPOSITIONOPTIONS_H
#define ONF_ACTIONADJUSTPLOTPOSITIONOPTIONS_H

#include "ct_view/actions/abstract/ct_gabstractactionforgraphicsviewoptions.h"

class ONF_ActionAdjustPlotPosition;

namespace Ui {
class ONF_ActionAdjustPlotPositionOptions;
}

class ONF_ActionAdjustPlotPositionOptions : public CT_GAbstractActionForGraphicsViewOptions
{
    Q_OBJECT

public:

    explicit ONF_ActionAdjustPlotPositionOptions(const ONF_ActionAdjustPlotPosition *action);
    ~ONF_ActionAdjustPlotPositionOptions();

    void setTranslation(double dx, double dy);
    bool isFixedHeight() const;
    double fixedHeight() const;
    double translationIncrement() const;
    bool hidePointsOutsideLimits() const;

private:
    Ui::ONF_ActionAdjustPlotPositionOptions *ui;

signals:
    void parametersChanged(double deltaX, double deltaY, bool circles, bool fixedH, double h);
    void colorizationChanged(bool intensity, int min, int max);

    void askForTranslation(bool reset);
    void setGradient(bool intensity, QString name, int min, int max);
    void changeHighlightedNumber(int n);

private slots:
    void on_pb_up_clicked();
    void on_pb_down_clicked();
    void on_pb_left_clicked();
    void on_pb_right_clicked();
    void on_cb_circles_toggled(bool checked);
    void on_cb_fixedH_toggled(bool checked);
    void on_dsb_height_valueChanged(double arg1);
    void on_cb_intensity_toggled(bool checked);
    void on_hs_min_valueChanged(int value);
    void on_hs_max_valueChanged(int value);
    void on_pb_translate_clicked();
    void on_pb_reset_clicked();
    void on_pb_grey_clicked();
    void on_pb_hot_clicked();
    void on_pb_rainbow_clicked();
    void on_pb_hsv_clicked();
    void on_sb_highlightedNumber_valueChanged(int arg1);
    void on_cb_hidePoints_toggled(bool checked);
};

#endif // ONF_ACTIONADJUSTPLOTPOSITIONOPTIONS_H
