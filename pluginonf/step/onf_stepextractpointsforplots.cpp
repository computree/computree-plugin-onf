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

#include "onf_stepextractpointsforplots.h"

#include "ct_result/ct_resultgroup.h"
#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/ct_outresultmodelgroupcopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"
#include "ct_itemdrawable/tools/iterator/ct_groupiterator.h"
#include "ct_view/ct_stepconfigurabledialog.h"

#include "ct_itemdrawable/abstract/ct_abstractareashape2d.h"
#include "ct_itemdrawable/ct_scene.h"
#include "ct_pointcloudindex/ct_pointcloudindexvector.h"
#include "ct_iterator/ct_pointiterator.h"

#include <QDebug>

// Alias for indexing models
#define DEFin_result "inres"
#define DEFin_grpScene "grpscene"
#define DEFin_scene "scene"
#define DEFin_grpPlot "grpplot"
#define DEFin_plot "plot"

// Constructor : initialization of parameters
ONF_StepExtractPointsForPlots::ONF_StepExtractPointsForPlots(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
}

// Step description (tooltip of contextual menu)
QString ONF_StepExtractPointsForPlots::getStepDescription() const
{
    return tr("Extraire les points par placette");
}

// Step detailled description
QString ONF_StepExtractPointsForPlots::getStepDetailledDescription() const
{
    return tr("No detailled description for this step");
}

// Step URL
QString ONF_StepExtractPointsForPlots::getStepURL() const
{
    //return tr("STEP URL HERE");
    return CT_AbstractStep::getStepURL(); //by default URL of the plugin
}

// Step copy method
CT_VirtualAbstractStep* ONF_StepExtractPointsForPlots::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new ONF_StepExtractPointsForPlots(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void ONF_StepExtractPointsForPlots::createInResultModelListProtected()
{
    CT_InResultModelGroupToCopy *resIn = createNewInResultModelForCopy(DEFin_result, tr("Placettes"), tr(""), true);
    resIn->setZeroOrMoreRootGroup();
    resIn->addGroupModel("", DEFin_grpScene, CT_AbstractItemGroup::staticGetType(), tr("Groupe Scene"));
    resIn->addItemModel(DEFin_grpScene, DEFin_scene, CT_AbstractItemDrawableWithPointCloud::staticGetType(), tr("Scène complete"));
    resIn->addGroupModel(DEFin_grpScene, DEFin_grpPlot, CT_AbstractItemGroup::staticGetType(), tr("Groupe Placette"));
    resIn->addItemModel(DEFin_grpPlot, DEFin_plot, CT_AbstractAreaShape2D::staticGetType(), tr("Emprise placette"));
}

// Creation and affiliation of OUT models
void ONF_StepExtractPointsForPlots::createOutResultModelListProtected()
{
    CT_OutResultModelGroupToCopyPossibilities *res = createNewOutResultModelToCopy(DEFin_result);
    res->addItemModel(DEFin_grpPlot, _outPoints_ModelName, new CT_Scene(), tr("Points"));
}

// Semi-automatic creation of step parameters DialogBox
void ONF_StepExtractPointsForPlots::createPostConfigurationDialog()
{
   //CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();
}

void ONF_StepExtractPointsForPlots::compute()
{
    QList<CT_ResultGroup*> outResultList = getOutResultList();
    CT_ResultGroup* resOut = outResultList.at(0);

    CT_ResultGroupIterator itOut(resOut, this, DEFin_grpScene);
    while (itOut.hasNext() && !isStopped())
    {
        CT_StandardItemGroup* groupScene = (CT_StandardItemGroup*) itOut.next();
        CT_AbstractItemDrawableWithPointCloud* inScene = (CT_AbstractItemDrawableWithPointCloud*) groupScene->firstItemByINModelName(this, DEFin_scene);

        if (inScene != NULL)
        {
           QMap<CT_AbstractAreaShape2D*, PlotPointsIndices> shapes;
           CT_GroupIterator grpPlotIt(groupScene, this, DEFin_grpPlot);
           while (grpPlotIt.hasNext() && !isStopped())
           {
               CT_StandardItemGroup* groupPlot = (CT_StandardItemGroup*) grpPlotIt.next();
               CT_AbstractAreaShape2D* areaShape = (CT_AbstractAreaShape2D*) groupPlot->firstItemByINModelName(this, DEFin_plot);

               if (areaShape != NULL)
               {
                   shapes.insert(areaShape, PlotPointsIndices(groupPlot));
               }
           }

           QMapIterator<CT_AbstractAreaShape2D*, PlotPointsIndices> itSh(shapes);

           const CT_AbstractPointCloudIndex *pointCloudIndex = inScene->getPointCloudIndex();
           CT_PointIterator itP(pointCloudIndex);
           while(itP.hasNext() && (!isStopped()))
           {
               const CT_Point &point = itP.next().currentPoint();
               size_t index = itP.currentGlobalIndex();

               itSh.toFront();
               while (itSh.hasNext())
               {
                   itSh.next();
                   CT_AbstractAreaShape2D* shape = itSh.key();
                   if (shape->contains(point(0), point(1)))
                   {
                       PlotPointsIndices& plotPointsIndices = (PlotPointsIndices&) itSh.value();
                       plotPointsIndices._indices.append(index);
                   }
               }
           }

           itSh.toFront();
           while (itSh.hasNext())
           {
               itSh.next();
               PlotPointsIndices& plotPointsIndices = (PlotPointsIndices&) itSh.value();
               CT_StandardItemGroup* grpSh = plotPointsIndices._group;

               if (plotPointsIndices._indices.size() > 0)
               {
                   CT_PointCloudIndexVector *plotPointCloudIndex = new CT_PointCloudIndexVector();

                   for (int i = 0 ; i < plotPointsIndices._indices.size() ; i++)
                   {
                       plotPointCloudIndex->addIndex(plotPointsIndices._indices.at(i));
                   }

                   CT_Scene* plotScene = new CT_Scene(_outPoints_ModelName.completeName(), resOut, PS_REPOSITORY->registerPointCloudIndex(plotPointCloudIndex));
                   plotScene->updateBoundingBox();

                   grpSh->addItemDrawable(plotScene);
               }
           }

        }
    }    
}
