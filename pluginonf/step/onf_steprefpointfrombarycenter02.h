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


#ifndef ONF_STEPREFPOINTFROMBARYCENTER02_H
#define ONF_STEPREFPOINTFROMBARYCENTER02_H

#include "ct_step/abstract/ct_abstractstep.h"
#include "ct_tools/model/ct_autorenamemodels.h"
#include "ct_itemdrawable/abstract/ct_abstractitemgroup.h"
#include "ct_itemdrawable/ct_pointcluster.h"
#include "ct_itemdrawable/ct_referencepoint.h"

/*!
 * \class ONF_StepRefPointFromBarycenter02
 * \ingroup Steps_OE
 * \brief <b>Short description of the step.</b>
 *
 * Detailed decription of step purpose.
 * Please also give a general view of the algorithm.
 *
 * \param ParameterName Description of the parameter to give in the DialogBox
 *
 *
 * <b>Input Models:</b>
 *
 *  - CT_ResultGroup \n
 *      - CT_StandardItemGroup (name)... \n
 *          - CT_ItemDrawable (name) \n
 *          - CT_ItemDrawable (name) \n
 *
 * <b>Output Models:</b>
 *
 *  - CT_ResultGroup \n
 *      - CT_StandardItemGroup (Section)... \n
 *          - <em>cpy CT_ItemDrawable (name)</em> \n
 *          - <em>cpy+ CT_ItemDrawable (name)</em> \n
 *
 */
class ONF_StepRefPointFromBarycenter02 : public CT_AbstractStep
{
    // IMPORTANT in order to obtain step name
    Q_OBJECT

public:

    /*! \brief Step constructor
     *
     * Create a new instance of the step
     *
     * \param dataInit Step parameters object
     */
    ONF_StepRefPointFromBarycenter02(CT_StepInitializeData &dataInit);

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


    static CT_ReferencePoint* addBarycenter(const CT_PointCluster *item, CT_AbstractItemGroup *group, const QString &outRefPointModelName, CT_ResultGroup *outResult);

protected:

    /*! \brief Input results specification
     *
     * Specification of input results models needed by the step (IN)
     */
    void createInResultModelListProtected();

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

    CT_AutoRenameModels _outRefPointModelName;
};


#endif // ONF_STEPREFPOINTFROMBARYCENTER02_H
