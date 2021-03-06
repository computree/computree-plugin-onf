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


#include "onf_stepextractplotbasedondtm.h"

#ifdef USE_OPENCV

#include "ct_global/ct_context.h"

#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/inModel/ct_inresultmodelgroup.h"
#include "ct_result/model/outModel/ct_outresultmodelgroup.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"

#include "ct_itemdrawable/abstract/ct_abstractitemdrawablewithpointcloud.h"
#include "ct_itemdrawable/ct_referencepoint.h"
#include "ct_itemdrawable/ct_image2d.h"
#include "ct_result/ct_resultgroup.h"
#include "ct_itemdrawable/abstract/ct_abstractitemgroup.h"
#include "ct_itemdrawable/ct_scene.h"

#include "ct_pointcloudindex/ct_pointcloudindexvector.h"
#include "ct_iterator/ct_pointiterator.h"
#include "ct_view/ct_stepconfigurabledialog.h"

#include "qdebug.h"
#include <limits>


#define DEF_SearchInResult  "ires"
#define DEF_SearchInGroup   "igrp"
#define DEF_SearchInScene   "isc"

#define DEF_SearchInMNTResult   "mntres"
#define DEF_SearchInMNTGroup    "mntgrp"
#define DEF_SearchInMNT         "mntitem"

#define DEF_SearchOutMNTResult       "rmnt"

ONF_StepExtractPlotBasedOnDTM::ONF_StepExtractPlotBasedOnDTM(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _zmin = 1.0;
    _zmax = 1.6;
}

QString ONF_StepExtractPlotBasedOnDTM::getStepDescription() const
{
    return tr("Extraire les points dans une tranche parallèle au MNT");
}

QString ONF_StepExtractPlotBasedOnDTM::getStepDetailledDescription() const
{
    return tr("Cette étape permet d'extraire les points de la scène d'entrée contenus dans une tranche de hauteur depuis le MNT.");
}

CT_VirtualAbstractStep* ONF_StepExtractPlotBasedOnDTM::createNewInstance(CT_StepInitializeData &dataInit)
{
    // cree une copie de cette etape
    return new ONF_StepExtractPlotBasedOnDTM(dataInit);
}

//////////////////// PROTECTED //////////////////

void ONF_StepExtractPlotBasedOnDTM::createInResultModelListProtected()
{
    CT_InResultModelGroup *resultMNTModel = createNewInResultModel(DEF_SearchInMNTResult, tr("MNT (Raster)"), "", true);
    resultMNTModel->setZeroOrMoreRootGroup();
    resultMNTModel->addGroupModel("", DEF_SearchInMNTGroup);
    resultMNTModel->addItemModel(DEF_SearchInMNTGroup, DEF_SearchInMNT, CT_Image2D<float>::staticGetType(), tr("Modèle Numérique de Terrain"));

    CT_InResultModelGroupToCopy *resultModel = createNewInResultModelForCopy(DEF_SearchInResult, tr("Scène(s)"));
    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_SearchInGroup);
    resultModel->addItemModel(DEF_SearchInGroup, DEF_SearchInScene, CT_Scene::staticGetType(), tr("Scène"));
}

void ONF_StepExtractPlotBasedOnDTM::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addDouble(tr("H minimum :"), "m", -1e+10, 1e+10, 2, _zmin);
    configDialog->addDouble(tr("H maximum :"), "m", -1e+10, 1e+10, 2, _zmax);
}

void ONF_StepExtractPlotBasedOnDTM::createOutResultModelListProtected()
{
    CT_OutResultModelGroupToCopyPossibilities *res = createNewOutResultModelToCopy(DEF_SearchInResult);

    if(res != NULL)
        res->addItemModel(DEF_SearchInGroup, _outSceneModel, new CT_Scene(), tr("Scène extraite"));
}

void ONF_StepExtractPlotBasedOnDTM::compute()
{
    // Récupération du résultat de sortie
    CT_ResultGroup* inMNTResult = getInputResults().at(0);
    CT_ResultGroup *outResult = getOutResultList().at(0);

    CT_ResultItemIterator it(inMNTResult, this, DEF_SearchInMNT);
    if(it.hasNext())
    {
        CT_Image2D<float>* mnt = (CT_Image2D<float>*) it.next();

        if (mnt != NULL)
        {
            double na = mnt->NA();

            CT_ResultGroupIterator itsc(outResult, this, DEF_SearchInGroup);
            while(!isStopped() && itsc.hasNext())
            {
                CT_StandardItemGroup *group = (CT_StandardItemGroup*)itsc.next();
                const CT_Scene *in_scene = (const CT_Scene*)group->firstItemByINModelName(this, DEF_SearchInScene);

                if(in_scene != NULL)
                {
                    const CT_AbstractPointCloudIndex *pointCloudIndex = in_scene->getPointCloudIndex();
                    size_t n_points = pointCloudIndex->size();

                    PS_LOG->addMessage(LogInterface::info, LogInterface::step, QString(tr("La scène d'entrée comporte %1 points.")).arg(n_points));

                    CT_PointCloudIndexVector *resPointCloudIndex = new CT_PointCloudIndexVector();
                    resPointCloudIndex->setSortType(CT_PointCloudIndexVector::NotSorted);

                    // Extraction des points de la placette
                    size_t i = 0;

                    CT_PointIterator itP(pointCloudIndex);
                    while(itP.hasNext() && !isStopped())
                    {
                        const CT_Point &point = itP.next().currentPoint();
                        size_t index = itP.currentGlobalIndex();

                        double hauteur;
                        double zMNT = mnt->valueAtCoords(point(0), point(1));
                        if (zMNT != na) {
                            hauteur = point(2) - zMNT;
                        } else {
                            hauteur = -std::numeric_limits<double>::max();
                        }

                        if (hauteur >= _zmin && hauteur <= _zmax) {

                            resPointCloudIndex->addIndex(index);

                        }

                        // progres de 0 à 100
                        setProgress(100.0*i/n_points);
                        ++i;
                    }

                    if (resPointCloudIndex->size() > 0)
                    {
                        resPointCloudIndex->setSortType(CT_PointCloudIndexVector::SortedInAscendingOrder);

                        // creation et ajout de la scene
                        CT_Scene *outScene = new CT_Scene(_outSceneModel.completeName(), outResult, PS_REPOSITORY->registerPointCloudIndex(resPointCloudIndex));

                        outScene->updateBoundingBox();
                        group->addItemDrawable(outScene);

                        PS_LOG->addMessage(LogInterface::info, LogInterface::step, QString(tr("La scène extraite comporte %1 points.")).arg(outScene->getPointCloudIndex()->size()));

                    } else {
                        delete resPointCloudIndex;
                        PS_LOG->addMessage(LogInterface::info, LogInterface::step, tr("Aucun point n'est dans l'emprise choisie"));

                    }
                }
            }
        }
    }
}
#endif
