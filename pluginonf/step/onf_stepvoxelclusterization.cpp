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


#include "onf_stepvoxelclusterization.h"

#ifdef USE_OPENCV

#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"

// Inclusion of actions methods
#include "ct_tools/model/ct_outmodelcopyactionaddmodelitemingroup.h"

// Inclusion of standard result class
#include "ct_result/ct_resultgroup.h"

// Inclusion of used ItemDrawable classes
#include "ct_itemdrawable/ct_scene.h"
#include "ct_itemdrawable/ct_pointcluster.h"
#include "ct_itemdrawable/ct_grid3d_sparse.h"
#include "ct_iterator/ct_pointiterator.h"

#include "ct_view/ct_stepconfigurabledialog.h"

#include <QFileInfo>
#include <QDebug>
#include <limits>

#define DEF_SearchInResult "r"
#define DEF_SearchInGroup  "g"
#define DEF_SearchInScene   "i"


ONF_StepVoxelClusterization::ONF_StepVoxelClusterization(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _res = 0.1;
    _gridMode = 0;
    _xBase = 0;
    _yBase = 0;
    _zBase = 0;
}

QString ONF_StepVoxelClusterization::getStepDescription() const
{
    // Gives the descrption to print in the GUI
    return tr("Clusterization selon une grille voxel");
}

// Step description (tooltip of contextual menu)
QString ONF_StepVoxelClusterization::getStepDetailledDescription() const
{
    return tr("TO DO");
}

CT_VirtualAbstractStep* ONF_StepVoxelClusterization::createNewInstance(CT_StepInitializeData &dataInit)
{
    // Creates an instance of this step
    return new ONF_StepVoxelClusterization(dataInit);
}

void ONF_StepVoxelClusterization::createInResultModelListProtected()
{
    CT_InResultModelGroupToCopy * resultModel = createNewInResultModelForCopy(DEF_SearchInResult, tr("Scène(s)"));
    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_SearchInGroup, CT_StandardItemGroup::staticGetType(), tr("Groupe"));
    resultModel->addItemModel(DEF_SearchInGroup, DEF_SearchInScene, CT_AbstractItemDrawableWithPointCloud::staticGetType(), tr("Item à clusteriser"));
}

void ONF_StepVoxelClusterization::createOutResultModelListProtected()
{    
    CT_OutResultModelGroupToCopyPossibilities *resultModel = createNewOutResultModelToCopy(DEF_SearchInResult);

    if(resultModel != NULL) {
        resultModel->addGroupModel(DEF_SearchInGroup, m_outClusterGroupModel, new CT_StandardItemGroup(), tr("Cluster (Grp)"));
        resultModel->addItemModel(m_outClusterGroupModel, m_outClusterModel, new CT_PointCluster(), tr("Cluster (Points)"));
    }
}

void ONF_StepVoxelClusterization::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addDouble(tr("Résolution de la grille"),tr("meters"),0.0001,10000,2, _res );

    configDialog->addText(tr("Callage du coin (minX, minY, minZ) :"),"", "");

    CT_ButtonGroup &bg_gridMode = configDialog->addButtonGroup(_gridMode);
    configDialog->addExcludeValue("", "", tr("Sur la boite englobante de la scène"), bg_gridMode, 0);
    configDialog->addExcludeValue("", "", tr("Par rapport aux coordonnées suivantes :"), bg_gridMode, 1);

    configDialog->addDouble(tr("Coordonnée X :"), "", -std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), 4, _xBase);
    configDialog->addDouble(tr("Coordonnée Y :"), "", -std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), 4, _yBase);
    configDialog->addDouble(tr("Coordonnée Z :"), "", -std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), 4, _zBase);
}

void ONF_StepVoxelClusterization::compute()
{
    // Gets the out result
    CT_ResultGroup* outResult = getOutResultList().first();

    CT_ResultGroupIterator it(outResult, this, DEF_SearchInGroup);
    // iterate over all groups
    while(!isStopped() && it.hasNext())
    {

        CT_StandardItemGroup *group = (CT_StandardItemGroup*)it.next();
        const CT_Scene* scene = (CT_Scene*)group->firstItemByINModelName(this, DEF_SearchInScene);

        if (scene!=NULL)
        {
            float minX = _xBase;
            float minY = _yBase;
            float minZ = _zBase;

            if (_gridMode == 0)
            {
                minX = scene->minX();
                minY = scene->minY();
                minZ = scene->minZ();
            } else {

                while (minX < scene->minX()) {minX += _res;};
                while (minY < scene->minY()) {minY += _res;};
                while (minZ < scene->minZ()) {minZ += _res;};

                while (minX > scene->minX()) {minX -= _res;};
                while (minY > scene->minY()) {minY -= _res;};
                while (minZ > scene->minZ()) {minZ -= _res;};
            }


            // Declaring the output grids
            CT_Grid3D_Sparse<int>* hitGrid = CT_Grid3D_Sparse<int>::createGrid3DFromXYZCoords(NULL, NULL,
                                                                                              minX, minY, minZ,
                                                                                              scene->maxX(), scene->maxY(), scene->maxZ(),
                                                                                              _res, -1, 0);


            QList<CT_PointCluster*> clusters;
            clusters.append(NULL);
            int clusterNumber = 1;

            const CT_AbstractPointCloudIndex *pointCloudIndex = scene->getPointCloudIndex();
            CT_PointIterator itP(pointCloudIndex);
            while(itP.hasNext() && (!isStopped()))
            {
                const CT_Point &point = itP.next().currentPoint();
                size_t index = itP.currentGlobalIndex();

                int val = hitGrid->valueAtXYZ(point(0), point(1), point(2));

                if (val == 0)
                {
                    val = clusterNumber;
                    hitGrid->setValueAtXYZ(point(0), point(1), point(2), clusterNumber++);
                    clusters.append(new CT_PointCluster(m_outClusterModel.completeName(), outResult));
                }
                clusters.at(val)->addPoint(index);
            }

            for (int i = 1 ; i < clusters.size() ; i++)
            {
                CT_StandardItemGroup* grpCl = new CT_StandardItemGroup(m_outClusterGroupModel.completeName(), outResult);
                group->addGroup(grpCl);
                grpCl->addItemDrawable(clusters.at(i));
            }

            delete hitGrid;
        }
    }

    setProgress(99);
}


#endif
