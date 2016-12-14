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

#include "onf_stepcomputeboundary.h"

#ifdef USE_OPENCV
#ifdef USE_GEOS

#include "ct_itemdrawable/ct_scene.h"
#include "ct_itemdrawable/ct_standarditemgroup.h"
#include "ct_itemdrawable/ct_loopcounter.h"
#include "ct_itemdrawable/ct_image2d.h"
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

#include "geos/operation/union/CascadedPolygonUnion.h"

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
ONF_StepComputeBoundary::ONF_StepComputeBoundary(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _res = 5.0;

    geos::geom::PrecisionModel precModel(geos::geom::PrecisionModel::FLOATING);
    _factory = geos::geom::GeometryFactory::create(&precModel, -1);
}

// Step description (tooltip of contextual menu)
QString ONF_StepComputeBoundary::getStepDescription() const
{
    return tr("Calculer enveloppe concave");
}

// Step detailled description
QString ONF_StepComputeBoundary::getStepDetailledDescription() const
{
    return tr("No detailled description for this step");
}

// Step URL
QString ONF_StepComputeBoundary::getStepURL() const
{
    //return tr("STEP URL HERE");
    return CT_AbstractStep::getStepURL(); //by default URL of the plugin
}

// Step copy method
CT_VirtualAbstractStep* ONF_StepComputeBoundary::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new ONF_StepComputeBoundary(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void ONF_StepComputeBoundary::createInResultModelListProtected()
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
void ONF_StepComputeBoundary::createOutResultModelListProtected()
{
    CT_OutResultModelGroup *resultConvexHull = createNewOutResultModel(DEFout_res, tr("Convave Hull"));
    resultConvexHull->setRootGroup("root");
    resultConvexHull->addItemModel("root", "raster", new CT_Image2D<uchar>(), tr("IMAGE Hull"));
    resultConvexHull->addItemModel("root", "raster2", new CT_Image2D<uchar>(), tr("Dilaté Hull"));
    resultConvexHull->addGroupModel("root", DEFout_grp);
    resultConvexHull->addItemModel(DEFout_grp, DEFout_convexhull, new CT_Polygon2D(), tr("Convave Hull"));
}

// Semi-automatic creation of step parameters DialogBox
void ONF_StepComputeBoundary::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();
    configDialog->addDouble(tr("Résolution"), "", 0.1, 10000, 1, _res);
}

void ONF_StepComputeBoundary::compute()
{
    QList<CT_ResultGroup*> inResultList = getInputResults();
    CT_ResultGroup* rscene = inResultList.at(0);

    bool last_turn = false;
    CT_ResultGroup* rcounter = NULL;
    if (inResultList.size() > 1) {rcounter = inResultList.at(1);}
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
                    last_turn = true;
               }
            }
        }
    }

    QList<CT_ResultGroup*> outResultList = getOutResultList();
    CT_ResultGroup* rconvexHull = outResultList.at(0);

    double xmin = std::numeric_limits<double>::max();
    double xmax = -std::numeric_limits<double>::max();
    double ymin = std::numeric_limits<double>::max();
    double ymax = -std::numeric_limits<double>::max();

    CT_ResultGroupIterator itSc0(rscene, this, DEFin_grpsc);
    while (itSc0.hasNext() && !isStopped())
    {
        CT_StandardItemGroup* grp = (CT_StandardItemGroup*) itSc0.next();
        const CT_Scene* scene = (CT_Scene*)grp->firstItemByINModelName(this, DEFin_scene);

        if (scene->minX() < xmin) {xmin = scene->minX();}
        if (scene->maxX() > xmax) {xmax = scene->maxX();}
        if (scene->minY() < ymin) {ymin = scene->minY();}
        if (scene->maxY() > ymax) {ymax = scene->maxY();}
    }

    // Obtain reguar coordinates (multiples of _res)
    double xmin2 = std::floor(xmin / _res) * _res - 2.0*_res;
    double ymin2 = std::floor(ymin / _res) * _res - 2.0*_res;

    double xmax2 = xmin2;
    double ymax2 = ymin2;

    while (xmax2 < xmax) {xmax2 += _res;}
    while (ymax2 < ymax) {ymax2 += _res;}

    xmax2 += 2.0*_res;
    ymax2 += 2.0*_res;

    CT_Image2D<uchar>* raster = CT_Image2D<uchar>::createImage2DFromXYCoords("raster", rconvexHull, xmin2, ymin2, xmax2, ymax2, _res, false, false, false);
    CT_Image2D<uchar>* raster2 = CT_Image2D<uchar>::createImage2DFromXYCoords("raster2", rconvexHull, xmin2, ymin2, xmax2, ymax2, _res, false, false, false);

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
                raster->setValueAtCoords(point(0), point(1), true);
            }
        }
    }

    std::vector<std::vector<cv::Point> > contours;
    cv::dilate(raster->getMat(), raster2->getMat(), cv::getStructuringElement(cv::MORPH_RECT, cv::Size2d(3,3)));
    cv::findContours(raster2->getMat(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

    CT_StandardItemGroup* rootGrp = new CT_StandardItemGroup("root", rconvexHull);
    rootGrp->addItemDrawable(raster);
    rootGrp->addItemDrawable(raster2);
    rconvexHull->addGroup(rootGrp);

    for (int i = 0 ; i < contours.size() ; i++)
    {
        const std::vector<cv::Point> &contour = contours.at(i);

        QList<geos::geom::Coordinate> vertices;

        for (int j = 0 ; j < contour.size() ; j++)
        {
            const cv::Point &vert = contour.at(j);
            int xx = vert.x;
            int yy = vert.y;
            vertices.append(geos::geom::Coordinate(raster->getCellCenterColCoord(xx), raster->getCellCenterLinCoord(yy)));
        }

        if (vertices.size() > 0)
        {
            geos::geom::Polygon* poly = createPolygon(vertices);
            _polygonsList.push_back(poly);
        }

    }

    if (last_turn)
    {
        geos::operation::geounion::CascadedPolygonUnion unionPoly(&_polygonsList);
        geos::geom::Geometry* geometry = unionPoly.Union();

        for (size_t i = 0 ; i < geometry->getNumGeometries() ; i++)
        {
            geos::geom::Geometry* polyGeom = (geos::geom::Geometry*) geometry->getGeometryN(i);
            geos::geom::CoordinateSequence* sequence = polyGeom->getBoundary()->getCoordinates();
            QVector<Eigen::Vector2d *> vertices;

            for (int j = 0 ; j < sequence->getSize() ; j++)
            {
                vertices.append(new Eigen::Vector2d(sequence->getX(j), sequence->getY(j)));
            }

            CT_Polygon2DData* dataPoly = new CT_Polygon2DData(vertices, false);
            CT_Polygon2D* convexHull = new CT_Polygon2D(DEFout_convexhull, rconvexHull, dataPoly);
            CT_StandardItemGroup* outGrp = new CT_StandardItemGroup(DEFout_grp, rconvexHull);
            outGrp->addItemDrawable(convexHull);
            rootGrp->addGroup(outGrp);
        }


        for (size_t i = 0 ; i < _polygonsList.size() ; i++)
        {
            delete _polygonsList[i];
        }
        _polygonsList.resize(0);
    }

}

geos::geom::Polygon* ONF_StepComputeBoundary::createPolygon(QList<geos::geom::Coordinate> vertices)
{
    geos::geom::CoordinateSequence* temp = _factory->getCoordinateSequenceFactory()->create((std::size_t) 0, 0);

    for (int i = 0 ; i < vertices.size() ; i++)
    {
        temp->add(vertices.at(i));

    }
    if (vertices.size() > 0)
    {
        temp->add(vertices.first());

    }
    geos::geom::LinearRing *shell = _factory->createLinearRing(temp);
    geos::geom::Polygon* poly = _factory->createPolygon(shell, NULL);

    return poly;
}

#endif
#endif
