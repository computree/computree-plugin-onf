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

#include "onf_steploaddatafromitemposition.h"

#include "ct_itemdrawable/abstract/ct_abstractsingularitemdrawable.h"
#include "ct_itemdrawable/tools/iterator/ct_itemiterator.h"
#include "ct_result/ct_resultgroup.h"
#include "ct_result/model/inModel/ct_inresultmodelgroup.h"
#include "ct_result/model/outModel/ct_outresultmodelgroup.h"
#include "ct_view/ct_stepconfigurabledialog.h"

#include "ct_itemdrawable/ct_datasourcegeo.h"
#include "ct_global/ct_context.h"
#include "ct_model/inModel/tools/ct_instdmodelpossibility.h"
#include "ct_model/tools/ct_modelsearchhelper.h"

// Alias for indexing models
#define DEFin_resultItems "resultItems"
#define DEFin_grpItems "grpItems"
#define DEFin_item "item"

#define DEFin_resultDataSource "resultDataSource"
#define DEFin_grpDataSource "grpDataSource"
#define DEFin_dataSource "dataSource"

#define DEFout_result "result"
#define DEFout_grp "grp_ONF_StepLoadDataFromItemPosition"


// Constructor : initialization of parameters
ONF_StepLoadDataFromItemPosition::ONF_StepLoadDataFromItemPosition(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _buffer = 0;
}

// Step description (tooltip of contextual menu)
QString ONF_StepLoadDataFromItemPosition::getStepDescription() const
{
    return tr("Fichiers d'un DataSource intersectant l'emprise d'Items");
}

// Step detailled description
QString ONF_StepLoadDataFromItemPosition::getStepDetailledDescription() const
{
    return tr("No detailled description for this step");
}

// Step URL
QString ONF_StepLoadDataFromItemPosition::getStepURL() const
{
    //return tr("STEP URL HERE");
    return CT_AbstractStep::getStepURL(); //by default URL of the plugin
}

// Step copy method
CT_VirtualAbstractStep* ONF_StepLoadDataFromItemPosition::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new ONF_StepLoadDataFromItemPosition(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void ONF_StepLoadDataFromItemPosition::createInResultModelListProtected()
{
    CT_InResultModelGroup *resIn_dataSource = createNewInResultModel(DEFin_resultDataSource, tr("Source de données géographique"), "", true);
    resIn_dataSource->setZeroOrMoreRootGroup();
    resIn_dataSource->addGroupModel("", DEFin_grpDataSource, CT_AbstractItemGroup::staticGetType(), tr("Groupe"));
    resIn_dataSource->addItemModel(DEFin_grpDataSource, DEFin_dataSource, CT_DataSourceGeo::staticGetType(), tr("Source de données géographique"));

    CT_InResultModelGroup *resIn_Items = createNewInResultModel(DEFin_resultItems, tr("Items"));
    resIn_Items->setZeroOrMoreRootGroup();
    resIn_Items->addGroupModel("", DEFin_grpItems, CT_AbstractItemGroup::staticGetType(), tr("Groupe"));
    resIn_Items->addItemModel(DEFin_grpItems, DEFin_item, CT_AbstractSingularItemDrawable::staticGetType(), tr("Item"));

}

// Creation and affiliation of OUT models
void ONF_StepLoadDataFromItemPosition::createOutResultModelListProtected()
{
    // Récupération du prototype de data_source = la dataSource
    CT_InAbstractResultModel *resultInModel = getInResultModel(DEFin_resultDataSource);

    CT_OutAbstractResultModel* resultInModelOut = NULL;
    CT_InAbstractSingularItemModel* sourceModel = NULL;
    CT_DataSourceGeo* sourcePrototype = NULL;

    // check if model have choice (can be empty if the step want to create a default out model list)
    if(!resultInModel->getPossibilitiesSavedSelected().isEmpty())
        resultInModelOut = (CT_OutAbstractResultModel*)resultInModel->getPossibilitiesSavedSelected().first()->outModel();

    if(resultInModelOut != NULL)
        sourceModel = (CT_InAbstractSingularItemModel*)PS_MODELS->searchModel(DEFin_dataSource, resultInModelOut, this);

    if((sourceModel != NULL) && !sourceModel->getPossibilitiesSavedSelected().isEmpty())
        sourcePrototype = (CT_DataSourceGeo*) ((CT_OutAbstractSingularItemModel*) sourceModel->getPossibilitiesSavedSelected().first()->outModel())->itemDrawable()->copy(NULL, NULL, CT_ResultCopyModeList() << CT_ResultCopyModeList::DontCopyItemDrawable);


    // Création du groupe racine
    CT_OutStdGroupModel *root = new CT_OutStdGroupModel(DEFout_grp);

        if (sourcePrototype != NULL && sourcePrototype->getNumberOfReader() > 0)
        {
            sourcePrototype->init();
            QSharedPointer<CT_AbstractReader> firstReader = sourcePrototype->getNextReader();
            if (!firstReader.isNull())
            {
                _itemModels.clear();
                _groupModels.clear();

                _itemModels.append(firstReader->outItemDrawableModels());
                _groupModels.append(firstReader->outGroupsModel());

                // Ajout du modèle de header
                root->addItem((CT_OutStdSingularItemModel*)firstReader->outHeaderModel()->copy());

                // Ajout des modèles d'items
                QListIterator<CT_OutStdSingularItemModel*> itIM(_itemModels);
                while (itIM.hasNext())
                {                                       
                    CT_OutStdSingularItemModel* itemModel = itIM.next();                    
                    root->addItem((CT_OutStdSingularItemModel*)itemModel->copy());
                }

                // Ajout des modèles de groupes
                QListIterator<CT_OutStdGroupModel*> itGM(_groupModels);
                while (itGM.hasNext())
                {
                    CT_OutStdGroupModel* groupModel = itGM.next();
                    root->addGroup((CT_OutStdGroupModel*)groupModel->copy());
                }
            }
        }

        // Ajout du modèle de résultat
        addOutResultModel(new CT_OutResultModelGroup(DEFout_result, root, tr("Données chargées")));
}

// Semi-automatic creation of step parameters DialogBox
void ONF_StepLoadDataFromItemPosition::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addDouble("Epaisseur de la zone tampon", "m", -1e+09, 1e+09, 3, _buffer, 1);
}

void ONF_StepLoadDataFromItemPosition::compute()
{
    QList<CT_ResultGroup*> inResultList = getInputResults();
    CT_ResultGroup* resIn_dataSource = inResultList.at(0);
    CT_ResultGroup* resIn_items = inResultList.at(1);

    QList<CT_ResultGroup*> outResultList = getOutResultList();
    CT_ResultGroup* res_result = outResultList.at(0);


    CT_ResultItemIterator itIn_dataSource(resIn_dataSource, this, DEFin_dataSource);
    if (itIn_dataSource.hasNext())
    {
        const CT_DataSourceGeo *dataSource = (CT_DataSourceGeo*) itIn_dataSource.next();

        if (dataSource != NULL && dataSource->getNumberOfReader() > 0)
        {
            CT_ResultItemIterator itIn_items(resIn_items, this, DEFin_item);
            while (itIn_items.hasNext() && !isStopped())
            {
                const CT_AbstractSingularItemDrawable* item = (CT_AbstractSingularItemDrawable*) itIn_items.next();

                if (item != NULL)
                {
                    Eigen::Vector3d min, max;
                    item->getBoundingBox(min, max);

                    QList<QSharedPointer<CT_AbstractReader> > readers = dataSource->getReadersIntersecting(min, max);

                    for (int i = 0 ; i < readers.size() ; i++)
                    {
                        QSharedPointer<CT_AbstractReader> reader = readers.at(i);

                        if (reader->readFile())
                        {
                            CT_StandardItemGroup* grp_grp= new CT_StandardItemGroup(DEFout_grp, res_result);
                            res_result->addGroup(grp_grp);

                            // Ajout du header
                            CT_OutAbstractItemModel *modelCreationH = (CT_OutAbstractItemModel*)PS_MODELS->searchModelForCreation(reader->outHeaderModel()->uniqueName(), res_result);
                            CT_AbstractSingularItemDrawable *itemH = reader->takeHeaderCopy(res_result, modelCreationH);
                            grp_grp->addItemDrawable(itemH);

                            // Ajout des items
                            QListIterator<CT_OutStdSingularItemModel*> itIM(_itemModels);
                            while (itIM.hasNext())
                            {
                                CT_OutStdSingularItemModel* itemModel = itIM.next();

                                CT_OutAbstractItemModel *modelCreation = (CT_OutAbstractItemModel*)PS_MODELS->searchModelForCreation(itemModel->uniqueName(), res_result);
                                CT_AbstractSingularItemDrawable *item = reader->takeFirstItemDrawableOfModel(itemModel->uniqueName(), res_result, modelCreation);
                                grp_grp->addItemDrawable(item);
                            }

                            // Ajout des groupes
                            QListIterator<CT_OutStdGroupModel*> itGM(_groupModels);
                            while (itGM.hasNext())
                            {
                                CT_OutStdGroupModel* groupModel = itGM.next();

                                CT_OutAbstractItemModel *modelCreation = (CT_OutAbstractItemModel*)PS_MODELS->searchModelForCreation(groupModel->uniqueName(), res_result);
                                CT_AbstractItemGroup *groupe = reader->takeFirstGroupOfModel(groupModel->uniqueName(), res_result, modelCreation);
                                grp_grp->addGroup(groupe);
                            }
                        }
                    }
                }
            }
        }
    }


    
}
