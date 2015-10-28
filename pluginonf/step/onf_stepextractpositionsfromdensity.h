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

#ifndef ONF_STEPEXTRACTPOSITIONSFROMDENSITY_H
#define ONF_STEPEXTRACTPOSITIONSFROMDENSITY_H

#include "ct_step/abstract/ct_abstractstep.h"
#include "ct_itemdrawable/ct_grid2dxy.h"
#include "ct_tools/model/ct_autorenamemodels.h"

/*!
 * \class ONF_StepExtractPositionsFromDensity
 * \ingroup Steps_PB
 * \brief <b>Créée des positions 2D à partir de la densité des points.</b>
 *
 * No detailled description for this step
 *
 * \param _resolution 
 * \param _threshold 
 *
 */

class ONF_StepExtractPositionsFromDensity: public CT_AbstractStep
{
    Q_OBJECT

public:

    /*! \brief Step constructor
     * 
     * Create a new instance of the step
     * 
     * \param dataInit Step parameters object
     */
    ONF_StepExtractPositionsFromDensity(CT_StepInitializeData &dataInit);

    /*! \brief Step description
     * 
     * Return a description of the step function
     */
    QString getStepDescription() const;

    /*! \brief Step detailled description
     * 
     * Return a detailled description of the step function
     */
    QString getStepDetailledDescription() const;

    /*! \brief Step URL
     * 
     * Return a URL of a wiki for this step
     */
    QString getStepURL() const;

    /*! \brief Step copy
     * 
     * Step copy, used when a step is added by step contextual menu
     */
    CT_VirtualAbstractStep* createNewInstance(CT_StepInitializeData &dataInit);

protected:

    /*! \brief Input results specification
     * 
     * Specification of input results models needed by the step (IN)
     */
    void createInResultModelListProtected();

    /*! \brief Parameters DialogBox
     * 
     * DialogBox asking for step parameters
     */
    void createPostConfigurationDialog();

    /*! \brief Output results specification
     * 
     * Specification of output results models created by the step (OUT)
     */
    void createOutResultModelListProtected();

    /*! \brief Algorithm of the step
     * 
     * Step computation, using input results, and creating output results
     */
    void compute();

private:

    // Step parameters
    double    _resolution;
    double    _threshold;
    int       _thresholdN;
    int       _relativeMode;

    CT_AutoRenameModels    _grpPosition2D_ModelName;
    CT_AutoRenameModels    _position2D_ModelName;
    CT_AutoRenameModels    _position2DAtt_ModelName;
    CT_AutoRenameModels    _position2DAttMax_ModelName;
    CT_AutoRenameModels    _grid2D_ModelName;

    void fillCellsInList(QList<size_t> &liste, const int cluster, CT_Grid2DXY<int> *clustersGrid, CT_Grid2DXY<int> *densityGrid, int &density, int &densityMax);
    QList<size_t> computeColonize(size_t originColumn, size_t originRow, const CT_Grid2DXY<int> *densityGrid);
    void appendIfNotNulValue(QList<size_t> &result, size_t col, size_t lin, const CT_Grid2DXY<int> *densityGrid);
};

#endif // ONF_STEPEXTRACTPOSITIONSFROMDENSITY_H
