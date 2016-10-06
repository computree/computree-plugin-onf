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

#include "onf_stepconverttintodtm.h"


#ifdef USE_OPENCV
#include "ct_global/ct_context.h"

#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/ct_outresultmodelgroupcopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"

#include "ct_result/ct_resultgroup.h"

#include "ct_itemdrawable/ct_scene.h"
#include "ct_itemdrawable/ct_triangulation2d.h"
#include "ct_itemdrawable/abstract/ct_abstractareashape2d.h"
#include "ct_pointcloudindex/ct_pointcloudindexvector.h"
#include "ct_iterator/ct_pointiterator.h"
#include "ct_view/ct_stepconfigurabledialog.h"

#include "ct_itemdrawable/tools/image2dtools/ct_image2dnaturalneighboursinterpolator.h"


#define DEF_SearchInResult  "ires"
#define DEF_SearchInGroup   "igrp"
#define DEF_SearchInTIN     "TIN"


ONF_StepConvertTINtoDTM::ONF_StepConvertTINtoDTM(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _gridsize   = 0.5;

}

QString ONF_StepConvertTINtoDTM::getStepDescription() const
{
    return tr("Convertir un TIN en MNT");
}

QString ONF_StepConvertTINtoDTM::getStepDetailledDescription() const
{
    return tr("TO DO");
}

CT_VirtualAbstractStep* ONF_StepConvertTINtoDTM::createNewInstance(CT_StepInitializeData &dataInit)
{
    // cree une copie de cette etape
    return new ONF_StepConvertTINtoDTM(dataInit);
}

/////////////////////// PROTECTED ///////////////////////

void ONF_StepConvertTINtoDTM::createInResultModelListProtected()
{
    CT_InResultModelGroupToCopy *resultModel = createNewInResultModelForCopy(DEF_SearchInResult, tr("TIN"));

    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_SearchInGroup);
    resultModel->addItemModel(DEF_SearchInGroup, DEF_SearchInTIN, CT_Triangulation2D::staticGetType(), tr("TIN"));
}

void ONF_StepConvertTINtoDTM::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addDouble(tr("Résolution de la grille :"), "cm", 1, 1000, 0, _gridsize, 100);
}

void ONF_StepConvertTINtoDTM::createOutResultModelListProtected()
{
    CT_OutResultModelGroupToCopyPossibilities *resultModel = createNewOutResultModelToCopy(DEF_SearchInResult);

    if(resultModel != NULL)
        resultModel->addItemModel(DEF_SearchInGroup, _outDTMModelName, new CT_Image2D<float>(), tr("MNT"));
}

void ONF_StepConvertTINtoDTM::compute()
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
            const CT_Triangulation2D *tin = (const CT_Triangulation2D*)group->firstItemByINModelName(this, DEF_SearchInTIN);

            if (tin != NULL)
            {

                Eigen::Vector3d min, max;
                tin->getBoundingBox(min, max);

                CT_Image2D<float>* mnt = CT_Image2D<float>::createImage2DFromXYCoords(_outDTMModelName.completeName(), outResult, min(0), min(1), max(0), max(1), _gridsize, 0, -9999, -9999);

                CT_DelaunayTriangulation *triangulation = tin->getDelaunayT();

                CT_DelaunayTriangle* refTriangle = (CT_DelaunayTriangle*) triangulation->getRefTriangle();

                for (size_t xx = 0 ; xx < mnt->colDim() ; xx++)
                {
                    for (size_t yy = 0 ; yy < mnt->linDim() ; yy++)
                    {
                        Eigen::Vector3d coord;
                        mnt->getCellCenterCoordinates(xx, yy, coord);

                        double z = -9999;
                        refTriangle = (CT_DelaunayTriangle*) triangulation->getZCoordForXY(coord(0), coord(1), z, refTriangle);

                        if (refTriangle != NULL)
                        {
                            mnt->setValue(xx, yy, (float)z);
                        }
                    }
                }


                setProgress(90);
                PS_LOG->addMessage(LogInterface::info, LogInterface::step, tr("Convertion terminée"));

                // ajout du raster MNT
                group->addItemDrawable(mnt);
                mnt->computeMinMax();
            }

        }
    }
    setProgress(100);
}



#endif
