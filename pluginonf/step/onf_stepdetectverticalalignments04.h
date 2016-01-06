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

#ifndef ONF_STEPDETECTVERTICALALIGNMENTS04_H
#define ONF_STEPDETECTVERTICALALIGNMENTS04_H

#include "ct_step/abstract/ct_abstractstep.h"

#include "ct_itemdrawable/ct_standarditemgroup.h"
#include "ct_itemdrawable/ct_line.h"


// Inclusion of auto-indexation system
#include "ct_tools/model/ct_autorenamemodels.h"
#include "ct_point.h"
#include "ct_itemdrawable/ct_pointcluster.h"

#include "eigen/Eigen/Core"

class CT_StandardItemGroup;


class ONF_StepDetectVerticalAlignments04: public CT_AbstractStep
{
    Q_OBJECT

public:

    /*! \brief Step constructor
     *
     * Create a new instance of the step
     *
     * \param dataInit Step parameters object
     */
    ONF_StepDetectVerticalAlignments04(CT_StepInitializeData &dataInit);

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

    struct DistValues {
        double _min;
        double _max;
        double _mean;
        double _q25;
        double _q50;
        double _q75;
    };


    struct ClusterHeightData {
        ClusterHeightData(CT_PointCluster* cluster, CT_LineData* mergedLineData)
        {
            _cluster = cluster;
            _mergedLineData = mergedLineData;
            _hMaxScene = 0;
        }

        CT_PointCluster* _cluster;
        CT_LineData* _mergedLineData;
        double _hMaxScene;
    };

    struct LineData {
        LineData(const CT_Point &pLow, const CT_Point &pHigh, size_t index1, size_t index2, float phi, double bottomLevel, double topLevel, double lineOfScan)
        {
            _processed = false;
            _distSum = 0;

            _index1 = index1;
            _index2 = index2;
            _pLow = pLow;
            _pHigh = pHigh;
            _phi = phi;

            Eigen::Vector3d dir = pHigh - pLow;
            dir.normalize();

            double t = (bottomLevel - pLow(2)) / dir(2);

            _lowCoord(0) = pLow(0) + t*dir(0);
            _lowCoord(1) = pLow(1) + t*dir(1);
            _lowCoord(2) = bottomLevel;

            t = (topLevel - pLow(2)) / dir(2);

            _highCoord(0) = pLow(0) + t*dir(0);
            _highCoord(1) = pLow(1) + t*dir(1);
            _highCoord(2) = topLevel;

            _lineOfScan = lineOfScan;
        }

        float _phi;
        size_t _index1;
        size_t _index2;
        Eigen::Vector3d _pLow;
        Eigen::Vector3d _pHigh;
        Eigen::Vector3d _lowCoord;
        Eigen::Vector3d _highCoord;
        double          _lineOfScan;


        QList<ONF_StepDetectVerticalAlignments04::LineData*> _neighbors;
        bool                                                 _processed;
        double                                               _distSum;

        inline size_t neighborsCount() const {return _neighbors.size();}

    };

    static bool orderByAscendingPhi(LineData *s1, LineData *s2)
    {
        return s1->_phi < s2->_phi;
    }

    static bool orderByDescendingNeighborCount(LineData *s1, LineData *s2)
    {
        if (s1->neighborsCount() > 0 && s1->neighborsCount() == s2->neighborsCount()) {return (s1->_distSum / s1->neighborsCount()) < (s2->_distSum / s2->neighborsCount());}
        return s1->neighborsCount() > s2->neighborsCount();
    }

    static bool orderByAscendingNumberOfPoints(CT_PointCluster *cl1, CT_PointCluster *cl2)
    {
        return cl1->getPointCloudIndexSize() < cl2->getPointCloudIndexSize();
    }

    class AlignmentsDetectorForScene
    {
    public:

        AlignmentsDetectorForScene(ONF_StepDetectVerticalAlignments04* step, CT_ResultGroup* res)
        {
            _step = step;
            _res = res;
        }

        typedef void result_type;
        void operator()(CT_StandardItemGroup* grp)
        {
            detectAlignmentsForScene(grp);
        }

        void detectAlignmentsForScene(CT_StandardItemGroup* grp);
        void validateScanLineCluster(CT_PointCluster *cluster, QList<CT_PointCluster *> &keptClusters, QList<size_t> &isolatedPointIndices, CT_PointAccessor &pointAccessor);
        void processCurrentList(QList<size_t> &isolatedPointIndices, QList<size_t> &currentList, QList<CT_PointCluster*> &keptClusters);
        void computeDBH(CT_PointCluster* cluster, Eigen::Vector3d &center, double &maxDist);



        void detectAlignmentsForScene_old(CT_StandardItemGroup* grp);

        ONF_StepDetectVerticalAlignments04::DistValues* computeDistVals(const CT_AbstractPointCloudIndex* cloudIndex, CT_LineData* lineData);
        void findNeighborLines(QList<ONF_StepDetectVerticalAlignments04::LineData*> candidateLines, double distThreshold);
        void sendToDroppedList(CT_StandardItemGroup* grp, CT_PointCluster* cluster, CT_LineData* lineData);
        void sendToConservedList(CT_StandardItemGroup* grp, CT_PointCluster* cluster, CT_LineData* mergedLineData, double hMaxScene);


    private:
        ONF_StepDetectVerticalAlignments04* _step;
        CT_ResultGroup* _res;
    };



    // Declaration of autoRenames Variables (groups or items added to In models copies)
    CT_AutoRenameModels    _grpCluster_ModelName;
    CT_AutoRenameModels    _cluster_ModelName;
    CT_AutoRenameModels    _line_ModelName;
    CT_AutoRenameModels    _attMaxDistXY_ModelName;


    CT_AutoRenameModels    _convexProj_ModelName;
    CT_AutoRenameModels    _circle_ModelName;
    CT_AutoRenameModels    _attMin_ModelName;
    CT_AutoRenameModels    _attQ25_ModelName;
    CT_AutoRenameModels    _attQ50_ModelName;
    CT_AutoRenameModels    _attQ75_ModelName;
    CT_AutoRenameModels    _attMax_ModelName;
    CT_AutoRenameModels    _attMean_ModelName;
    CT_AutoRenameModels    _attMaxDist_ModelName;
    CT_AutoRenameModels    _attDiamEq_ModelName;
    CT_AutoRenameModels    _attHmax_ModelName;

    CT_AutoRenameModels    _grpClusterDebug_ModelName;
    CT_AutoRenameModels    _grpClusterDebug2_ModelName;
    CT_AutoRenameModels    _clusterDebug_ModelName;
    CT_AutoRenameModels    _clusterDebug2_ModelName;


    // Step parameters
    double      _thresholdGPSTime;
    double      _thresholdDistXY;
    double      _thresholdZenithalAngle;
    double      _thresholdNeighbourTesting;
    double      _minimalMaxDistXY;
    int         _minPts;
    double      _DBH_azimRes;
    double      _DBH_zeniRes;
    double      _DBH_zeniMax;

    bool      _clusterDebugMode;



    double    _maxPhiAngle;
    double    _pointDistThreshold;
    double    _lineDistThreshold;
    double    _clusterDistThreshold;
    double    _maxDivergence;
    double    _maxPhiAngleAfterMerging;
    double    _radiusHmax;
    double    _maxDiameter;
    int       _minPtsNb;
    double    _lineLengthRatio;
    double    _lengthThreshold;
    double    _heightThreshold;
    double    _maxDiamRatio;
    double    _circleDistThreshold;

};

#endif // ONF_STEPDETECTVERTICALALIGNMENTS04_H
