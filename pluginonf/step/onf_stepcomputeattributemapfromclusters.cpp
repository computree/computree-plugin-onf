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

#include "onf_stepcomputeattributemapfromclusters.h"

#ifdef USE_OPENCV

#include "ct_itemdrawable/tools/iterator/ct_groupiterator.h"
#include "ct_result/ct_resultgroup.h"
#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"
#include "ct_view/ct_stepconfigurabledialog.h"
#include "ct_itemdrawable/ct_attributeslist.h"
#include "ct_iterator/ct_groupiterator.h"

// Alias for indexing models
#define DEFin_res "res"
#define DEFin_mainGrp "maingrp"
#define DEFin_clusters "clusters"
#define DEFin_attrib "attribut"
#define DEFin_grp "grp"
#define DEFin_item "item"
#define DEFin_itemID "itemID"
#define DEFin_itemAtt "att"

// Constructor : initialization of parameters
ONF_StepComputeAttributeMapFromClusters::ONF_StepComputeAttributeMapFromClusters(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _naValue = -9999;
    _defaultValue = 0;
}

// Step description (tooltip of contextual menu)
QString ONF_StepComputeAttributeMapFromClusters::getStepDescription() const
{
    return tr("Calcul d'un raster attribut à partir de clusters");
}

// Step detailled description
QString ONF_StepComputeAttributeMapFromClusters::getStepDetailledDescription() const
{
    return tr("No detailled description for this step");
}

// Step URL
QString ONF_StepComputeAttributeMapFromClusters::getStepURL() const
{
    //return tr("STEP URL HERE");
    return CT_AbstractStep::getStepURL(); //by default URL of the plugin
}

// Step copy method
CT_VirtualAbstractStep* ONF_StepComputeAttributeMapFromClusters::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new ONF_StepComputeAttributeMapFromClusters(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void ONF_StepComputeAttributeMapFromClusters::createInResultModelListProtected()
{
    CT_InResultModelGroupToCopy *res = createNewInResultModelForCopy(DEFin_res, tr("Clusters"));
    res->setZeroOrMoreRootGroup();
    res->addGroupModel("", DEFin_mainGrp, CT_AbstractItemGroup::staticGetType(), tr("Groupe"));
    res->addItemModel(DEFin_mainGrp, DEFin_clusters, CT_Image2D<qint32>::staticGetType(), tr("Image (Clusters)"));
    res->addItemModel(DEFin_mainGrp, DEFin_attrib, CT_AbstractImage2D::staticGetType(), tr("Image (attribut)"));

    res->addGroupModel(DEFin_mainGrp, DEFin_grp, CT_AbstractItemGroup::staticGetType(), tr("Groupe"));
    res->addItemModel(DEFin_grp, DEFin_item, CT_AbstractSingularItemDrawable::staticGetType(), tr("Item"));
    res->addItemAttributeModel(DEFin_item, DEFin_itemID, QList<QString>() << CT_AbstractCategory::DATA_VALUE, CT_AbstractCategory::INT32, tr("IDCluster"));
    res->addItemAttributeModel(DEFin_item, DEFin_itemAtt, QList<QString>() << CT_AbstractCategory::DATA_VALUE, CT_AbstractCategory::ANY, tr("Attribut"));
}

// Creation and affiliation of OUT models
void ONF_StepComputeAttributeMapFromClusters::createOutResultModelListProtected()
{
    CT_OutResultModelGroupToCopyPossibilities *res = createNewOutResultModelToCopy(DEFin_res);
    res->addItemModel(DEFin_mainGrp, _attMap_ModelName, new CT_Image2D<double>(), tr("Carte d'attribut"));

}

// Semi-automatic creation of step parameters DialogBox
void ONF_StepComputeAttributeMapFromClusters::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addDouble(tr("Valeur manquante dans le raster de sortie"), "", -1e+09, 1e+09, 2, _naValue);
    configDialog->addDouble(tr("Valeur par défaut dans le raster de sortie"), "", -1e+09, 1e+09, 2, _defaultValue);
}



void ONF_StepComputeAttributeMapFromClusters::compute()
{
    QList<CT_ResultGroup*> outResultList = getOutResultList();
    CT_ResultGroup* res = outResultList.at(0);

    // COPIED results browsing
    CT_ResultGroupIterator itCpy(res, this, DEFin_mainGrp);
    while (itCpy.hasNext() && !isStopped())
    {
        CT_StandardItemGroup* mainGrp = (CT_StandardItemGroup*) itCpy.next();
        CT_Image2D<qint32>* clustersIn = (CT_Image2D<qint32>*)mainGrp->firstItemByINModelName(this, DEFin_clusters);
        CT_AbstractImage2D* attribIn = (CT_AbstractImage2D*)mainGrp->firstItemByINModelName(this, DEFin_attrib);

        if (clustersIn != NULL && attribIn != NULL)
        {
            CT_Image2D<double>* attMap = new CT_Image2D<double>(_attMap_ModelName.completeName(), res, clustersIn->minX(), clustersIn->minY(), clustersIn->colDim(), clustersIn->linDim(), clustersIn->resolution(), clustersIn->level(), _naValue, _defaultValue);
            mainGrp->addItemDrawable(attMap);

            QMap<qint32, double> attValsMap;
            CT_GroupIterator itCpy(mainGrp, this, DEFin_grp);
            while (itCpy.hasNext() && !isStopped())
            {
                CT_StandardItemGroup* grp = (CT_StandardItemGroup*) itCpy.next();

                CT_AbstractSingularItemDrawable* itemWithAttribute = grp->firstItemByINModelName(this, DEFin_item);
                qint32 itemID = itemWithAttribute->firstItemAttributeByINModelName(res, this, DEFin_itemID)->toInt(itemWithAttribute, NULL);
                double attributeValue = itemWithAttribute->firstItemAttributeByINModelName(res, this, DEFin_itemAtt)->toDouble(itemWithAttribute, NULL);

                attValsMap.insert(itemID, attributeValue);
            }

            setProgress(20.0);

            for (size_t xx = 0 ; xx < clustersIn->colDim() ; xx++)
            {
                for (size_t yy = 0 ; yy < clustersIn->linDim() ; yy++)
                {
                    size_t index;
                    if (clustersIn->index(xx, yy, index))
                    {
                        qint32 cluster = clustersIn->value(xx, yy);

                        double attVal = attValsMap.value(cluster, _defaultValue);
                        if (cluster <= 0)
                        {
                            attVal = attribIn->valueAtIndexAsDouble(index);
                        }

                        attMap->setValue(xx, yy, attVal);
                    } else {
                        qDebug() << "Problème";
                    }
                }
                setProgress(20.0 + ((float)xx / (float)clustersIn->colDim()) * 70.0);

            }

            attMap->computeMinMax();

        }
        setProgress(100);
    }
}
#endif

