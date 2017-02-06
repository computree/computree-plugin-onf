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
#define DEFout_grpHull "outgrpHull"
#define DEFout_hull "outHull"

// Constructor : initialization of parameters
ONF_StepComputeBoundaryV2::ONF_StepComputeBoundaryV2(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _res = 10.0;
    _outRaster = NULL;
}

// Step description (tooltip of contextual menu)
QString ONF_StepComputeBoundaryV2::getStepDescription() const
{
    return tr("Calculer enveloppe concave");
}

// Step detailled description
QString ONF_StepComputeBoundaryV2::getStepDetailledDescription() const
{
    return tr("No detailled description for this step");
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
    CT_OutResultModelGroup *resultConvexHull = createNewOutResultModel(DEFout_res, tr("Convave Hull"));
    resultConvexHull->setRootGroup(DEFout_grp);
    resultConvexHull->addItemModel(DEFout_grp, DEFout_raster, new CT_Image2D<uchar>(), tr("Raster"));
    resultConvexHull->addItemModel(DEFout_grp, "toto", new CT_Image2D<uchar>(), tr("Raster2"));
    resultConvexHull->addGroupModel(DEFout_grp, DEFout_grpHull);
    resultConvexHull->addItemModel(DEFout_grpHull, DEFout_hull, new CT_Polygon2D(), tr("Convave Hull"));
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

    bool last_turn = false;
    bool first_turn = true;
    CT_ResultGroup* rcounter = NULL;
    if (inResultList.size() > 1) {rcounter = inResultList.at(2);}
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
                if (counter->getCurrentTurn() == counter->getNTurns())
                {
                    last_turn = true;
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

            cv::Mat_<uchar> raster2 = _outRaster->getMat().clone();
            //cv::dilate(_outRaster->getMat(), raster2, cv::getStructuringElement(cv::MORPH_RECT, cv::Size2d(3,3)));
            std::vector<std::vector<cv::Point> > contours;
            cv::findContours(_outRaster->getMat(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
            raster2.release();

            CT_Image2D<uchar>* raster00 = new CT_Image2D<uchar>("toto", rconvexHull, _outRaster->minX(), _outRaster->minY(), _outRaster->colDim(), _outRaster->linDim(), _outRaster->resolution(), 0, false, false);
            cv::Scalar color(1);
            cv::drawContours(raster00->getMat(), contours, -1, color, 1);
            outGrp->addItemDrawable(raster00);

            for (int i = 0 ; i < contours.size() ; i++)
            {
                const std::vector<cv::Point> &contour = contours.at(i);

                QVector<Eigen::Vector2d *> vertices;

                for (int j = 0 ; j < contour.size() ; j++)
                {
                    const cv::Point &vert = contour.at(j);
                    int xx = vert.x;
                    int yy = vert.y;

                    vertices.append(new Eigen::Vector2d(_outRaster->getCellCenterColCoord(xx), _outRaster->getCellCenterLinCoord(yy)));
                }

                if (vertices.size() > 0 && outGrp != NULL)
                {
                    CT_Polygon2DData* dataPoly = new CT_Polygon2DData(vertices, false);
                    CT_Polygon2D* convexHull = new CT_Polygon2D(DEFout_hull, rconvexHull, dataPoly);
                    CT_StandardItemGroup* outGrpHull = new CT_StandardItemGroup(DEFout_grpHull, rconvexHull);
                    outGrpHull->addItemDrawable(convexHull);
                    outGrp->addGroup(outGrpHull);
                }
            }

            _outRaster->changeResult(rconvexHull);
            _outRaster->setModel(DEFout_raster);
            outGrp->addItemDrawable(_outRaster);
        }
    }

}


#endif
