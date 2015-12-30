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
#include "ct_accessor/ct_pointaccessor.h"


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
    _thresholdDistXY = 0.25;



    _pointDistThreshold = 4.0;
    _maxPhiAngle = 30.0;

    _lineDistThreshold = 0.5;
    _minPtsNb = 3;
    _lineLengthRatio = 0.8;

    _clusterDistThreshold = 3.0;
    _maxDiameter = 1.2;
    _maxDivergence = 0.1;
    _maxPhiAngleAfterMerging = 15.0;
    _radiusHmax = 1.5;

    _lengthThreshold = 2.0;
    _heightThreshold = 0.6;

    _maxDiamRatio = 0.10;
    _circleDistThreshold = 0.05;

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
    resIn_res->addItemModel(DEFin_grp, DEFin_attLineOfScan, CT_AbstractPointAttributesScalar::staticGetType(), tr("Ligne de Scan"),
                            tr("Attribut codant la ligne de scan"), CT_InAbstractModel::C_ChooseOneIfMultiple, CT_InAbstractModel::F_IsOptional);
    resIn_res->addItemModel(DEFin_grp, DEFin_attGPSTime, CT_AbstractPointAttributesScalar::staticGetType(), tr("Temps GPS"),
                            tr("Attribut codant le temps GPS"), CT_InAbstractModel::C_ChooseOneIfMultiple, CT_InAbstractModel::F_IsOptional);

    resIn_res->addItemModel(DEFin_grp, DEFin_sceneAll, CT_AbstractItemDrawableWithPointCloud::staticGetType(), tr("Scène (complète)"), "",
                            CT_InAbstractModel::C_ChooseOneIfMultiple, CT_InAbstractModel::F_IsOptional);
}

// Creation and affiliation of OUT models
void ONF_StepDetectVerticalAlignments04::createOutResultModelListProtected()
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
    resCpy->addItemAttributeModel(_line_ModelName, _attMaxDist_ModelName, new CT_StdItemAttributeT<double>(CT_AbstractCategory::DATA_VALUE), tr("MaxDistBetweenPoints"));
    resCpy->addItemAttributeModel(_line_ModelName, _attDiamEq_ModelName, new CT_StdItemAttributeT<double>(CT_AbstractCategory::DATA_VALUE), tr("DiamEq"));
    resCpy->addItemAttributeModel(_line_ModelName, _attHmax_ModelName, new CT_StdItemAttributeT<double>(CT_AbstractCategory::DATA_VALUE), tr("Hmax"));

    resCpy->addGroupModel(DEFin_grp, _grpDroppedCluster_ModelName, new CT_StandardItemGroup(), tr("Clusters éliminés"));
    resCpy->addItemModel(_grpDroppedCluster_ModelName, _droppedCluster_ModelName, new CT_PointCluster(), tr("Cluster éliminé"));
    resCpy->addItemModel(_grpDroppedCluster_ModelName, _droppedLine_ModelName, new CT_Line(), tr("Ligne éliminée"));

}

// Semi-automatic creation of step parameters DialogBox
void ONF_StepDetectVerticalAlignments04::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addTitle( tr("1- Détéction des lignes de scan :"));
    configDialog->addDouble(tr("Seuil de temps GPS pour changer de ligne"), "m", -1e+10, 1e+10, 10, _thresholdGPSTime);
    configDialog->addDouble(tr("Seuil de distance XY"), "m", 0, 1e+4, 10, _thresholdDistXY);


//    configDialog->addTitle( tr("1- Paramètres de création des droites candidates :"));
//    configDialog->addDouble(tr("Distance maximum entre deux points d'une droite candidate"), "m", 0, 1000, 2, _pointDistThreshold);
//    configDialog->addDouble(tr("Angle zénithal maximal pour une droite candidate"), "°", 0, 180, 2, _maxPhiAngle);

//    configDialog->addEmpty();
//    configDialog->addTitle(tr("2- Paramètres de création des clusters (à partir des droites candidates) :"));
//    configDialog->addDouble(tr("Distance maximum XY entre deux droites candidates à agréger"), "m", 0, 1000, 2, _lineDistThreshold);
//    configDialog->addInt(tr("Nombre de points minimum dans un cluster"), "", 2, 1000, _minPtsNb);
//    configDialog->addDouble(tr("Pourcentage maximum de la longueur de segment sans points"), "%", 0, 100, 0, _lineLengthRatio, 100);


//    configDialog->addEmpty();
//    configDialog->addTitle(tr("3- Paramètres de validation/fusion des clusters obtenus :"));
//    configDialog->addDouble(tr("Distance de recherche des clusters voisins"), "m", 0, 1000, 2, _clusterDistThreshold);
//    configDialog->addDouble(tr("Diamètre maximal possible pour un arbre"), "cm", 0, 1000, 2, _maxDiameter, 100);
//    configDialog->addDouble(tr("Divergence maximale entre clusters à fusionner"), "m", 0, 1000, 2, _maxDivergence);
//    configDialog->addDouble(tr("Angle zénithal maximal de la droite après fusion"), "°", 0, 180, 2, _maxPhiAngleAfterMerging);
//    configDialog->addDouble(tr("Rayon de recherche pour Hmax"), "m", 0, 1000, 2, _radiusHmax);

//    configDialog->addDouble(tr("Supprimer les clusters dont la longueur est inférieure à"), "m", 0, 1000, 2, _lengthThreshold);
//    configDialog->addDouble(tr("Supprimer les clusters qui commence au dessus de "), "% de Hscene", 0, 100, 0, _heightThreshold, 100);


//    configDialog->addEmpty();
//    configDialog->addTitle(tr("4- Paramètres de validation des diamètres :"));
//    configDialog->addDouble(tr("Ecart max Dmax n et n-1"), "%", 0, 100, 0, _maxDiamRatio, 100);
//    configDialog->addDouble(tr("Epaisseur des cercles pour le scoring"), "cm", 0, 99999, 2, _circleDistThreshold, 100);

//    configDialog->addEmpty();
//    configDialog->addBool(tr("Mode Debug Clusters"), "", "", _clusterDebugMode);
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

    const CT_AbstractItemDrawableWithPointCloud* sceneStem = (CT_AbstractItemDrawableWithPointCloud*)grp->firstItemByINModelName(_step, DEFin_sceneStem);
    const CT_AbstractPointAttributesScalar* attributeGPSTime = (CT_AbstractPointAttributesScalar*)grp->firstItemByINModelName(_step, DEFin_attGPSTime);


    if (sceneStem != NULL && attributeGPSTime != NULL)
    {
        const CT_AbstractPointCloudIndex* pointCloudIndex = sceneStem->getPointCloudIndex();

        // Tri des indices par temps gps
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

        bool lastUpwards = false;
        double lastGPSTime = -std::numeric_limits<double>::max();
        double lastZ = -std::numeric_limits<double>::max();

        CT_PointCluster* cluster = NULL;

        QList<CT_PointCluster*> scanLineClusters;

        // Creation des lignes de scan
        QMapIterator<double, size_t> itMapSorted(sortedIndices);
        while (itMapSorted.hasNext())
        {
            itMapSorted.next();
            double gpsTime = itMapSorted.key();
            size_t index = itMapSorted.value();

            const CT_Point &point = pointAccessor.constPointAt(index);

            double delta = gpsTime - lastGPSTime;
            lastGPSTime = gpsTime;

            //qDebug() << "gpsTime= " << QString::number(gpsTime, 'f', 20);

            bool upwards = (point(2) > lastZ);
            lastZ = point(2);

            if (cluster != NULL && delta < _step->_thresholdGPSTime)// && upwards == lastUpwards)
            {
                // add to line of scan
                cluster->addPoint(index);
            } else {
                // create new line of scan and add
                cluster = new CT_PointCluster(_step->_cluster_ModelName.completeName(), _res);                                
                scanLineClusters.append(cluster);

                cluster->addPoint(index);
            }
            lastUpwards = upwards;
        }


        // Validate clusters
        QList<size_t> isolatedPointIndices;
        QList<CT_PointCluster*> keptClusters;
        for (int i = 0 ; i < scanLineClusters.size() ; i++)
        {
            validateScanLineCluster(scanLineClusters.at(i), keptClusters, isolatedPointIndices, pointAccessor);
        }

        for (int i = 0 ; i < keptClusters.size() ; i++)
        {
            cluster = keptClusters.at(i);

            if (cluster->getPointCloudIndexSize() > 2)
            {
                CT_StandardItemGroup* grpClKept = new CT_StandardItemGroup(_step->_grpCluster_ModelName.completeName(), _res);
                grp->addGroup(grpClKept);
                grpClKept->addItemDrawable(cluster);
            } else if (cluster->getPointCloudIndexSize() > 1) {
                cluster->setModel(_step->_droppedCluster_ModelName.completeName());

                CT_StandardItemGroup* grpClDropped = new CT_StandardItemGroup(_step->_grpDroppedCluster_ModelName.completeName(), _res);
                grp->addGroup(grpClDropped);
                grpClDropped->addItemDrawable(cluster);
            }
        }
    }
}

void ONF_StepDetectVerticalAlignments04::AlignmentsDetectorForScene::validateScanLineCluster(CT_PointCluster* cluster, QList<CT_PointCluster*> &keptClusters, QList<size_t> &isolatedPointIndices, CT_PointAccessor &pointAccessor)
{
    if (cluster == NULL) {return;}
    const CT_AbstractPointCloudIndex* pointCloudIndex = cluster->getPointCloudIndex();

    if (pointCloudIndex->size() == 1)
    {
        isolatedPointIndices.append(pointCloudIndex->indexAt(0));
        delete cluster;
    } else if (pointCloudIndex->size() == 2)
    {
        const size_t index1 = pointCloudIndex->constIndexAt(0);
        const size_t index2 = pointCloudIndex->constIndexAt(1);

        CT_Point point1 = pointAccessor.constPointAt(index1);
        CT_Point point2 = pointAccessor.constPointAt(index2);

        double distXY = sqrt(pow(point1(0) - point2(0), 2) + pow(point1(1) - point2(1), 2));

        if (distXY < _step->_thresholdDistXY)
        {
            keptClusters.append(cluster);
        } else {
            isolatedPointIndices.append(index1);
            isolatedPointIndices.append(index2);
            delete cluster;
        }
    } else if (pointCloudIndex->size() > 2)
    {
        QList<size_t> currentList;
        for (int i = 0 ; i < pointCloudIndex->size() ; i++)
        {
            const size_t index1 = pointCloudIndex->constIndexAt(i);
            CT_Point point1 = pointAccessor.constPointAt(index1);
            double distXY = std::numeric_limits<double>::max();

            if (i < pointCloudIndex->size() - 1)
            {
                const size_t index2 = pointCloudIndex->constIndexAt(i+1);
                CT_Point point2 = pointAccessor.constPointAt(index2);
                distXY = sqrt(pow(point1(0) - point2(0), 2) + pow(point1(1) - point2(1), 2));
            }

            currentList.append(index1);

            if (distXY >= _step->_thresholdDistXY)
            {
                processCurrentList(isolatedPointIndices, currentList, keptClusters);
            }
        }

        processCurrentList(isolatedPointIndices, currentList, keptClusters);

        delete cluster;
    } else {
        delete cluster;
    }
}

void ONF_StepDetectVerticalAlignments04::AlignmentsDetectorForScene::processCurrentList(QList<size_t> &isolatedPointIndices, QList<size_t> &currentList, QList<CT_PointCluster*> &keptClusters)
{
    if (currentList.size() == 1)
    {
        isolatedPointIndices.append(currentList.at(0));
    } else if (currentList.size() > 1)
    {
        CT_PointCluster* newCluster = new CT_PointCluster(_step->_cluster_ModelName.completeName(), _res);
        for (int j = 0 ; j < currentList.size() ; j++)
        {
            newCluster->addPoint(currentList.at(j));
        }
        keptClusters.append(newCluster);
    }
    currentList.clear();
}








void ONF_StepDetectVerticalAlignments04::AlignmentsDetectorForScene::detectAlignmentsForScene_old(CT_StandardItemGroup* grp)
{

    const CT_AbstractItemDrawableWithPointCloud* sceneStem = (CT_AbstractItemDrawableWithPointCloud*)grp->firstItemByINModelName(_step, DEFin_sceneStem);
    const CT_AbstractItemDrawableWithPointCloud* sceneAll = (CT_AbstractItemDrawableWithPointCloud*)grp->firstItemByINModelName(_step, DEFin_sceneAll);
    const CT_AbstractPointAttributesScalar* attributeLineOfScan = (CT_AbstractPointAttributesScalar*)grp->firstItemByINModelName(_step, DEFin_attLineOfScan);

    if (sceneStem != NULL)
    {
        double maxPhiRadians = M_PI*_step->_maxPhiAngle/180.0;
        double maxPhiRadiansAfterMerging = M_PI*_step->_maxPhiAngleAfterMerging/180.0;

        QList<ONF_StepDetectVerticalAlignments04::LineData*> candidateLines;
        const CT_AbstractPointCloudIndex* pointCloudIndex = sceneStem->getPointCloudIndex();

        //double deltaZ = scene->maxZ() - scene->minZ();
        double hMax = (sceneStem->maxZ() - sceneStem->minZ())*_step->_heightThreshold + sceneStem->minZ();

        // Parcours tous les couples de points 2 à deux
        CT_PointIterator itP1(pointCloudIndex);
        while(itP1.hasNext() && (!_step->isStopped()))
        {
            const CT_Point &point1 = itP1.next().currentPoint();
            size_t index1 = itP1.currentGlobalIndex();

            double lineOfScan1 = 0; // Récupération de la ligne de scan pour le point 1
            if (attributeLineOfScan != NULL)
            {
                size_t localIndex1 = attributeLineOfScan->getPointCloudIndex()->indexOf(index1);
                if (localIndex1 < attributeLineOfScan->getPointCloudIndex()->size())
                {
                    lineOfScan1 = attributeLineOfScan->dValueAt(localIndex1);
                }
            }

            bool before = true;
            CT_PointIterator itP2(pointCloudIndex);
            while(itP2.hasNext() && (!_step->isStopped()))
            {
                const CT_Point &point2 = itP2.next().currentPoint();
                size_t index2 = itP2.currentGlobalIndex();

                double lineOfScan2 = 0; // Récupération de la ligne de scan pour le point 2
                if (attributeLineOfScan != NULL)
                {
                    size_t localIndex2 = attributeLineOfScan->getPointCloudIndex()->indexOf(index2);
                    if (localIndex2 < pointCloudIndex->size())
                    {
                        lineOfScan2 = attributeLineOfScan->dValueAt(localIndex2);
                    }
                }



                // Les deux points doivent être disctinct dans l'espace
                if (!before &&
                    ((point1(0) != point2(0) || point1(1) != point2(1) || point1(2) != point2(2))) &&
                    lineOfScan1 == lineOfScan2) // Les deux points doivent avoir la même ligne de vol
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
                            candidateLines.append(new ONF_StepDetectVerticalAlignments04::LineData(pointLow, pointHigh, index1, index2, phi, sceneStem->minZ(), sceneStem->maxZ(), lineOfScan1));
                        }
                    }
                }

                if (index1 == index2) {before = false;}
            }
        }

        // Tri par Phi croissant... inutile
        //qSort(candidateLines.begin(), candidateLines.end(), ONF_StepDetectVerticalAlignments04::orderByAscendingPhi);

        // Affiliation des lignes proches
        findNeighborLines(candidateLines, _step->_lineDistThreshold);

        // Tri par NeighborCount descendant
        qSort(candidateLines.begin(), candidateLines.end(), ONF_StepDetectVerticalAlignments04::orderByDescendingNeighborCount);

        // Constitution des clusters de points alignés
        QMap<ONF_StepDetectVerticalAlignments04::LineData*, CT_PointCluster*> pointsClusters;
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

                if (nbPts >= _step->_minPtsNb)
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
                        sendToDroppedList(grp, cluster, fittedLineData);
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
                        double maxLength = _step->_lineLengthRatio * fittedLineData->length();
                        for (int dd = 1 ; dd < dists.size() && okLength; dd++)
                        {
                            double currentDist = dists.at(dd) - dists.at(dd - 1);
                            if (currentDist > maxLength) {okLength = false;}
                        }
                        dists.clear();

                        if (okLength)
                        {
                            ONF_StepDetectVerticalAlignments04::LineData *lineData = new ONF_StepDetectVerticalAlignments04::LineData(pointLow, pointHigh, 0, 0, phi, sceneStem->minZ(), sceneStem->maxZ(), candidateLine->_lineOfScan);

                            pointsClusters.insert(lineData, cluster);

                            if (_step->_clusterDebugMode) // mode Debug Cluster
                            {
                                CT_StandardItemGroup* grpClKept = new CT_StandardItemGroup(_step->_grpCluster_ModelName.completeName(), _res);
                                grp->addGroup(grpClKept);

                                cluster->setModel(_step->_cluster_ModelName.completeName());
                                grpClKept->addItemDrawable(cluster);

                                CT_Line* line = new CT_Line(_step->_line_ModelName.completeName(), _res, fittedLineData);
                                grpClKept->addItemDrawable(line);
                            } else {
                                delete fittedLineData;
                            }
                        } else {
                            sendToDroppedList(grp, cluster, fittedLineData);
                        }
                    }
                } else {                    
                    sendToDroppedList(grp, cluster, NULL);
                }
            }
        }

        qDeleteAll(candidateLines);
        candidateLines.clear();
        insertedPoints.clear();

        if (_step->_clusterDebugMode) {return;}


        // Regroupement des clusters voisins
        // Affiliation des clusters proches
        QList<ONF_StepDetectVerticalAlignments04::LineData*> candidateClustersLines = pointsClusters.keys();
        findNeighborLines(candidateClustersLines, _step->_clusterDistThreshold);

        // Tri par NeighborCount descendant
        qSort(candidateClustersLines.begin(), candidateClustersLines.end(), ONF_StepDetectVerticalAlignments04::orderByDescendingNeighborCount);


        QList<ClusterHeightData*> clustersHeightsData;

        // Création des clusters fusionnés
        for (int i = 0  ; i < candidateClustersLines.size() ; i++)
        {
            ONF_StepDetectVerticalAlignments04::LineData* lineData = candidateClustersLines.at(i);

            if (!lineData->_processed)
            {
                lineData->_processed = true;

                QList<ONF_StepDetectVerticalAlignments04::LineData*> toMerge;
                toMerge.append(lineData);                               

                for (int j = 0 ; j < lineData->_neighbors.size() ; j++)
                {
                    ONF_StepDetectVerticalAlignments04::LineData* neighborLineData  = lineData->_neighbors.at(j);

                    if (!neighborLineData->_processed)
                    {
                        double distLow  = sqrt(pow(lineData->_lowCoord(0)  - neighborLineData->_lowCoord(0), 2)  + pow(lineData->_lowCoord(1)  - neighborLineData->_lowCoord(1), 2));
                        double distHigh = sqrt(pow(lineData->_highCoord(0) - neighborLineData->_highCoord(0), 2) + pow(lineData->_highCoord(1) - neighborLineData->_highCoord(1), 2));

                        double distLow1ToHigh2  = sqrt(pow(lineData->_lowCoord(0)  - neighborLineData->_highCoord(0), 2)  + pow(lineData->_lowCoord(1)  - neighborLineData->_highCoord(1), 2));
                        double distLow2ToHigh2  = sqrt(pow(neighborLineData->_lowCoord(0)  - neighborLineData->_highCoord(0), 2)  + pow(neighborLineData->_lowCoord(1)  - neighborLineData->_highCoord(1), 2));

                        double deltaDist = distHigh - distLow;

                        if (distLow <= _step->_maxDiameter)
                        {
                            if (distHigh <= _step->_maxDiameter)
                            {
                                if (deltaDist <= _step->_maxDivergence)
                                {
                                    toMerge.append(neighborLineData);
                                    neighborLineData->_processed = true;
                                }
                            } else if (distLow1ToHigh2 < distLow2ToHigh2){

                                sendToDroppedList(grp, pointsClusters.take(neighborLineData), new CT_LineData(neighborLineData->_pLow, neighborLineData->_pHigh));
                                neighborLineData->_processed = true;
                            }

                        } else if (distHigh <= _step->_maxDiameter) {
                            sendToDroppedList(grp, pointsClusters.take(neighborLineData), new CT_LineData(neighborLineData->_pLow, neighborLineData->_pHigh));
                            neighborLineData->_processed = true;
                        }
                    }
                }

                CT_PointCluster* cluster = NULL;
                CT_LineData* mergedLineData = NULL;

                // merge clusters and fit line
                if (toMerge.size() > 1)
                {
                    cluster = new CT_PointCluster(_step->_cluster_ModelName.completeName(), _res);

                    for (int j = 0 ; j < toMerge.size() ; j++)
                    {
                        ONF_StepDetectVerticalAlignments04::LineData* toMergeLineData = toMerge.at(j);
                        CT_PointCluster* toMergeCluster = pointsClusters.take(toMergeLineData);

                        const CT_AbstractPointCloudIndex *toMergeCloudIndex = toMergeCluster->getPointCloudIndex();

                        CT_PointIterator itP(toMergeCloudIndex);
                        while(itP.hasNext())
                        {
                            size_t index = itP.next().currentGlobalIndex();
                            cluster->addPoint(index);
                        }

                        delete toMergeCluster;
                    }


                    CT_LineData* fittedLineData = CT_LineData::staticCreateLineDataFromPointCloud(*(cluster->getPointCloudIndex()));

                    Eigen::Vector3d pointLow  = fittedLineData->getP1();
                    Eigen::Vector3d pointHigh = fittedLineData->getP2();

                    if (pointHigh(2) < pointLow(2))
                    {
                        pointLow  = fittedLineData->getP2();
                        pointHigh = fittedLineData->getP1();
                    }

                    mergedLineData = new CT_LineData(pointLow, pointHigh);

                    delete fittedLineData;

                } else {
                    cluster = pointsClusters.take(lineData);
                    mergedLineData = new CT_LineData(lineData->_pLow, lineData->_pHigh);
                }

                float phi, theta, length;
                CT_SphericalLine3D::convertToSphericalCoordinates(&(mergedLineData->getP1()), &(mergedLineData->getP2()), phi, theta, length);

                if (mergedLineData->length() < _step->_lengthThreshold || (mergedLineData->getP1())(2) > hMax || phi > maxPhiRadiansAfterMerging)
                {
                    sendToDroppedList(grp, cluster, mergedLineData);
                } else {

                    clustersHeightsData.append(new ONF_StepDetectVerticalAlignments04::ClusterHeightData(cluster, mergedLineData));
                }
            }
        }

        // Computes Hmax for each cluster
        if (sceneAll != NULL)
        {
            const CT_AbstractPointCloudIndex* pointCloudIndexAll = sceneAll->getPointCloudIndex();

            CT_PointIterator itAll(pointCloudIndexAll);
            while(itAll.hasNext() && (!_step->isStopped()))
            {
                const CT_Point &point = itAll.next().currentPoint();

                for (int i = 0 ; i < clustersHeightsData.size() ; i++)
                {
                    ONF_StepDetectVerticalAlignments04::ClusterHeightData *clHD = clustersHeightsData.at(i);
                    const CT_PointClusterBarycenter &bary = clHD->_cluster->getBarycenter();

                    double distance = sqrt(pow(bary.x() - point(0), 2) + pow(bary.y() - point(1), 2));

                    if (distance < _step->_radiusHmax)
                    {
                        if (point(2) > clHD->_hMaxScene) {clHD->_hMaxScene = point(2);}
                    }
                }
            }
        }

        // Creates kepts items
        for (int i = 0 ; i < clustersHeightsData.size() ; i++)
        {
            ONF_StepDetectVerticalAlignments04::ClusterHeightData *clHD = clustersHeightsData.at(i);
            sendToConservedList(grp, clHD->_cluster, clHD->_mergedLineData, clHD->_hMaxScene);

            delete clHD;
        }

        qDeleteAll(candidateClustersLines);
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

            // Les lignes doivent être dans la même ligne de scan
            if (line2->_lineOfScan == line1->_lineOfScan)
            {
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
}

void ONF_StepDetectVerticalAlignments04::AlignmentsDetectorForScene::sendToDroppedList(CT_StandardItemGroup* grp, CT_PointCluster* cluster, CT_LineData* lineData)
{
    CT_StandardItemGroup* grpClDropped = new CT_StandardItemGroup(_step->_grpDroppedCluster_ModelName.completeName(), _res);
    grp->addGroup(grpClDropped);

    if (cluster != NULL)
    {
        cluster->setModel(_step->_droppedCluster_ModelName.completeName());
        grpClDropped->addItemDrawable(cluster);
    }

    if (lineData != NULL)
    {
        CT_Line* line = new CT_Line(_step->_droppedLine_ModelName.completeName(), _res, lineData);
        grpClDropped->addItemDrawable(line);
    }
}

void ONF_StepDetectVerticalAlignments04::AlignmentsDetectorForScene::sendToConservedList(CT_StandardItemGroup* grp, CT_PointCluster* cluster, CT_LineData* mergedLineData, double hMaxScene)
{
    CT_StandardItemGroup* grpClKept = new CT_StandardItemGroup(_step->_grpCluster_ModelName.completeName(), _res);
    grp->addGroup(grpClKept);

    cluster->setModel(_step->_cluster_ModelName.completeName());
    grpClKept->addItemDrawable(cluster);

    CT_Line* line = new CT_Line(_step->_line_ModelName.completeName(), _res, mergedLineData);
    grpClKept->addItemDrawable(line);

    // Enveloppe convexe des points projetés perpendiculairement à la ligne
    QList<Eigen::Vector2d*> projPts;
    Eigen::Hyperplane<double, 3> plane(mergedLineData->getDirection(), mergedLineData->getP1());

    const CT_AbstractPointCloudIndex* clusterPointCloudIndex = cluster->getPointCloudIndex();
    CT_PointIterator itCI(clusterPointCloudIndex);
    while(itCI.hasNext())
    {
        itCI.next();
        const CT_Point &pt = itCI.currentPoint();

        Eigen::Vector3d projectedPt = plane.projection(pt);
        projPts.append(new Eigen::Vector2d(projectedPt(0), projectedPt(1)));
    }

    CT_Polygon2DData::orderPointsByXY(projPts);
    CT_Polygon2DData* polyData = CT_Polygon2DData::createConvexHull(projPts);
    CT_Polygon2D* poly = NULL;
    if (polyData != NULL)
    {
        poly = new CT_Polygon2D(_step->_convexProj_ModelName.completeName(), _res, polyData);
        poly->setZValue(mergedLineData->getP1()(2));
        grpClKept->addItemDrawable(poly);
    }


    // Compute diameter of the stem (using max distance between two projected points)
    QList<float> diamsMax;
    for (int ii = 0 ; ii < projPts.size() ; ii++)
    {
        const Eigen::Vector2d *pt1 = projPts.at(ii);
        for (int jj = ii + 1 ; jj < projPts.size() ; jj++)
        {
            const Eigen::Vector2d *pt2 = projPts.at(jj);
            float dist = sqrt(pow((*pt1)(0) - (*pt2)(0), 2) + pow((*pt1)(1) - (*pt2)(1), 2));
            diamsMax.append(dist);
        }
    }
    qSort(diamsMax);

    double maxDistForDiameter = 0;
    if (diamsMax.size() > 0)
    {
        maxDistForDiameter = diamsMax.last();
        bool stop = false;
        int index = diamsMax.size() - 2;
        while (index >= 0 && !stop)
        {
            double previousDiam = diamsMax.at(index);
            double ratio = (maxDistForDiameter - previousDiam) / previousDiam;
            if (ratio > _step->_maxDiamRatio)
            {
                maxDistForDiameter = previousDiam;
                index--;
            } else {
                stop = true;
            }
        }
        diamsMax.clear();
    }



    QMultiMap<int, float> diamEqs;
    for (int ii = 0 ; ii < projPts.size() ; ii++)
    {
        const Eigen::Vector2d *pt1 = projPts.at(ii);
        for (int jj = ii + 1 ; jj < projPts.size() ; jj++)
        {
            const Eigen::Vector2d *pt2 = projPts.at(jj);

            float dist = sqrt(pow((*pt1)(0) - (*pt2)(0), 2) + pow((*pt1)(1) - (*pt2)(1), 2));

            Eigen::Vector2d center;
            center(0) = ((*pt1)(0) + (*pt2)(0)) / 2.0;
            center(1) = ((*pt1)(1) + (*pt2)(1)) / 2.0;

            float nb = 0;
            for (int kk = 0 ; kk < projPts.size(); kk++)
            {
                if (kk != ii && kk != jj)
                {
                    const Eigen::Vector2d *pt3 = projPts.at(kk);
                    float distCircle = sqrt(pow(center(0) - (*pt3)(0), 2) + pow(center(1) - (*pt3)(1), 2)) - (dist / 2.0);
                    float absDistCircle = fabs(distCircle);

                    if (absDistCircle > _step->_circleDistThreshold && distCircle < 0)
                    {
                        nb -= 1;
                    } else {
                        float ratio = _step->_circleDistThreshold / absDistCircle;
                        if (ratio > 1) {ratio = 1;}
                        nb += ratio;
                    }
                }
            }
            diamEqs.insert(nb, dist);
        }
    }

    double diamEq = 0;
    if (diamEqs.size() > 0)
    {
        diamEq = diamEqs.last();
        diamEqs.clear();
    }

    if (diamEq > 0)
    {
        Eigen::Vector2d center;
        center(0) = line->getP1_X();
        center(1) = line->getP1_Y();

        CT_Circle2D *circle = new CT_Circle2D(_step->_circle_ModelName.completeName(), _res, new CT_Circle2DData(center, diamEq/2.0));
        grpClKept->addItemDrawable(circle);
    }

    ONF_StepDetectVerticalAlignments04::DistValues* distVal = computeDistVals(clusterPointCloudIndex, mergedLineData);

    line->addItemAttribute(new CT_StdItemAttributeT<double>(_step->_attMin_ModelName.completeName(),
                                                            CT_AbstractCategory::DATA_VALUE,
                                                            _res,
                                                            distVal->_min));
    line->addItemAttribute(new CT_StdItemAttributeT<double>(_step->_attQ25_ModelName.completeName(),
                                                            CT_AbstractCategory::DATA_VALUE,
                                                            _res,
                                                            distVal->_q25));
    line->addItemAttribute(new CT_StdItemAttributeT<double>(_step->_attQ50_ModelName.completeName(),
                                                            CT_AbstractCategory::DATA_VALUE,
                                                            _res,
                                                            distVal->_q50));
    line->addItemAttribute(new CT_StdItemAttributeT<double>(_step->_attQ75_ModelName.completeName(),
                                                            CT_AbstractCategory::DATA_VALUE,
                                                            _res,
                                                            distVal->_q75));
    line->addItemAttribute(new CT_StdItemAttributeT<double>(_step->_attMax_ModelName.completeName(),
                                                            CT_AbstractCategory::DATA_VALUE,
                                                            _res,
                                                            distVal->_max));
    line->addItemAttribute(new CT_StdItemAttributeT<double>(_step->_attMean_ModelName.completeName(),
                                                            CT_AbstractCategory::DATA_VALUE,
                                                            _res,
                                                            distVal->_mean));

    line->addItemAttribute(new CT_StdItemAttributeT<double>(_step->_attMaxDist_ModelName.completeName(),
                                                            CT_AbstractCategory::DATA_VALUE,
                                                            _res,
                                                            maxDistForDiameter));

    line->addItemAttribute(new CT_StdItemAttributeT<double>(_step->_attDiamEq_ModelName.completeName(),
                                                            CT_AbstractCategory::DATA_VALUE,
                                                            _res,
                                                            diamEq));

    line->addItemAttribute(new CT_StdItemAttributeT<double>(_step->_attHmax_ModelName.completeName(),
                                                            CT_AbstractCategory::DATA_VALUE,
                                                            _res,
                                                            hMaxScene));
}



ONF_StepDetectVerticalAlignments04::DistValues* ONF_StepDetectVerticalAlignments04::AlignmentsDetectorForScene::computeDistVals(const CT_AbstractPointCloudIndex* cloudIndex, CT_LineData* lineData)
{
    ONF_StepDetectVerticalAlignments04::DistValues* distVal = new ONF_StepDetectVerticalAlignments04::DistValues();

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
