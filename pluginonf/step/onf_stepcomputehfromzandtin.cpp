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

#include "onf_stepcomputehfromzandtin.h"


#include "ct_global/ct_context.h"

#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/ct_outresultmodelgroupcopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"

#include "ct_result/ct_resultgroup.h"

#include "ct_itemdrawable/ct_scene.h"
#include "ct_itemdrawable/ct_triangulation2d.h"
#include "ct_itemdrawable/abstract/ct_abstractareashape2d.h"
#include "ct_pointcloudindex/ct_pointcloudindexvector.h"
#include "ct_iterator/ct_pointiterator.h"
#include "ct_iterator/ct_mutablepointiterator.h"

#include "ct_view/ct_stepconfigurabledialog.h"


#define DEF_SearchInResult  "ires"
#define DEF_SearchInGroup   "igrp"
#define DEF_SearchInTIN     "TIN"
#define DEF_SearchInSceneZ  "sceneZ"


ONF_StepComputeHfromZandTIN::ONF_StepComputeHfromZandTIN(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
}

QString ONF_StepComputeHfromZandTIN::getStepDescription() const
{
    return tr("Calculer les hauteurs à l'aide d'un TIN");
}

QString ONF_StepComputeHfromZandTIN::getStepDetailledDescription() const
{
    return tr("TO DO");
}

CT_VirtualAbstractStep* ONF_StepComputeHfromZandTIN::createNewInstance(CT_StepInitializeData &dataInit)
{
    // cree une copie de cette etape
    return new ONF_StepComputeHfromZandTIN(dataInit);
}

/////////////////////// PROTECTED ///////////////////////

void ONF_StepComputeHfromZandTIN::createInResultModelListProtected()
{
    CT_InResultModelGroupToCopy *resultModel = createNewInResultModelForCopy(DEF_SearchInResult, tr("TIN"));

    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_SearchInGroup);
    resultModel->addItemModel(DEF_SearchInGroup, DEF_SearchInSceneZ, CT_AbstractItemDrawableWithPointCloud::staticGetType(), tr("Scène en Z"));
    resultModel->addItemModel(DEF_SearchInGroup, DEF_SearchInTIN, CT_Triangulation2D::staticGetType(), tr("TIN"));
}

void ONF_StepComputeHfromZandTIN::createPostConfigurationDialog()
{
    //CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();
}

void ONF_StepComputeHfromZandTIN::createOutResultModelListProtected()
{
    CT_OutResultModelGroupToCopyPossibilities *resultModel = createNewOutResultModelToCopy(DEF_SearchInResult);

    if(resultModel != NULL)
    {
        resultModel->addItemModel(DEF_SearchInGroup, _outSceneH_ModelName, new CT_Scene(), tr("Scene en H"));
    }
}

void ONF_StepComputeHfromZandTIN::compute()
{
    // recupere les resultats de sortie
    const QList<CT_ResultGroup*> &outResList = getOutResultList();
    // récupération des modéles out
    CT_ResultGroup *outResult = outResList.at(0);

    CT_ResultGroupIterator it(outResult, this, DEF_SearchInGroup);
    while (!isStopped() && it.hasNext())
    {
        CT_StandardItemGroup* group = (CT_StandardItemGroup*) it.next();

        if (group != NULL)
        {
            const CT_AbstractItemDrawableWithPointCloud *in_sceneZ = (const CT_AbstractItemDrawableWithPointCloud*)group->firstItemByINModelName(this, DEF_SearchInSceneZ);
            const CT_Triangulation2D *tin = (const CT_Triangulation2D*)group->firstItemByINModelName(this, DEF_SearchInTIN);

            if (in_sceneZ != NULL && tin != NULL)
            {                
                Eigen::Vector3d min, max;
                tin->getBoundingBox(min, max);
                CT_DelaunayTriangulation *triangulation = tin->getDelaunayT();

                const CT_AbstractPointCloudIndex *pointCloudIndex = in_sceneZ->getPointCloudIndex();
                size_t n_points = pointCloudIndex->size();
                PS_LOG->addMessage(LogInterface::info, LogInterface::step, QString(tr("La scène d'entrée comporte %1 points.")).arg(n_points));

                CT_NMPCIR outPointCloudIndex = PS_REPOSITORY->createNewPointCloud(n_points);
                CT_MutablePointIterator itPM(outPointCloudIndex);

                int i = 0;
                double zTin = 0;
                CT_DelaunayTriangle* refTri = NULL;
                CT_PointIterator itP(pointCloudIndex);
                while(itP.hasNext() && (!isStopped()))
                {
                    CT_Point point = itP.next().currentPoint();
                    refTri = (CT_DelaunayTriangle*) triangulation->getZCoordForXY(point(0), point(1), zTin, refTri);

                    if (std::isnan(zTin))
                    {
                        point(2)  = 0;
                    } else {
                        point(2) -= zTin;
                    }

                    itPM.next().replaceCurrentPoint(point);

                    // progres de 0 à 100
                    if (i % 10000 == 0) {setProgress(100.0*i/n_points);}
                    ++i;
                }



                setProgress(90);
                PS_LOG->addMessage(LogInterface::info, LogInterface::step, tr("Convertion terminée"));

                // ajout de la scène en H
                CT_Scene *outSceneH = new CT_Scene(_outSceneH_ModelName.completeName(), outResult, outPointCloudIndex);
                outSceneH->updateBoundingBox();
                group->addItemDrawable(outSceneH);
            }
        }
    }
    setProgress(100);
}
