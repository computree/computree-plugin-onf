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


#include "onf_steprefpointfromarccenter.h"

#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"

#include "ct_itemdrawable/ct_pointcluster.h"
#include "ct_itemdrawable/ct_referencepoint.h"

#include "ct_itemdrawable/tools/pointclustertools/ct_polylinesalgorithms.h"

#include "ct_result/ct_resultgroup.h"
#include "qvector2d.h"

#include "ct_tools/model/ct_outmodelcopyactionaddmodelitemingroup.h"

#define DEF_SearchInGroup       "g"
#define DEF_SearchInPolyline    "p"
#define DEF_SearchInResult      "r"
#define DEF_SearchOutResult     "r"

ONF_StepRefPointFromArcCenter::ONF_StepRefPointFromArcCenter(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
}

QString ONF_StepRefPointFromArcCenter::getStepDescription() const
{
    return tr("Créer des points de référence à partir d'Arcs");
}

QString ONF_StepRefPointFromArcCenter::getStepDetailledDescription() const
{
    return tr("No detailled description for this step");
}

CT_VirtualAbstractStep* ONF_StepRefPointFromArcCenter::createNewInstance(CT_StepInitializeData &dataInit)
{
    // cree une copie de cette etape
    return new ONF_StepRefPointFromArcCenter(dataInit);
}

//////////////////// PROTECTED //////////////////

void ONF_StepRefPointFromArcCenter::createInResultModelListProtected()
{
    CT_InResultModelGroupToCopy *resultModel = createNewInResultModelForCopy(DEF_SearchInResult, tr("Polyline(s)"));

    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_SearchInGroup);
    resultModel->addItemModel(DEF_SearchInGroup, DEF_SearchInPolyline, CT_PointCluster::staticGetType(), tr("Polyligne"));
}

void ONF_StepRefPointFromArcCenter::createOutResultModelListProtected()
{
    CT_OutResultModelGroupToCopyPossibilities *res = createNewOutResultModelToCopy(DEF_SearchInResult);
    res->addItemModel(DEF_SearchInGroup, _outRefPointModelName, new CT_ReferencePoint(), tr("Barycentre"));
}

void ONF_StepRefPointFromArcCenter::compute()
{
    // on récupère le résultat copié
    CT_ResultGroup *outRes = getOutResultList().first();

    CT_ResultGroupIterator it(outRes, this, DEF_SearchInGroup);
    while (it.hasNext() && !isStopped())
    {
        CT_AbstractItemGroup *group = (CT_AbstractItemGroup*) it.next();

        const CT_PointCluster *item = (const CT_PointCluster*)group->firstItemByINModelName(this, DEF_SearchInPolyline);

        if(item != NULL)
        {
            double sagitta = 0;
            double chord = 0;
            double radius = 0;
            Eigen::Vector2d center = CT_PolylinesAlgorithms::compute2DArcData(item, sagitta, chord, radius);

            if (radius > chord)
            {
                radius = chord;
            }

            // et on ajoute un referencePoint
            group->addItemDrawable(new CT_ReferencePoint(_outRefPointModelName.completeName(), outRes, center.x(), center.y(), item->getCenterZ(), radius));
        }
    }

    setProgress(100);
}

