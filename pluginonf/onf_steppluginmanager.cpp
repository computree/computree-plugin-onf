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
#include "step/onf_stepcomputedsmonly.h"
#include "step/onf_stepcomputedtm02.h"
#include "step/onf_stepfiltermaximabyclusterpositions.h"
#include "step/onf_stepconvertfloatimagetoqint32.h"
#include "step/onf_stepcomputeattributemapfromclusters.h"
#include "step/onf_stepextractplotbasedondtm.h"
#include "step/onf_stepmergeclustersfrompositions02.h"
#include "step/onf_stepmanualinventory.h"
#include "step/onf_stepextractpositionsfromdensity.h"
#include "step/onf_stepsegmentcrownsfromstemclusters.h"
#include "step/onf_stepvoxelclusterization.h"
#include "step/onf_stepfiltermaximabyneighbourhood.h"
#include "step/onf_stepcreatemaximacloud.h"
#include "step/onf_stepoptimizegaussianonmaximanumber.h"
#include "step/onf_stepconverttintodtm.h"
#include "step/onf_stepremoveuppernoise.h"
#include "step/onf_stepcomputeboundaryv2.h"
#include "step/onf_steppolygonfrommask.h"
#include "step/onf_stepcomputepointheightattribute.h"
#include "step/onf_stepcreatecolorcomposite.h"
#endif

#include "step/onf_stepchangeclusterthickness02.h"
#include "step/onf_stepcomputelai2000data.h"
#include "step/onf_stepcomputespherevoxels.h"
#include "step/onf_stepconvertscenetocluster.h"
#include "step/onf_stepdetectsection07.h"
#include "step/onf_stepextractdiametersfromcylinders.h"
#include "step/onf_stepextractplot.h"
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
#include "step/onf_stepcomputeverticalprofile.h"
#include "step/onf_stepcorrectalsprofile.h"
#include "step/onf_stepcreateplotmanagerfromfile.h"
#include "step/onf_stepcreateplotmanagergrid.h"
#include "step/onf_stepcreateplotsfromlist.h"
#include "step/onf_stepextractpointsforplots.h"
#include "step/onf_stepdetectverticalalignments06.h"
#include "step/onf_stepdetectverticalalignments07.h"
#include "step/onf_stepextractlogbuffer.h"
#include "step/onf_stepfilteritemsbyposition.h"
#include "step/onf_stepfilterpointsbyboolgrid.h"
#include "step/onf_stepfitcylinderoncluster.h"
#include "step/onf_stepimportsegmafilesformatching.h"
#include "step/onf_steploadplotareas.h"
#include "step/onf_steploadpositionsformatching.h"
#include "step/onf_steploadtreemap.h"
#include "step/onf_stepmatchitemspositions.h"
#include "step/onf_stepmodifypositions2d.h"
#include "step/onf_stepreducepointsdensity.h"
#include "step/onf_stepsegmentcrowns.h"
#include "step/onf_stepsegmentgaps.h"
#include "step/onf_stepselectbboxbyfilename.h"
#include "step/onf_stepselectcellsingrid3d.h"
#include "step/onf_stepselectcellsingrid3dbybinarypattern.h"
#include "step/onf_stepselectgroupsbyreferenceheight.h"
#include "step/onf_stepsetaffiliationidfromreference.h"
#include "step/onf_stepslicepointcloud.h"
#include "step/onf_steptransformpointcloud.h"
#include "step/onf_stepvalidateinventory.h"
#include "step/onf_stepextractpointsinverticalcylinders.h"
#include "step/onf_stepaddtilexyareas.h"
#include "step/onf_stepcreatetiling.h"
#include "step/onf_stepcomputeedbhfromheightallometry.h"
#include "step/onf_stepaddlasdatatoplots.h"
#include "step/onf_stepfilterelementsbyxyarea.h"
#include "step/onf_stepfiltergridbycloud.h"
#include "step/onf_stepcomputecumulativeconvexhull.h"
#include "step/onf_stepcomputetin.h"
#include "step/onf_stepcomputehfromzandtin.h"
#include "step/onf_stepmergescenesbymodality.h"
#include "step/onf_stepmatchclouds.h"
#include "step/onf_stepcomputecumulativesummary.h"
#include "step/onf_stepcomputecumulativenrtable.h"
#include "step/onf_stepadjustplotposition.h"
#include "step/onf_stepcreatepointgrid.h"
#include "step/onf_stepcreateseedgrid.h"
#include "step/onf_stepsegmentfromseedgrid.h"

#include "filter/onf_filterbyreturntype.h"
#include "filter/onf_filterremoveupperoutliers.h"

#include "metric/onf_metricquantiles.h"
#include "metric/onf_metricnbylasclass.h"
#include "metric/onf_metricminmaxlasfields.h"
#include "metric/onf_metricnapexmean.h"

#include "itemdrawable/onf_lai2000data.h"
#include "itemdrawable/onf_lvoxsphere.h"


ONF_StepPluginManager::ONF_StepPluginManager() : CT_AbstractStepPlugin()
{
    _logListener = new CT_FileLogListener();

    // défini le chemin vers le fichier
    _logListener->setFilePath("log_onf.log");
}

ONF_StepPluginManager::~ONF_StepPluginManager()
{
    PS_LOG->removeLogListener(_logListener);
}

QString ONF_StepPluginManager::getPluginOfficialName() const
{
    return "ONF";
}

QString ONF_StepPluginManager::getPluginRISCitation() const
{
    return "TY  - COMP\n"
           "TI  - Plugin ONF for Computree\n"
           "AU  - Piboule, Alexandre\n"
           "PB  - Office National des Forêts, RDI Department\n"
           "PY  - 2017\n"
           "UR  - http://rdinnovation.onf.fr/projects/plugin-onf\n"
           "ER  - \n";
}


bool ONF_StepPluginManager::init()
{

    // enregistrement du listener auprès de PS_LOG
    PS_LOG->addPrioritaryLogListener(_logListener);

    return CT_AbstractStepPlugin::init();
}

bool ONF_StepPluginManager::loadGenericsStep()
{

    addNewGeometricalShapesStep<ONF_StepDetectSection07>(QObject::tr("Détécter (tiges) - ONF 2013"));
    addNewGeometricalShapesStep<ONF_StepFilterGroupsByGroupsNumber>(QObject::tr("Détécter (tiges) - ONF 2013"));
    addNewGeometricalShapesStep<ONF_StepMergeNeighbourSections04>(QObject::tr("Détécter (tiges) - ONF 2013"));
    addNewGeometricalShapesStep<ONF_StepMergeEndToEndSections04>(QObject::tr("Détécter (tiges) - ONF 2013"));
    addNewGeometricalShapesStep<ONF_StepFitAndFilterCylindersInSections>(QObject::tr("Détécter (tiges) - ONF 2013"));
    addNewGeometricalShapesStep<ONF_StepExtractDiametersFromCylinders>(QObject::tr("Détécter (tiges) - ONF 2013"));
    addNewGeometricalShapesStep<ONF_StepAdjustPlotPosition>(CT_StepsMenu::LP_Fit);

    addNewGeometricalShapesStep<ONF_StepFitCirclesAndFilter>(CT_StepsMenu::LP_Fit);
    addNewBetaStep<ONF_StepComputeSphereVoxels>(CT_StepsMenu::LP_Analyze);
    addNewPointsStep<ONF_StepChangeClusterThickness02>(CT_StepsMenu::LP_Clusters);
    addNewPointsStep<ONF_StepConvertSceneToCluster>(CT_StepsMenu::LP_Transform);
    addNewPointsStep<ONF_StepExtractPlot>(CT_StepsMenu::LP_Extract);
    addNewPointsStep<ONF_StepFilterClustersBySize>(CT_StepsMenu::LP_Clusters);
    addNewBetaStep<ONF_StepHorizontalClustering3D>(CT_StepsMenu::LP_Clusters);
    addNewGeometricalShapesStep<ONF_StepRefPointFromArcCenter>(QObject::tr("Points de référence"));
    addNewGeometricalShapesStep<ONF_StepRefPointFromBarycenter02>(QObject::tr("Points de référence"));
    addNewGeometricalShapesStep<ONF_StepSmoothSkeleton>(QObject::tr("Points de référence"));
    addNewPointsStep<ONF_StepComputeVerticalProfile>(CT_StepsMenu::LP_Analyze);
    addNewPointsStep<ONF_StepCorrectALSProfile>(CT_StepsMenu::LP_Analyze);
    addNewPointsStep<ONF_StepExtractPointsInVerticalCylinders>(CT_StepsMenu::LP_Extract);
    addNewPointsStep<ONF_StepComputeHfromZandTIN>(CT_StepsMenu::LP_Transform);
    addNewPointsStep<ONF_StepRemoveUpperNoise>(CT_StepsMenu::LP_Filter);
    addNewPointsStep<ONF_StepMergeScenesByModality>(CT_StepsMenu::LP_Create);
    addNewPointsStep<ONF_StepMatchClouds>(CT_StepsMenu::LP_Analyze);


    addNewGeometricalShapesStep<ONF_StepComputeCrownProjection>(CT_StepsMenu::LP_Crowns);
    addNewGeometricalShapesStep<ONF_StepFilterItemsByPosition>(CT_StepsMenu::LP_Filter);
    addNewGeometricalShapesStep<ONF_StepFitCylinderOnCluster>(CT_StepsMenu::LP_Fit);
    addNewGeometricalShapesStep<ONF_StepModifyPositions2D>(CT_StepsMenu::LP_Stems);
    addNewGeometricalShapesStep<ONF_StepSegmentCrowns>(CT_StepsMenu::LP_Crowns);
    addNewGeometricalShapesStep<ONF_StepSegmentGaps>(CT_StepsMenu::LP_Crowns);
    addNewGeometricalShapesStep<ONF_StepValidateInventory>(CT_StepsMenu::LP_Stems);
    addNewGeometricalShapesStep<ONF_StepAddAttributeValue>(CT_StepsMenu::LP_Analyze);
    addNewShapes2DStep<ONF_StepAddTileXYAreas>(CT_StepsMenu::LP_Footprints);
    addNewShapes2DStep<ONF_StepCreateTiling>(CT_StepsMenu::LP_Footprints);
    addNewLoadStep<ONF_StepLoadPlotAreas>(CT_StepsMenu::LP_Vector);
    addNewLoadStep<ONF_StepLoadTreeMap>(CT_StepsMenu::LP_Items);
    addNewPointsStep<ONF_StepReducePointsDensity>(CT_StepsMenu::LP_Filter);
    addNewPointsStep<ONF_StepSlicePointCloud>(CT_StepsMenu::LP_Extract);
    addNewVoxelsStep<ONF_StepCompare3DGridsContents>(CT_StepsMenu::LP_Analyze);
    addNewVoxelsStep<ONF_StepFilterGridByCloud>(CT_StepsMenu::LP_Filter);
    addNewVoxelsStep<ONF_StepComputeHitGrid>(CT_StepsMenu::LP_Create);
    addNewVoxelsStep<ONF_StepSelectCellsInGrid3D>(CT_StepsMenu::LP_Filter);
    addNewVoxelsStep<ONF_StepSelectCellsInGrid3DByBinaryPattern>(CT_StepsMenu::LP_Filter);
    addNewVoxelsStep<ONF_StepFilterPointsByBoolGrid>(CT_StepsMenu::LP_Filter);
    addNewWorkflowStep<ONF_StepAddAffiliationID>("Affiliations");
    addNewShapes2DStep<ONF_StepCreatePlotManagerFromFile>(CT_StepsMenu::LP_Plots);
    addNewShapes2DStep<ONF_StepCreatePlotManagerGrid>(CT_StepsMenu::LP_Plots);
    addNewShapes2DStep<ONF_StepCreatePlotsFromList>(CT_StepsMenu::LP_Plots);
    addNewShapes2DStep<ONF_StepExtractPointsForPlots>(CT_StepsMenu::LP_Plots);
    addNewShapes2DStep<ONF_StepAddLASDataToPlots>(CT_StepsMenu::LP_Plots);
    addNewShapes2DStep<ONF_StepFilterElementsByXYArea>(CT_StepsMenu::LP_Footprints);
    addNewShapes2DStep<ONF_StepSelectBBoxByFileName>(CT_StepsMenu::LP_Footprints);
    addNewShapes2DStep<ONF_StepComputeCumulativeConvexHull>(CT_StepsMenu::LP_Footprints);
    addNewGeometricalShapesStep<ONF_StepSelectGroupsByReferenceHeight>(CT_StepsMenu::LP_Filter);
    addNewWorkflowStep<ONF_StepSetAffiliationIDFromReference>("Affiliations");
    addNewGeometricalShapesStep<ONF_StepComputeDBHFromHeightAllometry>(CT_StepsMenu::LP_Stems);
    addNewGeometricalShapesStep<ONF_StepCreateSeedGrid>(CT_StepsMenu::LP_Stems);
    addNewGeometricalShapesStep<ONF_StepSegmentFromSeedGrid>(CT_StepsMenu::LP_Stems);

    addNewPointsStep<ONF_StepComputeTIN>(CT_StepsMenu::LP_Create);
    addNewPointsStep<ONF_StepCreatePointGrid>(CT_StepsMenu::LP_Create);
    addNewMetricStep<ONF_StepComputeCumulativeSummary>(CT_StepsMenu::LP_Analyze);
    addNewMetricStep<ONF_StepComputeCumulativeNRTable>(CT_StepsMenu::LP_Analyze);

    addNewPointsStep<ONF_StepTransformPointCloud>(CT_StepsMenu::LP_Transform);
    addNewBetaStep<ONF_StepComputeLAI2000Data>(CT_StepsMenu::LP_Analyze);
    addNewBetaStep<ONF_StepImportSegmaFilesForMatching>(CT_StepsMenu::LP_Others);
    addNewBetaStep<ONF_StepLoadPositionsForMatching>(CT_StepsMenu::LP_Others);
    addNewBetaStep<ONF_StepMatchItemsPositions>(CT_StepsMenu::LP_Others);
    addNewBetaStep<ONF_StepExtractLogBuffer>(CT_StepsMenu::LP_Extract);
    addNewBetaStep<ONF_StepDetectVerticalAlignments07>(CT_StepsMenu::LP_Stems);


#ifdef USE_OPENCV
    //addNewBetaStep<ONF_StepDetectVerticalAlignments06>(CT_StepsMenu::LP_Stems);

    addNewGeometricalShapesStep<ONF_StepSetFootCoordinatesVertically>(QObject::tr("Détécter (tiges) - ONF 2013"));
    addNewGeometricalShapesStep<ONF_StepExtractPositionsFromDensity>(CT_StepsMenu::LP_Stems);
    addNewBetaStep<ONF_StepSegmentCrownsFromStemClusters>(CT_StepsMenu::LP_Crowns);

    addNewPointsStep<ONF_StepClassifyGround>(CT_StepsMenu::LP_Classify);
    addNewPointsStep<ONF_StepExtractPlotBasedOnDTM>(CT_StepsMenu::LP_Extract);
    addNewRastersStep<ONF_StepComputeDTM02>(CT_StepsMenu::LP_DEM);
    addNewRastersStep<ONF_StepComputeDSM>(CT_StepsMenu::LP_DEM);
    addNewRastersStep<ONF_StepComputeDSMOnly>(CT_StepsMenu::LP_DEM);
    addNewRastersStep<ONF_StepFilterMaximaByNeighbourhood>(CT_StepsMenu::LP_DEM);
    addNewRastersStep<ONF_StepCreateMaximaCloud>(CT_StepsMenu::LP_DEM);
    addNewRastersStep<ONF_StepOptimizeGaussianOnMaximaNumber>(CT_StepsMenu::LP_DEM);
    addNewRastersStep<ONF_StepConvertTINtoDTM>(CT_StepsMenu::LP_DEM);

    addNewBetaStep<ONF_StepFilterMaximaByClusterPositions>(CT_StepsMenu::LP_Raster);
    addNewBetaStep<ONF_StepConvertFloatImageToqint32>(CT_StepsMenu::LP_Transform);
    addNewRastersStep<ONF_StepComputeAttributeMapFromClusters>("");

    addNewGeometricalShapesStep<ONF_StepMergeClustersFromPositions02>(CT_StepsMenu::LP_Crowns);
    addNewGeometricalShapesStep<ONF_StepManualInventory>(CT_StepsMenu::LP_Stems);
    addNewPointsStep<ONF_StepVoxelClusterization>(CT_StepsMenu::LP_Clusters);
    addNewShapes2DStep<ONF_StepComputeBoundaryV2>(CT_StepsMenu::LP_Footprints);
    addNewShapes2DStep<ONF_StepPolygonFromMask>(CT_StepsMenu::LP_Footprints);
    addNewPointsStep<ONF_StepComputePointHeightAttribute>(CT_StepsMenu::LP_Create);
    addNewRastersStep<ONF_StepCreateColorComposite>(CT_StepsMenu::LP_Create);

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

bool ONF_StepPluginManager::loadFilters()
{
    addNewFilter(new ONF_FilterByReturnType());
    addNewFilter(new ONF_FilterRemoveUpperOutliers());
    return true;
}

bool ONF_StepPluginManager::loadMetrics()
{
    addNewMetric(new ONF_MetricNbyLASClass());
    addNewMetric(new ONF_MetricMinMaxLASFields());
    addNewMetric(new ONF_MetricQuantiles());
    addNewMetric(new ONF_MetricNApexMean());
    return true;
}

bool ONF_StepPluginManager::loadItemDrawables()
{
    addNewItemDrawable<ONF_Lai2000Data>();
    addNewItemDrawable<ONF_LvoxSphere>();
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
