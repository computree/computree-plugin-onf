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

#include "ct_itemdrawable/ct_plotmanageritem.h"
#include "ct_plotmanager/ct_plotmanager_grid.h"

// Alias for indexing models
#define DEFin_result "resultDataSource"
#define DEFin_grpShape2D "grpshape2D"
#define DEFin_shape2D "shape2D"


#define DEF_typeSquare tr("Carrée")
#define DEF_typeCircular tr("Circulaire")

// Constructor : initialization of parameters
ONF_StepCreatePlotManagerGrid::ONF_StepCreatePlotManagerGrid(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _plotType = DEF_typeCircular;
    _plotSize = 15.0;
    _plotSpacing = 30.0;
    _xref = 15.0;
    _yref = 15.0;
}

// Step description (tooltip of contextual menu)
QString ONF_StepCreatePlotManagerGrid::getStepDescription() const
{
    return tr("Ajoute des gestionnaires de placette (grille)");
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
    resIn->addGroupModel("", DEFin_grpShape2D, CT_AbstractItemGroup::staticGetType(), tr("Groupe"));
    resIn->addItemModel(DEFin_grpShape2D, DEFin_shape2D, CT_AbstractAreaShape2D::staticGetType(), tr("Emprise (sans buffer)"));

}

// Creation and affiliation of OUT models
void ONF_StepCreatePlotManagerGrid::createOutResultModelListProtected()
{
    CT_OutResultModelGroupToCopyPossibilities *res = createNewOutResultModelToCopy(DEFin_result);
    res->addItemModel(DEFin_grpShape2D, _outPlotManagerGrid, new CT_PlotManagerItem(), tr("Gestionnaire de placettes (grille)"));
}

// Semi-automatic creation of step parameters DialogBox
void ONF_StepCreatePlotManagerGrid::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    QStringList types;
    types << DEF_typeCircular;
    types << DEF_typeSquare;

    configDialog->addStringChoice(tr("Type de placette"), "", types, _plotType);
    configDialog->addDouble(tr("Taille de placette (rayon/circulaire ; coté/carrée)"), "m", 0, 1e+10, 2, _plotSize);
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

    CT_ResultItemIterator itOut(resOut, this, DEFin_grpShape2D);
    while (itOut.hasNext() && !isStopped())
    {
        CT_StandardItemGroup* group = (CT_StandardItemGroup*) itOut.next();
        CT_AbstractAreaShape2D* shape = (CT_AbstractAreaShape2D*) group->firstItemByINModelName(this, DEFin_shape2D);

        if (shape != NULL)
        {
            // select type of plot
            CT_PlotManager_grid::Type type = CT_PlotManager_grid::T_Circle;
            if (_plotType == DEF_typeSquare) {type = CT_PlotManager_grid::T_Square;}

            // compute min (x,y) value and the number of row/col
            Eigen::Vector3d min, max;
            shape->getBoundingBox(min, max);

            Eigen::Vector2d minBB, maxBB;

            minBB(0) = std::floor((min(0) - _xref) / _plotSpacing) * _plotSpacing + _xref;
            minBB(1) = std::floor((min(1) - _yref) / _plotSpacing) * _plotSpacing + _yref;

            while (minBB(0) < min(0)) {minBB(0) += _plotSpacing;}
            while (minBB(1) < min(1)) {minBB(1) += _plotSpacing;}

            maxBB(0) = minBB(0);
            maxBB(1) = minBB(1);

            size_t ncol = 0;
            size_t nrow = 0;

            while (maxBB(0) < max(0)) {ncol++; maxBB(0) += _plotSpacing;}
            while (maxBB(1) < max(1)) {nrow++; maxBB(1) += _plotSpacing;}


            // create and add manager
            CT_PlotManagerItem* plotmanager = new CT_PlotManagerItem(_outPlotManagerGrid.completeName(), resOut, new CT_PlotManager_grid(0, 0, 0, 0, type));
            group->addItemDrawable(plotmanager);
        }
    }
    
}
