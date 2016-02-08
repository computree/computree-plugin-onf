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

#include "onf_stepextractlogbuffer.h"

#include "ct_itemdrawable/abstract/ct_abstractitemdrawablewithpointcloud.h"
#include "ct_itemdrawable/ct_circle.h"
#include "ct_itemdrawable/ct_pointcluster.h"
#include "ct_pointcloudindex/ct_pointcloudindexvector.h"
#include "ct_itemdrawable/tools/iterator/ct_groupiterator.h"
#include "ct_result/ct_resultgroup.h"
#include "ct_result/model/inModel/ct_inresultmodelgroup.h"
#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"
#include "ct_view/ct_stepconfigurabledialog.h"

#include "ct_iterator/ct_pointiterator.h"

#include "ct_math/ct_mathpoint.h"

// Alias for indexing models
#define DEFin_rpts "rpts"
#define DEFin_grpPts "grpPts"
#define DEFin_pts "pts"

#define DEFin_rcircles "rcircles"
#define DEFin_grpLog "grpLog"
#define DEFin_grpCircles "grpCircles"
#define DEFin_circle "circle"



// Constructor : initialization of parameters
ONF_StepExtractLogBuffer::ONF_StepExtractLogBuffer(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _circleIncrement = 0.02;
    _buffer = 0.1;
}

// Step description (tooltip of contextual menu)
QString ONF_StepExtractLogBuffer::getStepDescription() const
{
    return tr("Extraire les points autour d'un Billon");
}

// Step detailled description
QString ONF_StepExtractLogBuffer::getStepDetailledDescription() const
{
    return tr("No detailled description for this step");
}

// Step URL
QString ONF_StepExtractLogBuffer::getStepURL() const
{
    //return tr("STEP URL HERE");
    return CT_AbstractStep::getStepURL(); //by default URL of the plugin
}

// Step copy method
CT_VirtualAbstractStep* ONF_StepExtractLogBuffer::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new ONF_StepExtractLogBuffer(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void ONF_StepExtractLogBuffer::createInResultModelListProtected()
{
    CT_InResultModelGroup *resIn_rpts = createNewInResultModel(DEFin_rpts, tr("Points"), tr(""), true);
    resIn_rpts->setZeroOrMoreRootGroup();
    resIn_rpts->addGroupModel("", DEFin_grpPts, CT_AbstractItemGroup::staticGetType(), tr("Groupe"));
    resIn_rpts->addItemModel(DEFin_grpPts, DEFin_pts, CT_AbstractItemDrawableWithPointCloud::staticGetType(), tr("Points"));

    CT_InResultModelGroupToCopy *resIn_rcircles = createNewInResultModelForCopy(DEFin_rcircles, tr("Cerles"));
    resIn_rcircles->setZeroOrMoreRootGroup();
    resIn_rcircles->addGroupModel("", DEFin_grpLog, CT_AbstractItemGroup::staticGetType(), tr("Billon"));
    resIn_rcircles->addGroupModel(DEFin_grpLog, DEFin_grpCircles, CT_AbstractItemGroup::staticGetType(), tr("Groupe"));
    resIn_rcircles->addItemModel(DEFin_grpCircles, DEFin_circle, CT_Circle::staticGetType(), tr("Cercle"));

}

// Creation and affiliation of OUT models
void ONF_StepExtractLogBuffer::createOutResultModelListProtected()
{
    CT_OutResultModelGroupToCopyPossibilities *resCpy_rcircles = createNewOutResultModelToCopy(DEFin_rcircles);

    if(resCpy_rcircles != NULL)
        resCpy_rcircles->addItemModel(DEFin_grpLog, _outPoint_ModelName, new CT_PointCluster(), tr("Points extraits"));
}

// Semi-automatic creation of step parameters DialogBox
void ONF_StepExtractLogBuffer::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addDouble("Accroissement de la taille des cercles :", "cm", 0, 1e+09, 2, _circleIncrement, 100);
    configDialog->addDouble("Taille du buffer", "cm", 0, 1e+09, 2, _buffer, 100);
}

void ONF_StepExtractLogBuffer::compute()
{

    QList<CT_ResultGroup*> inResultList = getInputResults();
    CT_ResultGroup* resIn_rpts = inResultList.at(0);

    QList<CT_ResultGroup*> outResultList = getOutResultList();
    CT_ResultGroup* resCpy_rcircles = outResultList.at(0);  

    QMap<CT_StandardItemGroup*, QList<QPair<CT_CircleData, CT_CircleData> > > circlesMap;
    QMap<CT_StandardItemGroup*, CT_PointCluster*> clustersMap;

    CT_ResultGroupIterator itCpy_grpLog(resCpy_rcircles, this, DEFin_grpLog);
    while (itCpy_grpLog.hasNext() && !isStopped())
    {
        CT_StandardItemGroup* grpCpy_grpLog = (CT_StandardItemGroup*) itCpy_grpLog.next();
                
        QList<QPair<CT_CircleData, CT_CircleData> > circlesList;

        CT_GroupIterator itCpy_grpCircles(grpCpy_grpLog, this, DEFin_grpCircles);
        while (itCpy_grpCircles.hasNext() && !isStopped())
        {
            CT_StandardItemGroup* grpCpy_grpCircles = (CT_StandardItemGroup*) itCpy_grpCircles.next();
            
            const CT_Circle* itemCpy_circle = (CT_Circle*)grpCpy_grpCircles->firstItemByINModelName(this, DEFin_circle);
            if (itemCpy_circle != NULL)
            {
                const CT_CircleData &circle = (const CT_CircleData &) itemCpy_circle->getData();

                CT_CircleData smallCircle(circle.getCenter(), Eigen::Vector3d(0,0,1), circle.getRadius() + _circleIncrement);
                CT_CircleData bigCircle(circle.getCenter(), Eigen::Vector3d(0,0,1), circle.getRadius() + _circleIncrement + _buffer);

                circlesList.append(QPair<CT_CircleData, CT_CircleData>(smallCircle, bigCircle));
            }            
        }

        if (circlesList.size() >= 2)
        {
            circlesMap.insert(grpCpy_grpLog, circlesList);
            clustersMap.insert(grpCpy_grpLog, new CT_PointCluster(_outPoint_ModelName.completeName(), resCpy_rcircles));
        }
    }


    CT_ResultGroupIterator itIn_grpPts(resIn_rpts, this, DEFin_grpPts);
    while (itIn_grpPts.hasNext() && !isStopped())
    {
        const CT_AbstractItemGroup* grpIn_grpPts = (CT_AbstractItemGroup*) itIn_grpPts.next();

        const CT_AbstractItemDrawableWithPointCloud* itemIn_pts = (CT_AbstractItemDrawableWithPointCloud*)grpIn_grpPts->firstItemByINModelName(this, DEFin_pts);
        if (itemIn_pts != NULL)
        {
            CT_PointIterator itP(itemIn_pts->getPointCloudIndex());
            while (itP.hasNext())
            {
                const CT_Point &point = itP.next().currentPoint();
                size_t index = itP.currentGlobalIndex();

                QMapIterator<CT_StandardItemGroup*, CT_PointCluster*> itClusters(clustersMap);
                while (itClusters.hasNext())
                {
                    itClusters.next();

                    CT_StandardItemGroup* groupe = itClusters.key();
                    CT_PointCluster* cluster = itClusters.value();

                    const QList<QPair<CT_CircleData, CT_CircleData> > &list = circlesMap.value(groupe);

                    if (point(2) >= list.first().first.getCenter()(2) && point(2) <= list.last().first.getCenter()(2))
                    {
                        double zmin, zmax;

                        int sizeList = list.size();

                        for (int i = 0 ; i < sizeList ; i++)
                        {
                            const QPair<CT_CircleData, CT_CircleData> &pair = list.at(i);

                            zmin = pair.first.getCenter()(2);
                            zmax = zmin;

                            if (i > 0)
                            {
                                const QPair<CT_CircleData, CT_CircleData> &pairAvant = list.at(i - 1);
                                zmin = (zmin + pairAvant.first.getCenter()(2)) / 2.0;
                            }

                            if (i < sizeList - 1)
                            {
                                const QPair<CT_CircleData, CT_CircleData> &pairApres= list.at(i + 1);
                                zmax = (zmax + pairApres.first.getCenter()(2)) / 2.0;
                            }

                            if (point(2) >= zmin &&
                                point(2) <= zmax &&
                               !contains(pair.first, point) &&
                                contains(pair.second, point))
                            {
                                cluster->addPoint(index);
                            }
                        }
                    }
                }
            }
        }
    }


    QMapIterator<CT_StandardItemGroup*, CT_PointCluster*> itClusters(clustersMap);
    while (itClusters.hasNext())
    {
        itClusters.next();

        CT_StandardItemGroup* groupe = itClusters.key();
        CT_PointCluster* cluster = itClusters.value();

        if (cluster->getPointCloudIndex()->size() >0)
        {
            groupe->addItemDrawable(cluster);
        } else {
            delete cluster;
        }
    }
}

bool ONF_StepExtractLogBuffer::contains(const CT_CircleData &circle, const Eigen::Vector3d &point) const
{
    double dist = CT_MathPoint::distance2D(circle.getCenter(), point);
    return dist <= circle.getRadius();
}

