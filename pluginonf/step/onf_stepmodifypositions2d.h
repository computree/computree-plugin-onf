﻿/****************************************************************************
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

#ifndef ONF_STEPMODIFYPOSITIONS2D_H
#define ONF_STEPMODIFYPOSITIONS2D_H

#include "ct_step/abstract/ct_abstractstep.h"
#include "ct_itemdrawable/ct_point2d.h"

// Inclusion of standard result class
#include "ct_result/ct_resultgroup.h"


class ONF_StepModifyPositions2D: public CT_AbstractStep
{
    Q_OBJECT

public:

    /*! \brief Step constructor
     * 
     * Create a new instance of the step
     * 
     * \param dataInit Step parameters object
     */
    ONF_StepModifyPositions2D(CT_StepInitializeData &dataInit);

    /*! \brief Step description
     * 
     * Return a description of the step function
     */
    QString getStepDescription() const;

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

    void initManualMode();

    void useManualMode(bool quit = false);

private:

    // Step parameters
    DocumentInterface*      m_doc;
    int                     m_status;

    QList<CT_Point2D*>      _positions;
    CT_OutAbstractSingularItemModel* _modelCreation;
    CT_ResultGroup* _outRes;
};

#endif // ONF_STEPMODIFYPOSITIONS2D_H
