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

#include "onf_stepconvertfloatimagetoqint32.h"

#ifdef USE_OPENCV
#include "ct_itemdrawable/ct_image2d.h"
#include "ct_itemdrawable/tools/iterator/ct_groupiterator.h"
#include "ct_result/ct_resultgroup.h"
#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"

// Alias for indexing models
#define DEFin_res "res"
#define DEFin_grp "grp"
#define DEFin_image "image"



// Constructor : initialization of parameters
ONF_StepConvertFloatImageToqint32::ONF_StepConvertFloatImageToqint32(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
}

// Step description (tooltip of contextual menu)
QString ONF_StepConvertFloatImageToqint32::getStepDescription() const
{
    return tr("Convertir raster Float en raster quint32");
}

// Step detailled description
QString ONF_StepConvertFloatImageToqint32::getStepDetailledDescription() const
{
    return tr("No detailled description for this step");
}

// Step URL
QString ONF_StepConvertFloatImageToqint32::getStepURL() const
{
    //return tr("STEP URL HERE");
    return CT_AbstractStep::getStepURL(); //by default URL of the plugin
}

// Step copy method
CT_VirtualAbstractStep* ONF_StepConvertFloatImageToqint32::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new ONF_StepConvertFloatImageToqint32(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void ONF_StepConvertFloatImageToqint32::createInResultModelListProtected()
{
    CT_InResultModelGroupToCopy *resIn_res = createNewInResultModelForCopy(DEFin_res, tr("Raster"));
    resIn_res->setZeroOrMoreRootGroup();
    resIn_res->addGroupModel("", DEFin_grp, CT_AbstractItemGroup::staticGetType(), tr("Groupe"));
    resIn_res->addItemModel(DEFin_grp, DEFin_image, CT_Image2D<float>::staticGetType(), tr("Image (float)"));

}

// Creation and affiliation of OUT models
void ONF_StepConvertFloatImageToqint32::createOutResultModelListProtected()
{
    CT_OutResultModelGroupToCopyPossibilities *res = createNewOutResultModelToCopy(DEFin_res);

    if(res != NULL)
        res->addItemModel(DEFin_grp, _imageI_ModelName, new CT_Image2D<qint32>(), tr("Image (qint32)"));
}

// Semi-automatic creation of step parameters DialogBox
void ONF_StepConvertFloatImageToqint32::createPostConfigurationDialog()
{
    // No parameter dialog for this step
}

void ONF_StepConvertFloatImageToqint32::compute()
{
    QList<CT_ResultGroup*> outResultList = getOutResultList();
    CT_ResultGroup* res = outResultList.at(0);

    // COPIED results browsing
    CT_ResultGroupIterator itCpy_grp(res, this, DEFin_grp);
    while (itCpy_grp.hasNext() && !isStopped())
    {
        CT_StandardItemGroup* grpCpy_grp = (CT_StandardItemGroup*) itCpy_grp.next();
        
        const CT_Image2D<float>* image = (CT_Image2D<float>*)grpCpy_grp->firstItemByINModelName(this, DEFin_image);
        if (image != NULL)
        {
            CT_Image2D<qint32>* imageI = new CT_Image2D<qint32>(_imageI_ModelName.completeName(), res, image->minX(), image->minY(), image->colDim(), image->linDim(),
                                                                image->resolution(), image->level(), image->NA(), 0);
            grpCpy_grp->addItemDrawable(imageI);

            for (size_t xx = 0 ; xx < image->colDim() ; xx++)
            {
                for (size_t yy = 0 ; yy < image->linDim() ; yy++)
                {
                    float value = image->value(xx, yy);
                    imageI->setValue(xx, yy, (qint32) value);
                }
            }
        }

    }    
}
#endif
