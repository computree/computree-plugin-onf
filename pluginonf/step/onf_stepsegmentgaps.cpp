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

#include "onf_stepsegmentgaps.h"

// Inclusion of in models
#include "ct_result/model/inModel/ct_inresultmodelgroup.h"

// Inclusion of out models
#include "ct_result/model/outModel/ct_outresultmodelgroup.h"

#include "ct_view/ct_stepconfigurabledialog.h"

// Inclusion of results
#include "ct_result/ct_resultgroup.h"

// Inclusion of used ItemDrawable classes
#include "ct_itemdrawable/ct_grid3d.h"
#include "ct_itemdrawable/ct_scene.h"
#include "ct_triangulation/ct_nodet.h"
#include "ct_triangulation/ct_trianglet.h"
#include "ct_triangulation/ct_edget.h"
#include "ct_itemdrawable/ct_attributeslist.h"

#include <limits>
#include <QMessageBox>
#include <QDebug>

// Alias for indexing in models
#define DEF_resultIn "result"
#define DEF_GroupScene "grp"
#define DEF_itemScene "scene"

#define DEF_resultOut "result"
#define DEF_SearchOutGroup "group"
#define DEF_SearchOutDensityGrid "density"
#define DEF_SearchOutMNSGrid "mns"
#define DEF_SearchOutClustersGrid "cluster"

#define DEF_SearchOutGapAttributes "grattributes"
#define DEF_SearchOutGroupGap "gscene"
#define DEF_SearchOutGapArea "gapArea"
#define DEF_SearchOutGapClusterId "gapClusterId"

// Constructor : initialization of parameters
ONF_StepSegmentGaps::ONF_StepSegmentGaps(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    m_doc = NULL;
    _keepOnlyGapsInConvexHull = true;
    setManual(true);
}

// Step description (tooltip of contextual menu)
QString ONF_StepSegmentGaps::getStepDescription() const
{
    return tr("Segmentation de trouées en 2D");
}

// Step description (tooltip of contextual menu)
QString ONF_StepSegmentGaps::getStepDetailledDescription() const
{
    return tr("Cette étape permet de segmenter des trouées manuellement dans un plan horizontal.<br>"
              "Une première phase manuelle permet de déteminer la tranche verticale sur laquelle les points seront analysés. "
              "On y définit un niveau Z minimum, un niveau Z maximum, ainsi qu'une résolution pour les rasters de travail :"
              "<ul>"
              "<li>Le Modèle Numérique de Surface (MNS) = hauteur du point le plus haut pour chaque case</li>"
              "<li>La densité de point par case</li>"
              "</ul>"
              "Les rasters sont ensuite utilisés dans une seconde phase, afin de segmenter les trouées. "
              "Un système par couleurs permet de façon semi-automatique de délimiter l'emprise horizontale de chaque trouée. "
              "Sur la base d'une pré-segmentation, l'opérateur peut modifier les groupes (trouées) en les fusionnant ou en les divisant.<br>"
              "En sortie, cette étapes produit :"
              "<ul>"
              "<li>Un raster avec une valeur entière différente pour chaque trouée</li>"
              "<li>Une métrique donnant la surface des pixels pour chaque trouée</li>"
              "</ul>");
}

// Step copy method
CT_VirtualAbstractStep* ONF_StepSegmentGaps::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new ONF_StepSegmentGaps(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void ONF_StepSegmentGaps::createInResultModelListProtected()
{
    CT_InResultModelGroup *resultModel = createNewInResultModel(DEF_resultIn, tr("Scène(s)"));

    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_GroupScene);
    resultModel->addItemModel(DEF_GroupScene, DEF_itemScene, CT_Scene::staticGetType(), tr("Scène"));
}

// Creation and affiliation of OUT models
void ONF_StepSegmentGaps::createOutResultModelListProtected()
{
    CT_OutResultModelGroup *resultModel = createNewOutResultModel(DEF_resultOut, tr("Densité, MNS et clusters"));

    resultModel->setRootGroup(DEF_SearchOutGroup);
    resultModel->addItemModel(DEF_SearchOutGroup, DEF_SearchOutMNSGrid, new CT_Grid2DXY<double>(), tr("MNS"));
    resultModel->addItemModel(DEF_SearchOutGroup, DEF_SearchOutDensityGrid, new CT_Grid2DXY<int>(), tr("Densité"));
    resultModel->addItemModel(DEF_SearchOutGroup, DEF_SearchOutClustersGrid, new CT_Grid2DXY<int>(), tr("Clusters"));

    resultModel->addGroupModel(DEF_SearchOutGroup, DEF_SearchOutGroupGap);
    resultModel->addItemModel(DEF_SearchOutGroupGap, DEF_SearchOutGapAttributes, new CT_AttributesList(), tr("Attributs de la trouée"));
    resultModel->addItemAttributeModel(DEF_SearchOutGapAttributes, DEF_SearchOutGapArea,
                                       new CT_StdItemAttributeT<double>(NULL, PS_CATEGORY_MANAGER->findByUniqueName(CT_AbstractCategory::DATA_AREA), NULL, 0),
                                       tr("Aire de trouée"));
    resultModel->addItemAttributeModel(DEF_SearchOutGapAttributes, DEF_SearchOutGapClusterId,
                                       new CT_StdItemAttributeT<int>(NULL, PS_CATEGORY_MANAGER->findByUniqueName(CT_AbstractCategory::DATA_ID), NULL, 0),
                                       tr("ClusterID"));
}

// Semi-automatic creation of step parameters DialogBox
void ONF_StepSegmentGaps::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addBool(tr("Ne détécter les trouées que dans l'enveloppe convexe des points"),"","", _keepOnlyGapsInConvexHull);
}


void ONF_StepSegmentGaps::compute()
{
    _gridContainer = new ONF_ActionDefineHeightLayer_gridContainer();

    m_doc = NULL;

    CT_ResultGroup *inResult = getInputResults().first();

    _outResult = getOutResultList().at(0);

    m_itemDrawableToAdd.clear();

    _xmin = std::numeric_limits<double>::max();
    _ymin = std::numeric_limits<double>::max();
    _zmin = std::numeric_limits<double>::max();

    _xmax = -std::numeric_limits<double>::max();
    _ymax = -std::numeric_limits<double>::max();
    _zmax = -std::numeric_limits<double>::max();

    _clustersGrid = NULL;

    CT_ResultItemIterator itRes(inResult, this, DEF_itemScene);
    while(!isStopped() && itRes.hasNext())
    {
        CT_Scene *scene = (CT_Scene*) itRes.next();

        if (scene->minX() < _xmin) {_xmin = scene->minX();}
        if (scene->minY() < _ymin) {_ymin = scene->minY();}
        if (scene->minZ() < _zmin) {_zmin = scene->minZ();}

        if (scene->maxX() > _xmax) {_xmax = scene->maxX();}
        if (scene->maxY() > _ymax) {_ymax = scene->maxY();}
        if (scene->maxZ() > _zmax) {_zmax = scene->maxZ();}

        m_itemDrawableToAdd.append(scene);
    }

    if (!m_itemDrawableToAdd.isEmpty())
    {
        PS_LOG->addMessage(LogInterface::info, LogInterface::step,tr("Selection de l'épaisseur des houppiers"));

        m_status = 0;
        requestManualMode();

        CT_StandardItemGroup *group = new CT_StandardItemGroup(DEF_SearchOutGroup, _outResult);
        _outResult->addGroup(group);

        if (_gridContainer->_densityGrid!=NULL)
        {
            group->addItemDrawable(_gridContainer->_densityGrid);
        }

        if (_gridContainer->_mnsGrid!=NULL)
        {
            group->addItemDrawable(_gridContainer->_mnsGrid);

            _clustersGrid = new CT_Grid2DXY<int>(DEF_SearchOutClustersGrid, _outResult,
                                                 _gridContainer->_mnsGrid->minX(),
                                                 _gridContainer->_mnsGrid->minY(),
                                                 _gridContainer->_mnsGrid->colDim(),
                                                 _gridContainer->_mnsGrid->linDim(),
                                                 _gridContainer->_mnsGrid->resolution(),
                                                 _gridContainer->_mnsGrid->level(),
                                                 -1, -1);
        }

        if (_gridContainer->_densityGrid!=NULL &&
                _gridContainer->_mnsGrid!=NULL &&
                _clustersGrid!=NULL)
        {
            PS_LOG->addMessage(LogInterface::info, LogInterface::step,tr("Création des clusters"));

            m_status = 1;
            requestManualMode();

            group->addItemDrawable(_clustersGrid);
            _zmin = _gridContainer->_zmin;
            _zmax = _gridContainer->_zmax;

            m_status = 2;
            requestManualMode();


            PS_LOG->addMessage(LogInterface::info, LogInterface::step,tr("Démarrage des post_traitements"));
            PS_LOG->addMessage(LogInterface::info, LogInterface::step,tr("Elimination des clusters vides"));
            dropEmptyClusters();

            PS_LOG->addMessage(LogInterface::info, LogInterface::step,tr("Enregistrement des clusters"));
            QMap<int, QList<Eigen::Vector2d*>*> cellsMapByCluster;
            QMap<int, size_t> clusterCounts;
            registerClusterCells(cellsMapByCluster, clusterCounts);


            PS_LOG->addMessage(LogInterface::info, LogInterface::step,tr("Calcul des métriques"));
            computeMetrics(clusterCounts, group);

            QMapIterator<int, QList<Eigen::Vector2d *> *> it(cellsMapByCluster);
            while (it.hasNext())
            {
                it.next();
                QList<Eigen::Vector2d*> *cells = it.value();

                qDeleteAll(*cells);
                delete cells;
            }

        } else {
            QMessageBox::information(NULL, tr("Segmentation des trouées"), tr("Phase 2 (Segmentation des trouées) impossible à réaliser."), QMessageBox::Ok);
            m_status = 2;
            requestManualMode();
        }

        PS_LOG->addMessage(LogInterface::info, LogInterface::step,tr("Post-Traitements terminés"));
    }

    delete _gridContainer;
}

int ONF_StepSegmentGaps::dropEmptyClusters()
{
    // Cluster 0 : réservé (à ne pas prendre en compte)
    int newClusterNumber = 1;

    QMap<int, int> clustersMap;
    size_t ncells = _clustersGrid->nCells();
    for (size_t i = 0 ; i < ncells ; i++)
    {
        int value = _clustersGrid->valueAtIndex(i);
        if (value > 0 && !clustersMap.contains(value))
        {
            clustersMap.insert(value, newClusterNumber++);
        }
    }

    for (size_t i = 0 ; i < ncells ; i++)
    {
        int value = _clustersGrid->valueAtIndex(i);
        if (value > 0)
        {
            _clustersGrid->setValueAtIndex(i, clustersMap.value(value, -1));
        }
    }
    clustersMap.clear();

    // Calcul des limites
    _clustersGrid->computeMinMax();
    return newClusterNumber;
}


void ONF_StepSegmentGaps::registerClusterCells(QMap<int, QList<Eigen::Vector2d*> *> &cellsMapByCluster, QMap<int, size_t> &clusterCounts)
{
    for (size_t c = 0 ; c < _clustersGrid->colDim() ; c++)
    {
        for (size_t l = 0 ; l < _clustersGrid->linDim() ; l++)
        {
            Eigen::Vector2d* point = new Eigen::Vector2d(_clustersGrid->getCellCenterColCoord(c), _clustersGrid->getCellCenterLinCoord(l));

            int cluster = _clustersGrid->value(c, l);

            if (cluster >= 0)
            {
                clusterCounts.insert(cluster, clusterCounts.value(cluster, 0) + 1);

                QList<Eigen::Vector2d*> *liste = cellsMapByCluster.value(cluster, NULL);
                if (liste == NULL)
                {
                    liste = new QList<Eigen::Vector2d*>();
                    cellsMapByCluster.insert(cluster, liste);
                }
                liste->append(point);

            }
        }
    }
}


void ONF_StepSegmentGaps::computeMetrics(QMap<int, size_t> &clusterCounts, CT_StandardItemGroup* baseGroup)
{
    // aire d'une cellule
    double base_area = _clustersGrid->resolution()*_clustersGrid->resolution();

    QMapIterator<int, size_t> it(clusterCounts);
    while (it.hasNext())
    {
        it.next();
        int cluster = it.key();
        double gapArea = it.value() * base_area;

        CT_StandardItemGroup* groupGap = new CT_StandardItemGroup(DEF_SearchOutGroupGap, _outResult);
        baseGroup->addGroup(groupGap);

        CT_AttributesList* crAttributes= new CT_AttributesList(DEF_SearchOutGapAttributes, _outResult);
        groupGap->addItemDrawable(crAttributes);
        crAttributes->addItemAttribute(new CT_StdItemAttributeT<double>(DEF_SearchOutGapArea,
                                                                       CT_AbstractCategory::DATA_AREA,
                                                                       _outResult, gapArea));
        crAttributes->addItemAttribute(new CT_StdItemAttributeT<int>(DEF_SearchOutGapClusterId,
                                                                     CT_AbstractCategory::DATA_ID,
                                                                     _outResult, cluster));
    }
}

void ONF_StepSegmentGaps::initManualMode()
{
    if(m_doc == NULL)
    {
        // create a new 3D document
        m_doc = getGuiContext()->documentManager()->new3DDocument();
    }

    m_doc->removeAllItemDrawable();

    QListIterator<CT_Scene*> it(m_itemDrawableToAdd);

    while(it.hasNext())
    {
        m_doc->addItemDrawable(*it.next());
    }
}

void ONF_StepSegmentGaps::useManualMode(bool quit)
{
    if(m_status == 0)
    {
        if(!quit)
        {

            QMessageBox::information(NULL, tr("Segmentation des trouées"), tr("Mode manuel.\n"
                                                                              "Phase 1 : Calcul de la carte de densité et du MNS.\n"
                                                                              "L'initialisation peut prendre un peu de temps..."), QMessageBox::Ok);

            _action = new ONF_ActionDefineHeightLayer(_outResult,
                                                     DEF_SearchOutDensityGrid,
                                                     DEF_SearchOutMNSGrid,
                                                     _gridContainer,
                                                     m_itemDrawableToAdd,
                                                     _xmin, _ymin, _zmin, _xmax, _ymax, _zmax);

            m_doc->setCurrentAction(_action, false);
        }
    }else if(m_status == 1)
    {
        if(!quit)
        {
            if (_action!= NULL) {delete _action;}

            getGuiContext()->actionsManager()->removeAction("ONF_ActionDefineHeightLayer");

            QMessageBox::information(NULL, tr("Segmentation des trouées"), tr("Mode manuel.\n"
                                                                              "Phase 2 : Segmentation des trouées.\n"
                                                                              "La pré-segmentation automatique peut prendre un peu de temps..."), QMessageBox::Ok);

            _action = new ONF_ActionSegmentGaps(_gridContainer->_densityGrid,
                                               _gridContainer->_mnsGrid,
                                               _clustersGrid, _keepOnlyGapsInConvexHull);

            m_doc->setCurrentAction(_action);
            _action = NULL;
        }
    }
    else if(m_status == 2)
    {
        if(!quit)
        {
            m_doc->removeAllItemDrawable();
            m_doc = NULL;

            quitManualMode();

            QMessageBox::information(NULL, tr("Segmentation des trouées"), tr("Fin du mode manuel, démarrage des post-traitements\n"
                                                                              "Cela peut prendre un peu de temps..."), QMessageBox::Ok);
        }
    }
}
