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


#include "onf_stepmergeneighboursections04.h"
#include "step/onf_stepchangeclusterthickness02.h"
#include "step/onf_steprefpointfrombarycenter02.h"


#include "ct_result/ct_resultgroup.h"
#include "ct_result/model/inModel/ct_inresultmodelgroup.h"
#include "ct_result/model/outModel/ct_outresultmodelgroup.h"

#include "ct_itemdrawable/ct_referencepoint.h"
#include "ct_itemdrawable/ct_pointcluster.h"
#include "ct_itemdrawable/tools/ct_standardcontext.h"
#include "ct_math/ct_mathpoint.h"

#include "ct_view/ct_stepconfigurabledialog.h"

#include "ct_itemdrawable/tools/iterator/ct_groupiterator.h"

#include "qdebug.h"

#include <limits>

#define DEF_SearchInResult      "r"
#define DEF_SearchInSectionGroup  "sg"
#define DEF_SearchInClusterGroup  "cg"
#define DEF_SearchInPointCluster  "p"
#define DEF_SearchInRefPoint  "rp"

#define DEF_SearchOutResult      "ro"
#define DEF_SearchOutSectionGroup  "sgo"
#define DEF_SearchOutClusterGroup  "cgo"
#define DEF_SearchOutPointCluster  "po"
#define DEF_SearchOutRefPoint  "rpo"

ONF_StepMergeNeighbourSections04::ONF_StepMergeNeighbourSections04(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _thickness = 0.10;
    _searchDistance = 10;
    _maxDistance = 0.50;
    _deltaZ = 0.20;
    _maxInd1 = 1.5;
}

QString ONF_StepMergeNeighbourSections04::getStepDescription() const
{
    return tr("Fusion de billons parallèles");
}

QString ONF_StepMergeNeighbourSections04::getStepDetailledDescription() const
{
    return tr("Cette étape prend en entrée une liste de billons. Chaque billon est composée d'une séquence de clusters. "
              "<br>Un cluster est caractérisé par :"
              "<ul>"
              "<li>Une liste de points</li>"
              "<li>Un barycentre (le barycentre des points)</li>"
              "<li>Une valeur <em>buffer</em>, égale à la distance entre le barycentre et le point le plus éloigné du barycentre</li>"
              "</ul>"
              "<br>Ces billons sont issues d'une étape précédente telle que <em>ONF_StepDetectSection</em>. Cependant, en début d'étape "
              "elles sont remaniées de façon à ce que les clusters aient l' <b>épaisseur</b> choisie en paramètre de l'étape.<br>"
              "Au sein de chaque billon ce remaniement consiste à prendre tous les points de tous les clusters, afin de recréer des clusters de l' <b>épaisseur</b> choisie.<br>"
              "Ensuite, pour chaque cluster créé, on en détermine le barycentre et le buffer.<br>"
              "<b>Le but de cette étape est de fusionner des billons appartenant dans la réalité au même arbre</b>.<br>"
              "Elle traite spécifiquement le cas des billons se chevauchant verticalement. Elle est complétée par <em>ONF_StepMergeEndToEndSections</em>.<br>"
              "En plus de l' <b>épaisseur de cluster</b>, cette étape utilise les paramètres suivants :"
              "<ul>"
              "<li>Une <b>distance de recherche de voisinnage</b> (paramètre d'optimisation des calculs)</li>"
              "<li>Une distance <b>deltaZ</b> : écart vertical maximal entre deux barycentres comparés</li>"
              "<li>Un critère <b>distMax</b> : distance XY maximum entre deux barycentres de billons à fusionner</li>"
              "<li>Un critère <b>ratioMax</b> : accroissement maximal du buffer accepté en cas de fusion</li>"
              "</ul>"
              "<br>Le fonctionnement de l'étape est le suivant. Les billons sont comparées deux à deux par ordre décroissant de longueur selon Z."
              "A chaque itération, on compare une billon A (la plus longue) constituée de n clusters ayant des barycentres Ai (i = 1 à n), "
              "avec une billon B constituée de m clusters ayant des barycentres Bj (j = 1 à m).<br>"
              "Pour ce faire on commence par calculer <b>medBuffer</b> : la médiane des distances buffers des barycentres Ai.<br>"
              "Pour que A et B soient fusionnées, il faut que pour tout i et j tels que la distance verticale |Ai - Bj|z < <b>deltaZ</b>"
              "<ul>"
              "<li>Qu'aucune distance horizontale |Ai - Bj|xy ne soit supérieure à <b>distMax</b></li>"
              "<li>Qu'aucune distance horizontale |Ai - Bj|xy ne soit supérieure à <b>medDist</b></li>"
              "<li>Qu'au moins pour un couple Ai / Bj, le ratio |Ai - Bj| / MAX(buffer Ai, buffer Bj) soit inférieur à <b>ratioMax</b>"
              "</ul>"
              "En cas de fusion, les clusters et les barycentres sont recréés à partir de tous les points des deux billons sources pour former une nouvelle billon C.<br>"
              "La billon C devient la de facto la plus longue : elle est donc aussitôt utilisée dans l'itération suivant dans la comparaison avec la prochaine billon (plus petite) de la liste.");
}

CT_VirtualAbstractStep* ONF_StepMergeNeighbourSections04::createNewInstance(CT_StepInitializeData &dataInit)
{
    // cree une copie de cette etape
    return new ONF_StepMergeNeighbourSections04(dataInit);
}

//////////////////// PROTECTED //////////////////

void ONF_StepMergeNeighbourSections04::createInResultModelListProtected()
{
    CT_InResultModelGroup * resultModel = createNewInResultModel(DEF_SearchInResult, tr("Billons"));
    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_SearchInSectionGroup, CT_AbstractItemGroup::staticGetType(), tr("Billon (Grp)"));
    resultModel->addGroupModel(DEF_SearchInSectionGroup, DEF_SearchInClusterGroup, CT_AbstractItemGroup::staticGetType(), tr("Cluster (Grp)"));
    resultModel->addItemModel(DEF_SearchInClusterGroup, DEF_SearchInPointCluster, CT_PointCluster::staticGetType(), tr("Points"));
}

void ONF_StepMergeNeighbourSections04::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addDouble(tr("Epaisseur (en Z) des clusters  :"), "cm", 0, 1000, 2, _thickness, 100);
    configDialog->addDouble(tr("Distance de recherche de voisinage :"), "m", 0, 100, 2, _searchDistance);
    configDialog->addDouble(tr("Distance XY maximum entre barycentres de clusters de billons à fusionner :"), "cm", 0, 1000, 2, _maxDistance, 100);
    configDialog->addDouble(tr("Distance Z maximum entre barycentres de clusters de billons à fusionner :"), "cm", 0, 1000, 2, _deltaZ, 100);
    configDialog->addDouble(tr("Facteur d'accroissement maximal des distances XY entre barycentres de clusters de billons à fusionner' :"), tr("fois"), 0, 1000, 2, _maxInd1);
}

void ONF_StepMergeNeighbourSections04::createOutResultModelListProtected()
{
    CT_OutResultModelGroup * resultModel = createNewOutResultModel(DEF_SearchOutResult, tr("Billons Fusionnées"));
    resultModel->setRootGroup(DEF_SearchOutSectionGroup, new CT_StandardItemGroup(), tr("Billon (Grp)"));
    resultModel->addGroupModel(DEF_SearchOutSectionGroup, DEF_SearchOutClusterGroup, new CT_StandardItemGroup(), tr("Cluster (Grp)"));
    resultModel->addItemModel(DEF_SearchOutClusterGroup, DEF_SearchOutPointCluster, new CT_PointCluster(), tr("Points"));
    resultModel->addItemModel(DEF_SearchOutClusterGroup, DEF_SearchOutRefPoint, new CT_ReferencePoint(), tr("Barycentre"));
}

void ONF_StepMergeNeighbourSections04::compute()
{       
    CT_ResultGroup *inResult = getInputResults().first();
    CT_ResultGroup *outResult = getOutResultList().first();

    ////////////////////////////////////////////////////
    // CREATION DES SECTION INITIALES (CHANGETHICKNESS + AJOUT DES POINTS DE REFERENCE BARYCENTRES)
    ////////////////////////////////////////////////////

    QMap<CT_StandardItemGroup*, QList<CT_ReferencePoint*>* > sectionMap;
    QMap<double, CT_StandardItemGroup* > orderedSectionMap;

    // Parcours des sections in
    CT_ResultGroupIterator itSection(inResult, this, DEF_SearchInSectionGroup);
    while (itSection.hasNext() && !isStopped())
    {
        CT_AbstractItemGroup* section = (CT_AbstractItemGroup*) itSection.next();

        QList<CT_PointCluster*> clustersList;
        CT_GroupIterator itGrp(section, this, DEF_SearchInClusterGroup);
        while (itGrp.hasNext())
        {
            CT_AbstractItemGroup* group = (CT_AbstractItemGroup*) itGrp.next();

            CT_PointCluster *cluster = (CT_PointCluster*) group->firstItemByINModelName(this, DEF_SearchInPointCluster);
            if (cluster!=NULL) {clustersList.append(cluster);}
        }

        QList<CT_ReferencePoint*> *refPoints = new QList<CT_ReferencePoint*>();
        CT_StandardItemGroup* newSection = sectionFromSegment(clustersList, outResult, refPoints);
        sectionMap.insert(newSection, refPoints);

        // ordre par longueur verticale
        double sectionLength = refPoints->last()->z() - refPoints->first()->z();
        if (sectionLength < 0) {sectionLength = -sectionLength;}
        orderedSectionMap.insertMulti(sectionLength, newSection);

    }

    QList<CT_StandardItemGroup*> sectionList = orderedSectionMap.values();
    int size = sectionList.size();

    ////////////////////////////////////////////////////
    // ALGORITHME DE FUSION CONDITIONNELLE DES SECTIONS
    ////////////////////////////////////////////////////
    double distance2 = _searchDistance*_searchDistance;
    while (!sectionList.isEmpty() && (!isStopped()))
    {
        // récupération de la section la plus longue : BASE
        CT_StandardItemGroup* baseSection = sectionList.takeLast();
        QList<CT_ReferencePoint*> *seg_base = sectionMap.take(baseSection);

        bool hasToLoopAgain = true;
        while (hasToLoopAgain)
        {
                hasToLoopAgain = false;

                float x_fbase = seg_base->first()->x();
                float y_fbase = seg_base->first()->y();
                float baseMinZ = seg_base->first()->z();
                float baseMaxZ = seg_base->last()->z();

                // Parcours des autres segments : TESTED
                QMapIterator<CT_StandardItemGroup*, QList<CT_ReferencePoint*>* > it(sectionMap);
                while (it.hasNext() && !hasToLoopAgain)
                {
                    it.next();
                    CT_StandardItemGroup* testedSection = it.key();
                    QList<CT_ReferencePoint*> *seg_tested = it.value();

                    // Est-ce que les segments BASE et TESTED se chevauchent en Z ?
                    float testedMinZ = seg_tested->first()->z();
                    float testedMaxZ = seg_tested->last()->z();

                    // Est-ce que les section se chevauchent en Z
                    if ((baseMinZ <= testedMaxZ) && (testedMinZ <= baseMaxZ))
                    {
                        float dx = x_fbase - seg_tested->first()->x();
                        float dy = y_fbase - seg_tested->first()->y();
                        float distance = dx*dx + dy*dy;

                        // Est-ce que les segments sont a proximite first a moins de _searchdistance en (x,y) ?
                        if (distance < distance2)
                        {
                            // Si la fusion de TESTED et BASE n'a pas ete rejetee
                            if (IsFusionNeeded(seg_base, seg_tested))
                            {
                                // Creation du segment fusionne : MERGED
                                QList<CT_PointCluster*> clusterList = mergeSkeletonSegments(baseSection, testedSection);
                                QList<CT_ReferencePoint*> *seg_merged = new QList<CT_ReferencePoint*>();
                                CT_StandardItemGroup* mergedSection = sectionFromSegment(clusterList, outResult, seg_merged);

                                // Le segment MERGED est retenu, donc remplace BASE pour les tests suivants
                                // BASE et TESTED sont donc supprimés
                                seg_base->clear();
                                delete seg_base;
                                delete baseSection;
                                seg_base = seg_merged;
                                baseSection = mergedSection;
                                seg_merged = NULL;

                                sectionMap.remove(testedSection);
                                sectionList.removeOne(testedSection);
                                seg_tested->clear();
                                delete seg_tested;
                                delete testedSection;

                                hasToLoopAgain = true;

                            }
                        }
                    }
                } // FIN du while des TESTED
        }
        outResult->addGroup(baseSection);
        seg_base->clear();
        delete seg_base;

        waitForAckIfInDebugMode();
        setProgress(100*(1 - (float)sectionList.size()/(float)size));
    }

    setProgress(100);
}


CT_StandardItemGroup* ONF_StepMergeNeighbourSections04::sectionFromSegment(QList<CT_PointCluster*> clustersList, CT_ResultGroup *outResult, QList<CT_ReferencePoint*> *refPoints)
{
    CT_StandardItemGroup* newSection = new CT_StandardItemGroup(DEF_SearchOutSectionGroup, outResult);
    CT_StandardContext context;

    QString clusterGroupModelName = DEF_SearchOutClusterGroup;
    QString pointClusterModelName = DEF_SearchOutPointCluster;

    context.add("cgm", &clusterGroupModelName);
    context.add("pcm", &pointClusterModelName);
    context.add("cl", &clustersList);
    context.add("or", outResult);
    context.add("th", &_thickness);
    newSection->setContext(&context);

    // Création de clusters d'une nouvelle épaisseur (utilisation de l'étape ONF_StepChangeClusterThickness02)
    ONF_StepChangeClusterThickness02::computeOneSection(newSection);
    context.clear();
    newSection->setContext(NULL);

    // Création des barycentres (utilisation de l'étape ONF_StepRefPointFromBarycenter02)
    CT_GroupIterator itSection(newSection, this, DEF_SearchOutClusterGroup);
    while (itSection.hasNext() && !isStopped())
    {
        CT_AbstractItemGroup* newGroup = (CT_AbstractItemGroup*) itSection.next();

        const CT_PointCluster *newCluster = (const CT_PointCluster*) newGroup->firstItemByINModelName(this, DEF_SearchOutPointCluster);
        if (newCluster!=NULL)
        {
            CT_ReferencePoint* refPoint = ONF_StepRefPointFromBarycenter02::addBarycenter(newCluster, newGroup, DEF_SearchOutRefPoint, outResult);
            refPoints->append(refPoint);
        }
    }

    return newSection;
}


QList<CT_PointCluster*> ONF_StepMergeNeighbourSections04::mergeSkeletonSegments(CT_StandardItemGroup* baseSection, CT_StandardItemGroup* testedSection)
{
    QList<CT_PointCluster*> seg_merged;

    CT_GroupIterator itBase(baseSection, this, DEF_SearchOutClusterGroup);
    while (itBase.hasNext())
    {
        CT_AbstractItemGroup *clusterGroup = (CT_AbstractItemGroup*) itBase.next();
        CT_PointCluster *cluster = (CT_PointCluster*) clusterGroup->firstItemByINModelName(this, DEF_SearchOutPointCluster);
        if (cluster!=NULL) {seg_merged.append(cluster);}
    }

    CT_GroupIterator itTested(testedSection, this, DEF_SearchOutClusterGroup);
    while (itTested.hasNext())
    {
        CT_AbstractItemGroup *clusterGroup = (CT_AbstractItemGroup*) itTested.next();
        CT_PointCluster *cluster = (CT_PointCluster*) clusterGroup->firstItemByINModelName(this, DEF_SearchOutPointCluster);
        if (cluster!=NULL) {seg_merged.append(cluster);}
    }

    return seg_merged;
}

bool ONF_StepMergeNeighbourSections04::IsFusionNeeded (QList<CT_ReferencePoint*> *seg_base, QList<CT_ReferencePoint*> *seg_tested)
{
    int sizeTested = seg_tested->size();
    int sizeBase = seg_base->size();
    bool fusionNeeded = false;
    float zmin_tested = seg_tested->first()->z() - _deltaZ;
    float zmax_tested = seg_tested->last()->z() + _deltaZ;
    bool ok = true;

    QList<float> buffers;
    // Parcours des points de BASE
    for (int i = 0 ; (i < sizeBase) && (ok) ; i++)
    {
        CT_ReferencePoint *baseRefPoint = seg_base->at(i);    // Parcours des points de BASE
        buffers.append(baseRefPoint->xyBuffer());
    }

    float distMax = 0;
    if (buffers.size() > 0)
    {
        qSort(buffers);

        // Modification AP, 06/03/2015
        //distMax = buffers.at(buffers.size() / 2);
        distMax = _maxInd1*buffers.at(buffers.size() / 2);

    }

    // Parcours des points de BASE
    for (int i = 0 ; (i < sizeBase) && (ok) ; i++)
    {
        CT_ReferencePoint *baseRefPoint = seg_base->at(i);    // Parcours des points de BASE

        if (baseRefPoint->z() > zmax_tested)
        {
            ok = false;
        } else {
            if (baseRefPoint->z() > zmin_tested)
            {
                for (int j = 0 ; j < sizeTested ; j++)
                {
                    CT_ReferencePoint *testedRefPoint = seg_tested->at(j);

                    float deltaZ = fabs(baseRefPoint->z() - testedRefPoint->z());

                    // On ne teste que des refpoint de meme Z à deltaZ près
                    if (deltaZ <= _deltaZ)
                    {
                        // Proximite si la distance separant les barycentre < _distance
                        float distance = CT_MathPoint::distance2D((*baseRefPoint).getCenterCoordinate(), (*testedRefPoint).getCenterCoordinate());
                        //float dist_Ratio = distance / std::max(baseRefPoint->xyBuffer(), testedRefPoint->xyBuffer());

                        if (distance > _maxDistance) {return false;}
                        if (distance > distMax) {return false;}

                        //if (dist_Ratio <= _maxInd1) {fusionNeeded = true;}
                        fusionNeeded = true;
                    }
                }
            }
        }

    } // FIN du test des distances 2 a 2 des points ( meme Z) de BASE et TESTED

    return fusionNeeded;
}

