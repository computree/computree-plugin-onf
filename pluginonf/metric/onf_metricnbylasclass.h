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

#ifndef ONF_METRICNBYLASCLASS_H
#define ONF_METRICNBYLASCLASS_H

#include "ctliblas/metrics/abstract/ct_abstractmetric_las.h"
#include "ctlibmetrics/tools/ct_valueandbool.h"

class ONF_MetricNbyLASClass : public CT_AbstractMetric_LAS
{
    Q_OBJECT
public:

    struct Config {
        VaB<size_t>      n;
        VaB<size_t>      n_first;
        VaB<size_t>      n_last;
        VaB<size_t>      n_intermediate;
        VaB<size_t>      n_only;
        VaB<size_t>      n_error;
        VaB<size_t>      n_cla00;
        VaB<size_t>      n_cla01;
        VaB<size_t>      n_cla02;
        VaB<size_t>      n_cla03;
        VaB<size_t>      n_cla04;
        VaB<size_t>      n_cla05;
        VaB<size_t>      n_cla06;
        VaB<size_t>      n_cla07;
        VaB<size_t>      n_cla08;
        VaB<size_t>      n_cla09;
        VaB<size_t>      n_cla10;
        VaB<size_t>      n_cla11;
        VaB<size_t>      n_cla12;
        VaB<size_t>      n_cla13;
        VaB<size_t>      n_cla14;
        VaB<size_t>      n_cla15;
        VaB<size_t>      n_cla16;
        VaB<size_t>      n_cla17;
        VaB<size_t>      n_cla18;
        VaB<size_t>      n_cla19_63;
        VaB<size_t>      n_cla64_255;
        VaB<double>      max_m_min;
        VaB<size_t>      numberOfLines;
        VaB<size_t>      nBestLine;
        VaB<size_t>      nSecondLine;
        VaB<size_t>      nWorstLine;
    };

    ONF_MetricNbyLASClass();
    ONF_MetricNbyLASClass(const ONF_MetricNbyLASClass &other);

    QString getShortDescription() const;
    QString getDetailledDescription() const;

    /**
     * @brief Returns the metric configuration
     */
    ONF_MetricNbyLASClass::Config metricConfiguration() const;

    /**
     * @brief Change the configuration of this metric
     */
    void setMetricConfiguration(const ONF_MetricNbyLASClass::Config &conf);

    CT_AbstractConfigurableElement* copy() const;

protected:
    void computeMetric();
    void declareAttributes();

private:
    Config  m_configAndResults;
};


#endif // ONF_METRICNBYLASCLASS_H
