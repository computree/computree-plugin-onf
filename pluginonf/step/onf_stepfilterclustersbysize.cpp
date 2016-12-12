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


#include "onf_stepfilterclustersbysize.h"

#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/ct_outresultmodelgroupcopy.h"

#include "ct_result/ct_resultgroup.h"
#include "ct_itemdrawable/abstract/ct_abstractitemdrawablewithpointcloud.h"

#include "ct_view/ct_stepconfigurabledialog.h"

#include "ct_pointcloudindex/abstract/ct_abstractpointcloudindex.h"

#define DEF_SearchInGroup       "g"
#define DEF_SearchInPointCloud  "p"
#define DEF_SearchInResult      "r"
#define DEF_SearchOutResult     "r"


ONF_StepFilterClustersBySize::ONF_StepFilterClustersBySize(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _minSize = 4;
}

QString ONF_StepFilterClustersBySize::getStepDescription() const
{
    return tr("Filtrer les Clusters par nombre de points");
}

QString ONF_StepFilterClustersBySize::getStepDetailledDescription() const
{
    return tr("Cette étape filtre des clusters (tout item contenant des points).<br>"
              "Tout cluster ayant un nombre de points strictement inférieur au <b>nombre de points minimum</b> spécifié est éliminé.");
}

CT_VirtualAbstractStep* ONF_StepFilterClustersBySize::createNewInstance(CT_StepInitializeData &dataInit)
{
    // crée une copie de cette étape
    return new ONF_StepFilterClustersBySize(dataInit);
}

//////////////////// PROTECTED //////////////////

void ONF_StepFilterClustersBySize::createInResultModelListProtected()
{
    CT_InResultModelGroupToCopy * resultModel = createNewInResultModelForCopy(DEF_SearchInResult, tr("Clusters"));
    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_SearchInGroup);
    resultModel->addItemModel(DEF_SearchInGroup, DEF_SearchInPointCloud, CT_AbstractItemDrawableWithPointCloud::staticGetType(), tr("Points"));
}

void ONF_StepFilterClustersBySize::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addInt(tr("Nombre de points minimum dans un cluster"), "pts",1 , 1000000, _minSize);
}

void ONF_StepFilterClustersBySize::createOutResultModelListProtected()
{
    createNewOutResultModelToCopy(DEF_SearchInResult);
}

void ONF_StepFilterClustersBySize::compute()
{
    // on récupère le résultat copié
    CT_ResultGroup *outRes = getOutResultList().first();

    //CT_InAbstractItemDrawableModel *inPointClusterModel = (CT_InAbstractItemDrawableModel*)getInModelForResearchIfUseCopy(DEF_SearchInResult, DEF_SearchInPointCloud);

    size_t totalNumberOfClusters = 0;

    QList<CT_AbstractItemGroup*> groupsToBeRemoved;

    CT_ResultGroupIterator it(outRes, this, DEF_SearchInGroup);
    while (it.hasNext() && (!isStopped()))
    {
        CT_AbstractItemGroup *group = (CT_AbstractItemGroup*) it.next();
        const CT_AbstractItemDrawableWithPointCloud *item = (const CT_AbstractItemDrawableWithPointCloud*)group->firstItemByINModelName(this, DEF_SearchInPointCloud);
        //const CT_AbstractItemDrawableWithPointCloud *item = (const CT_AbstractItemDrawableWithPointCloud*)group->findFirstItem(inPointClusterModel);

        if(item != NULL)
        {
            ++totalNumberOfClusters;

            if (item->getPointCloudIndex()->size() < _minSize)
                groupsToBeRemoved.append(group);
        }
    }

    size_t numberOfRemovedClusters = groupsToBeRemoved.size();
    // Suppression effective des groupes
    while (!groupsToBeRemoved.isEmpty())
    {
        CT_AbstractItemGroup *group = groupsToBeRemoved.takeLast();
        recursiveRemoveGroupIfEmpty(group->parentGroup(), group);
    }

    PS_LOG->addMessage(LogInterface::info, LogInterface::step, QString(tr("Nombre de clusters avant filtrage : %1")).arg(totalNumberOfClusters));
    PS_LOG->addMessage(LogInterface::info, LogInterface::step, QString(tr("Nombre de clusters éliminés : %1")).arg(numberOfRemovedClusters));

    setProgress( 100 );
}

void ONF_StepFilterClustersBySize::recursiveRemoveGroupIfEmpty(CT_AbstractItemGroup *parent, CT_AbstractItemGroup *group) const
{
    if(parent != NULL)
    {
        parent->removeGroup(group);

        if(parent->isEmpty())
            recursiveRemoveGroupIfEmpty(parent->parentGroup(), parent);
    }
    else
    {
        ((CT_ResultGroup*)group->result())->removeGroupSomethingInStructure(group);
    }
}
