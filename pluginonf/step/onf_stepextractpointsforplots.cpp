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
#include "ct_iterator/ct_pointiterator.h"

#ifdef USE_OPENCV
#include "ct_itemdrawable/ct_image2d.h"
#endif

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
    _cellSize = 50.0;
}

// Step description (tooltip of contextual menu)
QString ONF_StepExtractPointsForPlots::getStepDescription() const
{
    return tr("3- Extraire les points par placette");
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
    if (res != NULL)
    {
        res->addItemModel(DEFin_grpPlot, _outPoints_ModelName, new CT_Scene(), tr("Points"));
    }
}

// Semi-automatic creation of step parameters DialogBox
void ONF_StepExtractPointsForPlots::createPostConfigurationDialog()
{
   CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();
   configDialog->addDouble(tr("Taille de la cellule de QuadTree pour l'optimisation"), "m", 0, 1e+5, 2, _cellSize);
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
           QList<CT_AbstractAreaShape2D*> shapesList;
           QList<PlotPointsIndices> plotPointsIndicesList;

#ifdef USE_OPENCV
           double xmin = std::numeric_limits<double>::max();
           double ymin = std::numeric_limits<double>::max();
           double xmax = -std::numeric_limits<double>::max();
           double ymax = -std::numeric_limits<double>::max();
#endif

           CT_GroupIterator grpPlotIt(groupScene, this, DEFin_grpPlot);
           while (grpPlotIt.hasNext() && !isStopped())
           {

               CT_StandardItemGroup* groupPlot = (CT_StandardItemGroup*) grpPlotIt.next();
               CT_AbstractAreaShape2D* areaShape = (CT_AbstractAreaShape2D*) groupPlot->firstItemByINModelName(this, DEFin_plot);

               if (areaShape != NULL)
               {
#ifdef USE_OPENCV
                   if (areaShape->minX() < xmin) {xmin = areaShape->minX();}
                   if (areaShape->minY() < ymin) {ymin = areaShape->minY();}
                   if (areaShape->maxX() > xmax) {xmax = areaShape->maxX();}
                   if (areaShape->maxY() > ymax) {ymax = areaShape->maxY();}
#endif

                   shapesList.append(areaShape);
                   plotPointsIndicesList.append(PlotPointsIndices(groupPlot));
               }
           }
           int sizeShapes = shapesList.size();
           if (sizeShapes <= 0) {return;}

           // Construction of quadTree
#ifdef USE_OPENCV
           xmin -= _cellSize;
           ymin -= _cellSize;
           xmax += _cellSize;
           ymax += _cellSize;

           CT_Image2D<int>* quadTree = CT_Image2D<int>::createImage2DFromXYCoords(NULL, NULL,xmin, ymin, xmax, ymax, _cellSize, 0, -1, -1);

           for (size_t index = 0 ; index < quadTree->nCells() ; index++)
           {
               quadTree->setValueAtIndex(index, index);
           }
           QVector<QList<int> > shapeLists(quadTree->nCells());

           for (int sh = 0 ; sh < sizeShapes ; sh++)
           {
               CT_AbstractAreaShape2D* shape = shapesList.at(sh);
               Eigen::Vector3d min, max;
               shape->getBoundingBox(min, max);

               size_t colB, colE, linB, linE;
               if (!quadTree->col(min(0), colB)) {colB = 0;}
               if (!quadTree->col(max(0), colE)) {colE = quadTree->colDim() - 1;}
               if (!quadTree->lin(min(1), linE)) {linE = quadTree->linDim() - 1;}
               if (!quadTree->lin(max(1), linB)) {linB = 0;}

               for (size_t cc = colB; cc <= colE ; cc++)
               {
                   for (size_t ll = linB ; ll <= linE ; ll++)
                   {
                       int shIdx = quadTree->value(cc, ll);
                       shapeLists[shIdx].append(sh);
                   }
               }
           }
#endif


           setProgress(10);

           const CT_AbstractPointCloudIndex *pointCloudIndex = inScene->getPointCloudIndex();

           size_t sizeCloud = pointCloudIndex->size();
           PS_LOG->addInfoMessage(LogInterface::step, tr("Le nuage de points contient %1 points").arg(sizeCloud));


           size_t cpt = 0;
           CT_PointIterator itP(pointCloudIndex);
           while(itP.hasNext() && (!isStopped()))
           {
               const CT_Point &point = itP.next().currentPoint();
               size_t index = itP.currentGlobalIndex();


#ifdef USE_OPENCV
               int shIdx = quadTree->valueAtCoords(point(0), point(1));
               const QList<int> &shNumberList = shapeLists.at(shIdx);

               for (int sh = 0 ; sh < shNumberList.size() ; sh++)
               {
                   int shNum = shNumberList.at(sh);
                   if (shapesList.at(shNum)->contains(point(0), point(1)))
                   {
                       plotPointsIndicesList[shNum]._indices->addIndex(index);
                   }
               }

#else
               for (int sh = 0 ; sh < sizeShapes ; sh++)
               {
                   if (shapesList.at(sh)->contains(point(0), point(1)))
                   {
                       plotPointsIndicesList[sh]._indices->addIndex(index);
                   }
               }
#endif

               setProgress(10.0 + 79.0*((float)cpt++ / (float)sizeCloud));
           }

#ifdef USE_OPENCV
           delete quadTree;
#endif

           setProgress(90);


           cpt = 0;
           for (int sh = 0 ; sh < sizeShapes ; sh++)
           {
               PlotPointsIndices& plotPointsIndices = plotPointsIndicesList[sh];
               CT_StandardItemGroup* grpSh = plotPointsIndices._group;

               CT_PointCloudIndexVector *plotPointCloudIndex = plotPointsIndices._indices;
//               if (plotPointCloudIndex->size() > 0)
//               {
                   plotPointCloudIndex->setSortType(CT_PointCloudIndexVector::SortedInAscendingOrder);

                   CT_Scene* plotScene = new CT_Scene(_outPoints_ModelName.completeName(), resOut, PS_REPOSITORY->registerPointCloudIndex(plotPointCloudIndex));
                   plotScene->updateBoundingBox();

                   grpSh->addItemDrawable(plotScene);
//               } else {
//                   delete plotPointCloudIndex;
//               }
               setProgress(90 + 9.0*((float)cpt++ / (float)sizeShapes));

           }

           setProgress(99);

        }
    }    
}
