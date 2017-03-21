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


#include "onf_stepcreateseedgrid.h"

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
#include "tools/onf_computehitsthread.h"

#ifdef USE_OPENCV
#include "ct_itemdrawable/ct_image2d.h"
#endif

#include "ct_view/ct_stepconfigurabledialog.h"

#include <QFileInfo>
#include <QDebug>
#include <limits>

#define DEF_SearchInResultSc "rsc"
#define DEF_SearchInGroupSc   "grsc"
#define DEF_SearchInScene   "sc"

#define DEF_SearchInResultIt "rit"
#define DEF_SearchInGroupIt   "grit"
#define DEF_SearchInItem   "item"

#define DEF_SearchInResultDTM "rdtm"
#define DEF_SearchInGroupDTM  "grdtm"
#define DEF_SearchInDTM   "dtm"

#define DEF_SearchInResult "r"
#define DEF_SearchInGroup   "gr"
#define DEF_SearchInGrid   "grid"

ONF_StepCreateSeedGrid::ONF_StepCreateSeedGrid(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _mode = 0;
    _offset = 1.3;
}

QString ONF_StepCreateSeedGrid::getStepDescription() const
{
    // Gives the descrption to print in the GUI
    return tr("Créer une grille de graines");
}

// Step description (tooltip of contextual menu)
QString ONF_StepCreateSeedGrid::getStepDetailledDescription() const
{
    return tr("");
}

CT_VirtualAbstractStep* ONF_StepCreateSeedGrid::createNewInstance(CT_StepInitializeData &dataInit)
{
    // Creates an instance of this step
    return new ONF_StepCreateSeedGrid(dataInit);
}

void ONF_StepCreateSeedGrid::createInResultModelListProtected()
{
    if (_mode == 0 || _mode == 2)
    {
        CT_InResultModelGroup* resultModelItem = createNewInResultModel(DEF_SearchInResultIt, tr("Items(s)"), "", true);
        resultModelItem->setZeroOrMoreRootGroup();
        resultModelItem->addGroupModel("", DEF_SearchInGroupIt);
        resultModelItem->addItemModel(DEF_SearchInGroupIt, DEF_SearchInItem, CT_AbstractSingularItemDrawable::staticGetType(), tr("Item"));
    }

    if (_mode == 1)
    {
        CT_InResultModelGroup* resultModelScene = createNewInResultModel(DEF_SearchInResultSc, tr("Scène(s)"), "", true);
        resultModelScene->setZeroOrMoreRootGroup();
        resultModelScene->addGroupModel("", DEF_SearchInGroupSc);
        resultModelScene->addItemModel(DEF_SearchInGroupSc, DEF_SearchInScene, CT_AbstractItemDrawableWithPointCloud::staticGetType(), tr("Scène"));
    }


#ifdef USE_OPENCV
    if (_mode == 2)
    {
        CT_InResultModelGroupToCopy *resultModelDTM = createNewInResultModelForCopy(DEF_SearchInResultDTM, tr("MNT"), "", true);
        resultModelDTM->setZeroOrMoreRootGroup();
        resultModelDTM->addGroupModel("", DEF_SearchInGroupDTM);
        resultModelDTM->addItemModel(DEF_SearchInGroupDTM, DEF_SearchInDTM, CT_Image2D<float>::staticGetType(), tr("MNT"));
    }
#endif



    CT_InResultModelGroupToCopy *resultModel = createNewInResultModelForCopy(DEF_SearchInResult, tr("Grille"), "", true);

    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_SearchInGroup);
    resultModel->addItemModel(DEF_SearchInGroup, DEF_SearchInGrid, CT_AbstractGrid3D::staticGetType(), tr("Grille"), "", CT_InAbstractModel::C_ChooseMultipleIfMultiple);
}

void ONF_StepCreateSeedGrid::createOutResultModelListProtected()
{    
    CT_OutResultModelGroupToCopyPossibilities *res = createNewOutResultModelToCopy(DEF_SearchInResult);

    if(res != NULL)
    {
        res->addItemModel(DEF_SearchInGroup, _outSeedGrid_ModelName, new CT_Grid3D_Sparse<int>(), tr("Grille de graines"));
    }
}

void ONF_StepCreateSeedGrid::createPreConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPreConfigurationDialog();

    CT_ButtonGroup &bg_mode = configDialog->addButtonGroup(_mode);

    configDialog->addExcludeValue("", "", tr("Items"), bg_mode, 0);
    configDialog->addExcludeValue("", "", tr("Scènes de points"), bg_mode, 1);

#ifdef USE_OPENCV
    configDialog->addExcludeValue("", "", tr("Positions2D + MNT"), bg_mode, 2);
#endif

}

void ONF_StepCreateSeedGrid::createPostConfigurationDialog()
{
    if (_mode == 2)
    {
        CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();
        configDialog->addDouble(tr("Offset :"), "m", -1e+10, 1e+10, 2, _offset);
    }
}

void ONF_StepCreateSeedGrid::compute()
{
    // Gets the out result
    CT_ResultGroup* inResult = getInputResults().first();
    CT_ResultGroup* outResult = getOutResultList().first();


#ifdef USE_OPENCV
    CT_Image2D<float> *dtm;
    if (_mode == 2 && getInputResults().size() > 1)
    {
        CT_ResultGroup* inResultDTM = getInputResults().at(1);
        CT_ResultItemIterator itDTM(inResultDTM, this, DEF_SearchInDTM);
        if (itDTM.hasNext())
        {
            dtm = (CT_Image2D<float> *)itDTM.next();
        }
    }
#endif


    CT_ResultGroupIterator itOut(outResult, this, DEF_SearchInGroup);
    // iterate over all groups
    while(itOut.hasNext())
    {
        CT_AbstractItemGroup *group = (CT_AbstractItemGroup*)itOut.next();
        const CT_AbstractGrid3D* gridIn = (CT_AbstractGrid3D*)group->firstItemByINModelName(this, DEF_SearchInGrid);

        if (gridIn != NULL)
        {
            // Declaring the output grids
            CT_Grid3D_Sparse<int>* outGrid = new CT_Grid3D_Sparse<int>(_outSeedGrid_ModelName.completeName(), outResult, gridIn->minX(), gridIn->minY(), gridIn->minZ(), gridIn->xdim(), gridIn->ydim(), gridIn->zdim(), gridIn->resolution(), -1, -1);

            if (_mode == 0)
            {
                int cpt = 1;
                CT_ResultItemIterator it(inResult, this, DEF_SearchInItem);
                // iterate over all groups
                while(it.hasNext())
                {
                    const CT_AbstractSingularItemDrawable *item = it.next();
                   outGrid->setValueAtXYZ(item->getCenterX(), item->getCenterY(), item->getCenterZ(), cpt);
                    ++cpt;
                }

            } else if (_mode == 1)
            {
                int cpt = 1;
                CT_ResultItemIterator it(inResult, this, DEF_SearchInScene);
                // iterate over all groups
                while(it.hasNext())
                {
                    CT_Scene *scene = (CT_Scene*)it.next();

                    const CT_AbstractPointCloudIndex *pointCloudIndex = scene->getPointCloudIndex();

                    CT_PointIterator itP(pointCloudIndex);
                    while(itP.hasNext())
                    {
                        const CT_Point &pt = itP.next().currentPoint();
                        outGrid->setValueAtXYZ(pt(0), pt(1), pt(2), cpt);
                    }
                    ++cpt;
                }

            } else if (_mode == 2)
            {
#ifdef USE_OPENCV
                int cpt = 1;
                CT_ResultItemIterator it(inResult, this, DEF_SearchInItem);
                // iterate over all groups
                while(it.hasNext())
                {
                    const CT_AbstractSingularItemDrawable *item = it.next();

                    double z = dtm->valueAtCoords(item->getCenterX(), item->getCenterY());
                    if (z == dtm->NA())
                    {
                        z = 0;
                    }
                    z += _offset;

                    outGrid->setValueAtXYZ(item->getCenterX(), item->getCenterY(), z, cpt);
                    ++cpt;
                }
#endif
            }

            outGrid->computeMinMax();
            group->addItemDrawable(outGrid);
        }
    }

    setProgress(99);
}


