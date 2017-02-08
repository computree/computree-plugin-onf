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

#include "onf_stepcomputeboundaryv2.h"

#ifdef USE_OPENCV

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
#include "ct_itemdrawable/ct_polygon2d.h"


#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgproc/types_c.h"
#include "opencv2/core/core.hpp"

#include <QDebug>


// Alias for indexing models
#define DEFin_rscene "rscene"
#define DEFin_grpsc "grpsc"
#define DEFin_scene "scene"

#define DEFin_rfootprint "rfootprint"
#define DEFin_grpfootprint "grpfootprint"
#define DEFin_footprint "footprint"

#define DEF_inResultCounter "rcounter"
#define DEF_inCounter "counter"

#define DEFout_res "outres"
#define DEFout_grp "outgrp"
#define DEFout_raster "outraster"

// Constructor : initialization of parameters
ONF_StepComputeBoundaryV2::ONF_StepComputeBoundaryV2(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _res = 10.0;
    _outRaster = NULL;
}

// Step description (tooltip of contextual menu)
QString ONF_StepComputeBoundaryV2::getStepDescription() const
{
    return tr("Calculer un raster d'emprise");
}

// Step detailled description
QString ONF_StepComputeBoundaryV2::getStepDetailledDescription() const
{
    return tr("Calcul d'un raster logique, avec une valeur 1 pour toute cellule contenant au moins un point");
}

// Step URL
QString ONF_StepComputeBoundaryV2::getStepURL() const
{
    //return tr("STEP URL HERE");
    return CT_AbstractStep::getStepURL(); //by default URL of the plugin
}

// Step copy method
CT_VirtualAbstractStep* ONF_StepComputeBoundaryV2::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new ONF_StepComputeBoundaryV2(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void ONF_StepComputeBoundaryV2::createInResultModelListProtected()
{
    CT_InResultModelGroup *resIn_footprint = createNewInResultModel(DEFin_rfootprint, tr("Emprise totale"), "", true);
    resIn_footprint->setZeroOrMoreRootGroup();
    resIn_footprint->addGroupModel("", DEFin_grpfootprint, CT_AbstractItemGroup::staticGetType(), tr("Groupe"));
    resIn_footprint->addItemModel(DEFin_grpfootprint, DEFin_footprint, CT_AbstractSingularItemDrawable::staticGetType(), tr("Emprise"));

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
void ONF_StepComputeBoundaryV2::createOutResultModelListProtected()
{
    CT_OutResultModelGroup *resultConvexHull = createNewOutResultModel(DEFout_res, tr("Footprint"));
    resultConvexHull->setRootGroup(DEFout_grp);
    resultConvexHull->addItemModel(DEFout_grp, DEFout_raster, new CT_Image2D<uchar>(), tr("Footprint Raster"));
}

// Semi-automatic creation of step parameters DialogBox
void ONF_StepComputeBoundaryV2::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();
    configDialog->addDouble(tr("Résolution"), "m", 0.1, 10000, 1, _res);
}

void ONF_StepComputeBoundaryV2::compute()
{
    QList<CT_ResultGroup*> inResultList = getInputResults();
    CT_ResultGroup* rfootprint = inResultList.at(0);
    CT_ResultGroup* rscene = inResultList.at(1);

    bool last_turn = true;
    bool first_turn = true;
    CT_ResultGroup* rcounter = NULL;
    if (inResultList.size() > 2) {rcounter = inResultList.at(2);}
    if (rcounter != NULL)
    {
        CT_ResultItemIterator itCounter(rcounter, this, DEF_inCounter);
        if (itCounter.hasNext())
        {
            const CT_LoopCounter* counter = (const CT_LoopCounter*) itCounter.next();
            if (counter != NULL)
            {
                if (counter->getCurrentTurn() > 1)
                {
                    first_turn = false;
                }
                if (counter->getCurrentTurn() != counter->getNTurns())
                {
                    last_turn = false;
                }
            }
        }
    }

    if (first_turn)
    {
        _outRaster = NULL;

        // Compute cumulated Bounding Box
        double xmin = std::numeric_limits<double>::max();
        double xmax = -std::numeric_limits<double>::max();
        double ymin = std::numeric_limits<double>::max();
        double ymax = -std::numeric_limits<double>::max();

        CT_ResultItemIterator itFootprint(rfootprint, this, DEFin_footprint);
        while (itFootprint.hasNext())
        {
            const CT_AbstractSingularItemDrawable* item = itFootprint.next();

            if (item->hasBoundingBox())
            {
                Eigen::Vector3d min, max;
                item->getBoundingBox(min, max);

                if (min(0) < xmin) {xmin = min(0);}
                if (min(1) < ymin) {ymin = min(1);}
                if (max(0) > xmax) {xmax = max(0);}
                if (max(1) > ymax) {ymax = max(1);}
            }
        }

        if (xmin < std::numeric_limits<double>::max() &&
                xmax > -std::numeric_limits<double>::max() &&
                ymin < std::numeric_limits<double>::max() &&
                ymax > -std::numeric_limits<double>::max())
        {
            // Obtain reguar coordinates (multiples of _res)
            double xmin2 = std::floor(xmin / _res) * _res - 2.0*_res;
            double ymin2 = std::floor(ymin / _res) * _res - 2.0*_res;

            double xmax2 = xmin2;
            double ymax2 = ymin2;

            while (xmax2 < xmax) {xmax2 += _res;}
            while (ymax2 < ymax) {ymax2 += _res;}

            xmax2 += 2.0*_res;
            ymax2 += 2.0*_res;

            _outRaster = CT_Image2D<uchar>::createImage2DFromXYCoords(NULL, NULL, xmin2, ymin2, xmax2, ymax2, _res, 0, false, false);
        }
    }

    if (_outRaster != NULL)
    {
        // parcours des scènes pour calculer l'enveloppe
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
                    _outRaster->setValueAtCoords(point(0), point(1), true);
                }
            }
        }

        if (last_turn)
        {
            QList<CT_ResultGroup*> outResultList = getOutResultList();
            CT_ResultGroup* rconvexHull = outResultList.at(0);
            CT_StandardItemGroup* outGrp = new CT_StandardItemGroup(DEFout_grp, rconvexHull);
            rconvexHull->addGroup(outGrp);

            _outRaster->changeResult(rconvexHull);
            _outRaster->setModel(DEFout_raster);
            outGrp->addItemDrawable(_outRaster);
        }
    }

}


#endif
