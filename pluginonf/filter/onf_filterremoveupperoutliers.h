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

#ifndef ONF_FILTERREMOVEUPPEROUTLIERS_H
#define ONF_FILTERREMOVEUPPEROUTLIERS_H


#include "ctlibfilters/filters/abstract/ct_abstractfilter_xyz.h"
#include "ct_iterator/ct_pointiterator.h"


class ONF_FilterRemoveUpperOutliers : public CT_AbstractFilter_XYZ
{
    Q_OBJECT
public:

    ONF_FilterRemoveUpperOutliers();
    ONF_FilterRemoveUpperOutliers(const ONF_FilterRemoveUpperOutliers &other);

    QString getDetailledDisplayableName();
    QString getShortDescription() const;

    bool filterPointCloudIndex();

    /**
     * @brief TODO : Cannot be used ?!
     */
    bool validatePoint(const CT_PointIterator& pointIt) { Q_UNUSED(pointIt); return false; }

    CT_AbstractConfigurableWidget* createConfigurationWidget();

    SettingsNodeGroup* getAllSettings() const;
    bool setAllSettings(const SettingsNodeGroup *settings);

    CT_AbstractConfigurableElement* copy() const;

private:
    double  _resolution;
    int     _threshold;
    int     _dc;

    size_t  _dimx;
    size_t  _dimy;
    size_t  _dimz;

    size_t gridIndex(const double &x, const double &y, const double &z) const;
    size_t gridIndex(const size_t &colx, const size_t &liny, const size_t &levz) const;
};


#endif // ONF_FILTERREMOVEUPPEROUTLIERS_H
