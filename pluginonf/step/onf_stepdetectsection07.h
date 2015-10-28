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


#ifndef ONF_STEPDETECTSECTION07_H
#define ONF_STEPDETECTSECTION07_H

#include "ct_step/abstract/ct_abstractstep.h"
#include "ct_itemdrawable/model/outModel/abstract/def_ct_abstractgroupmodelout.h"

/*!
 * \class ONF_StepDetectSection07
 * \ingroup Steps_OE
 * \brief <b>Group vertically clusters in sections.</b>
 *
 * CT_PointCluster are grouped in sections, if they have a BoudingBox (x,y) intersection
 * and if they are distant vertically less than _deltaz.
 *
 * \param _deltaz Maximum z distance to be included in an existing section
 *
 *
 * <b>Input Models:</b>
 *
 *  - CT_ResultGroup \n
 *      (...)
 *      - CT_StandardItemGroup (ClusterGroup)... \n
 *          - CT_PointCluster (Cluster) \n
 *
 * <b>Output Models:</b>
 *
 *  - CT_ResultGroup \n
 *      - CT_StandardItemGroup (Section)... \n
 *          - <em>cpy CT_StandardItemGroup (ClusterGroup)...</em> \n
 *              - <em>cpy CT_PointCluster (Cluster)</em> \n
 *
 */
class ONF_StepDetectSection07 : public CT_AbstractStep
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
    ONF_StepDetectSection07(CT_StepInitializeData &dataInit);

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

    void preProcessCreateOutResultModelListProtected();

private:

    double   _deltaz;                       /*!< Distance en z (en + et en -) maximum entre deux groupes de points à comparer */

    QString     _outclustergroupname;
    QString     _outsectiongroupname;

};


#endif // ONF_STEPDETECTSECTION07_H
