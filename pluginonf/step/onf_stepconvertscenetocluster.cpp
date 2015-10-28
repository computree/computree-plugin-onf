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

#include "onf_stepconvertscenetocluster.h"

#include "ct_itemdrawable/ct_pointcluster.h"
#include "ct_itemdrawable/ct_scene.h"
#include "ct_pointcloudindex/ct_pointcloudindexvector.h"
#include "ct_itemdrawable/tools/iterator/ct_groupiterator.h"
#include "ct_result/ct_resultgroup.h"
#include "ct_result/model/inModel/ct_inresultmodelgroup.h"
#include "ct_result/model/outModel/ct_outresultmodelgroup.h"

#include "ct_iterator/ct_pointiterator.h"

// Alias for indexing models
#define DEFin_res "res"
#define DEFin_g "g"
#define DEFin_scene "scene"

#define DEFout_res "res"
#define DEFout_gr "gr"
#define DEFout_cluster "cluster"


// Constructor : initialization of parameters
ONF_StepConvertSceneToCluster::ONF_StepConvertSceneToCluster(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
}

// Step description (tooltip of contextual menu)
QString ONF_StepConvertSceneToCluster::getStepDescription() const
{
    return tr("Conversion d'une scène en cluster ordonné");
}

// Step detailled description
QString ONF_StepConvertSceneToCluster::getStepDetailledDescription() const
{
    return tr("No detailled description for this step");
}

// Step URL
QString ONF_StepConvertSceneToCluster::getStepURL() const
{
    //return tr("STEP URL HERE");
    return CT_AbstractStep::getStepURL(); //by default URL of the plugin
}

// Step copy method
CT_VirtualAbstractStep* ONF_StepConvertSceneToCluster::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new ONF_StepConvertSceneToCluster(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void ONF_StepConvertSceneToCluster::createInResultModelListProtected()
{
    CT_InResultModelGroup *resIn_res = createNewInResultModel(DEFin_res);
    resIn_res->setRootGroup(DEFin_g);
    resIn_res->addItemModel(DEFin_g, DEFin_scene, CT_Scene::staticGetType());

}

// Creation and affiliation of OUT models
void ONF_StepConvertSceneToCluster::createOutResultModelListProtected()
{
    CT_OutResultModelGroup *res_res = createNewOutResultModel(DEFout_res, tr(""));
    res_res->setRootGroup(DEFout_gr, new CT_StandardItemGroup());
    res_res->addItemModel(DEFout_gr, DEFout_cluster, new CT_PointCluster());

}

// Semi-automatic creation of step parameters DialogBox
void ONF_StepConvertSceneToCluster::createPostConfigurationDialog()
{
    // No parameter dialog for this step
}

void ONF_StepConvertSceneToCluster::compute()
{
    QList<CT_ResultGroup*> inResultList = getInputResults();
    CT_ResultGroup* resIn_res = inResultList.at(0);

    QList<CT_ResultGroup*> outResultList = getOutResultList();
    CT_ResultGroup* res_res = outResultList.at(0);

    // IN results browsing
    CT_ResultGroupIterator itIn_g(resIn_res, this, DEFin_g);
    while (itIn_g.hasNext() && !isStopped())
    {
        const CT_AbstractItemGroup* grpIn_g = (CT_AbstractItemGroup*) itIn_g.next();
        
        const CT_Scene* itemIn_scene = (CT_Scene*)grpIn_g->firstItemByINModelName(this, DEFin_scene);
        if (itemIn_scene != NULL)
        {
            const CT_AbstractPointCloudIndex *PCI_itemIn_scene = itemIn_scene->getPointCloudIndex();

            CT_StandardItemGroup* grp_gr= new CT_StandardItemGroup(DEFout_gr, res_res);
            res_res->addGroup(grp_gr);

            CT_PointCluster* item_cluster = new CT_PointCluster(DEFout_cluster, res_res);
            grp_gr->addItemDrawable(item_cluster);

            CT_PointIterator itP(PCI_itemIn_scene);
            while (itP.hasNext() && !isStopped())
            {
                size_t index = itP.next().currentGlobalIndex();
                item_cluster->addPoint(index);
            }
        }
    }    

}
