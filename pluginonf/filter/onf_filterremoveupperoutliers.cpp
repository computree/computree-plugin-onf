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

#include "filter/onf_filterremoveupperoutliers.h"
#include "ct_pointcloudindex/ct_pointcloudindexvector.h"
#include "ct_view/ct_genericconfigurablewidget.h"

#define checkAndSetValue(ATT, NAME, TYPE) if((value = settings->firstValueByTagName(NAME)) == NULL) { return false; } else { ATT = value->value().value<TYPE>(); }

ONF_FilterRemoveUpperOutliers::ONF_FilterRemoveUpperOutliers() : CT_AbstractFilter_XYZ()
{
    _resolution = 1.0;
    _threshold = 2;
    _dc = 1;
}

ONF_FilterRemoveUpperOutliers::ONF_FilterRemoveUpperOutliers(const ONF_FilterRemoveUpperOutliers &other) : CT_AbstractFilter_XYZ(other)
{
    _resolution = other._resolution;
    _threshold = other._threshold;
    _dc = other._dc;
}

QString ONF_FilterRemoveUpperOutliers::getDetailledDisplayableName()
{
    return QString("Filtered");
}

CT_AbstractConfigurableWidget *ONF_FilterRemoveUpperOutliers::createConfigurationWidget()
{
    CT_GenericConfigurableWidget* configDialog = new CT_GenericConfigurableWidget();
    configDialog->addDouble(tr("Résolution de la grille de filtrage"), "m", 0.01, 1000, 2, _resolution);
    configDialog->addInt(tr("Nombre de points maximum d'une cellule éliminée"), "", 1, 1000, _threshold);
    configDialog->addInt(tr("Nombre de cases verticales autorisées entre la case filtrée et le voisinnage inférieur"), "", 0, 1000, _dc);

    return configDialog;
}

SettingsNodeGroup *ONF_FilterRemoveUpperOutliers::getAllSettings() const
{
    SettingsNodeGroup *root = new SettingsNodeGroup("ONF_FilterRemoveUpperOutliers");

    root->addValue(new SettingsNodeValue("resolution", _resolution));
    root->addValue(new SettingsNodeValue("threshold", _threshold));
    root->addValue(new SettingsNodeValue("dc", _dc));

    return root;
}

bool ONF_FilterRemoveUpperOutliers::setAllSettings(const SettingsNodeGroup *settings)
{
    if((settings == NULL) || (settings->name() != "ONF_FilterRemoveUpperOutliers"))
        return false;

    SettingsNodeValue *value = NULL;

    checkAndSetValue(_resolution, "resolution", double);
    checkAndSetValue(_threshold, "threshold", int);
    checkAndSetValue(_dc, "dc", int);

    return true;
}

bool ONF_FilterRemoveUpperOutliers::filterPointCloudIndex()
{
    if(inputPointCloudIndex() == NULL)
        return false;

    if(!updateMinMax())
        return false;

    _dimx = ceil((maxBBox()[0] - minBBox()[0])/_resolution);
    _dimy = ceil((maxBBox()[1] - minBBox()[1])/_resolution);
    _dimz = ceil((maxBBox()[2] - minBBox()[2])/_resolution);

    QMap<size_t, QList<size_t>* > indexMap;

    CT_PointCloudIndexVector *outCloud = outputPointCloudIndex();

    CT_PointIterator itP(inputPointCloudIndex());

    while(itP.hasNext())
    {
        const CT_Point &point = itP.next().currentPoint();
        size_t pointIndex = itP.currentGlobalIndex();

        outCloud->addIndex(pointIndex);

        size_t grdIndex = gridIndex(point(0), point(1), point(2));

        if (!indexMap.contains(grdIndex)) {indexMap.insert(grdIndex, new QList<size_t>());}

        QList<size_t> *points = indexMap.value(grdIndex);

        points->append(pointIndex);
    }

    for (size_t xx = 0 ; xx < _dimx ; xx++)
    {
        for (size_t yy = 0 ; yy < _dimy ; yy++)
        {
            bool goDown = true;
            for (size_t zz = _dimz - 1 ; goDown && (zz > _dc) ; zz--)
            {
                size_t grdIndex = gridIndex(xx, yy, zz);

                if (indexMap.contains(grdIndex))
                {
                    QList<size_t> *points = indexMap.value(grdIndex);

                    if (points->size() <= _threshold)
                    {
                        size_t zzm = zz - _dc - 1;
                        bool toremove = true;

                        if (toremove &&                                               indexMap.contains(gridIndex(xx,     yy,     zzm))) {toremove = false;}
                        if (toremove && (xx >= _dc)         && (yy >= _dc)         && indexMap.contains(gridIndex(xx-_dc, yy-_dc, zzm))) {toremove = false;}
                        if (toremove && (xx >= _dc)                                && indexMap.contains(gridIndex(xx-_dc, yy,     zzm))) {toremove = false;}
                        if (toremove && (xx >= _dc)         && (yy < _dimy - _dc)  && indexMap.contains(gridIndex(xx-_dc, yy+_dc, zzm))) {toremove = false;}
                        if (toremove &&                        (yy >= _dc)         && indexMap.contains(gridIndex(xx,     yy-_dc, zzm))) {toremove = false;}
                        if (toremove &&                        (yy < _dimy - _dc)  && indexMap.contains(gridIndex(xx,     yy+_dc, zzm))) {toremove = false;}
                        if (toremove && (xx < _dimx - _dc)  && (yy >= _dc)         && indexMap.contains(gridIndex(xx+_dc, yy-_dc, zzm))) {toremove = false;}
                        if (toremove && (xx < _dimx - _dc)  && (yy >= _dc)         && indexMap.contains(gridIndex(xx+_dc, yy,     zzm))) {toremove = false;}
                        if (toremove && (xx < _dimx - _dc)  && (yy < _dimy - _dc)  && indexMap.contains(gridIndex(xx+_dc, yy+_dc, zzm))) {toremove = false;}

                        if (toremove)
                        {
                            QListIterator<size_t> it(*points);

                            while (it.hasNext())
                                outCloud->removeIndex(it.next());

                        } else {
                            goDown = false;
                        }
                    } else {
                        goDown = false;
                    }
                }
            }
        }
    }
    qDeleteAll(indexMap.values());

    return true;
}


QString ONF_FilterRemoveUpperOutliers::getShortDescription() const
{
    return tr("Elimine les points au dessus de la canopée");
}

CT_AbstractConfigurableElement *ONF_FilterRemoveUpperOutliers::copy() const
{
    return new ONF_FilterRemoveUpperOutliers(*this);
}

size_t ONF_FilterRemoveUpperOutliers::gridIndex(const double &x, const double &y, const double &z) const
{
    size_t colx = (size_t) floor((x - minBBox()[0]) / _resolution);
    size_t liny = (size_t) floor((y - minBBox()[1]) / _resolution);
    size_t levz = (size_t) floor((z - minBBox()[2]) / _resolution);

    return gridIndex(colx, liny, levz);
}

size_t ONF_FilterRemoveUpperOutliers::gridIndex(const size_t &colx, const size_t &liny, const size_t &levz) const
{
    return levz*_dimx*_dimy + liny*_dimx + colx;
}


