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

#include "ctliblas/tools/las/ct_lasdata.h"

ONF_MetricComputeStats::ONF_MetricComputeStats() : CT_AbstractMetric_LAS()
{
}

ONF_MetricComputeStats::ONF_MetricComputeStats(const ONF_MetricComputeStats &other) : CT_AbstractMetric_LAS(other)
{
    m_configAndResults = other.m_configAndResults;
}

QString ONF_MetricComputeStats::getShortDescription() const
{
    return tr("Calcul d'indicateurs de diagnostique");
}

QString ONF_MetricComputeStats::getDetailledDescription() const
{
    return tr("Les valeurs suivantes sont calculées :<br>"
              "- N : Nombre total de points"
              "- N_first : Nombre de points First"
              "- N_last : Nombre de points Last"
              "- N_int : Nombre de points Intermediate"
              "- N_only : Nombre de points Only"
              "- N_error : Nombre de points en Erreur (nombre de retours ou numéro de retour aberrant)"
              "- N_ground : Nombre de points Sol"
              "- N_veg : Nombre de Végétation "
              "- N_other : Nombre de points Autres (ni Végétation, ni Sol)"
              "- Range : Zmax - Zmin"
              "- NumberOfLines : Nombre de lignes vols couvrant la placette"
              "- N_bestLine : Nombre de points de la ligne de vols ayant le plus de points"
              "- N_secondLine : Nombre de points de la seconde ligne de vols ayant le plus de points"
              "- N_worstLine : Nombre de points de la ligne de vols ayant le moins de points");
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

void ONF_MetricComputeStats::createAttributes()
{
    addAttribute<size_t>(m_configAndResults.n, CT_AbstractCategory::DATA_NUMBER, tr("N"));
    addAttribute<size_t>(m_configAndResults.n_first, CT_AbstractCategory::DATA_NUMBER, tr("N_first"));
    addAttribute<size_t>(m_configAndResults.n_last, CT_AbstractCategory::DATA_NUMBER, tr("N_last"));
    addAttribute<size_t>(m_configAndResults.n_intermediate, CT_AbstractCategory::DATA_NUMBER, tr("N_int"));
    addAttribute<size_t>(m_configAndResults.n_only, CT_AbstractCategory::DATA_NUMBER, tr("N_only"));
    addAttribute<size_t>(m_configAndResults.n_error, CT_AbstractCategory::DATA_NUMBER, tr("N_error"));
    addAttribute<size_t>(m_configAndResults.n_ground, CT_AbstractCategory::DATA_NUMBER, tr("N_ground"));
    addAttribute<size_t>(m_configAndResults.n_vegetation, CT_AbstractCategory::DATA_NUMBER, tr("N_veg"));
    addAttribute<size_t>(m_configAndResults.n_others, CT_AbstractCategory::DATA_NUMBER, tr("N_other"));
    addAttribute<double>(m_configAndResults.max_m_min, CT_AbstractCategory::DATA_NUMBER, tr("Range"));
    addAttribute<size_t>(m_configAndResults.numberOfLines, CT_AbstractCategory::DATA_NUMBER, tr("NumberOfLines"));
    addAttribute<size_t>(m_configAndResults.nBestLine, CT_AbstractCategory::DATA_NUMBER, tr("N_bestLine"));
    addAttribute<size_t>(m_configAndResults.nSecondLine, CT_AbstractCategory::DATA_NUMBER, tr("N_secondLine"));
    addAttribute<size_t>(m_configAndResults.nWorstLine, CT_AbstractCategory::DATA_NUMBER, tr("N_worstLine"));

    CT_AbstractMetric_LAS::createAttributes();
}

void ONF_MetricComputeStats::computeMetric()
{    
    m_configAndResults.n = 0;
    m_configAndResults.n_first = 0;
    m_configAndResults.n_last = 0;
    m_configAndResults.n_intermediate = 0;
    m_configAndResults.n_only = 0;
    m_configAndResults.n_error = 0;

    m_configAndResults.n_ground = 0;
    m_configAndResults.n_vegetation = 0;
    m_configAndResults.n_others = 0;

    m_configAndResults.max_m_min = NAN;
    m_configAndResults.numberOfLines = 0;
    m_configAndResults.nBestLine = 0;
    m_configAndResults.nSecondLine = 0;
    m_configAndResults.nWorstLine = 0;

    double max = -std::numeric_limits<double>::max();
    double min = std::numeric_limits<double>::max();

    QMap<quint16, size_t> linesCounts;

    CT_PointIterator itP(pointCloud());
    while(itP.hasNext())
    {
        const CT_Point& point = itP.next().currentPoint();

        if ((plotArea() == NULL) || plotArea()->contains(point(0), point(1)))
        {
            ++m_configAndResults.n;

            size_t index = itP.currentGlobalIndex();
            if (lasPointCloudIndex()->contains(index))
            {
                size_t lasIndex = lasPointCloudIndex()->indexOf(index);
                CT_LASData lasData;
                lasAttributes()->getLASDataAt(lasIndex, lasData);

                if (lasData._Return_Number == 1 && lasData._Number_of_Returns >= 1) {++m_configAndResults.n_first;}
                if (lasData._Return_Number == 1 && lasData._Number_of_Returns == 1) {++m_configAndResults.n_only;}
                if (lasData._Return_Number != 1 && lasData._Return_Number == lasData._Number_of_Returns && lasData._Number_of_Returns > 1) {++m_configAndResults.n_last;}
                if (lasData._Return_Number > 1 && lasData._Return_Number != lasData._Number_of_Returns && lasData._Number_of_Returns > 1) {++m_configAndResults.n_intermediate;}
                if (lasData._Return_Number < 1 || lasData._Return_Number < 1 || lasData._Return_Number > lasData._Return_Number) {++m_configAndResults.n_error;}

                if (lasData._Classification == 2) {++m_configAndResults.n_ground;}
                else if (lasData._Classification == 3 || lasData._Classification == 4 || lasData._Classification == 5) {++m_configAndResults.n_vegetation;}
                else {++m_configAndResults.n_others;}

                if (point(2) > max) {max = point(2);}
                if (point(2) < min) {min = point(2);}

                linesCounts[lasData._Point_Source_ID] = linesCounts.value(lasData._Point_Source_ID, 0) + 1;
            }
        }
    }

    QList<size_t> counts = linesCounts.values();
    qSort(counts.begin(), counts.end());

    m_configAndResults.numberOfLines = linesCounts.size();

    if (counts.size() > 1)
    {
        m_configAndResults.nBestLine = counts.last();
    } else {
        m_configAndResults.nBestLine = 0;
    }

    if (counts.size() > 1)
    {
        m_configAndResults.nSecondLine = counts.at(counts.size() - 2);
        m_configAndResults.nWorstLine  = counts.first();
    } else {
        m_configAndResults.nSecondLine = 0;
        m_configAndResults.nWorstLine  = 0;
    }

    if (m_configAndResults.n > 0)
    {
        m_configAndResults.max_m_min = max - min;
    }

    setAttributeValue(m_configAndResults.n);

    setAttributeValue(m_configAndResults.n_first);
    setAttributeValue(m_configAndResults.n_last);
    setAttributeValue(m_configAndResults.n_intermediate);
    setAttributeValue(m_configAndResults.n_only);
    setAttributeValue(m_configAndResults.n_error);

    setAttributeValue(m_configAndResults.n_ground);
    setAttributeValue(m_configAndResults.n_vegetation);
    setAttributeValue(m_configAndResults.n_others);

    setAttributeValue(m_configAndResults.max_m_min);
    setAttributeValue(m_configAndResults.numberOfLines);
    setAttributeValue(m_configAndResults.nBestLine);
    setAttributeValue(m_configAndResults.nSecondLine);
    setAttributeValue(m_configAndResults.nWorstLine);
}
