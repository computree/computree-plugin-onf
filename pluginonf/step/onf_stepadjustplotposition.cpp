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

#include "onf_stepadjustplotposition.h"

#include "ct_itemdrawable/ct_scene.h"
#include "ct_itemdrawable/ct_circle2d.h"
#include "ctliblas/itemdrawable/las/ct_stdlaspointsattributescontainer.h"
#include "ct_itemdrawable/ct_pointsattributesscalartemplated.h"

#include "ct_result/ct_resultgroup.h"
#include "ct_result/model/inModel/ct_inresultmodelgroup.h"
#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/ct_outresultmodelgroupcopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"

#include "ct_pointcloudindex/ct_pointcloudindexvector.h"

#include "ct_itemdrawable/tools/iterator/ct_groupiterator.h"
#include "ct_iterator/ct_pointiterator.h"

#ifdef USE_OPENCV
#include "ct_itemdrawable/ct_image2d.h"
#endif

#include <QMessageBox>
#include <limits>
#include <QDebug>

// Alias for indexing models
#define DEFin_resScene "resScene"
#define DEFin_grpSc "grpSc"
#define DEFin_scene "scene"
#define DEFin_lasAtt "lasAtt"
#define DEFin_heightAtt "heightAtt"

#define DEFin_resDTM "resdtm"
#define DEFin_DTMGrp "dtmgrp"
#define DEFin_DTM "dtm"

#define DEFin_resPlot "resPlot"

#define DEFin_grp "grp"
#define DEFin_ref "ref"
#define DEFin_refDbh "refDbh"
#define DEFin_refHeight "refHeight"
#define DEFin_refID "refID"
#define DEFin_refIDplot "refIDplot"
#define DEFin_species "species"


// Constructor : initialization of parameters
ONF_StepAdjustPlotPosition::ONF_StepAdjustPlotPosition(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _dataContainer = NULL;

    setManual(true);

    _m_status = 0;
    _m_doc = NULL;
}

ONF_StepAdjustPlotPosition::~ONF_StepAdjustPlotPosition()
{
}

// Step description (tooltip of contextual menu)
QString ONF_StepAdjustPlotPosition::getStepDescription() const
{
    return tr("Recaler une placette terrain");
}

// Step detailled description
QString ONF_StepAdjustPlotPosition::getStepDetailledDescription() const
{
    return tr("To Do");
}

// Step copy method
CT_VirtualAbstractStep* ONF_StepAdjustPlotPosition::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new ONF_StepAdjustPlotPosition(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void ONF_StepAdjustPlotPosition::createInResultModelListProtected()
{
    CT_InResultModelGroupToCopy *resIn_Plot = createNewInResultModelForCopy(DEFin_resPlot, tr("Placette"), "", true);
    resIn_Plot->setZeroOrMoreRootGroup();
    resIn_Plot->addGroupModel("", DEFin_grp, CT_AbstractItemGroup::staticGetType(), tr("Groupe"));
    resIn_Plot->addItemModel(DEFin_grp, DEFin_ref, CT_Circle2D::staticGetType(), tr("Arbre"));
    resIn_Plot->addItemAttributeModel(DEFin_ref, DEFin_refDbh, QList<QString>() << CT_AbstractCategory::DATA_VALUE, CT_AbstractCategory::NUMBER, tr("DBH"));
    resIn_Plot->addItemAttributeModel(DEFin_ref, DEFin_refHeight, QList<QString>() << CT_AbstractCategory::DATA_VALUE, CT_AbstractCategory::NUMBER, tr("Height"));
    resIn_Plot->addItemAttributeModel(DEFin_ref, DEFin_refID, QList<QString>() << CT_AbstractCategory::DATA_ID, CT_AbstractCategory::STRING, tr("IDtree"));
    resIn_Plot->addItemAttributeModel(DEFin_ref, DEFin_refIDplot, QList<QString>() << CT_AbstractCategory::DATA_ID, CT_AbstractCategory::STRING, tr("IDplot"));
    resIn_Plot->addItemAttributeModel(DEFin_ref, DEFin_species, QList<QString>() << CT_AbstractCategory::DATA_VALUE, CT_AbstractCategory::STRING, tr("Species"), "", CT_InAbstractModel::C_ChooseOneIfMultiple, CT_InAbstractModel::F_IsOptional);


    CT_InResultModelGroup *resIn_Scene = createNewInResultModel(DEFin_resScene, tr("Scène"), "", true);
    resIn_Scene->setZeroOrMoreRootGroup();
    resIn_Scene->addGroupModel("", DEFin_grpSc, CT_AbstractItemGroup::staticGetType(), tr("Groupe"));
    resIn_Scene->addItemModel(DEFin_grpSc, DEFin_scene, CT_AbstractItemDrawableWithPointCloud::staticGetType(), tr("Scène"));
    resIn_Scene->addItemModel(DEFin_grpSc, DEFin_lasAtt, CT_StdLASPointsAttributesContainer::staticGetType(), tr("Attributs LAS"), "", CT_InAbstractItemModel::C_ChooseOneIfMultiple, CT_InAbstractItemModel::F_IsOptional);
    resIn_Scene->addItemModel(DEFin_grpSc, DEFin_heightAtt, CT_PointsAttributesScalarTemplated<float>::staticGetType(), tr("Attribut Hauteur"), "", CT_InAbstractItemModel::C_ChooseOneIfMultiple, CT_InAbstractItemModel::F_IsOptional);


#ifdef USE_OPENCV
    CT_InResultModelGroup *resultDTM = createNewInResultModel(DEFin_resDTM, tr("MNT"), "", true);
    resultDTM->setZeroOrMoreRootGroup();
    resultDTM->addGroupModel("", DEFin_DTMGrp, CT_AbstractItemGroup::staticGetType(), tr("Group"));
    resultDTM->addItemModel(DEFin_DTMGrp, DEFin_DTM, CT_Image2D<float>::staticGetType(), tr("MNT"));
    resultDTM->setMinimumNumberOfPossibilityThatMustBeSelectedForOneTurn(0);
#endif

}

// Creation and affiliation of OUT models
void ONF_StepAdjustPlotPosition::createOutResultModelListProtected()
{
    CT_OutResultModelGroupToCopyPossibilities *res = createNewOutResultModelToCopy(DEFin_resPlot);

    if(res != NULL)
    {
        res->addItemModel(DEFin_grp, _outCircleModelName, new CT_Circle2D(), tr("Arbre déplacé"));
        res->addItemAttributeModel(_outCircleModelName, _outDBHAttModelName, new CT_StdItemAttributeT<float>(CT_AbstractCategory::DATA_NUMBER), tr("DBH"));
        res->addItemAttributeModel(_outCircleModelName, _outHeightAttModelName, new CT_StdItemAttributeT<float>(CT_AbstractCategory::DATA_NUMBER), tr("Height"));
        res->addItemAttributeModel(_outCircleModelName, _outPlotIDAttModelName, new CT_StdItemAttributeT<QString>(CT_AbstractCategory::DATA_ID), tr("IDplot"));
        res->addItemAttributeModel(_outCircleModelName, _outTreeIDAttModelName, new CT_StdItemAttributeT<QString>(CT_AbstractCategory::DATA_ID), tr("IDtree"));
        res->addItemAttributeModel(_outCircleModelName, _outSpeciesAttModelName, new CT_StdItemAttributeT<QString>(CT_AbstractCategory::DATA_VALUE), tr("Species"));
        res->addItemAttributeModel(_outCircleModelName, _outTransXAttModelName, new CT_StdItemAttributeT<float>(CT_AbstractCategory::DATA_NUMBER), tr("TransX"));
        res->addItemAttributeModel(_outCircleModelName, _outTransYAttModelName, new CT_StdItemAttributeT<float>(CT_AbstractCategory::DATA_NUMBER), tr("TransY"));
        res->addItemAttributeModel(_outCircleModelName, _outMovedAttModelName, new CT_StdItemAttributeT<bool>(CT_AbstractCategory::DATA_VALUE), tr("Moved"));
        res->addItemAttributeModel(_outCircleModelName, _outZPointAttModelName, new CT_StdItemAttributeT<float>(CT_AbstractCategory::DATA_NUMBER), tr("zPointClicked"));
    }
}

// Semi-automatic creation of step parameters DialogBox
void ONF_StepAdjustPlotPosition::createPostConfigurationDialog()
{
}

void ONF_StepAdjustPlotPosition::compute()
{
    _dataContainer = new ONF_ActionAdjustPlotPosition_dataContainer();

#ifdef USE_OPENCV
    if (getInputResults().size() > 2)
    {
        CT_ResultGroup* resin_DTM = getInputResults().at(2);
        CT_ResultItemIterator it(resin_DTM, this, DEFin_DTM);
        if (it.hasNext())
        {
            _dataContainer->_dtm = (CT_Image2D<float>*) it.next();
        }
    }
#endif

    _m_status = 0;
    QList<CT_ResultGroup*> inResultList = getInputResults();
    CT_ResultGroup* resIn_scene = inResultList.at(1);

    CT_ResultGroupIterator itGrpSc(resIn_scene, this, DEFin_grpSc);
    while (itGrpSc.hasNext())
    {
        const CT_AbstractItemGroup* grp = itGrpSc.next();

        CT_AbstractItemDrawableWithPointCloud* sc = (CT_AbstractItemDrawableWithPointCloud*) grp->firstItemByINModelName(this, DEFin_scene);
        CT_StdLASPointsAttributesContainer* lasAtt = (CT_StdLASPointsAttributesContainer*)grp->firstItemByINModelName(this, DEFin_lasAtt);
        CT_PointsAttributesScalarTemplated<float>* heightAtt = (CT_PointsAttributesScalarTemplated<float>*)grp->firstItemByINModelName(this, DEFin_heightAtt);

        if (sc != NULL)
        {            
            _dataContainer->_scenes.append(sc);
            _dataContainer->_LASattributes.append(lasAtt);
            _dataContainer->_heightAttributes.append(heightAtt);
        }
    }

    QList<CT_ResultGroup*> outResultList = getOutResultList();
    CT_ResultGroup* resOut_positions = outResultList.at(0);

    CT_ResultGroupIterator itGrp(resOut_positions, this, DEFin_grp);
    while (itGrp.hasNext())
    {
        CT_StandardItemGroup* grp = (CT_StandardItemGroup*) itGrp.next();

        CT_Circle2D* circle = (CT_Circle2D*) grp->firstItemByINModelName(this, DEFin_ref);
        if (circle != NULL)
        {
            ONF_ActionAdjustPlotPosition_treePosition* treePos = new ONF_ActionAdjustPlotPosition_treePosition();
            treePos->_x = circle->getCenterX();
            treePos->_y = circle->getCenterY();
            treePos->_originalX = circle->getCenterX();
            treePos->_originalY = circle->getCenterY();

            CT_AbstractItemAttribute* att = circle->firstItemAttributeByINModelName(resOut_positions, this, DEFin_refDbh);
            if (att != NULL) {treePos->_dbh = att->toFloat(circle, NULL);}
            if (treePos->_dbh <= 0) {treePos->_dbh = 7.5;}

            att = circle->firstItemAttributeByINModelName(resOut_positions, this, DEFin_refHeight);
            if (att != NULL) {treePos->_height = att->toFloat(circle, NULL);}
            treePos->_trueheight = treePos->_height;
            if (treePos->_height <= 0) {treePos->_height = 5.0;}

            att = circle->firstItemAttributeByINModelName(resOut_positions, this, DEFin_refID);
            if (att != NULL) {treePos->_idTree = att->toString(circle, NULL);}

            att = circle->firstItemAttributeByINModelName(resOut_positions, this, DEFin_refIDplot);
            if (att != NULL) {treePos->_idPlot = att->toString(circle, NULL);}

            att = circle->firstItemAttributeByINModelName(resOut_positions, this, DEFin_species);
            if (att != NULL) {treePos->_species = att->toString(circle, NULL);}

            _dataContainer->_positions.append(treePos);
            treePos->_grp = grp;
        }
    }

    requestManualMode();
    _m_status = 1;

    double transX = _dataContainer->_transX;
    double transY = _dataContainer->_transY;

    for (int i = 0 ; i < _dataContainer->_positions.size() ; i++)
    {
        ONF_ActionAdjustPlotPosition_treePosition* treePos = _dataContainer->_positions.at(i);

        if (treePos->_grp != NULL)
        {
            CT_Circle2D* circle = new CT_Circle2D(_outCircleModelName.completeName(), resOut_positions, new CT_Circle2DData(Eigen::Vector2d(treePos->_x + transX, treePos->_y + transY), treePos->_dbh / 200.0));
            circle->addItemAttribute(new CT_StdItemAttributeT<float>(_outDBHAttModelName.completeName(), CT_AbstractCategory::DATA_NUMBER, resOut_positions, treePos->_dbh));
            circle->addItemAttribute(new CT_StdItemAttributeT<float>(_outHeightAttModelName.completeName(), CT_AbstractCategory::DATA_NUMBER, resOut_positions, treePos->_trueheight));
            circle->addItemAttribute(new CT_StdItemAttributeT<QString>(_outPlotIDAttModelName.completeName(), CT_AbstractCategory::DATA_ID, resOut_positions, treePos->_idPlot));
            circle->addItemAttribute(new CT_StdItemAttributeT<QString>(_outTreeIDAttModelName.completeName(), CT_AbstractCategory::DATA_ID, resOut_positions, treePos->_idTree));
            circle->addItemAttribute(new CT_StdItemAttributeT<QString>(_outSpeciesAttModelName.completeName(), CT_AbstractCategory::DATA_VALUE, resOut_positions, treePos->_species));
            circle->addItemAttribute(new CT_StdItemAttributeT<float>(_outTransXAttModelName.completeName(), CT_AbstractCategory::DATA_NUMBER, resOut_positions, transX));
            circle->addItemAttribute(new CT_StdItemAttributeT<float>(_outTransYAttModelName.completeName(), CT_AbstractCategory::DATA_NUMBER, resOut_positions, transY));
            circle->addItemAttribute(new CT_StdItemAttributeT<bool>(_outMovedAttModelName.completeName(), CT_AbstractCategory::DATA_VALUE, resOut_positions, treePos->_moved));
            circle->addItemAttribute(new CT_StdItemAttributeT<float>(_outZPointAttModelName.completeName(), CT_AbstractCategory::DATA_NUMBER, resOut_positions, treePos->_zPoint));

            treePos->_grp->addItemDrawable(circle);
        }

        delete treePos;
    }

    delete _dataContainer;

    requestManualMode();
}

void ONF_StepAdjustPlotPosition::initManualMode()
{
    if(_m_doc == NULL)
    {
        // create a new 3D document
        _m_doc = getGuiContext()->documentManager()->new3DDocument();

        ONF_ActionAdjustPlotPosition* action = new ONF_ActionAdjustPlotPosition(_dataContainer);
        // set the action (a copy of the action is added at all graphics view, and the action passed in parameter is deleted)
        _m_doc->setCurrentAction(action, false);
    }

//    QMessageBox::information(NULL,
//                             tr("Mode manuel"),
//                             tr("Bienvenue dans le mode manuel de cette étape.\n"),
//                             QMessageBox::Ok);
}

void ONF_StepAdjustPlotPosition::useManualMode(bool quit)
{
    if(_m_status == 0)
    {
        if(quit)
        {
        }
    }
    else if(_m_status == 1)
    {
        if(!quit)
        {
            _m_doc = NULL;
            quitManualMode();
        }
    }

}
