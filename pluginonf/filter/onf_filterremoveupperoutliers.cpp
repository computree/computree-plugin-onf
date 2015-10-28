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

ONF_FilterRemoveUpperOutliers::ONF_FilterRemoveUpperOutliers() : CT_AbstractFilter_XYZ()
{
    _resolution = 1.0;
    _threshold = 2;
    _dc = 1;
}

ONF_FilterRemoveUpperOutliers::ONF_FilterRemoveUpperOutliers(const ONF_FilterRemoveUpperOutliers *other) : CT_AbstractFilter_XYZ(other)
{
    _resolution = 1.0;
    _threshold = 2.0;
    _dc = 1;
}

QString ONF_FilterRemoveUpperOutliers::getName()
{
    return QString("ONF_RemoveUpperOutliers");
}

QString ONF_FilterRemoveUpperOutliers::getCompleteName()
{
    return QString("Filtered");
}

void ONF_FilterRemoveUpperOutliers::createConfigurationDialog()
{
    CT_StepConfigurableDialog* configDialog = addConfigurationDialog();
    configDialog->addDouble(tr("Résolution de la grille de filtrage"), "m", 0.01, 1000, 2, _resolution);
    configDialog->addInt(tr("Nombre de points maximum d'une cellule éliminée"), "", 1, 1000, _threshold);
    configDialog->addInt(tr("Nombre de cases verticales autorisées entre la case filtrée et le voisinnage inférieur"), "", 0, 1000, _dc);
}

void ONF_FilterRemoveUpperOutliers::updateParamtersAfterConfiguration()
{
}

QString ONF_FilterRemoveUpperOutliers::getParametersAsString() const
{
    QString result = "";
    return result;
}

bool ONF_FilterRemoveUpperOutliers::setParametersFromString(QString parameters)
{
    return true;
}

CT_PointCloudIndexVector *ONF_FilterRemoveUpperOutliers::filterPointCloudIndex() const
{
    _minx = _inItem->minX();
    _miny = _inItem->minY();
    _minz = _inItem->minZ();

    _dimx = ceil((_inItem->maxX() - _minx)/_resolution);
    _dimy = ceil((_inItem->maxY() - _miny)/_resolution);
    _dimz = ceil((_inItem->maxZ() - _minz)/_resolution);

    QMap<size_t, QList<size_t>* > indexMap;

    CT_PointIterator itP(_inCloud);
    while(itP.hasNext())
    {
        const CT_Point &point = itP.next().currentPoint();
        size_t pointIndex = itP.currentGlobalIndex();

        _outCloud->addIndex(pointIndex);

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
                            {
                                _outCloud->removeIndex(it.next());
                            }
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

    return _outCloud;
}


QString ONF_FilterRemoveUpperOutliers::getShortDescription() const
{
    return tr("Elimine les points au dessus de la canopée");
}

QString ONF_FilterRemoveUpperOutliers::getDetailledDescription() const
{
    return tr("");
}

CT_AbstractConfigurableElement *ONF_FilterRemoveUpperOutliers::copy() const
{
    ONF_FilterRemoveUpperOutliers* filter = new ONF_FilterRemoveUpperOutliers(this);
    filter->_resolution = _resolution;
    filter->_threshold  = _threshold;
    filter->_dc         = _dc;
    return filter;
}




size_t ONF_FilterRemoveUpperOutliers::gridIndex(const double &x, const double &y, const double &z) const
{
    size_t colx = (size_t) floor((x - _minx) / _resolution);
    size_t liny = (size_t) floor((y - _miny) / _resolution);
    size_t levz = (size_t) floor((z - _minz) / _resolution);

    return gridIndex(colx, liny, levz);
}

size_t ONF_FilterRemoveUpperOutliers::gridIndex(const size_t &colx, const size_t &liny, const size_t &levz) const
{
    return levz*_dimx*_dimy + liny*_dimx + colx;
}


