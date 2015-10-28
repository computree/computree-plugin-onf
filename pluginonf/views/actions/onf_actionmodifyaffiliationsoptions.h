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

#ifndef ONF_ACTIONMODIFYAFFILIATIONSOPTIONS_H
#define ONF_ACTIONMODIFYAFFILIATIONSOPTIONS_H

#include "ct_view/actions/abstract/ct_gabstractactionoptions.h"

class ONF_ActionModifyAffiliations;

namespace Ui {
class ONF_ActionModifyAffiliationsOptions;
}

class ONF_ActionModifyAffiliationsOptions : public CT_GAbstractActionOptions
{
    Q_OBJECT

public:

    explicit ONF_ActionModifyAffiliationsOptions(const ONF_ActionModifyAffiliations *action);
    ~ONF_ActionModifyAffiliationsOptions();

    bool sourceVisible();
    bool targetVisible();
    bool affiliationsLinesVisible();
    bool itemsVisible();
    bool centersVisible();
    bool onlySelectedVisible();
    bool selectionActivated();
    bool showAttributes();

public slots:
    void toggleSelection();

signals:
    void parametersChanged();
    void askForAffiliation();
    void askForBreakingAffiliation();

private:
    Ui::ONF_ActionModifyAffiliationsOptions *ui;

};

#endif // ONF_ACTIONMODIFYAFFILIATIONSOPTIONS_H
