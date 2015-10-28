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


#include "onf_stepextractsoil04.h"


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
#define DEF_SearchOutResultSoil "rs"
#define DEF_SearchOutResultTriangulation "rt"
#define DEF_SearchOutResultMNT "rmnt"
#define DEF_SearchOutResultMNS "rmns"
#define DEF_SearchOutResultMNH "rmnh"
#define DEF_SearchOutResultDensite "rd"

#define DEF_SearchOutGroupVegetation  "gv"
#define DEF_SearchOutGroupSoil  "gs"
#define DEF_SearchOutGroupTriangulation  "gt"
#define DEF_SearchOutGroupMNT  "gmnt"
#define DEF_SearchOutGroupMNS  "gmns"
#define DEF_SearchOutGroupMNH  "gmnh"
#define DEF_SearchOutGroupDensite  "gd"

#define DEF_SearchOutSceneVegetation  "scv"
#define DEF_SearchOutSceneSoil  "scs"
#define DEF_SearchOutTriangulation  "tri"
#define DEF_SearchOutMNT  "mnt"
#define DEF_SearchOutMNS  "mns"
#define DEF_SearchOutMNH  "mnh"
#define DEF_SearchOutDensite  "dens"



ONF_StepExtractSoil04::ONF_StepExtractSoil04(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _gridsize   = 0.5;
    _soilwidth = 0.32;
    _min_density = 200;
    _filter = true;
    _dist = 3;
    _interpol = true;
    _smooth = true;
    _smoothDist = 2;
}

QString ONF_StepExtractSoil04::getStepDescription() const
{
    return tr("Séparation sol / végétation -> MNT V04");
}

QString ONF_StepExtractSoil04::getStepDetailledDescription() const
{
    return tr("Cette étape permet de séparer les points Sol et Végétation, et de générer :"
              "<ul>"
              "<li>Le Modèle Numérique de Terrain (MNT)</li>"
              "<li>Le Modèle Numérique de Surface (MNS)</li>"
              "<li>Le Modèle Numérique de Hauteur (MNH)</li>"
              "</ul>"
              "<br>Etapes de l'extraction du sol et de la création du MNT :"
              "<ul>"
              "<li>Une grille Zmin est créée à la <b>résolution</b> spécifiée</li>"
              "<li>La densité de points situés entre Zmin et (Zmin + <b>épaisseur du sol</b>) est calculée pour chaque case</li>"
              "<li>La valeur NA est affectée à toute case dont la densité est inférieure à la <b>densité minimum</b></li>"
              "<li>Un test de cohérence des Zmin restants est réalisé pour chaque case sur le <b>voisinage</b> spécifié (nombre de cases). La valeur NA est affectée aux cases incohérentes</li>"
              "<li>Si l' <b>interpolation</b> est activée, les valeur NA sont remplacées par une moyenne des voisins natuels dans la grille (triangulation de Delaunay en 2D, fournie en sortie)</li>"
              "<li>Si le <b>lissage</b> est activé, chaque cellule est tranformée en la moyenne du k-voisinnage, avec k = <b>voisinnage de lissage</b></li>"
              "</ul>"
              "<br>Le MNT est la grille résultante (interpolée et/ou lissée selon les options cochées).<br>"
              "Le MNS est simplement une grille Zmax de la même <b>résolution</b>.<br>"
              "Le MNH est la soutraction MNS-MNT.<br>"
              "Les points Sol sont tous les points dont Z &lt; (hauteur MNT + <b>épaisseur du sol</b>).<br>"
              "Les points Végétation sont tous les points non classés sol.");
}

CT_VirtualAbstractStep* ONF_StepExtractSoil04::createNewInstance(CT_StepInitializeData &dataInit)
{
    // cree une copie de cette etape
    return new ONF_StepExtractSoil04(dataInit);
}

/////////////////////// PROTECTED ///////////////////////

void ONF_StepExtractSoil04::createInResultModelListProtected()
{  
    CT_InResultModelGroup *resultModel = createNewInResultModel(DEF_SearchInResult, tr("Scène(s)"));

    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_SearchInGroup);
    resultModel->addItemModel(DEF_SearchInGroup, DEF_SearchInScene, CT_Scene::staticGetType(), tr("Scène"));
}

void ONF_StepExtractSoil04::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addDouble(tr("Résolution de la grille :"), "cm", 1, 1000, 0, _gridsize, 100);
    configDialog->addDouble(tr("Epaisseur du sol :"), "cm", 1, 100, 0, _soilwidth, 100);
    configDialog->addEmpty();
    configDialog->addBool(tr("Filtrage selon la densité et le voisinnage"), "", "", _filter);
    configDialog->addDouble(tr("Densité minimum :"), "pts/m2", 1, 99999999, 2, _min_density);
    configDialog->addDouble(tr("Voisinage (points isolés) :"), "Cases", 1, 99999999, 0, _dist);
    configDialog->addEmpty();
    configDialog->addBool(tr("Interpolation"), "", "", _interpol);
    configDialog->addBool(tr("Lissage"), "", "", _smooth);
    configDialog->addDouble(tr("Voisinage de lissage :"), "Cases", 1, 99999999, 0, _smoothDist);
}

void ONF_StepExtractSoil04::createOutResultModelListProtected()
{
    CT_OutResultModelGroup *resultModel;

    resultModel = createNewOutResultModel(DEF_SearchOutResultVegetation, tr("Points végétation"));
    resultModel->setRootGroup(DEF_SearchOutGroupVegetation);
    resultModel->addItemModel(DEF_SearchOutGroupVegetation, DEF_SearchOutSceneVegetation, new CT_Scene(), tr("Scène végétation"));

    resultModel = createNewOutResultModel(DEF_SearchOutResultSoil, tr("Points sol"));
    resultModel->setRootGroup(DEF_SearchOutGroupSoil);
    resultModel->addItemModel(DEF_SearchOutGroupSoil, DEF_SearchOutSceneSoil, new CT_Scene(), tr("Scène sol"));

    resultModel = createNewOutResultModel(DEF_SearchOutResultTriangulation, tr("Triangulation 2D"));
    resultModel->setRootGroup(DEF_SearchOutGroupTriangulation);
    resultModel->addItemModel(DEF_SearchOutGroupTriangulation, DEF_SearchOutTriangulation, new CT_Triangulation2D(), tr("Triangulation 2D"));

    resultModel = createNewOutResultModel(DEF_SearchOutResultMNT, tr("Modèle Numérique de terrain"));
    resultModel->setRootGroup(DEF_SearchOutGroupMNT);
    resultModel->addItemModel(DEF_SearchOutGroupMNT, DEF_SearchOutMNT, new CT_Image2D<float>(), tr("MNT (Raster)"));

    resultModel = createNewOutResultModel(DEF_SearchOutResultMNS, tr("Modèle Numérique de Surface"));
    resultModel->setRootGroup(DEF_SearchOutGroupMNS);
    resultModel->addItemModel(DEF_SearchOutGroupMNS, DEF_SearchOutMNS, new CT_Image2D<float>(), tr("MNS (Raster)"));

    resultModel = createNewOutResultModel(DEF_SearchOutResultMNH, tr("Modèle Numérique de Hauteur"));
    resultModel->setRootGroup(DEF_SearchOutGroupMNH);
    resultModel->addItemModel(DEF_SearchOutGroupMNH, DEF_SearchOutMNH, new CT_Image2D<float>(), tr("MNH (Raster)"));

    resultModel = createNewOutResultModel(DEF_SearchOutResultDensite, tr("Densité de points sol"));
    resultModel->setRootGroup(DEF_SearchOutGroupDensite);
    resultModel->addItemModel(DEF_SearchOutGroupDensite, DEF_SearchOutDensite, new CT_Image2D<int>(), tr("Densité pts sol (Raster)"));
}

void ONF_StepExtractSoil04::compute()
{

    // recupere le resultat d'entree
    CT_ResultGroup *inResult = getInputResults().first();

    // recupere les resultats de sortie
    const QList<CT_ResultGroup*> &outResList = getOutResultList();

    // récupération des modéles out
    CT_ResultGroup *outResultVegetation =       outResList.at(0);
    CT_ResultGroup *outResultSoil =             outResList.at(1);
    CT_ResultGroup *outResultTriangulation =    outResList.at(2);
    CT_ResultGroup *outResultMNT =              outResList.at(3);
    CT_ResultGroup *outResultMNS =              outResList.at(4);
    CT_ResultGroup *outResultMNH =              outResList.at(5);
    CT_ResultGroup *outResultDensite =          outResList.at(6);

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
        CT_Image2D<float>* mns = new CT_Image2D<float>(DEF_SearchOutMNS, outResultMNS, minX, minY, n_mntX, n_mntY, _gridsize, maxZ, -9999, -9999);
        CT_Image2D<float>* mnh = new CT_Image2D<float>(DEF_SearchOutMNH, outResultMNH, minX, minY, n_mntX, n_mntY, _gridsize, maxZ + 1, -9999, -9999);
        CT_Image2D<int>* densite = new CT_Image2D<int>(DEF_SearchOutDensite, outResultDensite, minX, minY, n_mntX, n_mntY, _gridsize, minZ - 1, -9999, 0);

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
                mns->setMaxValueAtCoords(point(0), point(1), point(2));
            }
        }

        size_t cpt = 0;
        if (_filter)
        {
            // Progression Etape 1
            setProgress(10);
            PS_LOG->addMessage(LogInterface::info, LogInterface::step, tr("Grille Zmin et MNS créés"));

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
            setProgress(20);
            PS_LOG->addMessage(LogInterface::info, LogInterface::step, tr("Filtrage sur la densité terminé"));

            double min_density = _min_density * (_gridsize*_gridsize);


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
                    setProgress(((double)(cpt++)/(double)tab_mnt_size)*20 + 20);
                }
            }
        }

        PS_LOG->addMessage(LogInterface::info, LogInterface::step, tr("Test de cohérence de voisinnage terminé"));


        cpt = 0;
        CT_DelaunayT *delaunay;
        // interpolation de la grille MNT
        if (_interpol) {
            // Triangulation des points non interpoles du MNT
            delaunay = new CT_DelaunayT();
            cpt = 0;
            for (xx=0 ; xx<n_mntX ; ++xx) {
                for (yy=0 ; yy<n_mntY ; ++yy) {
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
                    setProgress(((double)(cpt++)/(double)tab_mnt_size)*10 + 40);
                }
            }

            PS_LOG->addMessage(LogInterface::info, LogInterface::step, tr("Triangulation des cases conservées terminée"));

            CT_Image2D<float>* MNTinterpol = (CT_Image2D<float>*) mnt->copy(DEF_SearchOutMNT, outResultMNT, CT_ResultCopyModeList() << CT_ResultCopyModeList::CopyItemDrawableReference);
            for (xx=0 ; xx<n_mntX ; ++xx) {
                for (yy=0 ; yy<n_mntY ; ++yy) {

                    float value = mnt->value(xx,yy);
                    float x = mnt->getCellCenterColCoord(xx);
                    float y = mnt->getCellCenterLinCoord(yy);

                    if (value == mnt->NA()) {
                        Eigen::Vector3d* pt = new Eigen::Vector3d();
                        (*pt)(0) = mnt->getCellCenterColCoord(xx);
                        (*pt)(1) = mnt->getCellCenterLinCoord(yy);
                        (*pt)(2) = value;

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

                    // Progression Etape 5
                    setProgress(((double)(cpt++)/(double)tab_mnt_size)*20 + 50);
                }
            }

            delete mnt;
            mnt = MNTinterpol;

            PS_LOG->addMessage(LogInterface::info, LogInterface::step, tr("Interpolation du MNT terminée"));
        }


        cpt = 0;
        // Lissage de la grille MNT
        if (_smooth) {
            CT_Image2D<float>* MNTsmoothed = (CT_Image2D<float>*) mnt->copy(DEF_SearchOutMNT, outResultMNT, CT_ResultCopyModeList() << CT_ResultCopyModeList::CopyItemDrawableReference);
            for (xx=0 ; xx<n_mntX ; ++xx) {
                for (yy=0 ; yy<n_mntY ; ++yy) {

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
                    setProgress(((double)(cpt++)/(double)tab_mnt_size)*20 + 50);
                }
            }

            delete mnt;
            mnt = MNTsmoothed;

            PS_LOG->addMessage(LogInterface::info, LogInterface::step, tr("Lissage du MNT terminé"));
        }

        // Création du MNH
        cpt = 0;
        for (xx=0 ; xx<n_mntX ; ++xx) {
            for (yy=0 ; yy<n_mntY ; ++yy) {

                float valueMNT = mnt->value(xx,yy);
                float valueMNS = mns->value(xx,yy);

                if ((valueMNT == mnt->NA()) || (valueMNS == mns->NA())) {
                    mnh->setValue(xx, yy, mnh->NA());

                } else {
                    float valueMNH = valueMNS - valueMNT;
                    if (valueMNH < 0) {valueMNH = mnh->NA();}
                    mnh->setValue(xx, yy, valueMNH);
                }
                // Progression Etape 6
                setProgress(((double)(cpt++)/(double)tab_mnt_size)*10 + 80);
            }
        }

        PS_LOG->addMessage(LogInterface::info, LogInterface::step, tr("Création du MNH terminée"));


        CT_PointCloudIndexVector *tab_sol_index = NULL;
        CT_PointCloudIndexVector *tab_veg_index = NULL;

        setProgress(90);

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

                if (point(2) < value) {
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

            setProgress(95);          

            size_t numberOfSoilPoints = 0;
            size_t numberOfVegetationPoints = 0;

            if (tab_sol_index != NULL)
            {
                tab_sol_index->setSortType(CT_AbstractCloudIndex::SortedInAscendingOrder);
                numberOfSoilPoints = tab_sol_index->size();

                if (numberOfSoilPoints > 0)
                {
                    // creation de la scene sol
                    CT_StandardItemGroup *outGroupSoil = new CT_StandardItemGroup(DEF_SearchOutGroupSoil, outResultSoil);
                    CT_Scene *outSceneSoil = new CT_Scene(DEF_SearchOutSceneSoil, outResultSoil);
                    outSceneSoil->setPointCloudIndexRegistered(PS_REPOSITORY->registerPointCloudIndex(tab_sol_index));
                    outSceneSoil->updateBoundingBox();
                    outGroupSoil->addItemDrawable(outSceneSoil);
                    outResultSoil->addGroup(outGroupSoil);
                }
            }

            if (tab_veg_index != NULL)
            {
                tab_veg_index->setSortType(CT_AbstractCloudIndex::SortedInAscendingOrder);
                numberOfVegetationPoints = tab_veg_index->size();

                if (numberOfVegetationPoints > 0)
                {
                    // creation de la scene vegetation
                    CT_StandardItemGroup *outGroupVegetation = new CT_StandardItemGroup(DEF_SearchOutGroupVegetation, outResultVegetation);
                    CT_Scene *outSceneVegetation = new CT_Scene(DEF_SearchOutSceneVegetation, outResultVegetation);
                    outSceneVegetation->setPointCloudIndexRegistered(PS_REPOSITORY->registerPointCloudIndex(tab_veg_index));
                    outSceneVegetation->updateBoundingBox();
                    outGroupVegetation->addItemDrawable(outSceneVegetation);
                    outResultVegetation->addGroup(outGroupVegetation);
                }
            }

            PS_LOG->addMessage(LogInterface::info, LogInterface::step, QString(tr("Scène %3 : Création des scènes sol (%1 points) et végétation (%2 points) terminée")).arg(numberOfSoilPoints).arg(numberOfVegetationPoints).arg(nscenes++));
            tab_sol_index = NULL;
            tab_veg_index = NULL;
        }

        // Progression Etape 7
        setProgress(99);

        if (_interpol) {
            // ajout de la triangulation
            CT_StandardItemGroup *outGroupTriangulation = new CT_StandardItemGroup(DEF_SearchOutGroupTriangulation, outResultTriangulation);
            CT_Triangulation2D *outTriangulation = new CT_Triangulation2D(DEF_SearchOutTriangulation, outResultTriangulation, delaunay);
            outGroupTriangulation->addItemDrawable(outTriangulation);
            outResultTriangulation->addGroup(outGroupTriangulation);
        }

        // ajout du raster MNT
        CT_StandardItemGroup *outGroupMNT = new CT_StandardItemGroup(DEF_SearchOutGroupMNT, outResultMNT);
        outGroupMNT->addItemDrawable(mnt);
        outResultMNT->addGroup(outGroupMNT);
        mnt->computeMinMax();


        // ajout du raster MNS
        CT_StandardItemGroup *outGroupMNS = new CT_StandardItemGroup(DEF_SearchOutGroupMNS, outResultMNS);
        outGroupMNS->addItemDrawable(mns);
        outResultMNS->addGroup(outGroupMNS);
        mns->computeMinMax();

        // ajout du raster MNH
        CT_StandardItemGroup *outGroupMNH = new CT_StandardItemGroup(DEF_SearchOutGroupMNH, outResultMNH);
        outGroupMNH->addItemDrawable(mnh);
        outResultMNH->addGroup(outGroupMNH);
        mnh->computeMinMax();               

        // ajout du raster Densité
        CT_StandardItemGroup *outGroupDensite = new CT_StandardItemGroup(DEF_SearchOutGroupDensite, outResultDensite);
        outGroupDensite->addItemDrawable(densite);
        outResultDensite->addGroup(outGroupDensite);
        densite->computeMinMax();

        for (size_t index = 0 ; index < mnt->nCells() ; index++)
        {
            if (mnt->valueAtIndex(index) == mnt->NA()) {mnt->setValueAtIndex(index, mnt->dataMin());}
            if (mns->valueAtIndex(index) == mns->NA()) {mns->setValueAtIndex(index, mnt->valueAtIndex(index));}
            if (mnh->valueAtIndex(index) == mnh->NA()) {mnh->setValueAtIndex(index, 0);}
            if (densite->valueAtIndex(index) == densite->NA()) {densite->setValueAtIndex(index, 0);}
        }

        setProgress(100);
    }
}
#endif
