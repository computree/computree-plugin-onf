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


#ifndef ONF_STEPPLUGINMANAGER_H
#define ONF_STEPPLUGINMANAGER_H

#include "ct_abstractstepplugin.h"

class ONF_StepPluginManager : public CT_AbstractStepPlugin
{
public:
    ONF_StepPluginManager();
    ~ONF_StepPluginManager();

    QString getPluginURL() const {return QString("http://rdinnovation.onf.fr/projects/plugin-onf/wiki");}

protected:

    bool loadGenericsStep();
    bool loadOpenFileStep();
    bool loadCanBeAddedFirstStep();
    bool loadFilters();
    bool loadMetrics();

    bool loadActions();
    bool loadExporters();
    bool loadReaders();
};

#endif // ONF_STEPPLUGINMANAGER_H
