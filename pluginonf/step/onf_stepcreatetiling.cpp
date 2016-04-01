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


#include "onf_stepcreatetiling.h"
#include "ct_result/model/inModel/ct_inresultmodelgroup.h"
#include "ct_result/model/outModel/ct_outresultmodelgroup.h"
#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/ct_outresultmodelgroupcopy.h"


#include "ct_itemdrawable/abstract/ct_abstractitemdrawablewithpointcloud.h"
#include "ct_itemdrawable/ct_fileheader.h"
#include "ct_itemdrawable/ct_box2d.h"


#include "ct_result/ct_resultgroup.h"
#include "ct_view/ct_stepconfigurabledialog.h"

#define DEF_SearchInResult      "r"
#define DEF_SearchInGroup       "g"
#define DEF_SearchInItem      "item"

#define DEFout_result "r"
#define DEFout_grp "g"
#define DEFout_Tile "tile"

ONF_StepCreateTiling::ONF_StepCreateTiling(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _tileSize  = 500.0;
    _keepEmptyTiles = false;

    _useRefCoord = true;
    _xRefCoord = 0.0;
    _yRefCoord = 0.0;
}

QString ONF_StepCreateTiling::getStepDescription() const
{
    return tr("Créer un dallage");
}

QString ONF_StepCreateTiling::getStepDetailledDescription() const
{
    return tr("Créer un dallage couvrant l'emprise des items d'entrée.");
}

CT_VirtualAbstractStep* ONF_StepCreateTiling::createNewInstance(CT_StepInitializeData &dataInit)
{
    // cree une copie de cette etape
    return new ONF_StepCreateTiling(dataInit);
}

//////////////////// PROTECTED //////////////////

void ONF_StepCreateTiling::createInResultModelListProtected()
{
    CT_InResultModelGroupToCopy * resultModel = createNewInResultModelForCopy(DEF_SearchInResult, tr("Dalles"));
    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_SearchInGroup, CT_AbstractItemGroup::staticGetType(), tr("Grp"));
    resultModel->addItemModel(DEF_SearchInGroup, DEF_SearchInItem, CT_AbstractSingularItemDrawable::staticGetType(), tr("Item (avec BoundingBox)"));
}

void ONF_StepCreateTiling::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addDouble(tr("Taille de la dalle unitaire"), "m", -1e+10, 1e+10, 4, _tileSize);
    configDialog->addBool(tr("Conserver les emprises vides"), "", "", _keepEmptyTiles);

    configDialog->addEmpty();
    configDialog->addBool(tr("Recaler sur une coordonnée de référence"), "", "", _useRefCoord);
    configDialog->addDouble(tr("Coordonnée X de référence"), "m"  , -1e+10, 1e+10, 4, _xRefCoord);
    configDialog->addDouble(tr("Coordonnée Y de référence"), "m"  , -1e+10, 1e+10, 4, _yRefCoord);
}

void ONF_StepCreateTiling::createOutResultModelListProtected()
{
    CT_OutResultModelGroup *outRes = createNewOutResultModel(DEFout_result, tr("Emprise Créée"));
    outRes->setRootGroup(DEFout_grp, new CT_StandardItemGroup(), tr("Groupe"));
    outRes->addItemModel(DEFout_grp, DEFout_Tile, new CT_Box2D(), tr("Emprise créée"));

    createNewOutResultModelToCopy(DEF_SearchInResult);
}

void ONF_StepCreateTiling::compute()
{
    CT_ResultGroup *outRes = getOutResultList().first();
    CT_ResultGroup *inRes = getInputResults().first();

    Eigen::Vector2d minBB, maxBB;
    minBB(0) = std::numeric_limits<double>::max();
    minBB(1) = std::numeric_limits<double>::max();
    maxBB(0) = -std::numeric_limits<double>::max();
    maxBB(1) = -std::numeric_limits<double>::max();

    QList<Eigen::Vector3d> mins;
    QList<Eigen::Vector3d> maxs;

    setProgress(5);
    CT_ResultGroupIterator it(inRes, this, DEF_SearchInGroup);
    while (it.hasNext() && (!isStopped()))
    {
        CT_AbstractItemGroup *group = (CT_AbstractItemGroup*) it.next();
        const CT_AbstractSingularItemDrawable *item = group->firstItemByINModelName(this, DEF_SearchInItem);

        if(item != NULL)
        {
            if (item->hasBoundingBox()) // the header has to be geographical
            {
                Eigen::Vector3d min, max;
                item->getBoundingBox(min, max);

                if (!_keepEmptyTiles)
                {
                    mins.append(min);
                    maxs.append(max);
                }

                if (min(0) < minBB(0)) {minBB(0) = min(0);}
                if (min(1) < minBB(1)) {minBB(1) = min(1);}
                if (max(0) > maxBB(0)) {maxBB(0) = max(0);}
                if (max(1) > maxBB(1)) {maxBB(1) = max(1);}

            } else {
                PS_LOG->addMessage(LogInterface::warning, LogInterface::step, tr("Item (id= %1) sans BoundingBox (impossible de déterminer l'emprise)").arg(item->id()));
            }
        }
    }
    setProgress(50);

    if (_useRefCoord)
    {
        minBB(0) = std::floor((minBB(0) - _xRefCoord) / _tileSize) * _tileSize + _xRefCoord;
        minBB(1) = std::floor((minBB(1) - _yRefCoord) / _tileSize) * _tileSize + _yRefCoord;

        double maxx = maxBB(0);
        double maxy = maxBB(1);

        maxBB(0) = minBB(0);
        maxBB(1) = minBB(1);

        while (maxBB(0) < maxx) {maxBB(0) += _tileSize;}
        while (maxBB(1) < maxy) {maxBB(1) += _tileSize;}
    }

    for (double x = minBB(0) ; x < maxBB(0) ; x += _tileSize)
    {
        for (double y = maxBB(1) ; y > minBB(1) ; y -= _tileSize)
        {
            Eigen::Vector2d min(x, y - _tileSize);
            Eigen::Vector2d max(x + _tileSize, y);

            bool found = false;
            if (!_keepEmptyTiles)
            {
                for (int i = 0 ; !found && i < mins.size() ; i++)
                {
                    const Eigen::Vector3d& mini = mins.at(i);
                    const Eigen::Vector3d& maxi = maxs.at(i);

                    found = mini(0) < max(0) && maxi(0) > min(0) && mini(1) < max(1) && maxi(1) > min(1);
                }
            }

            if (found || _keepEmptyTiles)
            {
                CT_Box2DData* boxData = new CT_Box2DData(min, max);
                CT_Box2D* box2D = new CT_Box2D(DEFout_Tile, outRes, boxData);
                CT_StandardItemGroup* group = new CT_StandardItemGroup(DEFout_grp, outRes);
                group->addItemDrawable(box2D);
                outRes->addGroup(group);
            }
        }
    }
    setProgress(100);

}
