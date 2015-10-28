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

#include "onf_stepcreateplotmanagerfromfile.h"

#include "ct_itemdrawable/ct_plotmanageritem.h"
#include "ct_result/ct_resultgroup.h"
#include "ct_result/model/outModel/ct_outresultmodelgroup.h"
#include "ct_view/ct_stepconfigurabledialog.h"

#include "ct_plotmanager/ct_plotmanager_plotslist.h"

// Alias for indexing models
#define DEFout_result "result"
#define DEFout_grp "grp"
#define DEFout_plotmanageritem "plotmanageritem"


// Constructor : initialization of parameters
ONF_StepCreatePlotManagerFromFile::ONF_StepCreatePlotManagerFromFile(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
}

// Step description (tooltip of contextual menu)
QString ONF_StepCreatePlotManagerFromFile::getStepDescription() const
{
    return tr("Créée un gestionnaire de placettes à partir d'un fichier ASCII");
}

// Step detailled description
QString ONF_StepCreatePlotManagerFromFile::getStepDetailledDescription() const
{
    return tr("No detailled description for this step");
}

// Step URL
QString ONF_StepCreatePlotManagerFromFile::getStepURL() const
{
    //return tr("STEP URL HERE");
    return CT_AbstractStep::getStepURL(); //by default URL of the plugin
}

// Step copy method
CT_VirtualAbstractStep* ONF_StepCreatePlotManagerFromFile::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new ONF_StepCreatePlotManagerFromFile(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void ONF_StepCreatePlotManagerFromFile::createInResultModelListProtected()
{
    // No in result is needed
    setNotNeedInputResult();
}

// Creation and affiliation of OUT models
void ONF_StepCreatePlotManagerFromFile::createOutResultModelListProtected()
{
    CT_OutResultModelGroup *res_result = createNewOutResultModel(DEFout_result, tr("Gestionnaire de placette (Plot List)"));
    res_result->setRootGroup(DEFout_grp, new CT_StandardItemGroup(), tr("Groupe"));
    res_result->addItemModel(DEFout_grp, DEFout_plotmanageritem, new CT_PlotManagerItem(), tr("Gestionnaire de placettes (Plot List)"));

}

// Semi-automatic creation of step parameters DialogBox
void ONF_StepCreatePlotManagerFromFile::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addFileChoice("Fichier de placettes", CT_FileChoiceButton::OneExistingFile, "Fichier ascii (*.txt, *.asc)", _fileName);
}

void ONF_StepCreatePlotManagerFromFile::compute()
{
    QList<CT_ResultGroup*> outResultList = getOutResultList();
    CT_ResultGroup* res_result = outResultList.at(0);

    CT_StandardItemGroup* grp_grp= new CT_StandardItemGroup(DEFout_grp, res_result);
    res_result->addGroup(grp_grp);
    
    CT_PlotManagerItem* item_plotmanageritem = new CT_PlotManagerItem(DEFout_plotmanageritem, res_result, new CT_PlotManager_plotsList());
    grp_grp->addItemDrawable(item_plotmanageritem);

}
