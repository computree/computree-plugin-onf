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


#ifndef ONF_STEPMERGEENDTOENDSECTIONS04_H
#define ONF_STEPMERGEENDTOENDSECTIONS04_H

#include "ct_step/abstract/ct_abstractstep.h"
#include "ct_tools/model/ct_autorenamemodels.h"
#include "ct_itemdrawable/abstract/ct_abstractitemgroup.h"


#include "ct_itemdrawable/ct_referencepoint.h"
#include "ct_result/ct_resultgroup.h"
#include "ct_itemdrawable/ct_pointcluster.h"
#include "ct_itemdrawable/ct_standarditemgroup.h"
#include "ct_shapedata/ct_linedata.h"



/*!
 * \class ONF_StepMergeEndToEndSections04
 * \ingroup Steps_OE
 * \brief <b>Merging of sections with a vertical continuity, supposed describing the same real tree.</b>
 *
 * \em First: new clusters of vertical thickness \b _thickness are created in each section,
 * with the algorithm of the step ONF_StepChangeClusterThickness02. \n
 * \em Second: CT_ReferencePoint are created for barycenters of new clusters,
 * with the algorithm of the step ONF_StepRefPointFromBarycenter02. \n
 * \em Note : the CT_ReferencePoint xyBuffer value is the horizontal distance between the barycenter and the farthest point of the cluster. \n
 * \em Third: sections are taken by increasing vertical position (first cluster).
 * Each one (base section) is compared with other sections (tested sections).
 * A "base" line is fitted on the \b _n lasts (top) barycenters of the base section.
 * An "tested" line is fittes on the _n firsts (bottom) barycenters of the tested section.
 * A first intersection is computed between the base line and a plane at the tested first barycenter, perpendicular to tested line.
 * A second intersection is computed between the tested line and a plane at the base last barycenter, perpendicular to base line.
 *
 * For each intersection, the distance in the intersection plane and the corresponding barycenter is computed.
 * If the two intersections are inferior to \b _mult * MaxDist, the sections are merged.
 * MaxDist is computed as the maximum xyBuffer value of all barycenters of base section.
 *
 *
 * \param _thickness Vertical thickness of clusters (in meters)
 * \param _searchDistance Maximum autorised distance between extremities of two sections to be merged (in meters)
 * \param _n Number of barycenters to consider to define extremities directions
 * \param  _mult Multiplicative factor for MaxDist
 * \param _zTolerance Maximum accepted Z overlaping between sections to merge
 *
 *
 *
 * <b>Input Models:</b>
 *
 *  - CT_ResultGroup \n
 *      - CT_StandardItemGroup (Section)... \n
 *          - CT_StandardItemGroup (ClusterGroup)... \n
 *              - CT_PointCluster (Cluster) \n
 *
 * <b>Output Models:</b>
 *
 *  - CT_ResultGroup \n
 *      - CT_StandardItemGroup (Section)... \n
 *          - CT_StandardItemGroup (ClusterGroup)... \n
 *              - CT_PointCluster (Cluster) \n
 *              - CT_ReferencePoint (Barycenter) \n
 *
 */
class ONF_StepMergeEndToEndSections04 : public CT_AbstractStep
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
    ONF_StepMergeEndToEndSections04(CT_StepInitializeData &dataInit);

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

    double _thickness;      /*!< Epaisseur des clusters */
    double _n;             /*!< Nombre de barycentres a considerer aux extremitÃ©s */
    double _mult;          /*!< Facteur multiplicatif de maxDist */
    double _zTolerance;    /*!< Chevauchement en Z tolÃ©rÃ© */
    double _searchDistance; /*!< Distance 3D maximale entre extremitÃ©s de sections Ã  fusionner */


    CT_AutoRenameModels                 _autoSearchOutClusterGroup;
    CT_AutoRenameModels                 _autoSearchOutPointCluster;
    CT_AutoRenameModels                 _autoSearchOutRefPoint;

    CT_StandardItemGroup* sectionFromSegment(QList<CT_PointCluster*> clustersList, CT_ResultGroup *outResult, QList<CT_ReferencePoint*> *refPoints);
    QList<CT_PointCluster*> mergeSkeletonSegments(CT_StandardItemGroup* baseSection, CT_StandardItemGroup* testedSection);
    float distanceFromExtremityToLine(CT_LineData *lineL, double plan_x, double plan_y, double plan_z);

};

#endif // ONF_STEPMERGEENDTOENDSECTIONS04_H
