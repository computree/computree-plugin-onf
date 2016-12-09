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

#include "onf_stepcomputecumulativenrtable.h"

#include "ct_itemdrawable/ct_standarditemgroup.h"
#include "ct_itemdrawable/ct_loopcounter.h"
#include "ct_itemdrawable/abstract/ct_abstractitemdrawablewithpointcloud.h"
#include "ctliblas/itemdrawable/las/ct_stdlaspointsattributescontainer.h"

#include "ct_itemdrawable/tools/iterator/ct_groupiterator.h"
#include "ct_itemdrawable/tools/iterator/ct_itemiterator.h"
#include "ct_result/ct_resultgroup.h"
#include "ct_result/model/inModel/ct_inresultmodelgroup.h"
#include "ct_result/model/outModel/ct_outresultmodelgroup.h"
#include "ct_view/ct_stepconfigurabledialog.h"

#include "ct_iterator/ct_pointiterator.h"


#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <iostream>

// Alias for indexing models
#define DEFin_rscene "rscene"
#define DEFin_grp "grp"
#define DEFin_points "points"
#define DEFin_lasAtt "lasAtt"

#define DEF_inResultCounter "rcounter"
#define DEF_inCounter "counter"

#define DEFout_res "outres"
#define DEFout_grp "outgrp"
#define DEFout_summary "summary"

// Constructor : initialization of parameters
ONF_StepComputeCumulativeNRTable::ONF_StepComputeCumulativeNRTable(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _fileName << "NRTable.txt";
    _mat.resize(1,1);
    _mat(0,0) = 0;

    _numberOfNegatives = 0;
}

// Step description (tooltip of contextual menu)
QString ONF_StepComputeCumulativeNRTable::getStepDescription() const
{
    return tr("Export N-R Table");
}

// Step detailled description
QString ONF_StepComputeCumulativeNRTable::getStepDetailledDescription() const
{
    return tr("No detailled description for this step");
}

// Step URL
QString ONF_StepComputeCumulativeNRTable::getStepURL() const
{
    //return tr("STEP URL HERE");
    return CT_AbstractStep::getStepURL(); //by default URL of the plugin
}

// Step copy method
CT_VirtualAbstractStep* ONF_StepComputeCumulativeNRTable::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new ONF_StepComputeCumulativeNRTable(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void ONF_StepComputeCumulativeNRTable::createInResultModelListProtected()
{
    CT_InResultModelGroup *resIn_item = createNewInResultModel(DEFin_rscene, tr("Scène(s)"));
    resIn_item->setZeroOrMoreRootGroup();
    resIn_item->addGroupModel("", DEFin_grp, CT_AbstractItemGroup::staticGetType(), tr("Groupe"), "", CT_InAbstractGroupModel::CG_ChooseOneIfMultiple);
    resIn_item->addItemModel(DEFin_grp, DEFin_points, CT_AbstractItemDrawableWithPointCloud::staticGetType(), tr("Scène"));
    resIn_item->addItemModel(DEFin_grp, DEFin_lasAtt, CT_StdLASPointsAttributesContainer::staticGetType(), tr("Attributs LAS"));

    CT_InResultModelGroup* res_counter = createNewInResultModel(DEF_inResultCounter, tr("Résultat compteur"), "", true);
    res_counter->setZeroOrMoreRootGroup();
    res_counter->addItemModel("", DEF_inCounter, CT_LoopCounter::staticGetType(), tr("Compteur"));
    res_counter->setMinimumNumberOfPossibilityThatMustBeSelectedForOneTurn(0);
}

// Creation and affiliation of OUT models
void ONF_StepComputeCumulativeNRTable::createOutResultModelListProtected()
{
    CT_OutResultModelGroup *resultSummary = createNewOutResultModel(DEFout_res, tr("N-R Table"));
    resultSummary->setRootGroup(DEFout_grp);
}

// Semi-automatic creation of step parameters DialogBox
void ONF_StepComputeCumulativeNRTable::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addFileChoice(tr("Choose Export file"), CT_FileChoiceButton::OneNewFile, "Text file (*.txt)", _fileName);
}

void ONF_StepComputeCumulativeNRTable::compute()
{
    QList<CT_ResultGroup*> inResultList = getInputResults();
    CT_ResultGroup* ritem = inResultList.at(0);

    QList<CT_ResultGroup*> outResultList = getOutResultList();
    CT_ResultGroup* rSummary = outResultList.at(0);
    rSummary->addGroup(new CT_StandardItemGroup(DEFout_grp, rSummary));


    bool last_turn = false;
    CT_ResultGroup* rcounter = NULL;
    if (inResultList.size() > 1) {rcounter = inResultList.at(1);}
    if (rcounter != NULL)
    {
        CT_ResultItemIterator itCounter(rcounter, this, DEF_inCounter);
        if (itCounter.hasNext())
        {
            const CT_LoopCounter* counter = (const CT_LoopCounter*) itCounter.next();
            if (counter != NULL)
            {
                if (counter->getCurrentTurn() == counter->getNTurns())
                {
                    last_turn = true;
               }
            }
        }
    }

    CT_LASData lasData;

    // parcours des item
    CT_ResultGroupIterator itSc(ritem, this, DEFin_grp);
    while (itSc.hasNext() && !isStopped())
    {
        CT_StandardItemGroup* grp = (CT_StandardItemGroup*) itSc.next();

        const CT_AbstractItemDrawableWithPointCloud* points = (CT_AbstractItemDrawableWithPointCloud*)grp->firstItemByINModelName(this, DEFin_points);
        const CT_StdLASPointsAttributesContainer* lasAtt = (CT_StdLASPointsAttributesContainer*)grp->firstItemByINModelName(this, DEFin_lasAtt);

        if (points != NULL && lasAtt != NULL)
        {
            const CT_AbstractPointCloudIndex *lasPointCloudIndex = lasAtt->pointsAttributesAt(CT_LasDefine::Return_Number)->getPointCloudIndex();
            size_t maxAttSize = lasPointCloudIndex->size();
            CT_PointIterator itP(points->getPointCloudIndex());
            while (itP.hasNext())
            {
                size_t index = itP.next().currentGlobalIndex();
                size_t lasIndex = lasPointCloudIndex->indexOf(index);

                if (lasIndex < maxAttSize)
                {
                    lasAtt->getLASDataAt(lasIndex, lasData);
                }

                if (lasData._Return_Number < 0 || lasData._Number_of_Returns < 0)
                {
                    _numberOfNegatives++;
                } else {
                    resizeMatIfNeeded(lasData._Return_Number, lasData._Number_of_Returns);
                    _mat(lasData._Return_Number, lasData._Number_of_Returns) += 1;
                }
            }
        }
    }

    if (_fileName.size() > 0 && _fileName.first() != "" && (rcounter == NULL || last_turn))
    {
        QFile file(_fileName.first());

        if(file.open(QFile::WriteOnly))
        {
            QTextStream stream(&file);

            stream << "\t\tN\n";
            stream << "\t\t";
            for (int j = 0 ; j < _mat.cols() ; j++)
            {
                stream << j << "\t";
            }
            stream << "\n";

            for (int i = 0 ; i < _mat.rows() ; i++)
            {
                if (i == (_mat.rows() - 1))
                {
                    stream << "R\t" << i << "\t";

                } else {
                    stream << "\t" << i << "\t";
                }

                for (int j = 0 ; j < _mat.cols() ; j++)
                {
                    stream << _mat(i,j) << "\t";
                }
                stream << "\n";
            }
            stream << "\n";
            stream << "Number of N or R < 0 = " << _numberOfNegatives << "\n";

            file.close();
        }
    }

}


void ONF_StepComputeCumulativeNRTable::resizeMatIfNeeded(int row, int col)
{
    int nrow = _mat.rows();
    int ncol = _mat.cols();

    if (row >= nrow)
    {
        _mat.conservativeResize(row + 1, ncol);
        for (int i = nrow ; i <= row ; i++)
        {
            for (int j = 0 ; j < ncol ; j++)
            {
                _mat(i,j) = 0;
            }
        }
    }

    nrow = _mat.rows();

    if (col >= ncol)
    {
        _mat.conservativeResize(Eigen::NoChange, col + 1);
        for (int i = 0 ; i < nrow ; i++)
        {
            for (int j = ncol ; j <= col ; j++)
            {
                _mat(i,j) = 0;
            }
        }
    }
}
