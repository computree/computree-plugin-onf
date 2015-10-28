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


#include "onf_stepfiltergroupsbygroupsnumber.h"

#include "ct_itemdrawable/tools/ct_standardcontext.h"

#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"

#include "ct_result/ct_resultgroup.h"
#include "ct_itemdrawable/abstract/ct_abstractitemdrawablewithpointcloud.h"
#include "ct_view/ct_stepconfigurabledialog.h"
#include "ct_itemdrawable/tools/iterator/ct_groupiterator.h"

#define DEF_SearchInGroupToFilter  "gf"
#define DEF_SearchInGroupToCount  "gc"
#define DEF_SearchInResult      "r"
#define DEF_SearchOutResult     "r"


ONF_StepFilterGroupsByGroupsNumber::ONF_StepFilterGroupsByGroupsNumber(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _minSize = 4;
}

QString ONF_StepFilterGroupsByGroupsNumber::getStepDescription() const
{
    return tr("Filtrage de groupes niv.1 / nb. de groupes niv.2");
}

QString ONF_StepFilterGroupsByGroupsNumber::getStepDetailledDescription() const
{
    return tr("Cette étape très générique travaille sur deux niveau de groupes.<br>"
              "Tout groupe du niveau 1 contenant <b>nombre de groupes</b> de niveau 2 insuffisant est éliminé.<br>"
              "Un usage de cette étape est d'éliminer des groupes de niveau 1 ne contenant pas assez de groupes de niveau 2.<br>"
              "Comme par exemple après une étape ONF_StepDetectSection.");
}

CT_VirtualAbstractStep* ONF_StepFilterGroupsByGroupsNumber::createNewInstance(CT_StepInitializeData &dataInit)
{
    // crée une copie de cette étape
    return new ONF_StepFilterGroupsByGroupsNumber(dataInit);
}

//////////////////// PROTECTED //////////////////

void ONF_StepFilterGroupsByGroupsNumber::createInResultModelListProtected()
{

    CT_InResultModelGroupToCopy * resultModel = createNewInResultModelForCopy(DEF_SearchInResult, tr("Groupes niv.1 (à filter)"));
    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_SearchInGroupToFilter, CT_AbstractItemGroup::staticGetType(), tr("Groupe niv.1 (à filter)"));
    resultModel->addGroupModel(DEF_SearchInGroupToFilter, DEF_SearchInGroupToCount, CT_AbstractItemGroup::staticGetType(), tr("Groupe niv.2 (à dénombrer)"));
}

void ONF_StepFilterGroupsByGroupsNumber::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addInt(tr("Nombre de groupes minimum de niveau 2 dans un groupe de niveau 1"), "groupes", 0, 1000, _minSize);
}

void ONF_StepFilterGroupsByGroupsNumber::createOutResultModelListProtected()
{
    createNewOutResultModelToCopy(DEF_SearchInResult);
}

void ONF_StepFilterGroupsByGroupsNumber::compute()
{
    // on récupère le résultat copié
    CT_ResultGroup *outRes = getOutResultList().first();

    CT_ResultGroupIterator itGroupToFilter(outRes, this, DEF_SearchInGroupToFilter);
    while (itGroupToFilter.hasNext() && (!isStopped()))
    {
        CT_AbstractItemGroup *group = (CT_AbstractItemGroup*) itGroupToFilter.next();

        int cpt = 0;
        CT_GroupIterator itGroupToCount(group, this, DEF_SearchInGroupToCount);
        while (itGroupToCount.hasNext() && (!isStopped()))
        {
            itGroupToCount.next();
            cpt++;
        }

        if (cpt < _minSize) {recursiveRemoveGroupIfEmpty(group->parentGroup(), group);}
    }

    setProgress( 100 );
}

void ONF_StepFilterGroupsByGroupsNumber::recursiveRemoveGroupIfEmpty(CT_AbstractItemGroup *parent, CT_AbstractItemGroup *group) const
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
