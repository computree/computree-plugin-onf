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


#include "onf_stepaddtilexyareas.h"
#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"

#include "ct_itemdrawable/abstract/ct_abstractitemdrawablewithpointcloud.h"
#include "ct_itemdrawable/ct_fileheader.h"
#include "ct_itemdrawable/ct_box2d.h"


#include "ct_result/ct_resultgroup.h"
#include "ct_view/ct_stepconfigurabledialog.h"

#define DEF_SearchInResult      "r"
#define DEF_SearchInGroup       "g"
#define DEF_SearchInHeader      "h"
#define DEF_SearchOutResult     "r"

ONF_StepAddTileXYAreas::ONF_StepAddTileXYAreas(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _xRefCoord = 0.0;
    _yRefCoord = 0.0;
    _tileSize  = 500.0;
    _bufferSize  = 20.0;
    _bufferIncluded = false;
}

QString ONF_StepAddTileXYAreas::getStepDescription() const
{
    return tr("Ajout des emprises de dalles");
}

QString ONF_StepAddTileXYAreas::getStepDetailledDescription() const
{
    return tr("Pour chaque fichier d'entrée, ajoute l'emprise de la dalle");
}

CT_VirtualAbstractStep* ONF_StepAddTileXYAreas::createNewInstance(CT_StepInitializeData &dataInit)
{
    // cree une copie de cette etape
    return new ONF_StepAddTileXYAreas(dataInit);
}

//////////////////// PROTECTED //////////////////

void ONF_StepAddTileXYAreas::createInResultModelListProtected()
{
    CT_InResultModelGroupToCopy * resultModel = createNewInResultModelForCopy(DEF_SearchInResult, tr("Dalles"));
    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_SearchInGroup, CT_AbstractItemGroup::staticGetType(), tr("Grp"));
    resultModel->addItemModel(DEF_SearchInGroup, DEF_SearchInHeader, CT_FileHeader::staticGetType(), tr("Entête de fichier"));
}

void ONF_StepAddTileXYAreas::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addDouble(tr("Coordonnée X de référence"), "m"  , -1e+10, 1e+10, 4, _xRefCoord);
    configDialog->addDouble(tr("Coordonnée Y de référence"), "m"  , -1e+10, 1e+10, 4, _yRefCoord);
    configDialog->addDouble(tr("Taille de la dalle unitaire"), "m", -1e+10, 1e+10, 4, _tileSize);
    configDialog->addDouble(tr("Taille de la zone tampon"), "m", -1e+10, 1e+10, 4, _bufferSize);
    configDialog->addBool(tr("Les fichiers d'entrée contiennent les buffers"), "", "", _bufferIncluded);
}

void ONF_StepAddTileXYAreas::createOutResultModelListProtected()
{
    CT_OutResultModelGroupToCopyPossibilities *res = createNewOutResultModelToCopy(DEF_SearchInResult);

    if(res != NULL) {
        res->addItemModel(DEF_SearchInGroup, _outTileXYAreaModelName, new CT_Box2D(), tr("Emprise"));
        res->addItemModel(DEF_SearchInGroup, _outBufferTileXYAreaModelName, new CT_Box2D(), tr("Emprise (Buffer)"));
    }
}

void ONF_StepAddTileXYAreas::compute()
{
    // on récupre le résultat copié
    CT_ResultGroup *outRes = getOutResultList().first();

    CT_ResultGroupIterator it(outRes, this, DEF_SearchInGroup);
    while (it.hasNext() && (!isStopped()))
    {
        CT_AbstractItemGroup *group = (CT_AbstractItemGroup*) it.next();
        const CT_FileHeader *header = (const CT_FileHeader*)group->firstItemByINModelName(this, DEF_SearchInHeader);

        if(header != NULL)
        {
            if (header->hasBoundingBox()) // the header has to be geographical
            {
                QString fileName = header->getFileName();
                Eigen::Vector3d min, max;
                header->getBoundingBox(min, max);

                if (_bufferIncluded)
                {
                    min(0) += _bufferSize;
                    min(1) += _bufferSize;
                    max(0) -= _bufferSize;
                    max(1) -= _bufferSize;
                }

                Eigen::Vector2d minBB, maxBB;

                double baseX = min(0) + 0.1*_tileSize; // Sécurité au cas où il y quelques points à gauche du dallage théorique (dans le header)
                double baseY = min(1) + 0.1*_tileSize; // Sécurité au cas où il y quelques points en bas du dallage théorique (dans le header)

                minBB(0) = std::floor((baseX - _xRefCoord) / _tileSize) * _tileSize + _xRefCoord;
                minBB(1) = std::floor((baseY - _yRefCoord) / _tileSize) * _tileSize + _yRefCoord;


                maxBB(0) = minBB(0);
                maxBB(1) = minBB(1);

//                while (maxBB(0) < max(0)) {maxBB(0) += _tileSize;}
//                while (maxBB(1) < max(1)) {maxBB(1) += _tileSize;}

                maxBB(0) += _tileSize;
                maxBB(1) += _tileSize;

                CT_Box2DData* boxData = new CT_Box2DData(minBB, maxBB);
                CT_Box2D* box2D = new CT_Box2D(_outTileXYAreaModelName.completeName(), outRes, boxData);

                if (!fileName.isEmpty()) {box2D->setDisplayableName(fileName);}
                group->addItemDrawable(box2D);

                minBB(0) -= _bufferSize;
                minBB(1) -= _bufferSize;
                maxBB(0) += _bufferSize;
                maxBB(1) += _bufferSize;

                boxData = new CT_Box2DData(minBB, maxBB);
                box2D = new CT_Box2D(_outBufferTileXYAreaModelName.completeName(), outRes, boxData);

                if (!fileName.isEmpty()) {box2D->setDisplayableName(QString("%1_Buffer").arg(fileName));}
                group->addItemDrawable(box2D);

            } else {
                PS_LOG->addMessage(LogInterface::warning, LogInterface::step, tr("Header %1 non géographique (impossible de déterminer l'emprise)").arg(header->getFileName()));
            }
        }
    }

}
