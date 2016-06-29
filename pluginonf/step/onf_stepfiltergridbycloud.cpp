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


#include "onf_stepfiltergridbycloud.h"

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
#include "ct_itemdrawable/ct_grid3d.h"
#include "tools/onf_computehitsthread.h"

#include "ct_view/ct_stepconfigurabledialog.h"

#include <QFileInfo>
#include <QDebug>
#include <limits>

#define DEF_SearchInResultSc "rsc"
#define DEF_SearchInGroupSc   "grsc"
#define DEF_SearchInScene   "sc"

#define DEF_SearchInResult "r"
#define DEF_SearchInGroup   "gr"
#define DEF_SearchInGrid   "grid"

ONF_StepFilterGridByCloud::ONF_StepFilterGridByCloud(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _naVal = -9999;
}

QString ONF_StepFilterGridByCloud::getStepDescription() const
{
    // Gives the descrption to print in the GUI
    return tr("Filter une grille 3D en fonction de scènes");
}

// Step description (tooltip of contextual menu)
QString ONF_StepFilterGridByCloud::getStepDetailledDescription() const
{
    return tr("");
}

CT_VirtualAbstractStep* ONF_StepFilterGridByCloud::createNewInstance(CT_StepInitializeData &dataInit)
{
    // Creates an instance of this step
    return new ONF_StepFilterGridByCloud(dataInit);
}

void ONF_StepFilterGridByCloud::createInResultModelListProtected()
{
    CT_InResultModelGroup* resultModelScene = createNewInResultModel(DEF_SearchInResultSc, tr("Scène(s)"), "", true);
    resultModelScene->setZeroOrMoreRootGroup();
    resultModelScene->addGroupModel("", DEF_SearchInGroupSc);
    resultModelScene->addItemModel(DEF_SearchInGroupSc, DEF_SearchInScene, CT_Scene::staticGetType(), tr("Scène"));



    CT_InResultModelGroupToCopy *resultModel = createNewInResultModelForCopy(DEF_SearchInResult, tr("Grille"), "", true);

    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_SearchInGroup);
    resultModel->addItemModel(DEF_SearchInGroup, DEF_SearchInGrid, CT_AbstractGrid3D::staticGetType(), tr("Grille"), "", CT_InAbstractModel::C_ChooseMultipleIfMultiple);
}

void ONF_StepFilterGridByCloud::createOutResultModelListProtected()
{    
    CT_OutResultModelGroupToCopyPossibilities *res = createNewOutResultModelToCopy(DEF_SearchInResult);

    if(res != NULL)
    {
        res->addItemModel(DEF_SearchInGroup, _outGrid_ModelName, new CT_Grid3D<double>(), tr("Grille filtrée"));
    }
}

void ONF_StepFilterGridByCloud::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addDouble(tr("Valeur pour les cases hors scène"),"", -1e+10, 1e+10, 4, _naVal);
}

void ONF_StepFilterGridByCloud::compute()
{
    // Gets the out result
    CT_ResultGroup* inResult = getInputResults().first();
    CT_ResultGroup* outResult = getOutResultList().first();

    double minxScene = std::numeric_limits<double>::max();
    double minyScene = std::numeric_limits<double>::max();
    double minzScene = std::numeric_limits<double>::max();
    double maxxScene = -std::numeric_limits<double>::max();
    double maxyScene = -std::numeric_limits<double>::max();
    double maxzScene = -std::numeric_limits<double>::max();


    CT_ResultItemIterator it(inResult, this, DEF_SearchInScene);
    // iterate over all groups
    while(it.hasNext())
    {
        CT_Scene *scene = (CT_Scene*)it.next();

        Eigen::Vector3d min, max;
        scene->getBoundingBox(min, max);

        if (min(0) < minxScene) {minxScene = min(0);}
        if (min(1) < minyScene) {minyScene = min(1);}
        if (min(2) < minzScene) {minzScene = min(2);}

        if (max(0) > maxxScene) {maxxScene = max(0);}
        if (max(1) > maxyScene) {maxyScene = max(1);}
        if (max(2) > maxzScene) {maxzScene = max(2);}
    }


    CT_ResultGroupIterator itOut(outResult, this, DEF_SearchInGroup);
    // iterate over all groups
    while(itOut.hasNext())
    {
        CT_AbstractItemGroup *group = (CT_AbstractItemGroup*)itOut.next();
        const CT_AbstractGrid3D* gridIn = (CT_AbstractGrid3D*)group->firstItemByINModelName(this, DEF_SearchInGrid);

        if (gridIn!=NULL)
        {
            Eigen::Vector3d minGr, maxGr;
            double res = gridIn->resolution();
            gridIn->getMinCoordinates(minGr);
            gridIn->getMaxCoordinates(maxGr);

            while (minGr(0) < minxScene) {minGr(0) += res;}
            while (minGr(1) < minyScene) {minGr(1) += res;}
            while (minGr(2) < minzScene) {minGr(2) += res;}

            while (maxGr(0) > maxxScene && maxGr(0) > minGr(0)) {maxGr(0) -= res;}
            while (maxGr(1) > maxyScene && maxGr(1) > minGr(1)) {maxGr(1) -= res;}
            while (maxGr(2) > maxzScene && maxGr(2) > minGr(2)) {maxGr(2) -= res;}

            minGr(0) -= res;
            minGr(1) -= res;
            minGr(2) -= res;

            maxGr(0) += res;
            maxGr(1) += res;
            maxGr(2) += res;

            // Declaring the output grids
            CT_Grid3D<double>* outGrid = CT_Grid3D<double>::createGrid3DFromXYZCoords(_outGrid_ModelName.completeName(), outResult, minGr(0), minGr(1), minGr(2), maxGr(0), maxGr(1), maxGr(2), res, _naVal, _naVal);


            CT_ResultItemIterator itSc(inResult, this, DEF_SearchInScene);
            // iterate over all groups
            while(itSc.hasNext())
            {
                CT_Scene *scene = (CT_Scene*)itSc.next();

                const CT_AbstractPointCloudIndex *pointCloudIndex = scene->getPointCloudIndex();
                CT_PointIterator itP(pointCloudIndex);
                while(itP.hasNext() && (!isStopped()))
                {
                    const CT_Point &point = itP.next().currentPoint();

                    size_t index;
                    gridIn->indexAtXYZ(point(0), point(1), point(2), index);
                    outGrid->setValueAtXYZ(point(0), point(1), point(2), gridIn->valueAtIndexAsDouble(index));
                }
            }
            outGrid->computeMinMax();
            group->addItemDrawable(outGrid);
        }
    }

    setProgress(99);
}


