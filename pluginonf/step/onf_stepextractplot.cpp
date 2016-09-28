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


#include "onf_stepextractplot.h"

#include "ct_global/ct_context.h"

#include "ct_view/ct_stepconfigurabledialog.h"
#include "ct_result/model/inModel/ct_inresultmodelgroup.h"
#include "ct_result/model/outModel/ct_outresultmodelgroup.h"

#include "ct_result/ct_resultgroup.h"
#include "ct_itemdrawable/ct_scene.h"

#include "ct_pointcloudindex/ct_pointcloudindexvector.h"
#include "ct_iterator/ct_pointiterator.h"
#include "ct_view/ct_buttongroup.h"


#include <math.h>
#include <iostream>
#include <QList>
#include <limits>

#define DEF_SearchInResult "rin"
#define DEF_SearchInGroup   "gin"
#define DEF_SearchInScene   "scin"

#define DEF_SearchOutResult "rout"
#define DEF_SearchOutGroup  "gout"
#define DEF_SearchOutScene  "scout"

ONF_StepExtractPlot::ONF_StepExtractPlot(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _x   = 0.00;
    _y = 0.00;
    _radiusmin = 0.00;
    _radius = 17.00;
    _azbegin = 0;
    _azend = 400;
    _zmin = -10000;
    _zmax = 10000;
}

QString ONF_StepExtractPlot::getStepDescription() const
{
    return tr("Extraire une Placette circulaire");
}

QString ONF_StepExtractPlot::getStepDetailledDescription() const
{
    return tr("Cette étape permet d'extraire les points de la scène d'entrée contenus dans une placette circulaire.<br>"
              "On définit dans les paramètres son <b>centre (X,Y)</b>, son <b>rayon</b> (maximal), le <b>niveau Z minimum</b> et le <b>niveau Z maximum</b>.<br>"
              "Si on définit un <b>rayon de début de placette</b>, cela permet d'obtenir une placette annulaire.<br>"
              "On peut également définir un <b>azimut de début</b> et un <b>azimut de fin</b>, pour obtenir un secteur.");
}

CT_VirtualAbstractStep* ONF_StepExtractPlot::createNewInstance(CT_StepInitializeData &dataInit)
{
    // cree une copie de cette etape
    return new ONF_StepExtractPlot(dataInit);
}

//////////////////// PROTECTED //////////////////

void ONF_StepExtractPlot::createInResultModelListProtected()
{
    CT_InResultModelGroup *resultModel = createNewInResultModel(DEF_SearchInResult, tr("Scène(s)"));

    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_SearchInGroup);
    resultModel->addItemModel(DEF_SearchInGroup, DEF_SearchInScene, CT_Scene::staticGetType(), tr("Scène"));
}

// Création et affiliation des modèles OUT
void ONF_StepExtractPlot::createOutResultModelListProtected()
{    
    CT_OutResultModelGroup *resultModel = createNewOutResultModel(DEF_SearchOutResult, tr("Scène(s) extraites"));

    resultModel->setRootGroup(DEF_SearchOutGroup);
    resultModel->addItemModel(DEF_SearchOutGroup, DEF_SearchOutScene, new CT_Scene(), tr("Scène extraite"));
}

void ONF_StepExtractPlot::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addDouble(tr("Coordonnée X du centre de la placette :"), "m", -1e+10, 1e+10, 4, _x);
    configDialog->addDouble(tr("Coordonnée Y du centre de la placette :"), "m", -1e+10, 1e+10, 4, _y);
    configDialog->addDouble(tr("Rayon de début de la placette :"), "m", 0, 1e+10, 4, _radiusmin);
    configDialog->addDouble(tr("Rayon de la placette (maximum) :"), "m", 0.01, 1e+10, 4, _radius);
    configDialog->addDouble(tr("Azimut début (Nord = axe Y) :"), tr("Grades"), 0, 400, 4, _azbegin);
    configDialog->addDouble(tr("Azimut fin (Nord = axe Y) :"), tr("Grades"), 0, 400, 4, _azend);
    configDialog->addDouble(tr("Niveau Z minimum :"), "m", -1e+10, 1e+10, 4, _zmin);
    configDialog->addDouble(tr("Niveau Z maximum :"), "m", -1e+10, 1e+10, 4, _zmax);
}

void ONF_StepExtractPlot::compute()
{
    // récupération du résultats IN et OUT
    CT_ResultGroup *inResult = getInputResults().first();
    CT_ResultGroup *outResult = getOutResultList().first();

    CT_ResultItemIterator it(inResult, this, DEF_SearchInScene);
    while(!isStopped() && it.hasNext())
    {
        const CT_Scene *in_scene = (CT_Scene*) it.next();
        const CT_AbstractPointCloudIndex *pointCloudIndex = in_scene->getPointCloudIndex();
        size_t n_points = pointCloudIndex->size();

        PS_LOG->addMessage(LogInterface::info, LogInterface::step, QString(tr("La scène d'entrée comporte %1 points.")).arg(n_points));

        CT_PointCloudIndexVector *resPointCloudIndex = new CT_PointCloudIndexVector();
        resPointCloudIndex->setSortType(CT_PointCloudIndexVector::NotSorted);

        std::cout << " le nombre de points dans ma scene : " << n_points;

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
        while(itP.hasNext() && (!isStopped()))
        {
            const CT_Point &point = itP.next().currentPoint();
            size_t index = itP.currentGlobalIndex();

            xx = point(0) - _x;
            yy = point(1) - _y;

            // Calcul de l'azimut du point par rapport au centre de la placette extraite
            // Le nord est place dans la direction de l'axe Y
            distance = sqrt(xx*xx + yy*yy);

            if(distance != 0) {
                asinx = asin(xx/distance);
                acosy = acos(yy/distance);
            } else {
                asinx = 0;
                acosy = 0;
            }

            if (asinx>=0) {
                azimut = acosy;
            } else {
                azimut = 2*M_PI-acosy;
            }

            // Conversion en grades 0-400
            azimut = azimut/(2*M_PI)*400;


            if (distance <= _radius && distance >= _radiusmin) {
                if (point(2) >= _zmin && point(2) <= _zmax) {


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
            resPointCloudIndex->setSortType(CT_PointCloudIndexVector::SortedInAscendingOrder);

            // creation du groupe
            CT_StandardItemGroup *outGroup = new CT_StandardItemGroup(DEF_SearchOutGroup, outResult);

            // creation et ajout de la scene
            CT_Scene *outScene = new CT_Scene(DEF_SearchOutScene, outResult);

            outScene->setBoundingBox(xmin,ymin,zmin, xmax,ymax,zmax);
            outScene->setPointCloudIndexRegistered(PS_REPOSITORY->registerPointCloudIndex(resPointCloudIndex));
            outGroup->addItemDrawable(outScene);

            // ajout au résultat
            outResult->addGroup(outGroup);

            PS_LOG->addMessage(LogInterface::info, LogInterface::step, QString(tr("La scène extraite comporte %1 points.")).arg(outScene->getPointCloudIndex()->size()));
        } else {
            delete resPointCloudIndex;
            PS_LOG->addMessage(LogInterface::info, LogInterface::step, tr("Aucun point n'est dans l'emprise choisie"));
        }
    }

}
