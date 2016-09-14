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

#include "metric/onf_metricminmaxlasfields.h"
#include "ct_pointcloudindex/ct_pointcloudindexvector.h"
#include "ct_iterator/ct_pointiterator.h"

#include "ctliblas/tools/las/ct_lasdata.h"

ONF_MetricMinMaxLASFields::ONF_MetricMinMaxLASFields() : CT_AbstractMetric_LAS()
{
    declareAttributes();
}

ONF_MetricMinMaxLASFields::ONF_MetricMinMaxLASFields(const ONF_MetricMinMaxLASFields &other) : CT_AbstractMetric_LAS(other)
{
    declareAttributes();
    m_configAndResults = other.m_configAndResults;
}

QString ONF_MetricMinMaxLASFields::getShortDescription() const
{
    return tr("Min et Max pour chaque champ LAS");
}

QString ONF_MetricMinMaxLASFields::getDetailledDescription() const
{
    return tr("Les valeurs suivantes sont calculées :<br>"
              "- Intensity_Min<br>"
              "- Intensity_Max<br>"
              "- Return_Number_Min<br>"
              "- Return_Number_Max<br>"
              "- Number_of_Returns_Min<br>"
              "- Number_of_Returns_Max<br>"
              "- Classification_Flags_Min<br>"
              "- Classification_Flags_Max<br>"
              "- Scanner_Channel_Min<br>"
              "- Scanner_Channel_Max<br>"
              "- Scan_Direction_Flag_Min<br>"
              "- Scan_Direction_Flag_Max<br>"
              "- Edge_of_Flight_Line_Min<br>"
              "- Edge_of_Flight_Line_Max<br>"
              "- Classification_Min<br>"
              "- Classification_Max<br>"
              "- Scan_Angle_Rank_Min<br>"
              "- Scan_Angle_Rank_Max<br>"
              "- User_Data_Min<br>"
              "- User_Data_Max<br>"
              "- Point_Source_ID_Min<br>"
              "- Point_Source_ID_Max<br>"
              "- GPS_Time_Min<br>"
              "- GPS_Time_Max<br>"
              "- Red_Min<br>"
              "- Red_Max<br>"
              "- Green_Min<br>"
              "- Green_Max<br>"
              "- Blue_Min<br>"
              "- Blue_Max<br>");
}

ONF_MetricMinMaxLASFields::Config ONF_MetricMinMaxLASFields::metricConfiguration() const
{
    return m_configAndResults;
}

void ONF_MetricMinMaxLASFields::setMetricConfiguration(const ONF_MetricMinMaxLASFields::Config &conf)
{
    m_configAndResults = conf;
}

CT_AbstractConfigurableElement *ONF_MetricMinMaxLASFields::copy() const
{
    return new ONF_MetricMinMaxLASFields(*this);
}

void ONF_MetricMinMaxLASFields::computeMetric()
{    
    m_configAndResults._Intensity_Min.value = std::numeric_limits<int>::max();
    m_configAndResults._Intensity_Max.value = 0;
    m_configAndResults._Return_Number_Min.value = std::numeric_limits<int>::max();
    m_configAndResults._Return_Number_Max.value = 0;
    m_configAndResults._Number_of_Returns_Min.value = std::numeric_limits<int>::max();
    m_configAndResults._Number_of_Returns_Max.value = 0;
    m_configAndResults._Classification_Flags_Min.value = std::numeric_limits<int>::max();
    m_configAndResults._Classification_Flags_Max.value = 0;
    m_configAndResults._Scanner_Channel_Min.value = std::numeric_limits<int>::max();
    m_configAndResults._Scanner_Channel_Max.value = 0;
    m_configAndResults._Scan_Direction_Flag_Min.value = std::numeric_limits<int>::max();
    m_configAndResults._Scan_Direction_Flag_Max.value = 0;
    m_configAndResults._Edge_of_Flight_Line_Min.value = std::numeric_limits<int>::max();
    m_configAndResults._Edge_of_Flight_Line_Max.value = 0;
    m_configAndResults._Classification_Min.value = std::numeric_limits<int>::max();
    m_configAndResults._Classification_Max.value = 0;
    m_configAndResults._Scan_Angle_Rank_Min.value = std::numeric_limits<int>::max();
    m_configAndResults._Scan_Angle_Rank_Max.value = -std::numeric_limits<int>::max();
    m_configAndResults._User_Data_Min.value = std::numeric_limits<int>::max();
    m_configAndResults._User_Data_Max.value = 0;
    m_configAndResults._Point_Source_ID_Min.value = std::numeric_limits<int>::max();
    m_configAndResults._Point_Source_ID_Max.value = 0;
    m_configAndResults._GPS_Time_Min.value = std::numeric_limits<double>::max();
    m_configAndResults._GPS_Time_Max.value = -std::numeric_limits<double>::max();
    m_configAndResults._Red_Min.value = std::numeric_limits<int>::max();
    m_configAndResults._Red_Max.value = 0;
    m_configAndResults._Green_Min.value = std::numeric_limits<int>::max();
    m_configAndResults._Green_Max.value = 0;
    m_configAndResults._Blue_Min.value = std::numeric_limits<int>::max();
    m_configAndResults._Blue_Max.value = 0;


    CT_PointIterator itP(pointCloud());
    while(itP.hasNext())
    {
        const CT_Point& point = itP.next().currentPoint();

        if ((plotArea() == NULL) || plotArea()->contains(point(0), point(1)))
        {
            size_t index = itP.currentGlobalIndex();
            if (lasPointCloudIndex()->contains(index))
            {
                size_t lasIndex = lasPointCloudIndex()->indexOf(index);
                CT_LASData lasData;
                lasAttributes()->getLASDataAt(lasIndex, lasData);

                if (lasData._Intensity < m_configAndResults._Intensity_Min.value) {m_configAndResults._Intensity_Min.value = lasData._Intensity;}
                if (lasData._Intensity > m_configAndResults._Intensity_Max.value) {m_configAndResults._Intensity_Max.value = lasData._Intensity;}

                if (lasData._Return_Number < m_configAndResults._Return_Number_Min.value) {m_configAndResults._Return_Number_Min.value = lasData._Return_Number;}
                if (lasData._Return_Number > m_configAndResults._Return_Number_Max.value) {m_configAndResults._Return_Number_Max.value = lasData._Return_Number;}

                if (lasData._Number_of_Returns < m_configAndResults._Number_of_Returns_Min.value) {m_configAndResults._Number_of_Returns_Min.value = lasData._Number_of_Returns;}
                if (lasData._Number_of_Returns > m_configAndResults._Number_of_Returns_Max.value) {m_configAndResults._Number_of_Returns_Max.value = lasData._Number_of_Returns;}

                if (lasData._Classification_Flags < m_configAndResults._Classification_Flags_Min.value) {m_configAndResults._Classification_Flags_Min.value = lasData._Classification_Flags;}
                if (lasData._Classification_Flags > m_configAndResults._Classification_Flags_Max.value) {m_configAndResults._Classification_Flags_Max.value = lasData._Classification_Flags;}

                if (lasData._Scanner_Channel < m_configAndResults._Scanner_Channel_Min.value) {m_configAndResults._Scanner_Channel_Min.value = lasData._Scanner_Channel;}
                if (lasData._Scanner_Channel > m_configAndResults._Scanner_Channel_Max.value) {m_configAndResults._Scanner_Channel_Max.value = lasData._Scanner_Channel;}

                if (lasData._Scan_Direction_Flag < m_configAndResults._Scan_Direction_Flag_Min.value) {m_configAndResults._Scan_Direction_Flag_Min.value = lasData._Scan_Direction_Flag;}
                if (lasData._Scan_Direction_Flag > m_configAndResults._Scan_Direction_Flag_Max.value) {m_configAndResults._Scan_Direction_Flag_Max.value = lasData._Scan_Direction_Flag;}

                if (lasData._Edge_of_Flight_Line < m_configAndResults._Edge_of_Flight_Line_Min.value) {m_configAndResults._Edge_of_Flight_Line_Min.value = lasData._Edge_of_Flight_Line;}
                if (lasData._Edge_of_Flight_Line > m_configAndResults._Edge_of_Flight_Line_Max.value) {m_configAndResults._Edge_of_Flight_Line_Max.value = lasData._Edge_of_Flight_Line;}

                if (lasData._Classification < m_configAndResults._Classification_Min.value) {m_configAndResults._Classification_Min.value = lasData._Classification;}
                if (lasData._Classification > m_configAndResults._Classification_Max.value) {m_configAndResults._Classification_Max.value = lasData._Classification;}

                if (lasData._Scan_Angle_Rank < m_configAndResults._Scan_Angle_Rank_Min.value) {m_configAndResults._Scan_Angle_Rank_Min.value = lasData._Scan_Angle_Rank;}
                if (lasData._Scan_Angle_Rank > m_configAndResults._Scan_Angle_Rank_Max.value) {m_configAndResults._Scan_Angle_Rank_Max.value = lasData._Scan_Angle_Rank;}

                if (lasData._User_Data < m_configAndResults._User_Data_Min.value) {m_configAndResults._User_Data_Min.value = lasData._User_Data;}
                if (lasData._User_Data > m_configAndResults._User_Data_Max.value) {m_configAndResults._User_Data_Max.value = lasData._User_Data;}

                if (lasData._Point_Source_ID < m_configAndResults._Point_Source_ID_Min.value) {m_configAndResults._Point_Source_ID_Min.value = lasData._Point_Source_ID;}
                if (lasData._Point_Source_ID > m_configAndResults._Point_Source_ID_Max.value) {m_configAndResults._Point_Source_ID_Max.value = lasData._Point_Source_ID;}

                if (lasData._GPS_Time < m_configAndResults._GPS_Time_Min.value) {m_configAndResults._GPS_Time_Min.value = lasData._GPS_Time;}
                if (lasData._GPS_Time > m_configAndResults._GPS_Time_Max.value) {m_configAndResults._GPS_Time_Max.value = lasData._GPS_Time;}

                if (lasData._Red < m_configAndResults._Red_Min.value) {m_configAndResults._Red_Min.value = lasData._Red;}
                if (lasData._Red > m_configAndResults._Red_Max.value) {m_configAndResults._Red_Max.value = lasData._Red;}

                if (lasData._Green < m_configAndResults._Green_Min.value) {m_configAndResults._Green_Min.value = lasData._Green;}
                if (lasData._Green > m_configAndResults._Green_Max.value) {m_configAndResults._Green_Max.value = lasData._Green;}

                if (lasData._Blue < m_configAndResults._Blue_Min.value) {m_configAndResults._Blue_Min.value = lasData._Blue;}
                if (lasData._Blue > m_configAndResults._Blue_Max.value) {m_configAndResults._Blue_Max.value = lasData._Blue;}
            }
        }
    }

    setAttributeValueVaB(m_configAndResults._Intensity_Min);
    setAttributeValueVaB(m_configAndResults._Intensity_Max);
    setAttributeValueVaB(m_configAndResults._Return_Number_Min);
    setAttributeValueVaB(m_configAndResults._Return_Number_Max);
    setAttributeValueVaB(m_configAndResults._Number_of_Returns_Min);
    setAttributeValueVaB(m_configAndResults._Number_of_Returns_Max);
    setAttributeValueVaB(m_configAndResults._Classification_Flags_Min);
    setAttributeValueVaB(m_configAndResults._Classification_Flags_Max);
    setAttributeValueVaB(m_configAndResults._Scanner_Channel_Min);
    setAttributeValueVaB(m_configAndResults._Scanner_Channel_Max);
    setAttributeValueVaB(m_configAndResults._Scan_Direction_Flag_Min);
    setAttributeValueVaB(m_configAndResults._Scan_Direction_Flag_Max);
    setAttributeValueVaB(m_configAndResults._Edge_of_Flight_Line_Min);
    setAttributeValueVaB(m_configAndResults._Edge_of_Flight_Line_Max);
    setAttributeValueVaB(m_configAndResults._Classification_Min);
    setAttributeValueVaB(m_configAndResults._Classification_Max);
    setAttributeValueVaB(m_configAndResults._Scan_Angle_Rank_Min);
    setAttributeValueVaB(m_configAndResults._Scan_Angle_Rank_Max);
    setAttributeValueVaB(m_configAndResults._User_Data_Min);
    setAttributeValueVaB(m_configAndResults._User_Data_Max);
    setAttributeValueVaB(m_configAndResults._Point_Source_ID_Min);
    setAttributeValueVaB(m_configAndResults._Point_Source_ID_Max);
    setAttributeValueVaB(m_configAndResults._GPS_Time_Min);
    setAttributeValueVaB(m_configAndResults._GPS_Time_Max);
    setAttributeValueVaB(m_configAndResults._Red_Min);
    setAttributeValueVaB(m_configAndResults._Red_Max);
    setAttributeValueVaB(m_configAndResults._Green_Min);
    setAttributeValueVaB(m_configAndResults._Green_Max);
    setAttributeValueVaB(m_configAndResults._Blue_Min);
    setAttributeValueVaB(m_configAndResults._Blue_Max);
}

void ONF_MetricMinMaxLASFields::declareAttributes()
{    
    registerAttributeVaB(m_configAndResults._Intensity_Min, CT_AbstractCategory::DATA_NUMBER, tr("Intensity_Min"));
    registerAttributeVaB(m_configAndResults._Intensity_Max, CT_AbstractCategory::DATA_NUMBER, tr("Intensity_Max"));
    registerAttributeVaB(m_configAndResults._Return_Number_Min, CT_AbstractCategory::DATA_NUMBER, tr("Return_Number_Min"));
    registerAttributeVaB(m_configAndResults._Return_Number_Max, CT_AbstractCategory::DATA_NUMBER, tr("Return_Number_Max"));
    registerAttributeVaB(m_configAndResults._Number_of_Returns_Min, CT_AbstractCategory::DATA_NUMBER, tr("Number_of_Returns_Min"));
    registerAttributeVaB(m_configAndResults._Number_of_Returns_Max, CT_AbstractCategory::DATA_NUMBER, tr("Number_of_Returns_Max"));
    registerAttributeVaB(m_configAndResults._Classification_Flags_Min, CT_AbstractCategory::DATA_NUMBER, tr("Classification_Flags_Min"));
    registerAttributeVaB(m_configAndResults._Classification_Flags_Max, CT_AbstractCategory::DATA_NUMBER, tr("Classification_Flags_Max"));
    registerAttributeVaB(m_configAndResults._Scanner_Channel_Min, CT_AbstractCategory::DATA_NUMBER, tr("Scanner_Channel_Min"));
    registerAttributeVaB(m_configAndResults._Scanner_Channel_Max, CT_AbstractCategory::DATA_NUMBER, tr("Scanner_Channel_Max"));
    registerAttributeVaB(m_configAndResults._Scan_Direction_Flag_Min, CT_AbstractCategory::DATA_NUMBER, tr("Scan_Direction_Flag_Min"));
    registerAttributeVaB(m_configAndResults._Scan_Direction_Flag_Max, CT_AbstractCategory::DATA_NUMBER, tr("Scan_Direction_Flag_Max"));
    registerAttributeVaB(m_configAndResults._Edge_of_Flight_Line_Min, CT_AbstractCategory::DATA_NUMBER, tr("Edge_of_Flight_Line_Min"));
    registerAttributeVaB(m_configAndResults._Edge_of_Flight_Line_Max, CT_AbstractCategory::DATA_NUMBER, tr("Edge_of_Flight_Line_Max"));
    registerAttributeVaB(m_configAndResults._Classification_Min, CT_AbstractCategory::DATA_NUMBER, tr("Classification_Min"));
    registerAttributeVaB(m_configAndResults._Classification_Max, CT_AbstractCategory::DATA_NUMBER, tr("Classification_Max"));
    registerAttributeVaB(m_configAndResults._Scan_Angle_Rank_Min, CT_AbstractCategory::DATA_NUMBER, tr("Scan_Angle_Rank_Min"));
    registerAttributeVaB(m_configAndResults._Scan_Angle_Rank_Max, CT_AbstractCategory::DATA_NUMBER, tr("Scan_Angle_Rank_Max"));
    registerAttributeVaB(m_configAndResults._User_Data_Min, CT_AbstractCategory::DATA_NUMBER, tr("User_Data_Min"));
    registerAttributeVaB(m_configAndResults._User_Data_Max, CT_AbstractCategory::DATA_NUMBER, tr("User_Data_Max"));
    registerAttributeVaB(m_configAndResults._Point_Source_ID_Min, CT_AbstractCategory::DATA_NUMBER, tr("Point_Source_ID_Min"));
    registerAttributeVaB(m_configAndResults._Point_Source_ID_Max, CT_AbstractCategory::DATA_NUMBER, tr("Point_Source_ID_Max"));
    registerAttributeVaB(m_configAndResults._GPS_Time_Min, CT_AbstractCategory::DATA_NUMBER, tr("GPS_Time_Min"));
    registerAttributeVaB(m_configAndResults._GPS_Time_Max, CT_AbstractCategory::DATA_NUMBER, tr("GPS_Time_Max"));
    registerAttributeVaB(m_configAndResults._Red_Min, CT_AbstractCategory::DATA_NUMBER, tr("Red_Min"));
    registerAttributeVaB(m_configAndResults._Red_Max, CT_AbstractCategory::DATA_NUMBER, tr("Red_Max"));
    registerAttributeVaB(m_configAndResults._Green_Min, CT_AbstractCategory::DATA_NUMBER, tr("Green_Min"));
    registerAttributeVaB(m_configAndResults._Green_Max, CT_AbstractCategory::DATA_NUMBER, tr("Green_Max"));
    registerAttributeVaB(m_configAndResults._Blue_Min, CT_AbstractCategory::DATA_NUMBER, tr("Blue_Min"));
    registerAttributeVaB(m_configAndResults._Blue_Max, CT_AbstractCategory::DATA_NUMBER, tr("Blue_Max"));
}

