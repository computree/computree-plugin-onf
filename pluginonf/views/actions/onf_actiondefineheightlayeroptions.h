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

#ifndef ONF_ACTIONDEFINEHEIGHTLAYEROPTIONS_H
#define ONF_ACTIONDEFINEHEIGHTLAYEROPTIONS_H

#include "ct_view/actions/abstract/ct_gabstractactionoptions.h"

class ONF_ActionDefineHeightLayer;

namespace Ui {
class ONF_ActionDefineHeightLayerOptions;
}

class ONF_ActionDefineHeightLayerOptions : public CT_GAbstractActionOptions
{
    Q_OBJECT

public:

    explicit ONF_ActionDefineHeightLayerOptions(const ONF_ActionDefineHeightLayer *action, float zmin, float zmax, float res, int threshold);
    ~ONF_ActionDefineHeightLayerOptions();

    double getZmin();
    double getZmax();

    void chooseZmin();
    void chooseZmax();
    int getThreshold();
    double getResolution();

public slots:
    void increaseHValue();
    void decreaseHValue();
    void increaseSingleStep();
    void decreaseSingleStep();

private slots:

    void changeSingleStep(int button);

    void on_dsb_zmin_valueChanged(double arg1);
    void on_pb_computeCrownProjection_clicked();

signals:
    void redrawNeeded();
    void zValuesChanged(double zmin, double zmax);
    void askForSideView();
    void askForCrownProjectionComputing();
    void updateResolution(double);
    void updateThreshold(int);

private:
    Ui::ONF_ActionDefineHeightLayerOptions *ui;
};

#endif // ONF_ACTIONDEFINEHEIGHTLAYEROPTIONS_H
