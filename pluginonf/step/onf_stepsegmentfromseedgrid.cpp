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
    _maxDistZ = 1.5;
    _maxDistXY = 0.5;
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
        res->addItemModel(DEF_SearchInGroup, _outReverseTopologyGrid_ModelName, new CT_Grid3D_Points(), tr("Topologie inverse"));
    }
}


void ONF_StepSegmentFromSeedGrid::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addDouble(tr("Distance de recherche maximale en Z"), "m", 0, 1e+10, 2, _maxDistZ);
    configDialog->addDouble(tr("Distance de recherche maximale en XY"), "m", 0, 1e+10, 2, _maxDistXY);
}

void ONF_StepSegmentFromSeedGrid::compute()
{
    size_t NAval = std::numeric_limits<size_t>::max();

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
            CT_Grid3D_Sparse<size_t>* outTopologyGrid = new CT_Grid3D_Sparse<size_t>(_outTopologyGrid_ModelName.completeName(), outResult, pointGrid->minX(), pointGrid->minY(), pointGrid->minZ(), pointGrid->xdim(), pointGrid->ydim(), pointGrid->zdim(), pointGrid->resolution(), NAval, NAval);
            CT_Grid3D_Points* outReverseTopologyGrid = new CT_Grid3D_Points(_outReverseTopologyGrid_ModelName.completeName(), outResult, pointGrid->minX(), pointGrid->minY(), pointGrid->minZ(), pointGrid->xdim(), pointGrid->ydim(), pointGrid->zdim(), pointGrid->resolution());


            QList<size_t> list;
            seedGrid->getIndicesWithData(list);

            for (int i = 0 ; i < list.size() ; i++)
            {
                size_t index = list.at(i);
                outSegmentationGrid->setValueAtIndex(index, seedGrid->valueAtIndex(index));
            }

            setProgress(5.0);

            QList<size_t> filledCells;
            pointGrid->getIndicesWithPoints(filledCells);
            int size = filledCells.size();
            for (int i = 0 ; i < size ; i++)
            {
                if (outTopologyGrid->valueAtIndex(filledCells.at(i)) == NAval)
                {
                    findParentCell(outSegmentationGrid, outTopologyGrid, filledCells.at(i), true);
                }
                setProgress(5.0 + 64.0*((float)i / (float)size));
            }

            for (int i = filledCells.size() - 1 ; i >= 0 ; i--)
            {
                if (outTopologyGrid->valueAtIndex(filledCells.at(i)) == NAval)
                {
                    findParentCell(outSegmentationGrid, outTopologyGrid, filledCells.at(i), false);
                }
                setProgress(70.0 + 19.0*((float)(size - i) / (float)size));
            }


            for (int i = 0 ; i < size ; i++)
            {
                size_t index = filledCells.at(i);
                size_t value = outTopologyGrid->valueAtIndex(index);
                if (value != NAval)
                {
                    outReverseTopologyGrid->addPointAtIndex(value, index);
                }
                setProgress(90.0 + 9.0*((float)i / (float)size));
            }


            //            QMultiMap<double, size_t> sortedFilledCells;
            //            for (int i = 0 ; i < filledCells.size() ; i++)
            //            {
            //                size_t index = filledCells.at(i);
            //                Eigen::Vector3d center;
            //                outSegmentationGrid->getCellCenterCoordinates(index, center);
            //                sortedFilledCells.insert(center(0), index);
            //            }


            //            QMapIterator<double, size_t> itCell(sortedFilledCells);
            //            while (itCell.hasNext())
            //            {
            //                size_t index = itCell.next().value();
            //                if (outTopologyGrid->valueAtIndex(index) == -1)
            //                {
            //                    findParentCell(outSegmentationGrid, outTopologyGrid, index, true);
            //                }
            //            }

            //            itCell.toBack();
            //            while (itCell.hasPrevious())
            //            {
            //                size_t index = itCell.previous().value();
            //                if (outTopologyGrid->valueAtIndex(index) == -1)
            //                {
            //                    findParentCell(outSegmentationGrid, outTopologyGrid, index, false);
            //                }
            //            }


            outSegmentationGrid->computeMinMax();
            outTopologyGrid->computeMinMax();
            group->addItemDrawable(outSegmentationGrid);
            group->addItemDrawable(outTopologyGrid);
            group->addItemDrawable(outReverseTopologyGrid);
        }
    }

    setProgress(99);
}

void ONF_StepSegmentFromSeedGrid::findParentCell(CT_Grid3D_Sparse<int>* segmentationGrid, CT_Grid3D_Sparse<size_t>* topologyGrid, size_t cellIndex, bool growthUp)
{    
    Eigen::Vector3d baseCenter;
    if (!segmentationGrid->getCellCenterCoordinates(cellIndex, baseCenter)) {return;}
    int baseLabel = segmentationGrid->valueAtIndex(cellIndex);

    size_t xxbase, yybase, zzbase;
    segmentationGrid->indexToGrid(cellIndex, xxbase, yybase, zzbase);

    double maxDistZ2 = _maxDistZ*_maxDistZ;
    double maxDistXY2 = _maxDistXY*_maxDistXY;

    size_t ncellsXY = std::ceil(_maxDistXY / segmentationGrid->resolution());
    size_t firstX = 0;
    size_t lastX  = segmentationGrid->xdim() - 1;
    size_t firstY  = 0;
    size_t lastY  = segmentationGrid->ydim() - 1;

    if (xxbase >= ncellsXY) {firstX = xxbase - ncellsXY;}
    if ((xxbase + ncellsXY) < segmentationGrid->xdim()) {lastX = xxbase + ncellsXY;}

    if (yybase >= ncellsXY) {firstY = yybase - ncellsXY;}
    if ((yybase + ncellsXY) < segmentationGrid->xdim()) {lastY = yybase + ncellsXY;}

    size_t ncellsZ = std::ceil(_maxDistZ / segmentationGrid->resolution());
    size_t firstZ = 0;
    size_t lastZ  = 0;

    if (growthUp)
    {
        if (zzbase >= 1) {lastZ = zzbase - 1;}
        if (zzbase >= ncellsZ) {firstZ = zzbase - ncellsZ;}
    } else {
        if (zzbase < (segmentationGrid->zdim() - 1)) {firstZ = zzbase + 1;} else {firstZ = segmentationGrid->zdim() - 1;}
        if ((zzbase + ncellsZ) < segmentationGrid->zdim()) {lastZ = zzbase + ncellsZ;} else {lastZ = segmentationGrid->zdim() - 1;}
    }

    double smallestDist = sqrt(maxDistXY2 + maxDistZ2) + 0.00001;
    for (size_t zz = firstZ ; zz < lastZ ; zz++)
    {
        for (size_t xx = firstX ; xx <= lastX ; xx++)
        {
            for (size_t yy = firstY ; yy <= lastY ; yy++)
            {
                size_t currentIndex;
                if (segmentationGrid->index(xx, yy, zz, currentIndex))
                {
                    Eigen::Vector3d currentCenter;
                    if (segmentationGrid->getCellCenterCoordinates(currentIndex, currentCenter))
                    {
                        // double dist = pow(currentCenter(0) - baseCenter(0), 2) + pow(currentCenter(1) - baseCenter(1), 2) + pow(currentCenter(2) - baseCenter(2), 2);
                        double dx = currentCenter(0) - baseCenter(0);
                        double dy = currentCenter(1) - baseCenter(1);
                        double distXY = dx*dx + dy*dy;
                        if (distXY < maxDistXY2)
                        {
                            double dz = currentCenter(2) - baseCenter(2);
                            double dist = dx*dx + dy*dy + dz*dz;

                            if (dist < smallestDist)
                            {
                                int currentLabel = segmentationGrid->valueAtIndex(currentIndex);
                                if (currentLabel >= 0)
                                {
                                    if (baseLabel >= 0)
                                    {
                                        if (baseLabel == currentLabel && growthUp)
                                        {
                                            smallestDist = dist;
                                            topologyGrid->setValueAtIndex(cellIndex, currentIndex);
                                        }
                                    } else {
                                        smallestDist = dist;
                                        topologyGrid->setValueAtIndex(cellIndex, currentIndex);
                                        segmentationGrid->setValueAtIndex(cellIndex, currentLabel);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

}

