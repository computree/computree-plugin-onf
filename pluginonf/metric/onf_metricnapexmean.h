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

#ifndef ONF_METRICNAPEXMEAN_H
#define ONF_METRICNAPEXMEAN_H

#include "ctlibmetrics/ct_metric/abstract/ct_abstractmetric_xyz.h"

class ONF_MetricNApexMean : public CT_AbstractMetric_XYZ
{
    Q_OBJECT
public:
    ONF_MetricNApexMean();
    ONF_MetricNApexMean(const ONF_MetricNApexMean &other);

    QString getShortDescription() const;

    SettingsNodeGroup* getAllSettings() const;
    bool setAllSettings(const SettingsNodeGroup *settings);

    void createAttributes();
    void computeMetric();

    CT_AbstractConfigurableWidget* createConfigurationWidget();

    CT_AbstractConfigurableElement* copy() const;

private:
    int    _nApex;
    double _percHmax;

    VaB<double> _hmean;
    VaB<int> _nApexUsed;

    void declareAttributesVaB();

};

#endif // ONF_METRICNAPEXMEAN_H
