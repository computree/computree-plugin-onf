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


#include "onf_stepreducepointsdensity.h"

#include "ct_global/ct_context.h"

#include "ct_itemdrawable/model/outModel/ct_outstdgroupmodel.h"
#include "ct_itemdrawable/model/inModel/ct_instdsingularitemmodel.h"
#include "ct_itemdrawable/model/outModel/ct_outstdsingularitemmodel.h"

#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/ct_outresultmodelgroupcopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"

#include "ct_iterator/ct_resultgroupiterator.h"

#include "ct_result/ct_resultgroup.h"
#include "ct_itemdrawable/ct_scene.h"
#include "ct_itemdrawable/ct_grid3d.h"

#include "ct_pointcloudindex/ct_pointcloudindexvector.h"
#include "ct_view/ct_stepconfigurabledialog.h"

#include "ct_model/tools/ct_modelsearchhelper.h"

#include "ct_iterator/ct_pointiterator.h"
#include "ct_accessor/ct_pointaccessor.h"

#include <math.h>
#include <iostream>
#include <QList>
#include <limits>

#define DEF_SearchInResult "r"
#define DEF_SearchInScene   "sc"
#define DEF_SearchInGroup "grp"


ONF_StepReducePointsDensity::ONF_StepReducePointsDensity(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _resolution = 0.005;
}

QString ONF_StepReducePointsDensity::getStepDescription() const
{
    return tr("Réduire la Densité de points");
}

QString ONF_StepReducePointsDensity::getStepDetailledDescription() const
{
    return tr("Créée une grille régulière de la <b>résolution</b> choisie. Ne garde que le point le plus proche du centre dans chaque case. ");
}

CT_VirtualAbstractStep* ONF_StepReducePointsDensity::createNewInstance(CT_StepInitializeData &dataInit)
{
    // cree une copie de cette etape
    return new ONF_StepReducePointsDensity(dataInit);
}

//////////////////// PROTECTED //////////////////

void ONF_StepReducePointsDensity::createInResultModelListProtected()
{
    CT_InResultModelGroupToCopy *resultModel = createNewInResultModelForCopy(DEF_SearchInResult, tr("Scène(s)"));
    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_SearchInGroup);
    resultModel->addItemModel(DEF_SearchInGroup, DEF_SearchInScene, CT_Scene::staticGetType(), tr("Scène"));
}

// Création et affiliation des modèles OUT
void ONF_StepReducePointsDensity::createOutResultModelListProtected()
{
    CT_OutResultModelGroupToCopyPossibilities *resultModel = createNewOutResultModelToCopy(DEF_SearchInResult);
    resultModel->addItemModel(DEF_SearchInGroup, _outScene_ModelName, new CT_Scene(), tr("Scène réduite"));
}

void ONF_StepReducePointsDensity::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addDouble(tr("Résolution de la grille :"), "cm", 0, 10000, 3, _resolution, 100);
}

void ONF_StepReducePointsDensity::compute()
{

    // Result OUT
    const QList<CT_ResultGroup*> &outResList = getOutResultList();
    CT_ResultGroup *outResult = outResList.first();

    CT_ResultGroupIterator itR(outResult, this, DEF_SearchInGroup);

    CT_PointAccessor pAccess;

    while(itR.hasNext() && !isStopped())
    {
        CT_StandardItemGroup* grp = (CT_StandardItemGroup*) itR.next();

        CT_Scene *in_scene = (CT_Scene*)grp->firstItemByINModelName(this, DEF_SearchInScene);

        if (in_scene != NULL)
        {
            CT_PointIterator itP(in_scene->getPointCloudIndex());
            size_t n_points = itP.size();

            _minx = in_scene->minX();
            _miny = in_scene->minY();
            _minz = in_scene->minZ();

            _dimx = ceil((in_scene->maxX() - _minx)/_resolution);
            _dimy = ceil((in_scene->maxY() - _miny)/_resolution);
            _dimz = ceil((in_scene->maxZ() - _minz)/_resolution);

            _halfResolution = _resolution / 2;

            PS_LOG->addMessage(LogInterface::info, LogInterface::step, QString(tr("La scène d'entrée comporte %1 points.")).arg(n_points));

            QMap<size_t, size_t> indexMap;

            // Extraction des points de la placette
            size_t i = 0;

            while(itP.hasNext() && !isStopped())
            {
                const CT_Point &point = itP.next().currentPoint();
                size_t pointIndex = itP.cIndex();

                size_t col, row, levz;
                size_t grdIndex = gridIndex(point(0), point(1), point(2), col, row, levz);

                size_t previousPointGlobalIndex = indexMap.value(grdIndex, std::numeric_limits<size_t>::max());

                if (previousPointGlobalIndex == std::numeric_limits<size_t>::max())
                {
                    indexMap.insert(grdIndex, pointIndex);
                } else {

                    double gridx, gridy, gridz;
                    cellCoordinates(col, row, levz, gridx, gridy, gridz);

                    double distance = pow(point(0) - gridx, 2) + pow(point(1) - gridy, 2) + pow(point(2) - gridz, 2);

                    const CT_Point &previousPoint = pAccess.constPointAt(previousPointGlobalIndex);

                    double previousDistance = pow(previousPoint(0) - gridx, 2) + pow(previousPoint(1) - gridy, 2) + pow(previousPoint(2) - gridz, 2);

                    if (distance < previousDistance)
                        indexMap.insert(grdIndex, pointIndex);
                }
                ++i;

                setProgress(50 * i / n_points);
            }


            QMapIterator<size_t, size_t> it(indexMap);

            size_t npts = indexMap.size();
            i = 0;

            CT_PointCloudIndexVector *resPointCloudIndex = new CT_PointCloudIndexVector(npts);
            resPointCloudIndex->setSortType(CT_AbstractCloudIndex::NotSorted);


            while (it.hasNext() && (!isStopped()))
            {
                it.next();

                resPointCloudIndex->replaceIndex(i, it.value());

                setProgress(50 + 49 * i / npts);
                ++i;
            }

            resPointCloudIndex->setSortType(CT_AbstractCloudIndex::SortedInAscendingOrder);

            if (resPointCloudIndex->size() > 0)
            {
                CT_Scene *outScene = new CT_Scene(_outScene_ModelName.completeName(), outResult);

                outScene->setBoundingBox(in_scene->minX(),in_scene->minY(),in_scene->minZ(), in_scene->maxX(),in_scene->maxY(),in_scene->maxZ());
                outScene->setPointCloudIndexRegistered(PS_REPOSITORY->registerPointCloudIndex(resPointCloudIndex));

                grp->addItemDrawable(outScene);

                PS_LOG->addMessage(LogInterface::info, LogInterface::step, QString(tr("La scène de densité réduite comporte %1 points.")).arg(outScene->getPointCloudIndex()->size()));
            } else {

                PS_LOG->addMessage(LogInterface::info, LogInterface::step, tr("Aucun point conservé pour cette scène"));
            }

        }

        setProgress(99);
    }

}

size_t ONF_StepReducePointsDensity::gridIndex(const double &x, const double &y, const double &z, size_t &colx, size_t &liny, size_t &levz) const
{
    colx = (size_t) floor((x - _minx) / _resolution);
    liny = (size_t) floor((y - _miny) / _resolution);
    levz = (size_t) floor((z - _minz) / _resolution);

    return levz*_dimx*_dimy + liny*_dimx + colx;;
}

void ONF_StepReducePointsDensity::cellCoordinates(const size_t &colx, const size_t &liny, const size_t &levz, double &x, double &y, double &z) const
{
    x = _minx + colx*_resolution + _halfResolution;
    y = _miny + liny*_resolution + _halfResolution;
    z = _minz + levz*_resolution + _halfResolution;
}
