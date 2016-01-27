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

#include "onf_stepdetectverticalalignments06.h"

#include "ct_itemdrawable/abstract/ct_abstractitemdrawablewithpointcloud.h"
#include "ct_itemdrawable/ct_polygon2d.h"
#include "ct_itemdrawable/ct_line.h"
#include "ct_itemdrawable/ct_pointcluster.h"
#include "ct_itemdrawable/abstract/ct_abstractpointattributesscalar.h"
#include "ct_itemdrawable/ct_stdlaspointsattributescontainer.h"
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
#define DEFin_attLAS "attLAS"


// Constructor : initialization of parameters
ONF_StepDetectVerticalAlignments06::ONF_StepDetectVerticalAlignments06(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _thresholdGPSTime = 1e-5;
    _maxCurvature = 0.25;
    _maxXYDist = 0.35;
    _thresholdZenithalAngle = 30.0;
    _minPts = 2;

    _minDiameter = 0.075;
    _maxDiameter = 1.500;
    _maxSearchRadius = 1.5;
    _resolutionForDiameterEstimation = 1;
    _ratioDbhNbPtsMax = 0.07;
    _monoLineMult = 3.0;

    _pointDistThresholdSmall = 4.0;
    _maxPhiAngleSmall = 20.0;
    _lineDistThresholdSmall = 0.4;
    _minPtsSmall = 3;
    _lineLengthRatioSmall = 0.8;

    _clusterDebugMode = false;
}

// Step description (tooltip of contextual menu)
QString ONF_StepDetectVerticalAlignments06::getStepDescription() const
{
    return tr("Détecter des alignements verticaux de points (V6)");
}

// Step detailled description
QString ONF_StepDetectVerticalAlignments06::getStepDetailledDescription() const
{
    return tr("No detailled description for this step");
}

// Step URL
QString ONF_StepDetectVerticalAlignments06::getStepURL() const
{
    //return tr("STEP URL HERE");
    return CT_AbstractStep::getStepURL(); //by default URL of the plugin
}

// Step copy method
CT_VirtualAbstractStep* ONF_StepDetectVerticalAlignments06::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new ONF_StepDetectVerticalAlignments06(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void ONF_StepDetectVerticalAlignments06::createInResultModelListProtected()
{
    CT_InResultModelGroupToCopy *resIn_res = createNewInResultModelForCopy(DEFin_res, tr("Scènes"));
    resIn_res->setZeroOrMoreRootGroup();
    resIn_res->addGroupModel("", DEFin_grp, CT_AbstractItemGroup::staticGetType(), tr("Scènes (grp)"));
    resIn_res->addItemModel(DEFin_grp, DEFin_sceneStem, CT_AbstractItemDrawableWithPointCloud::staticGetType(), tr("Scène (tiges)"));
    resIn_res->addItemModel(DEFin_grp, DEFin_attLAS, CT_StdLASPointsAttributesContainer::staticGetType(), tr("Attributs LAS"), tr("Attribut LAS"));

    resIn_res->addItemModel(DEFin_grp, DEFin_sceneAll, CT_AbstractItemDrawableWithPointCloud::staticGetType(), tr("Scène (complète)"), "",
                            CT_InAbstractModel::C_ChooseOneIfMultiple, CT_InAbstractModel::F_IsOptional);
}

// Creation and affiliation of OUT models
void ONF_StepDetectVerticalAlignments06::createOutResultModelListProtected()
{
    CT_OutResultModelGroupToCopyPossibilities *resCpy = createNewOutResultModelToCopy(DEFin_res);

    resCpy->addGroupModel(DEFin_grp, _grpCluster_ModelName, new CT_StandardItemGroup(), tr("Tiges"));

    resCpy->addItemModel(_grpCluster_ModelName, _cluster_ModelName, new CT_PointCluster(), tr("Cluster"));
    resCpy->addItemAttributeModel(_cluster_ModelName, _attMaxDistXY_ModelName, new CT_StdItemAttributeT<double>(CT_AbstractCategory::DATA_VALUE), tr("Diamètre"));
    resCpy->addItemAttributeModel(_cluster_ModelName, _attStemType_ModelName, new CT_StdItemAttributeT<int>(CT_AbstractCategory::DATA_VALUE), tr("Type"), tr("0 = petite tige ; 1 = grosse tige"));

    resCpy->addItemModel(_grpCluster_ModelName, _circle_ModelName, new CT_Circle2D(), tr("Diamètre"));
    resCpy->addItemAttributeModel(_circle_ModelName, _attMaxDistXY2_ModelName, new CT_StdItemAttributeT<double>(CT_AbstractCategory::DATA_VALUE), tr("Diamètre"));
    resCpy->addItemAttributeModel(_circle_ModelName, _attStemType2_ModelName, new CT_StdItemAttributeT<int>(CT_AbstractCategory::DATA_VALUE), tr("Type"), tr("0 = petite tige ; 1 = grosse tige"));

    resCpy->addItemModel(_grpCluster_ModelName, _line_ModelName, new CT_PointCluster(), tr("Droite ajustée"));
    resCpy->addItemAttributeModel(_line_ModelName, _attMaxDistXY3_ModelName, new CT_StdItemAttributeT<double>(CT_AbstractCategory::DATA_VALUE), tr("Diamètre"));
    resCpy->addItemAttributeModel(_line_ModelName, _attStemType3_ModelName, new CT_StdItemAttributeT<int>(CT_AbstractCategory::DATA_VALUE), tr("Type"), tr("0 = petite tige ; 1 = grosse tige"));

    if (_clusterDebugMode)
    {
        resCpy->addGroupModel(DEFin_grp, _grpClusterDebug1_ModelName, new CT_StandardItemGroup(), tr("Debug"));
        resCpy->addItemModel(_grpClusterDebug1_ModelName, _clusterDebug1_ModelName, new CT_PointCluster(), tr("Lignes de scan complètes (toutes)"));
        resCpy->addGroupModel(DEFin_grp, _grpClusterDebug2_ModelName, new CT_StandardItemGroup(), tr("Debug"));
        resCpy->addItemModel(_grpClusterDebug2_ModelName, _clusterDebug2_ModelName, new CT_PointCluster(), tr("Lignes de scan débruitées (toutes)"));
        resCpy->addGroupModel(DEFin_grp, _grpClusterDebug3_ModelName, new CT_StandardItemGroup(), tr("Debug"));
        resCpy->addItemModel(_grpClusterDebug3_ModelName, _clusterDebug3_ModelName, new CT_PointCluster(), tr("Lignes de scan (conservées)"));
    }
}

// Semi-automatic creation of step parameters DialogBox
void ONF_StepDetectVerticalAlignments06::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addTitle( tr("1- Détéction des lignes de scan :"));
    configDialog->addDouble(tr("Seuil de temps GPS pour changer de ligne de scan"), "m", 0, 1e+10, 10, _thresholdGPSTime);
    configDialog->addDouble(tr("Courbure maximale d'une ligne de scan"), "cm", 0, 1e+4, 2, _maxCurvature, 100);
    configDialog->addDouble(tr("Distance XY maximale entre points d'une ligne de scan"), "cm", 0, 1e+4, 2, _maxXYDist, 100);
    configDialog->addDouble(tr("Angle zénithal maximal pour conserver une ligne de scan"), "°", 0, 360, 2, _thresholdZenithalAngle);
    configDialog->addInt(tr("Ne conserver que les lignes de scan avec au moins"), "points", 0, 1000, _minPts);

    configDialog->addEmpty();
    configDialog->addTitle( tr("2- Estimation du diamètre :"));
    configDialog->addDouble(tr("Diamètre minimal"), "cm", 0, 1e+4, 2, _minDiameter, 100);
    configDialog->addDouble(tr("Diamètre maximal"), "cm", 0, 1e+4, 2, _maxDiameter, 100);
    configDialog->addDouble(tr("Distance de recherche des voisins"), "m", 0, 1e+4, 2, _maxSearchRadius);
    configDialog->addDouble(tr("Résolution pour la recherche de tronc"), "°", 0, 1e+4, 2, _resolutionForDiameterEstimation);
    configDialog->addDouble(tr("Ratio maximal diamètre / nb. points"), "cm", 0, 1e+4, 2, _ratioDbhNbPtsMax, 100);
    configDialog->addDouble(tr("Multiplicateur de diamètre pour les lignes de scan uniques"), "fois", 0, 1e+4, 2, _monoLineMult);

    configDialog->addEmpty();
    configDialog->addTitle( tr("3- Détéction des petites tiges (alignements) :"));
    configDialog->addDouble(tr("Distance maximum entre deux points d'une droite candidate"), "m", 0, 1000, 2, _pointDistThresholdSmall);
    configDialog->addDouble(tr("Angle zénithal maximal pour une droite candidate"), "°", 0, 180, 2, _maxPhiAngleSmall);
    configDialog->addDouble(tr("Distance maximum XY entre deux droites candidates à agréger"), "m", 0, 1000, 2, _lineDistThresholdSmall);
    configDialog->addInt(tr("Nombre de points minimum dans un cluster"), "", 2, 1000, _minPtsSmall);
    configDialog->addDouble(tr("Pourcentage maximum de la longueur de segment sans points"), "%", 0, 100, 0, _lineLengthRatioSmall, 100);

    configDialog->addEmpty();
    configDialog->addBool(tr("Mode Debug Clusters"), "", "", _clusterDebugMode);
}



void ONF_StepDetectVerticalAlignments06::compute()
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

void ONF_StepDetectVerticalAlignments06::AlignmentsDetectorForScene::detectAlignmentsForScene(CT_StandardItemGroup* grp)
{
    CT_PointAccessor pointAccessor;
    double thresholdZenithalAngleRadians = M_PI * _step->_thresholdZenithalAngle / 180.0;

    const CT_AbstractItemDrawableWithPointCloud* sceneStem = (CT_AbstractItemDrawableWithPointCloud*)grp->firstItemByINModelName(_step, DEFin_sceneStem);
    const CT_StdLASPointsAttributesContainer* attributeLAS = (CT_StdLASPointsAttributesContainer*)grp->firstItemByINModelName(_step, DEFin_attLAS);

    if (sceneStem != NULL && attributeLAS != NULL)
    {
        // Retrieve attributes
        QHashIterator<CT_LasDefine::LASPointAttributesType, CT_AbstractPointAttributesScalar *> it(attributeLAS->lasPointsAttributes());
        if (!it.hasNext()) {return;}

        CT_AbstractPointAttributesScalar *firstAttribute = it.next().value();
        if (firstAttribute == NULL) {return;}

        const CT_AbstractPointCloudIndex* pointCloudIndexLAS = firstAttribute->getPointCloudIndex();
        if (pointCloudIndexLAS == NULL) {return;}

        CT_AbstractPointAttributesScalar* attributeGPS          = (CT_AbstractPointAttributesScalar*)attributeLAS->pointsAttributesAt(CT_LasDefine::GPS_Time);
        CT_AbstractPointAttributesScalar* attributeIntensity     = (CT_AbstractPointAttributesScalar*)attributeLAS->pointsAttributesAt(CT_LasDefine::Intensity);
        CT_AbstractPointAttributesScalar* attributeLineOfFlight = (CT_AbstractPointAttributesScalar*)attributeLAS->pointsAttributesAt(CT_LasDefine::Point_Source_ID);

        if (attributeIntensity == NULL || attributeGPS == NULL || attributeLineOfFlight == NULL) {return;}

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
            size_t localIndex = pointCloudIndexLAS->indexOf(index);

            double gpsTime = 0; // Récupération du temps GPS pour le point 1
            if (localIndex < pointCloudIndexLAS->size())
            {
                gpsTime = attributeGPS->dValueAt(localIndex);
                sortedIndices.insert(gpsTime, index);
            }
        }

        // List of not clusterized points
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
        QList<QList<size_t> > simplifiedLinesOfScan;
        QListIterator<QList<size_t> > itLines(linesOfScan);
        while (itLines.hasNext())
        {
            const QList<size_t> &completeLine = itLines.next();


            // Archive detection of lines of scan (all complete)
            if (_step->_clusterDebugMode)
            {
                CT_PointCluster* cluster = new CT_PointCluster(_step->_clusterDebug1_ModelName.completeName(), _res);
                for (int j = 0 ; j < completeLine.size() ; j++)
                {
                    cluster->addPoint(completeLine.at(j));
                }
                CT_StandardItemGroup* grpClKept = new CT_StandardItemGroup(_step->_grpClusterDebug1_ModelName.completeName(), _res);
                grp->addGroup(grpClKept);
                grpClKept->addItemDrawable(cluster);
            }


            // Find a reference point on the line of scan (max intensity point)
            double maxIntensity = -std::numeric_limits<double>::max();
            int refi = 0;
            for (int i = 0 ; i < completeLine.size() ; i++)
            {
                size_t localIndex = pointCloudIndexLAS->indexOf(completeLine.at(i));
                double intensity = maxIntensity;
                if (localIndex < pointCloudIndexLAS->size()) {intensity = attributeIntensity->dValueAt(localIndex);}

                if (intensity > maxIntensity)
                {
                    maxIntensity = intensity;
                    refi = i;
                }
            }

            int newRefI = 0;
            // Test all lines linking reference point to another point, and keep the best one: max number of points, or if equal min length
            CT_Point p1 = pointAccessor.constPointAt(completeLine.at(refi));
            QList<size_t> bestSimplifiedLine;
            double bestSimplifiedLineLength = std::numeric_limits<double>::max();
            for (int i = 0 ; i < completeLine.size() ; i++)
            {
                if (i != refi)
                {
                    CT_Point p2 = pointAccessor.constPointAt(completeLine.at(i));
                    Eigen::Vector3d direction = p2 - p1;
                    direction.normalize();

                    QList<size_t> simplifiedLine;

                    for (int j = 0 ; j < completeLine.size() ; j++)
                    {
                        if (j == refi)
                        {
                            simplifiedLine.append(completeLine.at(j));
                            newRefI = simplifiedLine.size() - 1;
                        } else if (j == i) {
                            simplifiedLine.append(completeLine.at(j));
                        } else {
                            size_t index = completeLine.at(j);
                            CT_Point p3 = pointAccessor.constPointAt(index);
                            double curv = CT_MathPoint::distancePointLine(p3, direction, p1);
                            if (curv < _step->_maxCurvature)
                            {
                                simplifiedLine.append(index);
                            }
                        }
                    }

                    // Compute bestSimplifiedLineLength
                    const size_t index01 = simplifiedLine.first();
                    const size_t index02 = simplifiedLine.last();

                    CT_Point p01 = pointAccessor.constPointAt(index01);
                    CT_Point p02 = pointAccessor.constPointAt(index02);

                    double length = sqrt(pow(p01(0) - p02(0), 2) + pow(p01(1) - p02(1), 2) + pow(p01(2) - p02(2), 2));

                    // Compare with previous best simplified line
                    if (simplifiedLine.size() > bestSimplifiedLine.size())
                    {
                        bestSimplifiedLine = simplifiedLine;
                        bestSimplifiedLineLength = length;
                    } else if (simplifiedLine.size() == bestSimplifiedLine.size() && length < bestSimplifiedLineLength)
                    {
                        bestSimplifiedLine = simplifiedLine;
                        bestSimplifiedLineLength = length;
                    }
                }
            }

            QList<size_t> finalSimplifiedLine;
            if (bestSimplifiedLine.size() > 1)
            {

                // Delete extremities from reference points, if a distXY between succesive points > threshold
                int firstI = 0;
                bool stop = false;
                for (int i = newRefI - 1 ; !stop && i >= 0 ; i--)
                {
                    const size_t index01 = bestSimplifiedLine.at(i);
                    const size_t index02 = bestSimplifiedLine.at(i+1);

                    CT_Point p01 = pointAccessor.constPointAt(index01);
                    CT_Point p02 = pointAccessor.constPointAt(index02);

                    double distXY = sqrt(pow(p01(0) - p02(0), 2) + pow(p01(1) - p02(1), 2));
                    if (distXY > _step->_maxXYDist)
                    {
                        firstI = i + 1;
                        stop = true;
                    }
                }

                int lastI = bestSimplifiedLine.size() - 1;
                stop = false;
                for (int i = newRefI + 1 ; !stop && i < bestSimplifiedLine.size() ; i++)
                {
                    const size_t index01 = bestSimplifiedLine.at(i);
                    const size_t index02 = bestSimplifiedLine.at(i-1);

                    CT_Point p01 = pointAccessor.constPointAt(index01);
                    CT_Point p02 = pointAccessor.constPointAt(index02);

                    double distXY = sqrt(pow(p01(0) - p02(0), 2) + pow(p01(1) - p02(1), 2));
                    if (distXY > _step->_maxXYDist)
                    {
                        lastI = i - 1;
                        stop = true;
                    }
                }

                // If it remains points in the line, create cluster
                if ((lastI - firstI) > 0)
                {
                    for (int i = firstI ; i <= lastI ; i++)
                    {
                        finalSimplifiedLine.append(bestSimplifiedLine.at(i));
                    }
                    simplifiedLinesOfScan.append(finalSimplifiedLine);
                }
            }

            // All noise points are added to isolatedPointsIndices
            for (int i = 0 ; i < completeLine.size() ; i++)
            {
                size_t index = completeLine.at(1);
                if (!finalSimplifiedLine.contains(index))
                {
                    isolatedPointIndices.append(index);
                }

            }

        }
        linesOfScan.clear();

        // Archive detection of lines of scan (all denoised)
        if (_step->_clusterDebugMode)
        {
            for (int i = 0 ; i < simplifiedLinesOfScan.size() ; i++)
            {
                const QList<size_t> &line = simplifiedLinesOfScan.at(i);
                CT_PointCluster* cluster = new CT_PointCluster(_step->_clusterDebug2_ModelName.completeName(), _res);
                for (int j = 0 ; j < line.size() ; j++)
                {
                    cluster->addPoint(line.at(j));
                }
                CT_StandardItemGroup* grpClKept = new CT_StandardItemGroup(_step->_grpClusterDebug2_ModelName.completeName(), _res);
                grp->addGroup(grpClKept);
                grpClKept->addItemDrawable(cluster);
            }
        }

        // Remove clusters with 1 point or with phi > maxPhi
        QList<ScanLineData*> keptLinesOfScan;
        for (int i = 0 ; i < simplifiedLinesOfScan.size() ; i++)
        {
            QList<size_t> simplifiedLine = simplifiedLinesOfScan.at(i);

            if (simplifiedLine.size() < _step->_minPts)
            {
                for (int j = 0 ; j < simplifiedLine.size() ; j++)
                {
                    isolatedPointIndices.append(simplifiedLine.at(j));
                }
            } else {

                const size_t index1 = simplifiedLine.first();
                const size_t index2 = simplifiedLine.last();

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
                    for (int j = 0 ; j < simplifiedLine.size() ; j++)
                    {
                        isolatedPointIndices.append(simplifiedLine.at(j));
                    }
                } else {
                    double length = sqrt(pow(p1(0) - p2(0), 2) + pow(p1(1) - p2(1), 2) + pow(p1(2) - p2(2), 2));
                    keptLinesOfScan.append(new ScanLineData(simplifiedLine, length, (p1(0) + p2(0)) / 2.0,  (p1(1) + p2(1)) / 2.0, std::min(p1(2), p2(2))));
                }
            }
        }
        simplifiedLinesOfScan.clear();


        // Archive detection of lines of scan (conserved)
        if (_step->_clusterDebugMode)
        {
            for (int i = 0 ; i < keptLinesOfScan.size() ; i++)
            {
                ScanLineData* line = keptLinesOfScan.at(i);
                CT_PointCluster* cluster = new CT_PointCluster(_step->_clusterDebug3_ModelName.completeName(), _res);
                for (int j = 0 ; j < line->size() ; j++)
                {
                    cluster->addPoint(line->at(j));
                }
                CT_StandardItemGroup* grpClKept = new CT_StandardItemGroup(_step->_grpClusterDebug3_ModelName.completeName(), _res);
                grp->addGroup(grpClKept);
                grpClKept->addItemDrawable(cluster);
            }
        }

        // Sorting list of lines by point Number and if equals, by length
        qSort(keptLinesOfScan.begin(), keptLinesOfScan.end(), ONF_StepDetectVerticalAlignments06::orderLinesByAscendingNumberAndLength);

        // Compute diameters using neighbourhoud
        QList<CT_Circle2D*> circles;
        while (!keptLinesOfScan.isEmpty())
        {
            ScanLineData *mainLine = keptLinesOfScan.takeLast();

            int mainLineOfFlight = -1;
            QList<CT_Point> mainLinePoints;
            for (int j = 0 ; j < mainLine->size() ; j++)
            {
                size_t index = mainLine->at(j);
                const CT_Point& point = pointAccessor.constPointAt(index);
                mainLinePoints.append(point);

                size_t localIndex = pointCloudIndexLAS->indexOf(index);
                if (localIndex < pointCloudIndexLAS->size())
                {
                    mainLineOfFlight = attributeLineOfFlight->dValueAt(localIndex);
                }
            }

            // Search for neighbours
            QList<ScanLineData*> neighbourLines;
            QList<CT_Point> neighbourPoints;
            QList<int> neighbourPointsOfDifferentsLinesOfFlight;
            for (int i = 0 ; i < keptLinesOfScan.size() ; i++)
            {
                ScanLineData* testedLine = keptLinesOfScan.at(i);
                double distXY = sqrt(pow(mainLine->_centerX - testedLine->_centerX, 2) + pow(mainLine->_centerY - testedLine->_centerY, 2));

                if (distXY < _step->_maxSearchRadius)
                {
                    neighbourLines.append(testedLine);
                    keptLinesOfScan.removeAt(i--);

                    for (int j = 0 ; j < testedLine->size() ; j++)
                    {
                        size_t index = testedLine->at(j);
                        const CT_Point& point = pointAccessor.constPointAt(index);
                        neighbourPoints.append(point);

                        size_t localIndex = pointCloudIndexLAS->indexOf(index);
                        int lineOfFlight = 0;
                        if (localIndex < pointCloudIndexLAS->size())
                        {
                            lineOfFlight = attributeLineOfFlight->dValueAt(localIndex);
                        }

                        if (lineOfFlight != mainLineOfFlight)
                        {
                            neighbourPointsOfDifferentsLinesOfFlight.append(neighbourPoints.size() - 1);
                        }
                    }
                }
            }           

            double diameter = 0.0;
            Eigen::Vector3d bestDirection(0, 0, 1);
            Eigen::Vector3d center(mainLine->_centerX, mainLine->_centerY, mainLine->_centerZ);

            if (!neighbourPointsOfDifferentsLinesOfFlight.isEmpty())
            {
                double bestScore = 0;
                // For each direction
                for (float zenithal = 0 ; zenithal <= _step->_thresholdZenithalAngle ; zenithal += _step->_resolutionForDiameterEstimation)
                {
                    float zeniRad = M_PI * zenithal / 180.0;
                    for (float azimut = 0 ; azimut <= 360 ; azimut += _step->_resolutionForDiameterEstimation)
                    {
                        float azimRad = M_PI * azimut / 180.0;

                        // Compute projected points
                        float dx, dy, dz;
                        CT_SphericalLine3D::convertToCartesianCoordinates(zeniRad, azimRad, 1, dx, dy, dz);
                        Eigen::Vector3d direction(dx, dy, dz);

                        QList<Eigen::Vector2d*> projPtsForMainLine;
                        QList<Eigen::Vector2d*> projPtsForNeighbourPoints;
                        Eigen::Hyperplane<double, 3> plane(direction, center);

                        // Compute projected points for main Line
                        for (int i = 0 ; i < mainLinePoints.size() ; i++)
                        {
                            Eigen::Vector3d projectedPt = plane.projection(mainLinePoints[i]);
                            projPtsForMainLine.append(new Eigen::Vector2d(projectedPt(0), projectedPt(1)));
                        }

                        // Compute projected points for neighbours Lines
                        for (int i = 0 ; i < neighbourPoints.size() ; i++)
                        {
                            Eigen::Vector3d projectedPt = plane.projection(neighbourPoints[i]);
                            projPtsForNeighbourPoints.append(new Eigen::Vector2d(projectedPt(0), projectedPt(1)));
                        }

                        // Test all possibile diameter between one point from the main line, and one point from the neighbours lines
                        for (int i = 0 ; i < projPtsForMainLine.size() ; i++)
                        {
                            Eigen::Vector2d* projectedPt = projPtsForMainLine.at(i);

                            for (int j = 0 ; j < neighbourPointsOfDifferentsLinesOfFlight.size() ; j++)
                            {
                                int currentNeighbourIndex = neighbourPointsOfDifferentsLinesOfFlight.at(j);
                                Eigen::Vector2d* neighbourProjectedPoint = projPtsForNeighbourPoints.at(currentNeighbourIndex);

                                double candidateDiameter = sqrt(pow ((*projectedPt)(0) - (*neighbourProjectedPoint)(0), 2) + pow ((*projectedPt)(1) - (*neighbourProjectedPoint)(1), 2));

                                // If candidate diameter is in the good range
                                if (candidateDiameter <= _step->_maxDiameter && candidateDiameter >= _step->_minDiameter)
                                {
                                    double candidateRadius = candidateDiameter / 2.0;
                                    double candidateScore = 0;
                                    Eigen::Vector2d circleCenter(((*projectedPt)(0) + (*neighbourProjectedPoint)(0)) / 2.0, ((*projectedPt)(1) + (*neighbourProjectedPoint)(1)) / 2.0);


                                    // test how many other points (from mainLine) are in the "valid zone" around diameter
                                    for (int k = 0 ; k < projPtsForMainLine.size() ; k++)
                                    {
                                        Eigen::Vector2d* point = projPtsForMainLine.at(k);
                                        double distXY = sqrt(pow (circleCenter(0) - (*point)(0), 2) + pow (circleCenter(1) - (*point)(1), 2));

                                        if (distXY <= candidateRadius)
                                        {
                                            candidateScore += distXY / candidateRadius;
                                        }
                                    }

                                    // test how many other points (from neighbours lines) are in the "valid zone" around diameter
                                    for (int k = 0 ; k < projPtsForNeighbourPoints.size() ; k++)
                                    {
                                        Eigen::Vector2d* point = projPtsForNeighbourPoints.at(k);
                                        double distXY = sqrt(pow (circleCenter(0) - (*point)(0), 2) + pow (circleCenter(1) - (*point)(1), 2));

                                        if (distXY <= candidateRadius)
                                        {
                                            candidateScore += distXY / candidateRadius;
                                        }
                                    }

                                    if (candidateScore > bestScore)
                                    {
                                        bestScore = candidateScore;
                                        diameter = candidateDiameter;
                                        bestDirection = direction;
                                    }
                                }
                            }
                        }

                        qDeleteAll(projPtsForMainLine);
                        qDeleteAll(projPtsForNeighbourPoints);
                    }
                }
            }

            CT_PointCluster* cluster = new CT_PointCluster(_step->_cluster_ModelName.completeName(), _res);
            // Add points of the main line
            for (int j = 0 ; j < mainLine->size() ; j++)
            {
                size_t index = mainLine->at(j);
                cluster->addPoint(index);
            }

            int type = 1; // Multi lines of scans, ok
            double centerX = mainLine->_centerX;
            double centerY = mainLine->_centerY;
            double centerZ = mainLine->_centerZ;

            // if not valid diameter, compute vertical projection diameter
            if (diameter >= _step->_maxDiameter || diameter <= 0)
            {
                const size_t index1 = mainLine->first();
                const size_t index2 = mainLine->last();

                CT_Point p1 = pointAccessor.constPointAt(index1);
                CT_Point p2 = pointAccessor.constPointAt(index2);

                Eigen::Vector3d direction;
                direction = p2 - p1;
                direction.normalize();

                diameter = _step->_monoLineMult * computeDiameterAlongLine(cluster, direction, p1);

                type = 2; // Mono line of scan or excessive diameter
                bestDirection = direction;

                centerX = p1(0);
                centerY = p1(1);
                centerZ = p1(2);

                if (bestDirection(2) < 0)
                {
                    bestDirection = -bestDirection;
                    centerX = p2(0);
                    centerY = p2(1);
                    centerZ = p2(2);
                }
            }

            // Add points of the neighbours lines
            for (int i = 0 ; i < neighbourLines.size() ; i++)
            {
                ScanLineData* testedLine = neighbourLines.at(i);
                for (int j = 0 ; j < testedLine->size() ; j++)
                {
                    size_t index = testedLine->at(j);
                    if (type == 1)
                    {
                        cluster->addPoint(index);
                    } else {
                        isolatedPointIndices.append(index);
                    }
                }
            }

            // Add to result
            int nbPts = cluster->getPointCloudIndexSize();
            if (nbPts == 0 || (diameter / nbPts) > _step->_ratioDbhNbPtsMax)
            {
                diameter = _step->_minDiameter;
                type = 3; // Excessive diameter
            }
            circles.append(addClusterToResult(grp, cluster, diameter, type, centerX, centerY,  centerZ, mainLine->_length, bestDirection));

            delete mainLine;
            qDeleteAll(neighbourLines);
        }

        ////////////////////////////////////////////////////
        /// Detection of small stems: points alignements ///
        ////////////////////////////////////////////////////

        double maxPhiRadians = M_PI*_step->_maxPhiAngleSmall/180.0;

        // Eliminate isolated point to close from already deteted diameters
        for (int iso = 0 ; iso < isolatedPointIndices.size(); iso++)
        {
            size_t index = isolatedPointIndices.at(iso);
            CT_Point point = pointAccessor.constPointAt(index);

            for (int i = 0 ; i < circles.size() ; i++)
            {
                CT_Circle2D* circle = circles.at(i);
                double dist = sqrt(pow(circle->getCenterX() - point(0), 2) + pow(circle->getCenterY() - point(1), 2));
                if (dist < _step->_maxSearchRadius)
                {
                    isolatedPointIndices.removeAt(iso--);
                }
            }
        }


        QList<ONF_StepDetectVerticalAlignments06::LineData*> candidateLines;
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
                            candidateLines.append(new ONF_StepDetectVerticalAlignments06::LineData(pointLow, pointHigh, index1, index2, phi, sceneStem->minZ(), sceneStem->maxZ()));
                        }
                    }
                }
            }
        }

        // Affiliation des lignes proches
        findNeighborLines(candidateLines, _step->_lineDistThresholdSmall);

        // Tri par NeighborCount descendant
        qSort(candidateLines.begin(), candidateLines.end(), ONF_StepDetectVerticalAlignments06::orderByDescendingNeighborCount);

        // Constitution des clusters de points alignés
        QList<size_t> insertedPoints;

        for (int i = 0 ; i < candidateLines.size() ; i++)
        {
            ONF_StepDetectVerticalAlignments06::LineData* candidateLine = candidateLines.at(i);
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

                QList<ONF_StepDetectVerticalAlignments06::LineData*> &neighborLines = candidateLine->_neighbors;
                for (int j = 0 ; j < neighborLines.size() ; j++)
                {
                    ONF_StepDetectVerticalAlignments06::LineData* neighborLine = neighborLines.at(j);
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
                                const Eigen::Vector3d &center = fittedLineData->getP1();
                                double diameter = computeDiameterAlongLine(cluster, fittedLineData->getDirection(), center);

                                int nbPts = cluster->getPointCloudIndexSize();
                                int type = 4; // Alignement, ok
                                if (nbPts == 0 || (diameter / nbPts) > _step->_ratioDbhNbPtsMax)
                                {
                                    diameter = _step->_minDiameter;
                                        type = 5; // Alignement, excessive diameter
                                }
                                addClusterToResult(grp, cluster, diameter, type, center(0), center(1),  center(2), fittedLineData->length(), fittedLineData->getDirection());
                        } else {
                            delete cluster;
                        }
                        delete fittedLineData;
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


double ONF_StepDetectVerticalAlignments06::AlignmentsDetectorForScene::computeDiameterAlongLine(CT_PointCluster* cluster, const Eigen::Vector3d& direction, const Eigen::Vector3d& origin)
{
    QList<Eigen::Vector2d> projPts;
    Eigen::Hyperplane<double, 3> plane(direction, origin);

    CT_PointIterator itP(cluster->getPointCloudIndex());
    while(itP.hasNext())
    {
        const CT_Point &point = itP.next().currentPoint();
        Eigen::Vector3d projectedPt = plane.projection(point);
        projPts.append(Eigen::Vector2d(projectedPt(0), projectedPt(1)));
    }

    double maxDist = 0;
    for (int i = 0 ; i < projPts.size() ; i++)
    {
        const Eigen::Vector2d& point1 = projPts.at(i);
        for (int j = i+1 ; j < projPts.size() ; j++)
        {
            const Eigen::Vector2d& point2 = projPts.at(j);
            double dist = sqrt(pow(point1(0) - point2(0), 2) + pow(point1(1) - point2(1), 2));
            if (dist > maxDist) {maxDist = dist;}
        }
    }

    if (maxDist < _step->_minDiameter) {maxDist = _step->_minDiameter;}
    if (maxDist > _step->_maxDiameter) {maxDist = _step->_maxDiameter;}

    return maxDist;
}


CT_Circle2D *ONF_StepDetectVerticalAlignments06::AlignmentsDetectorForScene::addClusterToResult(CT_StandardItemGroup* grp, CT_PointCluster* cluster, double diameter, int type, double centerX, double centerY, double centerZ, double length, const Eigen::Vector3d& direction)
{
    Eigen::Vector2d center2D(centerX, centerY);
    Eigen::Vector3d center3D(centerX, centerY, centerZ);

    CT_StandardItemGroup* grpClKept = new CT_StandardItemGroup(_step->_grpCluster_ModelName.completeName(), _res);
    grp->addGroup(grpClKept);


    cluster->addItemAttribute(new CT_StdItemAttributeT<double>(_step->_attMaxDistXY_ModelName.completeName(), CT_AbstractCategory::DATA_VALUE, _res, diameter*100.0));
    cluster->addItemAttribute(new CT_StdItemAttributeT<int>(_step->_attStemType_ModelName.completeName(), CT_AbstractCategory::DATA_VALUE, _res, type));
    grpClKept->addItemDrawable(cluster);

    CT_Circle2D *circle = new CT_Circle2D(_step->_circle_ModelName.completeName(), _res, new CT_Circle2DData(center2D, diameter/2.0));
    circle->addItemAttribute(new CT_StdItemAttributeT<double>(_step->_attMaxDistXY2_ModelName.completeName(), CT_AbstractCategory::DATA_VALUE, _res, diameter*100.0));
    circle->addItemAttribute(new CT_StdItemAttributeT<int>(_step->_attStemType2_ModelName.completeName(), CT_AbstractCategory::DATA_VALUE, _res, type));
    grpClKept->addItemDrawable(circle);

    CT_Line* line = new CT_Line(_step->_line_ModelName.completeName(), _res, new CT_LineData(center3D, center3D + length*direction));
    line->addItemAttribute(new CT_StdItemAttributeT<double>(_step->_attMaxDistXY3_ModelName.completeName(), CT_AbstractCategory::DATA_VALUE, _res, diameter*100.0));
    line->addItemAttribute(new CT_StdItemAttributeT<int>(_step->_attStemType3_ModelName.completeName(), CT_AbstractCategory::DATA_VALUE, _res, type));
    grpClKept->addItemDrawable(line);

    return(circle);
}


void ONF_StepDetectVerticalAlignments06::AlignmentsDetectorForScene::findNeighborLines(QList<ONF_StepDetectVerticalAlignments06::LineData*> candidateLines, double distThreshold)
{
    for (int i1 = 0 ; i1 < candidateLines.size() ; i1++)
    {
        ONF_StepDetectVerticalAlignments06::LineData* line1 = candidateLines.at(i1);

        for (int i2 = i1+1 ; i2 < candidateLines.size() ; i2++)
        {
            ONF_StepDetectVerticalAlignments06::LineData* line2 = candidateLines.at(i2);

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
