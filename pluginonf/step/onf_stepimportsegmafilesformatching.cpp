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

#include "onf_stepimportsegmafilesformatching.h"
#include "ct_itemdrawable/ct_point2d.h"
#include "ct_result/ct_resultgroup.h"
#include "ct_result/model/outModel/ct_outresultmodelgroup.h"
#include "ct_view/ct_stepconfigurabledialog.h"

#include <QFile>
#include <QTextStream>

// Alias for indexing models
#define DEFout_refRes "refRes"
#define DEFout_grpRef "grpRef"
#define DEFout_ref "ref"
#define DEFout_refVal "refval"
#define DEFout_refID "refID"

#define DEFout_transRes "transRes"
#define DEFout_grpTrans "grpTrans"
#define DEFout_trans "trans"
#define DEFout_transVal "transval"
#define DEFout_transID "transID"


// Constructor : initialization of parameters
ONF_StepImportSegmaFilesForMatching::ONF_StepImportSegmaFilesForMatching(CT_StepInitializeData &dataInit) : CT_AbstractStepCanBeAddedFirst(dataInit)
{
}

// Step description (tooltip of contextual menu)
QString ONF_StepImportSegmaFilesForMatching::getStepDescription() const
{
    return tr("Fichiers SEGMA : un de ref, un à transformer");
}

// Step detailled description
QString ONF_StepImportSegmaFilesForMatching::getStepDetailledDescription() const
{
    return tr("No detailled description for this step");
}

// Step URL
QString ONF_StepImportSegmaFilesForMatching::getStepURL() const
{
    //return tr("STEP URL HERE");
    return CT_AbstractStepCanBeAddedFirst::getStepURL(); //by default URL of the plugin
}

// Step copy method
CT_VirtualAbstractStep* ONF_StepImportSegmaFilesForMatching::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new ONF_StepImportSegmaFilesForMatching(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void ONF_StepImportSegmaFilesForMatching::createInResultModelListProtected()
{
    // No in result is needed
    setNotNeedInputResult();
}

// Creation and affiliation of OUT models
void ONF_StepImportSegmaFilesForMatching::createOutResultModelListProtected()
{
    CT_OutResultModelGroup *res_refRes = createNewOutResultModel(DEFout_refRes, tr("Positions de référence"));
    res_refRes->setRootGroup(DEFout_grpRef, new CT_StandardItemGroup(), tr("Groupe"));
    res_refRes->addItemModel(DEFout_grpRef, DEFout_ref, new CT_Point2D(), tr("Position de référence"));
    res_refRes->addItemAttributeModel(DEFout_ref, DEFout_refVal, new CT_StdItemAttributeT<float>(CT_AbstractCategory::DATA_HEIGHT), tr("Valeur"));
    res_refRes->addItemAttributeModel(DEFout_ref, DEFout_refID, new CT_StdItemAttributeT<QString>(CT_AbstractCategory::DATA_ID), tr("IDsegma"));

    CT_OutResultModelGroup *res_transRes = createNewOutResultModel(DEFout_transRes, tr("Positions à transformer"));
    res_transRes->setRootGroup(DEFout_grpTrans, new CT_StandardItemGroup(), tr("Groupe"));
    res_transRes->addItemModel(DEFout_grpTrans, DEFout_trans, new CT_Point2D(), tr("Position à transformer"));
    res_transRes->addItemAttributeModel(DEFout_trans, DEFout_transVal, new CT_StdItemAttributeT<float>(CT_AbstractCategory::DATA_HEIGHT), tr("Valeur"));
    res_transRes->addItemAttributeModel(DEFout_trans, DEFout_transID, new CT_StdItemAttributeT<QString>(CT_AbstractCategory::DATA_ID), tr("IDsegma"));

}

// Semi-automatic creation of step parameters DialogBox
void ONF_StepImportSegmaFilesForMatching::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addFileChoice(tr("Fichier SEGMA des positions de référence"), CT_FileChoiceButton::OneExistingFile, "Fichier ascii (*.*)", _refFile);
    configDialog->addFileChoice(tr("Fichier SEGMA des positions à transformer"), CT_FileChoiceButton::OneExistingFile, "Fichier ascii (*.*)", _transFile);
}

void ONF_StepImportSegmaFilesForMatching::compute()
{

    QList<CT_ResultGroup*> outResultList = getOutResultList();
    CT_ResultGroup* res_refRes = outResultList.at(0);
    CT_ResultGroup* res_transRes = outResultList.at(1);


    if(QFile::exists(_refFile.first()))
    {
        QFile fRef(_refFile.first());

        if (fRef.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream stream(&fRef);

            stream.readLine();

            size_t cpt = 1;
            while (!stream.atEnd())
            {
                QString line = stream.readLine();
                cpt++;
                if (!line.isNull())
                {
                    QStringList values = line.split(",");
                    if (values.size() >= 8)
                    {
                        bool okX, okY, okVal;
                        double x = values.at(3).toDouble(&okX);
                        double y = values.at(4).toDouble(&okY);
                        float val = values.at(7).toFloat(&okVal);
                        QString id = values.at(0);

                        if (okX && okY && okVal)
                        {
                            CT_StandardItemGroup* grp_grpRef= new CT_StandardItemGroup(DEFout_grpRef, res_refRes);
                            res_refRes->addGroup(grp_grpRef);

                            CT_Point2D* item_ref = new CT_Point2D(DEFout_ref, res_refRes, new CT_Point2DData(x,y));
                            grp_grpRef->addItemDrawable(item_ref);

                            item_ref->addItemAttribute(new CT_StdItemAttributeT<float>(DEFout_refVal, CT_AbstractCategory::DATA_HEIGHT, res_refRes, val));
                            item_ref->addItemAttribute(new CT_StdItemAttributeT<QString>(DEFout_refID, CT_AbstractCategory::DATA_ID, res_refRes, id));



                        } else {
                            PS_LOG->addMessage(LogInterface::info, LogInterface::step, QString(tr("Ligne %1 du fichier REF non valide")).arg(cpt));
                        }

                    }
                }
            }
            fRef.close();
        }
    }



    if(QFile::exists(_transFile.first()))
    {
        QFile fTrans(_transFile.first());

        if (fTrans.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream stream(&fTrans);

            stream.readLine();

            size_t cpt = 1;
            while (!stream.atEnd())
            {
                QString line = stream.readLine();
                if (!line.isNull())
                {
                    QStringList values = line.split(",");
                    if (values.size() >= 8)
                    {
                        bool okX, okY, okVal;
                        double x = values.at(3).toDouble(&okX);
                        double y = values.at(4).toDouble(&okY);
                        float val = values.at(7).toFloat(&okVal);
                        QString id = values.at(0);

                        if (okX && okY && okVal)
                        {
                            CT_StandardItemGroup* grp_grpTrans= new CT_StandardItemGroup(DEFout_grpTrans, res_transRes);
                            res_transRes->addGroup(grp_grpTrans);

                            CT_Point2D* item_trans = new CT_Point2D(DEFout_trans, res_transRes, new CT_Point2DData(x,y));
                            grp_grpTrans->addItemDrawable(item_trans);

                            item_trans->addItemAttribute(new CT_StdItemAttributeT<float>(DEFout_transVal, CT_AbstractCategory::DATA_HEIGHT, res_transRes, val));
                            item_trans->addItemAttribute(new CT_StdItemAttributeT<QString>(DEFout_transID, CT_AbstractCategory::DATA_ID, res_transRes, id));
                        } else {
                            PS_LOG->addMessage(LogInterface::info, LogInterface::step, QString(tr("Ligne %1 du fichier TRANS non valide")).arg(cpt));
                        }

                    }
                }
            }

            fTrans.close();
        }
    }

}
