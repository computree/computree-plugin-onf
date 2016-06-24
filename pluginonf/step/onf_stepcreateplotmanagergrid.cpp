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

#include "onf_stepcreateplotmanagergrid.h"

#include "ct_result/ct_resultgroup.h"
#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/ct_outresultmodelgroupcopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"
#include "ct_itemdrawable/tools/iterator/ct_groupiterator.h"
#include "ct_view/ct_stepconfigurabledialog.h"

#include "ct_itemdrawable/abstract/ct_abstractareashape2d.h"

#include "ct_itemdrawable/ct_plotgridmanager.h"
#include "ct_itemdrawable/ct_plotlistingrid.h"


#include <QDebug>

// Alias for indexing models
#define DEFin_result "resultDataSource"
#define DEFin_grpRoot "grproot"
#define DEFin_grpShape2D "grpshape2D"
#define DEFin_shape2D "shape2D"


// Constructor : initialization of parameters
ONF_StepCreatePlotManagerGrid::ONF_StepCreatePlotManagerGrid(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _plotSize = 15.0;
    _plotSpacing = 30.0;
    _xref = 15.0;
    _yref = 15.0;
}

// Step description (tooltip of contextual menu)
QString ONF_StepCreatePlotManagerGrid::getStepDescription() const
{
    return tr("1- Ajoute des gestionnaires de placette (grille)");
}

// Step detailled description
QString ONF_StepCreatePlotManagerGrid::getStepDetailledDescription() const
{
    return tr("No detailled description for this step");
}

// Step URL
QString ONF_StepCreatePlotManagerGrid::getStepURL() const
{
    //return tr("STEP URL HERE");
    return CT_AbstractStep::getStepURL(); //by default URL of the plugin
}

// Step copy method
CT_VirtualAbstractStep* ONF_StepCreatePlotManagerGrid::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new ONF_StepCreatePlotManagerGrid(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void ONF_StepCreatePlotManagerGrid::createInResultModelListProtected()
{
    CT_InResultModelGroupToCopy *resIn = createNewInResultModelForCopy(DEFin_result, tr("Tuiles"), tr(""), true);
    resIn->setZeroOrMoreRootGroup();
    resIn->addGroupModel("", DEFin_grpRoot, CT_AbstractItemGroup::staticGetType(), tr("Groupe Racine"));
    resIn->addGroupModel(DEFin_grpRoot, DEFin_grpShape2D, CT_AbstractItemGroup::staticGetType(), tr("Groupe"));
    resIn->addItemModel(DEFin_grpShape2D, DEFin_shape2D, CT_AbstractAreaShape2D::staticGetType(), tr("Emprise (sans buffer)"));

}

// Creation and affiliation of OUT models
void ONF_StepCreatePlotManagerGrid::createOutResultModelListProtected()
{
    CT_OutResultModelGroupToCopyPossibilities *res = createNewOutResultModelToCopy(DEFin_result);

    if(res != NULL) {
        //res->addItemModel(DEFin_grpRoot, _outPlotManagerGrid_ModelName, new CT_PlotGridManager(), tr("Gestionnaire de placettes (grille)"));
        res->addItemModel(DEFin_grpShape2D, _outPlotList_ModelName, new CT_PlotListInGrid(), tr("Grille de Placettes"));
    }
}

// Semi-automatic creation of step parameters DialogBox
void ONF_StepCreatePlotManagerGrid::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addDouble(tr("Taille de placette (rayon/circulaire ; DEMI-coté/carrée)"), "m", 0, 1e+10, 2, _plotSize);
    configDialog->addDouble(tr("Espacement des placettes"),   "m", 0, 1e+10, 2, _plotSpacing);

    configDialog->addEmpty();
    configDialog->addTitle(tr("Coordonnées de références pour les centres de placettes :"));
    configDialog->addDouble(tr("Coordonnée X de référence "), "m", 0, 1e+10, 2, _xref);
    configDialog->addDouble(tr("Coordonnée y de référence "), "m", 0, 1e+10, 2, _yref);
}

void ONF_StepCreatePlotManagerGrid::compute()
{
    QList<CT_ResultGroup*> outResultList = getOutResultList();
    CT_ResultGroup* resOut = outResultList.at(0);

    Eigen::Vector2d refCoords(_xref, _yref);

    Eigen::Vector2d  minAll, maxAll;
    minAll(0) = std::numeric_limits<double>::max();
    minAll(1) = std::numeric_limits<double>::max();
    maxAll(0) = -std::numeric_limits<double>::max();
    maxAll(1) = -std::numeric_limits<double>::max();

    CT_ResultGroupIterator itOut(resOut, this, DEFin_grpRoot);
    while (itOut.hasNext() && !isStopped())
    {
        CT_StandardItemGroup* group = (CT_StandardItemGroup*) itOut.next();

        QList<CT_PlotListInGrid*> plotLists;

        // Add a plot list for each input shape
        CT_GroupIterator itGrpShape(group, this, DEFin_grpShape2D);
        while (itGrpShape.hasNext() && !isStopped())
        {
            CT_StandardItemGroup* grpShape = (CT_StandardItemGroup*) itGrpShape.next();
            CT_AbstractAreaShape2D* shape = (CT_AbstractAreaShape2D*) grpShape->firstItemByINModelName(this, DEFin_shape2D);
            if (shape != NULL)
            {
                CT_AreaShape2DData* data = (CT_AreaShape2DData*) shape->getPointerData();

                if (data != NULL)
                {
                    CT_PlotListInGrid* plotList = new CT_PlotListInGrid(_outPlotList_ModelName.completeName(), resOut, data, refCoords, _plotSpacing, _plotSize);
                    plotLists.append(plotList);

                    Eigen::Vector2d  min, max;
                    plotList->getBoundingBox2D(min, max);

                    if (min(0) < minAll(0)) {minAll(0) = min(0);}
                    if (min(1) < minAll(1)) {minAll(1) = min(1);}
                    if (max(0) > maxAll(0)) {maxAll(0) = max(0);}
                    if (max(1) > maxAll(1)) {maxAll(1) = max(1);}

                    grpShape->addItemDrawable(plotList);
                }
            }
        }

        size_t indexJump = std::floor((maxAll(0) - minAll(0)) / _plotSpacing) + 1;

        // Set the index bounds for each plot list
        for (int i = 0 ; i < plotLists.size() ; i++)
        {
            CT_PlotListInGrid* plotList = plotLists.at(i);
            Eigen::Vector2d  min, max;
            plotList->getBoundingBox2D(min, max);

            size_t firstIndex = std::floor((min(0) - minAll(0)) / _plotSpacing) + 1 + std::floor((maxAll(1) - max(1)) / _plotSpacing) * indexJump;

            plotList->setIndices(firstIndex, indexJump);
        }


        // create and add manager
        //CT_PlotManager* plotmanager = new CT_PlotManager(_outPlotManagerGrid.completeName(), resOut, new CT_PlotManager_grid(0, 0, 0, 0, type));
        //group->addItemDrawable(plotmanager);
    }    
}
