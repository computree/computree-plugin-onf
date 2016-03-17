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

#include "onf_metricquantiles.h"
#include "ct_pointcloudindex/ct_pointcloudindexvector.h"
#include "ct_iterator/ct_pointiterator.h"
#include "ct_math/ct_mathstatistics.h"
#include "ct_view/ct_genericconfigurablewidget.h"

#define checkAndSetValue(ATT, NAME, TYPE) if((value = group->firstValueByTagName(NAME)) == NULL) { return false; } else { ATT = value->value().value<TYPE>(); }

ONF_MetricQuantiles::ONF_MetricQuantiles() : CT_AbstractMetric_XYZ()
{
    _quantMin = 0.05;
    _quantMax = 0.95;
    _quantStep = 0.05;
    _prefix = "H";

    declareAttributesVaB();
}

ONF_MetricQuantiles::ONF_MetricQuantiles(const ONF_MetricQuantiles &other) : CT_AbstractMetric_XYZ(other)
{
    declareAttributesVaB();

    _quantMin  = other._quantMin;
    _quantMax  = other._quantMax;
    _quantStep = other._quantStep;
    _prefix    = other._prefix;
    _hmin      = other._hmin;
    _hmed      = other._hmed;
    _h99       = other._h99;
    _hmax      = other._hmax;
}

QString ONF_MetricQuantiles::getShortDescription() const
{
    return tr("Calcul de quantiles sur les coordonnées Z des points");
}

CT_AbstractConfigurableWidget* ONF_MetricQuantiles::createConfigurationWidget()
{
    CT_GenericConfigurableWidget* configDialog = new CT_GenericConfigurableWidget();

    configDialog->addTitle(tr("Paramétrage des quantiles à calculer :"));
    configDialog->addDouble(tr("- A partir de"), "%", 0, 100, 0, _quantMin, 100);
    configDialog->addDouble(tr("- Jusqu'à"), "%", 0, 100, 0, _quantMax, 100);
    configDialog->addDouble(tr("- Avec un pas de"), "%", 0, 100, 0, _quantStep, 100);
    configDialog->addString(tr("Préfixe à utiliser (ex : P99, Q99, H99, Z99,...)"), "", _prefix);
    configDialog->addEmpty();
    configDialog->addTitle(tr("Métriques complémentaires :"));
    configDialog->addBool(tr("Minimum (Hmin ~ H00)"), "", "", _hmin.used);
    configDialog->addBool(tr("Médiane (Hmed ~ H50)"), "", "", _hmed.used);
    configDialog->addBool(tr("Percentile 99  (H99)"), "", "",  _h99.used);
    configDialog->addBool(tr("Maximum (Hmax ~ H100)"), "", "", _hmax.used);

    return configDialog;
}

SettingsNodeGroup *ONF_MetricQuantiles::getAllSettings() const
{
    SettingsNodeGroup *root = CT_AbstractMetric_XYZ::getAllSettings();
    SettingsNodeGroup *group = new SettingsNodeGroup("ONF_MetricQuantiles");

    group->addValue(new SettingsNodeValue("quantMin", _quantMin));
    group->addValue(new SettingsNodeValue("quantMax", _quantMax));
    group->addValue(new SettingsNodeValue("quantStep", _quantStep));
    group->addValue(new SettingsNodeValue("prefix", _prefix));

    return root;
}

bool ONF_MetricQuantiles::setAllSettings(const SettingsNodeGroup *settings)
{
    if(!CT_AbstractMetric_XYZ::setAllSettings(settings))
        return false;

    SettingsNodeGroup *group = settings->firstGroupByTagName("ONF_MetricQuantiles");

    if(group == NULL)
        return false;

    SettingsNodeValue *value = NULL;
    checkAndSetValue(_quantMin, "quantMin", double)
    checkAndSetValue(_quantMax, "quantMax", double)
    checkAndSetValue(_quantStep, "quantStep", double)
    checkAndSetValue(_prefix, "prefix", QString)

    return true;
}

void ONF_MetricQuantiles::createAttributes()
{    
    if(_quantMin > _quantMax)
        qSwap(_quantMin, _quantMax);

    int nb = (_quantMax - _quantMin) / _quantStep + 1;
    _quantile.resize(nb+1);

    for (int i = 0 ; i <= nb ; i++)
        addAttribute<double>(_quantile[i], CT_AbstractCategory::DATA_NUMBER, getQuantileString(_quantMin + i*_quantStep));

    CT_AbstractMetric_XYZ::createAttributes();
}

void ONF_MetricQuantiles::computeMetric()
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

    if(i > 0) {
        qSort(values.begin(), values.end());

        int nb = (_quantMax - _quantMin) / _quantStep + 1;
        for (int i = 0 ; i <= nb ; i++)
        {
            double quant = _quantMin + i*_quantStep;
            _quantile[i] = CT_MathStatistics::computeQuantile(values, quant, false);
            setAttributeValue(_quantile[i]);
        }

        _hmin.value = values.first();
        _hmed.value = CT_MathStatistics::computeQuantile(values, 0.50, false);
        _h99.value = CT_MathStatistics::computeQuantile(values, 0.99, false);
        _hmax.value = values.last();

        setAttributeValueVaB(_hmin);
        setAttributeValueVaB(_hmed);
        setAttributeValueVaB(_h99);
        setAttributeValueVaB(_hmax);
    }
}

CT_AbstractConfigurableElement* ONF_MetricQuantiles::copy() const
{
    return new ONF_MetricQuantiles(*this);
}

void ONF_MetricQuantiles::declareAttributesVaB()
{
    registerAttributeVaB(_hmin, CT_AbstractCategory::DATA_NUMBER, tr("Hmin"));
    registerAttributeVaB(_hmed, CT_AbstractCategory::DATA_NUMBER, tr("Hmed"));
    registerAttributeVaB(_h99, CT_AbstractCategory::DATA_NUMBER, tr("H99"));
    registerAttributeVaB(_hmax, CT_AbstractCategory::DATA_NUMBER, tr("Hmax"));
}

QString ONF_MetricQuantiles::getQuantileString(double quantile)
{
    QString number = QString::number(100.0*quantile, 'f', 0);
    if (number.size() <2) {number.prepend("0");}
    return QString("%1%2").arg(_prefix).arg(number);
}
