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


#include "onf_stepdetectsection07.h"

#include "ct_itemdrawable/model/outModel/ct_outstdgroupmodel.h"

#include "ct_result/model/inModel/ct_inresultmodelgroup.h"
#include "ct_result/model/outModel/ct_outresultmodelgroup.h"

#include "ct_result/ct_resultgroup.h"
#include "ct_itemdrawable/ct_pointcluster.h"
#include "ct_itemdrawable/ct_circle.h"
#include "ct_turn/inTurn/tools/ct_inturnmanager.h"
#include "ct_tools/model/ct_generateoutmodelname.h"
#include "ct_itemdrawable/tools/iterator/ct_groupiterator.h"
#include "ct_math/ct_mathboundingshape.h"

#include "ct_math/ct_mathboundingshape.h"

#include "ct_view/ct_stepconfigurabledialog.h"

#include "ct_model/inModel/tools/ct_instdmodelpossibility.h"

#include "ct_model/tools/ct_modelsearchhelper.h"

#include <cmath>
#include <iostream>
#include <QList>

#include "qdebug.h"


#define DEF_SearchInResult  "r"
#define DEF_SearchInLayer   "layer"
#define DEF_SearchInGroup   "g"
#define DEF_SearchInCluster "cl"

#define DEF_SearchOutResult "r"

ONF_StepDetectSection07::ONF_StepDetectSection07(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _deltaz = 0.1;
}

QString ONF_StepDetectSection07::getStepDescription() const
{
    return tr("1- Aggréger verticalement les Clusters en Billons");
}

QString ONF_StepDetectSection07::getStepDetailledDescription() const
{
    return tr("Cette étape prend en entrée des couches horizontales (layers) contenant des clusters.<br>"
              "Ce type de structure peut par exemple être produite par l'étape <em>ONF_StepHorizontalClustering</em>.<br>"
              "Les clusters adjacents verticalement sont regroupés en billons (groupes). Pour ce faire :"
              "<ul>"
              "<li> Les clusters dont la <b>distance verticale</b> les séparant est inférieure au seuil choisi sont comparés deux à deux.</li>"
              "<li>Si leurs boites englobantes s'intersectent dans le plan XY, les clusters sont regroupés dans la même billon.</li>"
              "</ul>"
              "N.B. : Les clusters ayant la plus grande boite englobante XY sont prioritaires.");
}

CT_VirtualAbstractStep* ONF_StepDetectSection07::createNewInstance(CT_StepInitializeData &dataInit)
{
    // cree une copie de cette etape
    return new ONF_StepDetectSection07(dataInit);
}

//////////////////// PROTECTED //////////////////

void ONF_StepDetectSection07::createInResultModelListProtected()
{
    CT_InResultModelGroup * resultModel = createNewInResultModel(DEF_SearchInResult, tr("Clusters"));
    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_SearchInLayer, CT_AbstractItemGroup::staticGetType(), tr("Niveau Z (Grp)"));
    resultModel->addGroupModel(DEF_SearchInLayer, DEF_SearchInGroup, CT_AbstractItemGroup::staticGetType(), tr("Cluster (Grp)"));
    resultModel->addItemModel(DEF_SearchInGroup, DEF_SearchInCluster, CT_PointCluster::staticGetType(), tr("Points"));

    resultModel->setMaximumNumberOfPossibilityThatCanBeSelectedForOneTurn(1);
}

void ONF_StepDetectSection07::createPostConfigurationDialog()
{
    // création effective de la configurationDialog
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();
    configDialog->addDouble(tr("Distance en z (en + et en -) maximum entre deux groupes de points à comparer"), "cm", 0, 1000, 2, _deltaz, 100);
}

void ONF_StepDetectSection07::preProcessCreateOutResultModelListProtected()
{
    CT_GenerateOutModelName generator;
    QList<QString> generatedNames;
    CT_InTurnManager *tm = getInTurnManager();

    _outclustergroupname = generator.getNewModelNameThatDontExistIn(*tm, generatedNames, "gcl");
    generatedNames.append(_outclustergroupname);
    _outsectiongroupname = generator.getNewModelNameThatDontExistIn(*tm, generatedNames, "gse");
}

void ONF_StepDetectSection07::createOutResultModelListProtected()
{
    // récupération du modèle in du résultat
    CT_InAbstractResultModel *resultmodel = getInResultModel(DEF_SearchInResult);

    if(!isCreateDefaultOutModelActive()
            && resultmodel->getPossibilitiesSavedSelected().isEmpty())
        qFatal("Error while create out model in ONF_StepDetectSection07");

    if(!isCreateDefaultOutModelActive())
    {
        // récupération du modèle out de LA possibilité liée au modèle de résultat (il n'y en a qu'une ici) comme spécifié implicitement dans addInResultModel
        CT_OutAbstractResultModel *resultpossibilitymodel = (CT_OutAbstractResultModel*)resultmodel->getPossibilitiesSavedSelected().first()->outModel();
        // récupération du modèle in de PointCluster
        CT_InAbstractModel *pointclustermodel = (CT_InAbstractModel*)PS_MODELS->searchModel(DEF_SearchInCluster, resultpossibilitymodel, this);
        // récupération du modèle out de LA possilibité liée au modèle de Pointcluster (il n'y en a qu'une ici) CAR on a utilisé C_ChooseOneIfMultiple
        CT_OutAbstractModel *pointclusterpossibilitymodel = pointclustermodel->getPossibilitiesSavedSelected().first()->outModel();
        // récupération du modèle du groupe contenant le PointCluster
        DEF_CT_AbstractGroupModelOut *pointclustergroupmodel = dynamic_cast<DEF_CT_AbstractGroupModelOut*> (pointclusterpossibilitymodel->parentModel()); //groupe qui le pointcluster

        // On copie le modèle du groupe
        DEF_CT_AbstractGroupModelOut *pointclustergroupoutmodel = pointclustergroupmodel->copyGroup();
        pointclustergroupoutmodel->setUniqueName(_outclustergroupname);

        // cette methode est appele apres avoir affiche la fenetre de post-configuration
        CT_OutStdGroupModel *sectiongroup = new CT_OutStdGroupModel(_outsectiongroupname, new CT_StandardItemGroup(), tr("Billon"));
        sectiongroup->addGroup(pointclustergroupoutmodel);

        // genere un resultat qui contient des groupes (sections) qui contiennent des groupes qui contiennent chacun un pointcluster
        CT_OutResultModelGroup *outRModel = createNewOutResultModel(DEF_SearchOutResult, tr("Billons"));
        outRModel->setRootGroup(sectiongroup);
    }
}

void ONF_StepDetectSection07::compute()
{
    CT_ResultGroup *inResult = getInputResults().first();
    CT_ResultGroup *outResult = getOutResultList().first();

    const CT_OutAbstractGroupModel* outClusterGroupModel = (CT_OutAbstractGroupModel*)PS_MODELS->searchModelForCreation(_outclustergroupname, outResult);


    // Layers triés par Z croissant, pour chacun : clusters triés par aire XY croissante
    QMap<float, QMultiMap<float, CT_PointCluster*>* > layersZMap;

    // Correspondance entre clusters et groupes copiés
    QMap<CT_PointCluster*, CT_AbstractItemGroup*> clusterToGroups;

    // Aires des clusters
    QMap<CT_PointCluster*, float> clustersArea;

    // Boucle sur les layers
    CT_ResultGroupIterator itLayer(inResult, this, DEF_SearchInLayer);
    while (itLayer.hasNext() && !isStopped())
    {
        CT_AbstractItemGroup* layer = (CT_AbstractItemGroup*) itLayer.next();
        bool firstCluster = true;
        float zLevel = 0;
        QMultiMap<float, CT_PointCluster*> *clusterMap;

        // Boucle sur les clusters du layer
        CT_GroupIterator itGrp(layer, this, DEF_SearchInGroup);
        while (itGrp.hasNext() && (!isStopped()))
        {
            CT_AbstractItemGroup* groupWithCluster = (CT_AbstractItemGroup*) itGrp.next();
            CT_PointCluster *cluster = (CT_PointCluster*)groupWithCluster->firstItemByINModelName(this, DEF_SearchInCluster);
            if(cluster != NULL)
            {
                if (firstCluster)
                {
                    firstCluster = false;
                    zLevel = cluster->getCenterZ();
                    clusterMap = new QMultiMap<float, CT_PointCluster*>();
                    layersZMap.insert(zLevel, clusterMap);
                }


                CT_AbstractItemGroup *outClusterGroup = (CT_AbstractItemGroup*) groupWithCluster->copy(outClusterGroupModel, outResult, CT_ResultCopyModeList() << CT_ResultCopyModeList::CopyItemDrawableReference);
                clusterToGroups.insert(cluster, outClusterGroup);

                Eigen::Vector3d min, max;
                cluster->getBoundingBox(min, max);
                float area = std::abs((max.x() - min.x()) * (max.y() - min.y()));
                clusterMap->insertMulti(area, cluster);
                clustersArea.insert(cluster, area);
            }
        }
    }

    setProgress( 25 );


    // Sections de sortie, triées par aire XY croissante
    QMultiMap<float, QList<CT_PointCluster*>* > sectionMap;

    // Section "terminées"
    QList<QList<CT_PointCluster*>* > finishedSections;

    // Boucle sur les layers
    QMapIterator<float, QMultiMap<float, CT_PointCluster*>* > itZMap(layersZMap);
    while (itZMap.hasNext() && !isStopped())
    {
        itZMap.next();
        // Récupération des clusters du layer en cours
        QMultiMap<float, CT_PointCluster*>* clusterMap = itZMap.value();
        float zLevel = itZMap.key();

        // Parcours des sections existantes
        QList<QList<CT_PointCluster*>*> sections = sectionMap.values();
        sectionMap.clear();
        while (!sections.isEmpty()&& !isStopped())
        {
            QList<CT_PointCluster*>* section = sections.takeLast();

            // La section testée doit être en dessous du layer ET, ne pas être à moins de la hauteur du layer - _deltaZ
            if ((section->last()->getCenterZ() <= zLevel) && (section->last()->getCenterZ() >= (zLevel - _deltaz)))
            {
                CT_PointCluster* sectionCluster = section->last();
                Eigen::Vector3d minSectionCluster, maxSectionCluster;
                sectionCluster->getBoundingBox(minSectionCluster, maxSectionCluster);

                // Boucle sur les clusters du layer
                QMutableMapIterator<float, CT_PointCluster*> itClusters(*clusterMap);
                itClusters.toBack();
                bool found = false;
                while (!found && itClusters.hasPrevious() && !isStopped())
                {
                    itClusters.previous();

                    CT_PointCluster* layerCluster = itClusters.value();
                    Eigen::Vector3d minLayerCluster, maxLayerCluster;
                    layerCluster->getBoundingBox(minLayerCluster, maxLayerCluster);

                    if (CT_MathBoundingShape::aabbIntersects2D(minSectionCluster, maxSectionCluster, minLayerCluster, maxLayerCluster))
                    {
                        found = true;
                        section->append(layerCluster);
                        itClusters.remove();
                    }
                }

                // Re-création de la map triée des sections
                sectionMap.insertMulti(clustersArea.value(section->last()), section);
            } else {
                finishedSections.append(section);
            }
        }

        // Création d'une nouvelle section pour chaque cluster non rattaché à une section existante
        QMutableMapIterator<float, CT_PointCluster*> itClusters(*clusterMap);
        while (itClusters.hasNext())
        {
            itClusters.next();
            QList<CT_PointCluster*>* newSection = new QList<CT_PointCluster*>();
            newSection->append(itClusters.value());
            sectionMap.insertMulti(itClusters.key(), newSection);
            itClusters.remove();
        }
    }

    setProgress( 50 );


    finishedSections.append(sectionMap.values());
    sectionMap.clear();


    // Enregistrement des sections dans le résultat
    QListIterator<QList<CT_PointCluster*>* > itSections(finishedSections);
    while (itSections.hasNext())
    {        
        QList<CT_PointCluster*> *list = itSections.next();
        CT_StandardItemGroup* section = new CT_StandardItemGroup(_outsectiongroupname, outResult);
        outResult->addGroup(section);
        int sizelist = list->size();

        for (int j = 0 ; j < sizelist ; j++)
        {
            section->addGroup(clusterToGroups.value(list->at(j)));
        }
    }

    qDeleteAll(finishedSections);
    qDeleteAll(layersZMap.values());

    setProgress( 100 );
}
