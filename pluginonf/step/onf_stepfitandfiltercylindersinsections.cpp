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


#include "onf_stepfitandfiltercylindersinsections.h"

#include "tools/onf_citations.h"

#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"

#include "ct_itemdrawable/ct_pointcluster.h"
#include "ct_itemdrawable/ct_referencepoint.h"
#include "ct_itemdrawable/ct_cylinder.h"
#include "ct_itemdrawable/ct_line.h"
#include "ct_itemdrawable/ct_circle.h"
#include "ct_math/ct_sphericalline3d.h"
#include "ct_view/ct_stepconfigurabledialog.h"
#include "ct_result/ct_resultgroup.h"
#include "ct_itemdrawable/tools/iterator/ct_groupiterator.h"

#include "qdebug.h"

#define DEF_SearchInRefPoint  "rp"
#define DEF_SearchInPointCluster  "pc"
#define DEF_SearchInGroup       "g"
#define DEF_SearchInSection       "s"
#define DEF_SearchInResult      "r"
#define DEF_SearchOutResult     "r"

ONF_StepFitAndFilterCylindersInSections::ONF_StepFitAndFilterCylindersInSections(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _max_error = 0.04;
    _max_relative_error = 0.30;
    _min_radius = 0.02;
    _max_radius = 0.80;
    _phi_max = 30;
    _activeFiltering = true;
    _activeFilteringRelative = true;
    _activeFilteringVerticality = true;

    _debugInfo.line = NULL;
    _debugInfo.circle = NULL;

    setDebuggable(true);
}

void ONF_StepFitAndFilterCylindersInSections::setDocuments(QList<DocumentInterface*> docList)
{
    if(docList.isEmpty())
        _debugInfo.currentDocument = NULL;
    else
        _debugInfo.currentDocument = docList.first();
}

QString ONF_StepFitAndFilterCylindersInSections::getStepDescription() const
{
    return tr("6- Ajuster des Cylindres par Clusters/Billons");
}

QString ONF_StepFitAndFilterCylindersInSections::getStepDetailledDescription() const
{
    return tr("No detailled description for this step");
}

QStringList ONF_StepFitAndFilterCylindersInSections::getStepRISCitations() const
{
    return QStringList() << ONF_citations::citation()._citationOthmaniEtAl2001;
}


CT_VirtualAbstractStep* ONF_StepFitAndFilterCylindersInSections::createNewInstance(CT_StepInitializeData &dataInit)
{
    // cree une copie de cette etape
    return new ONF_StepFitAndFilterCylindersInSections(dataInit);
}

//////////////////// PROTECTED //////////////////

void ONF_StepFitAndFilterCylindersInSections::createInResultModelListProtected()
{
    CT_InResultModelGroupToCopy * resultModel = createNewInResultModelForCopy(DEF_SearchInResult, tr("Billons"));
    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_SearchInSection, CT_AbstractItemGroup::staticGetType(), tr("Billon (Grp)"));
    resultModel->addGroupModel(DEF_SearchInSection, DEF_SearchInGroup, CT_AbstractItemGroup::staticGetType(), tr("Groupe"));
    resultModel->addItemModel(DEF_SearchInGroup, DEF_SearchInPointCluster, CT_PointCluster::staticGetType(), tr("Points"));
    resultModel->addItemModel(DEF_SearchInGroup, DEF_SearchInRefPoint, CT_ReferencePoint::staticGetType(), tr("Point de référence"));
}

void ONF_StepFitAndFilterCylindersInSections::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addDouble(tr("Rayon minimum  :"), "cm", 0, 1000, 2, _min_radius, 100);
    configDialog->addDouble(tr("Rayon maximum  :"), "cm", 0, 1000, 2, _max_radius, 100);
    configDialog->addBool("", "", tr("Filtrer les cylindres sur l'erreur absolue"), _activeFiltering);
    configDialog->addDouble(tr("Erreur maximum :"), "cm", 0, 1000, 2, _max_error, 100);
    configDialog->addBool("", "", tr("Filtrer les cylindres sur l'erreur relative"), _activeFilteringRelative);
    configDialog->addDouble(tr("Erreur maximum relative au diamètre :"), "%", 0, 100, 2, _max_relative_error, 100);
    configDialog->addBool("", "", tr("Filtrer les cylindres sur leur verticalité"), _activeFilteringVerticality);
    configDialog->addDouble(tr("Angle maximal à la verticale (depuis de zénith) :"), "°", 0, 180, 2, _phi_max);
}

void ONF_StepFitAndFilterCylindersInSections::createOutResultModelListProtected()
{
    CT_OutResultModelGroupToCopyPossibilities *res = createNewOutResultModelToCopy(DEF_SearchInResult);

    if(res != NULL)
        res->addItemModel(DEF_SearchInGroup, _outCylinderModelName, new CT_Cylinder(), tr("Cylindre"));
}

void ONF_StepFitAndFilterCylindersInSections::compute()
{
    // on récupère le résultat copié
    CT_ResultGroup *outRes = getOutResultList().first();

    double phi_rad = M_PI*_phi_max/180;

    CT_ResultGroupIterator itSection(outRes, this, DEF_SearchInSection);
    while (itSection.hasNext() && (!isStopped()))
    {
        const CT_AbstractItemGroup *section = itSection.next();

        QList<CT_ReferencePoint*> refPoints;
        QMap<CT_ReferencePoint*, CT_PointCluster*> clusters;
        QMap<CT_ReferencePoint*, CT_AbstractItemGroup*> groups;

        CT_GroupIterator itGroup(section, this, DEF_SearchInGroup);
        while (itGroup.hasNext() && (!isStopped()))
        {
            CT_AbstractItemGroup *group = (CT_AbstractItemGroup*) itGroup.next();

            CT_ReferencePoint* refPoint = (CT_ReferencePoint*) group->firstItemByINModelName(this, DEF_SearchInRefPoint);
            CT_PointCluster* pointCluster = (CT_PointCluster*) group->firstItemByINModelName(this, DEF_SearchInPointCluster);
            refPoints.append(refPoint);
            clusters.insert(refPoint, pointCluster);
            groups.insert(refPoint, group);

        }

        int size = refPoints.size();
        int i = 0;

        // parcours des points de référence de la section
        while((i < size) && !isStopped())
        {
            // liste contenant le point d'avant, le point et le point d'après
            QList<CT_AbstractSingularItemDrawable*> refPointsForDirection;
            if (i>0) {refPointsForDirection.append(refPoints.at(i-1));}
            refPointsForDirection.append(refPoints.at(i));
            if (i<(size-1)) {refPointsForDirection.append(refPoints.at(i+1));}

            // ajustement de la ligne de direction
            CT_LineData *direction = CT_LineData::staticCreateLineDataFromItemCenters(refPointsForDirection);
            // on récupère le groupe de point
            const CT_PointCluster *item = clusters.value(refPoints.at(i));

            CT_CircleData circleData;

            // ajustement du cylindre
            CT_CylinderData *cData = CT_CylinderData::staticCreate3DCylinderDataFromPointCloudAndDirection(*item->getPointCloudIndex(),
                                                                                                           Eigen::Vector3d(item->getBarycenter().x(),
                                                                                                                           item->getBarycenter().y(),
                                                                                                                           item->getBarycenter().z()),
                                                                                                           *direction,
                                                                                                           &circleData);

            // et on ajoute un CT_cylinder si le cylindre ajsté existe
            if(cData != NULL)
            {
                float phi = 0;
                float theta = 0;
                float length = 0;
                const Eigen::Vector3d &cylDirection = cData->getDirection();
                CT_SphericalLine3D::convertToSphericalCoordinates(cylDirection(0), cylDirection(1), cylDirection(2), phi, theta, length);

                if ((cData->getRadius() > _min_radius) &&
                        (cData->getRadius() < _max_radius) &&
                        ((!_activeFiltering)  || (cData->getCircleError() < _max_error)) &&
                        ((!_activeFilteringRelative)  || (cData->getCircleError()/(2*cData->getRadius()) < _max_relative_error)) &&
                        ((!_activeFilteringVerticality)  || (phi < phi_rad)))
                {
                    CT_Cylinder *cyl = new CT_Cylinder(_outCylinderModelName.completeName(),outRes,cData);

                    if(isDebugModeOn())
                    {
                        _debugInfo.refP = refPointsForDirection;
                        _debugInfo.lineData = direction;
                        _debugInfo.circleData = &circleData;
                        _debugInfo.cylinder = cyl;
                        _debugInfo.pC = (CT_PointCluster*)item;
                    }

                    waitForAckIfInDebugMode();

                    groups.value(refPoints.at(i))->addItemDrawable(cyl);
                }
                else
                {
                    delete cData;
                }
            }

            delete direction;
            ++i;
        }


    }
}

void ONF_StepFitAndFilterCylindersInSections::preWaitForAckIfInDebugMode()
{
    emit showMessage(QString("Debug : %1 / P1 = %2;%3;%4 / P2=%5;%6;%7").arg(currentDebugBreakPointNumber())
                     .arg(_debugInfo.lineData->getP1().x())
                     .arg(_debugInfo.lineData->getP1().y())
                     .arg(_debugInfo.lineData->getP1().z())
                     .arg(_debugInfo.lineData->getP2().x())
                     .arg(_debugInfo.lineData->getP2().y())
                     .arg(_debugInfo.lineData->getP2().z()));

    if(_debugInfo.currentDocument != NULL)
    {
        _debugInfo.line = new CT_Line(NULL, NULL, _debugInfo.lineData->clone());
        _debugInfo.circle = new CT_Circle(NULL, NULL, _debugInfo.circleData->clone());

        _debugInfo.currentDocument->addItemDrawable(*_debugInfo.line);
        _debugInfo.currentDocument->addItemDrawable(*_debugInfo.circle);

        QListIterator<CT_AbstractSingularItemDrawable*> it(_debugInfo.refP);

        while(it.hasNext())
        {
            CT_ReferencePoint *point = (CT_ReferencePoint*) it.next();
            _debugInfo.currentDocument->addItemDrawable(*point);
        }

        _debugInfo.currentDocument->addItemDrawable(*_debugInfo.cylinder);
        _debugInfo.currentDocument->addItemDrawable(*_debugInfo.pC);

        _debugInfo.currentDocument->redrawGraphics();
    }
}

void ONF_StepFitAndFilterCylindersInSections::postWaitForAckIfInDebugMode()
{
    emit showMessage("");

    if(_debugInfo.currentDocument != NULL)
    {
        _debugInfo.currentDocument->removeItemDrawable(*_debugInfo.line);
        _debugInfo.currentDocument->removeItemDrawable(*_debugInfo.circle);

        QListIterator<CT_AbstractSingularItemDrawable*> it(_debugInfo.refP);

        while(it.hasNext())
        {
            CT_ReferencePoint *point = (CT_ReferencePoint*) it.next();
            _debugInfo.currentDocument->removeItemDrawable(*point);
        }

        _debugInfo.currentDocument->removeItemDrawable(*_debugInfo.cylinder);
        _debugInfo.currentDocument->removeItemDrawable(*_debugInfo.pC);

        _debugInfo.currentDocument->redrawGraphics();
    }

    delete _debugInfo.circle;
    _debugInfo.circle = NULL;

    delete _debugInfo.line;
    _debugInfo.line = NULL;
}
