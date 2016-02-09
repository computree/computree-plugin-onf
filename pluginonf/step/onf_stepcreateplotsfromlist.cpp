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

#include "onf_stepcreateplotsfromlist.h"

#include "ct_result/ct_resultgroup.h"
#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/ct_outresultmodelgroupcopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"
#include "ct_itemdrawable/tools/iterator/ct_groupiterator.h"
#include "ct_view/ct_stepconfigurabledialog.h"

#include "ct_itemdrawable/abstract/ct_abstractareashape2d.h"
#include "ct_itemdrawable/ct_plotlistingrid.h"
#include "ct_itemdrawable/ct_circle2d.h"
#include "ct_itemdrawable/ct_box2d.h"

// Alias for indexing models
#define DEFin_result "resultDataSource"
#define DEFin_grpPlotList "grpplotlist"
#define DEFin_plotList "plotList"

#define DEF_typeSquare "Square"
#define DEF_typeCircular "Circle"

// Constructor : initialization of parameters
ONF_StepCreatePlotsFromList::ONF_StepCreatePlotsFromList(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _plotType = DEF_typeCircular;
    _createBuffers = true;
    _buffer = 5.0;
}

// Step description (tooltip of contextual menu)
QString ONF_StepCreatePlotsFromList::getStepDescription() const
{
    return tr("Créer des placettes à partir d'une liste");
}

// Step detailled description
QString ONF_StepCreatePlotsFromList::getStepDetailledDescription() const
{
    return tr("No detailled description for this step");
}

// Step URL
QString ONF_StepCreatePlotsFromList::getStepURL() const
{
    //return tr("STEP URL HERE");
    return CT_AbstractStep::getStepURL(); //by default URL of the plugin
}

// Step copy method
CT_VirtualAbstractStep* ONF_StepCreatePlotsFromList::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new ONF_StepCreatePlotsFromList(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void ONF_StepCreatePlotsFromList::createInResultModelListProtected()
{
    CT_InResultModelGroupToCopy *resIn = createNewInResultModelForCopy(DEFin_result, tr("Placettes"), tr(""), true);
    resIn->setZeroOrMoreRootGroup();
    resIn->addGroupModel("", DEFin_grpPlotList, CT_AbstractItemGroup::staticGetType(), tr("Groupe"));
    resIn->addItemModel(DEFin_grpPlotList, DEFin_plotList, CT_PlotListInGrid::staticGetType(), tr("Liste de placettes"));

}

// Creation and affiliation of OUT models
void ONF_StepCreatePlotsFromList::createOutResultModelListProtected()
{
    CT_OutResultModelGroupToCopyPossibilities *res = createNewOutResultModelToCopy(DEFin_result);

    if(res != NULL) {
        res->addGroupModel(DEFin_grpPlotList, _outGrpPlot_ModelName, new CT_StandardItemGroup(), tr("Placette (Groupe)"));

        if (_plotType == DEF_typeCircular)
        {
            res->addItemModel(_outGrpPlot_ModelName, _outPlot_ModelName, new CT_Circle2D(), tr("Placette circulaire"));
        } else {
            res->addItemModel(_outGrpPlot_ModelName, _outPlot_ModelName, new CT_Box2D(), tr("Placette carrée"));
        }
    }

    if (_createBuffers)
    {
        if (_plotType == DEF_typeCircular)
        {
            res->addItemModel(_outGrpPlot_ModelName, _outPlotBuffer_ModelName, new CT_Circle2D(), tr("Placette circulaire (buffer)"));
        } else {
            res->addItemModel(_outGrpPlot_ModelName, _outPlotBuffer_ModelName, new CT_Box2D(), tr("Placette carrée (buffer)"));
        }
    }
}

// Semi-automatic creation of step parameters DialogBox
void ONF_StepCreatePlotsFromList::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();
    QStringList types;
    types << DEF_typeCircular;
    types << DEF_typeSquare;

    configDialog->addStringChoice(tr("Type de placette"), "", types, _plotType);
    configDialog->addBool(tr("Créer les buffers"), "", "", _createBuffers);
    configDialog->addDouble(tr("Taille de buffer"), "m", 0, 1e+10, 2, _buffer);

}

void ONF_StepCreatePlotsFromList::compute()
{
    QList<CT_ResultGroup*> outResultList = getOutResultList();
    CT_ResultGroup* resOut = outResultList.at(0);

    CT_ResultGroupIterator itOut(resOut, this, DEFin_grpPlotList);
    while (itOut.hasNext() && !isStopped())
    {
        CT_StandardItemGroup* group = (CT_StandardItemGroup*) itOut.next();
        CT_PlotListInGrid* plotList = (CT_PlotListInGrid*) group->firstItemByINModelName(this, DEFin_plotList);

        if (plotList != NULL)
        {
            CT_PlotListInGrid::Type type;
            if (_plotType == DEF_typeCircular)
            {
                type = CT_PlotListInGrid::T_Circle;
            } else {
                type = CT_PlotListInGrid::T_Square;
            }

            QMap<CT_AreaShape2DData*, size_t> plots = plotList->createPlots(type);

            QMapIterator<CT_AreaShape2DData*, size_t> itPl(plots);
            while (itPl.hasNext())
            {
                itPl.next();

                CT_StandardItemGroup* plGroup = new CT_StandardItemGroup(_outGrpPlot_ModelName.completeName(), resOut);
                group->addGroup(plGroup);

                if (_plotType == DEF_typeCircular)
                {
                    CT_Circle2DData* circleData = (CT_Circle2DData*) itPl.key();
                    CT_Circle2D* circle  = new CT_Circle2D(_outPlot_ModelName.completeName(), resOut, circleData);
                    plGroup->addItemDrawable(circle);

                    if (_createBuffers)
                    {
                        CT_Circle2DData* circleDataBuffer = new CT_Circle2DData(circleData->getCenter(), circleData->getRadius() + _buffer);
                        CT_Circle2D* circleBuffer  = new CT_Circle2D(_outPlotBuffer_ModelName.completeName(), resOut, circleDataBuffer);
                        plGroup->addItemDrawable(circleBuffer);
                    }
                } else {
                    CT_Box2DData* squareData = (CT_Box2DData*) itPl.key();
                    CT_Box2D* square     = new CT_Box2D(_outPlot_ModelName.completeName(), resOut, squareData);
                    plGroup->addItemDrawable(square);

                    if (_createBuffers)
                    {
                        CT_Box2DData* squareDataBuffer = new CT_Box2DData(squareData->getCenter(), squareData->getWidth() + 2.0*_buffer, squareData->getHeight() + 2.0*_buffer);
                        CT_Box2D* squareBuffer  = new CT_Box2D(_outPlotBuffer_ModelName.completeName(), resOut, squareDataBuffer);
                        plGroup->addItemDrawable(squareBuffer);
                    }
                }
            }
        }
    }    
}
