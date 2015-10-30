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


#include "onf_stepfitcirclesandfilter.h"

#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"

#include "ct_itemdrawable/abstract/ct_abstractitemdrawablewithpointcloud.h"
#include "ct_itemdrawable/ct_circle.h"

#include "ct_result/ct_resultgroup.h"
#include "ct_view/ct_stepconfigurabledialog.h"

#define DEF_SearchInGroup       "g"
#define DEF_SearchInPointCloud  "p"
#define DEF_SearchInResult      "r"
#define DEF_SearchOutResult     "r"

ONF_StepFitCirclesAndFilter::ONF_StepFitCirclesAndFilter(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _max_error = 0.01;
    _min_radius = 0.02;
    _max_radius = 1.20;
    _activeFiltering = true;
}

QString ONF_StepFitCirclesAndFilter::getStepDescription() const
{
    return tr("Ajuster/Filtrer un Cercle horizontal par Cluster");
}

QString ONF_StepFitCirclesAndFilter::getStepDetailledDescription() const
{
    return tr("Cette étape ajoute un cercle dans chaque cluster d'entrée.<br>"
              "Les cercles sont ajustés par moindres carrés sur les groupes de points.<br>"
              "Les paramètres de l'étape permettent d'activer optionnellement un  <b>filtrage</b> de cercles.<br>"
              "Les criètres de filtrages sont le <b>rayon minimum</b>, le <b>rayon maximum</b> et l' <b>erreur d'ajustement du cercle</b> maximale autorisée.");
}

CT_VirtualAbstractStep* ONF_StepFitCirclesAndFilter::createNewInstance(CT_StepInitializeData &dataInit)
{
    // cree une copie de cette etape
    return new ONF_StepFitCirclesAndFilter(dataInit);
}

//////////////////// PROTECTED //////////////////

void ONF_StepFitCirclesAndFilter::createInResultModelListProtected()
{
    CT_InResultModelGroupToCopy * resultModel = createNewInResultModelForCopy(DEF_SearchInResult, tr("Clusters"));
    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_SearchInGroup, CT_AbstractItemGroup::staticGetType(), tr("Cluster (Grp)"));
    resultModel->addItemModel(DEF_SearchInGroup, DEF_SearchInPointCloud, CT_AbstractItemDrawableWithPointCloud::staticGetType(), tr("Points"));
}

void ONF_StepFitCirclesAndFilter::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addBool("", "", tr("Filtrer les cercles sur les critres suivants"), _activeFiltering);
    configDialog->addDouble(tr("Rayon minimum  :"), "cm", 0, 1000, 2, _min_radius, 100);
    configDialog->addDouble(tr("Rayon maximum  :"), "cm", 0, 1000, 2, _max_radius, 100);
    configDialog->addDouble(tr("Erreur maximum :"), "", 0, 99999.99, 4, _max_error);

}

void ONF_StepFitCirclesAndFilter::createOutResultModelListProtected()
{
    CT_OutResultModelGroupToCopyPossibilities *res = createNewOutResultModelToCopy(DEF_SearchInResult);
    res->addItemModel(DEF_SearchInGroup, _outCircleModelName, new CT_Circle(), tr("Cercle"));
}

void ONF_StepFitCirclesAndFilter::compute()
{
    // on récupre le résultat copié
    CT_ResultGroup *outRes = getOutResultList().first();

    CT_ResultGroupIterator it(outRes, this, DEF_SearchInGroup);
    while (it.hasNext() && (!isStopped()))
    {
        CT_AbstractItemGroup *group = (CT_AbstractItemGroup*) it.next();

        const CT_AbstractItemDrawableWithPointCloud *item = (const CT_AbstractItemDrawableWithPointCloud*)group->firstItemByINModelName(this, DEF_SearchInPointCloud);

        if(item != NULL)
        {
            CT_CircleData *cData = CT_CircleData::staticCreateZAxisAlignedCircleDataFromPointCloud(*item->getPointCloudIndex(), item->getCenterZ());

            // et on ajoute un cercle
            if(cData != NULL)
            {
                if (!_activeFiltering || ((cData->getError() < _max_error) && (cData->getRadius() > _min_radius) && (cData->getRadius() < _max_radius)))
                {
                    group->addItemDrawable(new CT_Circle(_outCircleModelName.completeName(),outRes,cData));
                }
                else
                {
                    delete cData;
                }
            }
        }
    }

}
