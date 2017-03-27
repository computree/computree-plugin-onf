CT_PREFIX = ../../computreev3

CHECK_CAN_USE_GEOS = 1
CHECK_CAN_USE_PCL = 1
CHECK_CAN_USE_GDAL = 1
CHECK_CAN_USE_OPENCV = 1


include($${CT_PREFIX}/shared.pri)
include($${PLUGIN_SHARED_DIR}/include.pri)

COMPUTREE += ctliblas ctlibfilters ctlibmetrics

include($${CT_PREFIX}/include_ct_library.pri)

greaterThan(QT_MAJOR_VERSION, 4): QT += concurrent

TARGET = plug_onf

HEADERS += \
    $${PLUGIN_SHARED_INTERFACE_DIR}/interfaces.h \
    onf_pluginentry.h \
    onf_steppluginmanager.h \
    actions/onf_actiondefineheightlayer.h \
    actions/onf_actionmanualinventory.h \
    actions/onf_actionmodifyaffiliations.h \
    actions/onf_actionmodifyclustersgroups02.h \
    actions/onf_actionmodifypositions2d.h \
    actions/onf_actionsegmentcrowns.h \
    actions/onf_actionsegmentgaps.h \
    actions/onf_actionselectcellsingrid3d.h \
    actions/onf_actionslicepointcloud.h \
    actions/onf_actionvalidateinventory.h \
    filter/onf_filterbyreturntype.h \
    filter/onf_filterremoveupperoutliers.h \
    itemdrawable/onf_lai2000data.h \
    itemdrawable/onf_lvoxsphere.h \
    metric/onf_metricquantiles.h \
    step/onf_stepaddaffiliationid.h \
    step/onf_stepaddattributevalue.h \
    step/onf_stepchangeclusterthickness02.h \
    step/onf_stepclassifyground.h \
    step/onf_stepcompare3dgridscontents.h \
    step/onf_stepcomputeattributemapfromclusters.h \
    step/onf_stepcomputecrownprojection.h \
    step/onf_stepcomputedsm.h \
    step/onf_stepcomputedtm02.h \
    step/onf_stepcomputehitgrid.h \
    step/onf_stepcomputelai2000data.h \
    step/onf_stepcomputespherevoxels.h \
    step/onf_stepconvertfloatimagetoqint32.h \
    step/onf_stepconvertscenetocluster.h \
    step/onf_stepcorrectalsprofile.h \
    step/onf_stepcreateplotmanagerfromfile.h \
    step/onf_stepcreateplotmanagergrid.h \
    step/onf_stepdetectsection07.h \
    step/onf_stepdetectverticalalignments.h \
    step/onf_stepextractdiametersfromcylinders.h \
    step/onf_stepextractlogbuffer.h \
    step/onf_stepextractplot.h \
    step/onf_stepextractplotbasedondtm.h \
    step/onf_stepextractpositionsfromdensity.h \
    step/onf_stepfilterclustersbysize.h \
    step/onf_stepfiltergroupsbygroupsnumber.h \
    step/onf_stepfilteritemsbyposition.h \
    step/onf_stepfiltermaximabyclusterpositions.h \
    step/onf_stepfilterpointsbyboolgrid.h \
    step/onf_stepfitandfiltercylindersinsections.h \
    step/onf_stepfitcirclesandfilter.h \
    step/onf_stepfitcylinderoncluster.h \
    step/onf_stephorizontalclustering3d.h \
    step/onf_stepimportsegmafilesformatching.h \
    step/onf_steploadplotareas.h \
    step/onf_steploadpositionsformatching.h \
    step/onf_steploadtreemap.h \
    step/onf_stepmanualinventory.h \
    step/onf_stepmatchitemspositions.h \
    step/onf_stepmergeclustersfrompositions02.h \
    step/onf_stepmergeendtoendsections04.h \
    step/onf_stepmergeneighboursections04.h \
    step/onf_stepmodifypositions2d.h \
    step/onf_stepreducepointsdensity.h \
    step/onf_steprefpointfromarccenter.h \
    step/onf_steprefpointfrombarycenter02.h \
    step/onf_stepsegmentcrowns.h \
    step/onf_stepsegmentcrownsfromstemclusters.h \
    step/onf_stepsegmentgaps.h \
    step/onf_stepselectbboxbyfilename.h \
    step/onf_stepselectcellsingrid3d.h \
    step/onf_stepselectcellsingrid3dbybinarypattern.h \
    step/onf_stepselectgroupsbyreferenceheight.h \
    step/onf_stepsetaffiliationidfromreference.h \
    step/onf_stepsetfootcoordinatesvertically.h \
    step/onf_stepslicepointcloud.h \
    step/onf_stepsmoothskeleton.h \
    step/onf_steptransformpointcloud.h \
    step/onf_stepvalidateinventory.h \
    tools/onf_computehitsthread.h \
    views/actions/onf_actiondefineheightlayeroptions.h \
    views/actions/onf_actionmanualinventoryattributesdialog.h \
    views/actions/onf_actionmanualinventoryoptions.h \
    views/actions/onf_actionmodifyaffiliationsoptions.h \
    views/actions/onf_actionmodifyclustersgroupsoptions02.h \
    views/actions/onf_actionmodifypositions2doptions.h \
    views/actions/onf_actionsegmentcrownsoptions.h \
    views/actions/onf_actionsegmentgapsoptions.h \
    views/actions/onf_actionselectcellsingrid3dcolonizedialog.h \
    views/actions/onf_actionselectcellsingrid3doptions.h \
    views/actions/onf_actionslicepointcloudoptions.h \
    views/actions/onf_actionvalidateinventoryattributesdialog.h \
    views/actions/onf_actionvalidateinventoryoptions.h \
    tools/onf_computeverticalprofilethread.h \
    step/onf_stepcomputeverticalprofile.h \
    step/onf_stepdetectverticalalignments02.h \
    step/onf_stepdetectverticalalignments03.h \
    step/onf_stepextractpointsinverticalcylinders.h \
    step/onf_stepdetectverticalalignments04.h \
    step/onf_stepaddtilexyareas.h \
    step/onf_stepdetectverticalalignments05.h \
    step/onf_stepdetectverticalalignments06.h \
    step/onf_stepcreateplotsfromlist.h \
    step/onf_stepextractpointsforplots.h \
    step/onf_stepcreatetiling.h \
    step/onf_stepcomputedsmonly.h \
    step/onf_stepcomputeedbhfromheightallometry.h \
    step/onf_stepdetectverticalalignments07.h \
    step/onf_stepaddlasdatatoplots.h \
    step/onf_stepfilterelementsbyxyarea.h \
    step/onf_stepfiltergridbycloud.h \
    step/onf_stepvoxelclusterization.h \
    step/onf_stepcomputecumulativeconvexhull.h \
    metric/onf_metricnbylasclass.h \
    metric/onf_metricminmaxlasfields.h \
    step/onf_stepfiltermaximabyneighbourhood.h \
    step/onf_stepcomputeboundary.h \
    step/onf_stepcreatemaximacloud.h \
    metric/onf_metricnapexmean.h \
    step/onf_stepoptimizegaussianonmaximanumber.h \
    step/onf_stepcomputetin.h \
    step/onf_stepconverttintodtm.h \
    step/onf_stepcomputehfromzandtin.h \
    step/onf_stepremoveuppernoise.h \
    actions/onf_actionaggregateitems.h \
    views/actions/onf_actionaggregateitemsoptions.h \
    views/actions/onf_actionaggregateitemsoptionsselectiondialog.h \
    step/onf_stepmergescenesbymodality.h \
    step/onf_stepmatchclouds.h \
    step/onf_stepcomputecumulativesummary.h \
    step/onf_stepcomputecumulativenrtable.h \
    step/onf_stepadjustplotposition.h \
    actions/onf_actionadjustplotposition.h \
    views/actions/onf_actionadjustplotpositionoptions.h \
    tools/onf_adjustplotpositioncylinderdrawmanager.h \
    tools/onf_colorlinearinterpolator.h \
    tools/onf_citations.h \
    step/onf_stepcomputeboundaryv2.h \
    step/onf_steppolygonfrommask.h \
    step/onf_stepcomputepointheightattribute.h \
    step/onf_stepcreatecolorcomposite.h \
    step/onf_stepcreatepointgrid.h \
    step/onf_stepcreateseedgrid.h \
    step/onf_stepsegmentfromseedgrid.h \
    step/onf_stepmodifyvoxelsegmentation.h \
    actions/onf_actionmodifyvoxelsegmentation.h \
    views/actions/onf_actionmodifyvoxelsegmentationoptions.h

SOURCES += \
    onf_pluginentry.cpp \
    onf_steppluginmanager.cpp \
    actions/onf_actiondefineheightlayer.cpp \
    actions/onf_actionmanualinventory.cpp \
    actions/onf_actionmodifyaffiliations.cpp \
    actions/onf_actionmodifyclustersgroups02.cpp \
    actions/onf_actionmodifypositions2d.cpp \
    actions/onf_actionsegmentcrowns.cpp \
    actions/onf_actionsegmentgaps.cpp \
    actions/onf_actionselectcellsingrid3d.cpp \
    actions/onf_actionslicepointcloud.cpp \
    actions/onf_actionvalidateinventory.cpp \
    filter/onf_filterbyreturntype.cpp \
    filter/onf_filterremoveupperoutliers.cpp \
    itemdrawable/onf_lai2000data.cpp \
    itemdrawable/onf_lvoxsphere.cpp \
    metric/onf_metricquantiles.cpp \
    step/onf_stepaddaffiliationid.cpp \
    step/onf_stepaddattributevalue.cpp \
    step/onf_stepchangeclusterthickness02.cpp \
    step/onf_stepclassifyground.cpp \
    step/onf_stepcompare3dgridscontents.cpp \
    step/onf_stepcomputeattributemapfromclusters.cpp \
    step/onf_stepcomputecrownprojection.cpp \
    step/onf_stepcomputedsm.cpp \
    step/onf_stepcomputedtm02.cpp \
    step/onf_stepcomputehitgrid.cpp \
    step/onf_stepcomputelai2000data.cpp \
    step/onf_stepcomputespherevoxels.cpp \
    step/onf_stepconvertfloatimagetoqint32.cpp \
    step/onf_stepconvertscenetocluster.cpp \
    step/onf_stepcorrectalsprofile.cpp \
    step/onf_stepcreateplotmanagerfromfile.cpp \
    step/onf_stepcreateplotmanagergrid.cpp \
    step/onf_stepdetectsection07.cpp \
    step/onf_stepdetectverticalalignments.cpp \
    step/onf_stepextractdiametersfromcylinders.cpp \
    step/onf_stepextractlogbuffer.cpp \
    step/onf_stepextractplot.cpp \
    step/onf_stepextractplotbasedondtm.cpp \
    step/onf_stepextractpositionsfromdensity.cpp \
    step/onf_stepfilterclustersbysize.cpp \
    step/onf_stepfiltergroupsbygroupsnumber.cpp \
    step/onf_stepfilteritemsbyposition.cpp \
    step/onf_stepfiltermaximabyclusterpositions.cpp \
    step/onf_stepfilterpointsbyboolgrid.cpp \
    step/onf_stepfitandfiltercylindersinsections.cpp \
    step/onf_stepfitcirclesandfilter.cpp \
    step/onf_stepfitcylinderoncluster.cpp \
    step/onf_stephorizontalclustering3d.cpp \
    step/onf_stepimportsegmafilesformatching.cpp \
    step/onf_steploadplotareas.cpp \
    step/onf_steploadpositionsformatching.cpp \
    step/onf_steploadtreemap.cpp \
    step/onf_stepmanualinventory.cpp \
    step/onf_stepmatchitemspositions.cpp \
    step/onf_stepmergeclustersfrompositions02.cpp \
    step/onf_stepmergeendtoendsections04.cpp \
    step/onf_stepmergeneighboursections04.cpp \
    step/onf_stepmodifypositions2d.cpp \
    step/onf_stepreducepointsdensity.cpp \
    step/onf_steprefpointfromarccenter.cpp \
    step/onf_steprefpointfrombarycenter02.cpp \
    step/onf_stepsegmentcrowns.cpp \
    step/onf_stepsegmentcrownsfromstemclusters.cpp \
    step/onf_stepsegmentgaps.cpp \
    step/onf_stepselectbboxbyfilename.cpp \
    step/onf_stepselectcellsingrid3d.cpp \
    step/onf_stepselectcellsingrid3dbybinarypattern.cpp \
    step/onf_stepselectgroupsbyreferenceheight.cpp \
    step/onf_stepsetaffiliationidfromreference.cpp \
    step/onf_stepsetfootcoordinatesvertically.cpp \
    step/onf_stepslicepointcloud.cpp \
    step/onf_stepsmoothskeleton.cpp \
    step/onf_steptransformpointcloud.cpp \
    step/onf_stepvalidateinventory.cpp \
    tools/onf_computehitsthread.cpp \
    views/actions/onf_actiondefineheightlayeroptions.cpp \
    views/actions/onf_actionmanualinventoryattributesdialog.cpp \
    views/actions/onf_actionmanualinventoryoptions.cpp \
    views/actions/onf_actionmodifyaffiliationsoptions.cpp \
    views/actions/onf_actionmodifyclustersgroupsoptions02.cpp \
    views/actions/onf_actionmodifypositions2doptions.cpp \
    views/actions/onf_actionsegmentcrownsoptions.cpp \
    views/actions/onf_actionsegmentgapsoptions.cpp \
    views/actions/onf_actionselectcellsingrid3dcolonizedialog.cpp \
    views/actions/onf_actionselectcellsingrid3doptions.cpp \
    views/actions/onf_actionslicepointcloudoptions.cpp \
    views/actions/onf_actionvalidateinventoryattributesdialog.cpp \
    views/actions/onf_actionvalidateinventoryoptions.cpp \
    tools/onf_computeverticalprofilethread.cpp \
    step/onf_stepcomputeverticalprofile.cpp \
    step/onf_stepdetectverticalalignments02.cpp \
    step/onf_stepdetectverticalalignments03.cpp \
    step/onf_stepextractpointsinverticalcylinders.cpp \
    step/onf_stepdetectverticalalignments04.cpp \
    step/onf_stepaddtilexyareas.cpp \
    step/onf_stepdetectverticalalignments05.cpp \
    step/onf_stepdetectverticalalignments06.cpp \
    step/onf_stepcreateplotsfromlist.cpp \
    step/onf_stepextractpointsforplots.cpp \
    step/onf_stepcreatetiling.cpp \
    step/onf_stepcomputedsmonly.cpp \
    step/onf_stepcomputeedbhfromheightallometry.cpp \
    step/onf_stepdetectverticalalignments07.cpp \
    step/onf_stepaddlasdatatoplots.cpp \
    step/onf_stepfilterelementsbyxyarea.cpp \
    step/onf_stepfiltergridbycloud.cpp \
    step/onf_stepvoxelclusterization.cpp \
    step/onf_stepcomputecumulativeconvexhull.cpp \
    metric/onf_metricnbylasclass.cpp \
    metric/onf_metricminmaxlasfields.cpp \
    step/onf_stepfiltermaximabyneighbourhood.cpp \
    step/onf_stepcomputeboundary.cpp \
    step/onf_stepcreatemaximacloud.cpp \
    metric/onf_metricnapexmean.cpp \
    step/onf_stepoptimizegaussianonmaximanumber.cpp \
    step/onf_stepcomputetin.cpp \
    step/onf_stepconverttintodtm.cpp \
    step/onf_stepcomputehfromzandtin.cpp \
    step/onf_stepremoveuppernoise.cpp \
    actions/onf_actionaggregateitems.cpp \
    views/actions/onf_actionaggregateitemsoptions.cpp \
    views/actions/onf_actionaggregateitemsoptionsselectiondialog.cpp \
    step/onf_stepmergescenesbymodality.cpp \
    step/onf_stepmatchclouds.cpp \
    step/onf_stepcomputecumulativesummary.cpp \
    step/onf_stepcomputecumulativenrtable.cpp \
    step/onf_stepadjustplotposition.cpp \
    actions/onf_actionadjustplotposition.cpp \
    views/actions/onf_actionadjustplotpositionoptions.cpp \
    tools/onf_adjustplotpositioncylinderdrawmanager.cpp \
    tools/onf_colorlinearinterpolator.cpp \
    tools/onf_citations.cpp \
    step/onf_stepcomputeboundaryv2.cpp \
    step/onf_steppolygonfrommask.cpp \
    step/onf_stepcomputepointheightattribute.cpp \
    step/onf_stepcreatecolorcomposite.cpp \
    step/onf_stepcreatepointgrid.cpp \
    step/onf_stepcreateseedgrid.cpp \
    step/onf_stepsegmentfromseedgrid.cpp \
    step/onf_stepmodifyvoxelsegmentation.cpp \
    actions/onf_actionmodifyvoxelsegmentation.cpp \
    views/actions/onf_actionmodifyvoxelsegmentationoptions.cpp

TRANSLATIONS += languages/pluginonf_en.ts \
                languages/pluginonf_fr.ts

INCLUDEPATH += .

FORMS += \
    views/actions/onf_actiondefineheightlayeroptions.ui \
    views/actions/onf_actionmanualinventoryattributesdialog.ui \
    views/actions/onf_actionmanualinventoryoptions.ui \
    views/actions/onf_actionmodifyaffiliationsoptions.ui \
    views/actions/onf_actionmodifyclustersgroupsoptions.ui \
    views/actions/onf_actionmodifyclustersgroupsoptions02.ui \
    views/actions/onf_actionmodifypositions2doptions.ui \
    views/actions/onf_actionsegmentcrownsoptions.ui \
    views/actions/onf_actionsegmentgapsoptions.ui \
    views/actions/onf_actionselectcellsingrid3dcolonizedialog.ui \
    views/actions/onf_actionselectcellsingrid3doptions.ui \
    views/actions/onf_actionslicepointcloudoptions.ui \
    views/actions/onf_actionvalidateinventoryattributesdialog.ui \
    views/actions/onf_actionvalidateinventoryoptions.ui \
    views/actions/onf_actionaggregateitemsoptions.ui \
    views/actions/onf_actionaggregateitemsoptionsselectiondialog.ui \
    views/actions/onf_actionadjustplotpositionoptions.ui \
    views/actions/onf_actionmodifyvoxelsegmentationoptions.ui

RESOURCES += \
    resource.qrc











