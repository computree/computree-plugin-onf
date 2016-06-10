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

#include "onf_stepdetectverticalalignments07.h"


#include "ct_itemdrawable/abstract/ct_abstractitemdrawablewithpointcloud.h"
#include "ct_itemdrawable/ct_polygon2d.h"
#include "ct_itemdrawable/ct_line.h"
#include "ct_itemdrawable/ct_pointcluster.h"
#include "ct_itemdrawable/ct_circle2d.h"
#include "ct_itemdrawable/abstract/ct_abstractareashape2d.h"
#include "ctliblas/itemdrawable/las/ct_stdlaspointsattributescontainer.h"
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

#include <stdlib.h>

// Alias for indexing models
#define DEFin_res "res"
#define DEFin_grp "grp"
#define DEFin_sceneStem "sceneStem"
#define DEFin_sceneAll "sceneAll"
#define DEFin_attLAS "attLAS"
#define DEFin_clusters "clusters"
#define DEFin_grpAllometry "grpAllo"
#define DEFin_circleAllometry "circleAllo"
#define DEF_SearchInArea   "emprise"



// Constructor : initialization of parameters
ONF_StepDetectVerticalAlignments07::ONF_StepDetectVerticalAlignments07(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _thresholdGPSTime = 1e-5;
    _maxCurvature = 0.25;
    _maxXYDist = 0.35;
    _thresholdZenithalAngle = 30.0;
    _minPts = 2;

    _minDiameter = 0.075;
    _maxDiameter = 1.500;
    _maxDiameterForUnderstorey = 0.225;
    _maxSearchRadiusByPts = 0.35;
    _maxLineSpacing = 0.07; // 0.15
    _resolutionForDiameterEstimation = 1;
    _zenithalAngleMax = 10;
    _ratioRadius = 0.8;
    _applySigmoid = true;
    _sigmoidCoefK = 10.0;
    _sigmoidX0 = 0.75;

    _ratioDbhNbPtsMax = 0.11; // 0.07
    _monoLineMult = 3.0;

    _pointDistThresholdSmall = 4.0;
    _maxPhiAngleSmall = 20.0;
    _lineDistThresholdSmall = 0.4;
    _minPtsSmall = 3;
    _lineLengthRatioSmall = 0.8;

    _zMinVerticalContinuity = 5;
    _zMaxVerticalContinuity = 15;
    _minRadiusForVerticalConstinuity = 2.0;
    _maxZDistForVerticalConstinuity = 3.0;


    _clusterDebugMode = false;
    _retrieveLostLinesParts = true;
}

// Step description (tooltip of contextual menu)
QString ONF_StepDetectVerticalAlignments07::getStepDescription() const
{
    return tr("Détecter des alignements verticaux de points (V7)");
}

// Step detailled description
QString ONF_StepDetectVerticalAlignments07::getStepDetailledDescription() const
{
    return tr("No detailled description for this step");
}

// Step URL
QString ONF_StepDetectVerticalAlignments07::getStepURL() const
{
    //return tr("STEP URL HERE");
    return CT_AbstractStep::getStepURL(); //by default URL of the plugin
}

// Step copy method
CT_VirtualAbstractStep* ONF_StepDetectVerticalAlignments07::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new ONF_StepDetectVerticalAlignments07(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void ONF_StepDetectVerticalAlignments07::createInResultModelListProtected()
{
    CT_InResultModelGroupToCopy *resIn_res = createNewInResultModelForCopy(DEFin_res, tr("Scènes"));
    resIn_res->setZeroOrMoreRootGroup();
    resIn_res->addGroupModel("", DEFin_grp, CT_AbstractItemGroup::staticGetType(), tr("Scènes (grp)"));
    resIn_res->addItemModel(DEFin_grp, DEFin_attLAS, CT_StdLASPointsAttributesContainer::staticGetType(), tr("Attributs LAS"), tr("Attribut LAS"));
    resIn_res->addItemModel(DEFin_grp, DEFin_sceneAll, CT_AbstractItemDrawableWithPointCloud::staticGetType(), tr("Scène (complète)"));
    resIn_res->addItemModel(DEFin_grp, DEFin_sceneStem, CT_AbstractItemDrawableWithPointCloud::staticGetType(), tr("Scène (tiges)"));
    resIn_res->addItemModel(DEFin_grp, DEF_SearchInArea, CT_AbstractAreaShape2D::staticGetType(),
                            tr("Emprise"), "", CT_InAbstractModel::C_ChooseOneIfMultiple, CT_InAbstractModel::F_IsOptional);

}

// Creation and affiliation of OUT models
void ONF_StepDetectVerticalAlignments07::createOutResultModelListProtected()
{
    CT_OutResultModelGroupToCopyPossibilities *resCpy = createNewOutResultModelToCopy(DEFin_res);

    if(resCpy != NULL) {
        resCpy->addGroupModel(DEFin_grp, _grpCluster_ModelName, new CT_StandardItemGroup(), tr("Tiges"));

        resCpy->addItemModel(_grpCluster_ModelName, _cluster_ModelName, new CT_PointCluster(), tr("Cluster"));
        resCpy->addItemAttributeModel(_cluster_ModelName, _attMaxDistXY_ModelName, new CT_StdItemAttributeT<double>(CT_AbstractCategory::DATA_VALUE), tr("Diamètre"));
        resCpy->addItemAttributeModel(_cluster_ModelName, _attScore_ModelName, new CT_StdItemAttributeT<double>(CT_AbstractCategory::DATA_VALUE), tr("Score"));
        resCpy->addItemAttributeModel(_cluster_ModelName, _attStemType_ModelName, new CT_StdItemAttributeT<int>(CT_AbstractCategory::DATA_VALUE), tr("Type"), tr("0 = petite tige ; 1 = grosse tige"));

        resCpy->addItemModel(_grpCluster_ModelName, _line_ModelName, new CT_PointCluster(), tr("Droite ajustée"));
        resCpy->addItemAttributeModel(_line_ModelName, _attMaxDistXY3_ModelName, new CT_StdItemAttributeT<double>(CT_AbstractCategory::DATA_VALUE), tr("Diamètre"));
        resCpy->addItemAttributeModel(_line_ModelName, _attScore3_ModelName, new CT_StdItemAttributeT<double>(CT_AbstractCategory::DATA_VALUE), tr("Score"));
        resCpy->addItemAttributeModel(_line_ModelName, _attStemType3_ModelName, new CT_StdItemAttributeT<int>(CT_AbstractCategory::DATA_VALUE), tr("Type"), tr("0 = petite tige ; 1 = grosse tige"));

        resCpy->addItemModel(_grpCluster_ModelName, _circle_ModelName, new CT_Circle2D(), tr("Diamètre"));
        resCpy->addItemAttributeModel(_circle_ModelName, _attMaxDistXY2_ModelName, new CT_StdItemAttributeT<double>(CT_AbstractCategory::DATA_VALUE), tr("Diamètre"));
        resCpy->addItemAttributeModel(_circle_ModelName, _attScore2_ModelName, new CT_StdItemAttributeT<double>(CT_AbstractCategory::DATA_VALUE), tr("Score"));
        resCpy->addItemAttributeModel(_circle_ModelName, _attStemType2_ModelName, new CT_StdItemAttributeT<int>(CT_AbstractCategory::DATA_VALUE), tr("Type"), tr("0 = petite tige ; 1 = grosse tige"));
        resCpy->addItemAttributeModel(_circle_ModelName, _attCorrectedDiameter_ModelName, new CT_StdItemAttributeT<double>(CT_AbstractCategory::DATA_VALUE), tr("Diamètre corrigé"));
        resCpy->addItemAttributeModel(_circle_ModelName, _attCorrectedFlag_ModelName, new CT_StdItemAttributeT<int>(CT_AbstractCategory::DATA_VALUE), tr("Flag Allométrie"));


        if (_clusterDebugMode)
        {
            resCpy->addGroupModel(DEFin_grp, _grpClusterDebug1_ModelName, new CT_StandardItemGroup(), tr("Debug (toutes)"));
            resCpy->addItemModel(_grpClusterDebug1_ModelName, _clusterDebug1_ModelName, new CT_PointCluster(), tr("Lignes de scan complètes (toutes)"));
            resCpy->addGroupModel(DEFin_grp, _grpClusterDebug2_ModelName, new CT_StandardItemGroup(), tr("Debug (débruitées)"));
            resCpy->addItemModel(_grpClusterDebug2_ModelName, _clusterDebug2_ModelName, new CT_PointCluster(), tr("Lignes de scan débruitées (toutes)"));
            resCpy->addGroupModel(DEFin_grp, _grpClusterDebug3_ModelName, new CT_StandardItemGroup(), tr("Debug (conservées)"));
            resCpy->addItemModel(_grpClusterDebug3_ModelName, _clusterDebug3_ModelName, new CT_PointCluster(), tr("Lignes de scan (conservées)"));
        }
    }
}

// Semi-automatic creation of step parameters DialogBox
void ONF_StepDetectVerticalAlignments07::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addTitle( tr("1- Détéction des lignes de scan :"));
    configDialog->addDouble(tr("Seuil de temps GPS pour changer de ligne de scan"), "m", 0, 1e+10, 10, _thresholdGPSTime);
    configDialog->addDouble(tr("Courbure maximale d'une ligne de scan"), "cm", 0, 1e+4, 2, _maxCurvature, 100);
    configDialog->addDouble(tr("Distance XY maximale entre points d'une ligne de scan"), "cm", 0, 1e+4, 2, _maxXYDist, 100);
    configDialog->addDouble(tr("Angle zénithal maximal pour conserver une ligne de scan"), "°", 0, 360, 2, _thresholdZenithalAngle);
    configDialog->addInt(tr("Ne conserver que les lignes de scan avec au moins"), "points", 0, 1000, _minPts);
    configDialog->addBool(tr("Récupérer les parties de lignes perdues"), "", "", _retrieveLostLinesParts);

    configDialog->addEmpty();
    configDialog->addTitle( tr("2- Estimation du diamètre :"));
    configDialog->addDouble(tr("Diamètre minimal"), "cm", 0, 1e+4, 2, _minDiameter, 100);
    configDialog->addDouble(tr("Diamètre maximal"), "cm", 0, 1e+4, 2, _maxDiameter, 100);
    configDialog->addDouble(tr("Diamètre maximal des tiges de sous-étage"), "cm", 0, 1e+4, 2, _maxDiameterForUnderstorey, 100);
    configDialog->addDouble(tr("Distance de recherche des voisins"), "m / pts", 0, 1e+4, 2, _maxSearchRadiusByPts);
    configDialog->addDouble(tr("Ecartement maximal"), "cm / m", 0, 1e+4, 2, _maxLineSpacing, 100);
    configDialog->addDouble(tr("Résolution pour la recherche de tronc"), "°", 0, 1e+4, 2, _resolutionForDiameterEstimation);
    configDialog->addDouble(tr("Angle zénithal maximal pour la recherche de tronc"), "°", 0, 180, 2, _zenithalAngleMax);
    configDialog->addDouble(tr("Proportion du rayon où les alignements sont à exclure"), "%", 0, 100, 0, _ratioRadius, 100);

    configDialog->addBool(tr("Appliquer une fonction sigmoide pour le scoring"), "", "", _applySigmoid);
    configDialog->addDouble(tr("Fonction Sigmoide : coefficient K"), "", 0, 1e+4, 2, _sigmoidCoefK);
    configDialog->addDouble(tr("Fonction Sigmoide : coordonnée x0 du point d'inflexion "), "", 0, 1, 2, _sigmoidX0);

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
    configDialog->addTitle( tr("4- Continuité verticale :"));
    configDialog->addDouble(tr("Limite basse"), "m", 0, 1000, 2, _zMinVerticalContinuity);
    configDialog->addDouble(tr("Limite haute"), "m", 0, 1000, 2, _zMaxVerticalContinuity);
    configDialog->addDouble(tr("Rayon de recherche"), "m", 0, 1000, 2, _minRadiusForVerticalConstinuity);
    configDialog->addDouble(tr("Saut maximal en Z entre deux points successifs"), "m", 0, 1000, 2, _maxZDistForVerticalConstinuity);


    configDialog->addEmpty();
    configDialog->addBool(tr("Mode Debug Clusters"), "", "", _clusterDebugMode);
}

void ONF_StepDetectVerticalAlignments07::compute()
{
    QList<CT_ResultGroup*> outResultList = getOutResultList();
    CT_ResultGroup* res = outResultList.at(0);


    QList<CT_StandardItemGroup*> groups;
    // COPIED results browsing
    CT_ResultGroupIterator itCpy_grp(res, this, DEFin_grp);
    while (itCpy_grp.hasNext())
    {
        CT_StandardItemGroup* grp = (CT_StandardItemGroup*) itCpy_grp.next();
        groups.append(grp);
    }


//    AlignmentsDetectorForScene align(this, res);
//    for (int i = 0 ; i < groups.size() ; i++)
//    {

//        CT_StandardItemGroup* grp = groups.at(i);
//        align(grp);
//        setProgress(99.0 * (float)i / (float)(groups.size()));
//    }

    QFuture<void> futur = QtConcurrent::map(groups, AlignmentsDetectorForScene(this, res));

    int progressMin = futur.progressMinimum();
    int progressTotal = futur.progressMaximum() - futur.progressMinimum();
    while (!futur.isFinished())
    {
        setProgress(100.0*(futur.progressValue() - progressMin)/progressTotal);
    }

}






void ONF_StepDetectVerticalAlignments07::AlignmentsDetectorForScene::detectAlignmentsForScene(CT_StandardItemGroup* grp)
{

    double thresholdZenithalAngleRadians = M_PI * _step->_thresholdZenithalAngle / 180.0;
    double zenithalAngleMaxRadians = M_PI * _step->_zenithalAngleMax / 180.0;
    double rangeUnderstorey = _step->_maxDiameterForUnderstorey - _step->_minDiameter;

    const CT_AbstractItemDrawableWithPointCloud* sceneStem = (CT_AbstractItemDrawableWithPointCloud*)grp->firstItemByINModelName(_step, DEFin_sceneStem);
    const CT_AbstractItemDrawableWithPointCloud* sceneAll = (CT_AbstractItemDrawableWithPointCloud*)grp->firstItemByINModelName(_step, DEFin_sceneAll);
    const CT_StdLASPointsAttributesContainer* attributeLAS = (CT_StdLASPointsAttributesContainer*)grp->firstItemByINModelName(_step, DEFin_attLAS);
    const CT_AbstractAreaShape2D *emprise = (const CT_AbstractAreaShape2D*)grp->firstItemByINModelName(_step, DEF_SearchInArea);

    if (sceneStem != NULL && attributeLAS != NULL)
    {

        if (sceneStem->getPointCloudIndexSize() == 0) {return;}

        QMultiMap<double, Eigen::Vector2d> allPoints;
        // Compute point slice for continuity check
        if (sceneAll != NULL)
        {
            const CT_AbstractPointCloudIndex* pointCloudIndexAll = sceneAll->getPointCloudIndex();
            CT_PointIterator itP(pointCloudIndexAll);
            while(itP.hasNext())
            {
                const CT_Point &point = itP.next().currentPoint();

                if (point(2) > _step->_zMinVerticalContinuity && point(2) < _step->_zMaxVerticalContinuity)
                {
                    allPoints.insert(point(2), Eigen::Vector2d(point(0), point(1)));
                }
            }
        }


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
        sortIndicesByGPSTime(pointCloudIndexLAS, attributeGPS, pointCloudIndex, sortedIndices);

        // List of not clusterized points
        QList<size_t> isolatedPointIndices;


        // Creation des lignes de scan
        QList<QList<size_t> > linesOfScan;
        createLinesOfScan(sortedIndices, linesOfScan, isolatedPointIndices);
        sortedIndices.clear();

        // Archive detection of lines of scan (all complete)
        if (_step->_clusterDebugMode)
        {
            for (int lineN = 0 ; lineN < linesOfScan.size() ; lineN++)
            {
                QList<size_t> completeLine = linesOfScan.at(lineN);

                CT_PointCluster* cluster = new CT_PointCluster(_step->_clusterDebug1_ModelName.completeName(), _res);
                for (int j = 0 ; j < completeLine.size() ; j++)
                {
                    cluster->addPoint(completeLine.at(j));
                }
                CT_StandardItemGroup* grpClKept = new CT_StandardItemGroup(_step->_grpClusterDebug1_ModelName.completeName(), _res);
                grp->addGroup(grpClKept);
                grpClKept->addItemDrawable(cluster);
            }
        }

        
        // Eliminate noise in lines of scan
        QList<QList<size_t> > simplifiedLinesOfScan;
        denoiseLinesOfScan(linesOfScan, pointCloudIndexLAS, attributeIntensity, simplifiedLinesOfScan, isolatedPointIndices);
        linesOfScan.clear();


        // Archive detection of lines of scan (all denoised)
        if (_step->_clusterDebugMode)
        {
            for (int i = 0 ; i < simplifiedLinesOfScan.size() ; i++)
            {
                QList<size_t> line = simplifiedLinesOfScan.at(i);
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
        filterLinesOfScan(simplifiedLinesOfScan, thresholdZenithalAngleRadians, keptLinesOfScan, isolatedPointIndices);
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
        qSort(keptLinesOfScan.begin(), keptLinesOfScan.end(), ONF_StepDetectVerticalAlignments07::orderLinesByAscendingNumberAndLength);


        // Compute diameters using neighbourhoud
        QMultiMap< int, CT_Circle2D*> circleTypes;
        QMap<CT_Circle2D*, double> circleScores;
        QMap<CT_Circle2D*, CT_StandardItemGroup*> circleGroups;
        while (!keptLinesOfScan.isEmpty() && keptLinesOfScan.last()->size() > 2)
        {
            // Get next MainLine
            ScanLineData *mainLine = keptLinesOfScan.takeLast();

            int mainLineOfFlight = -1;
            QList<CT_Point> mainLinePoints;
            getPointsOfMainLine(pointCloudIndexLAS, attributeLineOfFlight, mainLine, mainLinePoints, mainLineOfFlight);

            int type = 1; // Multi lines of scans, ok

            // Search for neighbours
            QList<ScanLineData*> neighbourLines;
            QList<CT_Point> neighbourPoints;
            QList<QList<int> > neighbourLinesToTest;
            QList<QList<int> > neighbourLinesToTestIfOnlyOneLineOfFlight;
            findNeighbours(mainLine, pointCloudIndexLAS, attributeLineOfFlight, keptLinesOfScan, neighbourLines, neighbourPoints, neighbourLinesToTest, neighbourLinesToTestIfOnlyOneLineOfFlight, mainLineOfFlight);


            if (neighbourLinesToTest.isEmpty())
            {
                neighbourLinesToTest.append(neighbourLinesToTestIfOnlyOneLineOfFlight);
                neighbourLinesToTestIfOnlyOneLineOfFlight.clear();
                type = 2;
            }

            double diameter = 0.0;
            double bestScore = 0;
            Eigen::Vector3d bestDirection(0, 0, 1);

            if (!neighbourLinesToTest.isEmpty())
            {
                findBestDirectionAndDiameter(zenithalAngleMaxRadians, mainLine, mainLinePoints, neighbourPoints, neighbourLinesToTest, bestDirection, diameter, bestScore);
            }


            // Create cluster containing mainline and neighbourhood
            CT_PointCluster* cluster = new CT_PointCluster(_step->_cluster_ModelName.completeName(), _res);
            // Add points of the main line
            for (int j = 0 ; j < mainLine->size() ; j++)
            {
                size_t index = mainLine->at(j);
                cluster->addPoint(index);
            }
            // Add points of the neighbours lines
            for (int i = 0 ; i < neighbourLines.size() ; i++)
            {
                ScanLineData* testedLine = neighbourLines.at(i);
                for (int j = 0 ; j < testedLine->size() ; j++)
                {
                    size_t index = testedLine->at(j);
                    cluster->addPoint(index);
                }
            }

            int nbPts = cluster->getPointCloudIndexSize();

            double centerX = mainLine->_centerX;
            double centerY = mainLine->_centerY;
            double centerZ = mainLine->_centerZ;

            // if not valid diameter, compute diameter along first-last points line
            if (diameter >= _step->_maxDiameter || diameter <= 0 || (diameter / nbPts) > _step->_ratioDbhNbPtsMax)
            {
                cluster = computeDiameterAlongFirstLastLine(centerX, centerY, centerZ, mainLine, neighbourLines, bestDirection, diameter, isolatedPointIndices, cluster);
                type = 3; // Mono line of scan or excessive diameter
                bestScore = 0;
            }

            // If not valid diameter, put points in isolated Points
            nbPts = cluster->getPointCloudIndexSize();
            if (nbPts == 0 || diameter > _step->_maxDiameter || (diameter / nbPts) > _step->_ratioDbhNbPtsMax)
            {
                CT_PointIterator itP(cluster->getPointCloudIndex());
                while(itP.hasNext())
                {
                    size_t index = itP.next().currentGlobalIndex();
                    isolatedPointIndices.append(index);
                }
                delete cluster;
            } else {

                // Add to result
                CT_StandardItemGroup* grpClKept = NULL;
                CT_Circle2D* cir = addClusterToResult(cluster, diameter, type, centerX, centerY,  centerZ, mainLine->_length, bestDirection, bestScore, grpClKept);
                circleTypes.insert(type, cir);
                circleScores.insert(cir, bestScore);
                circleGroups.insert(cir, grpClKept);
            }

            delete mainLine;
            qDeleteAll(neighbourLines);
        }


        // Put all point from not merged 2 points lines in isolated points
        while (!keptLinesOfScan.isEmpty())
        {
            ScanLineData *line = keptLinesOfScan.takeLast();
            if (line->size() > 0) {isolatedPointIndices.append(line->at(0));}
            if (line->size() > 1) {isolatedPointIndices.append(line->at(1));}
        }


        ////////////////////////////////////////////////////
        /// Detection of small stems: points alignements ///
        ////////////////////////////////////////////////////

        // Eliminate isolated point to close from already deteted diameters

        QList<CT_Circle2D*> circles;
        circles.append(circleTypes.values());
        removePointsToCloseFromDetectedDiameters(circles, isolatedPointIndices);


        QList<ONF_StepDetectVerticalAlignments07::LineData*> candidateLines;
        // Test all lines 2 by 2
        findCandidateLines(sceneStem, candidateLines, isolatedPointIndices);


        // Put together near lines
        findNeighborLines(candidateLines, _step->_lineDistThresholdSmall);

        // Sort by descending NeighborCount
        qSort(candidateLines.begin(), candidateLines.end(), ONF_StepDetectVerticalAlignments07::orderByDescendingNeighborCount);


        // Create clusters of aligned points
        QList<size_t> insertedPoints;

        for (int i = 0 ; i < candidateLines.size() ; i++)
        {
            ONF_StepDetectVerticalAlignments07::LineData* candidateLine = candidateLines.at(i);
            CT_PointCluster* cluster = new CT_PointCluster(_step->_cluster_ModelName.completeName(), _res);

            if (!candidateLine->_processed)
            {
                createClusterFromCandidateLines(candidateLine, cluster, insertedPoints);

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

                    double maxPhiRadians = M_PI*_step->_maxPhiAngleSmall/180.0;
                    if (phi > maxPhiRadians)
                    {
                        delete cluster;
                        delete fittedLineData;

                    } else {

                        if (testLengthBetweenPoints(cloudIndex, fittedLineData))
                        {
                            int type = 4; // Alignement, ok
                            Eigen::Vector3d center = fittedLineData->getP1();
                            double diameter = computeDiameterAlongLine(cluster, fittedLineData->getDirection(), center);

                            if (diameter > _step->_maxDiameter || (diameter / nbPts) > _step->_ratioDbhNbPtsMax)
                            {
                                type = 5; // Alignement, too high diameter
                                diameter = (std::rand() / (double)RAND_MAX) * rangeUnderstorey + _step->_minDiameter;
                            }

                            CT_StandardItemGroup* grpClKept = NULL;
                            CT_Circle2D* cir = addClusterToResult(cluster, diameter, type, center(0), center(1),  center(2), fittedLineData->length(), fittedLineData->getDirection(), 0, grpClKept);
                            circleTypes.insert(type, cir);
                            circleGroups.insert(cir, grpClKept);
                        } else {
                            delete cluster;
                        }
                        delete fittedLineData;
                    }
                } else {
                    delete cluster;
                }
            } else {
                delete cluster;
            }
        }


        // Create a list of circles ordered by
        // 1- Increasing Type (1, 2, 3, 4, 5)
        // 2- Deceasing Score (for types 1 and 2)
        // 3- Deceasing radius (for types 3, 4 and 5)
        createOrderedCircleList(circleTypes, circleScores, circles);


        for (int i = 0 ; i < circles.size() ; i++)
        {
            CT_Circle2D* circle = circles.at(i);
            double circleX = circle->getCenterX();
            double circleY = circle->getCenterY();

            // Remove intersecting diameter
            for (int j = i + 1 ; j < circles.size() ; j++)
            {
                CT_Circle2D* circle2 = circles.at(j);

                double dist = sqrt(pow(circleX - circle2->getCenterX(), 2) + pow(circleY - circle2->getCenterY(), 2));
                if (dist < (circle->getRadius() + circle2->getRadius()))
                {
                    circles.removeAt(j--);
                    delete circleGroups.value(circle2);
                }
            }

            // Remove diameters outside the XY Area (if it has been given)
            if (emprise != NULL && !emprise->contains(circleX, circleY))
            {
                circles.removeAt(i--);
                delete circleGroups.value(circle);

            } else {

                // Corrected diameter if too low
                int flag = 0; // no correction
                double correctedDbh = circle->getRadius() * 2.0;
                if (correctedDbh < _step->_minDiameter)
                {
                    flag = 1;
                    correctedDbh = (std::rand() / (double)RAND_MAX) * rangeUnderstorey + _step->_minDiameter;
                }


                // Check if the detected stem have a vertical continuity
                double largestZDist = 0;

                if (sceneAll != NULL)
                {
                    double lastZ = _step->_zMinVerticalContinuity;
                    double newZDist = 0;

                    QMapIterator<double, Eigen::Vector2d> itPts(allPoints);
                    while (itPts.hasNext())
                    {
                        itPts.next();
                        Eigen::Vector2d pt = itPts.value();
                        double dist = sqrt(pow(circleX - pt(0), 2) + pow(circleY - pt(1), 2));

                        if (dist < _step->_minRadiusForVerticalConstinuity)
                        {
                            double z = itPts.key();
                            newZDist = z - lastZ;
                            if (newZDist > largestZDist)
                            {
                                largestZDist = newZDist;
                            }
                            lastZ = z;
                        }
                    }

                    newZDist = _step->_zMaxVerticalContinuity - lastZ;
                    if (newZDist > largestZDist)
                    {
                        largestZDist = newZDist;
                    }
                }

                if (largestZDist > _step->_maxZDistForVerticalConstinuity)
                {
                    circles.removeAt(i--);
                    delete circleGroups.value(circle);
                } else {
                    circle->addItemAttribute(new CT_StdItemAttributeT<int>(_step->_attCorrectedFlag_ModelName.completeName(), CT_AbstractCategory::DATA_VALUE, _res, flag));
                    circle->addItemAttribute(new CT_StdItemAttributeT<double>(_step->_attCorrectedDiameter_ModelName.completeName(), CT_AbstractCategory::DATA_VALUE, _res, correctedDbh * 100.0));
                    ((CT_Circle2DData*) (circle->getPointerData()))->setRadius(correctedDbh / 2.0);

                    CT_StandardItemGroup* grpClKept = circleGroups.value(circle, NULL);
                    if (grpClKept != NULL)
                    {
                        grp->addGroup(grpClKept);
                    }
                }
            }
        }

        qDeleteAll(candidateLines);
        candidateLines.clear();
        insertedPoints.clear();
        circles.clear();
        allPoints.clear();
    }


}


void ONF_StepDetectVerticalAlignments07::AlignmentsDetectorForScene::sortIndicesByGPSTime(const CT_AbstractPointCloudIndex* pointCloudIndexLAS,
                                                                                          const CT_AbstractPointAttributesScalar* attributeGPS,
                                                                                          const CT_AbstractPointCloudIndex* pointCloudIndex,
                                                                                          QMultiMap<double, size_t> &sortedIndices)
{
    CT_PointIterator itP(pointCloudIndex);
    while(itP.hasNext())
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
}

void ONF_StepDetectVerticalAlignments07::AlignmentsDetectorForScene::createLinesOfScan(const QMultiMap<double, size_t> &sortedIndices,
                                                                                       QList<QList<size_t> > &linesOfScan,
                                                                                       QList<size_t> &isolatedPointIndices)
{
    QList<size_t> currentIndexList;

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
}

void ONF_StepDetectVerticalAlignments07::AlignmentsDetectorForScene::denoiseLinesOfScan(const QList<QList<size_t> > &linesOfScan,
                                                                                        const CT_AbstractPointCloudIndex* pointCloudIndexLAS,
                                                                                        const CT_AbstractPointAttributesScalar* attributeIntensity,
                                                                                        QList<QList<size_t> > &simplifiedLinesOfScan,
                                                                                        QList<size_t> &isolatedPointIndices)
{
    CT_PointAccessor pointAccessor;

    QList<QList<size_t> > linesOfScanTmp;
    linesOfScanTmp.append(linesOfScan);

    for (int lineN = 0 ; lineN < linesOfScanTmp.size() ; lineN++)
    {
        QList<size_t> completeLine = linesOfScanTmp.at(lineN);
        int sizeCompleteLine = completeLine.size();

        // Find a reference point on the line of scan (max intensity point)
        double maxIntensity = -std::numeric_limits<double>::max();
        int refi = 0;
        for (int i = 0 ; i < sizeCompleteLine ; i++)
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
        for (int i = 0 ; i < sizeCompleteLine ; i++)
        {
            if (i != refi)
            {
                CT_Point p2 = pointAccessor.constPointAt(completeLine.at(i));
                Eigen::Vector3d direction = p2 - p1;
                direction.normalize();

                QList<size_t> simplifiedLine;

                for (int j = 0 ; j < sizeCompleteLine ; j++)
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

        if (_step->_retrieveLostLinesParts)
        {
            // All noise points are added to isolatedPointsIndices or in lines list
            QList<size_t> previousRemovedIndices;
            for (int i = 0 ; i < sizeCompleteLine ; i++)
            {
                size_t index = completeLine.at(i);
                if (finalSimplifiedLine.contains(index))
                {
                    if (previousRemovedIndices.size() == 1)
                    {
                        isolatedPointIndices.append(previousRemovedIndices.first());
                    } else if (previousRemovedIndices.size() > 1) {
                        linesOfScanTmp.append(previousRemovedIndices);
                        previousRemovedIndices.clear();
                    }
                    previousRemovedIndices.clear();
                } else {
                    previousRemovedIndices.append(index);
                }
            }

            if (previousRemovedIndices.size() == 1)
            {
                isolatedPointIndices.append(previousRemovedIndices.first());
            } else if (previousRemovedIndices.size() > 1) {

                if (finalSimplifiedLine.size() > 0)
                {
                    linesOfScanTmp.append(previousRemovedIndices);
                } else {
                    for (int i = 0 ; i < previousRemovedIndices.size() ; i++)
                    {
                        isolatedPointIndices.append(previousRemovedIndices.at(i));
                    }
                }
                previousRemovedIndices.clear();
            }
        } else {
            // All noise points are added to isolatedPointsIndices
            for (int i = 0 ; i < sizeCompleteLine ; i++)
            {
                size_t index = completeLine.at(i);
                if (!finalSimplifiedLine.contains(index))
                {
                    isolatedPointIndices.append(index);
                }
            }
        }

    }
}

void ONF_StepDetectVerticalAlignments07::AlignmentsDetectorForScene::filterLinesOfScan(QList<QList<size_t> > &simplifiedLinesOfScan,
                                                                                       double thresholdZenithalAngleRadians,
                                                                                       QList<ScanLineData*> &keptLinesOfScan,
                                                                                       QList<size_t> &isolatedPointIndices)
{
    CT_PointAccessor pointAccessor;

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
            // revert line if pointing down
            if (p1(2) < p2(2))
            {
                CT_SphericalLine3D::convertToSphericalCoordinates(&p1, &p2, phi, theta, length);
            } else {
                CT_SphericalLine3D::convertToSphericalCoordinates(&p2, &p1, phi, theta, length);

                QList<size_t> simplifiedLineTmp;
                for (int k = simplifiedLine.size() - 1  ; k >= 0 ; k--)
                {
                    simplifiedLineTmp.append(simplifiedLine.at(k));
                }
                simplifiedLine.clear();
                simplifiedLine.append(simplifiedLineTmp);
            }


            if (phi >= thresholdZenithalAngleRadians)
            {
                for (int j = 0 ; j < simplifiedLine.size() ; j++)
                {
                    isolatedPointIndices.append(simplifiedLine.at(j));
                }
            } else {
                double length = sqrt(pow(p1(0) - p2(0), 2) + pow(p1(1) - p2(1), 2) + pow(p1(2) - p2(2), 2));
                CT_Point pLow = pointAccessor.constPointAt(simplifiedLine.first());
                keptLinesOfScan.append(new ScanLineData(simplifiedLine, length, pLow(0),  pLow(1), pLow(2)));
            }
        }
    }
}


void ONF_StepDetectVerticalAlignments07::AlignmentsDetectorForScene::getPointsOfMainLine(const CT_AbstractPointCloudIndex* pointCloudIndexLAS,
                                                                                         const CT_AbstractPointAttributesScalar* attributeLineOfFlight,
                                                                                         const ScanLineData *mainLine,
                                                                                         QList<CT_Point> &mainLinePoints,
                                                                                         int &mainLineOfFlight)
{
    CT_PointAccessor pointAccessor;

    for (int j = 0 ; j < mainLine->size() ; j++)
    {
        size_t index = mainLine->at(j);
        CT_Point point = pointAccessor.constPointAt(index);
        mainLinePoints.append(point);

        size_t localIndex = pointCloudIndexLAS->indexOf(index);
        if (localIndex < pointCloudIndexLAS->size())
        {
            mainLineOfFlight = attributeLineOfFlight->dValueAt(localIndex);
        }
    }
}

bool ONF_StepDetectVerticalAlignments07::AlignmentsDetectorForScene::getPointOnLineForZ(const ScanLineData *line, double z, Eigen::Vector3d &point)
{
    if (line->size() < 2) {return false;}

    bool found = false;
    CT_PointAccessor pointAccessor;
    for (int i = 0 ; i < line->size() - 1 && !found; i++)
    {
        CT_Point lowestPoint = pointAccessor.constPointAt(line->at(i));
        CT_Point highestPoint = pointAccessor.constPointAt(line->at(i+1));

        if (lowestPoint(2) < highestPoint(2) && z >= lowestPoint(2) && z <= highestPoint(2))
        {
            found = true;

            Eigen::Vector3d dir = highestPoint - lowestPoint;
            double t = (z - lowestPoint(2)) / dir(2);

            point(0) = lowestPoint(0) + t * dir(0);
            point(1) = lowestPoint(1) + t * dir(1);
            point(2) = z;
        }
    }
    return found;
}

double ONF_StepDetectVerticalAlignments07::AlignmentsDetectorForScene::computeSpacing(const ScanLineData *mainLine, ScanLineData* testedLine)
{
    double spacing = -1;
    CT_PointAccessor pointAccessor;

    CT_Point mainLineLowestPoint = pointAccessor.constPointAt(mainLine->at(0));
    CT_Point mainLineHighestPoint = pointAccessor.constPointAt(mainLine->at(mainLine->size() - 1));

    CT_Point testedLineLowestPoint = pointAccessor.constPointAt(testedLine->at(0));
    CT_Point testedLineHighestPoint = pointAccessor.constPointAt(testedLine->at(testedLine->size() - 1));

    double distLow = -1;
    double distHigh = -1;
    double zLow = 0;
    double zHigh = 0;
    Eigen::Vector3d pointOnLine;

    if (testedLineHighestPoint(2) >= mainLineLowestPoint(2) && testedLineLowestPoint(2) <= mainLineHighestPoint(2))
    {
        if (testedLineLowestPoint(2) <= mainLineLowestPoint(2))
        {
            if (getPointOnLineForZ(testedLine, mainLineLowestPoint(2), pointOnLine))
            {
                distLow = sqrt(pow(mainLineLowestPoint(0) - pointOnLine(0), 2) + pow(mainLineLowestPoint(1) - pointOnLine(1), 2));
                zLow = mainLineLowestPoint(2);
            } else {
                qDebug() << "Error ONF_StepDetectVerticalAlignments07";
            }
        } else
        {
            if (getPointOnLineForZ(mainLine, testedLineLowestPoint(2), pointOnLine))
            {
                distLow = sqrt(pow(testedLineLowestPoint(0) - pointOnLine(0), 2) + pow(testedLineLowestPoint(1) - pointOnLine(1), 2));
                zLow = testedLineLowestPoint(2);

            } else {
                qDebug() << "Error ONF_StepDetectVerticalAlignments07";
            }
        }

        if (testedLineHighestPoint(2) >= mainLineHighestPoint(2))
        {
            if (getPointOnLineForZ(testedLine, mainLineHighestPoint(2), pointOnLine))
            {
                distHigh = sqrt(pow(mainLineHighestPoint(0) - pointOnLine(0), 2) + pow(mainLineHighestPoint(1) - pointOnLine(1), 2));
                zHigh = mainLineHighestPoint(2);
            } else {
                qDebug() << "Error ONF_StepDetectVerticalAlignments07";
            }
        } else
        {
            if (getPointOnLineForZ(mainLine, testedLineHighestPoint(2), pointOnLine))
            {
                distHigh = sqrt(pow(testedLineHighestPoint(0) - pointOnLine(0), 2) + pow(testedLineHighestPoint(1) - pointOnLine(1), 2));
                zHigh = testedLineHighestPoint(2);
            }   else {
                qDebug() << "Error ONF_StepDetectVerticalAlignments07";
            }
        }
    }

    if (distLow >= 0 && distHigh >=0 && zHigh > zLow)
    {
        spacing = (distHigh - distLow) / (zHigh - zLow);
    }

    return spacing;
}

void ONF_StepDetectVerticalAlignments07::AlignmentsDetectorForScene::findNeighbours(const ScanLineData *mainLine,
                                                                                    const CT_AbstractPointCloudIndex* pointCloudIndexLAS,
                                                                                    const CT_AbstractPointAttributesScalar* attributeLineOfFlight,
                                                                                    QList<ScanLineData*> &keptLinesOfScan,
                                                                                    QList<ScanLineData*> &neighbourLines,
                                                                                    QList<CT_Point> &neighbourPoints,
                                                                                    QList<QList<int> > &neighbourLinesToTest,
                                                                                    QList<QList<int> > &neighbourLinesToTestIfOnlyOneLineOfFlight,
                                                                                    int &mainLineOfFlight)
{
    CT_PointAccessor pointAccessor;
    neighbourPoints.clear();

    for (int i = 0 ; i < keptLinesOfScan.size() ; i++)
    {
        ScanLineData* testedLine = keptLinesOfScan.at(i);
        double distXY = sqrt(pow(mainLine->_centerX - testedLine->_centerX, 2) + pow(mainLine->_centerY - testedLine->_centerY, 2));

        if (distXY < _step->_maxDiameter)
        {
            if (distXY < (_step->_maxSearchRadiusByPts * testedLine->size()))
            {
                double spacing = computeSpacing(mainLine, testedLine);

                if (spacing <= _step->_maxLineSpacing)
                {
                    neighbourLines.append(testedLine);
                    keptLinesOfScan.removeAt(i--);

                    QList<int> ranksForLineToTest;
                    QList<int> ranksForLineToTestIfOnlyOneLineOfFlight;

                    for (int j = 0 ; j < testedLine->size() ; j++)
                    {
                        size_t index = testedLine->at(j);
                        CT_Point point = pointAccessor.constPointAt(index);
                        neighbourPoints.append(point);

                        size_t localIndex = pointCloudIndexLAS->indexOf(index);
                        int lineOfFlight = 0;
                        if (localIndex < pointCloudIndexLAS->size())
                        {
                            lineOfFlight = attributeLineOfFlight->dValueAt(localIndex);
                        }

                        if (lineOfFlight != mainLineOfFlight)
                        {
                            ranksForLineToTest.append(neighbourPoints.size() - 1);
                        } else {
                            ranksForLineToTestIfOnlyOneLineOfFlight.append(neighbourPoints.size() - 1);
                        }
                    }

                    if (!ranksForLineToTest.isEmpty()) {neighbourLinesToTest.append(ranksForLineToTest);}
                    if (!ranksForLineToTestIfOnlyOneLineOfFlight.isEmpty()) {neighbourLinesToTestIfOnlyOneLineOfFlight.append(ranksForLineToTestIfOnlyOneLineOfFlight);}
                }
            }
        }
    }
}

void ONF_StepDetectVerticalAlignments07::AlignmentsDetectorForScene::findBestDirectionAndDiameter(double zenithalAngleMaxRadians,
                                                                                                  const ScanLineData *mainLine,
                                                                                                  const QList<CT_Point> &mainLinePoints,
                                                                                                  const QList<CT_Point> &neighbourPoints,
                                                                                                  const QList<QList<int> > &neighbourLinesToTest,
                                                                                                  Eigen::Vector3d &bestDirection,
                                                                                                  double &diameter,
                                                                                                  double &bestScore)
{
    int mainLinePointsSize = mainLinePoints.size();
    int neighbourPointsSize = neighbourPoints.size();
    int neighbourLinesToTestSize = neighbourLinesToTest.size();

    Eigen::Vector3d center(mainLine->_centerX, mainLine->_centerY, mainLine->_centerZ);


    std::vector<Eigen::Vector2d> prjPtsMainLine(mainLinePointsSize);
    std::vector<Eigen::Vector2d> prjPtsNeighbours(neighbourPointsSize);
    Eigen::Vector3d projectedPt;
    Eigen::Vector2d circleCenter;

    QVector<bool> lowestPointsIncludedForLines(neighbourLinesToTestSize);
    QVector<double> scoreForLines(neighbourLinesToTestSize);
    QVector<double> diameterForLines(neighbourLinesToTestSize);
    QVector<Eigen::Vector3d> directionForLines(neighbourLinesToTestSize);
    lowestPointsIncludedForLines.fill(false);
    scoreForLines.fill(0);
    diameterForLines.fill(0);

    float resolutionInRadians = M_PI*_step->_resolutionForDiameterEstimation / 180.0;
    float pi2 =  2.0*(float)M_PI;

    // Selected points pairs to test
    QMultiMap<int, QPair<int, int> > pairsToTest;
    for (int i = 0 ; i < mainLinePointsSize ; i++)
    {
        CT_Point mainPt = mainLinePoints[i];

        for (int j = 0 ; j < neighbourLinesToTestSize ; j++)
        {
            const QList<int> &pointOfTheLine = neighbourLinesToTest.at(j);

            double deltaZMin = std::numeric_limits<double>::max();
            int bestK = -1;

            for (int k = 0 ; k < pointOfTheLine.size() ; k++)
            {
                int currentNeighbourIndex = pointOfTheLine.at(k);
                CT_Point testedPt = neighbourPoints[currentNeighbourIndex];

                double deltaZ = abs(mainPt(2) - testedPt(2));
                if (deltaZ < deltaZMin)
                {
                    deltaZMin = deltaZ;
                    bestK = k;
                }
            }

            if (bestK >= 0)
            {
                pairsToTest.insert(i, QPair<int, int>(j, bestK));
            }
        }
    }


    // For each direction
    for (float zenithal = 0 ; zenithal <= zenithalAngleMaxRadians ; zenithal += resolutionInRadians)
    {
        for (float azimut = 0 ; azimut <= pi2 ; azimut += resolutionInRadians)
        {
            // Compute projected points
            float dx, dy, dz;
            CT_SphericalLine3D::convertToCartesianCoordinates(zenithal, azimut, 1, dx, dy, dz);
            Eigen::Vector3d direction(dx, dy, dz);

            Eigen::Hyperplane<double, 3> plane(direction, center);

            // Compute projected points for main Line
            for (int i = 0 ; i < mainLinePointsSize ; i++)
            {
                projectedPt = plane.projection(mainLinePoints[i]);
                prjPtsMainLine[i](0) = projectedPt(0);
                prjPtsMainLine[i](1) = projectedPt(1);
            }

            // Compute projected points for neighbours Lines
            for (int i = 0 ; i < neighbourPointsSize ; i++)
            {
                projectedPt = plane.projection(neighbourPoints[i]);
                prjPtsNeighbours[i](0) = projectedPt(0);
                prjPtsNeighbours[i](1) = projectedPt(1);
            }

            // Test all possible diameter between one point from the main line, and one point from the neighbours lines
            for (int i = 0 ; i < mainLinePointsSize ; i++)
            {
                QList<QPair<int, int> > pointsToTest = pairsToTest.values(i);
                for (int pp = 0 ; pp < pointsToTest.size() ; pp++)
                {
                    QPair<int, int> pair = pointsToTest.at(pp);
                    int j = pair.first;
                    int k = pair.second;

                    const QList<int> &pointOfTheLine = neighbourLinesToTest.at(j);
                    int currentNeighbourIndex = pointOfTheLine.at(k);
                    Eigen::Vector2d neighbourProjectedPoint = prjPtsNeighbours[currentNeighbourIndex];

                    double candidateDiameter = sqrt(pow (prjPtsMainLine[i](0) - neighbourProjectedPoint(0), 2) + pow (prjPtsMainLine[i](1) - neighbourProjectedPoint(1), 2));

                    // If candidate diameter is in the good range
                    if (candidateDiameter <= _step->_maxDiameter && candidateDiameter >= _step->_minDiameter)
                    {
                        double candidateRadius = candidateDiameter / 2.0;
                        double ajustedRadius = _step->_ratioRadius * candidateRadius;
                        double candidateScore = 0;
                        bool lowestPointsIncludedForLine = false;
                        circleCenter (0) = (prjPtsMainLine[i](0) + neighbourProjectedPoint(0)) / 2.0;
                        circleCenter (1) = (prjPtsMainLine[i](1) + neighbourProjectedPoint(1)) / 2.0;

                        // Verifiy if any lowest neighbour point is in the circle
                        if (ajustedRadius > 0)
                        {
                            for (int ll = 0 ; ll < neighbourLinesToTestSize ; ll++)
                            {
                                if (ll != j)
                                {
                                    int firstIndex = neighbourLinesToTest.at(ll).first();
                                    Eigen::Vector2d point = prjPtsNeighbours[firstIndex];
                                    double distXY = sqrt(pow (circleCenter(0) - point(0), 2) + pow (circleCenter(1) - point(1), 2));

                                    if (distXY < ajustedRadius)
                                    {
                                        lowestPointsIncludedForLine = true;
                                    }
                                }
                            }
                        }


                        // compute score for mainLine points
                        for (int k = 0 ; k < mainLinePointsSize ; k++)
                        {
                            Eigen::Vector2d point = prjPtsMainLine[k];
                            double distXY = sqrt(pow (circleCenter(0) - point(0), 2) + pow (circleCenter(1) - point(1), 2));


                            if (_step->_applySigmoid)
                            {
                                if (distXY <= candidateRadius)
                                {
                                    candidateScore += weightedScore(distXY / candidateRadius, _step->_sigmoidCoefK, _step->_sigmoidX0);
                                } else if (distXY <= candidateDiameter)
                                {
                                    candidateScore += weightedScore((candidateDiameter - distXY) / candidateRadius, _step->_sigmoidCoefK, _step->_sigmoidX0);
                                }
                            } else {
                                if (distXY >= ajustedRadius && distXY <= candidateRadius)
                                {
                                    candidateScore += 1;
                                }
                            }
                        }

                        // compute score for neighbour points
                        for (int k = 0 ; k < neighbourPointsSize ; k++)
                        {
                            Eigen::Vector2d point = prjPtsNeighbours[k];
                            double distXY = sqrt(pow (circleCenter(0) - point(0), 2) + pow (circleCenter(1) - point(1), 2));


                            if (_step->_applySigmoid)
                            {
                                if (distXY <= candidateRadius)
                                {
                                    candidateScore += weightedScore(distXY / candidateRadius, _step->_sigmoidCoefK, _step->_sigmoidX0);
                                } else if (distXY <= candidateDiameter)
                                {
                                    candidateScore += weightedScore((candidateDiameter - distXY) / candidateRadius, _step->_sigmoidCoefK, _step->_sigmoidX0);
                                }
                            } else {
                                if (distXY >= ajustedRadius && distXY <= candidateRadius)
                                {
                                    candidateScore += 1;
                                }
                            }
                        }

                        if (candidateScore > scoreForLines[j])
                        {
                            scoreForLines[j] = candidateScore;
                            diameterForLines[j] = candidateDiameter;
                            directionForLines[j] = direction;
                            lowestPointsIncludedForLines[j] = lowestPointsIncludedForLine;
                        }
                    }
                }
            }
        }
    }

    for (int j = 0 ; j < neighbourLinesToTestSize ; j++)
    {
        if (!lowestPointsIncludedForLines[j] && scoreForLines[j] > bestScore)
        {
            bestScore = scoreForLines[j];
            diameter = diameterForLines[j];
            bestDirection = directionForLines[j];
        }
    }

}

void ONF_StepDetectVerticalAlignments07::AlignmentsDetectorForScene::createClusterFromCandidateLines(ONF_StepDetectVerticalAlignments07::LineData* candidateLine,
                                                                                                     CT_PointCluster* cluster,
                                                                                                     QList<size_t> &insertedPoints)
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

    QList<ONF_StepDetectVerticalAlignments07::LineData*> &neighborLines = candidateLine->_neighbors;
    for (int j = 0 ; j < neighborLines.size() ; j++)
    {
        ONF_StepDetectVerticalAlignments07::LineData* neighborLine = neighborLines.at(j);
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
}


CT_PointCluster* ONF_StepDetectVerticalAlignments07::AlignmentsDetectorForScene::computeDiameterAlongFirstLastLine(double centerX,
                                                                                                                   double centerY,
                                                                                                                   double centerZ,
                                                                                                                   const ScanLineData *mainLine,
                                                                                                                   const QList<ScanLineData*> &neighbourLines,
                                                                                                                   Eigen::Vector3d &bestDirection,
                                                                                                                   double &diameter,
                                                                                                                   QList<size_t> &isolatedPointIndices,
                                                                                                                   CT_PointCluster* cluster)
{
    // If we have only kept main line, reconstruct the cluster only with it and put meighbourhood to isolatedpoints list
    delete cluster;
    cluster = new CT_PointCluster(_step->_cluster_ModelName.completeName(), _res);
    // Add points of the main line
    for (int j = 0 ; j < mainLine->size() ; j++)
    {
        size_t index = mainLine->at(j);
        cluster->addPoint(index);
    }

    CT_PointAccessor pointAccessor;

    const size_t index1 = mainLine->first();
    const size_t index2 = mainLine->last();

    CT_Point p1 = pointAccessor.constPointAt(index1);
    CT_Point p2 = pointAccessor.constPointAt(index2);

    Eigen::Vector3d direction;
    direction = p2 - p1;
    direction.normalize();

    diameter = _step->_monoLineMult * computeDiameterAlongLine(cluster, direction, p1);

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


    // Add points of the neighbours lines
    for (int i = 0 ; i < neighbourLines.size() ; i++)
    {
        ScanLineData* testedLine = neighbourLines.at(i);
        for (int j = 0 ; j < testedLine->size() ; j++)
        {
            size_t index = testedLine->at(j);
            isolatedPointIndices.append(index);
        }
    }

    return cluster;
}

void ONF_StepDetectVerticalAlignments07::AlignmentsDetectorForScene::removePointsToCloseFromDetectedDiameters(const QList<CT_Circle2D*> &circles,
                                                                                                              QList<size_t> &isolatedPointIndices)
{
    CT_PointAccessor pointAccessor;

    for (int iso = 0 ; iso < isolatedPointIndices.size(); iso++)
    {
        size_t index = isolatedPointIndices.at(iso);
        CT_Point point = pointAccessor.constPointAt(index);

        bool removed = false;
        for (int i = 0 ; i < circles.size() && !removed; i++)
        {
            CT_Circle2D* circle = circles.at(i);
            double dist = sqrt(pow(circle->getCenterX() - point(0), 2) + pow(circle->getCenterY() - point(1), 2));
            if (dist < _step->_maxDiameter)
            {
                isolatedPointIndices.removeAt(iso--);
                removed = true;
            }
        }
    }
}

void ONF_StepDetectVerticalAlignments07::AlignmentsDetectorForScene::findCandidateLines(const CT_AbstractItemDrawableWithPointCloud* sceneStem,
                                                                                        QList<ONF_StepDetectVerticalAlignments07::LineData*> &candidateLines,
                                                                                        QList<size_t> &isolatedPointIndices)
{
    CT_PointAccessor pointAccessor;

    double maxPhiRadians = M_PI*_step->_maxPhiAngleSmall/180.0;

    for (int iso1 = 0 ; iso1 < isolatedPointIndices.size(); iso1++)
    {
        size_t index1 = isolatedPointIndices.at(iso1);
        CT_Point point1 = pointAccessor.constPointAt(index1);

        for (int iso2 = iso1 + 1 ; iso2 < isolatedPointIndices.size(); iso2++)
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
                        candidateLines.append(new ONF_StepDetectVerticalAlignments07::LineData(pointLow, pointHigh, index1, index2, phi, sceneStem->minZ(), sceneStem->maxZ()));
                    }
                }
            }
        }
    }
}

double ONF_StepDetectVerticalAlignments07::AlignmentsDetectorForScene::computeDiameterAlongLine(const CT_PointCluster* cluster,
                                                                                                const Eigen::Vector3d& direction,
                                                                                                const Eigen::Vector3d& origin)
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
        Eigen::Vector2d point1 = projPts.at(i);
        for (int j = i+1 ; j < projPts.size() ; j++)
        {
            Eigen::Vector2d point2 = projPts.at(j);
            double dist = sqrt(pow(point1(0) - point2(0), 2) + pow(point1(1) - point2(1), 2));
            if (dist > maxDist) {maxDist = dist;}
        }
    }

    return maxDist;
}


CT_Circle2D *ONF_StepDetectVerticalAlignments07::AlignmentsDetectorForScene::addClusterToResult(CT_PointCluster* cluster,
                                                                                                double diameter,
                                                                                                int type,
                                                                                                double centerX,
                                                                                                double centerY,
                                                                                                double centerZ,
                                                                                                double length,
                                                                                                const Eigen::Vector3d& direction,
                                                                                                double score,
                                                                                                CT_StandardItemGroup* &grpClKept)
{
    Eigen::Vector2d center2D(centerX, centerY);
    Eigen::Vector3d center3D(centerX, centerY, centerZ);

    grpClKept = new CT_StandardItemGroup(_step->_grpCluster_ModelName.completeName(), _res);


    cluster->addItemAttribute(new CT_StdItemAttributeT<double>(_step->_attMaxDistXY_ModelName.completeName(), CT_AbstractCategory::DATA_VALUE, _res, diameter*100.0));
    cluster->addItemAttribute(new CT_StdItemAttributeT<double>(_step->_attScore_ModelName.completeName(), CT_AbstractCategory::DATA_VALUE, _res, score));
    cluster->addItemAttribute(new CT_StdItemAttributeT<int>(_step->_attStemType_ModelName.completeName(), CT_AbstractCategory::DATA_VALUE, _res, type));
    grpClKept->addItemDrawable(cluster);

    CT_Circle2D *circle = new CT_Circle2D(_step->_circle_ModelName.completeName(), _res, new CT_Circle2DData(center2D, diameter/2.0));
    circle->addItemAttribute(new CT_StdItemAttributeT<double>(_step->_attMaxDistXY2_ModelName.completeName(), CT_AbstractCategory::DATA_VALUE, _res, diameter*100.0));
    circle->addItemAttribute(new CT_StdItemAttributeT<double>(_step->_attScore2_ModelName.completeName(), CT_AbstractCategory::DATA_VALUE, _res, score));
    circle->addItemAttribute(new CT_StdItemAttributeT<int>(_step->_attStemType2_ModelName.completeName(), CT_AbstractCategory::DATA_VALUE, _res, type));
    grpClKept->addItemDrawable(circle);

    CT_Line* line = new CT_Line(_step->_line_ModelName.completeName(), _res, new CT_LineData(center3D, center3D + length*direction));
    line->addItemAttribute(new CT_StdItemAttributeT<double>(_step->_attMaxDistXY3_ModelName.completeName(), CT_AbstractCategory::DATA_VALUE, _res, diameter*100.0));
    line->addItemAttribute(new CT_StdItemAttributeT<double>(_step->_attScore3_ModelName.completeName(), CT_AbstractCategory::DATA_VALUE, _res, score));
    line->addItemAttribute(new CT_StdItemAttributeT<int>(_step->_attStemType3_ModelName.completeName(), CT_AbstractCategory::DATA_VALUE, _res, type));
    grpClKept->addItemDrawable(line);

    return(circle);
}


void ONF_StepDetectVerticalAlignments07::AlignmentsDetectorForScene::findNeighborLines(QList<ONF_StepDetectVerticalAlignments07::LineData*> &candidateLines,
                                                                                       double distThreshold)
{
    int size = candidateLines.size();
    for (int i1 = 0 ; i1 < size ; i1++)
    {
        ONF_StepDetectVerticalAlignments07::LineData* line1 = candidateLines.at(i1);

        for (int i2 = i1+1 ; i2 < size ; i2++)
        {
            ONF_StepDetectVerticalAlignments07::LineData* line2 = candidateLines.at(i2);

            double distLow = sqrt(pow(line1->_lowCoord(0) - line2->_lowCoord(0), 2) + pow(line1->_lowCoord(1) - line2->_lowCoord(1), 2));

            if (distLow < distThreshold)
            {
                double distHigh = sqrt(pow(line1->_highCoord(0) - line2->_highCoord(0), 2) + pow(line1->_highCoord(1) - line2->_highCoord(1), 2));

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

bool ONF_StepDetectVerticalAlignments07::AlignmentsDetectorForScene::testLengthBetweenPoints(const CT_AbstractPointCloudIndex* cloudIndex,
                                                                                             const CT_LineData* fittedLineData)
{
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

    return okLength;
}

void ONF_StepDetectVerticalAlignments07::AlignmentsDetectorForScene::createOrderedCircleList(const QMultiMap< int, CT_Circle2D*> &circleTypes, const QMap<CT_Circle2D *, double> &circleScores, QList<CT_Circle2D*> &circles)
{
    circles.clear();
    QMultiMap<double, CT_Circle2D*> circlesByScore;
    QList<CT_Circle2D*> tmpCircles;
    tmpCircles.append(circleTypes.values(1));
    for (int i = 0 ; i < tmpCircles.size() ; i++)
    {
        CT_Circle2D* cir = tmpCircles.at(i);
        double val = - circleScores.value(cir);
        circlesByScore.insert(val, cir);
    }
    circles.append(circlesByScore.values());

    tmpCircles.clear();
    circlesByScore.clear();
    tmpCircles.append(circleTypes.values(2));
    for (int i = 0 ; i < tmpCircles.size() ; i++)
    {
        CT_Circle2D* cir = tmpCircles.at(i);
        double val = - circleScores.value(cir);
        circlesByScore.insert(val, cir);
    }
    circles.append(circlesByScore.values());

    tmpCircles.clear();
    circlesByScore.clear();
    tmpCircles.append(circleTypes.values(3));
    for (int i = 0 ; i < tmpCircles.size() ; i++)
    {
        CT_Circle2D* cir = tmpCircles.at(i);
        double val = - cir->getRadius();
        circlesByScore.insert(val, cir);
    }
    circles.append(circlesByScore.values());

    tmpCircles.clear();
    circlesByScore.clear();
    tmpCircles.append(circleTypes.values(4));
    for (int i = 0 ; i < tmpCircles.size() ; i++)
    {
        CT_Circle2D* cir = tmpCircles.at(i);
        double val = - cir->getRadius();
        circlesByScore.insert(val, cir);
    }
    circles.append(circlesByScore.values());

    tmpCircles.clear();
    circlesByScore.clear();
    tmpCircles.append(circleTypes.values(5));
    for (int i = 0 ; i < tmpCircles.size() ; i++)
    {
        CT_Circle2D* cir = tmpCircles.at(i);
        double val = - cir->getRadius();
        circlesByScore.insert(val, cir);
    }
    circles.append(circlesByScore.values());
}

