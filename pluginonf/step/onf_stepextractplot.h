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


#ifndef ONF_STEPEXTRACTPLOT_H
#define ONF_STEPEXTRACTPLOT_H

#include "ct_step/abstract/ct_abstractstep.h"

/*!
 * \class ONF_StepExtractPlot
 * \ingroup Steps_OE
 * \brief <b>Extract a sub-plot from original scene</b>
 *
 * This step only keeps points included in the enveloppe defined by the steps parameter.
 * The enveloppe could be a simple cylinder (if azimuts and radius parameters are not changed).
 * The enveloppe is the zone beetween an external and an internal cylinder, both centered at the same point.
 * The internal cylinder is so included in the external one.
 * All points outside the external cylinder are removed.
 * All points inside the internal cylinder are removed also.
 * Moreover, only point beetween _azbegin and _azend (defined from the common center of cylinders), and beetween _zmin and _zmax are conserved.
 *
 * \param _x X coordinate of the center of both cylinders
 * \param _y Y coordinate of the center of both cylinders
 * \param _radiusmin Radius (m) of the internal cylinder
 * \param _radius Radius (m) of the external cylinder
 * \param _azbegin Begining azimut (grades : 0 - 400) of the conserved zone
 * \param _azend Ending azimut (grades : 0 - 400) of the conserved zone
 * \param _zmin Minimum Z value of conserved points
 * \param _zmax Maximum Z value of conserved points
 *
 *
 * <b>Input Models:</b>
 *
 *  - CT_ResultGroup \n
 *      (...)
 *      - CT_StandardItemGroup \n
 *          - CT_Scene (Scene) \n
 *
 * <b>Output Models:</b>
 *
 *  - CT_ResultGroup \n
 *      - CT_StandardItemGroup \n
 *          - CT_Scene (Scene) \n
 *
 */
class ONF_StepExtractPlot : public CT_AbstractStep
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
    ONF_StepExtractPlot(CT_StepInitializeData &dataInit);

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

    double _x;                            /*!< Coordonnee X du centre de la placette a extraire*/
    double _y;                            /*!< Coordonnee Y du centre de la placette a extraire*/
    double _radiusmin;                    /*!< Rayon (m) de debut de la placette a extraire*/
    double _radius;                       /*!< Rayon (m) de la placette a extraire*/
    double _azbegin;                      /*!< Azimut de debut (degres) de la placette a extraire*/
    double _azend;                        /*!< Azimut de fin (degres) de la placette a extraire*/
    double _zmin;                         /*!< Z minimum de la placette a extraire*/
    double _zmax;                         /*!< Z maximum de la placette a extraire*/

};


#endif // ONF_STEPEXTRACTPLOT_H
