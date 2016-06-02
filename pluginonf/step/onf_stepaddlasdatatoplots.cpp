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

#include "onf_stepaddlasdatatoplots.h"

#include "ct_result/ct_resultgroup.h"
#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/ct_outresultmodelgroupcopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"
#include "ct_itemdrawable/tools/iterator/ct_groupiterator.h"
#include "ct_view/ct_stepconfigurabledialog.h"

#include "ct_itemdrawable/ct_scene.h"
#include "ctliblas/itemdrawable/las/ct_stdlaspointsattributescontainershortcut.h"
#include "ct_pointcloudindex/ct_pointcloudindexvector.h"
#include "ct_iterator/ct_pointiterator.h"

#include <QDebug>

// Alias for indexing models
#define DEFin_result "inres"
#define DEFin_grpLASAll "grpLASAll"
#define DEFin_LASAll "LASAll"
#define DEFin_grpPlot "grpplot"
#define DEFin_plotPoints "plotpoints"

// Constructor : initialization of parameters
ONF_StepAddLASDataToPlots::ONF_StepAddLASDataToPlots(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
}

// Step description (tooltip of contextual menu)
QString ONF_StepAddLASDataToPlots::getStepDescription() const
{
    return tr("Ajoute les données LAS aux placettes");
}

// Step detailled description
QString ONF_StepAddLASDataToPlots::getStepDetailledDescription() const
{
    return tr("Cette étape peut être ajoutée après une étape générant des placettes à partir d'une scène.<br>"
              "Elle permet de récupérer pour chaque placette les données LAS adéquates, à partir des données LAS de la scène mère.<br>"
              "Ces données sont passée sous forme de référence, la mémoire occupée n'augmente donc pas.");
}

// Step URL
QString ONF_StepAddLASDataToPlots::getStepURL() const
{
    //return tr("STEP URL HERE");
    return CT_AbstractStep::getStepURL(); //by default URL of the plugin
}

// Step copy method
CT_VirtualAbstractStep* ONF_StepAddLASDataToPlots::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new ONF_StepAddLASDataToPlots(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void ONF_StepAddLASDataToPlots::createInResultModelListProtected()
{
    CT_InResultModelGroupToCopy *resIn = createNewInResultModelForCopy(DEFin_result, tr("Placettes"), tr(""), true);
    resIn->setZeroOrMoreRootGroup();
    resIn->addGroupModel("", DEFin_grpLASAll, CT_AbstractItemGroup::staticGetType(), tr("Groupe Scene complète"));
    resIn->addItemModel(DEFin_grpLASAll, DEFin_LASAll, CT_StdLASPointsAttributesContainer::staticGetType(), tr("Données LAS complètes"));
    resIn->addGroupModel(DEFin_grpLASAll, DEFin_grpPlot, CT_AbstractItemGroup::staticGetType(), tr("Groupe Placette"));
    resIn->addItemModel(DEFin_grpPlot, DEFin_plotPoints, CT_AbstractItemDrawableWithPointCloud::staticGetType(), tr("Points de la placette"));
}

// Creation and affiliation of OUT models
void ONF_StepAddLASDataToPlots::createOutResultModelListProtected()
{
    CT_OutResultModelGroupToCopyPossibilities *res = createNewOutResultModelToCopy(DEFin_result);
    if (res != NULL)
    {
        res->addItemModel(DEFin_grpPlot, _outLASData_ModelName, new CT_StdLASPointsAttributesContainerShortcut(), tr("Données LAS placette"));
    }
}

// Semi-automatic creation of step parameters DialogBox
void ONF_StepAddLASDataToPlots::createPostConfigurationDialog()
{
   //CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();
}

void ONF_StepAddLASDataToPlots::compute()
{
    QList<CT_ResultGroup*> outResultList = getOutResultList();
    CT_ResultGroup* resOut = outResultList.at(0);

    CT_ResultGroupIterator itOut(resOut, this, DEFin_grpLASAll);
    while (itOut.hasNext() && !isStopped())
    {
        CT_StandardItemGroup* groupLASAll = (CT_StandardItemGroup*) itOut.next();
        CT_StdLASPointsAttributesContainer* inLASAll = (CT_StdLASPointsAttributesContainer*) groupLASAll->firstItemByINModelName(this, DEFin_LASAll);

        if (inLASAll != NULL)
        {
            CT_GroupIterator itGrp(groupLASAll, this, DEFin_grpPlot);
            while (itGrp.hasNext())
            {
                CT_StandardItemGroup* groupPlot = (CT_StandardItemGroup*) itGrp.next();
                CT_AbstractItemDrawableWithPointCloud* plotPoints = (CT_AbstractItemDrawableWithPointCloud*) groupPlot->firstItemByINModelName(this, DEFin_plotPoints);

                if (plotPoints != NULL)
                {
                    CT_StdLASPointsAttributesContainerShortcut* containerShortcut = new CT_StdLASPointsAttributesContainerShortcut(_outLASData_ModelName.completeName(),
                                                                                                                                   resOut,
                                                                                                                                   inLASAll);
                    groupPlot->addItemDrawable(containerShortcut);
                }
            }
        }
    }    
}
