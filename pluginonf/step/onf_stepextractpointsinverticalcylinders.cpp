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

#include "onf_stepextractpointsinverticalcylinders.h"

#include "ct_itemdrawable/ct_scene.h"
#include "ct_result/ct_resultgroup.h"
#include "ct_result/model/outModel/ct_outresultmodelgroupcopy.h"
#include "ct_view/ct_stepconfigurabledialog.h"

#include "ct_view/ct_asciifilechoicebutton.h"
#include "ct_view/ct_combobox.h"

// Alias for indexing models
#define DEF_SearchInResult "inres"
#define DEF_SearchInGroup "ingrp"
#define DEF_SearchInScene "inscene"

#include <QFile>
#include <QTextStream>


// Constructor : initialization of parameters
ONF_StepExtractPointsInVerticalCylinders::ONF_StepExtractPointsInVerticalCylinders(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _neededFields.append(CT_TextFileConfigurationFields("ID_Plot", QRegExp("([iI][dD]|[nN][uU][mM]|[pP][lL][oO][tT]|[pP][lL][aA][cC][eE][tT][tT][eE])"), false));
    _neededFields.append(CT_TextFileConfigurationFields("ID_Tree", QRegExp("([iI][dD]|[nN][uU][mM])"), false));
    _neededFields.append(CT_TextFileConfigurationFields("X", QRegExp("[xX]"), false));
    _neededFields.append(CT_TextFileConfigurationFields("Y", QRegExp("[yY]"), false));
    _neededFields.append(CT_TextFileConfigurationFields("Zmin", QRegExp("([Zz][Mm][Ii][Nn]|[Hh][Mm][Ii][Nn])"), false));
    _neededFields.append(CT_TextFileConfigurationFields("Zmax", QRegExp("([Zz][Mm][Aa][Xx]|[Hh][Mm][Aa][Xx])"), false));
    _neededFields.append(CT_TextFileConfigurationFields(tr("Rayon"), QRegExp("([Rr][Aa][Dd][Ii][Uu][Ss]|[Rr][Aa][Yy][Oo][Nn]|[Rr])"), false));

    _refFileName = "";
    _refHeader = true;
    _refSeparator = "\t";
    _refDecimal = ".";
    _refLocale = QLocale(QLocale::English, QLocale::UnitedKingdom).name();
    _refSkip = 0;

    _plotID = "";
}

// Step description (tooltip of contextual menu)
QString ONF_StepExtractPointsInVerticalCylinders::getStepDescription() const
{
    return tr("Extraire points dans des cylindres verticaux (par placette)");
}

// Step detailled description
QString ONF_StepExtractPointsInVerticalCylinders::getStepDetailledDescription() const
{
    return tr("");
}

// Step URL
QString ONF_StepExtractPointsInVerticalCylinders::getStepURL() const
{
    //return tr("STEP URL HERE");
    return CT_AbstractStep::getStepURL(); //by default URL of the plugin
}

// Step copy method
CT_VirtualAbstractStep* ONF_StepExtractPointsInVerticalCylinders::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new ONF_StepExtractPointsInVerticalCylinders(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void ONF_StepExtractPointsInVerticalCylinders::createInResultModelListProtected()
{
    CT_InResultModelGroupToCopy *resultModel = createNewInResultModelForCopy(DEF_SearchInResult, tr("Scène(s)"));

//    resultModel->setZeroOrMoreRootGroup();
//    resultModel->addGroupModel("", DEF_SearchInGroup);
//    resultModel->addItemModel(DEF_SearchInGroup, DEF_SearchInScene, CT_Scene::staticGetType(), tr("Scène"));
}

// Creation and affiliation of OUT models
void ONF_StepExtractPointsInVerticalCylinders::createOutResultModelListProtected()
{
//    CT_OutResultModelGroupCopy *res_refRes = createNewOutResultModelToCopy(DEF_SearchInResult);
//    res_refRes->addItemModel(DEFout_grpRef, DEFout_ref, new CT_Circle2D(), tr("Position de référence"));
//    res_refRes->addItemAttributeModel(DEFout_ref, DEFout_refDbh, new CT_StdItemAttributeT<float>(CT_AbstractCategory::DATA_NUMBER), tr("DBH"));
//    res_refRes->addItemAttributeModel(DEFout_ref, DEFout_refID, new CT_StdItemAttributeT<QString>(CT_AbstractCategory::DATA_ID), tr("IDtree"));
//    res_refRes->addItemAttributeModel(DEFout_ref, DEFout_refIDplot, new CT_StdItemAttributeT<QString>(CT_AbstractCategory::DATA_ID), tr("IDplot"));
}

// Semi-automatic creation of step parameters DialogBox
void ONF_StepExtractPointsInVerticalCylinders::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    CT_AsciiFileChoiceButton *fileChoice = configDialog->addAsciiFileChoice("Fichier des arbres", "Fichier ASCII (*.txt ; *.asc)", true, _neededFields, _refFileName, _refHeader, _refSeparator, _refDecimal, _refLocale, _refSkip, _refColumns);
    CT_ComboBox *cbox = configDialog->addStringChoice(tr("Choix de la placette"), "", QStringList(), _plotID);

    connect(configDialog, SIGNAL(openned()), this, SLOT(fileChanged()));
    connect(fileChoice, SIGNAL(fileChanged()), this, SLOT(fileChanged()));
    connect(this, SIGNAL(updateComboBox(QStringList, QString)), cbox, SLOT(changeValues(QStringList, QString)));
}

void ONF_StepExtractPointsInVerticalCylinders::fileChanged()
{
    _plotsIds.clear();

    int colIDplot_ref  = _refColumns.value("ID_Plot", -1);

    if (colIDplot_ref < 0) {_plotID = ""; return;}

    QFile fRef(_refFileName);
    if (fRef.exists() && fRef.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream stream(&fRef);
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
        fRef.close();
    }

    QString val = "";
    if (_plotsIds.size() > 0) {val = _plotsIds.first();}
    emit updateComboBox(_plotsIds, val);
}

void ONF_StepExtractPointsInVerticalCylinders::compute()
{  

    QList<CT_ResultGroup*> outResultList = getOutResultList();
    CT_ResultGroup* res_refRes = outResultList.at(0);

    PS_LOG->addMessage(LogInterface::info, LogInterface::step, QString(tr("Placette en cours de traitement : %1")).arg(_plotID));

    int colIDplot_ref  = _refColumns.value("ID_Plot", -1);
    if (colIDplot_ref < 0) {_plotID = "";}

    QList<CylData*> cylinders;

    QFile fRef(_refFileName);
    if (fRef.exists() && fRef.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream stream(&fRef);
        stream.setLocale(_refLocale);

        int colID  = _refColumns.value("ID_Tree", -1);
        int colX   = _refColumns.value("X", -1);
        int colY   = _refColumns.value("Y", -1);
        int colZmin = _refColumns.value("Zmin", -1);
        int colZmax = _refColumns.value("Zmax", -1);
        int colRadius = _refColumns.value(tr("Rayon"), -1);

        if (colID < 0) {PS_LOG->addMessage(LogInterface::error, LogInterface::step, QString(tr("Champ IDtree non défini")));}
        if (colX < 0) {PS_LOG->addMessage(LogInterface::error, LogInterface::step, QString(tr("Champ X non défini")));}
        if (colY < 0) {PS_LOG->addMessage(LogInterface::error, LogInterface::step, QString(tr("Champ Y non défini")));}
        if (colZmin < 0) {PS_LOG->addMessage(LogInterface::error, LogInterface::step, QString(tr("Champ Zmin non défini")));}
        if (colZmax < 0) {PS_LOG->addMessage(LogInterface::error, LogInterface::step, QString(tr("Champ Zmax non défini")));}
        if (colRadius < 0) {PS_LOG->addMessage(LogInterface::error, LogInterface::step, QString(tr("Champ Rayon non défini")));}


        if (colID >=0 && colX >= 0 && colY >= 0 && colZmin >= 0 && colZmax >= 0 && colRadius >= 0)
        {

            int colMax = colID;
            if (colX   > colMax) {colMax = colX;}
            if (colY   > colMax) {colMax = colY;}
            if (colZmin > colMax) {colMax = colZmin;}
            if (colZmax > colMax) {colMax = colZmax;}
            if (colRadius > colMax) {colMax = colRadius;}
            if (colIDplot_ref > colMax) {colMax = colIDplot_ref;}

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
                        if (colIDplot_ref >= 0)
                        {
                            plot =  values.at(colIDplot_ref);
                        }

                        if (plot == _plotID)
                        {
                            bool okX, okY, okZmin, okZmax, okRadius;
                            double x = _refLocale.toDouble(values.at(colX), &okX);
                            double y = _refLocale.toDouble(values.at(colY), &okY);
                            double zmin = _refLocale.toFloat(values.at(colZmin), &okZmin);
                            double zmax = _refLocale.toFloat(values.at(colZmax), &okZmax);
                            double radius = _refLocale.toFloat(values.at(colRadius), &okRadius);

                            QString id = values.at(colID);

                            if (okX && okY && okZmin && okZmax && okRadius)
                            {
                                cylinders.append(new CylData(id, x, y, zmin, zmax, radius));
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

}
