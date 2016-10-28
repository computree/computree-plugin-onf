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


#include "onf_stepmatchclouds.h"

#include "ct_global/ct_context.h"

#include "ct_view/ct_stepconfigurabledialog.h"
#include "ct_result/model/inModel/ct_inresultmodelgroup.h"
#include "ct_result/model/outModel/ct_outresultmodelgroup.h"

#include "ct_result/ct_resultgroup.h"
#include "ct_itemdrawable/ct_scene.h"
#include "ct_itemdrawable/ct_attributeslist.h"

#include "ct_pointcloudindex/ct_pointcloudindexvector.h"
#include "ct_iterator/ct_pointiterator.h"


#include <math.h>
#include <iostream>
#include <QList>
#include <limits>

#define DEF_SearchInResultS "rinS"
#define DEF_SearchInGroupS   "ginS"
#define DEF_SearchInSceneS   "scinS"
#define DEF_SearchInItemS   "itinS"
#define DEF_SearchInAttS   "attinS"

#define DEF_SearchInResultT "rinT"
#define DEF_SearchInGroupT   "ginT"
#define DEF_SearchInSceneT   "scinT"
#define DEF_SearchInItemT   "itinT"
#define DEF_SearchInAttT   "attinS"

#define DEF_SearchOutResult "rout"
#define DEF_SearchOutGroup  "gout"
#define DEF_SearchOutAttList  "scout"
#define DEF_SearchOutAttS "outAttS"
#define DEF_SearchOutAttT "outAttT"
#define DEF_SearchOutAttCriteria "outAttCrit"


ONF_StepMatchClouds::ONF_StepMatchClouds(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
}

QString ONF_StepMatchClouds::getStepDescription() const
{
    return tr("Match points clouds");
}

QString ONF_StepMatchClouds::getStepDetailledDescription() const
{
    return tr("TO DO");
}

CT_VirtualAbstractStep* ONF_StepMatchClouds::createNewInstance(CT_StepInitializeData &dataInit)
{
    // cree une copie de cette etape
    return new ONF_StepMatchClouds(dataInit);
}

//////////////////// PROTECTED //////////////////

void ONF_StepMatchClouds::createInResultModelListProtected()
{
    CT_InResultModelGroup *resultModel = createNewInResultModel(DEF_SearchInResultS, tr("Source Scenes"), "", true);

    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_SearchInGroupS);
    resultModel->addItemModel(DEF_SearchInGroupS, DEF_SearchInSceneS, CT_AbstractItemDrawableWithPointCloud::staticGetType(), tr("Source Scene"));
    resultModel->addItemModel(DEF_SearchInGroupS, DEF_SearchInItemS, CT_AbstractSingularItemDrawable::staticGetType(), tr("Item (name)"));
    resultModel->addItemAttributeModel(DEF_SearchInItemS, DEF_SearchInAttS, QList<QString>() << CT_AbstractCategory::DATA_FILE_NAME << CT_AbstractCategory::DATA_VALUE, CT_AbstractCategory::ANY, tr("Name"));


    CT_InResultModelGroup *resultModel2 = createNewInResultModel(DEF_SearchInResultT, tr("Target Scenes"), "", true);

    resultModel2->setZeroOrMoreRootGroup();
    resultModel2->addGroupModel("", DEF_SearchInGroupT);
    resultModel2->addItemModel(DEF_SearchInGroupT, DEF_SearchInSceneT, CT_AbstractItemDrawableWithPointCloud::staticGetType(), tr("Target Scene"));
    resultModel2->addItemModel(DEF_SearchInGroupT, DEF_SearchInItemT, CT_AbstractSingularItemDrawable::staticGetType(), tr("Item (name)"));
    resultModel2->addItemAttributeModel(DEF_SearchInItemT, DEF_SearchInAttT, QList<QString>() << CT_AbstractCategory::DATA_FILE_NAME << CT_AbstractCategory::DATA_VALUE, CT_AbstractCategory::ANY, tr("Name"));
}

// Création et affiliation des modèles OUT
void ONF_StepMatchClouds::createOutResultModelListProtected()
{    
    CT_OutResultModelGroup *resultModel = createNewOutResultModel(DEF_SearchOutResult, tr("Matching"));

    resultModel->setRootGroup(DEF_SearchOutGroup);
    resultModel->addItemModel(DEF_SearchOutGroup, DEF_SearchOutAttList, new CT_AttributesList(), tr("Matching"));
    resultModel->addItemAttributeModel(DEF_SearchOutAttList, DEF_SearchOutAttS,
                                       new CT_StdItemAttributeT<QString>(NULL, PS_CATEGORY_MANAGER->findByUniqueName(CT_AbstractCategory::DATA_DISPLAYABLE_NAME), NULL, 0),
                                       tr("Source name"));
    resultModel->addItemAttributeModel(DEF_SearchOutAttList, DEF_SearchOutAttT,
                                       new CT_StdItemAttributeT<QString>(NULL, PS_CATEGORY_MANAGER->findByUniqueName(CT_AbstractCategory::DATA_DISPLAYABLE_NAME), NULL, 0),
                                       tr("Target name"));
    resultModel->addItemAttributeModel(DEF_SearchOutAttList, DEF_SearchOutAttCriteria,
                                       new CT_StdItemAttributeT<double>(NULL, PS_CATEGORY_MANAGER->findByUniqueName(CT_AbstractCategory::DATA_NUMBER), NULL, 0),
                                       tr("Distance"));


}

void ONF_StepMatchClouds::createPostConfigurationDialog()
{
    //CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();
}

void ONF_StepMatchClouds::compute()
{
    // récupération du résultats IN et OUT
    QList<CT_ResultGroup*> inResultList = getInputResults();
    CT_ResultGroup *inResultSource = inResultList.at(0);
    CT_ResultGroup *inResultTarget = inResultList.at(1);

    CT_ResultGroup *outResult = getOutResultList().first();

    QList<CT_AbstractItemDrawableWithPointCloud*> sourceScenes;
    QList<CT_AbstractItemDrawableWithPointCloud*> targetScenes;

    QList<Eigen::Vector3d> sourceBarycenters;
    QList<Eigen::Vector3d> targetBarycenters;

    QList<QString> sourceNames;
    QList<QString> targetNames;

    CT_ResultGroupIterator itS(inResultSource, this, DEF_SearchInGroupS);
    while(!isStopped() && itS.hasNext())
    {
        const CT_AbstractItemGroup* group = itS.next();
        CT_AbstractItemDrawableWithPointCloud *scene = (CT_AbstractItemDrawableWithPointCloud*) group->firstItemByINModelName(this, DEF_SearchInSceneS);
        const CT_AbstractSingularItemDrawable *item = group->firstItemByINModelName(this, DEF_SearchInItemS);

        if (scene != NULL && item != NULL)
        {
            QString name = "";
            CT_AbstractItemAttribute* att = item->firstItemAttributeByINModelName(inResultSource, this, DEF_SearchInAttS);
            if (att != NULL)
            {
                name = att->toString(item, NULL);
            }

            Eigen::Vector3d bary(0,0,0);

            const CT_AbstractPointCloudIndex *pointCloudIndex = scene->getPointCloudIndex();
            CT_PointIterator itP(pointCloudIndex);
            while(itP.hasNext() && (!isStopped()))
            {
                const CT_Point &point = itP.next().currentPoint();
                bary(0) += point(0);
                bary(1) += point(1);
                bary(2) += point(2);
            }

            size_t n = pointCloudIndex->size();
            if (n > 0)
            {
                bary(0) /= n;
                bary(1) /= n;
                bary(2) /= n;
            }

            sourceBarycenters.append(bary);
            sourceScenes.append(scene);
            sourceNames.append(name);
        }
    }

    CT_ResultGroupIterator itT(inResultTarget, this, DEF_SearchInGroupT);
    while(!isStopped() && itT.hasNext())
    {
        const CT_AbstractItemGroup* group = itT.next();
        CT_AbstractItemDrawableWithPointCloud *scene = (CT_AbstractItemDrawableWithPointCloud*) group->firstItemByINModelName(this, DEF_SearchInSceneT);
        const CT_AbstractSingularItemDrawable *item = group->firstItemByINModelName(this, DEF_SearchInItemT);

        if (scene != NULL && item != NULL)
        {
            QString name = "";
            CT_AbstractItemAttribute* att = item->firstItemAttributeByINModelName(inResultTarget, this, DEF_SearchInAttT);
            if (att != NULL)
            {
                name = att->toString(item, NULL);
            }

            Eigen::Vector3d bary(0,0,0);

            const CT_AbstractPointCloudIndex *pointCloudIndex = scene->getPointCloudIndex();
            CT_PointIterator itP(pointCloudIndex);
            while(itP.hasNext() && (!isStopped()))
            {
                const CT_Point &point = itP.next().currentPoint();
                bary(0) += point(0);
                bary(1) += point(1);
                bary(2) += point(2);
            }

            size_t n = pointCloudIndex->size();
            if (n > 0)
            {
                bary(0) /= n;
                bary(1) /= n;
                bary(2) /= n;
            }

            targetBarycenters.append(bary);
            targetScenes.append(scene);
            targetNames.append(name);
        }
    }


    for (int i = 0 ; i < sourceScenes.size() ; i++)
    {
        CT_AbstractItemDrawableWithPointCloud* sourceScene = sourceScenes.at(i);
        Eigen::Vector3d barySource = sourceBarycenters.at(i);

        double lowestDist = std::numeric_limits<double>::max();
        int lowestJ = -1;

        for (int j = 0 ; j < targetScenes.size() ; j++)
        {
            CT_AbstractItemDrawableWithPointCloud* targetScene = targetScenes.at(j);
            Eigen::Vector3d baryTarget = targetBarycenters.at(j);

            double distance = sqrt(pow(barySource(0) - baryTarget(0), 2) + pow(barySource(1) - baryTarget(1), 2));

            if (distance < lowestDist)
            {
                lowestDist = distance;
                lowestJ = j;
            }
        }

        if (lowestJ >= 0)
        {
            CT_StandardItemGroup* outGrp = new CT_StandardItemGroup(DEF_SearchOutGroup, outResult);

            CT_AttributesList* attList = new CT_AttributesList(DEF_SearchOutAttList, outResult);
            attList->addItemAttribute(new CT_StdItemAttributeT<QString>(DEF_SearchOutAttS,
                                                                           CT_AbstractCategory::DATA_DISPLAYABLE_NAME,
                                                                           outResult, sourceNames.at(i)));
            attList->addItemAttribute(new CT_StdItemAttributeT<QString>(DEF_SearchOutAttT,
                                                                           CT_AbstractCategory::DATA_DISPLAYABLE_NAME,
                                                                           outResult, targetNames.at(lowestJ)));
            attList->addItemAttribute(new CT_StdItemAttributeT<double>(DEF_SearchOutAttCriteria,
                                                                           CT_AbstractCategory::DATA_NUMBER,
                                                                           outResult, lowestDist));

            outGrp->addItemDrawable(attList);
            outResult->addGroup(outGrp);
        }
    }

}
