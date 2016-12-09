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

#include "onf_stepcomputecumulativesummary.h"

#include "ct_itemdrawable/ct_standarditemgroup.h"
#include "ct_itemdrawable/ct_loopcounter.h"
#include "ct_itemdrawable/ct_attributeslist.h"

#include "ct_itemdrawable/tools/iterator/ct_groupiterator.h"
#include "ct_itemdrawable/tools/iterator/ct_itemiterator.h"
#include "ct_result/ct_resultgroup.h"
#include "ct_result/model/inModel/ct_inresultmodelgroup.h"
#include "ct_result/model/outModel/ct_outresultmodelgroup.h"
#include "ct_view/ct_stepconfigurabledialog.h"

#include <QDebug>
#include <QFile>
#include <QTextStream>


// Alias for indexing models
#define DEFin_rscene "rscene"
#define DEFin_grpItem "grpitem"
#define DEFin_item "item"
#define DEFin_att "att"

#define DEF_inResultCounter "rcounter"
#define DEF_inCounter "counter"

#define DEFout_res "outres"
#define DEFout_grp "outgrp"
#define DEFout_summary "summary"

// Constructor : initialization of parameters
ONF_StepComputeCumulativeSummary::ONF_StepComputeCumulativeSummary(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _fileName << "summary.txt";
}

// Step description (tooltip of contextual menu)
QString ONF_StepComputeCumulativeSummary::getStepDescription() const
{
    return tr("Export summary of metrics");
}

// Step detailled description
QString ONF_StepComputeCumulativeSummary::getStepDetailledDescription() const
{
    return tr("No detailled description for this step");
}

// Step URL
QString ONF_StepComputeCumulativeSummary::getStepURL() const
{
    //return tr("STEP URL HERE");
    return CT_AbstractStep::getStepURL(); //by default URL of the plugin
}

// Step copy method
CT_VirtualAbstractStep* ONF_StepComputeCumulativeSummary::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new ONF_StepComputeCumulativeSummary(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void ONF_StepComputeCumulativeSummary::createInResultModelListProtected()
{
    CT_InResultModelGroup *resIn_item = createNewInResultModel(DEFin_rscene, tr("Scène(s)"));
    resIn_item->setZeroOrMoreRootGroup();
    resIn_item->addGroupModel("", DEFin_grpItem, CT_AbstractItemGroup::staticGetType(), tr("Groupe"), "", CT_InAbstractGroupModel::CG_ChooseOneIfMultiple);
    resIn_item->addItemModel(DEFin_grpItem, DEFin_item, CT_AbstractSingularItemDrawable::staticGetType(), tr("Item"),"", CT_InAbstractModel::C_ChooseMultipleIfMultiple);
    resIn_item->addItemAttributeModel(DEFin_item, DEFin_att, QList<QString>() << CT_AbstractCategory::DATA_VALUE, CT_AbstractCategory::NUMBER, tr("Attribut"), "", CT_InAbstractModel::C_ChooseMultipleIfMultiple);

    CT_InResultModelGroup* res_counter = createNewInResultModel(DEF_inResultCounter, tr("Résultat compteur"), "", true);
    res_counter->setZeroOrMoreRootGroup();
    res_counter->addItemModel("", DEF_inCounter, CT_LoopCounter::staticGetType(), tr("Compteur"));
    res_counter->setMinimumNumberOfPossibilityThatMustBeSelectedForOneTurn(0);
}

// Creation and affiliation of OUT models
void ONF_StepComputeCumulativeSummary::createOutResultModelListProtected()
{
    CT_OutResultModelGroup *resultSummary = createNewOutResultModel(DEFout_res, tr("Summary"));
    resultSummary->setRootGroup(DEFout_grp);
    resultSummary->addItemModel(DEFout_grp, DEFout_summary, new CT_AttributesList(), tr("Summary"));
}

// Semi-automatic creation of step parameters DialogBox
void ONF_StepComputeCumulativeSummary::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addFileChoice(tr("Choose Export file"), CT_FileChoiceButton::OneNewFile, "Text file (*.txt)", _fileName);
}

void ONF_StepComputeCumulativeSummary::compute()
{
    QList<CT_ResultGroup*> inResultList = getInputResults();
    CT_ResultGroup* ritem = inResultList.at(0);

    QList<CT_ResultGroup*> outResultList = getOutResultList();
    CT_ResultGroup* rSummary = outResultList.at(0);


    bool last_turn = false;
    bool first_turn = true;
    CT_ResultGroup* rcounter = NULL;
    if (inResultList.size() > 1) {rcounter = inResultList.at(1);}
    if (rcounter != NULL)
    {
        CT_ResultItemIterator itCounter(rcounter, this, DEF_inCounter);
        if (itCounter.hasNext())
        {
            const CT_LoopCounter* counter = (const CT_LoopCounter*) itCounter.next();
            if (counter != NULL)
            {
                if (counter->getCurrentTurn() > 1)
                {
                    first_turn = false;
                }

                if (counter->getCurrentTurn() == counter->getNTurns())
                {
                    last_turn = true;
                }
            }
        }
    }

    if (first_turn)
    {
        _hashAtt = PS_MODELS->splitSelectedAttributesModelBySelectedSingularItemModel(DEFin_att, DEFin_item, ritem->model(), this);
        QHashIterator<CT_OutAbstractSingularItemModel *, CT_OutAbstractItemAttributeModel *> ithItem(_hashAtt);
        while (ithItem.hasNext())
        {
            ithItem.next();
            _hashAtt.insert((CT_OutAbstractSingularItemModel*) (ithItem.key()->originalModel()), ithItem.value());
        }

        QHashIterator<CT_OutAbstractSingularItemModel *, CT_OutAbstractItemAttributeModel *> itModels(_hashAtt);
        while (itModels.hasNext())
        {
            itModels.next();

            CT_OutAbstractSingularItemModel  *itemModel = itModels.key();
            CT_OutAbstractItemAttributeModel *attrModel = itModels.value();

            QString itemDN = itemModel->displayableName();
            QString itemUN = itemModel->uniqueName();

            QString attrDN = attrModel->displayableName();
            QString attrUN = attrModel->uniqueName();

            if (attrModel->isADefaultItemAttributeModel() && attrModel->originalModel() != NULL) {attrUN = attrModel->originalModel()->uniqueName();}

            QString key = QString("ITEM_%1_ATTR_%2").arg(itemUN).arg(attrUN);
            _dataMap.insert(key, QVector<double>());
            _namesMap.insert(key, QString("%2_%1").arg(itemDN).arg(attrDN));
        }
    }



    // parcours des item
    CT_ResultGroupIterator itSc(ritem, this, DEFin_grpItem);
    while (itSc.hasNext() && !isStopped())
    {
        CT_StandardItemGroup* grp = (CT_StandardItemGroup*) itSc.next();

        CT_ItemIterator itGrp(grp, this, DEFin_item);
        while (itGrp.hasNext())
        {
            const CT_AbstractSingularItemDrawable* item = itGrp.next();

            QMutableMapIterator<QString, QVector<double> > itMap(_dataMap);
            while (itMap.hasNext())
            {
                itMap.next();
                QVector<double> &vect = itMap.value();
                vect.append(NAN);
            }

            if (item != NULL)
            {
                CT_OutAbstractSingularItemModel  *itemModel = (CT_OutAbstractSingularItemModel*)item->model();

                QList<CT_OutAbstractItemAttributeModel *> attributesModel = _hashAtt.values(itemModel);
                QList<CT_AbstractItemAttribute *> attributes = item->itemAttributes(attributesModel);

                for (int i = 0 ; i < attributes.size() ; i++)
                {
                    CT_AbstractItemAttribute* attribute = attributes.at(i);
                    if (attribute != NULL)
                    {
                        CT_OutAbstractItemAttributeModel* attrModel = (CT_OutAbstractItemAttributeModel*) attribute->model();

                        QString itemUN = itemModel->uniqueName();
                        QString attrUN = attrModel->uniqueName();

                        QString key = QString("ITEM_%1_ATTR_%2").arg(itemUN).arg(attrUN);

                        QVector<double> &vect = _dataMap[key];

                        bool ok;
                        vect[vect.size() - 1] = attribute->toDouble(item, &ok);
                        if (!ok) {vect[vect.size() - 1] = NAN;}
                    }
                }
            }
        }
    }

    if (_fileName.size() > 0 && _fileName.first() != "" && (rcounter == NULL || last_turn))
    {
        QFile file(_fileName.first());

        if(file.open(QFile::WriteOnly))
        {
            QTextStream stream(&file);

            stream << "Variable_Name\tMean\tMax\tMin\tnb_Val\tnb_NA\n";


            QMapIterator<QString, QVector<double> > itMap(_dataMap);
            while (itMap.hasNext())
            {
                itMap.next();
                QString key = itMap.key();
                const QVector<double> &values = itMap.value();

                QString varName = _namesMap.value(key);

                double mean = 0;
                double max = -std::numeric_limits<double>::max();
                double min = std::numeric_limits<double>::max();
                int cpt = 0;
                int NAcpt = 0;

                for (int i = 0 ; i < values.size() ; i++)
                {
                    double val = values.at(i);

                    if (isnan(val))
                    {
                        NAcpt++;
                    } else {
                        cpt++;
                        mean += val;
                        if (val > max) {max = val;}
                        if (val < min) {min = val;}
                    }
                }

                if (cpt > 0) {mean /= cpt;}

                stream << varName << "\t" << mean << "\t" << max << "\t" << min << "\t" << cpt << "\t" << NAcpt << "\n";
            }

            file.close();
        }
    }

}

