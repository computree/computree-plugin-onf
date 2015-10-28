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


#include "onf_stepdetectsection06.h"

#include "ct_itemdrawable/model/outModel/ct_outstdgroupmodel.h"

#include "ct_result/model/inModel/ct_inresultmodelgroup.h"
#include "ct_result/model/outModel/ct_outresultmodelgroup.h"

#include "ct_result/ct_resultgroup.h"
#include "ct_itemdrawable/ct_pointcluster.h"
#include "ct_itemdrawable/ct_circle.h"
#include "ct_turn/inTurn/tools/ct_inturnmanager.h"
#include "ct_tools/model/ct_generateoutmodelname.h"

#include "ct_math/ct_mathboundingshape.h"

#include "ct_view/ct_stepconfigurabledialog.h"

#include "ct_model/inModel/tools/ct_instdmodelpossibility.h"

#include "ct_model/tools/ct_modelsearchhelper.h"

#include <cmath>
#include <iostream>
#include <QList>

#include "qdebug.h"


#define DEF_SearchInResult  "r"
#define DEF_SearchInGroup   "g"
#define DEF_SearchInCluster "cl"

#define DEF_SearchOutResult "r"

ONF_StepDetectSection06::ONF_StepDetectSection06(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _deltaz = 0.2;
}

QString ONF_StepDetectSection06::getStepDescription() const
{
    return tr("Aggrégation verticale de clusters en billon (Ancienne Version)");
}

QString ONF_StepDetectSection06::getStepDetailledDescription() const
{
    return tr("Cette étape prend en entrée des couches horizontales (layers) contenant des clusters.<br>"
              "Ce type de structure peut par exemple être produite par l'étape <em>ONF_StepHorizontalClustering</em>.<br>"
              "Les clusters adjacents verticalement sont regroupés en billons (groupes). Pour ce faire :"
              "<ul>"
              "<li> Les clusters dont la <b>distance verticale</b> les séparant est inférieure au seuil choisi sont comparés deux à deux.</li>"
              "<li>Si leurs boites englobantes s'intersectent dans le plan XY, les clusters sont regroupés dans la même billon.</li>"
              "</ul>");
}

CT_VirtualAbstractStep* ONF_StepDetectSection06::createNewInstance(CT_StepInitializeData &dataInit)
{
    // cree une copie de cette etape
    return new ONF_StepDetectSection06(dataInit);
}

//////////////////// PROTECTED //////////////////

void ONF_StepDetectSection06::createInResultModelListProtected()
{
    CT_InResultModelGroup * resultModel = createNewInResultModel(DEF_SearchInResult, tr("Clusters"));
    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_SearchInGroup, CT_AbstractItemGroup::staticGetType(), tr("Groupe"));
    resultModel->addItemModel(DEF_SearchInGroup, DEF_SearchInCluster, CT_PointCluster::staticGetType(), tr("Points"));

    resultModel->setMaximumNumberOfPossibilityThatCanBeSelectedForOneTurn(1);
}

void ONF_StepDetectSection06::createPostConfigurationDialog()
{
    // création effective de la configurationDialog
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();
    configDialog->addDouble(tr("Distance en z (en + et en -) maximum entre deux groupes de points à comparer"), "cm", 0, 1000, 2, _deltaz, 100);
}

void ONF_StepDetectSection06::preProcessCreateOutResultModelListProtected()
{
    CT_GenerateOutModelName generator;
    QList<QString> generatedNames;
    CT_InTurnManager *tm = getInTurnManager();

    _outclustergroupname = generator.getNewModelNameThatDontExistIn(*tm, generatedNames, "gcl");
    generatedNames.append(_outclustergroupname);
    _outsectiongroupname = generator.getNewModelNameThatDontExistIn(*tm, generatedNames, "gse");
}

void ONF_StepDetectSection06::createOutResultModelListProtected()
{
    // récupération du modèle in du résultat
    CT_InAbstractResultModel *resultmodel = getInResultModel(DEF_SearchInResult);

    if(!isCreateDefaultOutModelActive()
            && resultmodel->getPossibilitiesSavedSelected().isEmpty())
        qFatal("Error while create out model in ONF_StepDetectSection06");

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

void ONF_StepDetectSection06::compute()
{
    CT_ResultGroup *inResult = getInputResults().first();
    CT_ResultGroup *outResult = getOutResultList().first();

    const CT_OutAbstractGroupModel* outClusterGroupModel = (CT_OutAbstractGroupModel*)PS_MODELS->searchModelForCreation(_outclustergroupname, outResult);

    CT_ResultGroupIterator itGrp(inResult, this, DEF_SearchInGroup);

    // Création d'une liste triée par Z croissant des groupes contenant des cluster IN
    QMap<float, CT_PointCluster*> clusterZMap;
    // Correspondance entre clusters et groupes copiés
    QMap<CT_PointCluster*, CT_AbstractItemGroup*> clusterToGroups;

    while (itGrp.hasNext() && (!isStopped()))
    {
        CT_AbstractItemGroup* groupWithCluster = (CT_AbstractItemGroup*) itGrp.next();
        CT_PointCluster *cluster = (CT_PointCluster*)groupWithCluster->firstItemByINModelName(this, DEF_SearchInCluster);
        if(cluster != NULL)
        {
            CT_AbstractItemGroup *outClusterGroup = (CT_AbstractItemGroup*) groupWithCluster->copy(outClusterGroupModel, outResult, CT_ResultCopyModeList() << CT_ResultCopyModeList::CopyItemDrawableReference);

            clusterZMap.insertMulti(cluster->getCenterZ(), cluster);
            clusterToGroups.insert(cluster, outClusterGroup);
        }
    }
    setProgress( 25 );

    int nbSection = 0;
    QList<QList<CT_PointCluster*> *> section_list;
    QMapIterator<float, CT_PointCluster*> it(clusterZMap);
    while (it.hasNext() && (!isStopped()))
    {
        it.next();
        CT_PointCluster *cluster = it.value();
        Eigen::Vector3d minCluster, maxCluster;
        cluster->getBoundingBox(minCluster, maxCluster);

        int t = nbSection-1;
        bool add = false;
        bool continue_list = true;
        QList<CT_PointCluster*> *sect;

        while((t>=0)
              && (!add)
              && (continue_list)
              && (!isStopped()))
        {
            sect = section_list.at(t);
            CT_PointCluster *last_cluster = sect->last();

            float distance_z = fabs(cluster->getCenterZ() - last_cluster->getCenterZ());

            if(distance_z >= _deltaz)
            {
                continue_list = false;
            }
            else
            {
                Eigen::Vector3d minLastcluster, maxLastcluster;
                last_cluster->getBoundingBox(minLastcluster, maxLastcluster);

                if (CT_MathBoundingShape::aabbIntersects2D(minCluster, maxCluster, minLastcluster, maxLastcluster))
                {
                    add = true;
                    sect->append(cluster);
                }
            }

            --t;
        }

        if(!add)
        {
            QList<CT_PointCluster*> *list = new QList<CT_PointCluster*>();
            list->append(cluster);
            section_list.append(list);
            ++nbSection;
        }
        else if(nbSection > 1)
        {
            ++t;

            int from = t;
            float sect_z = sect->last()->getCenterZ();
            bool sort = false;

            ++t;

            while((t < nbSection)
                  && (!sort))
            {
                QList<CT_PointCluster*> *sect2 = section_list.at(t);
                if(sect2->last()->getCenterZ() > sect_z)
                {
                    section_list.move(from, t-1);
                    sort = true;
                }

                ++t;
            }

            if(!sort)
            {
                section_list.move(from, nbSection-1);
            }
        }
    }

    // Enregistrement des sections dans le résultat
    int size = section_list.size();
    for (int i = 0 ; i < size ; i++)
    {
        QList<CT_PointCluster*> *list = section_list.at(i);
        CT_StandardItemGroup* section = new CT_StandardItemGroup(_outsectiongroupname, outResult);
        outResult->addGroup(section);
        int sizelist = list->size();

        for (int j = 0 ; j < sizelist ; j++)
        {
            section->addGroup(clusterToGroups.value(list->at(j)));
        }
    }
    qDeleteAll(section_list);

    setProgress( 100 );
}
