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


#include "onf_steprefpointfrombarycenter02.h"

#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"

#include "ct_itemdrawable/ct_pointcluster.h"
#include "ct_itemdrawable/ct_referencepoint.h"
#include "ct_itemdrawable/tools/ct_standardcontext.h"

#include "ct_result/ct_resultgroup.h"

#include "ct_tools/model/ct_outmodelcopyactionaddmodelitemingroup.h"

#include "ct_pointcloudindex/abstract/ct_abstractpointcloudindex.h"
#include "ct_iterator/ct_pointiterator.h"


#define DEF_SearchInGroup           "g"
#define DEF_SearchInPointCluster    "p"
#define DEF_SearchInResult          "r"
#define DEF_SearchOutResult         "r"

ONF_StepRefPointFromBarycenter02::ONF_StepRefPointFromBarycenter02(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
}

QString ONF_StepRefPointFromBarycenter02::getStepDescription() const
{
    return tr("Création de points de réf. à partir de barycentres");
}

QString ONF_StepRefPointFromBarycenter02::getStepDetailledDescription() const
{
    return tr("No detailled description for this step");
}

CT_VirtualAbstractStep* ONF_StepRefPointFromBarycenter02::createNewInstance(CT_StepInitializeData &dataInit)
{
    // cree une copie de cette etape
    return new ONF_StepRefPointFromBarycenter02(dataInit);
}

//////////////////// PROTECTED //////////////////

void ONF_StepRefPointFromBarycenter02::createInResultModelListProtected()
{
    CT_InResultModelGroupToCopy *resultModel = createNewInResultModelForCopy(DEF_SearchInResult, tr("Polylignes"));

    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_SearchInGroup);
    resultModel->addItemModel(DEF_SearchInGroup, DEF_SearchInPointCluster, CT_PointCluster::staticGetType(), tr("Polyligne"));
}

void ONF_StepRefPointFromBarycenter02::createOutResultModelListProtected()
{
    CT_OutResultModelGroupToCopyPossibilities *res = createNewOutResultModelToCopy(DEF_SearchInResult);
    res->addItemModel(DEF_SearchInGroup, _outRefPointModelName, new CT_ReferencePoint(), tr("Barycentre"));
}

void ONF_StepRefPointFromBarycenter02::compute()
{
    // on récupère le résultat copié
    CT_ResultGroup *outRes = getOutResultList().first();

    CT_ResultGroupIterator it(outRes, this, DEF_SearchInGroup);
    while (it.hasNext() && !isStopped())
    {
        CT_AbstractItemGroup *group = (CT_AbstractItemGroup*) it.next();

        const CT_PointCluster *item = (const CT_PointCluster*)group->firstItemByINModelName(this, DEF_SearchInPointCluster);

        if(item != NULL)
        {
            ONF_StepRefPointFromBarycenter02::addBarycenter(item, group, _outRefPointModelName.completeName(), outRes);
        }
    }
}

CT_ReferencePoint* ONF_StepRefPointFromBarycenter02::addBarycenter(const CT_PointCluster *item, CT_AbstractItemGroup *group, const QString &outRefPointModelName, CT_ResultGroup *outResult)
{
    const CT_PointClusterBarycenter barycentre = item->getBarycenter();

    // coordonnées du barycentre
    float xref = barycentre.x();
    float yref = barycentre.y();
    float zref = barycentre.z();

    // calcul du bufferXY
    // Maximum de la distance point/refPoint pour chaque segment
    float buffer = 0;

    CT_PointIterator itPt(item->getPointCloudIndex());
    while (itPt.hasNext())
    {
        const CT_Point &point = itPt.next().currentPoint();
        float distance = pow(xref-point(0), 2) + pow(yref-point(1), 2);
        if (distance > buffer) {buffer = distance;}
    }
    if (buffer > 0) {buffer = sqrt(buffer);}

    // et on ajoute un referencePoint
    CT_ReferencePoint *refPoint = new CT_ReferencePoint(outRefPointModelName, outResult, xref, yref, zref, buffer);
    group->addItemDrawable(refPoint);
    return refPoint;
}
