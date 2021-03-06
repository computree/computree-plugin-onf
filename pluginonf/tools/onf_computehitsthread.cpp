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


#include "onf_computehitsthread.h"

#include "ct_pointcloudindex/abstract/ct_abstractpointcloudindex.h"
#include "ct_iterator/ct_pointiterator.h"

ONF_ComputeHitsThread::ONF_ComputeHitsThread(CT_Grid3D<int> *grilleHits,
                                           const CT_Scene *scene) : CT_MonitoredQThread()
{
    _grilleHits = grilleHits;
    _scene = scene;
}

void ONF_ComputeHitsThread::run()
{
    CT_PointIterator itP(_scene->getPointCloudIndex());
    size_t n_points = itP.size();

    size_t progressStep = n_points / 20;
    size_t prog = 0;
    size_t i = 0;

    while(itP.hasNext())
    {
        const CT_Point &point = itP.next().currentPoint();
        size_t indice;

        if (_grilleHits->indexAtXYZ(point(0), point(1), point(2), indice))
        {
            // Hits Computing
            _grilleHits->addValueAtIndex(indice, 1);
        } else {
            qDebug() << "Le point "<< i << " de la scene n'est pas dans la grille";
        }

        ++i;

        if (i % progressStep == 0)
        {
            prog = (((size_t)100)*i)/n_points;
            _progress = prog;
            emit progressChanged();
        }
    }

    _grilleHits->computeMinMax();

    _progress = 100;
    emit progressChanged();
}

