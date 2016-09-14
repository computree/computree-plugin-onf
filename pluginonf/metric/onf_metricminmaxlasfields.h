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

#ifndef ONF_METRICMINMAXLASFIELDS_H
#define ONF_METRICMINMAXLASFIELDS_H

#include "ctliblas/metrics/abstract/ct_abstractmetric_las.h"
#include "ctlibmetrics/tools/ct_valueandbool.h"

class ONF_MetricMinMaxLASFields : public CT_AbstractMetric_LAS
{
    Q_OBJECT
public:

    struct Config {
        VaB<int>      _Intensity_Min;
        VaB<int>      _Intensity_Max;
        VaB<int>      _Return_Number_Min;
        VaB<int>      _Return_Number_Max;
        VaB<int>      _Number_of_Returns_Min;
        VaB<int>      _Number_of_Returns_Max;
        VaB<int>      _Classification_Flags_Min;
        VaB<int>      _Classification_Flags_Max;
        VaB<int>      _Scanner_Channel_Min;
        VaB<int>      _Scanner_Channel_Max;
        VaB<int>      _Scan_Direction_Flag_Min;
        VaB<int>      _Scan_Direction_Flag_Max;
        VaB<int>      _Edge_of_Flight_Line_Min;
        VaB<int>      _Edge_of_Flight_Line_Max;
        VaB<int>       _Classification_Min;
        VaB<int>       _Classification_Max;
        VaB<int>       _Scan_Angle_Rank_Min;
        VaB<int>       _Scan_Angle_Rank_Max;
        VaB<int>       _User_Data_Min;
        VaB<int>       _User_Data_Max;
        VaB<int>      _Point_Source_ID_Min;
        VaB<int>      _Point_Source_ID_Max;
        VaB<double>       _GPS_Time_Min;
        VaB<double>       _GPS_Time_Max;
        VaB<int>      _Red_Min;
        VaB<int>      _Red_Max;
        VaB<int>      _Green_Min;
        VaB<int>      _Green_Max;
        VaB<int>      _Blue_Min;
        VaB<int>      _Blue_Max;
    };

    ONF_MetricMinMaxLASFields();
    ONF_MetricMinMaxLASFields(const ONF_MetricMinMaxLASFields &other);

    QString getShortDescription() const;
    QString getDetailledDescription() const;

    /**
     * @brief Returns the metric configuration
     */
    ONF_MetricMinMaxLASFields::Config metricConfiguration() const;

    /**
     * @brief Change the configuration of this metric
     */
    void setMetricConfiguration(const ONF_MetricMinMaxLASFields::Config &conf);

    CT_AbstractConfigurableElement* copy() const;

protected:
    void computeMetric();
    void declareAttributes();

private:
    Config  m_configAndResults;
};


#endif // ONF_METRICMINMAXLASFIELDS_H
