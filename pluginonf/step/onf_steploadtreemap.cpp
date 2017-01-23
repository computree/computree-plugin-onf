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

#include "onf_steploadtreemap.h"

#include "ct_itemdrawable/ct_circle2d.h"
#include "ct_result/ct_resultgroup.h"
#include "ct_result/model/inModel/ct_inresultmodelgroup.h"
#include "ct_result/model/outModel/ct_outresultmodelgroup.h"
#include "ct_view/ct_stepconfigurabledialog.h"

#include "ct_view/ct_asciifilechoicebutton.h"
#include "ct_view/ct_combobox.h"

#include "ct_itemdrawable/ct_loopcounter.h"

#define DEF_inResult "r"
#define DEF_inGroup "g"
#define DEF_inCounter "c"


// Alias for indexing models
#define DEFout_refRes "refRes"
#define DEFout_grpRef "grpRef"
#define DEFout_ref "ref"
#define DEFout_refDbh "refval"
#define DEFout_refHeight "refHeight"
#define DEFout_refID "refID"
#define DEFout_refIDplot "refIDplot"

#include <QFile>
#include <QTextStream>


// Constructor : initialization of parameters
ONF_StepLoadTreeMap::ONF_StepLoadTreeMap(CT_StepInitializeData &dataInit) : CT_AbstractStepCanBeAddedFirst(dataInit)
{
    _mode = 1;
    _neededFields.append(CT_TextFileConfigurationFields("ID_Plot", QRegExp("([iI][dD]|[nN][uU][mM]|[pP][lL][oO][tT]|[pP][lL][aA][cC][eE][tT][tT][eE])"), false));
    _neededFields.append(CT_TextFileConfigurationFields("ID_Tree", QRegExp("([iI][dD]|[nN][uU][mM])"), false));
    _neededFields.append(CT_TextFileConfigurationFields("X", QRegExp("[xX]"), false));
    _neededFields.append(CT_TextFileConfigurationFields("Y", QRegExp("[yY]"), false));
    _neededFields.append(CT_TextFileConfigurationFields("DBH (cm)", QRegExp("([dD][bB][hH]|[dD][iI][aA][mM]|[D])"), false));
    _neededFields.append(CT_TextFileConfigurationFields("H (m)", QRegExp("([hH]*|[hH][eE][iI][gG][hH][tT]|[hH][aA][uU][tT][eE][uU][uR])"), false));

    _refFileName = "";
    _refHeader = true;
    _refSeparator = "\t";
    _refDecimal = ".";
    _refLocale = QLocale(QLocale::English, QLocale::UnitedKingdom).name();
    _refSkip = 0;

    _plotID = "";
}

// Step description (tooltip of contextual menu)
QString ONF_StepLoadTreeMap::getStepDescription() const
{
    return tr("Placette d'inventaire forestier (Tree Map)");
}

// Step detailled description
QString ONF_StepLoadTreeMap::getStepDetailledDescription() const
{
    return tr("Charge des données d'inventaire forestier depuis un fichier ASCII. <br>"
              "L'import est configurable, le fichier devant contenir les champs suivants :<br>"
              "- IDplot : Identifiant placette<br>"
              "- IDtree : Identifiant arbre<br>"
              "- X      : Coordonnée X de l'arbre<br>"
              "- Y      : Coordonnée Y de l'arbre<br>"
              "- DBH    : Diamètre à 1.30 m de l'arbre<br>"
              "- H      : Hauteur de l'arbre<br>"
              "<br>Une fois le format de fichier paramétré, l'utilisateur indique quelle placette doit être chargée.<br>"
              "Seules les données de la placette choisie seront chargées.");
}

// Step URL
QString ONF_StepLoadTreeMap::getStepURL() const
{
    //return tr("STEP URL HERE");
    return CT_AbstractStepCanBeAddedFirst::getStepURL(); //by default URL of the plugin
}

bool ONF_StepLoadTreeMap::setAllSettings(const SettingsNodeGroup *settings)
{
    if(CT_AbstractStepCanBeAddedFirst::setAllSettings(settings))
    {
        fileChanged();
        return true;
    }

    return false;
}

// Step copy method
CT_VirtualAbstractStep* ONF_StepLoadTreeMap::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new ONF_StepLoadTreeMap(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

void ONF_StepLoadTreeMap::createPreConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPreConfigurationDialog();

    CT_ButtonGroup &bg_mode = configDialog->addButtonGroup(_mode);

    configDialog->addExcludeValue("", "", tr("IDplacette à partir du nom de tour (boucles)"), bg_mode, 0);
    configDialog->addExcludeValue("", "", tr("Sélection manuelle de l'IDplacette"), bg_mode, 1);

}


// Creation and affiliation of IN models
void ONF_StepLoadTreeMap::createInResultModelListProtected()
{
    if (_mode == 0)
    {
        CT_InResultModelGroup* res = createNewInResultModel(DEF_inResult, tr("Résultat compteur"), "", true);
        res->setRootGroup(DEF_inGroup);
        res->addItemModel(DEF_inGroup, DEF_inCounter, CT_LoopCounter::staticGetType(), tr("Compteur"));

    } else {
        setNotNeedInputResult();
    }
}

// Creation and affiliation of OUT models
void ONF_StepLoadTreeMap::createOutResultModelListProtected()
{
    CT_OutResultModelGroup *res_refRes = createNewOutResultModel(DEFout_refRes, tr("Positions de référence"));
    res_refRes->setRootGroup(DEFout_grpRef, new CT_StandardItemGroup(), tr("Groupe"));
    res_refRes->addItemModel(DEFout_grpRef, DEFout_ref, new CT_Circle2D(), tr("Position de référence"));
    res_refRes->addItemAttributeModel(DEFout_ref, DEFout_refDbh, new CT_StdItemAttributeT<float>(CT_AbstractCategory::DATA_NUMBER), tr("DBH"));
    res_refRes->addItemAttributeModel(DEFout_ref, DEFout_refHeight, new CT_StdItemAttributeT<float>(CT_AbstractCategory::DATA_NUMBER), tr("Height"));
    res_refRes->addItemAttributeModel(DEFout_ref, DEFout_refID, new CT_StdItemAttributeT<QString>(CT_AbstractCategory::DATA_ID), tr("IDtree"));
    res_refRes->addItemAttributeModel(DEFout_ref, DEFout_refIDplot, new CT_StdItemAttributeT<QString>(CT_AbstractCategory::DATA_ID), tr("IDplot"));
}

// Semi-automatic creation of step parameters DialogBox
void ONF_StepLoadTreeMap::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    CT_AsciiFileChoiceButton *fileChoice = configDialog->addAsciiFileChoice("Fichier des arbres", "Fichier ASCII (*.txt ; *.asc)", true, _neededFields, _refFileName, _refHeader, _refSeparator, _refDecimal, _refLocale, _refSkip, _refColumns);

    if (_mode == 1)
    {
        CT_ComboBox *cbox = configDialog->addStringChoice(tr("Choix de la placette"), "", QStringList(), _plotID);

        connect(configDialog, SIGNAL(openned()), this, SLOT(fileChanged()));
        connect(fileChoice, SIGNAL(fileChanged()), this, SLOT(fileChanged()));
        connect(this, SIGNAL(updateComboBox(QStringList, QString)), cbox, SLOT(changeValues(QStringList, QString)));
    }
}

void ONF_StepLoadTreeMap::fileChanged()
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

void ONF_StepLoadTreeMap::compute()
{  
    QString turnName = "";
    if (_mode == 0)
    {
        CT_ResultGroup* res = getInputResults().first();

        CT_ResultItemIterator it(res, this, DEF_inCounter);
        if (it.hasNext())
        {
            CT_LoopCounter* counter = (CT_LoopCounter*) it.next();
            QFileInfo fileInfo(counter->getTurnName());
            turnName = fileInfo.baseName();
        }

        if (turnName != "") {_plotID = turnName;}
    }


    QList<CT_ResultGroup*> outResultList = getOutResultList();
    CT_ResultGroup* res_refRes = outResultList.at(0);

    PS_LOG->addMessage(LogInterface::info, LogInterface::step, QString(tr("Placette en cours de traitement : %1")).arg(_plotID));

    int colIDplot_ref  = _refColumns.value("ID_Plot", -1);
    if (colIDplot_ref < 0) {_plotID = "";}

    QFile fRef(_refFileName);
    if (fRef.exists() && fRef.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream stream(&fRef);
        stream.setLocale(_refLocale);

        int colID  = _refColumns.value("ID_Tree", -1);
        int colX   = _refColumns.value("X", -1);
        int colY   = _refColumns.value("Y", -1);
        int colVal = _refColumns.value("DBH (cm)", -1);
        int colHeight = _refColumns.value("H (m)", -1);

        if (colID < 0) {PS_LOG->addMessage(LogInterface::error, LogInterface::step, QString(tr("Champ IDtree non défini")));}
        if (colX < 0) {PS_LOG->addMessage(LogInterface::error, LogInterface::step, QString(tr("Champ X non défini")));}
        if (colY < 0) {PS_LOG->addMessage(LogInterface::error, LogInterface::step, QString(tr("Champ Y non défini")));}
        if (colVal < 0) {PS_LOG->addMessage(LogInterface::error, LogInterface::step, QString(tr("Champ DBH non défini")));}
        if (colHeight < 0) {PS_LOG->addMessage(LogInterface::error, LogInterface::step, QString(tr("Champ H non défini")));}


        if (colID >=0 && colX >= 0 && colY >= 0 && colVal >= 0)
        {

            int colMax = colID;
            if (colX   > colMax) {colMax = colX;}
            if (colY   > colMax) {colMax = colY;}
            if (colVal > colMax) {colMax = colVal;}
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
                            bool okX, okY, okVal, okHeight;
                            double x = _refLocale.toDouble(values.at(colX), &okX);
                            double y = _refLocale.toDouble(values.at(colY), &okY);
                            float val = _refLocale.toFloat(values.at(colVal), &okVal);

                            float height = -1;
                            if (colHeight >= 0)
                            {
                                height = _refLocale.toFloat(values.at(colHeight), &okHeight);
                                if (!okHeight) {height = -1;}
                            }

                            QString id = values.at(colID);

                            if (okX && okY && okVal)
                            {
                                CT_StandardItemGroup* grp_grpRef= new CT_StandardItemGroup(DEFout_grpRef, res_refRes);
                                res_refRes->addGroup(grp_grpRef);

                                CT_Circle2D* item_ref = new CT_Circle2D(DEFout_ref, res_refRes, new CT_Circle2DData(Eigen::Vector2d(x,y), val/200.0));
                                grp_grpRef->addItemDrawable(item_ref);

                                item_ref->addItemAttribute(new CT_StdItemAttributeT<float>(DEFout_refDbh, CT_AbstractCategory::DATA_NUMBER, res_refRes, val));
                                item_ref->addItemAttribute(new CT_StdItemAttributeT<float>(DEFout_refHeight, CT_AbstractCategory::DATA_NUMBER, res_refRes, height));
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

}
