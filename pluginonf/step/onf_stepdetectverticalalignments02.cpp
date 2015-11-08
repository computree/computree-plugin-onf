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

#include "onf_stepdetectverticalalignments02.h"

#include "ct_itemdrawable/abstract/ct_abstractitemdrawablewithpointcloud.h"
#include "ct_itemdrawable/ct_pointcluster.h"
#include "ct_itemdrawable/ct_polygon2d.h"
#include "ct_itemdrawable/ct_circle2d.h"
#include "ct_itemdrawable/ct_line.h"
#include "ct_itemdrawable/ct_attributeslist.h"
#include "ct_itemdrawable/tools/iterator/ct_groupiterator.h"
#include "ct_result/ct_resultgroup.h"
#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"
#include "ct_view/ct_stepconfigurabledialog.h"

#include "ct_iterator/ct_pointiterator.h"
#include "ct_math/ct_sphericalline3d.h"
#include "ct_math/ct_mathstatistics.h"
#include "ct_math/ct_mathpoint.h"

#include <QtConcurrent>


// Alias for indexing models
#define DEFin_res "res"
#define DEFin_grp "grp"
#define DEFin_scene "scene"



// Constructor : initialization of parameters
ONF_StepDetectVerticalAlignments02::ONF_StepDetectVerticalAlignments02(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _pointDistThreshold = 4.0;
    _maxPhiAngle = 20.0;

    _lineDistThreshold = 0.4;
    _maxSpacing = 0.4;
    _minPtsNb = 3;

    _lineLengthRatio = 0.8;
    _lengthThreshold = 2.0;
    _heightThreshold = 0.6;
    _circleDistThreshold = 0.05;

    _clusterDebugMode = false;
}

// Step description (tooltip of contextual menu)
QString ONF_StepDetectVerticalAlignments02::getStepDescription() const
{
    return tr("Détecter des alignements verticaux de points (V2)");
}

// Step detailled description
QString ONF_StepDetectVerticalAlignments02::getStepDetailledDescription() const
{
    return tr("No detailled description for this step");
}

// Step URL
QString ONF_StepDetectVerticalAlignments02::getStepURL() const
{
    //return tr("STEP URL HERE");
    return CT_AbstractStep::getStepURL(); //by default URL of the plugin
}

// Step copy method
CT_VirtualAbstractStep* ONF_StepDetectVerticalAlignments02::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new ONF_StepDetectVerticalAlignments02(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void ONF_StepDetectVerticalAlignments02::createInResultModelListProtected()
{
    CT_InResultModelGroupToCopy *resIn_res = createNewInResultModelForCopy(DEFin_res, tr("Scènes"));
    resIn_res->setZeroOrMoreRootGroup();
    resIn_res->addGroupModel("", DEFin_grp, CT_AbstractItemGroup::staticGetType(), tr("Scènes (grp)"));
    resIn_res->addItemModel(DEFin_grp, DEFin_scene, CT_AbstractItemDrawableWithPointCloud::staticGetType(), tr("Scène"));

}

// Creation and affiliation of OUT models
void ONF_StepDetectVerticalAlignments02::createOutResultModelListProtected()
{
    CT_OutResultModelGroupToCopyPossibilities *resCpy = createNewOutResultModelToCopy(DEFin_res);

    resCpy->addGroupModel(DEFin_grp, _grpCluster_ModelName, new CT_StandardItemGroup(), tr("Clusters conservés"));
    resCpy->addItemModel(_grpCluster_ModelName, _cluster_ModelName, new CT_PointCluster(), tr("Cluster conservé"));
    resCpy->addItemModel(_grpCluster_ModelName, _line_ModelName, new CT_Line(), tr("Ligne conservée"));
    resCpy->addItemModel(_grpCluster_ModelName, _convexProj_ModelName, new CT_Polygon2D(), tr("Enveloppe convexe projetée"));
    resCpy->addItemModel(_grpCluster_ModelName, _circle_ModelName, new CT_Circle2D(), tr("Diamètre Estimé"));
    resCpy->addItemAttributeModel(_line_ModelName, _attMin_ModelName, new CT_StdItemAttributeT<double>(CT_AbstractCategory::DATA_VALUE), tr("DistMin"));
    resCpy->addItemAttributeModel(_line_ModelName, _attQ25_ModelName, new CT_StdItemAttributeT<double>(CT_AbstractCategory::DATA_VALUE), tr("DistQ25"));
    resCpy->addItemAttributeModel(_line_ModelName, _attQ50_ModelName, new CT_StdItemAttributeT<double>(CT_AbstractCategory::DATA_VALUE), tr("DistMed"));
    resCpy->addItemAttributeModel(_line_ModelName, _attQ75_ModelName, new CT_StdItemAttributeT<double>(CT_AbstractCategory::DATA_VALUE), tr("DistQ75"));
    resCpy->addItemAttributeModel(_line_ModelName, _attMax_ModelName, new CT_StdItemAttributeT<double>(CT_AbstractCategory::DATA_VALUE), tr("DistMax"));
    resCpy->addItemAttributeModel(_line_ModelName, _attMean_ModelName, new CT_StdItemAttributeT<double>(CT_AbstractCategory::DATA_VALUE), tr("DistMean"));
    resCpy->addItemAttributeModel(_line_ModelName, _attDiamEq_ModelName, new CT_StdItemAttributeT<double>(CT_AbstractCategory::DATA_VALUE), tr("DiamEq"));

    resCpy->addGroupModel(DEFin_grp, _grpDroppedCluster_ModelName, new CT_StandardItemGroup(), tr("Clusters éliminés"));
    resCpy->addItemModel(_grpDroppedCluster_ModelName, _droppedCluster_ModelName, new CT_PointCluster(), tr("Cluster éliminé"));
    resCpy->addItemModel(_grpDroppedCluster_ModelName, _droppedLine_ModelName, new CT_Line(), tr("Ligne éliminée"));
    resCpy->addItemModel(_grpDroppedCluster_ModelName, _convexProjDropped_ModelName, new CT_Polygon2D(), tr("Enveloppe convexe projetée éliminée"));

}

// Semi-automatic creation of step parameters DialogBox
void ONF_StepDetectVerticalAlignments02::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addTitle( tr("1- Paramètres de validation des droites candidates :"));
    configDialog->addDouble(tr("Distance maximum entre deux points d'une droite candidate"), "m", 0, 1000, 2, _pointDistThreshold);
    configDialog->addDouble(tr("Angle zénithal maximal pour une droite candidate"), "°", 0, 180, 2, _maxPhiAngle);

    configDialog->addEmpty();
    configDialog->addTitle(tr("2- Paramètres de création des clusters (à partir des droites candidates) :"));
    configDialog->addDouble(tr("Distance maximum XY entre deux droites candidates à agréger"), "m", 0, 1000, 2, _lineDistThreshold);
    configDialog->addDouble(tr("Eloignement maximal sur 1 m en Z entre deux droites candidates à agréger"), "m", 0, 1000, 2, _maxSpacing);
    configDialog->addInt(   tr("Nombre de points minimum dans un cluster"), "", 3, 1000, _minPtsNb);
    configDialog->addDouble(tr("Pourcentage maximum de la longueur de segment sans points"), "%", 0, 100, 0, _lineLengthRatio, 100);

    configDialog->addEmpty();
    configDialog->addTitle(tr("3- Paramètres de validation des clusters obtenus :"));
    configDialog->addDouble(tr("Supprimer les clusters dont la longueur est inférieure à"), "m", 0, 1000, 2, _lengthThreshold);
    configDialog->addDouble(tr("Supprimer les clusters qui commence au dessus de "), "% de Hscene", 0, 100, 0, _heightThreshold, 100);
    configDialog->addDouble(tr("Epaisseur des cercles pour le scoring"), "cm", 0, 99999, 2, _circleDistThreshold, 100);

    configDialog->addEmpty();
    configDialog->addBool(tr("Mode Debug Clusters"), "", "", _clusterDebugMode);
}



void ONF_StepDetectVerticalAlignments02::compute()
{
    QList<CT_ResultGroup*> outResultList = getOutResultList();
    CT_ResultGroup* res = outResultList.at(0);

    QList<CT_StandardItemGroup*> groups;
    // COPIED results browsing
    CT_ResultGroupIterator itCpy_grp(res, this, DEFin_grp);
    while (itCpy_grp.hasNext() && !isStopped())
    {
        CT_StandardItemGroup* grp = (CT_StandardItemGroup*) itCpy_grp.next();
        groups.append(grp);
    }

    QFuture<void> futur = QtConcurrent::map(groups, AlignmentsDetectorForScene(this, res));

    int progressMin = futur.progressMinimum();
    int progressTotal = futur.progressMaximum() - futur.progressMinimum();
    while (!futur.isFinished())
    {
        setProgress(100.0*(futur.progressValue() - progressMin)/progressTotal);
    }

}


void ONF_StepDetectVerticalAlignments02::AlignmentsDetectorForScene::detectAlignmentsForScene(CT_StandardItemGroup* grp)
{

    const CT_AbstractItemDrawableWithPointCloud* scene = (CT_AbstractItemDrawableWithPointCloud*)grp->firstItemByINModelName(_step, DEFin_scene);
    if (scene != NULL)
    {
        double maxPhiRadians = M_PI*_step->_maxPhiAngle/180.0;

        QList<ONF_StepDetectVerticalAlignments02::LineData*> candidateLines;
        const CT_AbstractPointCloudIndex* pointCloudIndex = scene->getPointCloudIndex();

        //double deltaZ = scene->maxZ() - scene->minZ();
        double hMax = (scene->maxZ() - scene->minZ())*_step->_heightThreshold + scene->minZ();

        // Parcours tous les couples de points 2 à deux
        CT_PointIterator itP1(pointCloudIndex);
        while(itP1.hasNext() && (!_step->isStopped()))
        {
            const CT_Point &point1 = itP1.next().currentPoint();
            size_t index1 = itP1.currentGlobalIndex();

            bool before = true;
            CT_PointIterator itP2(pointCloudIndex);
            while(itP2.hasNext() && (!_step->isStopped()))
            {
                const CT_Point &point2 = itP2.next().currentPoint();
                size_t index2 = itP2.currentGlobalIndex();

                // Les deux points doivent être disctinct dans l'espace
                if (!before && ((point1(0) != point2(0) || point1(1) != point2(1) || point1(2) != point2(2))))
                {
                    // Les deux points doivent être à moins de _distThreshold
                    double dist = sqrt(pow(point1(0) - point2(0), 2) + pow(point1(1) - point2(1), 2) + pow(point1(2) - point2(2), 2));
                    if (dist < _step->_pointDistThreshold)
                    {
                        Eigen::Vector3d pointLow  = point1;
                        Eigen::Vector3d pointHigh = point2;

                        if (point2(2) < point1(2))
                        {
                            pointLow  = point2;
                            pointHigh = point1;
                        }

                        float phi, theta, length;
                        CT_SphericalLine3D::convertToSphericalCoordinates(&pointLow, &pointHigh, phi, theta, length);

                        if (phi < maxPhiRadians)
                        {
                            candidateLines.append(new ONF_StepDetectVerticalAlignments02::LineData(pointLow, pointHigh, index1, index2, phi, scene->minZ(), scene->maxZ()));
                        }
                    }
                }

                if (index1 == index2) {before = false;}
            }
        }

        // Tri par Phi croissant... inutile
        //qSort(candidateLines.begin(), candidateLines.end(), ONF_StepDetectVerticalAlignments02::orderByAscendingPhi);

        // Affiliation des lignes proches deux à deux
        for (int i1 = 0 ; i1 < candidateLines.size() ; i1++)
        {
            ONF_StepDetectVerticalAlignments02::LineData* line1 = candidateLines.at(i1);

            for (int i2 = i1+1 ; i2 < candidateLines.size() ; i2++)
            {
                ONF_StepDetectVerticalAlignments02::LineData* line2 = candidateLines.at(i2);

                double distLow = sqrt(pow(line1->_lowCoord(0) - line2->_lowCoord(0), 2) + pow(line1->_lowCoord(1) - line2->_lowCoord(1), 2));

                if (distLow < _step->_lineDistThreshold)
                {
                    double distHigh = sqrt(pow(line1->_highCoord(0) - line2->_highCoord(0), 2) + pow(line1->_highCoord(1) - line2->_highCoord(1), 2));
                    //double spacing = fabs(distHigh - distLow);

                    if (distHigh < _step->_lineDistThreshold)
                    {
                        double maxDist = std::max(distHigh, distLow);
                        line1->_neighbors.append(line2);
                        line1->_distSum += maxDist;

                        line2->_neighbors.append(line1);
                        line2->_distSum += maxDist;
                    }
                }
            }
        }

        // Tri par NeighborCount descendant
        qSort(candidateLines.begin(), candidateLines.end(), ONF_StepDetectVerticalAlignments02::orderByDescendingNeighborCount);

        // Constitution des clusters de points alignés
        QMap<CT_LineData*, CT_PointCluster*> pointsClusters;
        QList<size_t> insertedPoints;

        for (int i = 0 ; i < candidateLines.size() ; i++)
        {
            ONF_StepDetectVerticalAlignments02::LineData* candidateLine = candidateLines.at(i);
            CT_PointCluster* cluster = new CT_PointCluster(_step->_cluster_ModelName.completeName(), _res);

            if (!candidateLine->_processed)
            {
                if (!insertedPoints.contains(candidateLine->_index1))
                {
                    cluster->addPoint(candidateLine->_index1);
                    insertedPoints.append(candidateLine->_index1);
                }
                if (!insertedPoints.contains(candidateLine->_index2))
                {
                    cluster->addPoint(candidateLine->_index2);
                    insertedPoints.append(candidateLine->_index2);
                }
                candidateLine->_processed = true;

                QList<ONF_StepDetectVerticalAlignments02::LineData*> &neighborLines = candidateLine->_neighbors;
                for (int j = 0 ; j < neighborLines.size() ; j++)
                {
                    ONF_StepDetectVerticalAlignments02::LineData* neighborLine = neighborLines.at(j);
                    if (!neighborLine->_processed)
                    {
                        if (!insertedPoints.contains(neighborLine->_index1))
                        {
                            cluster->addPoint(neighborLine->_index1);
                            insertedPoints.append(neighborLine->_index1);
                        }
                        if (!insertedPoints.contains(neighborLine->_index2))
                        {
                            cluster->addPoint(neighborLine->_index2);
                            insertedPoints.append(neighborLine->_index2);
                        }
                        neighborLine->_processed = true;
                    }
                }

                const CT_AbstractPointCloudIndex* cloudIndex = cluster->getPointCloudIndex();
                size_t nbPts = cloudIndex->size();

                if (nbPts >= _step->_minPtsNb)
                {
                    CT_LineData* lineData = CT_LineData::staticCreateLineDataFromPointCloud(*cloudIndex);
                    pointsClusters.insert(lineData, cluster);

                    float phi, theta, length;
                    CT_SphericalLine3D::convertToSphericalCoordinates(&(lineData->getP1()), &(lineData->getP2()), phi, theta, length);

                    if (phi > maxPhiRadians)
                    {
                        if (_step->_clusterDebugMode) // mode Debug Cluster
                        {
                            CT_StandardItemGroup* grpClDropped = new CT_StandardItemGroup(_step->_grpDroppedCluster_ModelName.completeName(), _res);
                            grp->addGroup(grpClDropped);

                            cluster->setModel(_step->_droppedCluster_ModelName.completeName());
                            grpClDropped->addItemDrawable(cluster);

                            CT_Line* line = new CT_Line(_step->_droppedLine_ModelName.completeName(), _res, lineData);
                            grpClDropped->addItemDrawable(line);
                        } else {
                            delete cluster;
                            delete lineData;
                        }
                    } else {

                        QList<double> dists;

                        CT_PointIterator itP(cloudIndex);
                        while(itP.hasNext())
                        {
                            const CT_Point &point = itP.next().currentPoint();

                            Eigen::Vector3d projPoint;
                            dists.append(CT_MathPoint::distanceOnLineForPointProjection(lineData->getP1(), lineData->getDirection(), point, projPoint));
                        }

                        qSort(dists);

                        bool okLength = true;
                        double maxLength = _step->_lineLengthRatio * lineData->length();
                        for (int dd = 1 ; dd < dists.size() && okLength; dd++)
                        {
                            double currentDist = dists.at(dd) - dists.at(dd - 1);
                            if (currentDist > maxLength) {okLength = false;}
                        }
                        dists.clear();

                        if (okLength)
                        {
                            if (_step->_clusterDebugMode) // mode Debug Cluster
                            {
                                CT_StandardItemGroup* grpClKept = new CT_StandardItemGroup(_step->_grpCluster_ModelName.completeName(), _res);
                                grp->addGroup(grpClKept);

                                cluster->setModel(_step->_cluster_ModelName.completeName());
                                grpClKept->addItemDrawable(cluster);

                                CT_Line* line = new CT_Line(_step->_line_ModelName.completeName(), _res, lineData);
                                grpClKept->addItemDrawable(line);
                            }
                        } else {
                            if (_step->_clusterDebugMode) // mode Debug Cluster
                            {
                                CT_StandardItemGroup* grpClDropped = new CT_StandardItemGroup(_step->_grpDroppedCluster_ModelName.completeName(), _res);
                                grp->addGroup(grpClDropped);

                                cluster->setModel(_step->_droppedCluster_ModelName.completeName());
                                grpClDropped->addItemDrawable(cluster);

                                CT_Line* line = new CT_Line(_step->_droppedLine_ModelName.completeName(), _res, lineData);
                                grpClDropped->addItemDrawable(line);
                            } else {
                                delete cluster;
                                delete lineData;
                            }
                        }

                    }
                } else {
                    if (_step->_clusterDebugMode) // mode Debug Cluster
                    {
                        CT_StandardItemGroup* grpClDropped = new CT_StandardItemGroup(_step->_grpDroppedCluster_ModelName.completeName(), _res);
                        grp->addGroup(grpClDropped);

                        cluster->setModel(_step->_droppedCluster_ModelName.completeName());
                        grpClDropped->addItemDrawable(cluster);
                    } else {
                        delete cluster;
                    }
                }
            }
        }

        qDeleteAll(candidateLines);
        candidateLines.clear();
        insertedPoints.clear();

        if (_step->_clusterDebugMode) {return;}


//        // Regroupement des clusters par proximité
//        // Affiliation des clusters proches deux à deux
//        QMultiMap<CT_PointCluster*, CT_PointCluster*> clusterPairs;
//        QMultiMap<double, CT_PointCluster*> clusterLengths;

//        QList<CT_PointCluster*> allClusters = pointsClusters.keys();
//        for (int i1 = 0 ; i1 < allClusters.size() ; i1++)
//        {
//            CT_PointCluster* cluster1 = allClusters.at(i1);
//            CT_LineData*        line1 = pointsClusters.value(cluster1);
//            const CT_AbstractPointCloudIndex* cloudIndex = cluster1->getPointCloudIndex();

//            clusterLengths.insert(line1->length(), cluster1);

//            for (int i2 = 0 ; i2 < allClusters.size(); i2++)
//            {
//                if (i1 != i2)
//                {
//                    CT_PointCluster* cluster2 = allClusters.at(i2);
//                    CT_LineData*        line2 = pointsClusters.value(cluster2);
//                    ONF_StepDetectVerticalAlignments02::DistValues* dv2 = distValues.value(cluster2);

//                    const Eigen::Vector3d &dir2 = line2->getDirection();
//                    const Eigen::Vector3d &pt2 = line2->getP1();

//                    bool proximity = false;
//                    CT_PointIterator it(cloudIndex);
//                    while(it.hasNext() && !proximity)
//                    {
//                        it.next();
//                        const CT_Point &p = it.currentPoint();
//                        double distToLine = CT_MathPoint::distancePointLine(p, dir2, pt2);

//                        if (distToLine < dv2->_max)
//                        {
//                            proximity = true;
//                        }
//                    }

//                    if (proximity)
//                    {
//                        if (!clusterPairs.contains(cluster1, cluster2)) {clusterPairs.insert(cluster1, cluster2);}
//                        if (!clusterPairs.contains(cluster2, cluster1)) {clusterPairs.insert(cluster2, cluster1);}
//                    }
//                }

//            }
//        }

//        // inventaire des clusters à éliminer pour cause proximité
//        QList<CT_PointCluster*> toRemoveBecauseOfProximity;
//        QList<CT_PointCluster*> processedClusters;
//        QMapIterator<double, CT_PointCluster*> itLe(clusterLengths);
//        itLe.toBack();
//        while (itLe.hasPrevious())
//        {
//            itLe.previous();
//            CT_PointCluster* cluster = itLe.value();
//            processedClusters.append(cluster);

//            QList<CT_PointCluster*> toRemove = clusterPairs.values(cluster);
//            for (int i = 0 ; i < toRemove.size() ; i++)
//            {
//                CT_PointCluster* cli = toRemove.at(i);

//                if (!processedClusters.contains(cli))
//                {
//                    QList<CT_PointCluster*> toRemove2 = clusterPairs.values(cli);
//                    for (int j = 0 ; j < toRemove2.size() ; j++)
//                    {
//                        CT_PointCluster* clj = toRemove2.at(j);
//                        if (!toRemove.contains(clj) && !processedClusters.contains(clj)) {toRemove.append(clj);}
//                    }

//                    toRemoveBecauseOfProximity.append(cli);
//                }
//            }
//        }

//        // Création des pointClusters
//        QMapIterator<CT_PointCluster*, CT_LineData* > itCl2(pointsClusters);
//        while (itCl2.hasNext())
//        {
//            itCl2.next();
//            CT_PointCluster* cluster = itCl2.key();
//            CT_LineData* lineData = itCl2.value();
//            ONF_StepDetectVerticalAlignments02::DistValues* distVal = distValues.value(cluster);

//            if (lineData != NULL && distVal != NULL)
//            {

//                // Calcul de l'angle Phi
//                Eigen::Vector3d pointLow  = lineData->getP1();
//                Eigen::Vector3d pointHigh = lineData->getP2();

//                if (lineData->getP1()(2) < lineData->getP2()(2))
//                {
//                    pointLow  = lineData->getP2();
//                    pointHigh = lineData->getP1();
//                }

//                float phi, theta, length;
//                CT_SphericalLine3D::convertToSphericalCoordinates(&pointHigh, &pointLow, phi, theta, length);

//                const CT_AbstractPointCloudIndex* cloudIndex = cluster->getPointCloudIndex();
//                size_t nbPts = cloudIndex->size();

//                // Enveloppe convexe des points projetés perpendiculairement à la ligne
//                QList<Eigen::Vector2d*> projPts;
//                Eigen::Hyperplane<double, 3> plane(lineData->getDirection(), lineData->getP1());
//                CT_PointIterator itCI(cloudIndex);
//                while(itCI.hasNext())
//                {
//                    itCI.next();
//                    const CT_Point &pt = itCI.currentPoint();

//                    Eigen::Vector3d projectedPt = plane.projection(pt);
//                    projPts.append(new Eigen::Vector2d(projectedPt(0), projectedPt(1)));
//                }

//                CT_Polygon2DData::orderPointsByXY(projPts);
//                CT_Polygon2DData* polyData = CT_Polygon2DData::createConvexHull(projPts);
//                CT_Polygon2D* poly = NULL;
//                if (polyData != NULL)
//                {
//                    poly = new CT_Polygon2D(_step->_convexProj_ModelName.completeName(), _res, polyData);
//                    poly->setZValue(lineData->getP1()(2));
//                }



//                // Test de validité pour le cluster
//                if (    nbPts >= _step->_minPtsNb &&                        // Nombre de points
//                        phi < maxPhiRadians &&                              // Verticalité
//                        lineData->length() > _step->_lengthThreshold &&     // Longueur
//                        cluster->minZ() < hMax &&                           // Hauteur de base
//                        !toRemoveBecauseOfProximity.contains(cluster)  )    // Proximité
//                {

//                    // Compute diameter of the stem (using max distance between two projected points)
//                    QMultiMap<int, float> diamEqs;
//                    for (int ii = 0 ; ii < projPts.size() ; ii++)
//                    {
//                        const Eigen::Vector2d *pt1 = projPts.at(ii);
//                        for (int jj = ii + 1 ; jj < projPts.size() ; jj++)
//                        {
//                            const Eigen::Vector2d *pt2 = projPts.at(jj);

//                            float dist = sqrt(pow((*pt1)(0) - (*pt2)(0), 2) + pow((*pt1)(1) - (*pt2)(1), 2));

//                            Eigen::Vector2d center;
//                            center(0) = ((*pt1)(0) + (*pt2)(0)) / 2.0;
//                            center(1) = ((*pt1)(1) + (*pt2)(1)) / 2.0;

//                            float nb = 0;
//                            for (int kk = 0 ; kk < projPts.size(); kk++)
//                            {
//                                if (kk != ii && kk != jj)
//                                {
//                                    const Eigen::Vector2d *pt3 = projPts.at(kk);
//                                    float distCircle = sqrt(pow(center(0) - (*pt3)(0), 2) + pow(center(1) - (*pt3)(1), 2)) - (dist / 2.0);
//                                    float absDistCircle = fabs(distCircle);

//                                    if (absDistCircle > _step->_circleDistThreshold && distCircle < 0)
//                                    {
//                                        nb -= 1;
//                                    } else {
//                                        float ratio = _step->_circleDistThreshold / absDistCircle;
//                                        if (ratio > 1) {ratio = 1;}
//                                        nb += ratio;
//                                    }
//                                }
//                            }

//                            diamEqs.insert(nb, dist);
//                        }
//                    }

//                    double diamEq = 0;
//                    if (diamEqs.size() > 0)
//                    {
//                        diamEq = diamEqs.last();
//                        diamEqs.clear();
//                    }

//                    // Création des items
//                    CT_StandardItemGroup* grpCl = new CT_StandardItemGroup(_step->_grpCluster_ModelName.completeName(), _res);
//                    grp->addGroup(grpCl);
//                    grpCl->addItemDrawable(cluster);
//                    if (polyData != NULL)
//                    {
//                        grpCl->addItemDrawable(poly);
//                    }

//                    CT_Line* line = new CT_Line(_step->_line_ModelName.completeName(), _res, lineData);
//                    grpCl->addItemDrawable(line);

//                    if (diamEq > 0)
//                    {
//                        Eigen::Vector2d center;
//                        center(0) = line->getP1_X();
//                        center(1) = line->getP1_Y();

//                        CT_Circle2D *circle = new CT_Circle2D(_step->_circle_ModelName.completeName(), _res, new CT_Circle2DData(center, diamEq/2.0));
//                        grpCl->addItemDrawable(circle);
//                    }


//                    line->addItemAttribute(new CT_StdItemAttributeT<double>(_step->_attMin_ModelName.completeName(),
//                                                                               CT_AbstractCategory::DATA_VALUE,
//                                                                               _res,
//                                                                               distVal->_min));
//                    line->addItemAttribute(new CT_StdItemAttributeT<double>(_step->_attQ25_ModelName.completeName(),
//                                                                               CT_AbstractCategory::DATA_VALUE,
//                                                                               _res,
//                                                                               distVal->_q25));
//                    line->addItemAttribute(new CT_StdItemAttributeT<double>(_step->_attQ50_ModelName.completeName(),
//                                                                               CT_AbstractCategory::DATA_VALUE,
//                                                                               _res,
//                                                                               distVal->_q50));
//                    line->addItemAttribute(new CT_StdItemAttributeT<double>(_step->_attQ75_ModelName.completeName(),
//                                                                               CT_AbstractCategory::DATA_VALUE,
//                                                                               _res,
//                                                                               distVal->_q75));
//                    line->addItemAttribute(new CT_StdItemAttributeT<double>(_step->_attMax_ModelName.completeName(),
//                                                                               CT_AbstractCategory::DATA_VALUE,
//                                                                               _res,
//                                                                               distVal->_max));
//                    line->addItemAttribute(new CT_StdItemAttributeT<double>(_step->_attMean_ModelName.completeName(),
//                                                                               CT_AbstractCategory::DATA_VALUE,
//                                                                               _res,
//                                                                               distVal->_mean));

//                    line->addItemAttribute(new CT_StdItemAttributeT<double>(_step->_attDiamEq_ModelName.completeName(),
//                                                                               CT_AbstractCategory::DATA_VALUE,
//                                                                               _res,
//                                                                               diamEq));

//                } else {
//                    CT_StandardItemGroup* grpClDropped = new CT_StandardItemGroup(_step->_grpDroppedCluster_ModelName.completeName(), _res);
//                    grp->addGroup(grpClDropped);

//                    cluster->setModel(_step->_droppedCluster_ModelName.completeName());
//                    grpClDropped->addItemDrawable(cluster);

//                    if (polyData != NULL)
//                    {
//                        poly->setModel(_step->_convexProjDropped_ModelName.completeName());
//                        grpClDropped->addItemDrawable(poly);
//                    }

//                    CT_Line* line = new CT_Line(_step->_droppedLine_ModelName.completeName(), _res, lineData);
//                    grpClDropped->addItemDrawable(line);
//                }

//                qDeleteAll(projPts);

//            } else {
//                delete cluster;
//                qDebug() << "Problème";
//            }

//        }

    }
}

ONF_StepDetectVerticalAlignments02::DistValues* ONF_StepDetectVerticalAlignments02::AlignmentsDetectorForScene::computeDistVals(const CT_AbstractPointCloudIndex* cloudIndex, CT_LineData* lineData)
{
    ONF_StepDetectVerticalAlignments02::DistValues* distVal = new ONF_StepDetectVerticalAlignments02::DistValues();

    size_t nbPts = cloudIndex->size();

    distVal->_min  = std::numeric_limits<double>::max();
    distVal->_max  = 0;
    distVal->_mean = 0;

    QList<double> distances;

    const Eigen::Vector3d &dir = lineData->getDirection();

    CT_PointIterator it(cloudIndex);
    while(it.hasNext())
    {
        it.next();
        const CT_Point &p = it.currentPoint();
        double distToLine = CT_MathPoint::distancePointLine(p, dir, lineData->getP1());
        distances.append(distToLine);

        distVal->_mean += distToLine;
        if (distToLine < distVal->_min) {distVal->_min = distToLine;}
        if (distToLine > distVal->_max) {distVal->_max = distToLine;}
    }

    distVal->_mean /= nbPts;

    distVal->_q25 = CT_MathStatistics::computeQuantile(distances, 0.25, true);
    distVal->_q50 = CT_MathStatistics::computeQuantile(distances, 0.50, true);
    distVal->_q75 = CT_MathStatistics::computeQuantile(distances, 0.75, true);

    return distVal;
}
