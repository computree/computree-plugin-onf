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
    declareAttributes();
}

ONF_MetricComputeStats::ONF_MetricComputeStats(const ONF_MetricComputeStats &other) : CT_AbstractMetric_XYZ(other)
{
    declareAttributes();
    m_configAndResults = other.m_configAndResults;
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

ONF_MetricComputeStats::Config ONF_MetricComputeStats::metricConfiguration() const
{
    return m_configAndResults;
}

void ONF_MetricComputeStats::setMetricConfiguration(const ONF_MetricComputeStats::Config &conf)
{
    m_configAndResults = conf;
}

CT_AbstractConfigurableElement *ONF_MetricComputeStats::copy() const
{
    return new ONF_MetricComputeStats(*this);
}

void ONF_MetricComputeStats::declareAttributes()
{
    registerAttributeVaB(m_configAndResults.valHmean, CT_AbstractCategory::DATA_Z, tr("H mean"));
    registerAttributeVaB(m_configAndResults.valHsd, CT_AbstractCategory::DATA_Z, tr("H sd"));
    registerAttributeVaB(m_configAndResults.valHskew, CT_AbstractCategory::DATA_Z, tr("H skew"));
    registerAttributeVaB(m_configAndResults.valHkurt, CT_AbstractCategory::DATA_Z, tr("H kurt"));
    registerAttributeVaB(m_configAndResults.valHcv, CT_AbstractCategory::DATA_Z, tr("H cv"));
}

void ONF_MetricComputeStats::createAttributes()
{
    addAttribute<size_t>(m_configAndResults.n, CT_AbstractCategory::DATA_NUMBER, tr("N"));

    CT_AbstractMetric_XYZ::createAttributes();
}

void ONF_MetricComputeStats::computeMetric()
{    
    m_configAndResults.valHmean.value = 0;
    m_configAndResults.valHsd.value = 0;
    m_configAndResults.valHskew.value = 0;
    m_configAndResults.valHkurt.value = 0;
    m_configAndResults.valHcv.value = 0;
    m_configAndResults.n = 0;
    double m2 = 0;
    double m3 = 0;
    double m4 = 0;

    CT_PointIterator itP(pointCloud());
    std::vector<double> zValues(pointCloud()->size());
    double zValue;
    while(itP.hasNext())
    {
        const CT_Point& point = itP.next().currentPoint();

        if ((plotArea() == NULL) || plotArea()->contains(point(0), point(1)))
        {
            zValue = point(CT_Point::Z);
            zValues[m_configAndResults.n] = zValue;
            m_configAndResults.valHmean.value += zValue;
            ++m_configAndResults.n;
        }
    }

    zValues.resize(m_configAndResults.n);

    if (m_configAndResults.n > 1)
    {
        double nd = (double)m_configAndResults.n;

        m_configAndResults.valHmean.value /= nd;

        if (m_configAndResults.valHskew.used || m_configAndResults.valHkurt.used)
        {
            for(size_t i=0; i<m_configAndResults.n; ++i) {
                zValue = zValues[i] - m_configAndResults.valHmean.value;
                m2 += pow(zValue, 2.0);
                m3 += pow(zValue, 3.0);
                m4 += pow(zValue, 4.0);
            }

            m2 /= nd;
            m3 /= nd;
            m4 /= nd;

            m_configAndResults.valHsd.value = sqrt((nd/(nd-1))*m2);

            m_configAndResults.valHcv.value = m_configAndResults.valHsd.value / m_configAndResults.valHmean.value;

            m_configAndResults.valHskew.value = m3 / pow(m2, 3.0/2.0);
            m_configAndResults.valHskew.value *= sqrt(nd*(nd-1.0))/(nd-2.0); // SAS & SPSS
            //valHskew *= pow((nd-1.0)/nd, 3.0/2.0); // R package e1071

            m_configAndResults.valHkurt.value = m4 / pow(m2, 2.0) - 3.0;
            m_configAndResults.valHkurt.value = ((nd+1.0)*m_configAndResults.valHkurt.value + 6.0)*(nd-1.0)/((nd-2.0)*(nd-3.0)); // SAS & SPSS
            //valHkurt = (valHkurt + 3)*pow(1.0-1.0/nd, 2.0) - 3.0; // R package e1071
        }
    }

    setAttributeValue(m_configAndResults.n);
    setAttributeValueVaB(m_configAndResults.valHmean);
    setAttributeValueVaB(m_configAndResults.valHsd);
    setAttributeValueVaB(m_configAndResults.valHskew);
    setAttributeValueVaB(m_configAndResults.valHkurt);
    setAttributeValueVaB(m_configAndResults.valHcv);
}
