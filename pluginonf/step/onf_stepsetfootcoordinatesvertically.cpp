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


#include "onf_stepsetfootcoordinatesvertically.h"

#ifdef USE_OPENCV
#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"

#include "ct_itemdrawable/abstract/ct_abstractitemdrawablewithpointcloud.h"
#include "ct_itemdrawable/ct_referencepoint.h"
#include "ct_itemdrawable/ct_image2d.h"
#include "ct_result/ct_resultgroup.h"
#include "ct_itemdrawable/abstract/ct_abstractitemgroup.h"
#include "ct_itemdrawable/tools/iterator/ct_groupiterator.h"

#define DEF_SearchInMNTGroup         "gmnt"
#define DEF_SearchInMNT              "mnt"
#define DEF_SearchInBaseGroup         "g"
#define DEF_SearchInRefPoint         "rp"
#define DEF_SearchInRefPointGroup    "rpg"

#define DEF_SearchInMNTResult        "rmnt"
#define DEF_SearchInResult           "r"
#define DEF_SearchOutResult          "rmnt"
#define DEF_SearchOutMNTResult       "r"

ONF_StepSetFootCoordinatesVertically::ONF_StepSetFootCoordinatesVertically(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
}

QString ONF_StepSetFootCoordinatesVertically::getStepDescription() const
{
    return tr("5- Récupérer la coordonnée MNT pour chaque Billon");
}

QString ONF_StepSetFootCoordinatesVertically::getStepDetailledDescription() const
{
    return tr("No detailled description for this step");
}


CT_VirtualAbstractStep* ONF_StepSetFootCoordinatesVertically::createNewInstance(CT_StepInitializeData &dataInit)
{
    // cree une copie de cette etape
    return new ONF_StepSetFootCoordinatesVertically(dataInit);
}

//////////////////// PROTECTED //////////////////

void ONF_StepSetFootCoordinatesVertically::createInResultModelListProtected()
{   
    // MNT
    CT_InResultModelGroup* resultModelMNT = createNewInResultModel(DEF_SearchInMNTResult, tr("MNT (Raster)"), "", true);
    resultModelMNT->setZeroOrMoreRootGroup();
    resultModelMNT->addGroupModel("", DEF_SearchInMNTGroup);
    resultModelMNT->addItemModel(DEF_SearchInMNTGroup, DEF_SearchInMNT, CT_Image2D<float>::staticGetType(), tr("Modèle Numérique de Terrain"));


    // Sections
    CT_InResultModelGroupToCopy* resultModel = createNewInResultModelForCopy(DEF_SearchInResult, tr("Billons"));
    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_SearchInBaseGroup, CT_AbstractItemGroup::staticGetType(), tr("Billon (Grp)"));
    resultModel->addGroupModel(DEF_SearchInBaseGroup, DEF_SearchInRefPointGroup, CT_AbstractItemGroup::staticGetType(), tr("Cluster (Grp)"));
    resultModel->addItemModel(DEF_SearchInRefPointGroup, DEF_SearchInRefPoint, CT_ReferencePoint::staticGetType(), tr("Point de référence"));
}

void ONF_StepSetFootCoordinatesVertically::createPostConfigurationDialog()
{
}

void ONF_StepSetFootCoordinatesVertically::createOutResultModelListProtected()
{
    CT_OutResultModelGroupToCopyPossibilities *res = createNewOutResultModelToCopy(DEF_SearchInResult);

    if(res != NULL)
        res->addItemModel(DEF_SearchInBaseGroup, _outFootCoordinateModelName, new CT_ReferencePoint(), tr("Coordonnée MNT"));
}

void ONF_StepSetFootCoordinatesVertically::compute()
{
     // Liste des résultats d'entrée
    QList<CT_ResultGroup*> inResultList = getInputResults();
    // liste des resultats de sortie d'apres le modele
    const QList<CT_ResultGroup*> &outResultList = getOutResultList();

    // Modéles pour récupérer les MNT
    CT_ResultGroup* inMNTResult = inResultList.first();

    CT_ResultItemIterator itMNT(inMNTResult, this, DEF_SearchInMNT);
    // on demande si on peut commencer à parcourir les groupes
    if(itMNT.hasNext())
    {
        CT_Image2D<float>* mnt = (CT_Image2D<float>*)itMNT.next();

        if(mnt != NULL)
        {
            // récupération des modéles out
            CT_ResultGroup *outResult = outResultList.at(0);

            CT_ResultGroupIterator itGrp(outResult, this, DEF_SearchInBaseGroup);
            while (itGrp.hasNext() && !isStopped())
            {
                CT_AbstractItemGroup* basegroup = (CT_AbstractItemGroup*) itGrp.next();

                CT_GroupIterator itRefGroup(basegroup, this, DEF_SearchInRefPointGroup);
                if (itRefGroup.hasNext())
                {
                    const CT_AbstractItemGroup* refPointGroup = itRefGroup.next();
                    CT_ReferencePoint* refPoint = (CT_ReferencePoint*) refPointGroup->firstItemByINModelName(this, DEF_SearchInRefPoint);

                    if (refPoint != NULL)
                    {
                        float na = mnt->NA();
                        float z_value = mnt->valueAtCoords(refPoint->x(), refPoint->y());
                        CT_ReferencePoint* footCoordinate;
                        if (z_value != na) {
                            footCoordinate = new CT_ReferencePoint(_outFootCoordinateModelName.completeName(), outResult, refPoint->x(), refPoint->y(), z_value, mnt->resolution());
                        } else {
                            footCoordinate = new CT_ReferencePoint(_outFootCoordinateModelName.completeName(), outResult, refPoint->x(), refPoint->y(), refPoint->z(), refPoint->xyBuffer());
                        }
                        basegroup->addItemDrawable(footCoordinate);
                    }
                }
            }
        }
    }

}
#endif
