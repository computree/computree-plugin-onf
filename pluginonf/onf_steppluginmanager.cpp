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

#include "onf_steppluginmanager.h"

#include "ct_stepseparator.h"
#include "ct_steploadfileseparator.h"
#include "ct_stepcanbeaddedfirstseparator.h"


#ifdef USE_OPENCV
#include "step/onf_stepclassifyground.h"
#include "step/onf_stepcomputedsm.h"
#include "step/onf_stepcomputedtm.h"
#include "step/onf_stepcomputedtm02.h"
#include "step/onf_stepextractsoil04.h"
#include "step/onf_stepfiltermaximabyclusterpositions.h"
#include "step/onf_stepconvertfloatimagetoqint32.h"
#include "step/onf_stepcomputeattributemapfromclusters.h"
#endif

#include "step/onf_stepchangeclusterthickness02.h"
#include "step/onf_stepcomputelai2000data.h"
#include "step/onf_stepcomputespherevoxels.h"
#include "step/onf_stepconvertscenetocluster.h"
#include "step/onf_stepdetectsection06.h"
#include "step/onf_stepdetectsection07.h"
#include "step/onf_stepextractdiametersfromcylinders.h"
#include "step/onf_stepextractplot.h"
#include "step/onf_stepextractplotbasedondtm.h"
#include "step/onf_stepextractplotbasedonmnt02.h"
#include "step/onf_stepextractsoil03.h"
#include "step/onf_stepfilterclustersbysize.h"
#include "step/onf_stepfiltergroupsbygroupsnumber.h"
#include "step/onf_stepfitandfiltercylindersinsections.h"
#include "step/onf_stepfitcirclesandfilter.h"
#include "step/onf_stephorizontalclustering3d.h"
#include "step/onf_stepmergeendtoendsections04.h"
#include "step/onf_stepmergeneighboursections04.h"
#include "step/onf_steprefpointfromarccenter.h"
#include "step/onf_steprefpointfrombarycenter02.h"
#include "step/onf_stepsetfootcoordinatesvertically.h"
#include "step/onf_stepsmoothskeleton.h"
#include "step/onf_stepaddaffiliationid.h"
#include "step/onf_stepaddattributevalue.h"
#include "step/onf_stepcompare3dgridscontents.h"
#include "step/onf_stepcomputecrownprojection.h"
#include "step/onf_stepcomputehitgrid.h"
#include "step/onf_stepcorrectalsprofile.h"
#include "step/onf_stepcreateplotmanagerfromfile.h"
#include "step/onf_stepcreateplotmanagergrid.h"
#include "step/onf_stepdetectverticalalignments.h"
#include "step/onf_stepextractlogbuffer.h"
#include "step/onf_stepextractpositionsfromdensity.h"
#include "step/onf_stepfilteritemsbyposition.h"
#include "step/onf_stepfilterpointsbyboolgrid.h"
#include "step/onf_stepfitcylinderoncluster.h"
#include "step/onf_stepimportsegmafilesformatching.h"
#include "step/onf_steploaddatafromitemposition.h"
#include "step/onf_steploadplotareas.h"
#include "step/onf_steploadpositionsformatching.h"
#include "step/onf_steploadtreemap.h"
#include "step/onf_stepmanualinventory.h"
#include "step/onf_stepmatchitemspositions.h"
#include "step/onf_stepmergeclustersfrompositions.h"
#include "step/onf_stepmergeclustersfrompositions02.h"
#include "step/onf_stepmodifypositions2d.h"
#include "step/onf_stepreducepointsdensity.h"
#include "step/onf_stepsegmentcrowns.h"
#include "step/onf_stepsegmentcrownsfromstemclusters.h"
#include "step/onf_stepsegmentgaps.h"
#include "step/onf_stepselectbboxbyfilename.h"
#include "step/onf_stepselectcellsingrid3d.h"
#include "step/onf_stepselectcellsingrid3dbybinarypattern.h"
#include "step/onf_stepselectgroupsbyreferenceheight.h"
#include "step/onf_stepsetaffiliationidfromreference.h"
#include "step/onf_stepslicepointcloud.h"
#include "step/onf_steptransformpointcloud.h"
#include "step/onf_stepvalidateinventory.h"

#include "filter/onf_filterbyreturntype.h"
#include "filter/onf_filterremoveupperoutliers.h"

#include "metric/onf_metricquantiles.h"
#include "metric/onf_metriccomputestats.h"


ONF_StepPluginManager::ONF_StepPluginManager() : CT_AbstractStepPlugin()
{
}

ONF_StepPluginManager::~ONF_StepPluginManager()
{
}

bool ONF_StepPluginManager::loadGenericsStep()
{

    addNewGeometricalShapesStep<ONF_StepDetectSection07>(QObject::tr("Détéction tiges (ONF 2013)"));
    addNewGeometricalShapesStep<ONF_StepFilterGroupsByGroupsNumber>(QObject::tr("Détéction tiges (ONF 2013)"));
    addNewGeometricalShapesStep<ONF_StepMergeNeighbourSections04>(QObject::tr("Détéction tiges (ONF 2013)"));
    addNewGeometricalShapesStep<ONF_StepMergeEndToEndSections04>(QObject::tr("Détéction tiges (ONF 2013)"));
    addNewGeometricalShapesStep<ONF_StepSetFootCoordinatesVertically>(QObject::tr("Détéction tiges (ONF 2013)"));
    addNewGeometricalShapesStep<ONF_StepFitAndFilterCylindersInSections>(QObject::tr("Détéction tiges (ONF 2013)"));
    addNewGeometricalShapesStep<ONF_StepExtractDiametersFromCylinders>(QObject::tr("Détéction tiges (ONF 2013)"));

    addNewGeometricalShapesStep<ONF_StepFitCirclesAndFilter>(CT_StepsMenu::LP_Fit);
    addNewMetricsStep<ONF_StepComputeLAI2000Data>(QObject::tr("LAI"));
    addNewOtherStep<ONF_StepComputeSphereVoxels>("");
    addNewPointsStep<ONF_StepChangeClusterThickness02>(CT_StepsMenu::LP_Clusters);
    addNewPointsStep<ONF_StepConvertSceneToCluster>(CT_StepsMenu::LP_Clusters);
    addNewPointsStep<ONF_StepDetectSection06>(CT_StepsMenu::LP_Clusters);
    addNewPointsStep<ONF_StepExtractPlot>(CT_StepsMenu::LP_Extract);
    addNewPointsStep<ONF_StepExtractPlotBasedOnDTM>(CT_StepsMenu::LP_Extract);
    addNewPointsStep<ONF_StepExtractPlotBasedOnMNT02>(CT_StepsMenu::LP_Extract);
    addNewPointsStep<ONF_StepFilterClustersBySize>(CT_StepsMenu::LP_Clusters);
    addNewPointsStep<ONF_StepHorizontalClustering3D>(CT_StepsMenu::LP_Clusters);
    addNewPointsStep<ONF_StepRefPointFromArcCenter>(CT_StepsMenu::LP_Clusters);
    addNewPointsStep<ONF_StepRefPointFromBarycenter02>(CT_StepsMenu::LP_Clusters);
    addNewPointsStep<ONF_StepSmoothSkeleton>(CT_StepsMenu::LP_Clusters);
    addNewRastersStep<ONF_StepExtractSoil03>(CT_StepsMenu::LP_DEM);

    addNewGeometricalShapesStep<ONF_StepComputeCrownProjection>(CT_StepsMenu::LP_Crowns);
    addNewGeometricalShapesStep<ONF_StepDetectVerticalAlignments>(CT_StepsMenu::LP_Stems);
    addNewGeometricalShapesStep<ONF_StepExtractPositionsFromDensity>("");
    addNewGeometricalShapesStep<ONF_StepFilterItemsByPosition>(CT_StepsMenu::LP_Filter);
    addNewGeometricalShapesStep<ONF_StepFitCylinderOnCluster>(CT_StepsMenu::LP_Fit);
    addNewGeometricalShapesStep<ONF_StepManualInventory>(CT_StepsMenu::LP_DBH);
    addNewGeometricalShapesStep<ONF_StepMatchItemsPositions>("");
    addNewGeometricalShapesStep<ONF_StepMergeClustersFromPositions02>(CT_StepsMenu::LP_Crowns);
    addNewGeometricalShapesStep<ONF_StepMergeClustersFromPositions>(CT_StepsMenu::LP_Crowns);
    addNewGeometricalShapesStep<ONF_StepModifyPositions2D>(CT_StepsMenu::LP_Transform);
    addNewGeometricalShapesStep<ONF_StepSegmentCrowns>(CT_StepsMenu::LP_Crowns);
    addNewGeometricalShapesStep<ONF_StepSegmentCrownsFromStemClusters>(CT_StepsMenu::LP_Crowns);
    addNewGeometricalShapesStep<ONF_StepSegmentGaps>(CT_StepsMenu::LP_Crowns);
    addNewGeometricalShapesStep<ONF_StepValidateInventory>(CT_StepsMenu::LP_DBH);
    addNewLoadStep<ONF_StepImportSegmaFilesForMatching>("");
    addNewLoadStep<ONF_StepLoadDataFromItemPosition>("");
    addNewLoadStep<ONF_StepLoadPlotAreas>("");
    addNewLoadStep<ONF_StepLoadPositionsForMatching>();
    addNewLoadStep<ONF_StepLoadTreeMap>("");
    addNewMetricsStep<ONF_StepAddAttributeValue>("");
    addNewMetricsStep<ONF_StepCorrectALSProfile>("");
    addNewPointsStep<ONF_StepExtractLogBuffer>(CT_StepsMenu::LP_Extract);
    addNewPointsStep<ONF_StepFilterPointsByBoolGrid>(CT_StepsMenu::LP_Filter);
    addNewPointsStep<ONF_StepReducePointsDensity>(CT_StepsMenu::LP_Filter);
    addNewPointsStep<ONF_StepSlicePointCloud>(CT_StepsMenu::LP_Extract);
    addNewPointsStep<ONF_StepTransformPointCloud>(CT_StepsMenu::LP_Transform);
    addNewVoxelsStep<ONF_StepCompare3DGridsContents>("");
    addNewVoxelsStep<ONF_StepComputeHitGrid>(CT_StepsMenu::LP_Create);
    addNewVoxelsStep<ONF_StepSelectCellsInGrid3D>(CT_StepsMenu::LP_Filter);
    addNewVoxelsStep<ONF_StepSelectCellsInGrid3DByBinaryPattern>(CT_StepsMenu::LP_Filter);
    addNewWorkflowStep<ONF_StepAddAffiliationID>("");
    addNewWorkflowStep<ONF_StepCreatePlotManagerFromFile>("");
    addNewWorkflowStep<ONF_StepCreatePlotManagerGrid>("");
    addNewWorkflowStep<ONF_StepSelectBBoxByFileName>("");
    addNewWorkflowStep<ONF_StepSelectGroupsByReferenceHeight>(CT_StepsMenu::LP_Filter);
    addNewWorkflowStep<ONF_StepSetAffiliationIDFromReference>("");

#ifdef USE_OPENCV
    addNewRastersStep<ONF_StepExtractSoil04>(CT_StepsMenu::LP_DEM);
    addNewPointsStep<ONF_StepClassifyGround>(CT_StepsMenu::LP_Classify);
    addNewRastersStep<ONF_StepComputeDTM>(CT_StepsMenu::LP_DEM);
    addNewRastersStep<ONF_StepComputeDTM02>(CT_StepsMenu::LP_DEM);
    addNewRastersStep<ONF_StepComputeDSM>(CT_StepsMenu::LP_DEM);

    addNewRastersStep<ONF_StepFilterMaximaByClusterPositions>("");
    addNewRastersStep<ONF_StepConvertFloatImageToqint32>(CT_StepsMenu::LP_Transform);
    addNewMetricsStep<ONF_StepComputeAttributeMapFromClusters>("");
#endif


    return true;
}

bool ONF_StepPluginManager::loadOpenFileStep()
{
    return true;
}

bool ONF_StepPluginManager::loadCanBeAddedFirstStep()
{
    return true;
}

bool ONF_StepPluginManager::loadActions()
{
    return true;
}

bool ONF_StepPluginManager::loadExporters()
{
	return true;
}

bool ONF_StepPluginManager::loadReaders()
{
    return true;
}