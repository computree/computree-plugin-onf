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


#include "onf_stepcomputehitgrid.h"

#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"

// Inclusion of actions methods
#include "ct_tools/model/ct_outmodelcopyactionaddmodelitemingroup.h"

// Inclusion of standard result class
#include "ct_result/ct_resultgroup.h"

// Inclusion of used ItemDrawable classes
#include "ct_itemdrawable/ct_scene.h"
#include "ct_itemdrawable/ct_grid3d.h"
#include "ct_itemdrawable/ct_grid2dxy.h"
#include "ct_itemdrawable/ct_grid2dxz.h"
#include "ct_itemdrawable/ct_grid2dyz.h"
#include "ct_itemdrawable/ct_profile.h"
#include "tools/onf_computehitsthread.h"

#include "ct_view/ct_stepconfigurabledialog.h"

#include <QFileInfo>
#include <QDebug>
#include <limits>

#define DEF_SearchInResult "r"
#define DEF_SearchInScene   "sc"
#define DEF_SearchInGroup   "gr"

#define DEF_itemOut_grxy "grxy"
#define DEF_itemOut_grxz "grxz"
#define DEF_itemOut_gryz "gryz"

ONF_StepComputeHitGrid::ONF_StepComputeHitGrid(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _res = 0.5;
    _gridMode = 1;
    _xBase = 0;
    _yBase = 0;
    _zBase = 0;

}

QString ONF_StepComputeHitGrid::getStepDescription() const
{
    // Gives the descrption to print in the GUI
    return tr("Création d'une grille 3D de densité de points");
}

// Step description (tooltip of contextual menu)
QString ONF_StepComputeHitGrid::getStepDetailledDescription() const
{
    return tr("Cette étape génère une grille 3D à la <b>résolution</b> spécifiée.<br>"
              "Chaque case reçoit le nombre de points de la scène d'entrée qu'elle contient.<br>"
              "Elle calcule également les profils 2D cumulés en XY, XZ et YZ. ");
}

CT_VirtualAbstractStep* ONF_StepComputeHitGrid::createNewInstance(CT_StepInitializeData &dataInit)
{
    // Creates an instance of this step
    return new ONF_StepComputeHitGrid(dataInit);
}

void ONF_StepComputeHitGrid::createInResultModelListProtected()
{
    CT_InResultModelGroupToCopy *resultModel = createNewInResultModelForCopy(DEF_SearchInResult, tr("Scène(s)"));

    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_SearchInGroup);
    resultModel->addItemModel(DEF_SearchInGroup, DEF_SearchInScene, CT_Scene::staticGetType(), tr("Scène"));
}

void ONF_StepComputeHitGrid::createOutResultModelListProtected()
{    
    CT_OutResultModelGroupToCopyPossibilities *res = createNewOutResultModelToCopy(DEF_SearchInResult);

    res->addItemModel(DEF_SearchInGroup, _itemOut_grxy_ModelName, new CT_Grid2DXY<int>(), tr("DensityXY"));
    res->addItemModel(DEF_SearchInGroup, _itemOut_grxz_ModelName, new CT_Grid2DXZ<int>(), tr("DensityXZ"));
    res->addItemModel(DEF_SearchInGroup, _itemOut_gryz_ModelName, new CT_Grid2DYZ<int>(), tr("DensityYZ"));
    res->addItemModel(DEF_SearchInGroup, _itemOut_prx_ModelName, new CT_Profile<int>(), tr("ProfilX"));
    res->addItemModel(DEF_SearchInGroup, _itemOut_pry_ModelName, new CT_Profile<int>(), tr("ProfilY"));
    res->addItemModel(DEF_SearchInGroup, _itemOut_prz_ModelName, new CT_Profile<int>(), tr("ProfilZ"));
    res->addItemModel(DEF_SearchInGroup, _hits_ModelName, new CT_Grid3D<int>(), tr("Hits"));
}

void ONF_StepComputeHitGrid::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addDouble(tr("Résolution de la grille"),tr("meters"),0.0001,10000,2, _res );

    configDialog->addText(tr("Callage du coin (minX, minY, minZ) :"),"", "");

    CT_ButtonGroup &bg_gridMode = configDialog->addButtonGroup(_gridMode);
    configDialog->addExcludeValue("", "", tr("Sur la boite englobante de la scène"), bg_gridMode, 0);
    configDialog->addExcludeValue("", "", tr("Par rapport aux coordonnées suivantes :"), bg_gridMode, 1);

    configDialog->addDouble(tr("Coordonnée X :"), "", -std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), 4, _xBase);
    configDialog->addDouble(tr("Coordonnée Y :"), "", -std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), 4, _yBase);
    configDialog->addDouble(tr("Coordonnée Z :"), "", -std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), 4, _zBase);
}

void ONF_StepComputeHitGrid::compute()
{
    // Gets the out result
    CT_ResultGroup* outResult = getOutResultList().first();

    qDeleteAll(_threadList);
    _threadList.clear();

    CT_ResultGroupIterator it(outResult, this, DEF_SearchInGroup);
    // iterate over all groups
    while(!isStopped() && it.hasNext())
    {
        CT_AbstractItemGroup *group = (CT_AbstractItemGroup*)it.next();
        const CT_Scene* scene = (CT_Scene*)group->firstItemByINModelName(this, DEF_SearchInScene);

        if (scene!=NULL)
        {
            float minX = _xBase;
            float minY = _yBase;
            float minZ = _zBase;

            if (_gridMode == 0)
            {
                minX = scene->minX();
                minY = scene->minY();
                minZ = scene->minZ();
            } else {

                while (minX < scene->minX()) {minX += _res;};
                while (minY < scene->minY()) {minY += _res;};
                while (minZ < scene->minZ()) {minZ += _res;};

                while (minX > scene->minX()) {minX -= _res;};
                while (minY > scene->minY()) {minY -= _res;};
                while (minZ > scene->minZ()) {minZ -= _res;};
            }

            // Declaring the output grids
            CT_Grid3D<int>* hitGrid = CT_Grid3D<int>::createGrid3DFromXYZCoords(_hits_ModelName.completeName(), outResult,
                                                                                minX, minY, minZ,
                                                                                scene->maxX(), scene->maxY(), scene->maxZ(),
                                                                                _res, -1, 0);

            CT_Grid2DXY<int>* grxy = CT_Grid2DXY<int>::createGrid2DXYFromXYCoords(_itemOut_grxy_ModelName.completeName(), outResult,
                                                                                  minX, minY,
                                                                                  scene->maxX(), scene->maxY(),
                                                                                  _res, minZ, -1, 0);

            CT_Grid2DXZ<int>* grxz = CT_Grid2DXZ<int>::createGrid2DXZFromXZCoords(_itemOut_grxz_ModelName.completeName(), outResult,
                                                                                  minX, minZ,
                                                                                  scene->maxX(), scene->maxZ(),
                                                                                  _res, minY, -1, 0);

            CT_Grid2DYZ<int>* gryz = CT_Grid2DYZ<int>::createGrid2DYZFromYZCoords(_itemOut_gryz_ModelName.completeName(), outResult,
                                                                                  minY, minZ,
                                                                                  scene->maxY(), scene->maxZ(),
                                                                                  _res, minX, -1, 0);

            CT_Profile<int>* proX = CT_Profile<int>::createProfileFromSegment(_itemOut_prx_ModelName.completeName(), outResult,
                                                                              minX, minY, minZ,
                                                                              scene->maxX(), minY, minZ,
                                                                              _res, -1, 0);

            CT_Profile<int>* proY = CT_Profile<int>::createProfileFromSegment(_itemOut_pry_ModelName.completeName(), outResult,
                                                                              minX, minY, minZ,
                                                                              minX, scene->maxY(), minZ,
                                                                              _res, -1, 0);

            CT_Profile<int>* proZ = CT_Profile<int>::createProfileFromSegment(_itemOut_prz_ModelName.completeName(), outResult,
                                                                              minX, minY, minZ,
                                                                              minX, minY, scene->maxZ(),
                                                                              _res, -1, 0);

            grxy->initDrawManager("XY Density", true, true);
            grxz->initDrawManager("XZ Density", true, true);
            gryz->initDrawManager("YZ Density", true, true);

            group->addItemDrawable(hitGrid);
            group->addItemDrawable(grxy);
            group->addItemDrawable(grxz);
            group->addItemDrawable(gryz);
            group->addItemDrawable(proX);
            group->addItemDrawable(proY);
            group->addItemDrawable(proZ);

            ONF_ComputeHitsThread* hitsThread = new ONF_ComputeHitsThread(hitGrid, grxy, grxz, gryz, proX, proY, proZ, scene);
            connect(hitsThread, SIGNAL(progressChanged()), this, SLOT(updateProgress()));
            hitsThread->start();
            _threadList.append(hitsThread);
        }
    }

    int size = _threadList.size();
    for (int i = 0 ; i < size ; ++i)
    {
        _threadList.at(i)->wait();
        updateProgress();
    }

    _mutex.lock();
    qDeleteAll(_threadList);
    _threadList.clear();
    _mutex.unlock();

    setProgress(99);
}

void ONF_StepComputeHitGrid::updateProgress()
{
    float progress = 0;

    _mutex.lock();
    int size = _threadList.size();
    for (int i = 0 ; i < size ; ++i)
    {
        progress += _threadList.at(i)->getProgress();
    }
    _mutex.unlock();

    if(size > 0)
    {
        progress /= (float)size;
        setProgress(progress);
    }
}

