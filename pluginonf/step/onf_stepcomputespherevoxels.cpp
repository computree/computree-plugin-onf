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

#include "onf_stepcomputespherevoxels.h"

// Inclusion of in models
#include "ct_result/model/inModel/ct_inresultmodelgroup.h"

// Inclusion of out models
#include "ct_result/model/outModel/ct_outresultmodelgroup.h"

// Inclusion of standard result class
#include "ct_result/ct_resultgroup.h"

// Inclusion of used ItemDrawable classes
#include "ct_itemdrawable/ct_scene.h"
#include "itemdrawable/onf_lvoxsphere.h"
#include "math.h"

#include "ct_view/ct_stepconfigurabledialog.h"

#include "ct_pointcloudindex/abstract/ct_abstractpointcloudindex.h"
#include "ct_iterator/ct_pointiterator.h"

#include "qfile.h"
#include "qtextstream.h"
#include "qdebug.h"

// Alias for indexing in models
#define DEF_resultIn_r "r"
#define DEF_groupIn_g "g"
#define DEF_itemIn_sc "sc"

// Alias for indexing out models
#define DEF_resultOut_r "ro"
#define DEF_groupOut_g "go"
#define DEF_itemOut_sp "sp"

// Constructor : initialization of parameters
ONF_StepComputeSphereVoxels::ONF_StepComputeSphereVoxels(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _res = 0.036/180*M_PI;
    _xscan = 0;
    _yscan = 0;
    _zscan = 0;
}

// Step description (tooltip of contextual menu)
QString ONF_StepComputeSphereVoxels::getStepDescription() const
{
    return tr("Calul d'une densité de points corrigée / sphère");
}

QString ONF_StepComputeSphereVoxels::getStepDetailledDescription() const
{
    return tr("No detailled description for this step");
}

// Step copy method
CT_VirtualAbstractStep* ONF_StepComputeSphereVoxels::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new ONF_StepComputeSphereVoxels(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void ONF_StepComputeSphereVoxels::createInResultModelListProtected()
{
    CT_InResultModelGroup * resultModel = createNewInResultModel(DEF_resultIn_r, tr("Scène"));
    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_groupIn_g, CT_AbstractItemGroup::staticGetType(), tr("Groupe"));
    resultModel->addItemModel(DEF_groupIn_g, DEF_itemIn_sc, CT_Scene::staticGetType(), tr("Scène"));
}

// Creation and affiliation of OUT models
void ONF_StepComputeSphereVoxels::createOutResultModelListProtected()
{
    CT_OutResultModelGroup *resultModel = createNewOutResultModel(DEF_resultOut_r, tr("Sphères"));

    resultModel->setRootGroup(DEF_groupOut_g,new CT_StandardItemGroup(), tr("Groupe"));
    resultModel->addItemModel(DEF_groupOut_g, DEF_itemOut_sp, new ONF_LvoxSphere(), tr("Sphère"));
}

// Semi-automatic creation of step parameters DialogBox
void ONF_StepComputeSphereVoxels::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addFileChoice("Fichier des sphères", CT_FileChoiceButton::OneExistingFile, "Fichier texte (*.txt)", _spheresFileName);
    configDialog->addDouble("Résolution du scanner", "°", 0, 360, 5, _res, 180/M_PI);
    configDialog->addDouble("Centre du scanner X", "°", -10000, 10000, 3, _xscan);
    configDialog->addDouble("Centre du scanner Y", "°", -10000, 10000, 3, _yscan);
    configDialog->addDouble("Centre du scanner Z", "°", -10000, 10000, 3, _zscan);
}

void ONF_StepComputeSphereVoxels::compute()
{
    CT_ResultGroup* resultIn_r = getInputResults().first();
    CT_ResultGroup* resultOut_r = getOutResultList().first();

    CT_ResultItemIterator it(resultIn_r, this, DEF_itemIn_sc);

    if(it.hasNext())
    {
        const CT_Scene *in_scene = (const CT_Scene*)it.next();

        const CT_AbstractPointCloudIndex *pointCloudIndex = in_scene->getPointCloudIndex();

        QList<ONF_LvoxSphere*> sphereList;

        QFile file(_spheresFileName.at(0));
        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream stream(&file);

            while (!stream.atEnd())
            {
                QString line = stream.readLine();
                QStringList readedLine = line.split("\t");

                bool okxs, okys, okzs, okrs, okre;

                QString name = readedLine.at(0);
                double xs = readedLine.at(1).toDouble(&okxs);
                double ys = readedLine.at(2).toDouble(&okys);
                double zs = readedLine.at(3).toDouble(&okzs);
                double rs = readedLine.at(4).toDouble(&okrs);
                double re = readedLine.at(5).toDouble(&okre);

                if (okxs && okys && okzs && okrs && okre)
                {
                    CT_StandardItemGroup* groupOut_g = new CT_StandardItemGroup(DEF_groupOut_g, resultOut_r);
                    ONF_LvoxSphere* sphere = new ONF_LvoxSphere(DEF_itemOut_sp, resultOut_r, name, xs, ys, zs, rs, re);
                    groupOut_g->addItemDrawable(sphere);
                    resultOut_r->addGroup(groupOut_g);

                    sphereList.append(sphere);
                } else {
                    qDebug() << "Impossible de céer une sphère à partir de la ligne suivante : ";
                    qDebug() << line;
                }
            }

            file.close();
        }

        int nSpheres = sphereList.size();

        // number of hits in the sphere
        int* ni = new int[nSpheres];
        // number of hits in the sphere, but excluded
        int* ne = new int[nSpheres];
        // number of befors hits
        int* nb = new int[nSpheres];
        // number of theorical rays
        double* nt = new double[nSpheres];

        // distance between scanner and sphere centers
        double* dist = new double[nSpheres];
        // angle between direction of the sphere center and sphere side
        double* alpha = new double[nSpheres];

        double rayArea = 2 * pow(_res, 2) / M_PI;


        for (int n = 0 ; n < nSpheres ; n++)
        {
            ni[n] = 0;
            nb[n] = 0;
            ne[n] = 0;

            ONF_LvoxSphere* sphere = sphereList.at(n);

            dist[n] = sqrt(pow(sphere->getXs() - _xscan, 2) + pow(sphere->getYs() - _yscan, 2) + pow(sphere->getZs() - _zscan, 2));

            alpha[n] = tan(sphere->getRs()/dist[n]);

            nt[n] = (1 - cos(alpha[n])) / rayArea;

        }

        CT_PointIterator itP(pointCloudIndex);

        while(itP.hasNext() && !isStopped())
        {
            const CT_Point &point = itP.next().currentPoint();

            double pointDist = sqrt(pow(point(0) - _xscan, 2) + pow(point(1) - _yscan, 2) + pow(point(2) - _zscan, 2));

            for (int n = 0 ; n < nSpheres ; n++)
            {
                ONF_LvoxSphere* sphere = sphereList.at(n);

                double distToSphereCenter = sqrt(pow(point(0) - sphere->getXs(), 2) + pow(point(1) - sphere->getYs(), 2) + pow(point(2) - sphere->getZs(), 2));

                // The point is in the sphere
                if (distToSphereCenter <= sphere->getRs())
                {
                    if (distToSphereCenter >= sphere->getRe())
                    {
                        ni[n]++;
                    } else {
                        ne[n]++;
                    }

                // The point is between the scanner center and the sphere
                } else if (pointDist < dist[n]) {
                    double scalarProduct = (point(0) - _xscan)*(sphere->getXs() - _xscan) + (point(1) - _yscan)*(sphere->getYs() - _yscan) + (point(2) - _zscan)*(sphere->getZs() - _zscan);
                    double pointAlpha = acos(scalarProduct / (pointDist * dist[n]));

                    // The point is in the solid angle of the sphere
                    if (pointAlpha <= alpha[n])
                    {
                        nb[n]++;
                    }
                }
            }

        }



        for (int n = 0 ; n < nSpheres ; n++)
        {
            ONF_LvoxSphere* sphere = sphereList.at(n);
            sphere->setNi(ni[n]);
            sphere->setNb(nb[n]);
            sphere->setNe(ne[n]);
            sphere->setNt((float) nt[n]);
        }

        delete ni;
        delete nb;
        delete nt;
        delete dist;
        delete alpha;
    }

}
