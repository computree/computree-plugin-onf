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

#include "onf_stepmergescenesbymodality.h"

#include "ct_itemdrawable/ct_scene.h"
#include "ct_pointcloudindex/ct_pointcloudindexvector.h"
#include "ct_itemdrawable/tools/iterator/ct_groupiterator.h"
#include "ct_result/ct_resultgroup.h"
#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/ct_outresultmodelgroupcopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"

#include "ct_view/ct_stepconfigurabledialog.h"

#include "ct_iterator/ct_pointiterator.h"
#include "ct_global/ct_context.h"

//Inclusion of actions
#include "actions/onf_actionaggregateitems.h"

#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
#include <QtConcurrentMap>
#else
#include <QtConcurrent/QtConcurrentMap>
#endif

#include <QMessageBox>
#include <limits>

// Alias for indexing models
#define DEFin_rPos "res"
#define DEFin_rootgrp "rootgrp"
#define DEFin_grp "grp"
#define DEFin_scene "scene"

// Constructor : initialization of parameters
ONF_StepMergeScenesByModality::ONF_StepMergeScenesByModality(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    m_doc = NULL;

    _modalitiesString = "Data,Intermediate,Noise";
    setManual(true);
}

// Step description (tooltip of contextual menu)
QString ONF_StepMergeScenesByModality::getStepDescription() const
{
    return tr("Merge scenes interactively");
}

// Step detailled description
QString ONF_StepMergeScenesByModality::getStepDetailledDescription() const
{
    return tr("TO DO");
}

// Step URL
QString ONF_StepMergeScenesByModality::getStepURL() const
{
    //return tr("STEP URL HERE");
    return CT_AbstractStep::getStepURL(); //by default URL of the plugin
}

// Step copy method
CT_VirtualAbstractStep* ONF_StepMergeScenesByModality::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new ONF_StepMergeScenesByModality(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void ONF_StepMergeScenesByModality::createInResultModelListProtected()
{  
    CT_InResultModelGroupToCopy *resIn = createNewInResultModelForCopy(DEFin_rPos, tr("Scenes"), tr(""), true);
    resIn->setZeroOrMoreRootGroup();
    resIn->addGroupModel("", DEFin_rootgrp);
    resIn->addGroupModel(DEFin_rootgrp, DEFin_grp);
    resIn->addItemModel(DEFin_grp, DEFin_scene, CT_AbstractItemDrawableWithPointCloud::staticGetType(), tr("Scene"));
}


// Creation and affiliation of OUT models
void ONF_StepMergeScenesByModality::createOutResultModelListProtected()
{
    _modalities = _modalitiesString.split(",", QString::SkipEmptyParts);

    _outSceneModelName.resize(_modalities.size());

    CT_OutResultModelGroupToCopyPossibilities *res = createNewOutResultModelToCopy(DEFin_rPos);

    if(res != NULL) {
        for (int i = 0 ; i < _modalities.size() ; i++)
        {
            res->addItemModel(DEFin_rootgrp, _outSceneModelName[i], new CT_Scene(), _modalities.at(i));
        }
    }
}

// Semi-automatic creation of step parameters DialogBox
void ONF_StepMergeScenesByModality::createPreConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();
    configDialog->addString(tr("Modalities (comma separated)"), "", _modalitiesString, tr("List all wanted modalities, separated by commas)"));
}

void ONF_StepMergeScenesByModality::compute()
{

    QList<CT_ResultGroup*> outResultList = getOutResultList();
    CT_ResultGroup* resOut = outResultList.at(0);

    _inputScenes.clear();

    // Création de la liste des positions 2D
    CT_ResultGroupIterator rootGrpIt(resOut, this, DEFin_rootgrp);
    while (rootGrpIt.hasNext() && !isStopped())
    {
        CT_StandardItemGroup* rootGroup = (CT_StandardItemGroup*) rootGrpIt.next();

        CT_GroupIterator grpIt(rootGroup, this, DEFin_grp);
        while (grpIt.hasNext())
        {
            CT_StandardItemGroup* group = (CT_StandardItemGroup*) grpIt.next();
            CT_AbstractItemDrawableWithPointCloud* sceneIn = (CT_AbstractItemDrawableWithPointCloud*)group->firstItemByINModelName(this, DEFin_scene);
            if (sceneIn != NULL)
            {
                _inputScenes.append(sceneIn);
            }
        }



        // Début de la partie interactive
        m_doc = NULL;
        m_status = 0;
        requestManualMode();
        // Fin de la partie interactive


        for (int i = 0 ; i < _modalities.size() ; i++)
        {
            CT_PointCloudIndexVector *outCloudIndex = new CT_PointCloudIndexVector();
            outCloudIndex->setSortType(CT_PointCloudIndexVector::NotSorted);

            for (int j = 0 ; j < _inputScenes.size() ; j++)
            {
                if (_inputScenesModalities.size() > j && _inputScenesModalities.at(j) == _modalities.at(i))
                {
                    CT_AbstractItemDrawableWithPointCloud* sceneIn  = (CT_AbstractItemDrawableWithPointCloud*) _inputScenes.at(j);
                    const CT_AbstractPointCloudIndex *pointCloudIndex = sceneIn->getPointCloudIndex();

                    CT_PointIterator itP(pointCloudIndex);
                    while(itP.hasNext())
                    {
                        size_t index = itP.next().currentGlobalIndex();
                        outCloudIndex->addIndex(index);
                    }
                }
            }

            if (outCloudIndex->size() > 0)
            {
                outCloudIndex->setSortType(CT_PointCloudIndexVector::SortedInAscendingOrder);
                CT_Scene* scene = new CT_Scene(_outSceneModelName[i].completeName(), resOut, PS_REPOSITORY->registerPointCloudIndex(outCloudIndex));
                scene->updateBoundingBox();
                rootGroup->addItemDrawable(scene);
            } else {
                delete outCloudIndex;
            }
        }

        m_status = 1;
        requestManualMode();

    }


    setProgress(100);
}

void ONF_StepMergeScenesByModality::initManualMode()
{
    // create a new 3D document
    if(m_doc == NULL)
        m_doc = getGuiContext()->documentManager()->new3DDocument();

    m_doc->removeAllItemDrawable();

    // set the action (a copy of the action is added at all graphics view, and the action passed in parameter is deleted)
    m_doc->setCurrentAction(new ONF_ActionAggregateItems(_modalities, _inputScenes, _inputScenesModalities));

    QMessageBox::information(NULL, tr("Mode manuel"), tr("Bienvenue dans le mode manuel de cette "
                                                         "étape de filtrage."), QMessageBox::Ok);
}

void ONF_StepMergeScenesByModality::useManualMode(bool quit)
{
    if(m_status == 0)
    {
        if(quit)
        {
        }
    }
    else if(m_status == 1)
    {
        if(!quit)
        {
            m_doc = NULL;

            quitManualMode();
        }
    }
}

