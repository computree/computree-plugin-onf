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

#ifndef ONF_STEPDETECTVERTICALALIGNMENTS06_H
#define ONF_STEPDETECTVERTICALALIGNMENTS06_H

#ifdef USE_OPENCV

#include "ct_step/abstract/ct_abstractstep.h"

#include "ct_itemdrawable/ct_standarditemgroup.h"
#include "ct_itemdrawable/ct_line.h"
#include "ct_itemdrawable/ct_circle2d.h"
#include "ct_itemdrawable/ct_image2d.h"


// Inclusion of auto-indexation system
#include "ct_tools/model/ct_autorenamemodels.h"
#include "ct_point.h"
#include "ct_itemdrawable/ct_pointcluster.h"
#include "ct_accessor/ct_pointaccessor.h"
#include "ct_itemdrawable/abstract/ct_abstractpointattributesscalar.h"

#include "eigen/Eigen/Core"

class CT_StandardItemGroup;


class ONF_StepDetectVerticalAlignments06: public CT_AbstractStep
{
    Q_OBJECT

public:

    /*! \brief Step constructor
     *
     * Create a new instance of the step
     *
     * \param dataInit Step parameters object
     */
    ONF_StepDetectVerticalAlignments06(CT_StepInitializeData &dataInit);

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
    QMap<double, double> _radii;
    double getExclusionRadius(double height);


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

protected:

    class ScanLineData : public QList<size_t> {
    public:
        ScanLineData() : QList<size_t>()
        {
            _length = 0;
            _centerX = 0;
            _centerY = 0;
            _centerZ = 0;
        }

        ScanLineData(const ScanLineData& line) : QList<size_t>(line)
        {
            _length = line._length;
            _centerX = line._centerX;
            _centerY = line._centerY;
            _centerZ = line._centerZ;
        }

        ScanLineData(double length, double centerX, double centerY, double centerZ) : QList<size_t>()
        {
            _length = length;
            _centerX = centerX;
            _centerY = centerY;
            _centerZ = centerZ;
        }

        ScanLineData(const QList<size_t> &list, double length, double centerX, double centerY, double centerZ) : QList<size_t>(list)
        {
            _length = length;
            _centerX = centerX;
            _centerY = centerY;
            _centerZ = centerZ;
        }

        ScanLineData& operator=(const ScanLineData& arg)
        {
            this->append(arg);
            this->_length = arg._length;
            this->_centerX = arg._centerX;
            this->_centerY = arg._centerY;
            this->_centerZ = arg._centerZ;
            return *this;
        }

        double _length;
        double _centerX;
        double _centerY;
        double _centerZ;

    };

    static bool orderLinesByAscendingNumberAndLength(ScanLineData *l1, ScanLineData *l2)
    {
        if (l1->size() < l2->size()) {return true;}
        if (l1->size() > l2->size()) {return false;}
        return l1->_length < l2->_length;
    }

    struct LineData {
        LineData(const CT_Point &pLow, const CT_Point &pHigh, size_t index1, size_t index2, float phi, double bottomLevel, double topLevel)
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
        }

        float _phi;
        size_t _index1;
        size_t _index2;
        Eigen::Vector3d _pLow;
        Eigen::Vector3d _pHigh;
        Eigen::Vector3d _lowCoord;
        Eigen::Vector3d _highCoord;


        QList<ONF_StepDetectVerticalAlignments06::LineData*> _neighbors;
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

        AlignmentsDetectorForScene(ONF_StepDetectVerticalAlignments06* step, CT_ResultGroup* res)
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
        inline double weightedScore(bool applySigmoid, double val, const double &k, const double &x0)
        {
            if (applySigmoid)
            {
                return 1.0/ (1.0 + std::exp(-k*(val - x0)));
            }
            return val;
        }
        
        void createCHM(const CT_AbstractItemDrawableWithPointCloud* sceneStemAll,
                       CT_Image2D<float>* maxHeightRaster);

        void sortIndicesByGPSTime(const CT_AbstractPointCloudIndex* pointCloudIndexLAS,
                                  const CT_AbstractPointAttributesScalar* attributeGPS,
                                  const CT_AbstractPointCloudIndex* pointCloudIndex,
                                  QMultiMap<double, size_t> &sortedIndices);

        void createLinesOfScan(const QMultiMap<double, size_t> &sortedIndices,
                               QList<QList<size_t> > &linesOfScan,
                               QList<size_t> &isolatedPointIndices);

        void filterLinesOfScan(QList<QList<size_t> > &simplifiedLinesOfScan,
                               double thresholdZenithalAngleRadians,
                               QList<ScanLineData *> &keptLinesOfScan,
                               QList<size_t> &isolatedPointIndices);

        void denoiseLinesOfScan(const QList<QList<size_t> > &linesOfScan,
                                const CT_AbstractPointCloudIndex* pointCloudIndexLAS,
                                const CT_AbstractPointAttributesScalar* attributeIntensity,
                                CT_StandardItemGroup* grp,
                                QList<QList<size_t> > &simplifiedLinesOfScan,
                                QList<size_t> &isolatedPointIndices);

        void getPointsOfMainLine(const CT_AbstractPointCloudIndex* pointCloudIndexLAS,
                                 const CT_AbstractPointAttributesScalar* attributeLineOfFlight,
                                 const ScanLineData *mainLine,
                                 QList<CT_Point> &mainLinePoints,
                                 int &mainLineOfFlight);

        void findNeighbours(const ScanLineData *mainLine,
                            const CT_AbstractPointCloudIndex* pointCloudIndexLAS,
                            const CT_AbstractPointAttributesScalar* attributeLineOfFlight,
                            QList<ScanLineData *> &keptLinesOfScan,
                            QList<ScanLineData *> &neighbourLines,
                            QList<CT_Point> &neighbourPoints,
                            QList<int> &neighbourPointsToTest,
                            QList<int> &neighbourPointsToTestIfOnlyOneLineOfFlight,
                            int &mainLineOfFlight);

        void findBestDirectionAndDiameter(double thresholdZenithalAngleRadians,
                                          const ScanLineData *mainLine,
                                          const QList<CT_Point> &mainLinePoints,
                                          const QList<CT_Point> &neighbourPoints,
                                          const QList<int> &neighbourPointsToTest,
                                          Eigen::Vector3d &bestDirection,
                                          double &diameter,
                                          double &bestScore);

        double computeHmaxForTree(double centerX,
                                  double centerY,
                                  const CT_Image2D<float>* maxHeightRaster);

        CT_PointCluster *computeDiameterAlongFirstLastLine(double centerX,
                                               double centerY,
                                               double centerZ,
                                               const ScanLineData *mainLine,
                                               const QList<ScanLineData *> &neighbourLines,
                                               Eigen::Vector3d &bestDirection,
                                               double &diameter,
                                               QList<size_t> &isolatedPointIndices,
                                               CT_PointCluster* cluster);

        void removePointsToCloseFromDetectedDiameters(const QList<CT_Circle2D *> &circles,
                                                      QList<size_t> &isolatedPointIndices);

        double computeDiameterAlongLine(const CT_PointCluster *cluster,
                                        const Eigen::Vector3d &direction,
                                        const Eigen::Vector3d &origin);

        CT_Circle2D *addClusterToResult(CT_StandardItemGroup* grp,
                                        CT_PointCluster* cluster,
                                        double diameter,
                                        int type,
                                        double centerX,
                                        double centerY,
                                        double centerZ,
                                        double length,
                                        const Eigen::Vector3d &direction,
                                        double score);

        void findNeighborLines(QList<LineData *> &candidateLines,
                               double distThreshold);

        void findCandidateLines(const CT_AbstractItemDrawableWithPointCloud* sceneStem,
                                QList<LineData *> &candidateLines,
                                QList<size_t> &isolatedPointIndices);

        void createClusterFromCandidateLines(ONF_StepDetectVerticalAlignments06::LineData* candidateLine,
                                             CT_PointCluster* cluster,
                                             QList<size_t> &insertedPoints);

        void computeHmaxForEachDetectedStem(const CT_AbstractItemDrawableWithPointCloud* sceneStemAll,
                                            const QList<CT_Circle2D *> &circles,
                                            QVector<double> &heights);

        double computeAllometricDFromH(double h);

        void applyExclusionRadiiToHeightsVector(const QList<CT_Circle2D *> &circles,
                                                QVector<double> &heights);

        bool testLengthBetweenPoints(const CT_AbstractPointCloudIndex* cloudIndex,
                                     const CT_LineData* fittedLineData);


    private:
        ONF_StepDetectVerticalAlignments06* _step;
        CT_ResultGroup* _res;
    };



    // Declaration of autoRenames Variables (groups or items added to In models copies)
    CT_AutoRenameModels    _grpCluster_ModelName;
    CT_AutoRenameModels    _cluster_ModelName;
    CT_AutoRenameModels    _attMaxDistXY_ModelName;
    CT_AutoRenameModels    _attScore_ModelName;
    CT_AutoRenameModels    _attStemType_ModelName;
    CT_AutoRenameModels    _circle_ModelName;
    CT_AutoRenameModels    _attMaxDistXY2_ModelName;
    CT_AutoRenameModels    _attScore2_ModelName;
    CT_AutoRenameModels    _attStemType2_ModelName;
    CT_AutoRenameModels    _line_ModelName;
    CT_AutoRenameModels    _attMaxDistXY3_ModelName;
    CT_AutoRenameModels    _attScore3_ModelName;
    CT_AutoRenameModels    _attStemType3_ModelName;
    CT_AutoRenameModels    _attMaxHeight_ModelName;    
    CT_AutoRenameModels    _attCorrectedDiameter_ModelName;
    CT_AutoRenameModels    _attCorrectedFlag_ModelName;


    CT_AutoRenameModels    _grpClusterDebug1_ModelName;
    CT_AutoRenameModels    _grpClusterDebug2_ModelName;
    CT_AutoRenameModels    _grpClusterDebug3_ModelName;
    CT_AutoRenameModels    _clusterDebug1_ModelName;
    CT_AutoRenameModels    _clusterDebug2_ModelName;
    CT_AutoRenameModels    _clusterDebug3_ModelName;


    // Step parameters
    double      _thresholdGPSTime;
    double      _maxCurvature;
    double      _maxXYDist;
    double      _thresholdZenithalAngle;
    int         _minPts;

    double      _minDiameter;
    double      _maxDiameter;
    double      _maxDiameterForUnderstorey;
    double      _maxSearchRadius;
    double      _maxLineSpacing;
    double      _resolutionForDiameterEstimation;


    bool        _applySigmoid;
    double      _sigmoidCoefK;
    double      _sigmoidX0;

    double      _ratioDbhNbPtsMax;
    double      _monoLineMult;

    double      _maxPhiAngleSmall;
    double      _pointDistThresholdSmall;
    double      _lineDistThresholdSmall;
    int         _minPtsSmall;
    double      _lineLengthRatioSmall;
    double      _exclusionRadiusSmall;

    double      _radiusHmax;
    double      _param_Hmax;
    double      _param_a;
    double      _param_m;
    double      _deltaHmax;
    QStringList _fileName;

    bool      _clusterDebugMode;

};

#endif

#endif // ONF_STEPDETECTVERTICALALIGNMENTS06_H
