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


#include "onf_stepcomputetin.h"

#include "ct_global/ct_context.h"

#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/ct_outresultmodelgroupcopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"

#include "ct_result/ct_resultgroup.h"

#include "ct_itemdrawable/ct_scene.h"
#include "ct_itemdrawable/ct_triangulation2d.h"
#include "ct_pointcloudindex/ct_pointcloudindexvector.h"
#include "ct_iterator/ct_pointiterator.h"
#include "ct_view/ct_stepconfigurabledialog.h"

#define DEF_SearchInResult   "ires"
#define DEF_SearchInGroup   "igrp"
#define DEF_SearchInScene   "isc"


ONF_StepComputeTIN::ONF_StepComputeTIN(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{

}

QString ONF_StepComputeTIN::getStepDescription() const
{
    return tr("Créer TIN à partir de points");
}

QString ONF_StepComputeTIN::getStepDetailledDescription() const
{
    return tr("TO DO");
}

CT_VirtualAbstractStep* ONF_StepComputeTIN::createNewInstance(CT_StepInitializeData &dataInit)
{
    // cree une copie de cette etape
    return new ONF_StepComputeTIN(dataInit);
}

/////////////////////// PROTECTED ///////////////////////

void ONF_StepComputeTIN::createInResultModelListProtected()
{  
    CT_InResultModelGroupToCopy *resultModel = createNewInResultModelForCopy(DEF_SearchInResult, tr("Points sol"));

    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_SearchInGroup);
    resultModel->addItemModel(DEF_SearchInGroup, DEF_SearchInScene, CT_Scene::staticGetType(), tr("Points sol"));
}

void ONF_StepComputeTIN::createPostConfigurationDialog()
{
    //CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();
}

void ONF_StepComputeTIN::createOutResultModelListProtected()
{
    CT_OutResultModelGroupToCopyPossibilities *resultModel = createNewOutResultModelToCopy(DEF_SearchInResult);

    if(resultModel != NULL)
        resultModel->addItemModel(DEF_SearchInGroup, _outTIN_ModelName, new CT_Triangulation2D(), tr("TIN"));
}

void ONF_StepComputeTIN::compute()
{
    // recupere les resultats de sortie
    const QList<CT_ResultGroup*> &outResList = getOutResultList();
    CT_ResultGroup *outResult = outResList.at(0);

    CT_ResultGroupIterator it(outResult, this, DEF_SearchInGroup);
    while (!isStopped() && it.hasNext())
    {
        CT_StandardItemGroup* group = (CT_StandardItemGroup*) it.next();

        if (group != NULL)
        {
            const CT_Scene *scene = (const CT_Scene*)group->firstItemByINModelName(this, DEF_SearchInScene);
            const CT_AbstractPointCloudIndex *pointCloudIndex = scene->getPointCloudIndex();

            CT_DelaunayTriangulation *delaunay = new CT_DelaunayTriangulation();
            delaunay->init(scene->minX() - 1.0, scene->minY() - 1.0, scene->maxX() + 1.0, scene->maxY() + 1.0);

            CT_PointIterator itP(pointCloudIndex);
            while(itP.hasNext() && !isStopped())
            {
                const CT_Point &point =itP.next().currentPoint();

                Eigen::Vector3d* pt = new Eigen::Vector3d(point);
                delaunay->addVertex(pt, true);
            }

            delaunay->doInsertion();

            CT_Triangulation2D* triangulation = new CT_Triangulation2D(_outTIN_ModelName.completeName(), outResult, delaunay);
            group->addItemDrawable(triangulation);
        }
        setProgress(100);
    }
}
