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

#include "metric/onf_metriccomputestats.h"
#include "ct_pointcloudindex/ct_pointcloudindexvector.h"
#include "ct_iterator/ct_pointiterator.h"

ONF_MetricComputeStats::ONF_MetricComputeStats() : CT_AbstractMetric_XYZ()
{
    _computeHmean = true;
    _computeHsd = true;
    _computeHskew = true;
    _computeHkurt = true;
    _computeHcv  = true;
}

ONF_MetricComputeStats::ONF_MetricComputeStats(const ONF_MetricComputeStats *other) : CT_AbstractMetric_XYZ(other)
{
}

QString ONF_MetricComputeStats::getName()
{
    return QString("ONF_Stats");
}

void ONF_MetricComputeStats::createConfigurationDialog()
{
    CT_StepConfigurableDialog* configDialog = addConfigurationDialog();

    configDialog->addBool("Hmean", "", "", _computeHmean);
    configDialog->addBool("Hsd"  , "", "", _computeHsd);
    configDialog->addBool("Hskew", "", "", _computeHskew);
    configDialog->addBool("Hkurt", "", "", _computeHkurt);
    configDialog->addBool("HCV"  , "", "", _computeHcv);
}

void ONF_MetricComputeStats::updateParamtersAfterConfiguration()
{
}

QString ONF_MetricComputeStats::getParametersAsString() const
{
    QString result = "";
    return result;
}

bool ONF_MetricComputeStats::setParametersFromString(QString parameters)
{
    return true;
}

void ONF_MetricComputeStats::createAttributes()
{
    addAttribute("N", CT_AbstractMetric::AT_size_t, CT_AbstractCategory::DATA_NUMBER);
    if (_computeHmean) {addAttribute("Hmean", CT_AbstractMetric::AT_double, CT_AbstractCategory::DATA_Z);}
    if (_computeHsd)   {addAttribute("Hsd", CT_AbstractMetric::AT_double, CT_AbstractCategory::DATA_Z);}
    if (_computeHskew) {addAttribute("Hskew", CT_AbstractMetric::AT_double, CT_AbstractCategory::DATA_Z);}
    if (_computeHkurt) {addAttribute("Hkurt", CT_AbstractMetric::AT_double, CT_AbstractCategory::DATA_Z);}
    if (_computeHcv)   {addAttribute("Hcv", CT_AbstractMetric::AT_double, CT_AbstractCategory::DATA_NUMBER);}
}

void ONF_MetricComputeStats::computeMetric()
{    
    double valHmean = 0;
    double valHsd = 0;
    double valHskew = 0;
    double valHkurt = 0;
    double valHcv = 0;
    double m2 = 0;
    double m3 = 0;
    double m4 = 0;
    size_t n = 0;

    CT_PointIterator itP(_inCloud);
    while(itP.hasNext())
    {
        const CT_Point& point = itP.next().currentPoint();

        if (_plotArea->contains(point(0), point(1)))
        {
            n++;
            if (_computeHmean || _computeHsd   || _computeHskew || _computeHkurt || _computeHcv) {valHmean += point(2);}
        }
    }

    if (n > 1)
    {
        double nd = (double)n;
        if (_computeHmean || _computeHsd   || _computeHskew || _computeHkurt || _computeHcv)
        {
            valHmean /= nd;
        }


        if (_computeHskew || _computeHkurt)
        {
            CT_PointIterator itP2(_inCloud);
            while(itP2.hasNext())
            {
                const CT_Point& point = itP2.next().currentPoint();

                if (_plotArea->contains(point(0), point(1)))
                {
                    double val = point(2) - valHmean;
                    if (_computeHsd || _computeHskew || _computeHkurt) {m2 += pow(val, 2.0);}
                    if (_computeHskew)                                 {m3 += pow(val, 3.0);}
                    if (_computeHkurt)                                 {m4 += pow(val, 4.0);}
                }
            }
            m2 /= nd;
            m3 /= nd;
            m4 /= nd;

            if (_computeHsd || _computeHcv)
            {
                valHsd = sqrt((nd/(nd-1))*m2);
            }

            if (_computeHcv)
            {
                valHcv = valHsd / valHmean;
            }

            if (_computeHskew)
            {
                valHskew = m3 / pow(m2, 3.0/2.0);
                valHskew *= sqrt(nd*(nd-1.0))/(nd-2.0); // SAS & SPSS
                //valHskew *= pow((nd-1.0)/nd, 3.0/2.0); // R package e1071

            }

            if (_computeHkurt)
            {
                valHkurt = m4 / pow(m2, 2.0) - 3.0;
                valHkurt = ((nd+1.0)*valHkurt + 6.0)*(nd-1.0)/((nd-2.0)*(nd-3.0)); // SAS & SPSS
                //valHkurt = (valHkurt + 3)*pow(1.0-1.0/nd, 2.0) - 3.0; // R package e1071
            }
        }
    } else {
        valHsd = 0;
        valHskew = 0;
        valHkurt = 0;
        valHcv = 0;
    }


    setAttributeValue("N", n);
    if (_computeHmean) {setAttributeValue("Hmean", valHmean);}
    if (_computeHsd)   {setAttributeValue("Hsd", valHsd);}
    if (_computeHskew) {setAttributeValue("Hskew", valHskew);}
    if (_computeHkurt) {setAttributeValue("Hkurt", valHkurt);}
    if (_computeHcv)   {setAttributeValue("Hcv", valHcv);}
}

QString ONF_MetricComputeStats::getShortDescription() const
{
    return tr("Calcul des indicateurs statistiques standard");
}

QString ONF_MetricComputeStats::getDetailledDescription() const
{
    return tr("Les valeurs suivantes sont calculées :<br>"
              "- Hmean : Moyenne<br>"
              "- Hsd   : Ecart-type (non biaisé)<br>"
              "_ Hskew : Skewness  (non biaisé)<br>"
              "- Hkurt : Kurtosis  (non biaisé)<br>"
              "- Hcv   : Coefficient de variation<br>"
              "<em>N.B. : Les formules du Skewness et du Kurtosis sont issues du package e1071 de R (versions SAS).</em>");
}

CT_AbstractConfigurableElement *ONF_MetricComputeStats::copy() const
{
    ONF_MetricComputeStats* metric = new ONF_MetricComputeStats(this);
    metric->_computeHmean = _computeHmean;
    metric->_computeHsd   = _computeHsd;
    metric->_computeHskew = _computeHskew;
    metric->_computeHkurt = _computeHkurt;
    metric->_computeHcv   = _computeHcv;

    return metric;
}
