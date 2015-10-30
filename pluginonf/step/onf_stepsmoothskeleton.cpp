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


#include "onf_stepsmoothskeleton.h"

#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"

#include "ct_itemdrawable/ct_referencepoint.h"
#include "ct_shapedata/ct_linedata.h"

#include "ct_itemdrawable/tools/iterator/ct_groupiterator.h"

#include "ct_result/ct_resultgroup.h"
#include "qdebug.h"

#define DEF_SearchInRefPoint  "rp"
#define DEF_SearchInGroup       "g"
#define DEF_SearchInSection       "s"
#define DEF_SearchInResult      "r"
#define DEF_SearchOutResult     "r"


ONF_StepSmoothSkeleton::ONF_StepSmoothSkeleton(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
}

QString ONF_StepSmoothSkeleton::getStepDescription() const
{
    return tr("Lisser une séquence de Points de référence");
}

QString ONF_StepSmoothSkeleton::getStepDetailledDescription() const
{
    return tr("No detailled description for this step");
}

CT_VirtualAbstractStep* ONF_StepSmoothSkeleton::createNewInstance(CT_StepInitializeData &dataInit)
{
    // cree une copie de cette etape
    return new ONF_StepSmoothSkeleton(dataInit);
}

//////////////////// PROTECTED //////////////////

void ONF_StepSmoothSkeleton::createInResultModelListProtected()
{
    CT_InResultModelGroupToCopy* resultModel = createNewInResultModelForCopy(DEF_SearchInResult, tr("Billons / Clusters / Points de référence"));
    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_SearchInSection, CT_AbstractItemGroup::staticGetType(), tr("Billon (Grp)"));
    resultModel->addGroupModel(DEF_SearchInSection, DEF_SearchInGroup, CT_AbstractItemGroup::staticGetType(), tr("Cluster (Grp)"));
    resultModel->addItemModel(DEF_SearchInGroup, DEF_SearchInRefPoint, CT_ReferencePoint::staticGetType(), tr("Point de référence"));
}

void ONF_StepSmoothSkeleton::createPostConfigurationDialog()
{
}

void ONF_StepSmoothSkeleton::createOutResultModelListProtected()
{
    CT_OutResultModelGroupToCopyPossibilities *res = createNewOutResultModelToCopy(DEF_SearchInResult);
    res->addItemModel(DEF_SearchInGroup, _outRefPointModelName, new CT_ReferencePoint(), tr("Point de référence (lissé)"));
}

void ONF_StepSmoothSkeleton::compute()
{
    // on récupère le résultat copié
    CT_ResultGroup *outRes = getOutResultList().first();

    CT_ResultGroupIterator itSection(outRes, this, DEF_SearchInSection);
    while (itSection.hasNext() && (!isStopped()))
    {
        const CT_AbstractItemGroup *section = itSection.next();

        QList<CT_ReferencePoint*> refPoints;
        QList<CT_ReferencePoint*> outRefPoints;
        QMap<CT_ReferencePoint*, CT_AbstractItemGroup*> groups;

        CT_GroupIterator itGrp(section, this, DEF_SearchInGroup);
        while (itGrp.hasNext())
        {
            CT_AbstractItemGroup *group = (CT_AbstractItemGroup*) itGrp.next();
            CT_ReferencePoint* refPoint = (CT_ReferencePoint*) group->firstItemByINModelName(this, DEF_SearchInRefPoint);
            refPoints.append(refPoint);
            groups.insert(refPoint, group);
        }

        Eigen::Vector3d vert(0,0,1); // verticale
        CT_ReferencePoint* mp = NULL;
        double xm = 0;
        double ym = 0;
        double zm = 0;

        int size = refPoints.size();
        if (size==0) {return;}

        // lissage des points autres que les extrêmes
        for (int i = 1 ; i < (size-1) ; i++)
        {
            CT_ReferencePoint* p1 = refPoints.at(i-1);
            CT_ReferencePoint* p2 = refPoints.at(i);
            CT_ReferencePoint* p3 = refPoints.at(i+1);

            CT_LineData line(Eigen::Vector3d(p1->x(), p1->y(), p1->z()), Eigen::Vector3d(p3->x(), p3->y(), p3->z()));

            if (line.intersectionWithRect3D(p2->x(), p2->y(), p2->z(), vert, &xm, &ym, &zm))
            {
                xm = (xm + p2->x())/2;
                ym = (ym + p2->y())/2;
                zm = p2->z();

                mp = new CT_ReferencePoint(_outRefPointModelName.completeName(), outRes, xm, ym, zm, p2->xyBuffer());
            } else {
                qDebug() << "Problème : Pas d'intersection trouvée, RefID=" << p2->refId();
                mp = (CT_ReferencePoint*) p2->copy(_outRefPointModelName.completeName(), outRes, CT_ResultCopyModeList() << CT_ResultCopyModeList::CopyItemDrawableReference);
            }
            outRefPoints.append(mp);
            groups.value(p2)->addItemDrawable(mp);
        }


        // Gestion des premier et dernier points
        CT_ReferencePoint* firstIn = refPoints.first();
        CT_ReferencePoint* lastIn = refPoints.last();
        CT_ReferencePoint* firstOut = NULL;
        CT_ReferencePoint* lastOut = NULL;


        int sizeOut = outRefPoints.size();
        if (size <= 2)
        {
            firstOut = (CT_ReferencePoint*) firstIn->copy(_outRefPointModelName.completeName(), outRes, CT_ResultCopyModeList() << CT_ResultCopyModeList::CopyItemDrawableReference);
            groups.value(firstIn)->addItemDrawable(firstOut);
        }
        if (size == 2 || size==3)
        {
            lastOut = (CT_ReferencePoint*) lastIn->copy(_outRefPointModelName.completeName(), outRes, CT_ResultCopyModeList() << CT_ResultCopyModeList::CopyItemDrawableReference);
            groups.value(lastIn)->addItemDrawable(lastOut);
        }

        if (size > 3)
        {
            CT_ReferencePoint* p1 = outRefPoints.at(0);
            CT_ReferencePoint* p2 = outRefPoints.at(1);

            CT_LineData lineF(Eigen::Vector3d(p1->x(), p1->y(), p1->z()), Eigen::Vector3d(p2->x(), p2->y(), p2->z()));

            if (lineF.intersectionWithRect3D(firstIn->x(), firstIn->y(), firstIn->z(), vert, &xm, &ym, &zm))
            {
                firstOut = new CT_ReferencePoint(_outRefPointModelName.completeName(), outRes, xm, ym, zm, firstIn->xyBuffer());
            } else {
                qDebug() << "Problème : Pas d'intersection trouvée, RefID=" << firstIn->refId();
                firstOut = (CT_ReferencePoint*) firstIn->copy(_outRefPointModelName.completeName(), outRes, CT_ResultCopyModeList() << CT_ResultCopyModeList::CopyItemDrawableReference);
            }
            groups.value(firstIn)->addItemDrawable(firstOut);



            CT_ReferencePoint* p3 = outRefPoints.at(sizeOut-2);
            CT_ReferencePoint* p4 = outRefPoints.at(sizeOut-1);

            CT_LineData lineL(Eigen::Vector3d(p3->x(), p3->y(), p3->z()), Eigen::Vector3d(p4->x(), p4->y(), p4->z()));

            if (lineL.intersectionWithRect3D(lastIn->x(), lastIn->y(), lastIn->z(), vert, &xm, &ym, &zm))
            {
                lastOut = new CT_ReferencePoint(_outRefPointModelName.completeName(), outRes, xm, ym, zm, firstIn->xyBuffer());
            } else {
                qDebug() << "Problème : Pas d'intersection trouvée, RefID=" << lastIn->refId();
                lastOut = (CT_ReferencePoint*) lastIn->copy(_outRefPointModelName.completeName(), outRes, CT_ResultCopyModeList() << CT_ResultCopyModeList::CopyItemDrawableReference);
            }
            groups.value(lastIn)->addItemDrawable(lastOut);

        }
    }
}
