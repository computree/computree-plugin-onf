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

#include "metric/onf_metricnbylasclass.h"
#include "ct_pointcloudindex/ct_pointcloudindexvector.h"
#include "ct_iterator/ct_pointiterator.h"

#include "ctliblas/tools/las/ct_lasdata.h"

ONF_MetricNbyLASClass::ONF_MetricNbyLASClass() : CT_AbstractMetric_LAS()
{
    declareAttributes();
}

ONF_MetricNbyLASClass::ONF_MetricNbyLASClass(const ONF_MetricNbyLASClass &other) : CT_AbstractMetric_LAS(other)
{
    declareAttributes();
    m_configAndResults = other.m_configAndResults;
}

QString ONF_MetricNbyLASClass::getShortDescription() const
{
    return tr("Nombre de points par modalité LAS");
}

QString ONF_MetricNbyLASClass::getDetailledDescription() const
{
    return tr("Les valeurs suivantes sont calculées :<br>"
              "- N : Nombre total de points"
              "- N_first : Nombre de points First"
              "- N_last : Nombre de points Last"
              "- N_int : Nombre de points Intermediate"
              "- N_only : Nombre de points Only"
              "- N_error : Nombre de points en Erreur (nombre de retours ou numéro de retour aberrant)"              
              "- N_Cla00_NeverClassified : Nombre de points classés 0"
              "- N_Cla01_Unclassified : Nombre de points classés 1"
              "- N_Cla02_Ground : Nombre de points classés 2"
              "- N_Cla03_LowVegetation : Nombre de points classés 3"
              "- N_Cla04_MediumVegetation : Nombre de points classés 4"
              "- N_Cla05_HighVegetation : Nombre de points classés 5"
              "- N_Cla06_Building : Nombre de points classés 6"
              "- N_Cla07_LowPointNoise : Nombre de points classés 7"
              "- N_Cla08_ModelKeyPoint : Nombre de points classés 8"
              "- N_Cla09_Water : Nombre de points classés 9"
              "- N_Cla10_Rail : Nombre de points classés 10"
              "- N_Cla11_RoadSurface : Nombre de points classés 11"
              "- N_Cla12_OverlapPoint : Nombre de points classés 12"
              "- N_Cla13_WireGuard : Nombre de points classés 13"
              "- N_Cla14_WireConductor : Nombre de points classés 14"
              "- N_Cla15_TransmissionTower : Nombre de points classés 15"
              "- N_Cla16_WireStructureConnector : Nombre de points classés 16"
              "- N_Cla17_BridgeDeck : Nombre de points classés 17"
              "- N_Cla18_HighNoise : Nombre de points classés 18"
              "- N_Cla19_63_Reserved : Nombre de points classés 19 à 63"
              "- N_Cla64_255_UserDefinable : Nombre de points classés 63 à 255"
              "- ZRange : Zmax - Zmin"
              "- NumberOfLines : Nombre de lignes vols couvrant la placette"
              "- N_bestLine : Nombre de points de la ligne de vols ayant le plus de points"
              "- N_secondLine : Nombre de points de la seconde ligne de vols ayant le plus de points"
              "- N_worstLine : Nombre de points de la ligne de vols ayant le moins de points");
}

ONF_MetricNbyLASClass::Config ONF_MetricNbyLASClass::metricConfiguration() const
{
    return m_configAndResults;
}

void ONF_MetricNbyLASClass::setMetricConfiguration(const ONF_MetricNbyLASClass::Config &conf)
{
    m_configAndResults = conf;
}

CT_AbstractConfigurableElement *ONF_MetricNbyLASClass::copy() const
{
    return new ONF_MetricNbyLASClass(*this);
}

void ONF_MetricNbyLASClass::computeMetric()
{    
    m_configAndResults.n.value = 0;
    m_configAndResults.n_first.value = 0;
    m_configAndResults.n_last.value = 0;
    m_configAndResults.n_intermediate.value = 0;
    m_configAndResults.n_only.value = 0;
    m_configAndResults.n_error.value = 0;

    m_configAndResults.n_cla00.value = 0;
    m_configAndResults.n_cla01.value = 0;
    m_configAndResults.n_cla02.value = 0;
    m_configAndResults.n_cla03.value = 0;
    m_configAndResults.n_cla04.value = 0;
    m_configAndResults.n_cla05.value = 0;
    m_configAndResults.n_cla06.value = 0;
    m_configAndResults.n_cla07.value = 0;
    m_configAndResults.n_cla08.value = 0;
    m_configAndResults.n_cla09.value = 0;
    m_configAndResults.n_cla10.value = 0;
    m_configAndResults.n_cla11.value = 0;
    m_configAndResults.n_cla12.value = 0;
    m_configAndResults.n_cla13.value = 0;
    m_configAndResults.n_cla14.value = 0;
    m_configAndResults.n_cla15.value = 0;
    m_configAndResults.n_cla16.value = 0;
    m_configAndResults.n_cla17.value = 0;
    m_configAndResults.n_cla18.value = 0;
    m_configAndResults.n_cla19_63.value = 0;
    m_configAndResults.n_cla64_255.value = 0;

    m_configAndResults.max_m_min.value = NAN;
    m_configAndResults.numberOfLines.value = 0;
    m_configAndResults.nBestLine.value = 0;
    m_configAndResults.nSecondLine.value = 0;
    m_configAndResults.nWorstLine.value = 0;

    double max = -std::numeric_limits<double>::max();
    double min = std::numeric_limits<double>::max();

    QMap<quint16, size_t> linesCounts;

    CT_PointIterator itP(pointCloud());
    while(itP.hasNext())
    {
        const CT_Point& point = itP.next().currentPoint();

        if ((plotArea() == NULL) || plotArea()->contains(point(0), point(1)))
        {
            ++(m_configAndResults.n.value);

            size_t index = itP.currentGlobalIndex();
            if (lasPointCloudIndex()->contains(index))
            {
                size_t lasIndex = lasPointCloudIndex()->indexOf(index);
                CT_LASData lasData;
                lasAttributes()->getLASDataAt(lasIndex, lasData);

                if (lasData._Return_Number == 1 && lasData._Number_of_Returns >= 1) {++(m_configAndResults.n_first.value);}
                if (lasData._Return_Number == 1 && lasData._Number_of_Returns == 1) {++(m_configAndResults.n_only.value);}
                if (lasData._Return_Number != 1 && lasData._Return_Number == lasData._Number_of_Returns && lasData._Number_of_Returns > 1) {++(m_configAndResults.n_last.value);}
                if (lasData._Return_Number > 1 && lasData._Return_Number != lasData._Number_of_Returns && lasData._Number_of_Returns > 1) {++(m_configAndResults.n_intermediate.value);}
                if (lasData._Return_Number < 1 || lasData._Return_Number < 1 || lasData._Return_Number > lasData._Return_Number) {++(m_configAndResults.n_error.value);}

                if (lasData._Classification == 0) {++(m_configAndResults.n_cla00.value);}
                else if (lasData._Classification == 1) {++(m_configAndResults.n_cla01.value);}
                else if (lasData._Classification == 2) {++(m_configAndResults.n_cla02.value);}
                else if (lasData._Classification == 3) {++(m_configAndResults.n_cla03.value);}
                else if (lasData._Classification == 4) {++(m_configAndResults.n_cla04.value);}
                else if (lasData._Classification == 5) {++(m_configAndResults.n_cla05.value);}
                else if (lasData._Classification == 6) {++(m_configAndResults.n_cla06.value);}
                else if (lasData._Classification == 7) {++(m_configAndResults.n_cla07.value);}
                else if (lasData._Classification == 8) {++(m_configAndResults.n_cla08.value);}
                else if (lasData._Classification == 9) {++(m_configAndResults.n_cla09.value);}
                else if (lasData._Classification == 10) {++(m_configAndResults.n_cla10.value);}
                else if (lasData._Classification == 11) {++(m_configAndResults.n_cla11.value);}
                else if (lasData._Classification == 12) {++(m_configAndResults.n_cla12.value);}
                else if (lasData._Classification == 13) {++(m_configAndResults.n_cla13.value);}
                else if (lasData._Classification == 14) {++(m_configAndResults.n_cla14.value);}
                else if (lasData._Classification == 15) {++(m_configAndResults.n_cla15.value);}
                else if (lasData._Classification == 16) {++(m_configAndResults.n_cla16.value);}
                else if (lasData._Classification == 17) {++(m_configAndResults.n_cla17.value);}
                else if (lasData._Classification == 18) {++(m_configAndResults.n_cla18.value);}
                else if (lasData._Classification >= 19 && lasData._Classification <= 63) {++(m_configAndResults.n_cla19_63.value);}
                else {++(m_configAndResults.n_cla64_255.value);}

                if (point(2) > max) {max = point(2);}
                if (point(2) < min) {min = point(2);}

                linesCounts[lasData._Point_Source_ID] = linesCounts.value(lasData._Point_Source_ID, 0) + 1;
            }
        }
    }

    QList<size_t> counts = linesCounts.values();
    qSort(counts.begin(), counts.end());

    m_configAndResults.numberOfLines.value = linesCounts.size();

    if (counts.size() > 1)
    {
        m_configAndResults.nBestLine.value = counts.last();
    } else {
        m_configAndResults.nBestLine.value = 0;
    }

    if (counts.size() > 1)
    {
        m_configAndResults.nSecondLine.value = counts.at(counts.size() - 2);
        m_configAndResults.nWorstLine.value  = counts.first();
    } else {
        m_configAndResults.nSecondLine.value = 0;
        m_configAndResults.nWorstLine.value  = 0;
    }

    if (m_configAndResults.n.value > 0)
    {
        m_configAndResults.max_m_min.value = max - min;
    }

    setAttributeValueVaB(m_configAndResults.n);

    setAttributeValueVaB(m_configAndResults.n_first);
    setAttributeValueVaB(m_configAndResults.n_last);
    setAttributeValueVaB(m_configAndResults.n_intermediate);
    setAttributeValueVaB(m_configAndResults.n_only);
    setAttributeValueVaB(m_configAndResults.n_error);

    setAttributeValueVaB(m_configAndResults.n_cla00);
    setAttributeValueVaB(m_configAndResults.n_cla01);
    setAttributeValueVaB(m_configAndResults.n_cla02);
    setAttributeValueVaB(m_configAndResults.n_cla03);
    setAttributeValueVaB(m_configAndResults.n_cla04);
    setAttributeValueVaB(m_configAndResults.n_cla05);
    setAttributeValueVaB(m_configAndResults.n_cla06);
    setAttributeValueVaB(m_configAndResults.n_cla07);
    setAttributeValueVaB(m_configAndResults.n_cla08);
    setAttributeValueVaB(m_configAndResults.n_cla09);
    setAttributeValueVaB(m_configAndResults.n_cla10);
    setAttributeValueVaB(m_configAndResults.n_cla11);
    setAttributeValueVaB(m_configAndResults.n_cla12);
    setAttributeValueVaB(m_configAndResults.n_cla13);
    setAttributeValueVaB(m_configAndResults.n_cla14);
    setAttributeValueVaB(m_configAndResults.n_cla15);
    setAttributeValueVaB(m_configAndResults.n_cla16);
    setAttributeValueVaB(m_configAndResults.n_cla17);
    setAttributeValueVaB(m_configAndResults.n_cla18);
    setAttributeValueVaB(m_configAndResults.n_cla19_63);
    setAttributeValueVaB(m_configAndResults.n_cla64_255);

    setAttributeValueVaB(m_configAndResults.max_m_min);
    setAttributeValueVaB(m_configAndResults.numberOfLines);
    setAttributeValueVaB(m_configAndResults.nBestLine);
    setAttributeValueVaB(m_configAndResults.nSecondLine);
    setAttributeValueVaB(m_configAndResults.nWorstLine);
}

void ONF_MetricNbyLASClass::declareAttributes()
{
    registerAttributeVaB(m_configAndResults.n, CT_AbstractCategory::DATA_NUMBER, tr("N"));
    registerAttributeVaB(m_configAndResults.n_first, CT_AbstractCategory::DATA_NUMBER, tr("N_first"));
    registerAttributeVaB(m_configAndResults.n_last, CT_AbstractCategory::DATA_NUMBER, tr("N_last"));
    registerAttributeVaB(m_configAndResults.n_intermediate, CT_AbstractCategory::DATA_NUMBER, tr("N_int"));
    registerAttributeVaB(m_configAndResults.n_only, CT_AbstractCategory::DATA_NUMBER, tr("N_only"));
    registerAttributeVaB(m_configAndResults.n_error, CT_AbstractCategory::DATA_NUMBER, tr("N_error"));

    registerAttributeVaB(m_configAndResults.n_cla00, CT_AbstractCategory::DATA_NUMBER, tr("N_Cla00_NeverClassified"));
    registerAttributeVaB(m_configAndResults.n_cla01, CT_AbstractCategory::DATA_NUMBER, tr("N_Cla01_Unclassified"));
    registerAttributeVaB(m_configAndResults.n_cla02, CT_AbstractCategory::DATA_NUMBER, tr("N_Cla02_Ground"));
    registerAttributeVaB(m_configAndResults.n_cla03, CT_AbstractCategory::DATA_NUMBER, tr("N_Cla03_LowVegetation"));
    registerAttributeVaB(m_configAndResults.n_cla04, CT_AbstractCategory::DATA_NUMBER, tr("N_Cla04_MediumVegetation"));
    registerAttributeVaB(m_configAndResults.n_cla05, CT_AbstractCategory::DATA_NUMBER, tr("N_Cla05_HighVegetation"));
    registerAttributeVaB(m_configAndResults.n_cla06, CT_AbstractCategory::DATA_NUMBER, tr("N_Cla06_Building"));
    registerAttributeVaB(m_configAndResults.n_cla07, CT_AbstractCategory::DATA_NUMBER, tr("N_Cla07_LowPointNoise"));
    registerAttributeVaB(m_configAndResults.n_cla08, CT_AbstractCategory::DATA_NUMBER, tr("N_Cla08_ModelKeyPoint"));
    registerAttributeVaB(m_configAndResults.n_cla09, CT_AbstractCategory::DATA_NUMBER, tr("N_Cla09_Water"));
    registerAttributeVaB(m_configAndResults.n_cla10, CT_AbstractCategory::DATA_NUMBER, tr("N_Cla10_Rail"));
    registerAttributeVaB(m_configAndResults.n_cla11, CT_AbstractCategory::DATA_NUMBER, tr("N_Cla11_RoadSurface"));
    registerAttributeVaB(m_configAndResults.n_cla12, CT_AbstractCategory::DATA_NUMBER, tr("N_Cla12_OverlapPoint"));
    registerAttributeVaB(m_configAndResults.n_cla13, CT_AbstractCategory::DATA_NUMBER, tr("N_Cla13_WireGuard"));
    registerAttributeVaB(m_configAndResults.n_cla14, CT_AbstractCategory::DATA_NUMBER, tr("N_Cla14_WireConductor"));
    registerAttributeVaB(m_configAndResults.n_cla15, CT_AbstractCategory::DATA_NUMBER, tr("N_Cla15_TransmissionTower"));
    registerAttributeVaB(m_configAndResults.n_cla16, CT_AbstractCategory::DATA_NUMBER, tr("N_Cla16_WireStructureConnector"));
    registerAttributeVaB(m_configAndResults.n_cla17, CT_AbstractCategory::DATA_NUMBER, tr("N_Cla17_BridgeDeck"));
    registerAttributeVaB(m_configAndResults.n_cla18, CT_AbstractCategory::DATA_NUMBER, tr("N_Cla18_HighNoise"));
    registerAttributeVaB(m_configAndResults.n_cla19_63, CT_AbstractCategory::DATA_NUMBER, tr("N_Cla19_63_Reserved"));
    registerAttributeVaB(m_configAndResults.n_cla64_255, CT_AbstractCategory::DATA_NUMBER, tr("N_Cla64_255_UserDefinable"));

    registerAttributeVaB(m_configAndResults.max_m_min, CT_AbstractCategory::DATA_NUMBER, tr("ZRange"));

    registerAttributeVaB(m_configAndResults.numberOfLines, CT_AbstractCategory::DATA_NUMBER, tr("NumberOfLines"));
    registerAttributeVaB(m_configAndResults.nBestLine, CT_AbstractCategory::DATA_NUMBER, tr("N_bestLine"));
    registerAttributeVaB(m_configAndResults.nSecondLine, CT_AbstractCategory::DATA_NUMBER, tr("N_secondLine"));
    registerAttributeVaB(m_configAndResults.nWorstLine, CT_AbstractCategory::DATA_NUMBER, tr("N_worstLine"));
}

