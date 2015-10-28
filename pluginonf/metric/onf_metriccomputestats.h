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

#ifndef ONF_METRICCOMPUTESTATS_H
#define ONF_METRICCOMPUTESTATS_H


#include "ct_metric/abstract/ct_abstractmetric_xyz.h"

class ONF_MetricComputeStats : public CT_AbstractMetric_XYZ
{
    Q_OBJECT
public:

    ONF_MetricComputeStats();
    ONF_MetricComputeStats(const ONF_MetricComputeStats* other);

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
    bool _computeHmean;
    bool _computeHsd;
    bool _computeHskew;
    bool _computeHkurt;
    bool _computeHcv;
};


#endif // ONF_METRICCOMPUTESTATS_H
