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


#include "onf_stepcomputeverticalprofile.h"

#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"

// Inclusion of actions methods
#include "ct_tools/model/ct_outmodelcopyactionaddmodelitemingroup.h"

// Inclusion of standard result class
#include "ct_result/ct_resultgroup.h"

// Inclusion of used ItemDrawable classes
#include "ct_itemdrawable/ct_scene.h"
#include "ct_itemdrawable/ct_profile.h"
#include "tools/onf_computeverticalprofilethread.h"

#include "ct_view/ct_stepconfigurabledialog.h"

#include <QFileInfo>
#include <QDebug>
#include <limits>

#define DEF_SearchInResult "r"
#define DEF_SearchInScene   "sc"
#define DEF_SearchInGroup   "gr"

ONF_StepComputeVerticalProfile::ONF_StepComputeVerticalProfile(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _res = 0.5;
    _gridMode = 1;
    _xBase = 0;
    _yBase = 0;
    _zBase = 0;
}

QString ONF_StepComputeVerticalProfile::getStepDescription() const
{
    // Gives the descrption to print in the GUI
    return tr("Créer profil vertical de densité de points");
}

// Step description (tooltip of contextual menu)
QString ONF_StepComputeVerticalProfile::getStepDetailledDescription() const
{
    return tr("Cette étape génère une profil selon l'axe Z.");
}

CT_VirtualAbstractStep* ONF_StepComputeVerticalProfile::createNewInstance(CT_StepInitializeData &dataInit)
{
    // Creates an instance of this step
    return new ONF_StepComputeVerticalProfile(dataInit);
}

void ONF_StepComputeVerticalProfile::createInResultModelListProtected()
{
    CT_InResultModelGroupToCopy *resultModel = createNewInResultModelForCopy(DEF_SearchInResult, tr("Scène(s)"));

    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_SearchInGroup);
    resultModel->addItemModel(DEF_SearchInGroup, DEF_SearchInScene, CT_Scene::staticGetType(), tr("Scène"));
}

void ONF_StepComputeVerticalProfile::createOutResultModelListProtected()
{    
    CT_OutResultModelGroupToCopyPossibilities *res = createNewOutResultModelToCopy(DEF_SearchInResult);

    if(res != NULL)
        res->addItemModel(DEF_SearchInGroup, _itemOut_prz_ModelName, new CT_Profile<int>(), tr("ProfilZ"));
}

void ONF_StepComputeVerticalProfile::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addDouble(tr("Résolution du profil"),tr("mètres"),0.0001,10000,2, _res );

    configDialog->addText(tr("Callage de l'origine (X, Y, Z) :"),"", "");

    CT_ButtonGroup &bg_gridMode = configDialog->addButtonGroup(_gridMode);
    configDialog->addExcludeValue("", "", tr("Sur la boite englobante de la scène"), bg_gridMode, 0);
    configDialog->addExcludeValue("", "", tr("Par rapport aux coordonnées suivantes :"), bg_gridMode, 1);

    configDialog->addDouble(tr("Coordonnée X :"), "", -std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), 4, _xBase);
    configDialog->addDouble(tr("Coordonnée Y :"), "", -std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), 4, _yBase);
    configDialog->addDouble(tr("Coordonnée Z :"), "", -std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), 4, _zBase);
}

void ONF_StepComputeVerticalProfile::compute()
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
            CT_Profile<int>* proZ = CT_Profile<int>::createProfileFromSegment(_itemOut_prz_ModelName.completeName(), outResult,
                                                                              minX, minY, minZ,
                                                                              minX, minY, scene->maxZ(),
                                                                              _res, -1, 0);

            group->addItemDrawable(proZ);

            ONF_ComputeVerticalProfileThread* hitsThread = new ONF_ComputeVerticalProfileThread(proZ, scene);
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

void ONF_StepComputeVerticalProfile::updateProgress()
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

