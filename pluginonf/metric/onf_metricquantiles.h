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

#ifndef ONF_METRICQUANTILES_H
#define ONF_METRICQUANTILES_H

#include "ct_metric/abstract/ct_abstractmetric_xyz.h"

class ONF_MetricQuantiles : public CT_AbstractMetric_XYZ
{
    Q_OBJECT
public:

    ONF_MetricQuantiles();
    ONF_MetricQuantiles(const ONF_MetricQuantiles* other);

    QString getName();

    void createConfigurationDialog();
    void updateParamtersAfterConfiguration();

    QString getShortDescription() const;
    QString getDetailledDescription() const;

    QString getParametersAsString() const;
    virtual bool setParametersFromString(QString parameters);

    void createAttributes();
    void computeMetric();


    CT_AbstractConfigurableElement* copy() const;

private:
    double _quantMin;
    double _quantMax;
    double _quantStep;
    QString _prefix;
    bool _hmin;
    bool _hmed;
    bool _h99;
    bool _hmax;


    QString getQuantileString(double quantile);
};

#endif // ONF_METRICQUANTILES_H
