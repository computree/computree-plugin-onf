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


#include "onf_stepsegmentfromseedgrid.h"

#include "ct_result/model/inModel/ct_inresultmodelgroup.h"
#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"

// Inclusion of actions methods
#include "ct_tools/model/ct_outmodelcopyactionaddmodelitemingroup.h"
#include "ct_iterator/ct_pointiterator.h"
#include "ct_iterator/ct_resultgroupiterator.h"
#include "ct_iterator/ct_resultitemiterator.h"

// Inclusion of standard result class
#include "ct_result/ct_resultgroup.h"

// Inclusion of used ItemDrawable classes
#include "ct_itemdrawable/ct_scene.h"
#include "ct_itemdrawable/ct_grid3d_sparse.h"
#include "ct_itemdrawable/ct_grid3d_points.h"

#include "ct_view/ct_stepconfigurabledialog.h"

#include <QFileInfo>
#include <QDebug>
#include <limits>

#define DEF_SearchInResult "r"
#define DEF_SearchInGroup   "gr"
#define DEF_SearchInGridPoints   "gridPts"
#define DEF_SearchInGridSeeds   "gridSeeds"

ONF_StepSegmentFromSeedGrid::ONF_StepSegmentFromSeedGrid(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _maxDist = 2.0;
}

QString ONF_StepSegmentFromSeedGrid::getStepDescription() const
{
    // Gives the descrption to print in the GUI
    return tr("Segmenter à partir d'une grille de graines");
}

// Step description (tooltip of contextual menu)
QString ONF_StepSegmentFromSeedGrid::getStepDetailledDescription() const
{
    return tr("");
}

CT_VirtualAbstractStep* ONF_StepSegmentFromSeedGrid::createNewInstance(CT_StepInitializeData &dataInit)
{
    // Creates an instance of this step
    return new ONF_StepSegmentFromSeedGrid(dataInit);
}

void ONF_StepSegmentFromSeedGrid::createInResultModelListProtected()
{
    CT_InResultModelGroupToCopy *resultModel = createNewInResultModelForCopy(DEF_SearchInResult, tr("Grilles"), "", true);

    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_SearchInGroup);
    resultModel->addItemModel(DEF_SearchInGroup, DEF_SearchInGridPoints, CT_Grid3D_Points::staticGetType(), tr("Grille de points"), "", CT_InAbstractModel::C_ChooseMultipleIfMultiple);
    resultModel->addItemModel(DEF_SearchInGroup, DEF_SearchInGridSeeds, CT_Grid3D_Sparse<int>::staticGetType(), tr("Grille de graines"), "", CT_InAbstractModel::C_ChooseMultipleIfMultiple);
}

void ONF_StepSegmentFromSeedGrid::createOutResultModelListProtected()
{    
    CT_OutResultModelGroupToCopyPossibilities *res = createNewOutResultModelToCopy(DEF_SearchInResult);

    if(res != NULL)
    {
        res->addItemModel(DEF_SearchInGroup, _outSegmentationGrid_ModelName, new CT_Grid3D_Sparse<int>(), tr("Segmentation"));
        res->addItemModel(DEF_SearchInGroup, _outTopologyGrid_ModelName, new CT_Grid3D_Sparse<size_t>(), tr("Topologie"));
    }
}


void ONF_StepSegmentFromSeedGrid::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addDouble(tr("Distance de recherche maximale"), "m", 0, 1e+10, 2, _maxDist);
}

void ONF_StepSegmentFromSeedGrid::compute()
{
    CT_ResultGroup* outResult = getOutResultList().first();

    CT_ResultGroupIterator itOut(outResult, this, DEF_SearchInGroup);
    // iterate over all groups
    while(itOut.hasNext())
    {
        CT_AbstractItemGroup *group = (CT_AbstractItemGroup*)itOut.next();
        CT_Grid3D_Points* pointGrid = (CT_Grid3D_Points*)group->firstItemByINModelName(this, DEF_SearchInGridPoints);
        CT_Grid3D_Sparse<int>* seedGrid = (CT_Grid3D_Sparse<int>*)group->firstItemByINModelName(this, DEF_SearchInGridSeeds);

        if (pointGrid != NULL && seedGrid != NULL)
        {
            // Declaring the output grids
            CT_Grid3D_Sparse<int>* outSegmentationGrid = new CT_Grid3D_Sparse<int>(_outSegmentationGrid_ModelName.completeName(), outResult, pointGrid->minX(), pointGrid->minY(), pointGrid->minZ(), pointGrid->xdim(), pointGrid->ydim(), pointGrid->zdim(), pointGrid->resolution(), -1, -1);
            CT_Grid3D_Sparse<size_t>* outTopologyGrid = new CT_Grid3D_Sparse<size_t>(_outTopologyGrid_ModelName.completeName(), outResult, pointGrid->minX(), pointGrid->minY(), pointGrid->minZ(), pointGrid->xdim(), pointGrid->ydim(), pointGrid->zdim(), pointGrid->resolution(), -1, -1);

            QList<size_t> list;
            seedGrid->getIndicesWithData(list);

            for (int i = 0 ; i < list.size() ; i++)
            {
                size_t index = list.at(i);
                outSegmentationGrid->setValueAtIndex(index, seedGrid->valueAtIndex(index));
            }






            outSegmentationGrid->computeMinMax();
            outTopologyGrid->computeMinMax();
            group->addItemDrawable(outSegmentationGrid);
            group->addItemDrawable(outTopologyGrid);
        }
    }

    setProgress(99);
}

bool ONF_StepSegmentFromSeedGrid::findParentCell(double maxDist, bool up, int baseSeed)
{
    return true;

}

