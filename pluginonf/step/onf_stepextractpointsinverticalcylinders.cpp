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

#include "ct_result/ct_resultgroup.h"
#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"
#include "ct_view/ct_stepconfigurabledialog.h"
#include "ct_iterator/ct_pointiterator.h"
#include "ct_pointcloudindex/ct_pointcloudindexvector.h"

#include "ct_view/ct_asciifilechoicebutton.h"
#include "ct_view/ct_combobox.h"

// Alias for indexing models
#define DEF_SearchInResult "inres"
#define DEF_SearchInGroup "ingrp"
#define DEF_SearchInScene "inscene"
#define DEF_SearchInFileNameItem "initem"
#define DEF_SearchInFileName "infilename"

#include <QFile>
#include <QTextStream>
#include <QFileInfo>

#include <QDebug>


// Constructor : initialization of parameters
ONF_StepExtractPointsInVerticalCylinders::ONF_StepExtractPointsInVerticalCylinders(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _neededFields.append(CT_TextFileConfigurationFields("ID_Plot", QRegExp("([iI][dD]|[nN][uU][mM]|[pP][lL][oO][tT]|[pP][lL][aA][cC][eE][tT][tT][eE])"), true));
    _neededFields.append(CT_TextFileConfigurationFields("ID_Tree", QRegExp("([iI][dD]|[nN][uU][mM])"), false));
    _neededFields.append(CT_TextFileConfigurationFields("X", QRegExp("[xX]"), false));
    _neededFields.append(CT_TextFileConfigurationFields("Y", QRegExp("[yY]"), false));
    _neededFields.append(CT_TextFileConfigurationFields("Z", QRegExp("[yZ]"), false));
    _neededFields.append(CT_TextFileConfigurationFields("Zmin", QRegExp("([Zz][Mm][Ii][Nn]|[Hh][Mm][Ii][Nn])"), false));
    _neededFields.append(CT_TextFileConfigurationFields("Zmax", QRegExp("([Zz][Mm][Aa][Xx]|[Hh][Mm][Aa][Xx])"), false));
    _neededFields.append(CT_TextFileConfigurationFields(tr("Rayon"), QRegExp("([Rr][Aa][Dd][Ii][Uu][Ss]|[Rr][Aa][Yy][Oo][Nn]|[Rr])"), false));

    _refFileName = "";
    _refHeader = true;
    _refSeparator = "\t";
    _refDecimal = ".";
    _refLocale = QLocale(QLocale::English, QLocale::UnitedKingdom).name();
    _refSkip = 0;

    _translate = false;
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

    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_SearchInGroup);
    resultModel->addItemModel(DEF_SearchInGroup, DEF_SearchInScene, CT_Scene::staticGetType(), tr("Scène"));
    resultModel->addItemModel(DEF_SearchInGroup, DEF_SearchInFileNameItem, CT_AbstractSingularItemDrawable::staticGetType(), tr("Item avec nom de fichier"));
    resultModel->addItemAttributeModel(DEF_SearchInFileNameItem, DEF_SearchInFileName, QList<QString>() << CT_AbstractCategory::DATA_VALUE, CT_AbstractCategory::STRING, tr("Nom de fichier"));
}

// Creation and affiliation of OUT models
void ONF_StepExtractPointsInVerticalCylinders::createOutResultModelListProtected()
{
    CT_OutResultModelGroupToCopyPossibilities *res = createNewOutResultModelToCopy(DEF_SearchInResult);

    res->addGroupModel(DEF_SearchInGroup, _outSceneGroupModelName);
    res->addItemModel(_outSceneGroupModelName, _outSceneModelName, new CT_Scene(), tr("Extracted Scene"));
    res->addItemAttributeModel(_outSceneModelName, _outAttIDModelName, new CT_StdItemAttributeT<QString>(CT_AbstractCategory::DATA_VALUE), tr("PlotID"));
}

// Semi-automatic creation of step parameters DialogBox
void ONF_StepExtractPointsInVerticalCylinders::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addAsciiFileChoice(tr("Fichier des cylindres par placette"), tr("Fichier ASCII (*.txt ; *.asc)"), true, _neededFields, _refFileName, _refHeader, _refSeparator, _refDecimal, _refLocale, _refSkip, _refColumns);
    configDialog->addBool(tr("Appliquer translation"), "", "", _translate);
}


void ONF_StepExtractPointsInVerticalCylinders::compute()
{  

    QList<CT_ResultGroup*> outResultList = getOutResultList();
    CT_ResultGroup* res = outResultList.at(0);

    // Listing available plots scenes
    QList<QString> plotIds;
    CT_ResultGroupIterator it(res, this, DEF_SearchInGroup);
    while(!isStopped() && it.hasNext())
    {
        CT_StandardItemGroup* group = (CT_StandardItemGroup*) it.next();
        const CT_AbstractSingularItemDrawable *in_item = (CT_AbstractSingularItemDrawable*) group->firstItemByINModelName(this, DEF_SearchInFileNameItem);

        if (in_item != NULL)
        {
            QString attributeValue = in_item->firstItemAttributeByINModelName(res, this, DEF_SearchInFileName)->toString(in_item, NULL);
            QFileInfo fileInfo(attributeValue);
            plotIds.append(fileInfo.baseName());
        }
    }


    QMultiMap<QString, CylData*> cylinders;

    QFile fRef(_refFileName);
    if (fRef.exists() && fRef.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream stream(&fRef);
        stream.setLocale(_refLocale);

        int colIDplot_ref  = _refColumns.value("ID_Plot", -1);
        int colID  = _refColumns.value("ID_Tree", -1);
        int colX   = _refColumns.value("X", -1);
        int colY   = _refColumns.value("Y", -1);
        int colZ   = _refColumns.value("Z", -1);
        int colZmin = _refColumns.value("Zmin", -1);
        int colZmax = _refColumns.value("Zmax", -1);
        int colRadius = _refColumns.value(tr("Rayon"), -1);

        if (colIDplot_ref < 0) {PS_LOG->addMessage(LogInterface::info, LogInterface::step, QString(tr("Champ IDplot non défini (non bloquant)")));}
        if (colID < 0) {PS_LOG->addMessage(LogInterface::error, LogInterface::step, QString(tr("Champ IDtree non défini")));}
        if (colX < 0) {PS_LOG->addMessage(LogInterface::error, LogInterface::step, QString(tr("Champ X non défini")));}
        if (colY < 0) {PS_LOG->addMessage(LogInterface::error, LogInterface::step, QString(tr("Champ Y non défini")));}
        if (colZ < 0) {PS_LOG->addMessage(LogInterface::error, LogInterface::step, QString(tr("Champ Z non défini")));}
        if (colZmin < 0) {PS_LOG->addMessage(LogInterface::error, LogInterface::step, QString(tr("Champ Zmin non défini")));}
        if (colZmax < 0) {PS_LOG->addMessage(LogInterface::error, LogInterface::step, QString(tr("Champ Zmax non défini")));}
        if (colRadius < 0) {PS_LOG->addMessage(LogInterface::error, LogInterface::step, QString(tr("Champ Rayon non défini")));}


        if (colID >=0 && colX >= 0 && colY >= 0 && colZ >= 0 && colZmin >= 0 && colZmax >= 0 && colRadius >= 0)
        {
            int colMax = colID;
            if (colX   > colMax) {colMax = colX;}
            if (colY   > colMax) {colMax = colY;}
            if (colZ   > colMax) {colMax = colZ;}
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

                        if (plot == "" || plotIds.contains(plot))
                        {
                            bool okX, okY, okZ, okZmin, okZmax, okRadius;
                            double x = _refLocale.toDouble(values.at(colX), &okX);
                            double y = _refLocale.toDouble(values.at(colY), &okY);
                            double z = _refLocale.toDouble(values.at(colZ), &okZ);
                            double zmin = _refLocale.toFloat(values.at(colZmin), &okZmin);
                            double zmax = _refLocale.toFloat(values.at(colZmax), &okZmax);
                            double radius = _refLocale.toFloat(values.at(colRadius), &okRadius);

                            QString id = values.at(colID);

                            if (okX && okY && okZ && okZmin && okZmax && okRadius)
                            {
                                cylinders.insert(plot, new CylData(id, x, y, z, zmin, zmax, radius));
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


    CT_ResultGroupIterator it2(res, this, DEF_SearchInGroup);
    while(!isStopped() && it2.hasNext())
    {
        CT_StandardItemGroup* group = (CT_StandardItemGroup*) it2.next();
        const CT_Scene *in_scene = (CT_Scene*) group->firstItemByINModelName(this, DEF_SearchInScene);
        const CT_AbstractSingularItemDrawable *in_item = (CT_AbstractSingularItemDrawable*) group->firstItemByINModelName(this, DEF_SearchInFileNameItem);

        if (in_scene != NULL && in_item != NULL)
        {
            QString attributeValue = in_item->firstItemAttributeByINModelName(res, this, DEF_SearchInFileName)->toString(in_item, NULL);
            QFileInfo fileInfo(attributeValue);
            QString plotName = fileInfo.baseName();

            if (_translate)
            {
                extractCylindersWithTranslation(group, cylinders, in_scene, res, plotName);
            } else {
                extractCylindersWithoutTranslation(group, cylinders, in_scene, res, plotName);
            }
        }
    }
}


void ONF_StepExtractPointsInVerticalCylinders::extractCylindersWithoutTranslation(CT_StandardItemGroup* group, QMultiMap<QString, CylData*> cylinders, const CT_Scene *in_scene, CT_ResultGroup* res, QString plotName)
{
    QMap<CylData*, CT_PointCloudIndexVector *> cloudIndexVectors;
    QList<CylData*> cylindersForActivePlot = cylinders.values(plotName);
    for (int i = 0 ; i < cylindersForActivePlot.size() ; i++)
    {
        cloudIndexVectors.insert(cylindersForActivePlot.at(i), new CT_PointCloudIndexVector());
    }

    if (cylindersForActivePlot.size() > 0)
    {
        QMapIterator<CylData*, CT_PointCloudIndexVector *> itCl(cloudIndexVectors);

        const CT_AbstractPointCloudIndex *pointCloudIndex = in_scene->getPointCloudIndex();
        CT_PointIterator itP(pointCloudIndex);
        while(itP.hasNext())
        {
            const CT_Point &point = itP.next().currentPoint();
            size_t index = itP.currentGlobalIndex();

            itCl.toFront();
            while (itCl.hasNext())
            {
                itCl.next();

                if (itCl.key()->contains(point(0), point(1), point(2)))
                {
                    itCl.value()->addIndex(index);;
                }
            }
        }

        itCl.toFront();
        while (itCl.hasNext())
        {
            itCl.next();

            // creation du groupe
            CT_StandardItemGroup *outGroup = new CT_StandardItemGroup(_outSceneGroupModelName.completeName(), res);

            // creation et ajout de la scene
            CT_Scene *outScene = new CT_Scene(_outSceneModelName.completeName(), res, PS_REPOSITORY->registerPointCloudIndex(itCl.value()));
            outScene->updateBoundingBox();

            outScene->addItemAttribute(new CT_StdItemAttributeT<QString>(_outAttIDModelName.completeName(), CT_AbstractCategory::DATA_VALUE, res, plotName));

            outGroup->addItemDrawable(outScene);
            group->addGroup(outGroup);
        }
    }
}

void ONF_StepExtractPointsInVerticalCylinders::extractCylindersWithTranslation(CT_StandardItemGroup* group, QMultiMap<QString, CylData*> cylinders, const CT_Scene *in_scene, CT_ResultGroup* res, QString plotName)
{
    QList<CylData*> cylindersForActivePlot = cylinders.values(plotName);
    for (int i = 0 ; i < cylindersForActivePlot.size() ; i++)
    {
        CylData* cyl = cylindersForActivePlot.at(i);
        CT_AbstractUndefinedSizePointCloud *cloud = PS_REPOSITORY->createNewUndefinedSizePointCloud();

        const CT_AbstractPointCloudIndex *pointCloudIndex = in_scene->getPointCloudIndex();
        CT_PointIterator itP(pointCloudIndex);
        while(itP.hasNext())
        {
            const CT_Point &point = itP.next().currentPoint();

            if (cyl->contains(point(0), point(1), point(2)))
            {
                cloud->addPoint(Eigen::Vector3d(point(0) - cyl->_x, point(1) - cyl->_y, point(2) - cyl->_z));
            }
        }

        // creation du groupe
        CT_StandardItemGroup *outGroup = new CT_StandardItemGroup(_outSceneGroupModelName.completeName(), res);

        // creation et ajout de la scene
        CT_Scene *outScene = new CT_Scene(_outSceneModelName.completeName(), res, PS_REPOSITORY->registerUndefinedSizePointCloud(cloud));
        outScene->updateBoundingBox();

        outScene->addItemAttribute(new CT_StdItemAttributeT<QString>(_outAttIDModelName.completeName(), CT_AbstractCategory::DATA_VALUE, res, plotName));

        outGroup->addItemDrawable(outScene);
        group->addGroup(outGroup);
    }
}



