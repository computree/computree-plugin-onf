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


#include "onf_stepclassifyground.h"


#ifdef USE_OPENCV
#include "ct_global/ct_context.h"

#include "ct_result/model/inModel/ct_inresultmodelgroup.h"
#include "ct_result/model/outModel/ct_outresultmodelgroup.h"

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

#define DEF_SearchOutResultVegetation "rv"
#define DEF_SearchOutResultMNT "rmnt"

#define DEF_SearchOutGroupVegetation  "gv"
#define DEF_SearchOutGroupMNT  "gmnt"

#define DEF_SearchOutSceneVegetation  "scv"
#define DEF_SearchOutSceneSoil  "scs"
#define DEF_SearchOutMNT  "mnt"
#define DEF_SearchOutDensite  "dens"



ONF_StepClassifyGround::ONF_StepClassifyGround(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _gridsize   = 0.5;
    _soilwidth = 0.32;
    _min_density = 200;
    _dist = 3;
}

QString ONF_StepClassifyGround::getStepDescription() const
{
    return tr("Classification des points sol");
}

QString ONF_StepClassifyGround::getStepDetailledDescription() const
{
    return tr("Cette étape permet de séparer les points Sol et Végétation"
              "<ul>"
              "<li>Une grille Zmin est créée à la <b>résolution</b> spécifiée</li>"
              "<li>La densité de points situés entre Zmin et (Zmin + <b>épaisseur du sol</b>) est calculée pour chaque case</li>"
              "<li>La valeur NA est affectée à toute case dont la densité est inférieure à la <b>densité minimum</b></li>"
              "<li>Un test de cohérence des Zmin restants est réalisé pour chaque case sur le <b>voisinage</b> spécifié (nombre de cases). La valeur NA est affectée aux cases incohérentes</li>"
              "</ul>");
}

CT_VirtualAbstractStep* ONF_StepClassifyGround::createNewInstance(CT_StepInitializeData &dataInit)
{
    // cree une copie de cette etape
    return new ONF_StepClassifyGround(dataInit);
}

/////////////////////// PROTECTED ///////////////////////

void ONF_StepClassifyGround::createInResultModelListProtected()
{  
    CT_InResultModelGroup *resultModel = createNewInResultModel(DEF_SearchInResult, tr("Scène(s)"));

    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_SearchInGroup);
    resultModel->addItemModel(DEF_SearchInGroup, DEF_SearchInScene, CT_Scene::staticGetType(), tr("Scène"));
}

void ONF_StepClassifyGround::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addDouble(tr("Résolution de la grille :"), "cm", 1, 1000, 0, _gridsize, 100);
    configDialog->addDouble(tr("Epaisseur du sol :"), "cm", 1, 100, 0, _soilwidth, 100);
    configDialog->addBool(tr("Filtrage selon la densité et le voisinnage"), "", "", _filter);
    configDialog->addDouble(tr("Densité minimum :"), "pts/m2", 0, 99999999, 2, _min_density);
    configDialog->addDouble(tr("Voisinage (points isolés) :"), "Cases", 1, 99999999, 0, _dist);
}

void ONF_StepClassifyGround::createOutResultModelListProtected()
{
    CT_OutResultModelGroup *resultModel;

    resultModel = createNewOutResultModel(DEF_SearchOutResultVegetation, tr("Points classifiés"));
    resultModel->setRootGroup(DEF_SearchOutGroupVegetation);
    resultModel->addItemModel(DEF_SearchOutGroupVegetation, DEF_SearchOutSceneVegetation, new CT_Scene(), tr("Points végétation"));
    resultModel->addItemModel(DEF_SearchOutGroupVegetation, DEF_SearchOutSceneSoil, new CT_Scene(), tr("Points sol"));

    resultModel = createNewOutResultModel(DEF_SearchOutResultMNT, tr("Rasters de classification"));
    resultModel->setRootGroup(DEF_SearchOutGroupMNT);
    resultModel->addItemModel(DEF_SearchOutGroupMNT, DEF_SearchOutMNT, new CT_Image2D<float>(), tr("MNT (Zmin)"));
    resultModel->addItemModel(DEF_SearchOutGroupMNT, DEF_SearchOutDensite, new CT_Image2D<int>(), tr("Densité pts sol"));
}

void ONF_StepClassifyGround::compute()
{

    // recupere le resultat d'entree
    CT_ResultGroup *inResult = getInputResults().first();

    // recupere les resultats de sortie
    const QList<CT_ResultGroup*> &outResList = getOutResultList();

    // récupération des modéles out
    CT_ResultGroup *outResultVegetation =       outResList.at(0);
    CT_ResultGroup *outResultMNT =              outResList.at(1);

    CT_ResultItemIterator it(inResult, this, DEF_SearchInScene);
    if (!isStopped() && it.hasNext())
    {

        // Determination de l'emprise du MNT, à partir de la bounding box des scènes d'entrée
        double minX = std::numeric_limits<double>::max();
        double minY = std::numeric_limits<double>::max();
        double minZ = std::numeric_limits<double>::max();
        double maxX = -std::numeric_limits<double>::max();
        double maxY = -std::numeric_limits<double>::max();
        double maxZ = -std::numeric_limits<double>::max();
        size_t xx = 0;
        size_t yy = 0;

        int nSc = 1;
        while (!isStopped() && it.hasNext())
        {
            const CT_Scene *scene = (CT_Scene*)it.next();
            const CT_AbstractPointCloudIndex *pointCloudIndex = scene->getPointCloudIndex();
            size_t n_points = pointCloudIndex->size();

            if (scene->minX() < minX) {minX = scene->minX();}
            if (scene->minY() < minY) {minY = scene->minY();}
            if (scene->minZ() < minZ) {minZ = scene->minZ();}
            if (scene->maxX() > maxX) {maxX = scene->maxX();}
            if (scene->maxY() > maxY) {maxY = scene->maxY();}
            if (scene->maxZ() > maxZ) {maxZ = scene->maxZ();}
            PS_LOG->addMessage(LogInterface::info, LogInterface::step, QString(tr("La scène d'entrée %2 comporte %1 points.")).arg(n_points).arg(nSc++));
        }

        // Creation des rasters
        size_t n_mntX = abs((maxX - minX)/_gridsize) + 2;
        size_t n_mntY = abs((maxY - minY)/_gridsize) + 2;
        size_t tab_mnt_size = n_mntX*n_mntY;

        CT_Image2D<float>* mnt = new CT_Image2D<float>(DEF_SearchOutMNT, outResultMNT, minX, minY, n_mntX, n_mntY, _gridsize, minZ, -9999, -9999);
        CT_Image2D<int>* densite = new CT_Image2D<int>(DEF_SearchOutDensite, outResultMNT, minX, minY, n_mntX, n_mntY, _gridsize, minZ - 1, -9999, 0);

        // Création MNT (version Zmin) + MNS
        CT_ResultItemIterator it2(inResult, this, DEF_SearchInScene);
        while (!isStopped() && it2.hasNext())
        {
            const CT_Scene *scene = (CT_Scene*)it2.next();
            const CT_AbstractPointCloudIndex *pointCloudIndex = scene->getPointCloudIndex();

            CT_PointIterator itP(pointCloudIndex);
            while(itP.hasNext() && !isStopped())
            {
                const CT_Point &point =itP.next().currentPoint();
                mnt->setMinValueAtCoords(point(0), point(1), point(2));
            }
        }

        size_t cpt = 0;
        // Progression Etape 1
        setProgress(20);
        PS_LOG->addMessage(LogInterface::info, LogInterface::step, tr("Grille Zmin créée"));

        // Creation raster densité points sol (sur la base de Zmin + _soilwidth)
        CT_ResultItemIterator it3(inResult, this, DEF_SearchInScene);
        while (!isStopped() && it3.hasNext())
        {
            const CT_Scene *scene = (CT_Scene*)it3.next();
            const CT_AbstractPointCloudIndex *pointCloudIndex = scene->getPointCloudIndex();

            CT_PointIterator itP(pointCloudIndex);
            while(itP.hasNext() && !isStopped())
            {
                const CT_Point &point =itP.next().currentPoint();
                float value = mnt->valueAtCoords(point(0), point(1)) + _soilwidth;
                if (point(2) < value)
                {
                    densite->addValueAtCoords(point(0), point(1), 1);
                }

            }
        }

        // Progression Etape 2
        setProgress(40);
        PS_LOG->addMessage(LogInterface::info, LogInterface::step, tr("Filtrage sur la densité terminé"));

        double min_density = _min_density * (_gridsize*_gridsize);


        if (_filter)
        {
            // Test de cohérence de voisinnage
            for (xx=0 ; xx<n_mntX ; ++xx) {
                for (yy=0 ; yy<n_mntY ; ++yy) {
                    float value = mnt->value(xx, yy);
                    if (value != mnt->NA())
                    {
                        if (densite->value(xx,yy) < min_density)
                        {
                            mnt->setValue(xx, yy, mnt->NA());
                        } else {
                            QList<float> neighbours = mnt->neighboursValues(xx, yy, _dist, false, CT_Image2D<float>::CM_DropCenter);
                            qSort(neighbours.begin(), neighbours.end());
                            int size_neighbours = neighbours.size();

                            if (size_neighbours > 0) {
                                int med_ind = (int) (size_neighbours/2);
                                float median = neighbours.at(med_ind);
                                float val_test = std::min(fabs(neighbours.last() - median), fabs(neighbours.first()  - median));

                                if (fabs(value - median) > (val_test*5)) {
                                    mnt->setValue(xx, yy, mnt->NA());
                                } else {
                                    val_test = neighbours.first();

                                    if (fabs(value - val_test) > (_dist * _gridsize))
                                    {
                                        mnt->setValue(xx, yy, mnt->NA());
                                    }
                                }
                            } else {
                                mnt->setValue(xx, yy, mnt->NA());
                            }
                        }
                    }

                    // Progression Etape 3
                    setProgress(((double)(cpt++)/(double)tab_mnt_size)*20 + 40);
                }
            }

            setProgress(60);
            PS_LOG->addMessage(LogInterface::info, LogInterface::step, tr("Test de cohérence de voisinnage terminé"));
        }

        CT_PointCloudIndexVector *tab_sol_index = NULL;
        CT_PointCloudIndexVector *tab_veg_index = NULL;


        // creation des scenes pour les points sol et vegetation
        CT_ResultItemIterator it4(inResult, this, DEF_SearchInScene);
        int nscenes = 1;
        while (!isStopped() && it4.hasNext())
        {
            const CT_Scene *scene = (CT_Scene*)it4.next();
            const CT_AbstractPointCloudIndex *pointCloudIndex = scene->getPointCloudIndex();

            CT_PointIterator itP(pointCloudIndex);
            while(itP.hasNext() && !isStopped())
            {
                const CT_Point &point = itP.next().currentPoint();
                size_t index = itP.currentGlobalIndex();

                float value = mnt->valueAtCoords(point(0), point(1)) + _soilwidth;

                if (value != mnt->NA() && point(2) < value) {
                    if (tab_sol_index == NULL)
                    {
                        tab_sol_index = new CT_PointCloudIndexVector();
                        tab_sol_index->setSortType(CT_AbstractCloudIndex::NotSorted);
                    }
                    tab_sol_index->addIndex(index);

                } else {
                    if (tab_veg_index == NULL)
                    {
                        tab_veg_index = new CT_PointCloudIndexVector();
                        tab_veg_index->setSortType(CT_AbstractCloudIndex::NotSorted);
                    }
                    tab_veg_index->addIndex(index);
                }
            }

            setProgress(80);

            size_t numberOfSoilPoints = 0;
            size_t numberOfVegetationPoints = 0;

            CT_StandardItemGroup *outGroupVegetation = new CT_StandardItemGroup(DEF_SearchOutGroupVegetation, outResultVegetation);
            outResultVegetation->addGroup(outGroupVegetation);

            if (tab_sol_index != NULL)
            {
                tab_sol_index->setSortType(CT_AbstractCloudIndex::SortedInAscendingOrder);
                numberOfSoilPoints = tab_sol_index->size();

                if (numberOfSoilPoints > 0)
                {
                    // creation de la scene sol
                    CT_Scene *outSceneSoil = new CT_Scene(DEF_SearchOutSceneSoil, outResultVegetation);
                    outSceneSoil->setPointCloudIndexRegistered(PS_REPOSITORY->registerPointCloudIndex(tab_sol_index));
                    outSceneSoil->updateBoundingBox();
                    outGroupVegetation->addItemDrawable(outSceneSoil);
                }
            }

            if (tab_veg_index != NULL)
            {
                tab_veg_index->setSortType(CT_AbstractCloudIndex::SortedInAscendingOrder);
                numberOfVegetationPoints = tab_veg_index->size();

                if (numberOfVegetationPoints > 0)
                {
                    // creation de la scene vegetation
                    CT_Scene *outSceneVegetation = new CT_Scene(DEF_SearchOutSceneVegetation, outResultVegetation);
                    outSceneVegetation->setPointCloudIndexRegistered(PS_REPOSITORY->registerPointCloudIndex(tab_veg_index));
                    outSceneVegetation->updateBoundingBox();
                    outGroupVegetation->addItemDrawable(outSceneVegetation);
                }
            }

            PS_LOG->addMessage(LogInterface::info, LogInterface::step, QString(tr("Scène %3 : Création des scènes sol (%1 points) et végétation (%2 points) terminée")).arg(numberOfSoilPoints).arg(numberOfVegetationPoints).arg(nscenes++));
            tab_sol_index = NULL;
            tab_veg_index = NULL;
        }

        // Progression Etape 4
        setProgress(90);

        // ajout du raster MNT
        CT_StandardItemGroup *outGroupMNT = new CT_StandardItemGroup(DEF_SearchOutGroupMNT, outResultMNT);
        outGroupMNT->addItemDrawable(mnt);
        outResultMNT->addGroup(outGroupMNT);
        mnt->computeMinMax();

        // ajout du raster Densité
        outGroupMNT->addItemDrawable(densite);
        densite->computeMinMax();

        for (size_t index = 0 ; index < densite->nCells() ; index++)
        {
            if (densite->valueAtIndex(index) == densite->NA()) {densite->setValueAtIndex(index, 0);}
        }

        setProgress(100);
    }
}
#endif
