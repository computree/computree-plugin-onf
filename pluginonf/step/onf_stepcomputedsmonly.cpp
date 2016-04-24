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

#include "onf_stepcomputedsmonly.h"


#ifdef USE_OPENCV
#include "ct_global/ct_context.h"

#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/ct_outresultmodelgroupcopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"

#include "ct_result/ct_resultgroup.h"

#include "ct_itemdrawable/ct_scene.h"
#include "ct_itemdrawable/abstract/ct_abstractareashape2d.h"
#include "ct_itemdrawable/ct_triangulation2d.h"
#include "ct_triangulation/ct_delaunayt.h"
#include "ct_triangulation/ct_nodet.h"
#include "ct_triangulation/ct_trianglet.h"
#include "ct_pointcloudindex/ct_pointcloudindexvector.h"
#include "ct_iterator/ct_pointiterator.h"
#include "ct_view/ct_stepconfigurabledialog.h"

#include "ct_itemdrawable/tools/image2dtools/ct_image2dnaturalneighboursinterpolator.h"


#include <QtConcurrent>
#include <QTimer>
#include <math.h>
#include <stdlib.h>
#include <limits>

#define DEF_SearchInResult   "ires"
#define DEF_SearchInGroup   "igrp"
#define DEF_SearchInScene   "isc"
#define DEF_SearchInArea   "emprise"

#define EPSILON 0.000001

ONF_StepComputeDSMOnly::ONF_StepComputeDSMOnly(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _gridsize   = 0.5;
    _interpol = true;
    _smooth = false;
    _smoothDist = 2;
    _nCells = 10;

    _convertNA = true;
}

QString ONF_StepComputeDSMOnly::getStepDescription() const
{
    return tr("Créer DSM (Zmax)");
}

QString ONF_StepComputeDSMOnly::getStepDetailledDescription() const
{
    return tr("Cette étape permet de générer un Modèle Numérique de Surface (MNS).<br>"
              "Le MNS est calculé comme une grille Zmax à la <b>résolution</b> spécifiée. <br>"
              "Ce MNS peut être optionellement interpolé et/ou lissée selon les options cochées.");
}

CT_VirtualAbstractStep* ONF_StepComputeDSMOnly::createNewInstance(CT_StepInitializeData &dataInit)
{
    // cree une copie de cette etape
    return new ONF_StepComputeDSMOnly(dataInit);
}

/////////////////////// PROTECTED ///////////////////////

void ONF_StepComputeDSMOnly::createInResultModelListProtected()
{
    CT_InResultModelGroupToCopy *resultModel = createNewInResultModelForCopy(DEF_SearchInResult, tr("Points végétation"));

    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_SearchInGroup);
    resultModel->addItemModel(DEF_SearchInGroup, DEF_SearchInScene, CT_Scene::staticGetType(), tr("Points végétation"));
    resultModel->addItemModel(DEF_SearchInGroup, DEF_SearchInArea, CT_AbstractAreaShape2D::staticGetType(),
                              tr("Emprise"), "", CT_InAbstractModel::C_ChooseOneIfMultiple, CT_InAbstractModel::F_IsOptional);
}

void ONF_StepComputeDSMOnly::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addDouble(tr("Résolution de la grille :"), "cm", 1, 1000, 0, _gridsize, 100);
    configDialog->addBool(tr("Interpolation"), "", "", _interpol);
    configDialog->addInt(tr("Taille de la fenêtre d'interpolation"), tr("Cases"), 1, 10000, _nCells);
    configDialog->addBool(tr("Lissage (filtre moyen)"), "", "", _smooth);
    configDialog->addDouble(tr("Voisinage de lissage :"), "Cases", 1, 99999999, 0, _smoothDist);
    configDialog->addBool(tr("Convertir valeurs NA en min(MNS) ?"), "", "", _convertNA);
}

void ONF_StepComputeDSMOnly::createOutResultModelListProtected()
{
    CT_OutResultModelGroupToCopyPossibilities *resultModel = createNewOutResultModelToCopy(DEF_SearchInResult);

    if(resultModel != NULL)
        resultModel->addItemModel(DEF_SearchInGroup, _outDSMModelName, new CT_Image2D<float>(), tr("MNS"));
}

void ONF_StepComputeDSMOnly::compute()
{
    // recupere les resultats de sortie
    const QList<CT_ResultGroup*> &outResList = getOutResultList();
    // récupération des modéles out
    CT_ResultGroup *outResult = outResList.at(0);

    CT_ResultGroupIterator it(outResult, this, DEF_SearchInGroup);
    while (!isStopped() && it.hasNext())
    {
        CT_StandardItemGroup* group = (CT_StandardItemGroup*) it.next();

        if (group != NULL)
        {
            const CT_Scene *scene = (const CT_Scene*)group->firstItemByINModelName(this, DEF_SearchInScene);
            const CT_AbstractPointCloudIndex *pointCloudIndex = scene->getPointCloudIndex();

            const CT_AbstractAreaShape2D *emprise = (const CT_AbstractAreaShape2D*)group->firstItemByINModelName(this, DEF_SearchInArea);


            // Creation du raster
            size_t n_mnsX = abs((scene->maxX() - scene->minX())/_gridsize) + 2;
            size_t n_mnsY = abs((scene->maxY() - scene->minY())/_gridsize) + 2;
            size_t tab_mns_size = n_mnsX*n_mnsY;

            double minX = scene->minX();
            double minY = scene->minY();
            double maxX = scene->maxX();
            double maxY = scene->maxY();

            if (emprise != NULL)
            {
                Eigen::Vector3d min, max;
                emprise->getBoundingBox(min, max);

                minX = min(0);
                minY = min(1);
                maxX = max(0) - EPSILON;
                maxY = max(1) - EPSILON;
            }

            CT_Image2D<float>* mns = CT_Image2D<float>::createImage2DFromXYCoords(_outDSMModelName.completeName(), outResult, minX, minY, maxX, maxY, _gridsize, scene->minZ(), -9999, -9999);

            // Création MNS (version Zmax)
            CT_PointIterator itP(pointCloudIndex);
            while(itP.hasNext() && !isStopped())
            {
                const CT_Point &point =itP.next().currentPoint();
                mns->setMaxValueAtCoords(point(0), point(1), point(2));
            }

            setProgress(20);

            size_t cpt = 0;

            // interpolation de la grille MNT
            if (_interpol) {
                CT_Image2D<float>* mnsInterpol = (CT_Image2D<float>*) mns->copy(_outDSMModelName.completeName(), outResult, CT_ResultCopyModeList() << CT_ResultCopyModeList::CopyItemDrawableReference);

                QList<size_t> NAindices;

                size_t index;
                float value;
                for (size_t xx=0 ; xx<n_mnsX ; ++xx) {
                    for (size_t yy=0 ; yy<n_mnsY ; ++yy) {
                        value = mns->value(xx,yy);
                        if (value == mns->NA())
                        {
                            mns->index(xx, yy, index);
                            NAindices.append(index);
                        }
                    }
                }

                QFuture<void> futur = QtConcurrent::map(NAindices, CT_Image2DNaturalNeighboursInterpolator(mns, mnsInterpol, _nCells));

                int progressMin = futur.progressMinimum();
                int progressTotal = futur.progressMaximum() - futur.progressMinimum();
                while (!futur.isFinished())
                {
                    setProgress(60.0*(futur.progressValue() - progressMin)/progressTotal + 20.0);
                }

                delete mns;
                mns = mnsInterpol;

                setProgress(80);
                PS_LOG->addMessage(LogInterface::info, LogInterface::step, tr("Interpolation du MNS terminée"));
            }


            cpt = 0;
            // Lissage de la grille MNS
            if (_smooth) {
                CT_Image2D<float>* MNSsmoothed = (CT_Image2D<float>*) mns->copy(_outDSMModelName.completeName(), outResult, CT_ResultCopyModeList() << CT_ResultCopyModeList::CopyItemDrawableReference);
                for (size_t xx=0 ; xx<n_mnsX ; ++xx) {
                    for (size_t yy=0 ; yy<n_mnsY ; ++yy) {

                        QList<float> neighbours = mns->neighboursValues(xx, yy, _smoothDist, false, CT_Image2D<float>::CM_DropCenter);
                        int size_neigh = neighbours.size();
                        double somme = 0;
                        for (int i = 0 ; i < size_neigh ; i++)
                        {
                            somme += neighbours.at(i);
                        }

                        if (size_neigh >0)
                        {
                            somme = somme / (double)size_neigh;
                        }

                        MNSsmoothed->setValue(xx, yy, somme);

                        // Progression Etape 5
                        setProgress(((double)(cpt++)/(double)tab_mns_size)*10 + 80);
                    }
                }

                delete mns;
                mns = MNSsmoothed;

                setProgress(90);
                PS_LOG->addMessage(LogInterface::info, LogInterface::step, tr("Lissage du MNS terminé"));
            }


            // Progression Etape 7
            setProgress(95);

            // ajout du raster MNS
            group->addItemDrawable(mns);
            mns->computeMinMax();

            if (_convertNA)
            {
                for (size_t index = 0 ; index < mns->nCells() ; index++)
                {
                    if (mns->valueAtIndex(index) == mns->NA()) {mns->setValueAtIndex(index, mns->dataMin());}
                }
            }
        }
        setProgress(100);
    }
}


#endif
