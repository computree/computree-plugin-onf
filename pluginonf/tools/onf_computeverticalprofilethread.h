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


#ifndef ONF_COMPUTEVERTICALPROFILETHREAD_H
#define ONF_COMPUTEVERTICALPROFILETHREAD_H

#include "ct_tools/ct_monitoredqthread.h"

#include "ct_itemdrawable/ct_scene.h"
#include "ct_itemdrawable/ct_profile.h"


#include <QThread>

class ONF_ComputeVerticalProfileThread : public CT_MonitoredQThread
{
    Q_OBJECT
public:
    ONF_ComputeVerticalProfileThread(CT_Profile<int> *profile,
                         const CT_Scene *scene);

    void run();

private:
    CT_Profile<int>*     _profile;
    const CT_Scene*     _scene;
};

#endif // ONF_COMPUTEVERTICALPROFILETHREAD_H
