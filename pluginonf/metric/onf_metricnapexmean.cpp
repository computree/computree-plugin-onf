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

#include "onf_metricnapexmean.h"
#include "ct_pointcloudindex/ct_pointcloudindexvector.h"
#include "ct_iterator/ct_pointiterator.h"
#include "ct_math/ct_mathstatistics.h"
#include "ct_view/ct_genericconfigurablewidget.h"

#define checkAndSetValue(ATT, NAME, TYPE) if((value = group->firstValueByTagName(NAME)) == NULL) { return false; } else { ATT = value->value().value<TYPE>(); }

ONF_MetricNApexMean::ONF_MetricNApexMean() : CT_AbstractMetric_XYZ()
{
    _nApex = 6;
    _percHmax = 0.70;

    declareAttributesVaB();
}

ONF_MetricNApexMean::ONF_MetricNApexMean(const ONF_MetricNApexMean &other) : CT_AbstractMetric_XYZ(other)
{
    declareAttributesVaB();

    _nApex      = other._nApex;
    _percHmax   = other._percHmax;
}

QString ONF_MetricNApexMean::getShortDescription() const
{
    return tr("Moyenne des N plus hauts apex");
}

CT_AbstractConfigurableWidget* ONF_MetricNApexMean::createConfigurationWidget()
{
    CT_GenericConfigurableWidget* configDialog = new CT_GenericConfigurableWidget();

    configDialog->addInt(tr("Nombre apex"), "", 0, 10000, _nApex);
    configDialog->addDouble(tr("%Hmax en dessous duquel les apex ne sont plus pris en compte"), "%", 0, 100, 1, _percHmax, 100);

    return configDialog;
}

SettingsNodeGroup *ONF_MetricNApexMean::getAllSettings() const
{
    SettingsNodeGroup *root = CT_AbstractMetric_XYZ::getAllSettings();
    SettingsNodeGroup *group = new SettingsNodeGroup("ONF_MetricNApexMean");

    group->addValue(new SettingsNodeValue("nApex", _nApex));
    group->addValue(new SettingsNodeValue("percHmax", _percHmax));

    return root;
}

bool ONF_MetricNApexMean::setAllSettings(const SettingsNodeGroup *settings)
{
    if(!CT_AbstractMetric_XYZ::setAllSettings(settings))
        return false;

    SettingsNodeGroup *group = settings->firstGroupByTagName("ONF_MetricNApexMean");

    if(group == NULL)
        return false;

    SettingsNodeValue *value = NULL;
    checkAndSetValue(_nApex, "nApex", int)
    checkAndSetValue(_percHmax, "percHmax", double)

    return true;
}

void ONF_MetricNApexMean::createAttributes()
{    
    CT_AbstractMetric_XYZ::createAttributes();
}

void ONF_MetricNApexMean::computeMetric()
{
    QList<double> values;

    size_t i = 0;

    CT_PointIterator itP(pointCloud());
    while(itP.hasNext())
    {
        const CT_Point& point = itP.next().currentPoint();

        if ((plotArea() == NULL) || plotArea()->contains(point(0), point(1))) {
            values.append(point(2));
            ++i;
        }
    }

    if (i > 0) {
        qSort(values.begin(), values.end());

        double limit = values.last() * _percHmax;
        _nApexUsed.value = 0;
        _hmean.value = 0;

        for (int a = 0 ; a < _nApex && a < values.size(); a++)
        {
            double val = values.at(values.size() - a - 1);
            if (val >= limit)
            {
                _nApexUsed.value++;
                _hmean.value += val;
            }
        }

        if (_nApexUsed.value > 0)
        {
            _hmean.value /= _nApexUsed.value;
        }

        setAttributeValueVaB(_hmean);
        setAttributeValueVaB(_nApexUsed);
    }
}

CT_AbstractConfigurableElement* ONF_MetricNApexMean::copy() const
{
    return new ONF_MetricNApexMean(*this);
}

void ONF_MetricNApexMean::declareAttributesVaB()
{
    registerAttributeVaB(_hmean, CT_AbstractCategory::DATA_NUMBER, tr("HmApex"));
    registerAttributeVaB(_nApexUsed, CT_AbstractCategory::DATA_NUMBER, tr("nApex"));
}


