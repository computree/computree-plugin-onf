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


#include "onf_stepchangeclusterthickness02.h"

#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"

#include "ct_itemdrawable/tools/ct_standardcontext.h"

#include "ct_itemdrawable/ct_pointcluster.h"
#include "ct_itemdrawable/ct_circle.h"

#include "ct_result/ct_resultgroup.h"
#include "ct_iterator/ct_pointiterator.h"
#include "qfuture.h"

#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
#include <QtConcurrentMap>
#else
#include <QtConcurrent/QtConcurrentMap>
#endif

#include "ct_tools/model/ct_outmodelcopyactionaddmodelgroupingroup.h"
#include "ct_tools/model/ct_outmodelcopyactionaddmodelitemingroup.h"
#include "ct_tools/model/ct_outmodelcopyactionremovemodelgroupingroup.h"

#include "ct_view/ct_stepconfigurabledialog.h"
#include "ct_itemdrawable/tools/iterator/ct_groupiterator.h"

#include "qdebug.h"

#define DEF_SearchInResult      "r"
#define DEF_SearchInSectionGroup  "sg"
#define DEF_SearchInClusterGroup  "cg"
#define DEF_SearchInPointCluster  "p"

#define DEF_SearchOutResult     "r"

ONF_StepChangeClusterThickness02::ONF_StepChangeClusterThickness02(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _thickness = 0.1;
}

QString ONF_StepChangeClusterThickness02::getStepDescription() const
{
    return tr("Création de clusters horizontaux / billon");
}

QString ONF_StepChangeClusterThickness02::getStepDetailledDescription() const
{
    return tr("No detailled description for this step");
}

CT_VirtualAbstractStep* ONF_StepChangeClusterThickness02::createNewInstance(CT_StepInitializeData &dataInit)
{
    // cree une copie de cette etape
    return new ONF_StepChangeClusterThickness02(dataInit);
}

//////////////////// PROTECTED //////////////////

void ONF_StepChangeClusterThickness02::createInResultModelListProtected()
{
    CT_InResultModelGroupToCopy * resultModel = createNewInResultModelForCopy(DEF_SearchInResult, tr("Billons / Clusters"));
    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_SearchInSectionGroup, CT_AbstractItemGroup::staticGetType(), tr("Billon (Grp)"));
    resultModel->addGroupModel(DEF_SearchInSectionGroup, DEF_SearchInClusterGroup, CT_AbstractItemGroup::staticGetType(), tr("Cluster (Grp)"));
    resultModel->addItemModel(DEF_SearchInClusterGroup, DEF_SearchInPointCluster, CT_PointCluster::staticGetType(), tr("Points"));
}

void ONF_StepChangeClusterThickness02::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addDouble(tr("Epaisseur en Z  :"), "cm", 0, 1000, 2, _thickness, 100);

}

void ONF_StepChangeClusterThickness02::createOutResultModelListProtected()
{
    CT_OutResultModelGroupToCopyPossibilities* resultCopy = createNewOutResultModelToCopy(DEF_SearchInResult);
    resultCopy->addGroupModel(DEF_SearchInSectionGroup, _autoSearchOutClusterGroup, new CT_StandardItemGroup(), tr("Cluster (Grp)"));
    resultCopy->addItemModel(_autoSearchOutClusterGroup, _autoSearchOutPointCluster, new CT_PointCluster(), tr("Points"));
    resultCopy->removeGroupModel(DEF_SearchInClusterGroup);

    // ATTENTION il est important de faire l'ajout D'ABORD des nouveaux
    // modèles et seulement APRES de supprimer l'ancien clusterGroup afin que les nouveaux éléments n'est pas le même nom
    // qu'un de ceux qui était déjà présent dans le modèle
}

void ONF_StepChangeClusterThickness02::compute()
{
    // recupere le resultat d'entree
    QList<CT_ResultGroup*> inResultList = getInputResults();
    CT_ResultGroup *inResult = inResultList.first();

    // recupere le resultat de sortie
    QList<CT_ResultGroup*> outResultList = getOutResultList();
    CT_ResultGroup *outResult = outResultList.first();

    QString pointClusterModelName = _autoSearchOutPointCluster.completeName();
    QString clusterGroupModelName = _autoSearchOutClusterGroup.completeName();

    QList<CT_StandardItemGroup*> sections;

    CT_ResultGroupIterator itIn(inResult, this, DEF_SearchInSectionGroup);
    CT_ResultGroupIterator itOut(outResult, this, DEF_SearchInSectionGroup);

        // pour chaque groupe Section
        while(itIn.hasNext() && itOut.hasNext() && (!isStopped()))
        {
            CT_AbstractItemGroup *inSectionGroup = (CT_AbstractItemGroup*) itIn.next();
            CT_AbstractItemGroup *outSectionGroup = (CT_AbstractItemGroup*) itOut.next();

            QList<CT_PointCluster*> *liste = new QList<CT_PointCluster*>();
            liste->append(getClusters(inSectionGroup));

            // création d'un contexte standard
            CT_StandardContext *context = new CT_StandardContext();
            context->add("cl", liste);
            context->add("or", outResult);
            context->add("th", &_thickness);
            context->add("pcm", &pointClusterModelName);
            context->add("cgm", &clusterGroupModelName);

            outSectionGroup->setContext(context);

            // remplissage de la nouvelle section
            sections.append((CT_StandardItemGroup*)outSectionGroup);
        }

        QFuture<void> futur = QtConcurrent::map(sections, ONF_StepChangeClusterThickness02::computeOneSection);
        int progressMin = futur.progressMinimum();
        int progressTotal = futur.progressMaximum() - futur.progressMinimum();
        while (!futur.isFinished())
        {
            setProgress(99*(futur.progressValue() - progressMin)/progressTotal);
        }


        while (!sections.isEmpty())
        {
            CT_StandardItemGroup* group = sections.takeLast();
            // suppression du contexte
            CT_StandardContext *context = (CT_StandardContext*) group->getContext();
            QList<CT_PointCluster*> *liste = (QList<CT_PointCluster*>*) context->get("cl").first();
            liste->clear();
            delete liste;
            delete context;
            group->setContext(NULL);
        }

    setProgress(100);
}

QList<CT_PointCluster*> ONF_StepChangeClusterThickness02::getClusters(CT_AbstractItemGroup *oldSection)
{
    QList<CT_PointCluster*> liste;

    CT_GroupIterator it(oldSection, this, DEF_SearchInClusterGroup);
    while (it.hasNext())
    {
        const CT_AbstractItemGroup *group = it.next();
        CT_PointCluster *item = (CT_PointCluster*)group->firstItemByINModelName(this,DEF_SearchInPointCluster);
        if (item != NULL)
        {
            liste.append(item);
        }
    }
    return liste;
}


void ONF_StepChangeClusterThickness02::computeOneSection(CT_StandardItemGroup *newSection)
{
    CT_StandardContext* context = (CT_StandardContext*) newSection->getContext();

    QList<CT_PointCluster*> *liste = (QList<CT_PointCluster*>*) context->get("cl").first();
    CT_ResultGroup *outResult = (CT_ResultGroup*) context->get("or").first();
    double thickness = *(double*)context->get("th").first();
    QString pointClusterModelName = *(QString*)context->get("pcm").first();
    QString clusterGroupModelName = *(QString*)context->get("cgm").first();

    QMultiMap<double, size_t> indexesSortedByZ;

    int size = liste->size();
    for (int g = 0 ; g < size ; g++)
    {
        const CT_PointCluster *item = (const CT_PointCluster*)liste->at(g);

        CT_PointIterator itP(item->getPointCloudIndex());
        while (itP.hasNext())
        {
            size_t index = itP.next().currentGlobalIndex();
            double z = itP.currentPoint()(2);
            indexesSortedByZ.insert(z, index);
        }
    }

    QMapIterator<double, size_t> itPoints(indexesSortedByZ);
    if (itPoints.hasNext())
    {
        double maxzPoint = itPoints.next().key();
        maxzPoint += thickness;

        CT_PointCluster* activecluster = NULL;

        itPoints.toFront();
        while (itPoints.hasNext())
        {
            itPoints.next();
            double zPoint = itPoints.key();
            size_t indexPoint = itPoints.value();

            while ((maxzPoint - zPoint) < 0)
            {
                maxzPoint += thickness;
                activecluster = NULL;
            }

            if (activecluster == NULL)
            {
                activecluster = new CT_PointCluster(pointClusterModelName, outResult);
                CT_StandardItemGroup* clustergroup = new CT_StandardItemGroup(clusterGroupModelName, outResult);
                clustergroup->addItemDrawable(activecluster);
                newSection->addGroup(clustergroup);
            }

            activecluster->addPoint(indexPoint, false);
        }

    }

}
