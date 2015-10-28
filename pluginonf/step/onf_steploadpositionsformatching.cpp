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

#include "onf_steploadpositionsformatching.h"

#include "ct_itemdrawable/ct_point2d.h"
#include "ct_result/ct_resultgroup.h"
#include "ct_result/model/outModel/ct_outresultmodelgroup.h"
#include "ct_view/ct_stepconfigurabledialog.h"

// Alias for indexing models
#define DEFout_refRes "refRes"
#define DEFout_grpRef "grpRef"
#define DEFout_ref "ref"
#define DEFout_refVal "refval"
#define DEFout_refID "refID"
#define DEFout_refIDplot "refIDplot"

#define DEFout_transRes "transRes"
#define DEFout_grpTrans "grpTrans"
#define DEFout_trans "trans"
#define DEFout_transVal "transval"
#define DEFout_transID "transID"
#define DEFout_transIDplot "transIDplot"

#include <QFile>
#include <QTextStream>


// Constructor : initialization of parameters
ONF_StepLoadPositionsForMatching::ONF_StepLoadPositionsForMatching(CT_StepInitializeData &dataInit) : CT_StepBeginLoop(dataInit)
{
    _neededFields << "ID_Plot" << "ID" << "X" << "Y" << "VALUE";

    _refFileName = "";
    _transFileName = "";

    _refHeader = true;
    _transHeader = true;

    _refSeparator = "\t";
    _transSeparator = "\t";

    _refDecimal = ".";
    _transDecimal = ".";  

    _refLocale = QLocale(QLocale::English, QLocale::UnitedKingdom).name();
    _transLocale = QLocale(QLocale::English, QLocale::UnitedKingdom).name();

    _refSkip = 0;
    _transSkip = 0;
}

// Step description (tooltip of contextual menu)
QString ONF_StepLoadPositionsForMatching::getStepDescription() const
{
    return tr("Charge deux fichiers de positions pour mise en correspondance");
}

// Step detailled description
QString ONF_StepLoadPositionsForMatching::getStepDetailledDescription() const
{
    return tr("No detailled description for this step");
}

// Step URL
QString ONF_StepLoadPositionsForMatching::getStepURL() const
{
    //return tr("STEP URL HERE");
    return CT_AbstractStepCanBeAddedFirst::getStepURL(); //by default URL of the plugin
}

// Step copy method
CT_VirtualAbstractStep* ONF_StepLoadPositionsForMatching::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new ONF_StepLoadPositionsForMatching(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void ONF_StepLoadPositionsForMatching::createInResultModelListProtected()
{
    // No in result is needed
    setNotNeedInputResult();
}

// Creation and affiliation of OUT models
void ONF_StepLoadPositionsForMatching::createOutResultModelListProtected(CT_OutResultModelGroup *firstResultModel)
{
    Q_UNUSED(firstResultModel);

    CT_OutResultModelGroup *res_refRes = createNewOutResultModel(DEFout_refRes, tr("Positions de référence"));
    res_refRes->setRootGroup(DEFout_grpRef, new CT_StandardItemGroup(), tr("Groupe"));
    res_refRes->addItemModel(DEFout_grpRef, DEFout_ref, new CT_Point2D(), tr("Position de référence"));
    res_refRes->addItemAttributeModel(DEFout_ref, DEFout_refVal, new CT_StdItemAttributeT<float>(CT_AbstractCategory::DATA_NUMBER), tr("Valeur"));
    res_refRes->addItemAttributeModel(DEFout_ref, DEFout_refID, new CT_StdItemAttributeT<QString>(CT_AbstractCategory::DATA_ID), tr("IDtree"));
    res_refRes->addItemAttributeModel(DEFout_ref, DEFout_refIDplot, new CT_StdItemAttributeT<QString>(CT_AbstractCategory::DATA_ID), tr("IDplot"));

    CT_OutResultModelGroup *res_transRes = createNewOutResultModel(DEFout_transRes, tr("Positions à transformer"));
    res_transRes->setRootGroup(DEFout_grpTrans, new CT_StandardItemGroup(), tr("Groupe"));
    res_transRes->addItemModel(DEFout_grpTrans, DEFout_trans, new CT_Point2D(), tr("Position à transformer"));
    res_transRes->addItemAttributeModel(DEFout_trans, DEFout_transVal, new CT_StdItemAttributeT<float>(CT_AbstractCategory::DATA_NUMBER), tr("Valeur"));
    res_transRes->addItemAttributeModel(DEFout_trans, DEFout_transID, new CT_StdItemAttributeT<QString>(CT_AbstractCategory::DATA_ID), tr("IDtree"));
    res_transRes->addItemAttributeModel(DEFout_trans, DEFout_transIDplot, new CT_StdItemAttributeT<QString>(CT_AbstractCategory::DATA_ID), tr("IDplot"));

}

// Semi-automatic creation of step parameters DialogBox
void ONF_StepLoadPositionsForMatching::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addAsciiFileChoice("Fichier de positions de référence", "Fichier ASCII (*.txt ; *.asc)", true, _neededFields, _refFileName, _refHeader, _refSeparator, _refDecimal, _refLocale, _refSkip, _refColumns);
    configDialog->addAsciiFileChoice("Fichier de positions à transformer", "Fichier ASCII (*.txt ; *.asc)", true, _neededFields, _transFileName, _transHeader, _transSeparator, _transDecimal, _transLocale, _transSkip, _transColumns);
}

void ONF_StepLoadPositionsForMatching::compute(CT_ResultGroup *outRes, CT_StandardItemGroup* group)
{
    Q_UNUSED(outRes);
    Q_UNUSED(group);

    QList<CT_ResultGroup*> outResultList = getOutResultList();
    CT_ResultGroup* res_refRes = outResultList.at(1);
    CT_ResultGroup* res_transRes = outResultList.at(2);


    int colIDplot_ref  = _refColumns.value("ID_Plot", -1);
    int colIDplot_trans  = _transColumns.value("ID_Plot", -1);
    bool multiPlots = (colIDplot_ref >= 0 && colIDplot_trans >= 0);

    QSharedPointer<CT_Counter> counter = getCounter();
    size_t  currentTurn = counter->getCurrentTurn();


    // Au premier tour : création de la liste des placettes
    if (multiPlots && currentTurn == 1)
    {
        _plotsIds.clear();

        QFile fRef00(_refFileName);
        if (fRef00.exists() && fRef00.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream stream(&fRef00);
            stream.setLocale(_refLocale);

            for (int i = 0 ; i < _refSkip ; i++) {stream.readLine();}
            if (_refHeader) {stream.readLine();}

            while (!stream.atEnd())
            {
                QString line = stream.readLine();
                if (!line.isEmpty())
                {
                    QStringList values = line.split(_refSeparator);

                    if (values.size() > colIDplot_ref)
                    {
                        const QString &val = values.at(colIDplot_ref);
                        if (!_plotsIds.contains(val)) {_plotsIds.append(val);}
                    }
                }
            }
            fRef00.close();
        }
        counter->setNTurns(_plotsIds.size());
        if (_plotsIds.size() <= 0) {return;}
    }


    QString currentPlot = "";
    if (multiPlots) {currentPlot = _plotsIds.at(currentTurn - 1);}
    NTurnsSelected();
    PS_LOG->addMessage(LogInterface::info, LogInterface::step, QString(tr("Placette en cours de traitement : %1")).arg(currentPlot));



    QFile fRef(_refFileName);
    if (fRef.exists() && fRef.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream stream(&fRef);
        stream.setLocale(_refLocale);

        int colID  = _refColumns.value("ID", -1);
        int colX   = _refColumns.value("X", -1);
        int colY   = _refColumns.value("Y", -1);
        int colVal = _refColumns.value("VALUE", -1);

        if (colID >=0 && colX >= 0 && colY >= 0 && colVal >= 0)
        {

            int colMax = colID;
            if (colX   > colMax) {colMax = colX;}
            if (colY   > colMax) {colMax = colY;}
            if (colVal > colMax) {colMax = colVal;}

            for (int i = 0 ; i < _refSkip ; i++) {stream.readLine();}
            if (_refHeader) {stream.readLine();}

            size_t cpt = 1;
            while (!stream.atEnd())
            {
                QString line = stream.readLine();
                cpt++;
                if (!line.isEmpty())
                {
                    QStringList values = line.split(_refSeparator);
                    if (values.size() >= colMax)
                    {
                        QString plot = "";
                        if (multiPlots) {plot = values.at(colIDplot_ref);}

                        if (plot == currentPlot)
                        {
                            bool okX, okY, okVal;
                            double x = _refLocale.toDouble(values.at(colX), &okX);
                            double y = _refLocale.toDouble(values.at(colY), &okY);
                            float val = _refLocale.toFloat(values.at(colVal), &okVal);
                            QString id = values.at(colID);

                            if (okX && okY && okVal)
                            {
                                CT_StandardItemGroup* grp_grpRef= new CT_StandardItemGroup(DEFout_grpRef, res_refRes);
                                res_refRes->addGroup(grp_grpRef);

                                CT_Point2D* item_ref = new CT_Point2D(DEFout_ref, res_refRes, new CT_Point2DData(x,y));
                                grp_grpRef->addItemDrawable(item_ref);

                                item_ref->addItemAttribute(new CT_StdItemAttributeT<float>(DEFout_refVal, CT_AbstractCategory::DATA_NUMBER, res_refRes, val));
                                item_ref->addItemAttribute(new CT_StdItemAttributeT<QString>(DEFout_refID, CT_AbstractCategory::DATA_ID, res_refRes, id));
                                item_ref->addItemAttribute(new CT_StdItemAttributeT<QString>(DEFout_refIDplot, CT_AbstractCategory::DATA_ID, res_refRes, plot));
                            } else {
                                PS_LOG->addMessage(LogInterface::info, LogInterface::step, QString(tr("Ligne %1 du fichier REF non valide")).arg(cpt));
                            }
                        }
                    }
                }
            }
        }
        fRef.close();
    }



    QFile fTrans(_transFileName);

    if (fTrans.exists() && fTrans.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream stream(&fTrans);
        stream.setLocale(_transLocale);

        int colID = _transColumns.value("ID", -1);
        int colX = _transColumns.value("X", -1);
        int colY = _transColumns.value("Y", -1);
        int colVal = _transColumns.value("VALUE", -1);

        if (colID >=0 && colX >= 0 && colY >= 0 && colVal >= 0)
        {

            int colMax = colID;
            if (colX   > colMax) {colMax = colX;}
            if (colY   > colMax) {colMax = colY;}
            if (colVal > colMax) {colMax = colVal;}


            for (int i = 0 ; i < _transSkip ; i++) {stream.readLine();}
            if (_transHeader) {stream.readLine();}

            size_t cpt = 1;
            while (!stream.atEnd())
            {
                QString line = stream.readLine();
                if (!line.isEmpty())
                {
                    QStringList values = line.split(_transSeparator);
                    if (values.size() >= colMax)
                    {
                        QString plot = "";
                        if (multiPlots) {plot = values.at(colIDplot_trans);}

                        if (plot == currentPlot)
                        {

                            bool okX, okY, okVal;
                            double x = _transLocale.toDouble(values.at(colX), &okX);
                            double y = _transLocale.toDouble(values.at(colY), &okY);
                            float val = _transLocale.toFloat(values.at(colVal), &okVal);
                            QString id = values.at(colID);

                            if (okX && okY && okVal)
                            {
                                CT_StandardItemGroup* grp_grpTrans= new CT_StandardItemGroup(DEFout_grpTrans, res_transRes);
                                res_transRes->addGroup(grp_grpTrans);

                                CT_Point2D* item_trans = new CT_Point2D(DEFout_trans, res_transRes, new CT_Point2DData(x,y));
                                grp_grpTrans->addItemDrawable(item_trans);

                                item_trans->addItemAttribute(new CT_StdItemAttributeT<float>(DEFout_transVal, CT_AbstractCategory::DATA_NUMBER, res_transRes, val));
                                item_trans->addItemAttribute(new CT_StdItemAttributeT<QString>(DEFout_transID, CT_AbstractCategory::DATA_ID, res_transRes, id));
                                item_trans->addItemAttribute(new CT_StdItemAttributeT<QString>(DEFout_transIDplot, CT_AbstractCategory::DATA_ID, res_transRes, plot));
                            } else {
                                PS_LOG->addMessage(LogInterface::info, LogInterface::step, QString(tr("Ligne %1 du fichier TRANS non valide")).arg(cpt));
                            }
                        }
                    }
                }
            }
        }

        fTrans.close();
    }

}
