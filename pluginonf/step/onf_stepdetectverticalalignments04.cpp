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

#include "onf_stepdetectverticalalignments04.h"

#include "ct_itemdrawable/abstract/ct_abstractitemdrawablewithpointcloud.h"
#include "ct_itemdrawable/ct_polygon2d.h"
#include "ct_itemdrawable/ct_circle2d.h"
#include "ct_itemdrawable/ct_line.h"
#include "ct_itemdrawable/ct_pointcluster.h"
#include "ct_itemdrawable/abstract/ct_abstractpointattributesscalar.h"
#include "ct_itemdrawable/ct_standarditemgroup.h"

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
#define DEFin_sceneStem "sceneStem"
#define DEFin_sceneAll "sceneAll"
#define DEFin_attLineOfScan "lineOfScan"
#define DEFin_attGPSTime "gpstime"



// Constructor : initialization of parameters
ONF_StepDetectVerticalAlignments04::ONF_StepDetectVerticalAlignments04(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _thresholdGPSTime = 1e-5;
    _maxCurvature = 0.25;
    _thresholdZenithalAngle = 30.0;
    _minPts = 2;

    _curvatureMultiplier = 5.0;
    _nbPointDistStep = 0.20;
    _maxMergingDist = 1.5;

    _DBH_resAzimZeni = 1;
    _DBH_zeniMax = 20;

    _pointDistThresholdSmall = 4.0;
    _maxPhiAngleSmall = 20.0;
    _lineDistThresholdSmall = 0.4;
    _minPtsSmall = 3;
    _lineLengthRatioSmall = 0.8;
    _exclusionRadiusSmall = 1.5;

    _ratioDbhNbptsMax = 0.07;
    _dbhMin = 0.075;
    _dbhMax = 0.275;
    _nbPtsForDbhMax = 10;

    _clusterDebugMode = false;
}

// Step description (tooltip of contextual menu)
QString ONF_StepDetectVerticalAlignments04::getStepDescription() const
{
    return tr("Détecter des alignements verticaux de points (V4)");
}

// Step detailled description
QString ONF_StepDetectVerticalAlignments04::getStepDetailledDescription() const
{
    return tr("No detailled description for this step");
}

// Step URL
QString ONF_StepDetectVerticalAlignments04::getStepURL() const
{
    //return tr("STEP URL HERE");
    return CT_AbstractStep::getStepURL(); //by default URL of the plugin
}

// Step copy method
CT_VirtualAbstractStep* ONF_StepDetectVerticalAlignments04::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new ONF_StepDetectVerticalAlignments04(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void ONF_StepDetectVerticalAlignments04::createInResultModelListProtected()
{
    CT_InResultModelGroupToCopy *resIn_res = createNewInResultModelForCopy(DEFin_res, tr("Scènes"));
    resIn_res->setZeroOrMoreRootGroup();
    resIn_res->addGroupModel("", DEFin_grp, CT_AbstractItemGroup::staticGetType(), tr("Scènes (grp)"));
    resIn_res->addItemModel(DEFin_grp, DEFin_sceneStem, CT_AbstractItemDrawableWithPointCloud::staticGetType(), tr("Scène (tiges)"));
    resIn_res->addItemModel(DEFin_grp, DEFin_attGPSTime, CT_AbstractPointAttributesScalar::staticGetType(), tr("Temps GPS"), tr("Attribut codant le temps GPS"));

    resIn_res->addItemModel(DEFin_grp, DEFin_sceneAll, CT_AbstractItemDrawableWithPointCloud::staticGetType(), tr("Scène (complète)"), "",
                            CT_InAbstractModel::C_ChooseOneIfMultiple, CT_InAbstractModel::F_IsOptional);
}

// Creation and affiliation of OUT models
void ONF_StepDetectVerticalAlignments04::createOutResultModelListProtected()
{
    CT_OutResultModelGroupToCopyPossibilities *resCpy = createNewOutResultModelToCopy(DEFin_res);

    resCpy->addGroupModel(DEFin_grp, _grpCluster_ModelName, new CT_StandardItemGroup(), tr("Tiges"));
    resCpy->addItemModel(_grpCluster_ModelName, _cluster_ModelName, new CT_PointCluster(), tr("Cluster"));
    resCpy->addItemAttributeModel(_cluster_ModelName, _attMaxDistXY_ModelName, new CT_StdItemAttributeT<double>(CT_AbstractCategory::DATA_VALUE), tr("Diamètre"));
    resCpy->addItemAttributeModel(_cluster_ModelName, _attStemType_ModelName, new CT_StdItemAttributeT<int>(CT_AbstractCategory::DATA_VALUE), tr("Type"), tr("0 = petite tige ; 1 = grosse tige"));
    resCpy->addItemModel(_grpCluster_ModelName, _circle_ModelName, new CT_Circle2D(), tr("Diamètre"));
    resCpy->addItemModel(_grpCluster_ModelName, _line_ModelName, new CT_PointCluster(), tr("Droite ajustée"));

    if (_clusterDebugMode)
    {
        resCpy->addGroupModel(DEFin_grp, _grpClusterDebug_ModelName, new CT_StandardItemGroup(), tr("Debug"));
        resCpy->addItemModel(_grpClusterDebug_ModelName, _clusterDebug_ModelName, new CT_PointCluster(), tr("Lignes de scan (toutes)"));
        resCpy->addGroupModel(DEFin_grp, _grpClusterDebug2_ModelName, new CT_StandardItemGroup(), tr("Debug"));
        resCpy->addItemModel(_grpClusterDebug2_ModelName, _clusterDebug2_ModelName, new CT_PointCluster(), tr("Lignes de scan (conservées)"));
    }
}

// Semi-automatic creation of step parameters DialogBox
void ONF_StepDetectVerticalAlignments04::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addTitle( tr("1- Détéction des lignes de scan :"));
    configDialog->addDouble(tr("Seuil de temps GPS pour changer de ligne de scan"), "m", 0, 1e+10, 10, _thresholdGPSTime);
    configDialog->addDouble(tr("Courbure maximale d'une ligne de scan"), "cm/m", 0, 1e+4, 2, _maxCurvature, 100);
    configDialog->addDouble(tr("Angle zénithal maximal conserver une ligne de scan"), "°", 0, 360, 2, _thresholdZenithalAngle);
    configDialog->addInt(tr("Ne conserver que les lignes de scan avec au moins"), "points", 0, 1000, _minPts);

    configDialog->addEmpty();
    configDialog->addTitle( tr("2- Détéction des grosses tiges (fusion des lignes de scan adjacentes) :"));
    configDialog->addDouble(tr("Multiplicateur pour la courbure"), "fois", 0, 100, 2, _curvatureMultiplier);
    configDialog->addDouble(tr("Increment en distance par point"), "m", 0, 1e+4, 2, _nbPointDistStep);
    configDialog->addDouble(tr("Distance de recherche maximum"), "m", 0, 1e+4, 2, _maxMergingDist);

    configDialog->addEmpty();
    configDialog->addTitle( tr("3- Détéction de la direction des grosses tiges:"));
    configDialog->addDouble(tr("Résolution en azimuth / angle zénithal"), "°", 0, 90, 4, _DBH_resAzimZeni);
    configDialog->addDouble(tr("Angle zénithal maximal"), "°", 0, 90, 2, _DBH_zeniMax);

    configDialog->addEmpty();
    configDialog->addTitle( tr("4- Détéction des petites tiges (alignements) :"));
    configDialog->addDouble(tr("Distance maximum entre deux points d'une droite candidate"), "m", 0, 1000, 2, _pointDistThresholdSmall);
    configDialog->addDouble(tr("Angle zénithal maximal pour une droite candidate"), "°", 0, 180, 2, _maxPhiAngleSmall);
    configDialog->addDouble(tr("Distance maximum XY entre deux droites candidates à agréger"), "m", 0, 1000, 2, _lineDistThresholdSmall);
    configDialog->addInt(tr("Nombre de points minimum dans un cluster"), "", 2, 1000, _minPtsSmall);
    configDialog->addDouble(tr("Pourcentage maximum de la longueur de segment sans points"), "%", 0, 100, 0, _lineLengthRatioSmall, 100);
    configDialog->addDouble(tr("Rayon d'exclusion autour des grosses tiges"), "m", 0, 1e+4, 2, _exclusionRadiusSmall);

    configDialog->addEmpty();
    configDialog->addTitle( tr("5- Estimation des diamètres :"));
    configDialog->addDouble(tr("Ratio maximal diamètre / nb. points"), "cm", 0, 1e+4, 2, _ratioDbhNbptsMax, 100);
    configDialog->addDouble(tr("Diamètre minimal"), "cm", 0, 1e+4, 2, _dbhMin, 100);
    configDialog->addDouble(tr("Diamètre maximal des \"petites tiges\""), "cm", 0, 1e+4, 2, _dbhMax, 100);
    configDialog->addInt(tr("Nombre de points équivalents au diamètre maximal"), "points", 2, 1e+4, _nbPtsForDbhMax);

    configDialog->addEmpty();
    configDialog->addBool(tr("Mode Debug Clusters"), "", "", _clusterDebugMode);
}



void ONF_StepDetectVerticalAlignments04::compute()
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

void ONF_StepDetectVerticalAlignments04::AlignmentsDetectorForScene::detectAlignmentsForScene(CT_StandardItemGroup* grp)
{
    CT_PointAccessor pointAccessor;
    double thresholdZenithalAngleRadians = M_PI * _step->_thresholdZenithalAngle / 180.0;

    const CT_AbstractItemDrawableWithPointCloud* sceneStem = (CT_AbstractItemDrawableWithPointCloud*)grp->firstItemByINModelName(_step, DEFin_sceneStem);
    const CT_AbstractPointAttributesScalar* attributeGPSTime = (CT_AbstractPointAttributesScalar*)grp->firstItemByINModelName(_step, DEFin_attGPSTime);

    if (sceneStem != NULL && attributeGPSTime != NULL)
    {
        const CT_AbstractPointCloudIndex* pointCloudIndex = sceneStem->getPointCloudIndex();

        ////////////////////////////////////////////////////
        /// Detection of big stems: lines of scan        ///
        ////////////////////////////////////////////////////

        // Sort indices by GPS time
        QMultiMap<double, size_t> sortedIndices;
        CT_PointIterator itP(pointCloudIndex);
        while(itP.hasNext() && (!_step->isStopped()))
        {
            size_t index = itP.next().currentGlobalIndex();

            const CT_AbstractPointCloudIndex* pci_att = attributeGPSTime->getPointCloudIndex();
            size_t localIndex = pci_att->indexOf(index);

            double gpsTime = 0; // Récupération de la ligne de scan pour le point 1
            if (localIndex < pci_att->size())
            {
                gpsTime = attributeGPSTime->dValueAt(localIndex);
                sortedIndices.insert(gpsTime, index);
            }
        }

        // List of not attributes points
        QList<size_t> isolatedPointIndices;

        QList<size_t> currentIndexList;
        QList<QList<size_t> > linesOfScan;

        // Creation des lignes de scan
        double lastGPSTime = -std::numeric_limits<double>::max();
        QMapIterator<double, size_t> itMapSorted(sortedIndices);
        while (itMapSorted.hasNext())
        {
            itMapSorted.next();
            double gpsTime = itMapSorted.key();
            size_t index = itMapSorted.value();

            double delta = gpsTime - lastGPSTime;
            lastGPSTime = gpsTime;

            if (delta < _step->_thresholdGPSTime)
            {
                // add point to current line of scan
                currentIndexList.append(index);
            } else {
                // init a new line of scan
                if (currentIndexList.size() > 1)
                {
                    linesOfScan.append(currentIndexList);
                } else {
                    if (!currentIndexList.isEmpty()) {isolatedPointIndices.append(currentIndexList.first());}
                }
                currentIndexList.clear();
                // then add the point
                currentIndexList.append(index);
            }
        }
        sortedIndices.clear();

        // Eliminate noise in lines of scan
        QList<CT_PointCluster*> scanLineClusters;
        QListIterator<QList<size_t> > itLines(linesOfScan);
        while (itLines.hasNext())
        {
            const QList<size_t> &completeLine = itLines.next();

            QList<ScanLineData> candidateLines;

            for (int i = 0 ; i < completeLine.size() ; i++)
            {
                size_t index1 = completeLine.at(i);


                for (int j = i + 2 ; j < completeLine.size() ; j++)
                {
                    size_t index2 = completeLine.at(i);

                    ScanLineData simplifiedLine;
                    simplifiedLine._indices.append(index1);

                    CT_Point p1 = pointAccessor.constPointAt(index1);
                    CT_Point p2 = pointAccessor.constPointAt(index2);
                    Eigen::Vector3d direction = p2 - p1;
                    double length = direction.norm();
                    direction.normalize();

                    for (int k = i + 1 ; k < j ; k++)
                    {
                        size_t index3 = completeLine.at(k);
                        CT_Point p3 = pointAccessor.constPointAt(index3);

                        double curv = CT_MathPoint::distancePointLine(p3, direction, p1) / length;
                        if (curv < _step->_maxCurvature)
                        {
                            simplifiedLine._indices.append(index3);
                        }
                    }

                    simplifiedLine._indices.append(index2);

                    if (simplifiedLine._indices.size() > 2)
                    {
                        candidateLines.append(simplifiedLine);
                    }
                }
            }

            //todo

        }
        linesOfScan.clear();

        // Archive detection of lines of scan (all)
        if (_step->_clusterDebugMode)
        {
            for (int i = 0 ; i < scanLineClusters.size() ; i++)
            {
                CT_PointCluster* cluster = scanLineClusters.at(i);
                CT_PointCluster* cpy = new CT_PointCluster(_step->_clusterDebug_ModelName.completeName(), _res);

                const CT_AbstractPointCloudIndex* pointCloudIndexCl = cluster->getPointCloudIndex();
                CT_PointIterator itP(pointCloudIndexCl);
                while(itP.hasNext())
                {
                    size_t index = itP.next().currentGlobalIndex();
                    cpy->addPoint(index);
                }

                CT_StandardItemGroup* grpClKept = new CT_StandardItemGroup(_step->_grpClusterDebug_ModelName.completeName(), _res);
                grp->addGroup(grpClKept);
                grpClKept->addItemDrawable(cpy);
            }
        }

        // Remove clusters with 1 point or with phi > maxPhi
        QList<CT_PointCluster*> keptClustersScanLineOk;
        for (int i = 0 ; i < scanLineClusters.size() ; i++)
        {
            CT_PointCluster* cluster = scanLineClusters.at(i);

            if (cluster->getPointCloudIndexSize() < _step->_minPts)
            {
                transferPointsToIsolatedList(isolatedPointIndices, cluster);
            } else {
                const CT_AbstractPointCloudIndex* pointCloudIndexCl = cluster->getPointCloudIndex();

                const size_t index1 = pointCloudIndexCl->constIndexAt(0);
                const size_t index2 = pointCloudIndexCl->constIndexAt(pointCloudIndexCl->size() - 1);

                CT_Point p1 = pointAccessor.constPointAt(index1);
                CT_Point p2 = pointAccessor.constPointAt(index2);

                float phi, theta, length;
                if (p1(2) < p2(2))
                {
                    CT_SphericalLine3D::convertToSphericalCoordinates(&p1, &p2, phi, theta, length);
                } else {
                    CT_SphericalLine3D::convertToSphericalCoordinates(&p2, &p1, phi, theta, length);
                }

                if (phi >= thresholdZenithalAngleRadians)
                {
                    transferPointsToIsolatedList(isolatedPointIndices, cluster);
                } else {
                    keptClustersScanLineOk.append(cluster);
                }
            }
        }


        // Archive detection of lines of scan (conserved)
        if (_step->_clusterDebugMode)
        {
            for (int i = 0 ; i < keptClustersScanLineOk.size() ; i++)
            {
                CT_PointCluster* cluster = keptClustersScanLineOk.at(i);
                CT_PointCluster* cpy = new CT_PointCluster(_step->_clusterDebug2_ModelName.completeName(), _res);

                const CT_AbstractPointCloudIndex* pointCloudIndexCl = cluster->getPointCloudIndex();
                CT_PointIterator itP(pointCloudIndexCl);
                while(itP.hasNext())
                {
                    size_t index = itP.next().currentGlobalIndex();
                    cpy->addPoint(index);
                }

                CT_StandardItemGroup* grpClKept = new CT_StandardItemGroup(_step->_grpClusterDebug2_ModelName.completeName(), _res);
                grp->addGroup(grpClKept);
                grpClKept->addItemDrawable(cpy);
            }
        }


        //Compute curvatures
        QMap<CT_PointCluster*, double> curvatures;
        for (int i = 0 ; i < keptClustersScanLineOk.size() ; i++)
        {
            CT_PointCluster* cluster = keptClustersScanLineOk.at(i);
            const CT_AbstractPointCloudIndex* pointCloudIndexCl = cluster->getPointCloudIndex();

            const size_t index1 = pointCloudIndexCl->constIndexAt(0);
            const size_t index2 = pointCloudIndexCl->constIndexAt(pointCloudIndexCl->size() - 1);

            CT_Point p1 = pointAccessor.constPointAt(index1);
            CT_Point p2 = pointAccessor.constPointAt(index2);
            Eigen::Vector3d direction = p2 - p1;
            direction.normalize();

            double curvature = 0;
            CT_PointIterator itP(pointCloudIndexCl);
            while(itP.hasNext())
            {
                const CT_Point &p = itP.next().currentPoint();
                double curv = CT_MathPoint::distancePointLine(p, direction, p1);
                if (curv > curvature) {curvature = curv;}
            }
            curvatures.insert(cluster, curvature);
        }


        // find neighbors clusters
        QMultiMap<CT_PointCluster*, CT_PointCluster*> correspondances;
        for (int i = 0 ; i < keptClustersScanLineOk.size() ; i++)
        {
            CT_PointCluster* cluster1 = keptClustersScanLineOk.at(i);
            const CT_PointClusterBarycenter& bary1 = cluster1->getBarycenter();
            double curvature1 = curvatures.value(cluster1);

            for (int j = i + 1 ; j < keptClustersScanLineOk.size() ; j++)
            {
                CT_PointCluster* cluster2 = keptClustersScanLineOk.at(j);

                if (cluster1->getPointCloudIndexSize() > 2 || cluster2->getPointCloudIndexSize() > 2)
                {
                    const CT_PointClusterBarycenter& bary2 = cluster2->getBarycenter();
                    double curvature2 = curvatures.value(cluster2);

                    double maxDist = std::max(curvature1, curvature2) * _step->_curvatureMultiplier;
                    double md2 = (std::max(cluster1->getPointCloudIndexSize(), cluster2->getPointCloudIndexSize())) * _step->_nbPointDistStep;
                    if (md2 > maxDist) {maxDist = md2;}
                    if (maxDist > _step->_maxMergingDist) {maxDist = _step->_maxMergingDist;}

                    double dist = sqrt(pow(bary1.x() - bary2.x(), 2) + pow(bary1.y() - bary2.y(), 2));
                    if (dist < maxDist)
                    {
                        correspondances.insert(cluster1, cluster2);
                        correspondances.insert(cluster2, cluster1);
                    }
                }
            }
        }
        

        // merge neighbours clusters
        qSort(keptClustersScanLineOk.begin(), keptClustersScanLineOk.end(), ONF_StepDetectVerticalAlignments04::orderByAscendingNumberOfPoints);

        QList<CT_PointCluster*> mergedClusters;
        while (!keptClustersScanLineOk.isEmpty())
        {
            QList<CT_PointCluster*> toMerge;
            CT_PointCluster* first = keptClustersScanLineOk.takeLast();
            toMerge.append(first);

            QList<CT_PointCluster*> list = correspondances.values(first);
            for (int j = 0 ; j < list.size() ; j++)
            {
                CT_PointCluster* currentClust = list.at(j);
                if (keptClustersScanLineOk.contains(currentClust) && !toMerge.contains(currentClust))
                {
                    toMerge.append(currentClust);
                }
                keptClustersScanLineOk.removeOne(currentClust);
            }

            if (toMerge.size() == 1)
            {
                mergedClusters.append(toMerge.first());
            } else {
                CT_PointCluster* newCluster = new CT_PointCluster(_step->_cluster_ModelName.completeName(), _res);

                for (int i = 0 ; i < toMerge.size() ; i++)
                {
                    CT_PointCluster* cluster = toMerge.at(i);
                    const CT_AbstractPointCloudIndex* pointCloudIndexCl = cluster->getPointCloudIndex();

                    CT_PointIterator itP(pointCloudIndexCl);
                    while(itP.hasNext())
                    {
                        size_t index = itP.next().currentGlobalIndex();
                        newCluster->addPoint(index);
                    }
                    delete cluster;
                }
                mergedClusters.append(newCluster);
            }
        }


        // Compute diameters
        QMap<CT_PointCluster*, double> clustersDiameters;
        for (int i = 0 ; i < mergedClusters.size() ; i++)
        {
            CT_PointCluster* cluster = mergedClusters.at(i);

            // Compute max dist between points in clusters (in the direction where it is smallest)
            double maxDist = 0;
            Eigen::Vector3d center;
            center(0) = cluster->getBarycenter().x();
            center(1) = cluster->getBarycenter().y();
            center(2) = cluster->getBarycenter().z();

            Eigen::Vector2d center2D;
            center2D(0) = center(0);
            center2D(1) = center(1);

            computeDBH(cluster, center, maxDist);
            clustersDiameters.insert(cluster, maxDist);
        }


        // Remove clusters with 2 points or less, or clusters with not enough points considering diameter, and add others to result
        QList<CT_Circle2D*> circles;
        for (int i = 0 ; i < mergedClusters.size() ; i++)
        {
            CT_PointCluster* cluster = mergedClusters.at(i);
            size_t nbPts = cluster->getPointCloudIndexSize();
            double dbh = clustersDiameters.value(cluster);
            double ptsDbhRatio = _step->_ratioDbhNbptsMax + 1.0;

            if (nbPts > 0)  {ptsDbhRatio = dbh / nbPts;}

            if (nbPts <= 2 || ptsDbhRatio > _step->_ratioDbhNbptsMax)
            {
                transferPointsToIsolatedList(isolatedPointIndices, cluster);
            } else {
                bool corrected;
                dbh = correctDbh(dbh, nbPts, &corrected);

                CT_StandardItemGroup* grpClKept = new CT_StandardItemGroup(_step->_grpCluster_ModelName.completeName(), _res);
                grp->addGroup(grpClKept);
                grpClKept->addItemDrawable(cluster);

                cluster->addItemAttribute(new CT_StdItemAttributeT<double>(_step->_attMaxDistXY_ModelName.completeName(), CT_AbstractCategory::DATA_VALUE, _res, dbh*100.0));

                if (corrected)
                {
                    cluster->addItemAttribute(new CT_StdItemAttributeT<int>(_step->_attStemType_ModelName.completeName(), CT_AbstractCategory::DATA_VALUE, _res, 1));
                } else {
                    cluster->addItemAttribute(new CT_StdItemAttributeT<int>(_step->_attStemType_ModelName.completeName(), CT_AbstractCategory::DATA_VALUE, _res, 2));
                }

                Eigen::Vector3d center;
                center(0) = cluster->getBarycenter().x();
                center(1) = cluster->getBarycenter().y();
                center(2) = cluster->getBarycenter().z();

                Eigen::Vector2d center2D;
                center2D(0) = center(0);
                center2D(1) = center(1);

                CT_Circle2D *circle = new CT_Circle2D(_step->_circle_ModelName.completeName(), _res, new CT_Circle2DData(center2D, dbh/2.0));
                grpClKept->addItemDrawable(circle);

                circles.append(circle);
            }
        }


        ////////////////////////////////////////////////////
        /// Detection of small stems: points alignements ///
        ////////////////////////////////////////////////////

        double maxPhiRadians = M_PI*_step->_maxPhiAngleSmall/180.0;

        QList<ONF_StepDetectVerticalAlignments04::LineData*> candidateLines;
        // Parcours tous les couples de points 2 à deux
        for (int iso1 = 0 ; iso1 < isolatedPointIndices.size() && (!_step->isStopped()); iso1++)
        {
            size_t index1 = isolatedPointIndices.at(iso1);
            CT_Point point1 = pointAccessor.constPointAt(index1);

            for (int iso2 = iso1 + 1 ; iso2 < isolatedPointIndices.size() && (!_step->isStopped()); iso2++)
            {
                size_t index2 = isolatedPointIndices.at(iso2);
                CT_Point point2 = pointAccessor.constPointAt(index2);

                // Les deux points doivent être disctinct dans l'espace
                if ((point1(0) != point2(0) || point1(1) != point2(1) || point1(2) != point2(2)))
                {
                    // Les deux points doivent être à moins de _distThreshold
                    double dist = sqrt(pow(point1(0) - point2(0), 2) + pow(point1(1) - point2(1), 2) + pow(point1(2) - point2(2), 2));
                    if (dist < _step->_pointDistThresholdSmall)
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
                            candidateLines.append(new ONF_StepDetectVerticalAlignments04::LineData(pointLow, pointHigh, index1, index2, phi, sceneStem->minZ(), sceneStem->maxZ()));
                        }
                    }
                }
            }
        }

        // Affiliation des lignes proches
        findNeighborLines(candidateLines, _step->_lineDistThresholdSmall);

        // Tri par NeighborCount descendant
        qSort(candidateLines.begin(), candidateLines.end(), ONF_StepDetectVerticalAlignments04::orderByDescendingNeighborCount);

        // Constitution des clusters de points alignés
        QList<size_t> insertedPoints;

        for (int i = 0 ; i < candidateLines.size() ; i++)
        {
            ONF_StepDetectVerticalAlignments04::LineData* candidateLine = candidateLines.at(i);
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

                QList<ONF_StepDetectVerticalAlignments04::LineData*> &neighborLines = candidateLine->_neighbors;
                for (int j = 0 ; j < neighborLines.size() ; j++)
                {
                    ONF_StepDetectVerticalAlignments04::LineData* neighborLine = neighborLines.at(j);
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

                if (nbPts >= _step->_minPtsSmall)
                {
                    CT_LineData* fittedLineData = CT_LineData::staticCreateLineDataFromPointCloud(*cloudIndex);

                    Eigen::Vector3d pointLow  = fittedLineData->getP1();
                    Eigen::Vector3d pointHigh = fittedLineData->getP2();

                    if (pointHigh(2) < pointLow(2))
                    {
                        pointLow  = fittedLineData->getP2();
                        pointHigh = fittedLineData->getP1();
                    }

                    float phi, theta, length;
                    CT_SphericalLine3D::convertToSphericalCoordinates(&pointLow, &pointHigh, phi, theta, length);

                    if (phi > maxPhiRadians)
                    {
                        delete cluster;
                        delete fittedLineData;

                    } else {

                        QList<double> dists;

                        CT_PointIterator itP(cloudIndex);
                        while(itP.hasNext())
                        {
                            const CT_Point &point = itP.next().currentPoint();

                            Eigen::Vector3d projPoint;
                            dists.append(CT_MathPoint::distanceOnLineForPointProjection(fittedLineData->getP1(), fittedLineData->getDirection(), point, projPoint));
                        }

                        qSort(dists);

                        bool okLength = true;
                        double maxLength = _step->_lineLengthRatioSmall * fittedLineData->length();
                        for (int dd = 1 ; dd < dists.size() && okLength; dd++)
                        {
                            double currentDist = dists.at(dd) - dists.at(dd - 1);
                            if (currentDist > maxLength) {okLength = false;}
                        }
                        dists.clear();

                        if (okLength)
                        {
                            const CT_PointClusterBarycenter& bary = cluster->getBarycenter();

                            bool toClose = false;
                            for (int cc = 0 ; cc < circles.size() && !toClose ; cc++)
                            {
                                const CT_Circle2D& circle = *(circles.at(cc));
                                double dist = sqrt(pow(circle.getCenterX() - bary.x(), 2) + pow(circle.getCenterY() - bary.y(), 2));
                                if (dist < _step->_exclusionRadiusSmall) {toClose = true;}
                            }


                            if (toClose)
                            {
                                delete cluster;
                                delete fittedLineData;
                            } else {

                                // compute diameter
                                double diameter = correctDbh(0, nbPts);

                                // add items to result
                                CT_StandardItemGroup* grpClKept = new CT_StandardItemGroup(_step->_grpCluster_ModelName.completeName(), _res);
                                grp->addGroup(grpClKept);

                                grpClKept->addItemDrawable(cluster);

                                cluster->addItemAttribute(new CT_StdItemAttributeT<double>(_step->_attMaxDistXY_ModelName.completeName(), CT_AbstractCategory::DATA_VALUE, _res, diameter*100.0));
                                cluster->addItemAttribute(new CT_StdItemAttributeT<int>(_step->_attStemType_ModelName.completeName(), CT_AbstractCategory::DATA_VALUE, _res, 0));


                                CT_Line* line = new CT_Line(_step->_line_ModelName.completeName(), _res, fittedLineData);
                                grpClKept->addItemDrawable(line);

                                Eigen::Vector3d center;
                                center(0) = cluster->getBarycenter().x();
                                center(1) = cluster->getBarycenter().y();
                                center(2) = cluster->getBarycenter().z();

                                Eigen::Vector2d center2D;
                                center2D(0) = center(0);
                                center2D(1) = center(1);

                                CT_Circle2D *circle = new CT_Circle2D(_step->_circle_ModelName.completeName(), _res, new CT_Circle2DData(center2D, diameter/2.0));
                                grpClKept->addItemDrawable(circle);
                            }
                        } else {
                            delete cluster;
                            delete fittedLineData;
                        }
                    }
                } else {
                    delete cluster;
                }
            }
        }

        qDeleteAll(candidateLines);
        candidateLines.clear();
        insertedPoints.clear();
    }
}

double ONF_StepDetectVerticalAlignments04::AlignmentsDetectorForScene::computeCurvature(CT_PointAccessor &pointAccessor, const QList<size_t> &line)
{
    if (line.size() < 2) {return -1;}

    const size_t index1 = line.first();
    const size_t index2 = line.last();

    CT_Point p1 = pointAccessor.constPointAt(index1);
    CT_Point p2 = pointAccessor.constPointAt(index2);
    Eigen::Vector3d direction = p2 - p1;
    direction.normalize();

    double curvature = 0;
    double curv;
    for (int i = 1 ; i < line.size() - 1 ; i++)
    {
        size_t index = line.at(i);
        CT_Point p = pointAccessor.constPointAt(index);
        curv = CT_MathPoint::distancePointLine(p, direction, p1);
        if (curv > curvature) {curvature = curv;}
    }
    return curvature;
}


void ONF_StepDetectVerticalAlignments04::AlignmentsDetectorForScene::computeDBH(CT_PointCluster* cluster, Eigen::Vector3d &center, double &maxDist)
{
    maxDist = std::numeric_limits<double>::max();

    const CT_AbstractPointCloudIndex* pointCloudIndex = cluster->getPointCloudIndex();
    CT_PointIterator itP(pointCloudIndex);

    QVector<Eigen::Vector3d> points(pointCloudIndex->size());
    int cpt = 0;
    while(itP.hasNext())
    {
        itP.next();
        const CT_Point &pt = itP.currentPoint();
        points[cpt++] = pt;
    }

    for (float zenithal = 0 ; zenithal <= _step->_DBH_zeniMax ; zenithal += _step->_DBH_resAzimZeni)
    {
        float zeniRad = M_PI * zenithal / 180.0;
        for (float azimut = 0 ; azimut <= 360 ; azimut += _step->_DBH_resAzimZeni)
        {
            float azimRad = M_PI * azimut / 180.0;

            // Compute projected points
            float dx, dy, dz;
            CT_SphericalLine3D::convertToCartesianCoordinates(zeniRad, azimRad, 1, dx, dy, dz);
            Eigen::Vector3d direction(dx, dy, dz);

            QList<Eigen::Vector2d*> projPts;
            Eigen::Hyperplane<double, 3> plane(direction, center);

            for (int i = 0 ; i < points.size() ; i++)
            {
                Eigen::Vector3d projectedPt = plane.projection(points[i]);
                projPts.append(new Eigen::Vector2d(projectedPt(0), projectedPt(1)));
            }

            double maxDistIteration = 0;
            // Compute maxDist
            for (int i = 0 ; i < projPts.size() ; i++)
            {
                Eigen::Vector2d* pt1 = projPts.at(i);
                for (int j = i+1 ; j < projPts.size() ; j++)
                {
                    Eigen::Vector2d* pt2 = projPts.at(j);

                    double dist = sqrt(pow((*pt1)(0) - (*pt2)(0), 2) + pow((*pt1)(1) - (*pt2)(1), 2));
                    if (dist > maxDistIteration) {maxDistIteration = dist;}
                }
            }

            qDeleteAll(projPts);

            if (maxDistIteration < maxDist)
            {
                maxDist = maxDistIteration;
            }
        }
    }

}

void ONF_StepDetectVerticalAlignments04::AlignmentsDetectorForScene::findNeighborLines(QList<ONF_StepDetectVerticalAlignments04::LineData*> candidateLines, double distThreshold)
{
    for (int i1 = 0 ; i1 < candidateLines.size() ; i1++)
    {
        ONF_StepDetectVerticalAlignments04::LineData* line1 = candidateLines.at(i1);

        for (int i2 = i1+1 ; i2 < candidateLines.size() ; i2++)
        {
            ONF_StepDetectVerticalAlignments04::LineData* line2 = candidateLines.at(i2);

            double distLow = sqrt(pow(line1->_lowCoord(0) - line2->_lowCoord(0), 2) + pow(line1->_lowCoord(1) - line2->_lowCoord(1), 2));

            if (distLow < distThreshold)
            {
                double distHigh = sqrt(pow(line1->_highCoord(0) - line2->_highCoord(0), 2) + pow(line1->_highCoord(1) - line2->_highCoord(1), 2));
                //double spacing = fabs(distHigh - distLow);

                if (distHigh < distThreshold)
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
}

void ONF_StepDetectVerticalAlignments04::AlignmentsDetectorForScene::transferPointsToIsolatedList(QList<size_t> &isolatedPointIndices, CT_PointCluster* cluster)
{
    const CT_AbstractPointCloudIndex* pointCloudIndexCl = cluster->getPointCloudIndex();
    for (int j = 0 ; j < pointCloudIndexCl->size() ; j++)
    {
        isolatedPointIndices.append(pointCloudIndexCl->constIndexAt(j));
    }
    delete cluster;
}

double ONF_StepDetectVerticalAlignments04::AlignmentsDetectorForScene::correctDbh(double diameter, int pointsNumber, bool* corrected)
{
    if (diameter >= _step->_dbhMax)
    {
        if (corrected != NULL) {*corrected = false;}
        return diameter;
    }
    if (corrected != NULL) {*corrected = true;}

    double ratio = (float) pointsNumber / (float) _step->_nbPtsForDbhMax;
    if (ratio > 1) {ratio = 1;}

    return ratio * (_step->_dbhMax - _step->_dbhMin) + _step->_dbhMin;
}
