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


#include "onf_stepremoveuppernoise.h"

#ifdef USE_OPENCV

#include "ct_global/ct_context.h" // pour accéder au PS_LOG et au PS_REPOSITORY

#include "ct_itemdrawable/model/outModel/ct_outstdgroupmodel.h"
#include "ct_itemdrawable/model/inModel/ct_instdsingularitemmodel.h"
#include "ct_itemdrawable/model/outModel/ct_outstdsingularitemmodel.h"

#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/ct_outresultmodelgroupcopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"

#include "ct_iterator/ct_resultgroupiterator.h"

#include "ct_result/ct_resultgroup.h"
#include "ct_itemdrawable/ct_scene.h"
#include "ct_itemdrawable/ct_grid3d_sparse.h"
#include "ct_itemdrawable/ct_image2d.h"

#include "ct_pointcloudindex/ct_pointcloudindexvector.h"
#include "ct_view/ct_stepconfigurabledialog.h"

#include "ct_model/tools/ct_modelsearchhelper.h"

#include "ct_iterator/ct_pointiterator.h"
#include "ct_accessor/ct_pointaccessor.h"

#include <math.h>
#include <iostream>
#include <QList>
#include <limits>

#define DEF_SearchInResult "r"
#define DEF_SearchInScene   "sc"
#define DEF_SearchInGroup "grp"


ONF_StepRemoveUpperNoise::ONF_StepRemoveUpperNoise(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _resolution = 2.0;
    _threshold = 1;
    _thresholdValid = 100;
    _length = 3.0;
}

QString ONF_StepRemoveUpperNoise::getStepDescription() const
{
    return tr("Remove upper noise points");
}

QString ONF_StepRemoveUpperNoise::getStepDetailledDescription() const
{
    return tr("TO DO");
}

CT_VirtualAbstractStep* ONF_StepRemoveUpperNoise::createNewInstance(CT_StepInitializeData &dataInit)
{
    // cree une copie de cette etape
    return new ONF_StepRemoveUpperNoise(dataInit);
}

//////////////////// PROTECTED //////////////////

void ONF_StepRemoveUpperNoise::createInResultModelListProtected()
{
    CT_InResultModelGroupToCopy *resultModel = createNewInResultModelForCopy(DEF_SearchInResult, tr("Scène à débruiter"), tr("Par exemple pour des scènes où les filtres matériels sont désactivés"));
    resultModel->setZeroOrMoreRootGroup();
    //resultModel->addGroupModel("", DEF_SearchInGroup);
    resultModel->addGroupModel("", DEF_SearchInGroup, CT_StandardItemGroup::staticGetType(), tr("Groupe contenant la scène"));
    resultModel->addItemModel(DEF_SearchInGroup, DEF_SearchInScene, CT_AbstractItemDrawableWithPointCloud::staticGetType(), tr("Scène bruitée"));
}

// Création et affiliation des modèles OUT
void ONF_StepRemoveUpperNoise::createOutResultModelListProtected()
{
    CT_OutResultModelGroupToCopyPossibilities *resultModel = createNewOutResultModelToCopy(DEF_SearchInResult);

    if(resultModel != NULL)
    {
        resultModel->addItemModel(DEF_SearchInGroup, _outScene_ModelName, new CT_Scene(), tr("Scène réduite"));
    }
}

void ONF_StepRemoveUpperNoise::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addDouble(tr("Grid resolution:"), "m", 0, 10000, 2, _resolution);
    configDialog->addInt(tr("Minimum number of points for a filled cell:"), "nb pts", 1, 1000000, _threshold);
    configDialog->addInt(tr("Minimum number of points for a valid cell:"), "nb pts", 1, 1000000, _thresholdValid);
    configDialog->addDouble(tr("Maximum gap length:"), "m", 0, 10000, 2, _length);
}

void ONF_StepRemoveUpperNoise::compute()
{

    // Récupération de la liste des résultats de sortie (en l'occurence la copie du résultat d'entrée)
    const QList<CT_ResultGroup*> &outResList = getOutResultList();
    // Récupération de la copie en elle même, au rang 0 de la liste précédente
    CT_ResultGroup *outResult = outResList.first();

    // Création de l'itérateur permettant de parcourir tous les groupes correspondants au niveau DEF_SearchInGroup coché
    CT_ResultGroupIterator itR(outResult, this, DEF_SearchInGroup);
    while(itR.hasNext() && !isStopped())
    {
        // Le groupe suivant
        CT_StandardItemGroup* grp = (CT_StandardItemGroup*) itR.next();

        // Au sein de ce groupe on récupère le nuage de points correpondant au niveau DEF_SearchInScene coché
        CT_AbstractItemDrawableWithPointCloud *in_scene = (CT_AbstractItemDrawableWithPointCloud*)grp->firstItemByINModelName(this, DEF_SearchInScene);

        if (in_scene != NULL) // on vérifie que ça existe
        {
            // Réupération de la liste de points du nuage
            const CT_AbstractPointCloudIndex* pointCloudIndexVector = in_scene->getPointCloudIndex();

            // Création d'un itérateur sur les points du nuage
            CT_PointIterator itP(pointCloudIndexVector);
            size_t n_points = itP.size(); // nombre de points du nuage

            PS_LOG->addMessage(LogInterface::info, LogInterface::step, QString(tr("La scène d'entrée comporte %1 points.")).arg(n_points));

            CT_Grid3D_Sparse<int>* densityGrd = CT_Grid3D_Sparse<int>::createGrid3DFromXYZCoords(NULL, NULL, in_scene->minX(), in_scene->minY(), in_scene->minZ(), in_scene->maxX()+_resolution, in_scene->maxY()+_resolution, in_scene->maxZ()+_resolution, _resolution, -1, 0);

            // Boucle sur les poi;ts
            int i = 0;
            while(itP.hasNext() && !isStopped())
            {
                itP.next(); // point suivant
                const CT_Point &point = itP.currentPoint(); // accès aux coordonnées du points, sous forme de référence constante

                densityGrd->addValueAtXYZ(point(0), point(1), point(2), 1);
                ++i;
                // Mise à jour de la barre de progression
                setProgress(50 * i / n_points);
            }

            CT_Image2D<float>* safeMaxHeight = new CT_Image2D<float>(NULL, NULL, densityGrd->minX(), densityGrd->minY(), densityGrd->xdim(), densityGrd->ydim(), _resolution, 0, NAN, in_scene->minZ());

            for (size_t x = 0 ; x < densityGrd->xdim() ; x++)
            {
                for (size_t y = 0 ; y < densityGrd->ydim() ; y++)
                {
                    size_t safez = 0;
                    for (size_t z = 0 ; z < densityGrd->zdim() ; z++)
                    {
                        int density = densityGrd->value(x, y, z);
                        if (density >= _thresholdValid) {safez = z;}
                        //if (safez == 0 && density > _threshold) {safez = z;}
                    }

                    float safeHeight = densityGrd->getCellCenterZ(safez) + _resolution / 2.0;

                    for (size_t z = safez + 1 ; z < densityGrd->zdim() ; z++)
                    {
                        int density = densityGrd->value(x, y, z);
                        safeHeight = densityGrd->getCellCenterZ(safez) + _resolution / 2.0;
                        float newHeight = densityGrd->getCellCenterZ(z) + _resolution / 2.0;

                        if ((newHeight - safeHeight) < _length && density >= _threshold)
                        {
                            safez = z;
                            safeHeight = newHeight;
                        }
                    }

                    safeMaxHeight->setValueAtCoords(densityGrd->getCellCenterX(x), densityGrd->getCellCenterY(y), safeHeight);
                }
            }


            // Création du nuage de points pour la scène de sortie
            CT_PointCloudIndexVector *resPointCloudIndex = new CT_PointCloudIndexVector();
            resPointCloudIndex->setSortType(CT_AbstractCloudIndex::NotSorted);

            i = 0;
            CT_PointIterator itP2(pointCloudIndexVector);
            while(itP2.hasNext() && !isStopped())
            {
                itP2.next(); // point suivant
                const CT_Point &point = itP2.currentPoint(); // accès aux coordonnées du points, sous forme de référence constante
                size_t pointIndex = itP2.cIndex(); // Accès à l'indice du points (dans le dépôt)

                if (point(2) <= safeMaxHeight->valueAtCoords(point(0), point(1)))
                {
                    resPointCloudIndex->addIndex(pointIndex);
                }

                ++i;
                // Mise à jour de la barre de progression
                setProgress(50 + 40 * i / n_points);
            }

            delete densityGrd;
            delete safeMaxHeight;

            if (resPointCloudIndex->size() > 0)
            {
                // Optimisation : on réactive de tri à la volée (du coup déclenche un tri complet)
                resPointCloudIndex->setSortType(CT_AbstractCloudIndex::SortedInAscendingOrder);

                // Création de l'item scène et ajout au résultat
                // Au passage le nuage d'indices de points est enregistré auprès du dépôt
                CT_Scene *outScene = new CT_Scene(_outScene_ModelName.completeName(), outResult, PS_REPOSITORY->registerPointCloudIndex(resPointCloudIndex));
                outScene->updateBoundingBox();

                grp->addItemDrawable(outScene);

                PS_LOG->addMessage(LogInterface::info, LogInterface::step, QString(tr("La scène de densité réduite comporte %1 points.")).arg(outScene->getPointCloudIndex()->size()));
            } else {

                PS_LOG->addMessage(LogInterface::info, LogInterface::step, tr("Aucun point conservé pour cette scène"));
            }

        }

        setProgress(99);
    }

}

#endif
