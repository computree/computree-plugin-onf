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

#include "onf_stepcreateplotmanagergrid.h"

#include "ct_itemdrawable/ct_datasourcegeo.h"
#include "ct_itemdrawable/ct_plotmanageritem.h"
#include "ct_itemdrawable/tools/iterator/ct_groupiterator.h"
#include "ct_result/ct_resultgroup.h"
#include "ct_result/model/inModel/ct_inresultmodelgroup.h"
#include "ct_result/model/outModel/ct_outresultmodelgroup.h"

#include "ct_plotmanager/ct_plotmanager_grid.h"

// Alias for indexing models
#define DEFin_resultDataSource "resultDataSource"
#define DEFin_grpDataSource "grpDataSource"
#define DEFin_dataSource "dataSource"

#define DEFout_result "result"
#define DEFout_grp "grp"
#define DEFout_plotmanager "plotmanager"


// Constructor : initialization of parameters
ONF_StepCreatePlotManagerGrid::ONF_StepCreatePlotManagerGrid(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
}

// Step description (tooltip of contextual menu)
QString ONF_StepCreatePlotManagerGrid::getStepDescription() const
{
    return tr("Créée un gestionnaire de placettes (grille) à partir d'une DataSource");
}

// Step detailled description
QString ONF_StepCreatePlotManagerGrid::getStepDetailledDescription() const
{
    return tr("No detailled description for this step");
}

// Step URL
QString ONF_StepCreatePlotManagerGrid::getStepURL() const
{
    //return tr("STEP URL HERE");
    return CT_AbstractStep::getStepURL(); //by default URL of the plugin
}

// Step copy method
CT_VirtualAbstractStep* ONF_StepCreatePlotManagerGrid::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new ONF_StepCreatePlotManagerGrid(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void ONF_StepCreatePlotManagerGrid::createInResultModelListProtected()
{
    CT_InResultModelGroup *resIn_resultDataSource = createNewInResultModel(DEFin_resultDataSource, tr("Source de données géographique"), tr(""), true);
    resIn_resultDataSource->setZeroOrMoreRootGroup();
    resIn_resultDataSource->addGroupModel("", DEFin_grpDataSource, CT_AbstractItemGroup::staticGetType(), tr("Groupe"));
    resIn_resultDataSource->addItemModel(DEFin_grpDataSource, DEFin_dataSource, CT_DataSourceGeo::staticGetType(), tr("Source de données géographique"));

}

// Creation and affiliation of OUT models
void ONF_StepCreatePlotManagerGrid::createOutResultModelListProtected()
{
    CT_OutResultModelGroup *res_result = createNewOutResultModel(DEFout_result, tr("Gestionnaire de placettes (grid)"));
    res_result->setRootGroup(DEFout_grp, new CT_StandardItemGroup(), tr("Groupe"));
    res_result->addItemModel(DEFout_grp, DEFout_plotmanager, new CT_PlotManagerItem(), tr("Gestionnaire de placettes (grid)"));

}

// Semi-automatic creation of step parameters DialogBox
void ONF_StepCreatePlotManagerGrid::createPostConfigurationDialog()
{
    // No parameter dialog for this step
}

void ONF_StepCreatePlotManagerGrid::compute()
{
    QList<CT_ResultGroup*> inResultList = getInputResults();
    CT_ResultGroup* resIn_resultDataSource = inResultList.at(0);

    QList<CT_ResultGroup*> outResultList = getOutResultList();
    CT_ResultGroup* res_result = outResultList.at(0);

    // IN results browsing
    CT_ResultItemIterator itIn_DataSource(resIn_resultDataSource, this, DEFin_dataSource);
    while (itIn_DataSource.hasNext() && !isStopped())
    {
        const CT_DataSourceGeo* dataSource = (CT_DataSourceGeo*) itIn_DataSource.next();
        
        if (dataSource != NULL)
        {


            // OUT results creation (move it to the appropried place in the code)
            CT_StandardItemGroup* grp_grp= new CT_StandardItemGroup(DEFout_grp, res_result);
            res_result->addGroup(grp_grp);

            CT_PlotManagerItem* item_plotmanager = new CT_PlotManagerItem(DEFout_plotmanager, res_result, new CT_PlotManager_grid(0, 0, 0, 0, CT_PlotManager_grid::T_Circle));
            grp_grp->addItemDrawable(item_plotmanager);
        }
    }
    
}
