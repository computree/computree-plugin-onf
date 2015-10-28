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


#ifndef ONF_STEPFILTERCLUSTERSBYSIZE_H
#define ONF_STEPFILTERCLUSTERSBYSIZE_H

#include "ct_step/abstract/ct_abstractstep.h"

class CT_AbstractItemGroup;

/*!
 * \class ONF_StepFilterClustersBySize
 * \ingroup Steps_OE
 * \brief <b>Drop clusters with less than _minSize points.</b>
 *
 * For each CT_AbstractItemDrawableWithPointCloud, the number of CT_Point is count.
 * If the number is less than \b _minSize, the group containing the cluster is dropped.
 * If all groups in a parent group are dropped, this parent group is removed too (and so on recusively).
 *
 * \param _minSize Minimum number of point in a cluster
 *
 *
 * <b>Input Models:</b>
 *
 *  - CT_ResultGroup \n
 *      (...)
 *      - CT_StandardItemGroup (ClusterGroup)... \n
 *          - CT_AbstractItemDrawableWithPointCloud (Cluster) \n
 *
 * <b>Output Models:</b>
 *
 *  - CT_ResultGroup \n
 *      (...)
 *      - <em>cpy- CT_StandardItemGroup (ClusterGroup)...</em> \n
 *          - <em>cpy CT_AbstractItemDrawableWithPointCloud (Cluster)</em> \n
 *
 */
class ONF_StepFilterClustersBySize : public CT_AbstractStep
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
    ONF_StepFilterClustersBySize(CT_StepInitializeData &dataInit);

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

    int     _minSize;   /*!< Nombre de points minimum dans un cluster */

    void recursiveRemoveGroupIfEmpty(CT_AbstractItemGroup *parent, CT_AbstractItemGroup *group) const;
};
#endif // ONF_STEPFILTERCLUSTERSBYSIZE_H
