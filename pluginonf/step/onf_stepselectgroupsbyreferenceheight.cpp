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

#include "onf_stepselectgroupsbyreferenceheight.h"

// Inclusion of in models
#include "ct_itemdrawable/model/inModel/ct_inzeroormoregroupmodel.h"
#include "ct_itemdrawable/model/inModel/ct_instdgroupmodel.h"
#include "ct_itemdrawable/model/inModel/ct_instdsingularitemmodel.h"
#include "ct_itemdrawable/ct_referencepoint.h"
#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/tools/iterator/ct_resultgroupiterator.h"
#include "ct_itemdrawable/tools/iterator/ct_groupiterator.h"

// Inclusion of out models
#include "ct_itemdrawable/model/outModel/ct_outstdgroupmodel.h"

// Inclusion of standard result class
#include "ct_result/ct_resultgroup.h"

// Inclusion of used ItemDrawable classes
#include "ct_itemdrawable/abstract/ct_abstractsingularitemdrawable.h"

#include "ct_view/ct_stepconfigurabledialog.h"

#include "ct_model/tools/ct_modelsearchhelper.h"

#include <QMessageBox>
#include <limits>

// Alias for indexing models
#define DEFin_scres "scres"
#define DEFin_scBase "scBase"
#define DEFin_group "group"
#define DEFin_item "item"
#define DEFin_dtmValue "dtmValue"

// Constructor : initialization of parameters
ONF_StepSelectGroupsByReferenceHeight::ONF_StepSelectGroupsByReferenceHeight(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _hRef = 1.3;
}

// Step description (tooltip of contextual menu)
QString ONF_StepSelectGroupsByReferenceHeight::getStepDescription() const
{
    return tr("Séléction de groupes // hauteur de référence");
}

// Step copy method
CT_VirtualAbstractStep* ONF_StepSelectGroupsByReferenceHeight::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new ONF_StepSelectGroupsByReferenceHeight(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void ONF_StepSelectGroupsByReferenceHeight::createInResultModelListProtected()
{
    CT_InResultModelGroupToCopy *resIn_scres = createNewInResultModelForCopy(DEFin_scres, tr("Items"));
    resIn_scres->setZeroOrMoreRootGroup();
    resIn_scres->addGroupModel("", DEFin_scBase, CT_AbstractItemGroup::staticGetType(), tr("Groupe de base"));
    resIn_scres->addItemModel(DEFin_scBase, DEFin_dtmValue, CT_ReferencePoint::staticGetType(), tr("Z MNT"));
    resIn_scres->addGroupModel(DEFin_scBase, DEFin_group, CT_AbstractItemGroup::staticGetType(), tr("Groupe"));
    resIn_scres->addItemModel(DEFin_group, DEFin_item, CT_AbstractSingularItemDrawable::staticGetType(), tr("Item"));
}

// Creation and affiliation of OUT models
void ONF_StepSelectGroupsByReferenceHeight::createOutResultModelListProtected()
{
    createNewOutResultModelToCopy(DEFin_scres);
}

// Semi-automatic creation of step parameters DialogBox
void ONF_StepSelectGroupsByReferenceHeight::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();
    configDialog->addDouble(tr("Hauteur de référence"), "m", 0, 9999, 2, _hRef);

}

void ONF_StepSelectGroupsByReferenceHeight::compute()
{
    CT_ResultGroup *resultOut_R = getOutResultList().first();

    QList<CT_AbstractItemGroup*> groupsToRemove;

    CT_ResultGroupIterator itG(resultOut_R, this, DEFin_scBase);
    while(itG.hasNext() && !isStopped())
    {
        const CT_AbstractItemGroup *groupOut_G = itG.next();

        if (groupOut_G != NULL)
        {

            CT_ReferencePoint *dtmRefPoint = (CT_ReferencePoint*) groupOut_G->firstItemByINModelName(this, DEFin_dtmValue);

            if (dtmRefPoint != NULL)
            {
                float href = dtmRefPoint->getCenterZ() + _hRef;

                float zmin = std::numeric_limits<float>::max();
                float zmax = -std::numeric_limits<float>::max();

                CT_GroupIterator itG2(groupOut_G, this, DEFin_group);
                while(itG2.hasNext() && !isStopped())
                {

                    const CT_AbstractItemGroup *groupOut_G2 = itG2.next();

                    if (groupOut_G2 != NULL)
                    {
                        CT_AbstractSingularItemDrawable *itemOut_I = groupOut_G2->firstItemByINModelName(this, DEFin_item);

                        if (itemOut_I != NULL)
                        {
                            if (itemOut_I->getCenterZ() < zmin) {zmin = itemOut_I->getCenterZ();}
                            if (itemOut_I->getCenterZ() > zmax) {zmax = itemOut_I->getCenterZ();}
                        }
                    }
                }

                if (zmin > href || zmax < href)
                {
                    groupsToRemove.append((CT_AbstractItemGroup*)groupOut_G);
                }
            }

        }

    }


    // we remove the parent group of all ItemDrawable that must be deleted from the out result
    // and all groups that don't contains a ItemDrawable researched
    QListIterator<CT_AbstractItemGroup*> itE(groupsToRemove);

    while(itE.hasNext())
    {
        CT_AbstractItemGroup *group = itE.next();
        recursiveRemoveGroup(group->parentGroup(), group);
    }

}

void ONF_StepSelectGroupsByReferenceHeight::recursiveRemoveGroup(CT_AbstractItemGroup *parent, CT_AbstractItemGroup *group) const
{
    if(parent != NULL)
    {
        parent->removeGroup(group);

        if(parent->isEmptyOfGroups())
            recursiveRemoveGroup(parent->parentGroup(), parent);
    }
    else
    {
        ((CT_ResultGroup*)group->result())->removeGroupSomethingInStructure(group);
    }
}

