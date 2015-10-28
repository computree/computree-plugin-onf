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

#include "onf_steploadplotareas.h"

#include "ct_itemdrawable/ct_fileheader.h"
#include "ct_itemdrawable/ct_circle2d.h"
#include "ct_result/ct_resultgroup.h"
#include "ct_result/model/outModel/ct_outresultmodelgroupcopy.h"
#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_view/ct_stepconfigurabledialog.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"

#include "ct_view/ct_asciifilechoicebutton.h"
#include "ct_view/ct_combobox.h"

// Alias for indexing models
#define DEFin_res "res"
#define DEFin_grp "grp"
#define DEFin_header "points"

#include <QFile>
#include <QTextStream>


// Constructor : initialization of parameters
ONF_StepLoadPlotAreas::ONF_StepLoadPlotAreas(CT_StepInitializeData &dataInit) : CT_AbstractStepCanBeAddedFirst(dataInit)
{
    _neededFields.append(CT_TextFileConfigurationFields("ID_Plot", QRegExp("([iI][dD]|[nN][uU][mM]|[pP][lL][oO][tT]|[pP][lL][aA][cC][eE][tT][tT][eE])"), false));
    _neededFields.append(CT_TextFileConfigurationFields("X", QRegExp("[xX]"), false));
    _neededFields.append(CT_TextFileConfigurationFields("Y", QRegExp("[yY]"), false));
    _neededFields.append(CT_TextFileConfigurationFields("R (m)", QRegExp("([rR][aA][dD]|[iI][uU][sS]|[rR][aA][yY]|[oO][nN]|[R])"), false));

    _refFileName = "";
    _refHeader = true;
    _refSeparator = "\t";
    _refDecimal = ".";
    _refLocale = QLocale(QLocale::English, QLocale::UnitedKingdom).name();
    _refSkip = 0;

    _plotID = "";
}

// Step description (tooltip of contextual menu)
QString ONF_StepLoadPlotAreas::getStepDescription() const
{
    return tr("Charge l'emprise de placettes");
}

// Step detailled description
QString ONF_StepLoadPlotAreas::getStepDetailledDescription() const
{
    return tr("No detailled description for this step");
}

// Step URL
QString ONF_StepLoadPlotAreas::getStepURL() const
{
    //return tr("STEP URL HERE");
    return CT_AbstractStepCanBeAddedFirst::getStepURL(); //by default URL of the plugin
}

// Step copy method
CT_VirtualAbstractStep* ONF_StepLoadPlotAreas::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new ONF_StepLoadPlotAreas(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void ONF_StepLoadPlotAreas::createInResultModelListProtected()
{
    CT_InResultModelGroupToCopy *resIn_res = createNewInResultModelForCopy(DEFin_res, tr("Points"));
    resIn_res->setZeroOrMoreRootGroup();
    resIn_res->addGroupModel("", DEFin_grp, CT_AbstractItemGroup::staticGetType(), tr("Groupe"));
    resIn_res->addItemModel(DEFin_grp, DEFin_header, CT_FileHeader::staticGetType(), tr("Entête de fichier"));
}

// Creation and affiliation of OUT models
void ONF_StepLoadPlotAreas::createOutResultModelListProtected()
{
    CT_OutResultModelGroupToCopyPossibilities *resCpy_res = createNewOutResultModelToCopy(DEFin_res);
    resCpy_res->addItemModel(DEFin_grp, _areaModelName, new CT_Circle2D(), tr("Emprise"));
}

// Semi-automatic creation of step parameters DialogBox
void ONF_StepLoadPlotAreas::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();
    configDialog->addAsciiFileChoice("Fichier des arbres", "Fichier ASCII (*.txt ; *.asc)", true, _neededFields, _refFileName, _refHeader, _refSeparator, _refDecimal, _refLocale, _refSkip, _refColumns);
}

void ONF_StepLoadPlotAreas::compute()
{  

    QList<CT_ResultGroup*> outResultList = getOutResultList();
    CT_ResultGroup* outRes = outResultList.at(0);

    QMap<QString, CT_StandardItemGroup*> groups;

    // Création de la liste des id recherchés
    CT_ResultGroupIterator itGrp(outRes, this, DEFin_grp);
    while (itGrp.hasNext() && !isStopped())
    {
        CT_StandardItemGroup* group = (CT_StandardItemGroup*) itGrp.next();

        if (group != NULL)
        {
            CT_FileHeader* fileHeader  = (CT_FileHeader*) group->firstItemByINModelName(this, DEFin_header);

            if (fileHeader != NULL)
            {
                QString id = fileHeader->getFileInfo().baseName();
                groups.insert(id.toLower(), group);
            }
        }
    }

    // Création des emprises présentes dans le fichier
    QFile fRef(_refFileName);
    if (fRef.exists() && fRef.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream stream(&fRef);
        stream.setLocale(_refLocale);

        int colID  = _refColumns.value("ID_Plot", -1);
        int colX   = _refColumns.value("X", -1);
        int colY   = _refColumns.value("Y", -1);
        int colVal = _refColumns.value("R (m)", -1);

        if (colID < 0) {PS_LOG->addMessage(LogInterface::error, LogInterface::step, QString(tr("Champ ID_Plot non défini")));}
        if (colX < 0) {PS_LOG->addMessage(LogInterface::error, LogInterface::step, QString(tr("Champ X non défini")));}
        if (colY < 0) {PS_LOG->addMessage(LogInterface::error, LogInterface::step, QString(tr("Champ Y non défini")));}
        if (colVal < 0) {PS_LOG->addMessage(LogInterface::error, LogInterface::step, QString(tr("Champ R non défini")));}


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

                        bool okX, okY, okVal;
                        double x = _refLocale.toDouble(values.at(colX), &okX);
                        double y = _refLocale.toDouble(values.at(colY), &okY);
                        float val = _refLocale.toFloat(values.at(colVal), &okVal);
                        QString id = values.at(colID);

                        if (okX && okY && okVal)
                        {
                            CT_StandardItemGroup* group = groups.value(id.toLower(), NULL);
                            groups.remove(id.toLower());

                            if (group != NULL)
                            {
                                CT_Circle2D* circle = new CT_Circle2D(_areaModelName.completeName(), outRes, new CT_Circle2DData(Eigen::Vector2d(x,y), val));
                                group->addItemDrawable(circle);
                            }

                        } else {
                            PS_LOG->addMessage(LogInterface::info, LogInterface::step, QString(tr("Ligne %1 du fichier REF non valide")).arg(cpt));
                        }
                    }
                }
            }
        }
        fRef.close();
    }

}
