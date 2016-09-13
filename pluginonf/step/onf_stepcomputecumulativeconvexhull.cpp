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

#include "onf_stepcomputecumulativeconvexhull.h"

#include "ct_itemdrawable/ct_scene.h"
#include "ct_itemdrawable/ct_standarditemgroup.h"
#include "ct_itemdrawable/ct_loopcounter.h"
#include "ct_iterator/ct_pointiterator.h"
#include "ct_itemdrawable/tools/iterator/ct_groupiterator.h"
#include "ct_itemdrawable/tools/iterator/ct_itemiterator.h"
#include "ct_result/ct_resultgroup.h"
#include "ct_result/model/inModel/ct_inresultmodelgroup.h"
#include "ct_result/model/outModel/ct_outresultmodelgroup.h"
#include "ct_view/ct_stepconfigurabledialog.h"

#include <QDebug>

// Alias for indexing models
#define DEFin_rscene "rscene"
#define DEFin_grpsc "grpsc"
#define DEFin_scene "scene"

#define DEF_inResultCounter "rcounter"
#define DEF_inCounter "counter"

#define DEFout_res "outres"
#define DEFout_grp "outgrp"
#define DEFout_convexhull "outconvexHull"

// Constructor : initialization of parameters
ONF_StepComputeCumulativeConvexHull::ONF_StepComputeCumulativeConvexHull(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _cumulatedConvexHull = NULL;
}

// Step description (tooltip of contextual menu)
QString ONF_StepComputeCumulativeConvexHull::getStepDescription() const
{
    return tr("Calculer l'enveloppe convexe cumulée");
}

// Step detailled description
QString ONF_StepComputeCumulativeConvexHull::getStepDetailledDescription() const
{
    return tr("No detailled description for this step");
}

// Step URL
QString ONF_StepComputeCumulativeConvexHull::getStepURL() const
{
    //return tr("STEP URL HERE");
    return CT_AbstractStep::getStepURL(); //by default URL of the plugin
}

// Step copy method
CT_VirtualAbstractStep* ONF_StepComputeCumulativeConvexHull::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new ONF_StepComputeCumulativeConvexHull(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void ONF_StepComputeCumulativeConvexHull::createInResultModelListProtected()
{
    CT_InResultModelGroup *resIn_rscene = createNewInResultModel(DEFin_rscene, tr("Scène(s)"));
    resIn_rscene->setZeroOrMoreRootGroup();
    resIn_rscene->addGroupModel("", DEFin_grpsc, CT_AbstractItemGroup::staticGetType(), tr("Groupe"));
    resIn_rscene->addItemModel(DEFin_grpsc, DEFin_scene, CT_Scene::staticGetType(), tr("Scène"));

    CT_InResultModelGroup* res_counter = createNewInResultModel(DEF_inResultCounter, tr("Résultat compteur"), "", true);
    res_counter->setZeroOrMoreRootGroup();
    res_counter->addItemModel("", DEF_inCounter, CT_LoopCounter::staticGetType(), tr("Compteur"));
    res_counter->setMinimumNumberOfPossibilityThatMustBeSelectedForOneTurn(0);
}

// Creation and affiliation of OUT models
void ONF_StepComputeCumulativeConvexHull::createOutResultModelListProtected()
{
    CT_OutResultModelGroup *resultConvexHull = createNewOutResultModel(DEFout_res, tr("Convex Hull (cumulative)"));
    resultConvexHull->setRootGroup(DEFout_grp);
    resultConvexHull->addItemModel(DEFout_grp, DEFout_convexhull, new CT_Polygon2D(), tr("Convex Hull"));
}

// Semi-automatic creation of step parameters DialogBox
void ONF_StepComputeCumulativeConvexHull::createPostConfigurationDialog()
{
//    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();
}

void ONF_StepComputeCumulativeConvexHull::compute()
{
    QList<CT_ResultGroup*> inResultList = getInputResults();
    CT_ResultGroup* rscene = inResultList.at(0);

    CT_ResultGroup* rcounter = NULL;
    if (inResultList.size() > 1) {rcounter = inResultList.at(1);}

    QList<CT_ResultGroup*> outResultList = getOutResultList();
    CT_ResultGroup* rconvexHull = outResultList.at(0);

    QList<Eigen::Vector2d *> allPoints;

    if (_cumulatedConvexHull != NULL)
    {
        const QVector<Eigen::Vector2d*>& vertices = _cumulatedConvexHull->getVertices();
        for (int i = 0 ; i < _cumulatedConvexHull->getVerticesNumber() ; i++)
        {
            Eigen::Vector2d* vert = vertices.at(i);
            Eigen::Vector2d* vertcpy = new Eigen::Vector2d((*vert)(0), (*vert)(1));
            allPoints.append(vertcpy);
        }
        delete _cumulatedConvexHull;
        _cumulatedConvexHull = NULL;
    }

    // parcours des scènes pour calculer l'enveloppe convexe
    CT_ResultGroupIterator itSc(rscene, this, DEFin_grpsc);
    while (itSc.hasNext() && !isStopped())
    {
        CT_StandardItemGroup* grp = (CT_StandardItemGroup*) itSc.next();
        const CT_Scene* scene = (CT_Scene*)grp->firstItemByINModelName(this, DEFin_scene);
        if (scene != NULL)
        {
            // création de la liste complète des points
            CT_PointIterator itP(scene->getPointCloudIndex());
            while(itP.hasNext() && (!isStopped()))
            {
                const CT_Point &point = itP.next().currentPoint();
                Eigen::Vector2d *point2D = new Eigen::Vector2d(point(0), point(1));
                allPoints.append(point2D);
            }
        }
    }

    CT_Polygon2DData::orderPointsByXY(allPoints);
    _cumulatedConvexHull = CT_Polygon2DData::createConvexHull(allPoints);
    if (_cumulatedConvexHull != NULL)
    {
        CT_Polygon2D* convexHull = new CT_Polygon2D(DEFout_convexhull, rconvexHull, (CT_Polygon2DData*) _cumulatedConvexHull->copy());
        CT_StandardItemGroup* outGrp = new CT_StandardItemGroup(DEFout_grp, rconvexHull);
        outGrp->addItemDrawable(convexHull);
        rconvexHull->addGroup(outGrp);

        if (rcounter != NULL)
        {
            CT_ResultItemIterator itCounter(rcounter, this, DEF_inCounter);
            if (itCounter.hasNext())
            {
                const CT_LoopCounter* counter = (const CT_LoopCounter*) itCounter.next();
                if (counter != NULL)
                {
                    if (counter->getCurrentTurn() == counter->getNTurns())
                    {
                        delete _cumulatedConvexHull;
                        _cumulatedConvexHull = NULL;
                    }
                }
            }
        }
    }

}

