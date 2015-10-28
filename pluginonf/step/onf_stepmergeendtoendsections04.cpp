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


#include "onf_stepmergeendtoendsections04.h"
#include "step/onf_stepchangeclusterthickness02.h"
#include "step/onf_steprefpointfrombarycenter02.h"


#include "ct_result/ct_resultgroup.h"

#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/ct_outresultmodelgroup.h"
#include "ct_result/model/outModel/ct_outresultmodelgroupcopy.h"

#include "ct_itemdrawable/ct_referencepoint.h"
#include "ct_itemdrawable/ct_pointcluster.h"
#include "ct_itemdrawable/tools/ct_standardcontext.h"
#include "ct_math/ct_mathpoint.h"
#include "ct_view/ct_stepconfigurabledialog.h"

#include "ct_itemdrawable/tools/iterator/ct_groupiterator.h"

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

ONF_StepMergeEndToEndSections04::ONF_StepMergeEndToEndSections04(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _thickness = 0.10;
    _searchDistance = 1;
    _n = 10;
    _mult = 2;
    _zTolerance = 0.20;
}

QString ONF_StepMergeEndToEndSections04::getStepDescription() const
{
    return tr("Fusion de billons alignés");
}

QString ONF_StepMergeEndToEndSections04::getStepDetailledDescription() const
{
    return tr("No detailled description for this step");
}

CT_VirtualAbstractStep* ONF_StepMergeEndToEndSections04::createNewInstance(CT_StepInitializeData &dataInit)
{
    // cree une copie de cette etape
    return new ONF_StepMergeEndToEndSections04(dataInit);
}

//////////////////// PROTECTED //////////////////

void ONF_StepMergeEndToEndSections04::createInResultModelListProtected()
{
    CT_InResultModelGroup * resultModel = createNewInResultModel(DEF_SearchInResult, tr("Billons"));
    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_SearchInSectionGroup, CT_AbstractItemGroup::staticGetType(), tr("Billon (Grp)"));
    resultModel->addGroupModel(DEF_SearchInSectionGroup, DEF_SearchInClusterGroup, CT_AbstractItemGroup::staticGetType(), tr("Cluster (Grp)"));
    resultModel->addItemModel(DEF_SearchInClusterGroup, DEF_SearchInPointCluster, CT_PointCluster::staticGetType(), tr("Points"));
}

void ONF_StepMergeEndToEndSections04::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addDouble(tr("Epaisseur des groupes en Z  :"), "cm", 0, 1000, 2, _thickness, 100);
    configDialog->addDouble(tr("Distance maximale entre extremités de billons à fusionner :"), "m", 0, 100, 2, _searchDistance);
    configDialog->addDouble(tr("Nombre de barycentres a considerer aux extremites :"), "", 0, 50, 0, _n);
    configDialog->addDouble(tr("Facteur multiplicatif de maxDist :"), "", 0, 10000, 0, _mult);
    configDialog->addDouble(tr("Chevauchement toléré en Z :"), "cm", 0, 1000, 0, _zTolerance, 100);
}

void ONF_StepMergeEndToEndSections04::createOutResultModelListProtected()
{
    CT_OutResultModelGroup * resultModel = createNewOutResultModel(DEF_SearchOutResult, tr("Billons Fusionnées"));
    resultModel->setRootGroup(DEF_SearchOutSectionGroup, new CT_StandardItemGroup(), tr("Billon (Grp)"));
    resultModel->addGroupModel(DEF_SearchOutSectionGroup, DEF_SearchOutClusterGroup, new CT_StandardItemGroup(), tr("Cluster (Grp)"));
    resultModel->addItemModel(DEF_SearchOutClusterGroup, DEF_SearchOutPointCluster, new CT_PointCluster(), tr("Points"));
    resultModel->addItemModel(DEF_SearchOutClusterGroup, DEF_SearchOutRefPoint, new CT_ReferencePoint(), tr("Barycentre"));
}

void ONF_StepMergeEndToEndSections04::compute()
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
        double sectionZ = refPoints->first()->z();
        orderedSectionMap.insertMulti(sectionZ, newSection);
    }

    QList<CT_StandardItemGroup*> sectionList = orderedSectionMap.values();
    int size = sectionList.size();

    ////////////////////////////////////////////////////
    // ALGORITHME DE FUSION CONDITIONNELLE DES SECTIONS
    ////////////////////////////////////////////////////
    while (!sectionList.isEmpty() && (!isStopped()))
    {
        // récupération de la section la plus longue : BASE
        CT_StandardItemGroup* baseSection = sectionList.takeFirst();
        QList<CT_ReferencePoint*> *seg_base = sectionMap.take(baseSection);

        bool hasToLoopAgain = true;
        while (hasToLoopAgain)
        {
                hasToLoopAgain = false;

                // Creation de la QList des _n barycentres du haut de BASE
                float maxDist_base = 0;
                QList <CT_AbstractSingularItemDrawable*> n_bary_base;
                QMap<float, CT_ReferencePoint*> refPoints_base;

                int size_seg_base = seg_base->size();
                for (int bs = 0 ; bs < size_seg_base ; bs++)
                {
                    CT_ReferencePoint *refPoint = seg_base->at(bs);
                    refPoints_base.insertMulti(refPoint->z(), refPoint);
                }

                float x_base = -std::numeric_limits<float>::max();
                float y_base = -std::numeric_limits<float>::max();
                float z_base = -std::numeric_limits<float>::max();
                int i_base = 0;
                int size_base = std::min ((float) _n, (float) refPoints_base.size());
                QMapIterator<float, CT_ReferencePoint*> it_base(refPoints_base);
                it_base.toBack();
                while (it_base.hasPrevious() && (i_base < size_base))
                {
                    it_base.previous();
                    CT_ReferencePoint* refPt_base = it_base.value();
                    n_bary_base.append(refPt_base);
                    if (maxDist_base < refPt_base->xyBuffer()) {maxDist_base = refPt_base->xyBuffer();}
                    if (i_base == 0)
                    {
                        x_base = refPt_base->x();
                        y_base = refPt_base->y();
                        z_base = refPt_base->z();
                    }
                    ++i_base;
                }

                // Ajustement de la droite passant par les _n barycentres du haut de BASE
                CT_LineData *lineb = CT_LineData::staticCreateLineDataFromItemCenters(n_bary_base);


                // Parcours des autres segments : TESTED (toujours dans l'ordre des Zmin)
                QListIterator<CT_StandardItemGroup*> it(sectionList);
                while (it.hasNext() && !hasToLoopAgain)
                {
                    CT_StandardItemGroup* testedSection = it.next();
                    QList<CT_ReferencePoint*> *seg_tested = sectionMap.value(testedSection);

                    // Est-ce que les segments BASE et TESTED se chevauchent en Z ?
                    float testedMinZ = seg_tested->first()->z();
                    float baseMaxZ = seg_base->last()->z();

                    double extremitiesDistance = CT_MathPoint::distance3D((*seg_tested->first()).getCenterCoordinate(), (*seg_base->last()).getCenterCoordinate());

                    // Est-ce que TESTED commence au dessus de BASE
                    if ((extremitiesDistance < _searchDistance) && (testedMinZ >= (baseMaxZ - _zTolerance)))
                    {
                        // a priori le TESTED doit etre fusionne avec le BASE sauf si...
                        bool hasToBeMerged = true;

                        // Creation de la QList des _n barycentres du bas de TESTED
                        float maxDist_tested = 0;
                        QList <CT_AbstractSingularItemDrawable*> n_bary_tested;
                        QMap<float, CT_ReferencePoint*> refPoints_tested;

                        int size_seg_tested = seg_tested->size();
                        for (int ts = 0 ; ts < size_seg_tested ; ts++)
                        {
                            CT_ReferencePoint *refPoint = seg_tested->at(ts);
                            refPoints_tested.insertMulti(refPoint->z(), refPoint);
                        }

                        float x_tested = -std::numeric_limits<float>::max();
                        float y_tested = -std::numeric_limits<float>::max();
                        float z_tested = -std::numeric_limits<float>::max();
                        int i_tested = 0;
                        int size_tested = std::min ((float) _n, (float) refPoints_tested.size());
                        QMapIterator<float, CT_ReferencePoint*> it_tested(refPoints_tested);
                        while (it_tested.hasNext() && (i_tested < size_tested))
                        {
                            it_tested.next();
                            CT_ReferencePoint* refPt_tested = it_tested.value();
                            n_bary_tested.append(refPt_tested);
                            if (maxDist_tested < refPt_tested->xyBuffer()) {maxDist_tested = refPt_tested->xyBuffer();}
                            if (i_tested == 0)
                            {
                                x_tested = refPt_tested->x();
                                y_tested = refPt_tested->y();
                                z_tested = refPt_tested->z();
                            }
                            ++i_tested;
                        }

                        // Ajustement de la droite passant par les _n barycentres du bas de TESTED
                        CT_LineData *linet = CT_LineData::staticCreateLineDataFromItemCenters(n_bary_tested);

                        float dist_base   = distanceFromExtremityToLine(linet, x_base, y_base, z_base);
                        float dist_tested = distanceFromExtremityToLine(lineb, x_tested, y_tested, z_tested);

                        delete linet;

                        if (dist_base   > _mult*maxDist_base) {hasToBeMerged = false;}
                        if (size_tested>1 && dist_tested > _mult*maxDist_tested) {hasToBeMerged = false;}

                        // Si la fusion de TESTED et BASE n'a pas ete rejetee
                        if (hasToBeMerged)
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
                } // FIN du while des TESTED
                delete lineb;
        }
        outResult->addGroup(baseSection);
        seg_base->clear();
        delete seg_base;

        waitForAckIfInDebugMode();
        setProgress(100*(1 - (float)sectionList.size()/(float)size));
    }

    setProgress(100);
}


CT_StandardItemGroup* ONF_StepMergeEndToEndSections04::sectionFromSegment(QList<CT_PointCluster*> clustersList, CT_ResultGroup *outResult, QList<CT_ReferencePoint*> *refPoints)
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


QList<CT_PointCluster*> ONF_StepMergeEndToEndSections04::mergeSkeletonSegments(CT_StandardItemGroup* baseSection, CT_StandardItemGroup* testedSection)
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


float ONF_StepMergeEndToEndSections04::distanceFromExtremityToLine(CT_LineData *lineL, double plan_x, double plan_y, double plan_z)
{

    double x_inter = 0;
    double y_inter = 0;
    double z_inter = 0;

    Eigen::Vector3d vert(0,0,1);

    bool ok = lineL->intersectionWithRect3D(plan_x, plan_y, plan_z, vert, &x_inter, &y_inter, &z_inter);

    if (!ok) {
        return 10000;
    }

    float dx = x_inter - plan_x;
    float dy = y_inter - plan_y;

    return  sqrt(dx*dx + dy*dy);
}
