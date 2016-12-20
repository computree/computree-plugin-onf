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

#ifndef ONF_STEPMATCHITEMSPOSITIONS_H
#define ONF_STEPMATCHITEMSPOSITIONS_H

#include "ct_step/abstract/ct_abstractstep.h"
#include "eigen/Eigen/Core"
#include <math.h>


class ONF_StepMatchItemsPositions: public CT_AbstractStep
{
    Q_OBJECT

public:

    /*! \brief Step constructor
     * 
     * Create a new instance of the step
     * 
     * \param dataInit Step parameters object
     */
    ONF_StepMatchItemsPositions(CT_StepInitializeData &dataInit);

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
     *
     * Adapted from :
     * Marius Hauglin, Vegard Lien, Erik Næsset & Terje Gobakken (2014)
     * Geo-referencing forest field plots by co-registration of terrestrial and airborne laser scanning data,
     * International Journal of Remote Sensing, 35:9, 3135-3149, DOI: 10.1080/01431161.2014.903440
     */
    void compute();

private:

    // Step parameters
    double      _distThreshold;
    double      _relativeSizeThreshold;
    double      _minRelativeSize;
    double      _maxTheta;
    bool        _possiblyInvertedDirection;
    double      _coef_nbRwc;
    double      _coef_nbTwc;
    double      _coef_nbSim;
    int         _drawMode;
    int         _relativeMode;
    double      _minval;
    double      _maxval;
    bool        _exportReport;
    QStringList _reportFileName;
    bool        _exportData;
    QStringList _transformedDataFileName;
    QStringList _transformationDataFileName;


    template <typename T> int sgn(T val)
    {
        return (val > T(0)) - (val < T(0));
    }

    Eigen::Matrix3d Kabsch(Eigen::MatrixXd &refPositions, Eigen::MatrixXd &transPositions, int pointsNumber, Eigen::Vector3d &center, Eigen::Vector3d &translation);

    Eigen::Matrix3d computeTransfMatrix2D(const Eigen::Vector2d &center, const Eigen::Vector2d &tranlation, const Eigen::Matrix2d &rotMat);

};

#endif // ONF_STEPMATCHITEMSPOSITIONS_H
