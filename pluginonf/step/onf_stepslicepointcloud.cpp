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

#include "onf_stepslicepointcloud.h"

#include "ct_itemdrawable/ct_pointcluster.h"
#include "ct_pointcloudindex/abstract/ct_abstractpointcloudindex.h"
#include "ct_itemdrawable/tools/iterator/ct_groupiterator.h"
#include "ct_result/ct_resultgroup.h"
#include "ct_result/model/inModel/ct_inresultmodelgroup.h"
#include "ct_result/model/outModel/ct_outresultmodelgroup.h"

#include "ct_view/ct_stepconfigurabledialog.h"
#include "ct_pointcloudindex/ct_pointcloudindexvector.h"

#include "ct_iterator/ct_pointiterator.h"

#include <QMessageBox>
#include <limits>

// Alias for indexing models
#define DEFin_resScene "resScene"
#define DEFin_grp "grp"
#define DEFin_scene "scene"

#define DEFout_resScene "resScene"
#define DEFout_rootslice "rootslice"
#define DEFout_slice "slice"
#define DEFout_cluster "cluster"


// Constructor : initialization of parameters
ONF_StepSlicePointCloud::ONF_StepSlicePointCloud(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _dataContainer = new ONF_ActionSlicePointCloud_dataContainer();
    _dataContainer->_thickness = 0.1;
    _dataContainer->_spacing = 0.2;
    _dataContainer->_zBase = 0;
    _manual = false;

    _xmin = 0;
    _ymin = 0;
    _zmin = 0;
    _xmax = 0;
    _ymax = 0;
    _zmax = 0;

    setManual(true);

    _m_status = 0;
    _m_doc = NULL;

    _sceneList = new QList<CT_Scene*>();
}

ONF_StepSlicePointCloud::~ONF_StepSlicePointCloud()
{
    _sceneList->clear();
    delete _sceneList;
    delete _dataContainer;
}

// Step description (tooltip of contextual menu)
QString ONF_StepSlicePointCloud::getStepDescription() const
{
    return tr("Découper une scène en Tranches Horizontales");
}

// Step detailled description
QString ONF_StepSlicePointCloud::getStepDetailledDescription() const
{
    return tr("Action manuelle permettant de découper une scène d'entrée en tranches horizontales.<br>"
              "Il est possible d'en régler intéractivement :<br>"
              "- L'épaisseur (<b>_thickness</b>)<br>"
              "- L'espacement entre deux tranches (<b>_spacing</b>)<br>"
              "<br>"
              "N.B. : Cette étape peut également fonctionner en mode non interactif, avec les paramètres choisis dans la boite de configuration. ");
}

// Step copy method
CT_VirtualAbstractStep* ONF_StepSlicePointCloud::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new ONF_StepSlicePointCloud(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void ONF_StepSlicePointCloud::createInResultModelListProtected()
{
    CT_InResultModelGroup *resIn_resScene = createNewInResultModel(DEFin_resScene, tr("Scène à découper"));
    resIn_resScene->setZeroOrMoreRootGroup();
    resIn_resScene->addGroupModel("", DEFin_grp, CT_AbstractItemGroup::staticGetType(), tr("Groupe"));
    resIn_resScene->addItemModel(DEFin_grp, DEFin_scene, CT_Scene::staticGetType(), tr("Scène à découper"));
}

// Creation and affiliation of OUT models
void ONF_StepSlicePointCloud::createOutResultModelListProtected()
{
    CT_OutResultModelGroup *res_resScene = createNewOutResultModel(DEFout_resScene, tr("Scène découpée"));
    res_resScene->setRootGroup(DEFout_rootslice);
    res_resScene->addGroupModel(DEFout_rootslice, DEFout_slice);
    res_resScene->addItemModel(DEFout_slice, DEFout_cluster, new CT_PointCluster(), tr("Points de la tranche"));
}

// Semi-automatic creation of step parameters DialogBox
void ONF_StepSlicePointCloud::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *dialog = newStandardPostConfigurationDialog();

    dialog->addDouble(tr("Epaisseur des tranches :"), tr("cm"), 0.1, 100000, 2, _dataContainer->_thickness, 100);
    dialog->addDouble(tr("Espacement des tranches :"), tr("cm"), 0, 100000, 2, _dataContainer->_spacing, 100);
    dialog->addBool("","",tr("Choix interactif des paramètres"), _manual);
}

void ONF_StepSlicePointCloud::compute()
{
    setManual(_manual);

    _m_status = 0;

    QList<CT_ResultGroup*> inResultList = getInputResults();
    CT_ResultGroup* resIn_resScene = inResultList.at(0);

    QList<CT_ResultGroup*> outResultList = getOutResultList();
    CT_ResultGroup* res_resScene = outResultList.at(0);

    _xmin = std::numeric_limits<double>::max();
    _ymin = _xmin;
    _zmin = _xmin;

    _xmax = -_xmin;
    _ymax = -_xmin;
    _zmax = -_xmin;

    CT_ResultItemIterator itIn_scene(resIn_resScene, this, DEFin_scene);
    while (itIn_scene.hasNext())
    {
        CT_Scene* itemIn_scene = (CT_Scene*) itIn_scene.next();
        _sceneList->append(itemIn_scene);

        Eigen::Vector3d min, max;
        itemIn_scene->getBoundingBox(min, max);

        if (min(0) < _xmin) {_xmin = min(0);}
        if (min(1) < _ymin) {_ymin = min(1);}
        if (min(2) < _zmin) {_zmin = min(2);}

        if (max(0) > _xmax) {_xmax = max(0);}
        if (max(1) > _ymax) {_ymax = max(1);}
        if (max(2) > _zmax) {_zmax = max(2);}
    }

    _dataContainer->_zBase = _zmin;

    requestManualMode();
    _m_status = 1;

    QMap<QPair<double, double>, CT_PointCluster*> levels;

    for(double base = _dataContainer->_zBase ; base < _zmax ; base = (base + _dataContainer->_thickness + _dataContainer->_spacing))
    {
        levels.insert(QPair<double, double>(base, base + _dataContainer->_thickness), new CT_PointCluster(DEFout_cluster, res_resScene));
    }

    // Do the slices
    for (int sc = 0 ; sc < _sceneList->size() ; sc++)
    {
        CT_Scene* itemIn_scene = (CT_Scene*) _sceneList->at(sc);

        CT_PointIterator itP(itemIn_scene->getPointCloudIndex());

        while(itP.hasNext())
        {
            const CT_Point &point = itP.next().currentPoint();
            size_t index = itP.cIndex();

            bool found = false;

            QMapIterator<QPair<double, double>, CT_PointCluster*> it(levels);
            while (!found && it.hasNext())
            {
                it.next();
                double zminLevel = it.key().first;
                double zmaxLevel = it.key().second;
                CT_PointCluster* cluster = it.value();

                if ((point(2) >= zminLevel) && (point(2) < zmaxLevel))
                {
                    cluster->addPoint(index, false);
                    found = true;
                }
            }
        }
    }

    CT_StandardItemGroup* rootGroup = new CT_StandardItemGroup(DEFout_rootslice, res_resScene);
    res_resScene->addGroup(rootGroup);

    QMapIterator<QPair<double, double>, CT_PointCluster*> it(levels);
    while (it.hasNext())
    {
        it.next();
        CT_PointCluster* cluster = it.value();

        if (cluster->getPointCloudIndex()->size() > 0)
        {
            CT_StandardItemGroup* group = new CT_StandardItemGroup(DEFout_slice, res_resScene);
            group->addItemDrawable(cluster);
            rootGroup->addGroup(group);
        } else {
            delete cluster;
        }
    }

    levels.clear();

    // OUT results creation (move it to the appropried place in the code)
//    CT_StandardItemGroup* grp_slice= new CT_StandardItemGroup(DEFout_slice, res_resScene);
//    res_resScene->addGroup(grp_slice);
    
//    CT_PointCluster* item_cluster = new CT_PointCluster(DEFout_cluster, res_resScene);
//    grp_slice->addItemDrawable(item_cluster);

    requestManualMode();
}

void ONF_StepSlicePointCloud::initManualMode()
{
    if(_m_doc == NULL)
    {
        // create a new 3D document
        _m_doc = getGuiContext()->documentManager()->new3DDocument();

        ONF_ActionSlicePointCloud* action = new ONF_ActionSlicePointCloud(_sceneList, _xmin, _ymin, _zmin, _xmax, _ymax, _zmax, _dataContainer);
        // set the action (a copy of the action is added at all graphics view, and the action passed in parameter is deleted)
        _m_doc->setCurrentAction(action, false);
    }

    QMessageBox::information(NULL,
                             tr("Mode manuel"),
                             tr("Bienvenue dans le mode manuel de cette étape.\n"
                             "Veuillez sélectionner les paramètres pour réaliser les tranches."),
                             QMessageBox::Ok);
}

void ONF_StepSlicePointCloud::useManualMode(bool quit)
{
    if(_m_status == 0)
    {
        if(quit)
        {
        }
    }
    else if(_m_status == 1)
    {
        if(!quit)
        {
            _m_doc = NULL;
            quitManualMode();
        }
    }

}
