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

#ifndef ONF_ACTIONSLICEPOINTCLOUDOPTIONS_H
#define ONF_ACTIONSLICEPOINTCLOUDOPTIONS_H

#include "ct_view/actions/abstract/ct_gabstractactionforgraphicsviewoptions.h"

class ONF_ActionSlicePointCloud;

namespace Ui {
class ONF_ActionSlicePointCloudOptions;
}

class ONF_ActionSlicePointCloudOptions : public CT_GAbstractActionForGraphicsViewOptions
{
    Q_OBJECT

public:

    explicit ONF_ActionSlicePointCloudOptions(const ONF_ActionSlicePointCloud *action);
    ~ONF_ActionSlicePointCloudOptions();

    double getThickness() const;
    double getSpacing() const;
    double getIncrement() const;

    void setThickness(double t) const;
    void setSpacing(double s) const;

    void increaseIncrement();
    void decreaseIncrement();

private:
    Ui::ONF_ActionSlicePointCloudOptions *ui;

signals:
    void parametersChanged();

private slots:
    void changeSingleStep(int button);

};

#endif // ONF_ACTIONSLICEPOINTCLOUDOPTIONS_H
