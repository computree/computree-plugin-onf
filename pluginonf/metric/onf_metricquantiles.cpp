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

ONF_MetricQuantiles::ONF_MetricQuantiles() : CT_AbstractMetric_XYZ()
{
    _quantMin = 0.05;
    _quantMax = 0.95;
    _quantStep = 0.05;
    _prefix = "H";
    _hmin = true;
    _hmed = true;
    _h99 = true;
    _hmax = true;
}

ONF_MetricQuantiles::ONF_MetricQuantiles(const ONF_MetricQuantiles *other) : CT_AbstractMetric_XYZ(other)
{
}

QString ONF_MetricQuantiles::getName()
{
    return QString("ONF_Quantiles");
}

void ONF_MetricQuantiles::createConfigurationDialog()
{
    CT_StepConfigurableDialog* configDialog = addConfigurationDialog();

    configDialog->addTitle(tr("Paramétrage des quantiles à calculer :"));
    configDialog->addDouble(tr("- A partir de"), "%", 0, 100, 0, _quantMin, 100);
    configDialog->addDouble(tr("- Jusqu'à"), "%", 0, 100, 0, _quantMax, 100);
    configDialog->addDouble(tr("- Avec un pas de"), "%", 0, 100, 0, _quantStep, 100);
    configDialog->addString(tr("Préfixe à utiliser (ex : P99, Q99, H99, Z99,...)"), "", _prefix);
    configDialog->addEmpty();
    configDialog->addTitle(tr("Métriques complémentaires :"));
    configDialog->addBool(tr("Minimum (Hmin ~ H00)"), "", "", _hmin);
    configDialog->addBool(tr("Médiane (Hmed ~ H50)"), "", "", _hmed);
    configDialog->addBool(tr("Percentile 99  (H99)"), "", "",  _h99);
    configDialog->addBool(tr("Maximum (Hmax ~ H100)"), "", "", _hmax);
}

void ONF_MetricQuantiles::updateParamtersAfterConfiguration()
{
}

QString ONF_MetricQuantiles::getParametersAsString() const
{
    QString result = "";
    result.append(QString("%4;%1;%2;%3;%5;%6;%7;%8")).arg(_quantMin).arg(_quantMax).arg(_quantStep).arg(_prefix).arg((int)_hmin).arg((int)_hmed).arg((int)_h99).arg((int)_hmax);
    return result;
}

bool ONF_MetricQuantiles::setParametersFromString(QString parameters)
{
    QStringList values = parameters.split(";");
    if (values.size() < 8) {return false;}
    bool ok1, ok2, ok3, ok5, ok6, ok7, ok8;

    _quantMin  = values.at(0).toDouble(&ok1);
    _quantMax  = values.at(1).toDouble(&ok2);
    _quantStep = values.at(2).toDouble(&ok3);
    _prefix = values.at(3); if (_prefix.isEmpty()) {_prefix = "H";}
    _hmin = (bool) values.at(4).toInt(&ok5);
    _hmed = (bool) values.at(5).toInt(&ok6);
    _h99  = (bool) values.at(6).toInt(&ok7);
    _hmax = (bool) values.at(7).toInt(&ok8);

    return (ok1 && ok2 && ok3 && ok5 && ok6 && ok7 && ok8);
}

void ONF_MetricQuantiles::createAttributes()
{    
    int nb = (_quantMax - _quantMin) / _quantStep + 1;
    for (int i = 0 ; i <= nb ; i++)
    {
        addAttribute(getQuantileString(_quantMin + i*_quantStep), CT_AbstractMetric::AT_double, CT_AbstractCategory::DATA_NUMBER);
    }

    for (double i = _quantMin ; i <= _quantMax ; i += _quantStep)
    {
        addAttribute(getQuantileString(i), CT_AbstractMetric::AT_double, CT_AbstractCategory::DATA_NUMBER);
    }

    if (_hmin) {addAttribute("Hmin", CT_AbstractMetric::AT_double, CT_AbstractCategory::DATA_NUMBER);}
    if (_hmed) {addAttribute("Hmed", CT_AbstractMetric::AT_double, CT_AbstractCategory::DATA_NUMBER);}
    if (_h99 ) {addAttribute("H99",  CT_AbstractMetric::AT_double, CT_AbstractCategory::DATA_NUMBER);}
    if (_hmax) {addAttribute("Hmax", CT_AbstractMetric::AT_double, CT_AbstractCategory::DATA_NUMBER);}
}

void ONF_MetricQuantiles::computeMetric()
{
    QList<double> values;

    CT_PointIterator itP(_inCloud);
    while(itP.hasNext())
    {
        const CT_Point& point = itP.next().currentPoint();

        if (_plotArea->contains(point(0), point(1))) {values.append(point(2));}
    }

    qSort(values.begin(), values.end());

    double val;
    int nb = (_quantMax - _quantMin) / _quantStep + 1;
    for (int i = 0 ; i <= nb ; i++)
    {
        double quant = _quantMin + i*_quantStep;
        val = CT_MathStatistics::computeQuantile(values, quant, false);
        setAttributeValue(getQuantileString(quant), val);
    }

    if (_hmin) {val = values.first();                                           setAttributeValue("Hmin", val);}
    if (_hmed) {val = CT_MathStatistics::computeQuantile(values, 0.50, false);  setAttributeValue("Hmed", val);}
    if (_h99)  {val = CT_MathStatistics::computeQuantile(values, 0.99, false);  setAttributeValue("H99",  val);}
    if (_hmax) {val = values.last();                                            setAttributeValue("Hmax", val);}
}

QString ONF_MetricQuantiles::getShortDescription() const
{
    return tr("Calcul de quantiles sur les coordonnées Z des points");
}

QString ONF_MetricQuantiles::getDetailledDescription() const
{
    return tr("");
}

CT_AbstractConfigurableElement *ONF_MetricQuantiles::copy() const
{
    ONF_MetricQuantiles* metric = new ONF_MetricQuantiles(this);

    metric->_quantMin  = _quantMin;
    metric->_quantMax  = _quantMax;
    metric->_quantStep = _quantStep;
    metric->_prefix    = _prefix;
    metric->_hmin      = _hmin;
    metric->_hmed      = _hmed;
    metric->_h99       = _h99;
    metric->_hmax      = _hmax;

    return metric;
}

QString ONF_MetricQuantiles::getQuantileString(double quantile)
{
    QString number = QString::number(100.0*quantile, 'f', 0);
    if (number.size() <2) {number.prepend("0");}
    return QString("%1%2").arg(_prefix).arg(number);
}
