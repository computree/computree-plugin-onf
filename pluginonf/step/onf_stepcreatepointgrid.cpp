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


#include "onf_stepcreatepointgrid.h"

#include "ct_global/ct_context.h"

#include "ct_view/ct_stepconfigurabledialog.h"
#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/ct_outresultmodelgroupcopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"

#include "ct_result/ct_resultgroup.h"
#include "ct_itemdrawable/ct_scene.h"
#include "ct_itemdrawable/ct_grid3d_points.h"

#include "ct_pointcloudindex/ct_pointcloudindexvector.h"
#include "ct_iterator/ct_pointiterator.h"
#include "ct_iterator/ct_resultgroupiterator.h"
#include "ct_view/ct_buttongroup.h"


#include <math.h>
#include <iostream>
#include <QList>
#include <limits>

#define DEF_SearchInResult "rin"
#define DEF_SearchInGroup   "gin"
#define DEF_SearchInScene   "scin"

ONF_StepCreatePointGrid::ONF_StepCreatePointGrid(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _resolution   = 0.25;
}

QString ONF_StepCreatePointGrid::getStepDescription() const
{
    return tr("Créer une grille de points");
}

QString ONF_StepCreatePointGrid::getStepDetailledDescription() const
{
    return tr("");
}

CT_VirtualAbstractStep* ONF_StepCreatePointGrid::createNewInstance(CT_StepInitializeData &dataInit)
{
    // cree une copie de cette etape
    return new ONF_StepCreatePointGrid(dataInit);
}

//////////////////// PROTECTED //////////////////

void ONF_StepCreatePointGrid::createInResultModelListProtected()
{
    CT_InResultModelGroupToCopy *resultModel = createNewInResultModelForCopy(DEF_SearchInResult, tr("Scène(s)"), "", true);

    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_SearchInGroup);
    resultModel->addItemModel(DEF_SearchInGroup, DEF_SearchInScene, CT_Scene::staticGetType(), tr("Scène"));
}

// Création et affiliation des modèles OUT
void ONF_StepCreatePointGrid::createOutResultModelListProtected()
{    
    CT_OutResultModelGroupToCopyPossibilities *resultModel = createNewOutResultModelToCopy(DEF_SearchInResult);

    if (resultModel != NULL)
    {
        resultModel->addItemModel(DEF_SearchInGroup, _outPointGridModelName, new CT_Grid3D_Points(), tr("Grille de points"));
    }
}

void ONF_StepCreatePointGrid::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addDouble(tr("Résolution :"), "m", 0, 1e+10, 2, _resolution);
}

void ONF_StepCreatePointGrid::compute()
{
    // récupération du résultats IN et OUT
    CT_ResultGroup *outResult = getOutResultList().first();

    CT_ResultGroupIterator it(outResult, this, DEF_SearchInGroup);
    while(!isStopped() && it.hasNext())
    {
        CT_StandardItemGroup *group = (CT_StandardItemGroup*) it.next();

        if (group != NULL)
        {
            const CT_Scene *in_scene = (CT_Scene*) group->firstItemByINModelName(this, DEF_SearchInScene);
            const CT_AbstractPointCloudIndex *pointCloudIndex = in_scene->getPointCloudIndex();
            size_t n_points = pointCloudIndex->size();

            CT_Grid3D_Points* grid = CT_Grid3D_Points::createGrid3DFromXYZCoords(_outPointGridModelName.completeName(), outResult,
                                                                in_scene->minX() - 1.0, in_scene->minY() - 1.0, in_scene->minZ() - 1.0,
                                                                in_scene->maxX() + 1.0, in_scene->maxY() + 1.0, in_scene->maxZ() + 1.0,
                                                                _resolution, false);


            PS_LOG->addMessage(LogInterface::info, LogInterface::step, QString(tr("La scène d'entrée comporte %1 points.")).arg(n_points));

            int i = 0;
            CT_PointIterator itP(pointCloudIndex);
            while(itP.hasNext() && (!isStopped()))
            {
                const CT_Point &pt = itP.next().currentPoint();
                size_t index = itP.currentGlobalIndex();

                grid->addPoint(index, pt(0), pt(1), pt(2));

                // progres de 0 à 100
                setProgress(100.0*i/n_points);
                ++i;
            }

            group->addItemDrawable(grid);
        }
    }

}
