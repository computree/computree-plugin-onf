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


#include "onf_stepextractplotbasedondtm.h"

#include "ct_global/ct_context.h"

#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/inModel/ct_inresultmodelgroup.h"
#include "ct_result/model/outModel/ct_outresultmodelgroup.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"

#include "ct_itemdrawable/abstract/ct_abstractitemdrawablewithpointcloud.h"
#include "ct_itemdrawable/ct_referencepoint.h"
#include "ct_itemdrawable/ct_grid2dxy.h"
#include "ct_result/ct_resultgroup.h"
#include "ct_itemdrawable/abstract/ct_abstractitemgroup.h"
#include "ct_itemdrawable/ct_scene.h"

#include "ct_pointcloudindex/ct_pointcloudindexvector.h"
#include "ct_iterator/ct_pointiterator.h"
#include "ct_view/ct_stepconfigurabledialog.h"

#include "qdebug.h"
#include <limits>


#define DEF_SearchInResult  "ires"
#define DEF_SearchInGroup   "igrp"
#define DEF_SearchInScene   "isc"

#define DEF_SearchInMNTResult   "mntres"
#define DEF_SearchInMNTGroup    "mntgrp"
#define DEF_SearchInMNT         "mntitem"

#define DEF_SearchOutMNTResult       "rmnt"

ONF_StepExtractPlotBasedOnDTM::ONF_StepExtractPlotBasedOnDTM(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _x   = 0.00;
    _y = 0.00;
    _radiusmin = 0.00;
    _radius = 17.00;
    _azbegin = 0;
    _azend = 400;
    _zmin = 1.0;
    _zmax = 1.6;
}

QString ONF_StepExtractPlotBasedOnDTM::getStepDescription() const
{
    return tr("Extraction d'une placette // MNT (cpy)");
}

QString ONF_StepExtractPlotBasedOnDTM::getStepDetailledDescription() const
{
    return tr("Cette étape permet d'extraire les points de la scène d'entrée contenus dans une placette circulaire.<br>"
              "On définit dans les paramètres son <b>centre (X,Y)</b>, son <b>rayon</b> (maximal), le <b>niveau Z minimum</b> et le <b>niveau Z maximum</b>.<br>"
              "Si on définit un <b>rayon de début de placette</b>, cela permet d'obtenir une placette annulaire.<br>"
              "On peut également définir un <b>azimut de début</b> et un <b>azimut de fin</b>, pour obtenir un secteur.<br>"
              "Cette étape fonctionne comme <em>ONF_StepExtractPlot</em>, mais les niveaux Z sont spécifiés sous forme de hauteurs par rapport au MNT de référence choisi.");
}

CT_VirtualAbstractStep* ONF_StepExtractPlotBasedOnDTM::createNewInstance(CT_StepInitializeData &dataInit)
{
    // cree une copie de cette etape
    return new ONF_StepExtractPlotBasedOnDTM(dataInit);
}

//////////////////// PROTECTED //////////////////

void ONF_StepExtractPlotBasedOnDTM::createInResultModelListProtected()
{
    CT_InResultModelGroupToCopy *resultMNTModel = createNewInResultModelForCopy(DEF_SearchInMNTResult, tr("MNT (Raster)"), "", true);
    resultMNTModel->setZeroOrMoreRootGroup();
    resultMNTModel->addGroupModel("", DEF_SearchInMNTGroup);
    resultMNTModel->addItemModel(DEF_SearchInMNTGroup, DEF_SearchInMNT, CT_Grid2DXY<double>::staticGetType(), tr("Modèle Numérique de Terrain"));

    CT_InResultModelGroupToCopy *resultModel = createNewInResultModelForCopy(DEF_SearchInResult, tr("Scène(s)"));
    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_SearchInGroup);
    resultModel->addItemModel(DEF_SearchInGroup, DEF_SearchInScene, CT_Scene::staticGetType(), tr("Scène"));
}

void ONF_StepExtractPlotBasedOnDTM::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addDouble(tr("Coordonnee X du centre de la placette :"), "m", -1000, 1000, 2, _x);
    configDialog->addDouble(tr("Coordonnee Y du centre de la placette :"), "m", -1000, 1000, 2, _y);
    configDialog->addDouble(tr("Rayon de debut de la placette"), "m", 0, 1000, 2, _radiusmin);
    configDialog->addDouble(tr("Rayon de la placette :"), "m", 1, 1000, 2, _radius);
    configDialog->addDouble(tr("Azimut debut (Nord = axe Y) :"), "Grades", 0, 400, 2, _azbegin);
    configDialog->addDouble(tr("Azimut fin (Nord = axe Y) :"), "Grades", 0, 400, 2, _azend);
    configDialog->addDouble(tr("H minimum :"), "m", -10000, 10000, 2, _zmin);
    configDialog->addDouble(tr("H maximum :"), "m", -10000, 10000, 2, _zmax);
}

void ONF_StepExtractPlotBasedOnDTM::createOutResultModelListProtected()
{
    createNewOutResultModelToCopy(DEF_SearchInMNTResult);

    CT_OutResultModelGroupToCopyPossibilities *res = createNewOutResultModelToCopy(DEF_SearchInResult);
    res->addItemModel(DEF_SearchInGroup, _outSceneModel, new CT_Scene(), tr("Scène extraite"));
}

void ONF_StepExtractPlotBasedOnDTM::compute()
{
    // Récupération du résultat de sortie
    CT_ResultGroup* outMNTResult = getOutResultList().at(0);
    CT_ResultGroup *outResult = getOutResultList().at(1);

    CT_ResultItemIterator it(outMNTResult, this, DEF_SearchInMNT);
    if(it.hasNext())
    {
        CT_Grid2DXY<double>* mnt = (CT_Grid2DXY<double>*) it.next();

        if (mnt != NULL)
        {        
            CT_ResultGroupIterator itsc(outResult, this, DEF_SearchInGroup);
            while(!isStopped() && itsc.hasNext())
            {
                CT_StandardItemGroup *group = (CT_StandardItemGroup*)itsc.next();
                const CT_Scene *in_scene = (const CT_Scene*)group->firstItemByINModelName(this, DEF_SearchInScene);

                if(in_scene != NULL)
                {
                    const CT_AbstractPointCloudIndex *pointCloudIndex = in_scene->getPointCloudIndex();
                    size_t n_points = pointCloudIndex->size();

                    PS_LOG->addMessage(LogInterface::info, LogInterface::step, QString(tr("La scène d'entrée comporte %1 points.")).arg(n_points));

                    CT_PointCloudIndexVector *resPointCloudIndex = new CT_PointCloudIndexVector();

                    qDebug() << " le nombre de points dans ma scene : " << n_points;

                    // Extraction des points de la placette
                    size_t i = 0;
                    double distance = 0;
                    double azimut = 0;
                    double asinx = 0;
                    double acosy = 0;
                    double xx = 0;
                    double yy = 0;

                    double xmin = std::numeric_limits<double>::max();
                    double ymin = std::numeric_limits<double>::max();
                    double zmin = std::numeric_limits<double>::max();

                    double xmax = -std::numeric_limits<double>::max();
                    double ymax = -std::numeric_limits<double>::max();
                    double zmax = -std::numeric_limits<double>::max();

                    CT_PointIterator itP(pointCloudIndex);
                    while(itP.hasNext() && !isStopped())
                    {
                        const CT_Point &point = itP.next().currentPoint();
                        size_t index = itP.currentGlobalIndex();

                        xx = point(0) - _x;
                        yy = point(1) - _y;

                        double hauteur;
                        double na = mnt->NA();
                        double zMNT = mnt->valueAtXY(point(0), point(1));
                        if (zMNT != na) {
                            hauteur = point(2) - zMNT;
                        } else {
                            hauteur = -std::numeric_limits<double>::max();
                        }

                        // Calcul de l'azimut du point par rapport au centre de la placette extraite
                        // Le nord est place dans la direction de l'axe Y
                        distance = sqrt(xx*xx + yy*yy);
                        asinx = asin(xx/distance);
                        acosy = acos(yy/distance);

                        if (asinx>=0) {
                            azimut = acosy;
                        } else {
                            azimut = 2*M_PI-acosy;
                        }

                        // Conversion en grades 0-400
                        azimut = azimut/(2*M_PI)*400;


                        if (distance <= _radius && distance >= _radiusmin) {
                            if (hauteur >= _zmin && hauteur <= _zmax) {


                                if (_azbegin <= _azend) {
                                    if (azimut >= _azbegin && azimut <= _azend)
                                    {
                                        resPointCloudIndex->addIndex(index);

                                        if (point(0)<xmin) {xmin = point(0);}
                                        if (point(0)>xmax) {xmax = point(0);}
                                        if (point(1)<ymin) {ymin = point(1);}
                                        if (point(1)>ymax) {ymax = point(1);}
                                        if (point(2)<zmin) {zmin = point(2);}
                                        if (point(2)>zmax) {zmax = point(2);}
                                    }
                                } else {
                                    if ((azimut >= _azbegin && azimut <= 400) || (azimut >= 0 && azimut <= _azend))
                                    {

                                        resPointCloudIndex->addIndex(index);

                                        if (point(0)<xmin) {xmin = point(0);}
                                        if (point(0)>xmax) {xmax = point(0);}
                                        if (point(1)<ymin) {ymin = point(1);}
                                        if (point(1)>ymax) {ymax = point(1);}
                                        if (point(2)<zmin) {zmin = point(2);}
                                        if (point(2)>zmax) {zmax = point(2);}
                                    }
                                }
                            }
                        }

                        // progres de 0 à 100
                        setProgress(100.0*i/n_points);
                        ++i;
                    }

                    if (resPointCloudIndex->size() > 0)
                    {
                        // creation et ajout de la scene
                        CT_Scene *outScene = new CT_Scene(_outSceneModel.completeName(), outResult);

                        outScene->setPointCloudIndexRegistered(PS_REPOSITORY->registerPointCloudIndex(resPointCloudIndex));
                        outScene->setBoundingBox(xmin,ymin,zmin,xmax,ymax,zmax);
                        group->addItemDrawable(outScene);

                        PS_LOG->addMessage(LogInterface::info, LogInterface::step, QString(tr("La scène extraite comporte %1 points.")).arg(outScene->getPointCloudIndex()->size()));

                    } else {
                        delete resPointCloudIndex;
                        PS_LOG->addMessage(LogInterface::info, LogInterface::step, tr("Aucun point n'est dans l'emprise choisie"));

                    }
                }
            }
        }
    }
}
