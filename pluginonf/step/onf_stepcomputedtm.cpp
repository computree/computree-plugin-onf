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


#include "onf_stepcomputedtm.h"


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

#include <math.h>
#include <stdlib.h>
#include <limits>

#define DEF_SearchInResult   "ires"
#define DEF_SearchInGroup   "igrp"
#define DEF_SearchInScene   "isc"

ONF_StepComputeDTM::ONF_StepComputeDTM(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _gridsize   = 0.5;
    _interpol = true;
    _smooth = false;
    _smoothDist = 2;

    _convertNA = true;
}

QString ONF_StepComputeDTM::getStepDescription() const
{
    return tr("Création d'un MNT");
}

QString ONF_StepComputeDTM::getStepDetailledDescription() const
{
    return tr("Cette étape permet de séparer les points Sol et Végétation, et de générer :un Modèle Numérique de Terrain (MNT).<br>"
              "Le MNT est calculé comme une grille Zmin à la <b>résolution</b> spécifiée. <br>"
              "Ce MNT peut être optionellement interpolé et/ou lissée selon les options cochées.");
}

CT_VirtualAbstractStep* ONF_StepComputeDTM::createNewInstance(CT_StepInitializeData &dataInit)
{
    // cree une copie de cette etape
    return new ONF_StepComputeDTM(dataInit);
}

/////////////////////// PROTECTED ///////////////////////

void ONF_StepComputeDTM::createInResultModelListProtected()
{  
    CT_InResultModelGroupToCopy *resultModel = createNewInResultModelForCopy(DEF_SearchInResult, tr("Points sol"));

    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_SearchInGroup);
    resultModel->addItemModel(DEF_SearchInGroup, DEF_SearchInScene, CT_Scene::staticGetType(), tr("Points sol"));
}

void ONF_StepComputeDTM::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addDouble(tr("Résolution de la grille :"), "cm", 1, 1000, 0, _gridsize, 100);
    configDialog->addBool(tr("Interpolation"), "", "", _interpol);
    configDialog->addBool(tr("Lissage"), "", "", _smooth);
    configDialog->addDouble(tr("Voisinage de lissage :"), "Cases", 1, 99999999, 0, _smoothDist);
    configDialog->addBool(tr("Convertir valeurs NA en min(MNT) ?"), "", "", _convertNA);
}

void ONF_StepComputeDTM::createOutResultModelListProtected()
{
    CT_OutResultModelGroupToCopyPossibilities *resultModel = createNewOutResultModelToCopy(DEF_SearchInResult);
    resultModel->addItemModel(DEF_SearchInGroup, _outDTMModelName, new CT_Image2D<float>(), tr("MNT"));
}

void ONF_StepComputeDTM::compute()
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

            // Creation du raster
            size_t n_mntX = abs((scene->maxX() - scene->minX())/_gridsize) + 2;
            size_t n_mntY = abs((scene->maxY() - scene->minY())/_gridsize) + 2;
            size_t tab_mnt_size = n_mntX*n_mntY;

            CT_Image2D<float>* mnt = new CT_Image2D<float>(_outDTMModelName.completeName(), outResult, scene->minX(), scene->minY(), n_mntX, n_mntY, _gridsize, scene->minZ(), -9999, -9999);

            // Création MNT (version Zmin)
            CT_PointIterator itP(pointCloudIndex);
            while(itP.hasNext() && !isStopped())
            {
                const CT_Point &point =itP.next().currentPoint();
                mnt->setMinValueAtCoords(point(0), point(1), point(2));
            }

            setProgress(20);

            size_t cpt = 0;

            // interpolation de la grille MNT
            if (_interpol) {
                // Triangulation des points non interpoles du MNT
                CT_DelaunayT *delaunay = new CT_DelaunayT();

                for (size_t xx=0 ; xx<n_mntX ; ++xx) {
                    for (size_t yy=0 ; yy<n_mntY ; ++yy) {
                        float value = mnt->value(xx,yy);

                        if (value != mnt->NA())
                        {
                            Eigen::Vector3d* pt = new Eigen::Vector3d();
                            (*pt)(0) = mnt->getCellCenterColCoord(xx);
                            (*pt)(1) = mnt->getCellCenterLinCoord(yy);
                            (*pt)(2) = value;
                            delaunay->insertNode(CT_NodeT::create(pt, NULL, true));
                        }
                        // Progression Etape 4
                        setProgress(((double)(cpt++)/(double)tab_mnt_size)*40 + 20);
                    }
                }

                setProgress(60);
                PS_LOG->addMessage(LogInterface::info, LogInterface::step, tr("Triangulation des cases conservées terminée"));

                cpt = 0;

                CT_Image2D<float>* MNTinterpol = (CT_Image2D<float>*) mnt->copy(_outDTMModelName.completeName(), outResult, CT_ResultCopyModeList() << CT_ResultCopyModeList::CopyItemDrawableReference);
                for (size_t xx=0 ; xx<n_mntX ; ++xx) {
                    for (size_t yy=0 ; yy<n_mntY ; ++yy) {

                        float value = mnt->value(xx,yy);
                        float x = mnt->getCellCenterColCoord(xx);
                        float y = mnt->getCellCenterLinCoord(yy);

                        if (value == mnt->NA()) {
                            Eigen::Vector3d* pt = new Eigen::Vector3d();
                            (*pt)(0) = (double)x;
                            (*pt)(1) = (double)y;
                            (*pt)(2) = (double)value;

                            QSharedPointer<CT_NodeT> noeud = CT_NodeT::create(pt, NULL, true);

                            delaunay->insertNode(noeud);

                            QList< QSharedPointer<CT_NodeT> > voisins = noeud.data()->getNodesAround();

                            double sum = 0;
                            double sum_poids = 0;
                            QListIterator < QSharedPointer<CT_NodeT> > it(voisins);

                            // Calcul de la somme ponderee
                            while (it.hasNext())
                            {
                                CT_NodeT *node = it.next().data();

                                Eigen::Vector3d* nodePt = node->getPoint();

                                double dx = x - (*nodePt)(0);
                                double dy = y - (*nodePt)(1);
                                double distance = sqrt(dx*dx + dy*dy);

                                sum += (*nodePt)(2) / distance;
                                sum_poids += 1 / distance;
                            }

                            delaunay->removeNode(noeud);

                            if (sum_poids != 0) {
                                MNTinterpol->setValue(xx, yy, sum / sum_poids);
                            }
                        }

                    }
                    // Progression Etape 5
                    setProgress(((double)(cpt++)/(double)tab_mnt_size)*20 + 60);
                }

                delete mnt;
                mnt = MNTinterpol;

                setProgress(80);
                PS_LOG->addMessage(LogInterface::info, LogInterface::step, tr("Interpolation du MNT terminée"));
            }


            cpt = 0;
            // Lissage de la grille MNT
            if (_smooth) {
                CT_Image2D<float>* MNTsmoothed = (CT_Image2D<float>*) mnt->copy(_outDTMModelName.completeName(), outResult, CT_ResultCopyModeList() << CT_ResultCopyModeList::CopyItemDrawableReference);
                for (size_t xx=0 ; xx<n_mntX ; ++xx) {
                    for (size_t yy=0 ; yy<n_mntY ; ++yy) {

                        QList<float> neighbours = mnt->neighboursValues(xx, yy, _smoothDist, false, CT_Image2D<float>::CM_DropCenter);
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

                        MNTsmoothed->setValue(xx, yy, somme);

                        // Progression Etape 5
                        setProgress(((double)(cpt++)/(double)tab_mnt_size)*10 + 80);
                    }
                }

                delete mnt;
                mnt = MNTsmoothed;

                setProgress(90);
                PS_LOG->addMessage(LogInterface::info, LogInterface::step, tr("Lissage du MNT terminé"));
            }


            // Progression Etape 7
            setProgress(95);

            // ajout du raster MNT
            group->addItemDrawable(mnt);
            mnt->computeMinMax();

            if (_convertNA)
            {
                for (size_t index = 0 ; index < mnt->nCells() ; index++)
                {
                    if (mnt->valueAtIndex(index) == mnt->NA()) {mnt->setValueAtIndex(index, mnt->dataMin());}
                }
            }
        }
        setProgress(100);
    }
}
#endif
