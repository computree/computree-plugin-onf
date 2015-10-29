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

#include "onf_stepcomputelai2000data.h"

// Inclusion of in models
#include "ct_result/model/inModel/ct_inresultmodelgroup.h"

// Inclusion of out models
#include "ct_result/model/outModel/ct_outresultmodelgroup.h"

// Inclusion of standard result class
#include "ct_result/ct_resultgroup.h"
#include "ct_iterator/ct_pointiterator.h"

// Inclusion of used ItemDrawable classes
#include "ct_itemdrawable/ct_scene.h"
#include "itemdrawable/onf_lai2000data.h"

#include "ct_math/ct_sphericalline3d.h"

#include "ct_view/ct_stepconfigurabledialog.h"

// Alias for indexing in models
#define DEF_resultIn_r "r"
#define DEF_groupIn_g "g"
#define DEF_itemIn_sc "sc"

// Alias for indexing out models
#define DEF_resultOut_r "r"
#define DEF_groupOut_g "g"
#define DEF_itemOut_lai "lai"

// Constructor : initialization of parameters
ONF_StepComputeLAI2000Data::ONF_StepComputeLAI2000Data(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _resInRadians = 0.036*M_PI/180;
}

// Step description (tooltip of contextual menu)
QString ONF_StepComputeLAI2000Data::getStepDescription() const
{
    return tr("Calculer données LAI-2000");
}

QString ONF_StepComputeLAI2000Data::getStepDetailledDescription() const
{
    return tr("No detailled description for this step");
}

// Step copy method
CT_VirtualAbstractStep* ONF_StepComputeLAI2000Data::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new ONF_StepComputeLAI2000Data(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void ONF_StepComputeLAI2000Data::createInResultModelListProtected()
{
    CT_InResultModelGroup * resultModel = createNewInResultModel(DEF_resultIn_r, tr("Scène"));
    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_groupIn_g, CT_AbstractItemGroup::staticGetType(), tr("Groupe"));
    resultModel->addItemModel(DEF_groupIn_g, DEF_itemIn_sc, CT_Scene::staticGetType(), tr("Scène"));
}

// Creation and affiliation of OUT models
void ONF_StepComputeLAI2000Data::createOutResultModelListProtected()
{
    CT_OutResultModelGroup *resultModel = createNewOutResultModel(DEF_resultOut_r, tr("Données LAI2000"));

    resultModel->setRootGroup(DEF_groupOut_g,new CT_StandardItemGroup(), tr("Groupe"));
    resultModel->addItemModel(DEF_groupOut_g, DEF_itemOut_lai, new ONF_Lai2000Data(), tr("Données LAI2000"));
}

// Semi-automatic creation of step parameters DialogBox
void ONF_StepComputeLAI2000Data::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addDouble(tr("Résolution de scan"), "°", 0, 360, 5, _resInRadians, 180/M_PI);
}

void ONF_StepComputeLAI2000Data::compute()
{
    double nbAzimutalPoints = 2*M_PI/_resInRadians;

    double nbRays_1 = nbAzimutalPoints * (ANGLE1e - ANGLE1b)/_resInRadians;
    double nbRays_2 = nbAzimutalPoints * (ANGLE2e - ANGLE2b)/_resInRadians;
    double nbRays_3 = nbAzimutalPoints * (ANGLE3e - ANGLE3b)/_resInRadians;
    double nbRays_4 = nbAzimutalPoints * (ANGLE4e - ANGLE4b)/_resInRadians;
    double nbRays_5 = nbAzimutalPoints * (ANGLE5e - ANGLE5b)/_resInRadians;

    qDebug() << nbRays_1;
    qDebug() << nbRays_2;
    qDebug() << nbRays_3;
    qDebug() << nbRays_4;
    qDebug() << nbRays_5;

    int count_1 = 0;
    int count_2 = 0;
    int count_3 = 0;
    int count_4 = 0;
    int count_5 = 0;

    // Gets IN results
    CT_ResultGroup* resultIn_r = getInputResults().first();

    // Gets OUT results
    CT_ResultGroup* resultOut_r = getOutResultList().first();

    CT_ResultItemIterator it(resultIn_r, this, DEF_itemIn_sc);


    if (it.hasNext())
    {
        const CT_Scene* itemIn_sc = (const CT_Scene*) it.next();

        const CT_AbstractPointCloudIndex *pointCloudIndex = itemIn_sc->getPointCloudIndex();

        size_t n_points = pointCloudIndex->size();

        // Extraction des points de la placette
        size_t i=0;

        CT_PointIterator itP(pointCloudIndex);
        while(itP.hasNext() && !isStopped())
        {
            const CT_Point &point = itP.next().currentPoint();

            float phi;
            float theta;
            float length;
            CT_SphericalLine3D::convertToSphericalCoordinates(point(0), point(1), point(2), phi, theta, length);

            if (phi >= ANGLE1b && phi <= ANGLE1e) {
                count_1++;
            } else if (phi >= ANGLE2b && phi <= ANGLE2e) {
                count_2++;
            } else if (phi >= ANGLE3b && phi <= ANGLE3e) {
                count_3++;
            } else if (phi >= ANGLE4b && phi <= ANGLE4e) {
                count_4++;
            } else if (phi >= ANGLE5b && phi <= ANGLE5e) {
                count_5++;
            }

            setProgress(100*((float)i)/(float)n_points);
            ++i;
        }
    }


    qDebug() ;
    qDebug() << count_1;
    qDebug() << count_2;
    qDebug() << count_3;
    qDebug() << count_4;
    qDebug() << count_5;

    double t1 = (nbRays_1 - ((double) count_1)) / nbRays_1;
    double t2 = (nbRays_2 - ((double) count_2)) / nbRays_2;
    double t3 = (nbRays_3 - ((double) count_3)) / nbRays_3;
    double t4 = (nbRays_4 - ((double) count_4)) / nbRays_4;
    double t5 = (nbRays_5 - ((double) count_5)) / nbRays_5;

    // ------------------------------
    // Create OUT groups and items

    // ----------------------------------------------------------------------------
    // Works on the result corresponding to DEF_resultOut_r
    CT_StandardItemGroup* groupOut_g = new CT_StandardItemGroup(DEF_groupOut_g, resultOut_r);

    ONF_Lai2000Data* itemOut_lai = new ONF_Lai2000Data(DEF_itemOut_lai, resultOut_r, t1, t2, t3, t4, t5);
    groupOut_g->addItemDrawable(itemOut_lai);

    resultOut_r->addGroup(groupOut_g);

}
