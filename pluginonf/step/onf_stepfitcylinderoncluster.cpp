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

#include "onf_stepfitcylinderoncluster.h"

#include "ct_itemdrawable/abstract/ct_abstractitemdrawablewithpointcloud.h"
#include "ct_itemdrawable/ct_beam.h"
#include "ct_itemdrawable/ct_cylinder.h"
#include "ct_itemdrawable/tools/iterator/ct_groupiterator.h"
#include "ct_result/ct_resultgroup.h"
#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"

// Alias for indexing models
#define DEFin_rpoints "rpoints"
#define DEFin_grp "grp"
#define DEFin_points "points"



// Constructor : initialization of parameters
ONF_StepFitCylinderOnCluster::ONF_StepFitCylinderOnCluster(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
}

// Step description (tooltip of contextual menu)
QString ONF_StepFitCylinderOnCluster::getStepDescription() const
{
    return tr("Ajuster un Cylindre par cluster (en 3D)");
}

// Step detailled description
QString ONF_StepFitCylinderOnCluster::getStepDetailledDescription() const
{
    return tr("No detailled description for this step");
}

// Step URL
QString ONF_StepFitCylinderOnCluster::getStepURL() const
{
    //return tr("STEP URL HERE");
    return CT_AbstractStep::getStepURL(); //by default URL of the plugin
}

// Step copy method
CT_VirtualAbstractStep* ONF_StepFitCylinderOnCluster::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new ONF_StepFitCylinderOnCluster(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void ONF_StepFitCylinderOnCluster::createInResultModelListProtected()
{
    CT_InResultModelGroupToCopy *resIn_rpoints = createNewInResultModelForCopy(DEFin_rpoints, tr("Points"));
    resIn_rpoints->setZeroOrMoreRootGroup();
    resIn_rpoints->addGroupModel("", DEFin_grp, CT_AbstractItemGroup::staticGetType(), tr("Groupe"));
    resIn_rpoints->addItemModel(DEFin_grp, DEFin_points, CT_AbstractItemDrawableWithPointCloud::staticGetType(), tr("Points"));

}

// Creation and affiliation of OUT models
void ONF_StepFitCylinderOnCluster::createOutResultModelListProtected()
{
    CT_OutResultModelGroupToCopyPossibilities *resCpy_rpoints = createNewOutResultModelToCopy(DEFin_rpoints);

    if(resCpy_rpoints != NULL)
        resCpy_rpoints->addItemModel(DEFin_grp, _cyl_ModelName, new CT_Cylinder(), tr("Cylindre"));
}

// Semi-automatic creation of step parameters DialogBox
void ONF_StepFitCylinderOnCluster::createPostConfigurationDialog()
{
    // No parameter dialog for this step
}

void ONF_StepFitCylinderOnCluster::compute()
{
    QList<CT_ResultGroup*> outResultList = getOutResultList();
    CT_ResultGroup* resCpy_rpoints = outResultList.at(0);

    CT_ResultGroupIterator itCpy_grp(resCpy_rpoints, this, DEFin_grp);
    while (itCpy_grp.hasNext() && !isStopped())
    {
        CT_StandardItemGroup* grpCpy_grp = (CT_StandardItemGroup*) itCpy_grp.next();
        
        const CT_AbstractItemDrawableWithPointCloud* itemCpy_points = (CT_AbstractItemDrawableWithPointCloud*)grpCpy_grp->firstItemByINModelName(this, DEFin_points);
        if (itemCpy_points != NULL)
        {           
            CT_Cylinder* itemCpy_cyl = CT_Cylinder::staticCreate3DCylinderFromPointCloud(_cyl_ModelName.completeName(), itemCpy_points->id(), resCpy_rpoints, *(itemCpy_points->getPointCloudIndex()), itemCpy_points->getCenterCoordinate());

            if(itemCpy_cyl != NULL)
                grpCpy_grp->addItemDrawable(itemCpy_cyl);
        }

    }    

}
