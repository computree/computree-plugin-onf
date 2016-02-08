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


#include "onf_stepextractdiametersfromcylinders.h"

#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"

#include "ct_result/ct_resultgroup.h"
#include "ct_itemdrawable/ct_cylinder.h"
#include "ct_itemdrawable/ct_circle.h"
#include "ct_itemdrawable/ct_referencepoint.h"
#include "qvector2d.h"
#include "qvector3d.h"

#include "ct_pointcloudindex/ct_pointcloudindexvector.h"
#include "ct_math/ct_mathfittedline2d.h"
#include "ct_view/ct_stepconfigurabledialog.h"
#include "ct_itemdrawable/tools/iterator/ct_groupiterator.h"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <QList>
#include <limits>

#define DEF_SearchInCylinder   "cyl"
#define DEF_SearchInFootCoordinate   "fo"
#define DEF_SearchInCylinderGroup   "cg"
#define DEF_SearchInSectionGroup   "sg"

#define DEF_SearchInResult  "r"
#define DEF_SearchOutResult "r"

ONF_StepExtractDiametersFromCylinders::ONF_StepExtractDiametersFromCylinders(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _h = 1.3;
    _hmin = 1.0;
    _hmax = 1.6;
    _deltaDMax = 0.05;
    _minCylinderNumber = 3;
}

QString ONF_StepExtractDiametersFromCylinders::getStepDescription() const
{
    return tr("7- Calcul d'un DBH par Billon à partir des Cylindres");
}

QString ONF_StepExtractDiametersFromCylinders::getStepDetailledDescription() const
{
    return tr("No detailled description for this step");
}

CT_VirtualAbstractStep* ONF_StepExtractDiametersFromCylinders::createNewInstance(CT_StepInitializeData &dataInit)
{
    // cree une copie de cette etape
    return new ONF_StepExtractDiametersFromCylinders(dataInit);
}

//////////////////// PROTECTED //////////////////

void ONF_StepExtractDiametersFromCylinders::createInResultModelListProtected()
{ 
    CT_InResultModelGroupToCopy * resultModel = createNewInResultModelForCopy(DEF_SearchInResult, tr("Billons contenant des cylindres"));
    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_SearchInSectionGroup, CT_AbstractItemGroup::staticGetType(), tr("Billon (Grp)"));
    resultModel->addItemModel(DEF_SearchInSectionGroup, DEF_SearchInFootCoordinate, CT_ReferencePoint::staticGetType(), tr("Cordonnée MNT"));
    resultModel->addGroupModel(DEF_SearchInSectionGroup, DEF_SearchInCylinderGroup, CT_AbstractItemGroup::staticGetType(), tr("Groupe"));
    resultModel->addItemModel(DEF_SearchInCylinderGroup, DEF_SearchInCylinder, CT_Cylinder::staticGetType(), tr("Cylindre"));
}

void ONF_StepExtractDiametersFromCylinders::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addDouble(tr("Hauteur de référence : "), "m", 0, 1000, 2, _h);
    configDialog->addDouble(tr("Hauteur minimale d'évaluation :"), "m", 0, 1000, 2, _hmin);
    configDialog->addDouble(tr("Hauteur maximale d'évaluation : "), "m", 0, 1000, 2, _hmax);
    configDialog->addDouble(tr("Décroissance métrique maximale : "), "cm", 0, 100, 2, _deltaDMax, 100);
    configDialog->addInt(tr("Nombre de cylindres minimum pour ajuster un cercle : "), "", 2, 100, _minCylinderNumber);
}

void ONF_StepExtractDiametersFromCylinders::createOutResultModelListProtected()
{
    CT_OutResultModelGroupToCopyPossibilities *res = createNewOutResultModelToCopy(DEF_SearchInResult);

    if(res != NULL)
        res->addItemModel(DEF_SearchInSectionGroup, _outCircleModelName, new CT_Circle(), tr("Diamètre à 1.30m"));
}

void ONF_StepExtractDiametersFromCylinders::compute()
{
    // on récupère le résultat copié
    CT_ResultGroup *outRes = getOutResultList().first();


    /**************************************************/
    // on va rechercher tous les groupes contenant des nuages de points (qui ont été choisi par l'utilisateur)
    CT_ResultGroupIterator itSection(outRes, this, DEF_SearchInSectionGroup);

    while (itSection.hasNext())
    {
        CT_AbstractItemGroup *section = (CT_AbstractItemGroup*) itSection.next();

        if ((section != NULL) && !isStopped())
        {
            CT_ReferencePoint* footCoordinate = (CT_ReferencePoint*) section->firstItemByINModelName(this, DEF_SearchInFootCoordinate);

            QList<Eigen::Vector2d*> diameters;
            float deltaHmax = std::max(fabs(_hmax - _h), fabs(_h - _hmin));
            float xSection = footCoordinate->x();
            float ySection = footCoordinate->y();

            if (footCoordinate != NULL)
            {
                CT_GroupIterator itGrp(section, this, DEF_SearchInCylinderGroup);
                while (itGrp.hasNext())
                {
                    const CT_AbstractItemGroup* group = itGrp.next();
                    CT_Cylinder* cylinder = (CT_Cylinder*) group->firstItemByINModelName(this, DEF_SearchInCylinder);
                    if (cylinder!=NULL)
                    {
                        float h_value = cylinder->getCenterZ() - footCoordinate->z();
                        float deltaH = fabs(h_value - _h);
                        if ((h_value >= _hmin) && (h_value <= _hmax))
                        {
                            diameters.append(new Eigen::Vector2d(h_value, cylinder->getRadius()));
                            if (deltaH < deltaHmax)
                            {
                                deltaHmax = deltaH;
                                xSection = cylinder->getCenterX();
                                ySection = cylinder->getCenterY();
                            }
                        }
                    }
                }
            }

            if (diameters.size() >= _minCylinderNumber)
            {
                CT_MathFittedLine2D fittedLine(diameters);

                if (fabs(fittedLine._a) < _deltaDMax)
                {
                    float radius = fittedLine._a*_h + fittedLine._b;

                    CT_CircleData *cData = new CT_CircleData(Eigen::Vector3d(xSection, ySection, _h + footCoordinate->z()), Eigen::Vector3d(0, 0, 1), radius);
                    if(cData != NULL)
                    {
                        section->addItemDrawable(new CT_Circle(_outCircleModelName.completeName(), outRes, cData));
                    }
                }
            }

            qDeleteAll(diameters);
            diameters.clear();
        }
    }

}

