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

#include "onf_stepcomputeedbhfromheightallometry.h"

#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"

#include "ct_itemdrawable/ct_referencepoint.h"
#include "ct_itemdrawable/ct_circle2d.h"
#include "ct_shapedata/ct_linedata.h"

#include "ct_itemdrawable/tools/iterator/ct_groupiterator.h"
#include "ct_view/ct_stepconfigurabledialog.h"

#include "ct_result/ct_resultgroup.h"
#include "qdebug.h"

#define DEF_SearchInRefPoint  "rp"
#define DEF_SearchInGroup       "g"
#define DEF_SearchInResult      "r"


ONF_StepComputeDBHFromHeightAllometry::ONF_StepComputeDBHFromHeightAllometry(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _param_Hmax = 40.0;
    _param_a = 1.6;
    _param_m = 51.0;
}

QString ONF_StepComputeDBHFromHeightAllometry::getStepDescription() const
{
    return tr("Calculer un DBH par allométrie sur la hauteur");
}

QString ONF_StepComputeDBHFromHeightAllometry::getStepDetailledDescription() const
{
    return tr("No detailled description for this step");
}

CT_VirtualAbstractStep* ONF_StepComputeDBHFromHeightAllometry::createNewInstance(CT_StepInitializeData &dataInit)
{
    // cree une copie de cette etape
    return new ONF_StepComputeDBHFromHeightAllometry(dataInit);
}

//////////////////// PROTECTED //////////////////

void ONF_StepComputeDBHFromHeightAllometry::createInResultModelListProtected()
{
    CT_InResultModelGroupToCopy* resultModel = createNewInResultModelForCopy(DEF_SearchInResult, tr("Apex"));
    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_SearchInGroup, CT_AbstractItemGroup::staticGetType(), tr("Apex (Grp)"));
    resultModel->addItemModel(DEF_SearchInGroup, DEF_SearchInRefPoint, CT_ReferencePoint::staticGetType(), tr("Apex"));
}

void ONF_StepComputeDBHFromHeightAllometry::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addTitle( tr("Relation allometrique (H - Hmax)*(D - a*(H - 1.3)) = m :"));
    configDialog->addDouble(tr("Paramètre Hmax"), "m", 0, 1000, 2, _param_Hmax);
    configDialog->addDouble(tr("Paramètre a"), "", 0, 1000, 2, _param_a);
    configDialog->addDouble(tr("Paramètre m"), "", 0, 1000, 2, _param_m);

}

void ONF_StepComputeDBHFromHeightAllometry::createOutResultModelListProtected()
{
    CT_OutResultModelGroupToCopyPossibilities *res = createNewOutResultModelToCopy(DEF_SearchInResult);

    if(res != NULL)
        res->addItemModel(DEF_SearchInGroup, _outDBHModelName, new CT_Circle2D(), tr("DBH"));
}

void ONF_StepComputeDBHFromHeightAllometry::compute()
{
    // on récupère le résultat copié
    CT_ResultGroup *outRes = getOutResultList().first();

    CT_ResultGroupIterator itSection(outRes, this, DEF_SearchInGroup);
    while (itSection.hasNext() && (!isStopped()))
    {
        CT_StandardItemGroup *group = (CT_StandardItemGroup*) itSection.next();

        CT_ReferencePoint* apex = (CT_ReferencePoint*) group->firstItemByINModelName(this, DEF_SearchInRefPoint);

        if (apex != NULL)
        {

            double radius = computeAllometricDFromH(apex->getCenterZ()) / 2.0;
            CT_Circle2D* circle = new CT_Circle2D(_outDBHModelName.completeName(), outRes, new CT_Circle2DData(Eigen::Vector2d(apex->getCenterX(), apex->getCenterY()), radius));
            group->addItemDrawable(circle);
        }
    }
}

double ONF_StepComputeDBHFromHeightAllometry::computeAllometricDFromH(double h)
{
    if (h <= 0.0) {return 0.0;}
    return (_param_a*(h - 1.3) - _param_m / (h - _param_Hmax)) / 100.0;
}
