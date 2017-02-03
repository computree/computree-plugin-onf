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


#include "onf_stepcomputedsm.h"


#ifdef USE_OPENCV
#include "ct_global/ct_context.h"

#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/ct_outresultmodelgroupcopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"

#include "ct_result/ct_resultgroup.h"

#include "ct_itemdrawable/ct_scene.h"
#include "ct_itemdrawable/ct_triangulation2d.h"
#include "ct_triangulation/ct_delaunayt.h"
#include "ct_triangulation/ct_nodet.h"
#include "ct_triangulation/ct_trianglet.h"
#include "ct_itemdrawable/ct_image2d.h"
#include "ct_pointcloudindex/ct_pointcloudindexvector.h"
#include "ct_iterator/ct_pointiterator.h"
#include "ct_view/ct_stepconfigurabledialog.h"
#include "ct_itemdrawable/tools/image2dtools/ct_image2dnaturalneighboursinterpolator.h"


#include <QtConcurrent>
#include <math.h>
#include <stdlib.h>
#include <limits>


#define DEF_SearchInResult   "ires"
#define DEF_SearchInGroup   "igrp"
#define DEF_SearchInMNT   "imnt"
#define DEF_SearchInScene   "isc"

ONF_StepComputeDSM::ONF_StepComputeDSM(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _interpol = true;
    _nCells = 10;
    _convertNA = true;

}

QString ONF_StepComputeDSM::getStepDescription() const
{
    return tr("Créer MNS et MNH");
}

QString ONF_StepComputeDSM::getStepDetailledDescription() const
{
    return tr("Cette étape permet de générer un MNS et un MNH à partir d'un nuage de points et d'un MNT");
}

CT_VirtualAbstractStep* ONF_StepComputeDSM::createNewInstance(CT_StepInitializeData &dataInit)
{
    // cree une copie de cette etape
    return new ONF_StepComputeDSM(dataInit);
}

/////////////////////// PROTECTED ///////////////////////

void ONF_StepComputeDSM::createInResultModelListProtected()
{  
    CT_InResultModelGroupToCopy *resultModel = createNewInResultModelForCopy(DEF_SearchInResult, tr("Points sol"));

    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_SearchInGroup);
    resultModel->addItemModel(DEF_SearchInGroup, DEF_SearchInMNT, CT_Image2D<float>::staticGetType(), tr("MNT"));
    resultModel->addItemModel(DEF_SearchInGroup, DEF_SearchInScene, CT_Scene::staticGetType(), tr("Scène"));
}

void ONF_StepComputeDSM::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addBool(tr("Interpolation"), "", "", _interpol);
    configDialog->addInt(tr("Taille de la fenêtre d'interpolation"), tr("Cases"), 1, 10000, _nCells);
    configDialog->addBool(tr("Convertir valeurs NA en min(MNS) ?"), "", "", _convertNA);
}

void ONF_StepComputeDSM::createOutResultModelListProtected()
{
    CT_OutResultModelGroupToCopyPossibilities *resultModel = createNewOutResultModelToCopy(DEF_SearchInResult);

    if(resultModel != NULL) {
        resultModel->addItemModel(DEF_SearchInGroup, _outDSMModelName, new CT_Image2D<float>(), tr("MNS"));
        resultModel->addItemModel(DEF_SearchInGroup, _outCHMModelName, new CT_Image2D<float>(), tr("MNH"));
    }
}

void ONF_StepComputeDSM::compute()
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
            const CT_Scene *scene = (CT_Scene*)group->firstItemByINModelName(this, DEF_SearchInScene);
            const CT_AbstractPointCloudIndex *pointCloudIndex = scene->getPointCloudIndex();

            const CT_Image2D<float> *mnt = (CT_Image2D<float>*)group->firstItemByINModelName(this, DEF_SearchInMNT);

            // Creation du rasters
            size_t n_mntX = mnt->colDim();
            size_t n_mntY = mnt->linDim();

            CT_Image2D<float>* dsm = new CT_Image2D<float>(_outDSMModelName.completeName(), outResult, mnt->minX(), mnt->minY(), n_mntX, n_mntY, mnt->resolution(), mnt->minZ(), -9999, -9999);
            CT_Image2D<float>* chm = new CT_Image2D<float>(_outCHMModelName.completeName(), outResult, mnt->minX(), mnt->minY(), n_mntX, n_mntY, mnt->resolution(), mnt->minZ(), -9999, 0);

            // Création DSM (version Zmin)
            CT_PointIterator itP(pointCloudIndex);
            while(itP.hasNext() && !isStopped())
            {
                const CT_Point &point =itP.next().currentPoint();
                dsm->setMaxValueAtCoords(point(0), point(1), point(2));
            }

            setProgress(20);


            // interpolation de la grille DSM
            if (_interpol) {
                CT_Image2D<float>* dsmInterpol = (CT_Image2D<float>*) dsm->copy(_outDSMModelName.completeName(), outResult, CT_ResultCopyModeList() << CT_ResultCopyModeList::CopyItemDrawableReference);

                QList<size_t> NAindices;

                size_t index;
                float value;
                for (size_t xx=0 ; xx<n_mntX ; ++xx) {
                    for (size_t yy=0 ; yy<n_mntY ; ++yy) {
                        value = dsm->value(xx,yy);
                        if (value == dsm->NA())
                        {
                            dsm->index(xx, yy, index);
                            NAindices.append(index);
                        }
                    }
                }

                QFuture<void> futur = QtConcurrent::map(NAindices, CT_Image2DNaturalNeighboursInterpolator(dsm, dsmInterpol, _nCells));

                int progressMin = futur.progressMinimum();
                int progressTotal = futur.progressMaximum() - futur.progressMinimum();
                while (!futur.isFinished())
                {
                    setProgress(60.0*(futur.progressValue() - progressMin)/progressTotal + 20.0);
                }

                delete dsm;
                dsm = dsmInterpol;

                setProgress(80);
                PS_LOG->addMessage(LogInterface::info, LogInterface::step, tr("Interpolation du MNS terminée"));
            }


            // Progression Etape 7
            setProgress(95);

            // ajout du raster MNS
            group->addItemDrawable(dsm);
            dsm->computeMinMax();

            for (size_t index = 0 ; index < dsm->nCells() ; index++)
            {
                if (_convertNA)
                {
                    if (dsm->valueAtIndex(index) == dsm->NA()) {dsm->setValueAtIndex(index, dsm->dataMin());}
                }

                if (dsm->valueAtIndex(index) == dsm->NA() || mnt->valueAtIndex(index) == mnt->NA())
                {
                    chm->setValueAtIndex(index, 0);
                } else {
                    chm->setValueAtIndex(index, dsm->valueAtIndex(index) - mnt->valueAtIndex(index));
                }

            }

            // ajout du raster MNS
            group->addItemDrawable(chm);
            chm->computeMinMax();


        }
        setProgress(100);
    }
}
#endif
