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


#ifndef ONF_PLUGINENTRY_H
#define ONF_PLUGINENTRY_H

#include "interfaces.h"

class ONF_StepPluginManager;

class ONF_PluginEntry : public PluginEntryInterface
{
    Q_OBJECT

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID PluginEntryInterface_iid)
#endif

    Q_INTERFACES(PluginEntryInterface)

public:
    ONF_PluginEntry();
    ~ONF_PluginEntry();

    QString getVersion() const;

    CT_AbstractStepPlugin* getPlugin() const;

private:
    ONF_StepPluginManager *_stepPluginManager;
};

#endif // ONF_PLUGINENTRY_H
