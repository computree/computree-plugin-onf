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

#ifndef ONF_STEPLOADPOSITIONSFORMATCHING_H
#define ONF_STEPLOADPOSITIONSFORMATCHING_H

#include "ct_step/ct_stepbeginloop.h"

/*!
 * \class ONF_StepLoadPositionsForMatching
 * \ingroup Steps_PB
 * \brief <b>Charge deux sources de positions pour mise en correspondance.</b>
 *
 * No detailled description for this step
 *
 * \param _refFileName 
 * \param _transFileName 
 *
 */

class ONF_StepLoadPositionsForMatching: public CT_StepBeginLoop
{
    Q_OBJECT

public:

    /*! \brief Step constructor
     * 
     * Create a new instance of the step
     * 
     * \param dataInit Step parameters object
     */
    ONF_StepLoadPositionsForMatching(CT_StepInitializeData &dataInit);

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

    virtual void createOutResultModelListProtected(CT_OutResultModelGroup *firstResultModel);

    virtual void compute(CT_ResultGroup *outRes, CT_StandardItemGroup* group);


private:

    // Step parameters
    QStringList _neededFields;

    QString _refFileName;
    QString _transFileName;

    bool _refHeader;
    bool _transHeader;

    QString _refSeparator;
    QString _transSeparator;

    QString _refDecimal;
    QString _transDecimal;

    QLocale _refLocale;
    QLocale _transLocale;


    int _refSkip;
    int _transSkip;

    QMap<QString, int> _refColumns;
    QMap<QString, int> _transColumns;

    QList<QString> _plotsIds;


};

#endif // ONF_STEPLOADPOSITIONSFORMATCHING_H
