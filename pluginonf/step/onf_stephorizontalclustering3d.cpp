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


#include "onf_stephorizontalclustering3d.h"

#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"
#include "ct_result/ct_resultgroup.h"
#include "ct_result/tools/iterator/ct_resultgroupiterator.h"

#include "ct_itemdrawable/ct_scene.h"
#include "ct_itemdrawable/ct_pointcluster.h"
#include "ct_itemdrawable/tools/ct_standardcontext.h"

#include "ct_math/ct_mathpoint.h"
#include "ct_math/ct_mathboundingshape.h"

#include "ct_pointcloudindex/abstract/ct_abstractpointcloudindex.h"
#include "ct_iterator/ct_pointiterator.h"

#include "ct_global/ct_context.h"

#include "ct_view/ct_stepconfigurabledialog.h"

#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
#include <QtConcurrentMap>
#else
#include <QtConcurrent/QtConcurrentMap>
#endif

#include <QFutureWatcher>

#define DEF_SearchInResult "r"
#define DEF_SearchInGroup   "p"
#define DEF_SearchInScene   "sc"

CT_PointAccessor ONF_StepHorizontalClustering3D::P_ACCESS = CT_PointAccessor();

ONF_StepHorizontalClustering3D::ONF_StepHorizontalClustering3D(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _maxSearchRadiusInMeters = 0.03;
    _thicknessInMeters = 0.02;
}

QString ONF_StepHorizontalClustering3D::getStepDescription() const
{
    return tr("Clustering 3D");
}

QString ONF_StepHorizontalClustering3D::getStepDetailledDescription() const
{
    return tr("Cette étape vise à constituer de petits groupes de points aggrégés. "
              "L'idée est d'obtenir, dans le cas de troncs d'arbres des arcs de cercle peu épais.<br>"
              "Pour ce faire, l'étape fonctionne en deux étapes :"
              "<ul>"
              "<li> La scène est découpée en tranches horizontales (Layers) de l' <b>épaisseur</b> choisie</li>"
              "<li> Dans chacune des tranches, les points sont aggrégés en clusters en fonction de leur espacement en (x,y)</li>"
              "</ul>"
              "<br>La <b>distance maximale séparant deux points d'un même groupe</b> est spécifiée en paramètre."
              "<br>Cette version 05 de l'étape permet de traiter séparément chaque scène d'entrée."
              "<br>De plus dans cette version, le résultat d'entrée est en copie.");
}

CT_VirtualAbstractStep* ONF_StepHorizontalClustering3D::createNewInstance(CT_StepInitializeData &dataInit)
{
    // crée une copie de cette étape
    return new ONF_StepHorizontalClustering3D(dataInit);
}

//////////////////// PROTECTED //////////////////

void ONF_StepHorizontalClustering3D::createInResultModelListProtected()
{
    CT_InResultModelGroupToCopy * resultModel = createNewInResultModelForCopy(DEF_SearchInResult, tr("Scène(s)"));
    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_SearchInGroup, CT_AbstractItemGroup::staticGetType(), tr("Groupe"));
    resultModel->addItemModel(DEF_SearchInGroup, DEF_SearchInScene, CT_Scene::staticGetType(), tr("Scène à clusteriser"));
}

void ONF_StepHorizontalClustering3D::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addDouble(tr("Distance maximum pour intégrer un point à un groupe :"), "cm", 0, 500, 0, _maxSearchRadiusInMeters, 100);
    configDialog->addDouble(tr("Epaisseur des tranches horizontales :"), "cm", 1, 1e+09, 0, _thicknessInMeters, 100);
}

void ONF_StepHorizontalClustering3D::createOutResultModelListProtected()
{   
    CT_OutResultModelGroupToCopyPossibilities *resultModel = createNewOutResultModelToCopy(DEF_SearchInResult);

    if(resultModel != NULL) {
        resultModel->addGroupModel(DEF_SearchInGroup, _outLayerGroupModel, new CT_StandardItemGroup(), tr("Niveau Z (Grp)"));
        resultModel->addGroupModel(_outLayerGroupModel, _outClusterGroupModel, new CT_StandardItemGroup(), tr("Cluster (Grp)"));
        resultModel->addItemModel(_outClusterGroupModel, _outClusterModel, new CT_PointCluster(), tr("Points"));
    }
}

void ONF_StepHorizontalClustering3D::compute()
{
    // recupere les resultats d'entree
    // les resultats sont dans l'ordre qu'on les a demande dans la methode createInResultModelListProtected()
    _outResult = (CT_ResultGroup*) getOutResultList().first();

    int iThickness = 10;
    float thicknessInCm = _thicknessInMeters*100.0;
    float thicknessDivider = thicknessInCm/10.0;

    if(thicknessInCm < 10)
    {
        iThickness = 100;
        thicknessDivider = thicknessInCm;
    }
    else if(thicknessInCm >= 100)
    {
        iThickness = 1;
        thicknessDivider = thicknessInCm/100;
    }

    QString outClusterGroupModel = _outClusterGroupModel.completeName();
    QString outClusterModel = _outClusterModel.completeName();

    QMap<float, CT_StandardItemGroup*> mapMultithread;

    CT_ResultGroupIterator itSceneGrp(_outResult, this, DEF_SearchInGroup);
    while (itSceneGrp.hasNext() && (!isStopped()))
    {
        CT_StandardItemGroup* sceneGroup = (CT_StandardItemGroup*) itSceneGrp.next();
        const CT_Scene *scene = (const CT_Scene*) sceneGroup->firstItemByINModelName(this, DEF_SearchInScene);

        if(scene != NULL)
        {
            const CT_AbstractPointCloudIndex *pointCloudIndex = scene->getPointCloudIndex();
            size_t indexSize = pointCloudIndex->size();

            PS_LOG->addMessage(LogInterface::info, LogInterface::step, QString(tr("La scène à clusteriser comporte %1 points.")).arg(indexSize));

            size_t i = 0;

            // Tout d'abord on range tous les points dans diffrentes couches Z d'épaisseur définie par l'utilisateur
            CT_PointIterator itP(pointCloudIndex);
            while(itP.hasNext() && !isStopped())
            {
                const CT_Point &point = itP.next().currentPoint();
                size_t indexPoint = itP.currentGlobalIndex();

                // calcul de l'index de la couche
                float indexZ = (((int)((point(2)*iThickness)/thicknessDivider))+1) * _thicknessInMeters;

                CT_StandardItemGroup *layer = mapMultithread.value(indexZ, NULL);

                CT_StandardContext *context;
                if(layer == NULL)
                {
                    layer = new CT_StandardItemGroup(_outLayerGroupModel.completeName(), _outResult);
                    context = new CT_StandardContext();
                    context->setStep(this);
                    context->add("_outClusterModelName", &outClusterModel);
                    context->add("_outClusterGroupModelName", &outClusterGroupModel);
                    layer->setContext(context);
                    mapMultithread.insert(indexZ, layer);
                } else {
                    context = (CT_StandardContext*) layer->getContext();
                }

                context->add("p", new size_t(indexPoint));

                // progres de 0 a 25
                setProgress((i*25)/indexSize);

                ++i;
            }



            _sizeMap = mapMultithread.size();

            if (_sizeMap > 0)
            {
                _nLayerFinished = 0;

                QtConcurrent::blockingMap(mapMultithread, &ONF_StepHorizontalClustering3D::staticComputeLayer);

                QMapIterator<float, CT_StandardItemGroup*> it(mapMultithread);

                while(it.hasNext())
                {
                    it.next();
                    CT_StandardContext* context = ((CT_StandardContext*) it.value()->getContext());

                    QList<void*> mypoints = context->get("p");

                    int size = mypoints.size();

                    for (int i = 0 ; i < size ; i++)
                        delete ((size_t*) mypoints.at(i));

                    delete context;
                    it.value()->setContext(NULL);
                }

                it.toFront();

                if(!isStopped())
                {
                    while(it.hasNext())
                    {
                        it.next();
                        sceneGroup->addGroup(it.value());
                    }
                }
                else
                {
                    qDeleteAll(mapMultithread);
                }
                mapMultithread.clear();

                PS_LOG->addMessage(LogInterface::info, LogInterface::step, QString(tr("L'étape a généré %1 couches horizontales.")).arg(_sizeMap));
            }

        }
    }

}

void ONF_StepHorizontalClustering3D::addLayerFinished()
{
    int progress;

    _mutexUpdateProgress.lock();

    ++_nLayerFinished;
    progress = 25 + ((_nLayerFinished*75)/_sizeMap);

    _mutexUpdateProgress.unlock();

    setProgress(progress);
}

void ONF_StepHorizontalClustering3D::staticComputeLayer(CT_StandardItemGroup *layer)
{
    CT_StandardContext* context = (CT_StandardContext*) layer->getContext();
    ONF_StepHorizontalClustering3D* step = (ONF_StepHorizontalClustering3D*) context->step();
    QList<void*> mypoints = context->get("p");
    QString outClusterModelName = *((const QString*) context->get("_outClusterModelName").first());
    QString outClusterGroupModelName = *((const QString*) context->get("_outClusterGroupModelName").first());

    QList<CT_PointCluster*> listCluster;

    while(!step->isStopped()
          && !mypoints.isEmpty())
    {
        size_t *p = (size_t*) mypoints.takeFirst();

        bool continueLoop = true;
        QListIterator<CT_PointCluster*> it(listCluster);

        while(continueLoop
              && it.hasNext())
        {
            continueLoop = staticAddPointToPointCluster(*p, *(it.next()), step);
        }

        if(continueLoop)
        {
            CT_PointCluster *pCluster = new CT_PointCluster(outClusterModelName, step->_outResult);
            pCluster->addPoint(*p);

            listCluster.append(pCluster);

        }
    }

    while(!listCluster.isEmpty()
          && !step->isStopped())
    {
        CT_PointCluster *pClusterToOptimize = listCluster.takeFirst();

        int size = listCluster.size();
        bool continueLoop = true;

        do
        {
            continueLoop = true;
            int j = 0;

            while(continueLoop
                  && (j<size)
                  && !step->isStopped())
            {
                CT_PointCluster *pClusterToVerify = (CT_PointCluster*)listCluster.at(j);

                CT_PointCluster *pClusterMerged = staticCombinePointCluster(pClusterToOptimize->id(), *(step->_outResult), *pClusterToOptimize, *pClusterToVerify, step, outClusterModelName);

                if(pClusterMerged != NULL)
                {
                    delete pClusterToOptimize;
                    pClusterToOptimize = pClusterMerged;

                    listCluster.removeAt(j);
                    delete pClusterToVerify;

                    --size;
                    continueLoop = false;
                }
                else
                {
                    ++j;
                }
            }

        } while(!continueLoop);

        CT_StandardItemGroup* grp = new CT_StandardItemGroup(outClusterGroupModelName, step->_outResult);
        grp->addItemDrawable(pClusterToOptimize);
        layer->addGroup(grp);
    }

    //layer->updateCenter();


    step->addLayerFinished();
}

bool ONF_StepHorizontalClustering3D::staticAddPointToPointCluster(const size_t &pointIndex,
                                                                 CT_PointCluster &pCluster,
                                                                 ONF_StepHorizontalClustering3D *ptrClass)
{
    CT_Point point = P_ACCESS.pointAt(pointIndex);

    Eigen::Vector3d min, max;
    pCluster.getBufferedBoundingBox(ptrClass->_maxSearchRadiusInMeters, ptrClass->_maxSearchRadiusInMeters, 1, min, max);

    if(CT_MathBoundingShape::containsPointIn2D(min, max, point))
    {
        const CT_AbstractPointCloudIndex *pointCloudIndex = ((const CT_PointCluster&)pCluster).getPointCloudIndex();

        if (pointCloudIndex->size() == 0) {return true;}

        CT_PointIterator itP(pointCloudIndex);

        itP.toBack();
        while (itP.hasPrevious())
        {
            const CT_Point &otherPoint = itP.previous().currentPoint();

            if(CT_MathPoint::distance3D(otherPoint, point) < ptrClass->_maxSearchRadiusInMeters)
            {
                pCluster.addPoint(pointIndex);
                return false;
            }
        }

    }
    return true;
}

CT_PointCluster* ONF_StepHorizontalClustering3D::staticCombinePointCluster(int id,
                                                                          CT_AbstractResult &outResult,
                                                                          CT_PointCluster &pClusterToOptimize,
                                                                          CT_PointCluster &pClusterToCombine,
                                                                          ONF_StepHorizontalClustering3D *ptrClass,
                                                                          QString outClusterModelName)
{
    Eigen::Vector3d min, max;
    pClusterToOptimize.getBufferedBoundingBox(ptrClass->_maxSearchRadiusInMeters, ptrClass->_maxSearchRadiusInMeters, 1, min, max);

    const CT_AbstractPointCloudIndex *pointCloudIndexGpToOptimize = pClusterToOptimize.getPointCloudIndex();
    const CT_AbstractPointCloudIndex *pointCloudIndexGpToCombine = pClusterToCombine.getPointCloudIndex();

    if (pointCloudIndexGpToOptimize->size() == 0) {return NULL;}
    if (pointCloudIndexGpToCombine->size() == 0) {return NULL;}

    CT_PointIterator itP(pointCloudIndexGpToCombine);
    CT_PointIterator itP2(pointCloudIndexGpToOptimize);

    itP.toBack();
    while (itP.hasPrevious())
    {
        const CT_Point &point = itP.previous().currentPoint();

        if(CT_MathBoundingShape::containsPointIn2D(min, max, point))
        {
            itP2.toBack();
            while (itP2.hasPrevious())
            {
                const CT_Point &other_point = itP2.previous().currentPoint();

                if(CT_MathPoint::distance3D(other_point, point) < ptrClass->_maxSearchRadiusInMeters)
                {
                    return CT_PointCluster::merge(pClusterToOptimize, pClusterToCombine, outClusterModelName, id, outResult, false);
                }
            }
        }

    }

    return NULL;
}
