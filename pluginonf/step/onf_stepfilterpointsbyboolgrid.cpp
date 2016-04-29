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

#include "onf_stepfilterpointsbyboolgrid.h"

#include "ct_result/model/inModel/ct_inresultmodelgroup.h"
#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"

// Inclusion of standard result class
#include "ct_result/ct_resultgroup.h"

// Inclusion of used ItemDrawable classes
#include "ct_itemdrawable/abstract/ct_abstractitemdrawablewithpointcloud.h"
#include "ct_itemdrawable/ct_grid3d.h"
#include "ct_itemdrawable/ct_scene.h"
#include "ct_pointcloudindex/ct_pointcloudindexvector.h"

#include "ct_iterator/ct_pointiterator.h"

#include <limits>

// Alias for indexing in models
#define DEF_IN_Result_Scenes "rsc"
#define DEF_IN_Group_Scenes "grp"
#define DEF_IN_scene "sc"
#define DEF_IN_Result_Grids "rgr"
#define DEF_IN_GridGroup "gg"
#define DEF_IN_grid "gr"


// Constructor : initialization of parameters
ONF_StepFilterPointsByBoolGrid::ONF_StepFilterPointsByBoolGrid(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
}

// Step description (tooltip of contextual menu)
QString ONF_StepFilterPointsByBoolGrid::getStepDescription() const
{
    return tr("Filtrer les points par une Grille Booléenne");
}

// Step description (tooltip of contextual menu)
QString ONF_StepFilterPointsByBoolGrid::getStepDetailledDescription() const
{
    return tr("Cette étape teste pour chaque point des scènes d'entrée s'il est contenu dans une case \"vraie\" de la grille booléenne choisie. "
              "Si oui, le point est conservé. Sinon, il n'est pas conservé.<br>"
              "Plusieures scènes peuvent être traitées avec la même étape.<br>"
              "Chaque scène filtrée est ajoutée au groupe contenant la grille d'entrée."
              "Si le résultat d'entrée contient plusieurs grilles, une scène est produite pour chacune (sur la base du cumul de toutes les scènes d'entrée)");
}

// Step copy method
CT_VirtualAbstractStep* ONF_StepFilterPointsByBoolGrid::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new ONF_StepFilterPointsByBoolGrid(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void ONF_StepFilterPointsByBoolGrid::createInResultModelListProtected()
{
    CT_InResultModelGroup *resultModel = createNewInResultModel(DEF_IN_Result_Scenes, tr("Scènes à filtrer"), "", true);

    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_IN_Group_Scenes);
    resultModel->addItemModel(DEF_IN_Group_Scenes, DEF_IN_scene, CT_Scene::staticGetType(), tr("Scène"));


    CT_InResultModelGroupToCopy *resultModelGrd = createNewInResultModelForCopy(DEF_IN_Result_Grids, tr("Grille(s) de filtrage"));

    resultModelGrd->setZeroOrMoreRootGroup();
    resultModelGrd->addGroupModel("", DEF_IN_GridGroup);
    resultModelGrd->addItemModel(DEF_IN_GridGroup, DEF_IN_grid, CT_Grid3D<bool>::staticGetType(), tr("Grille(s) de filtrage"));
}

// Creation and affiliation of OUT models
void ONF_StepFilterPointsByBoolGrid::createOutResultModelListProtected()
{
    CT_OutResultModelGroupToCopyPossibilities *res = createNewOutResultModelToCopy(DEF_IN_Result_Grids);

    if(res != NULL)
        res->addItemModel(DEF_IN_GridGroup, _ModelOut_Scene, new CT_Scene(), tr("Scène filtrée"));
}

// Semi-automatic creation of step parameters DialogBox
void ONF_StepFilterPointsByBoolGrid::createPostConfigurationDialog()
{
    // No parameter dialog for this step
}

void ONF_StepFilterPointsByBoolGrid::compute()
{
    // Gets IN result
    CT_ResultGroup* resultIn_Scenes = getInputResultsForModel(DEF_IN_Result_Scenes).first();

    // Gets OUT result
    CT_ResultGroup* resultOut = getOutResultList().first();

    int gridNum = 1;

    // create a iterator to find groups that user selected with the IN model named DEF_IN_GridGroup
    CT_ResultGroupIterator it(resultOut, this, DEF_IN_GridGroup);

    // iterate over all groups
    while(!isStopped() && it.hasNext())
    {
        CT_AbstractItemGroup *group = (CT_AbstractItemGroup*)it.next();
        CT_Grid3D<bool> *boolGrid = (CT_Grid3D<bool>*) group->firstItemByINModelName(this, DEF_IN_grid);

        if (boolGrid != NULL)
        {
            // Indices de la scène filtrée
            CT_PointCloudIndexVector *resPointCloudIndex = new CT_PointCloudIndexVector();

            // BoundingBox de la nouvelle scène
            double xmin = std::numeric_limits<double>::max();
            double ymin = xmin;
            double zmin = xmin;

            double xmax = -xmin;
            double ymax = -xmin;
            double zmax = -xmin;

            int sceneNum = 1;

            CT_ResultItemIterator itSc(resultIn_Scenes, this, DEF_IN_scene);
            while(!isStopped() && itSc.hasNext())
            {
                const CT_Scene *in_scene = (CT_Scene*) itSc.next();

                PS_LOG->addMessage(LogInterface::info, LogInterface::step, QString(tr("Grille %1, Scène %2:")).arg(gridNum++).arg(sceneNum++));

                CT_PointIterator itP(in_scene->getPointCloudIndex());
                size_t n_points = itP.size();

                PS_LOG->addMessage(LogInterface::info, LogInterface::step, QString(tr("La scène %1 points...")).arg(n_points));

                size_t i = 0;

                size_t nbOfFilteredPoints = 0;
                while(itP.hasNext())
                {
                    const CT_Point &point = itP.next().currentPoint();
                    double x = point(0);
                    double y = point(1);
                    double z = point(2);

                    if (boolGrid->valueAtXYZ(x, y, z))
                    {
                        resPointCloudIndex->addIndex(itP.cIndex());

                        if (x < xmin) {xmin = x;}
                        if (x > xmax) {xmax = x;}
                        if (y < ymin) {ymin = y;}
                        if (y > ymax) {ymax = y;}
                        if (z < zmin) {zmin = z;}
                        if (z > zmax) {zmax = z;}
                        ++nbOfFilteredPoints;
                    }

                    ++i;
                    if (i % 1000 == 0) {setProgress(((float)i / (float)n_points) * 99.0);}
                }

                PS_LOG->addMessage(LogInterface::info, LogInterface::step, QString(tr("...%1 points ont été conservés")).arg(nbOfFilteredPoints));
            }

            if (resPointCloudIndex->size() > 0)
            {
                // creation et ajout de la scene
                CT_Scene *outScene = new CT_Scene(_ModelOut_Scene.completeName(), resultOut);

                outScene->setBoundingBox(xmin, ymin, zmin, xmax, ymax, zmax);
                outScene->setPointCloudIndexRegistered(PS_REPOSITORY->registerPointCloudIndex(resPointCloudIndex));
                group->addItemDrawable(outScene);
            }
        }
    }
}



