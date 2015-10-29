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
