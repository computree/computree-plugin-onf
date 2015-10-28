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

#include "onf_stepmodifypositions2d.h"
// Inclusion of in models
#include "ct_itemdrawable/model/inModel/ct_inzeroormoregroupmodel.h"
#include "ct_itemdrawable/model/inModel/ct_instdgroupmodel.h"
#include "ct_itemdrawable/model/inModel/ct_instdsingularitemmodel.h"
#include "ct_result/model/inModel/ct_inresultmodelgroup.h"

// Inclusion of out models
#include "ct_itemdrawable/model/outModel/ct_outstdgroupmodel.h"
#include "ct_result/model/outModel/ct_outresultmodelgroup.h"


// Inclusion of used ItemDrawable classes
#include "ct_itemdrawable/abstract/ct_abstractsingularitemdrawable.h"

//Inclusion of actions
#include "actions/onf_actionmodifypositions2d.h"

#include "ct_model/tools/ct_modelsearchhelper.h"

#include <QMessageBox>

// Alias for indexing models
#define DEFin_res "res"
#define DEFin_grp "grp"
#define DEFin_pos "pos"

#define DEFout_res "res"
#define DEFout_grp "grp"
#define DEFout_pos "pos"


// Constructor : initialization of parameters
ONF_StepModifyPositions2D::ONF_StepModifyPositions2D(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    m_doc = NULL;
    setManual(true);
}

// Step description (tooltip of contextual menu)
QString ONF_StepModifyPositions2D::getStepDescription() const
{
    return tr("Modifier des positions 2D");
}

// Step copy method
CT_VirtualAbstractStep* ONF_StepModifyPositions2D::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new ONF_StepModifyPositions2D(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void ONF_StepModifyPositions2D::createInResultModelListProtected()
{
    CT_InResultModelGroup *resIn_res = createNewInResultModel(DEFin_res, tr("Positions 2D"));
    resIn_res->setZeroOrMoreRootGroup();
    resIn_res->addGroupModel("", DEFin_grp, CT_AbstractItemGroup::staticGetType(), tr("Groupe"));
    resIn_res->addItemModel(DEFin_grp, DEFin_pos, CT_Point2D::staticGetType(), tr("Position 2D"));
}

// Creation and affiliation of OUT models
void ONF_StepModifyPositions2D::createOutResultModelListProtected()
{
    CT_OutResultModelGroup *res = createNewOutResultModel(DEFout_res, tr("Positions 2D"));
    res->setRootGroup(DEFout_grp, new CT_StandardItemGroup(), tr("Groupe"));
    res->addItemModel(DEFout_grp, DEFout_pos, new CT_Point2D(), tr("Position 2D"));
}

// Semi-automatic creation of step parameters DialogBox
void ONF_StepModifyPositions2D::createPostConfigurationDialog()
{
    // No parameter dialog for this step
}

void ONF_StepModifyPositions2D::compute()
{
    m_doc = NULL;
    m_status = 0;

    QList<CT_ResultGroup*> inResultList = getInputResults();
    CT_ResultGroup* inRes = inResultList.at(0);

    QList<CT_ResultGroup*> outResultList = getOutResultList();
    _outRes = outResultList.at(0);

    _modelCreation = (CT_OutAbstractSingularItemModel*)PS_MODELS->searchModelForCreation(DEFout_pos, _outRes);


    // IN results browsing
    CT_ResultGroupIterator itIn_grp(inRes, this, DEFin_grp);
    while (itIn_grp.hasNext() && !isStopped())
    {
        const CT_AbstractItemGroup* grpIn_grp = (CT_AbstractItemGroup*) itIn_grp.next();

        CT_Point2D* itemIn_pos = (CT_Point2D*)grpIn_grp->firstItemByINModelName(this, DEFin_pos);
        if (itemIn_pos != NULL)
        {
            _positions.append((CT_Point2D*) itemIn_pos->copy(_modelCreation, _outRes, CT_ResultCopyModeList() << CT_ResultCopyModeList::CopyItemDrawableCompletely));
        }
    }

    // request the manual mode
    requestManualMode();

    for (int i = 0 ; i < _positions.size() ; i++)
    {
        CT_StandardItemGroup* grp_grp = new CT_StandardItemGroup(DEFout_grp, _outRes);
        _outRes->addGroup(grp_grp);

        grp_grp->addItemDrawable(_positions.at(i));
    }

    m_status = 1;
    requestManualMode();
}



void ONF_StepModifyPositions2D::initManualMode()
{
    // create a new 3D document
    if(m_doc == NULL)
        m_doc = getGuiContext()->documentManager()->new3DDocument();

    // change camera type to orthographic
    if(m_doc != NULL && !m_doc->views().isEmpty())
        dynamic_cast<GraphicsViewInterface*>(m_doc->views().at(0))->camera()->setType(CameraInterface::ORTHOGRAPHIC);

    m_doc->removeAllItemDrawable();

    m_doc->setCurrentAction(new ONF_ActionModifyPositions2D(_positions, _modelCreation, _outRes));

    QMessageBox::information(NULL, tr("Mode manuel"), tr("Bienvenue dans le mode manuel de cette étape"), QMessageBox::Ok);
}

void ONF_StepModifyPositions2D::useManualMode(bool quit)
{
    if(m_status == 0)
    {
        if(quit)
        {
        }
    }
    else if(m_status == 1)
    {
        if(!quit)
        {
            m_doc = NULL;
            quitManualMode();
        }
    }
}
