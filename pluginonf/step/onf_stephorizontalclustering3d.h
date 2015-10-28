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


#ifndef ONF_STEPHORIZONTALCLUSTERING3D_H
#define ONF_STEPHORIZONTALCLUSTERING3D_H

#include "ct_step/abstract/ct_abstractstep.h"

#include "ct_pointcloud/abstract/ct_abstractpointcloud.h"
#include "ct_itemdrawable/ct_pointcluster.h"
#include "ct_itemdrawable/ct_standarditemgroup.h"

#include "ct_tools/model/ct_autorenamemodels.h"

#include "ct_accessor/ct_pointaccessor.h"

#include <QMutex>

/*!
 * \class ONF_StepHorizontalClustering3D
 * \ingroup Steps_OE
 * \brief <b>Create small clusters of points in horizontal thin layers by scene</b>
 *
 *
 */
class ONF_StepHorizontalClustering3D : public CT_AbstractStep
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
    ONF_StepHorizontalClustering3D(CT_StepInitializeData &dataInit);

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

    double  _maxSearchRadiusInMeters;   /*!< distance d'un point à un groupe */
    double  _thicknessInMeters;         /*!< l'epaisseur des couches */

    int     _sizeMap;
    int     _nLayerFinished;
    QMutex  _mutexUpdateProgress;

    CT_AutoRenameModels     _outLayerGroupModel;
    CT_AutoRenameModels     _outClusterGroupModel;
    CT_AutoRenameModels     _outClusterModel;

    // champs rendus disponibles par context.step()
    CT_ResultGroup                              *_outResult;

    static CT_PointAccessor                     P_ACCESS;

    void addLayerFinished();

    static void staticComputeLayer(CT_StandardItemGroup *layer);

    static bool staticAddPointToPointCluster(const size_t &pointIndex,
                                             CT_PointCluster &pCluster,
                                             ONF_StepHorizontalClustering3D *ptrClass);

    static CT_PointCluster* staticCombinePointCluster(int id,
                                                      CT_AbstractResult &outResult,
                                                      CT_PointCluster &pClusterToOptimize,
                                                      CT_PointCluster &pClusterToCombine, ONF_StepHorizontalClustering3D *ptrClass, QString outClusterModelName);
};

#endif // ONF_STEPHORIZONTALCLUSTERING3D_H
