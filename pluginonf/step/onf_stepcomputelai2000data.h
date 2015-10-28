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

#ifndef ONF_STEPCOMPUTELAI2000DATA_H
#define ONF_STEPCOMPUTELAI2000DATA_H

#include "ct_step/abstract/ct_abstractstep.h"

/*!
 * \class ONF_StepComputeLAI2000Data
 * \ingroup Steps_OE
 * \brief <b>Computes indicators correponding to LAI-2000 outputs.</b>
 *
 * \param _resInRadians Scan angular resolution
 *
 *
 * <b>Input Models:</b>
 *
 * - CT_ResultGroup (inResultModel)\n
 *     - CT_StandardItemGroup...\n
 *         - CT_Scene (sceneModel)\n
 *
 * <b>Output Models:</b>
 *
 * - CT_ResultGroup (outResultModel)\n
 *     - CT_StandardItemGroup (groupModel)...\n
 *         - CT_Scene (laiDataModel)\n
 *
 */

class ONF_StepComputeLAI2000Data: public CT_AbstractStep
{
    Q_OBJECT

public:

    /*! \brief Step constructor
     * 
     * Create a new instance of the step
     * 
     * \param dataInit Step parameters object
     */
    ONF_StepComputeLAI2000Data(CT_StepInitializeData &dataInit);

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
    double    _resInRadians;    /*!< Scan angular resolution */

};

#endif // ONF_STEPCOMPUTELAI2000DATA_H
