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

#include "onf_stepfilterelementsbyxyarea.h"

#include "ct_itemdrawable/abstract/ct_abstractsingularitemdrawable.h"
#include "ct_itemdrawable/abstract/ct_abstractareashape2d.h"
#include "ct_itemdrawable/tools/iterator/ct_groupiterator.h"
#include "ct_result/ct_resultgroup.h"
#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"
#include "ct_view/ct_stepconfigurabledialog.h"

// Alias for indexing models
#define DEFin_res "res"
#define DEFin_grpRoot "grpRoot"
#define DEFin_XYArea "xyarea"
#define DEFin_grp "grp"
#define DEFin_item "item"



// Constructor : initialization of parameters
ONF_StepFilterElementsByXYArea::ONF_StepFilterElementsByXYArea(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
}

// Step description (tooltip of contextual menu)
QString ONF_StepFilterElementsByXYArea::getStepDescription() const
{
    return tr("Garder les Items contenus dans une emprise");
}

// Step detailled description
QString ONF_StepFilterElementsByXYArea::getStepDetailledDescription() const
{
    return tr("No detailled description for this step");
}

// Step URL
QString ONF_StepFilterElementsByXYArea::getStepURL() const
{
    //return tr("STEP URL HERE");
    return CT_AbstractStep::getStepURL(); //by default URL of the plugin
}

// Step copy method
CT_VirtualAbstractStep* ONF_StepFilterElementsByXYArea::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new ONF_StepFilterElementsByXYArea(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void ONF_StepFilterElementsByXYArea::createInResultModelListProtected()
{
    CT_InResultModelGroupToCopy *resIn_res = createNewInResultModelForCopy(DEFin_res, tr("Items à filtrer"));
    resIn_res->setZeroOrMoreRootGroup();
    resIn_res->addGroupModel("", DEFin_grpRoot, CT_AbstractItemGroup::staticGetType(), tr("Groupe Emprise"));
    resIn_res->addItemModel(DEFin_grpRoot, DEFin_XYArea, CT_AbstractAreaShape2D::staticGetType(), tr("Emprise"));
    resIn_res->addGroupModel(DEFin_grpRoot, DEFin_grp, CT_AbstractItemGroup::staticGetType(), tr("Groupe à filtrer"));
    resIn_res->addItemModel(DEFin_grp, DEFin_item, CT_AbstractSingularItemDrawable::staticGetType(), tr("XY Item"));

}

// Creation and affiliation of OUT models
void ONF_StepFilterElementsByXYArea::createOutResultModelListProtected()
{
    createNewOutResultModelToCopy(DEFin_res);
}

// Semi-automatic creation of step parameters DialogBox
void ONF_StepFilterElementsByXYArea::createPostConfigurationDialog()
{
//    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();
}

void ONF_StepFilterElementsByXYArea::compute()
{

    QList<CT_ResultGroup*> outResultList = getOutResultList();
    CT_ResultGroup* resCpy = outResultList.at(0);

    QList<CT_AbstractItemGroup*> groupsToBeRemoved;

    CT_ResultGroupIterator it_RootGrp(resCpy, this, DEFin_grpRoot);
    while (it_RootGrp.hasNext() && !isStopped())
    {
        CT_StandardItemGroup* rootGrp = (CT_StandardItemGroup*) it_RootGrp.next();
        const CT_AbstractAreaShape2D* xyArea = (CT_AbstractAreaShape2D*)rootGrp->firstItemByINModelName(this, DEFin_XYArea);

        if (xyArea != NULL)
        {
            CT_GroupIterator itGrp(rootGrp, this, DEFin_grp);
            while (itGrp.hasNext())
            {
                CT_StandardItemGroup* grp = (CT_StandardItemGroup*) itGrp.next();
                const CT_AbstractSingularItemDrawable* item = (CT_AbstractSingularItemDrawable*)grp->firstItemByINModelName(this, DEFin_item);

                if (item != NULL && !xyArea->contains(item->getCenterX(), item->getCenterY()))
                {
                    groupsToBeRemoved.append(grp);
                }
            }
        }
    }

    while (!groupsToBeRemoved.isEmpty())
    {
        CT_AbstractItemGroup *group = groupsToBeRemoved.takeLast();
        recursiveRemoveGroupIfEmpty(group->parentGroup(), group);
    }    
}

void ONF_StepFilterElementsByXYArea::recursiveRemoveGroupIfEmpty(CT_AbstractItemGroup *parent, CT_AbstractItemGroup *group) const
{
    if(parent != NULL)
    {
        parent->removeGroup(group);
    } else
    {
        ((CT_ResultGroup*)group->result())->removeGroupSomethingInStructure(group);
    }
}

