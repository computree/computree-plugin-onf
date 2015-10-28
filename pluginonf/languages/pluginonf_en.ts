<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="en_US">
<context>
    <name>ONF_StepChangeClusterThickness02</name>
    <message>
        <location filename="../step/onf_stepchangeclusterthickness02.cpp" line="71"/>
        <source>Création de clusters horizontaux / billon</source>
        <translation>Creation of horizontal clusters by log</translation>
    </message>
    <message>
        <location filename="../step/onf_stepchangeclusterthickness02.cpp" line="76"/>
        <source>No detailled description for this step</source>
        <translation>Pas de description détaillée pour cette étape</translation>
    </message>
    <message>
        <location filename="../step/onf_stepchangeclusterthickness02.cpp" line="89"/>
        <source>Billons / Clusters</source>
        <translation>Logs / Clusters</translation>
    </message>
    <message>
        <location filename="../step/onf_stepchangeclusterthickness02.cpp" line="91"/>
        <source>Billon (Grp)</source>
        <translation>Log (Grp)</translation>
    </message>
    <message>
        <location filename="../step/onf_stepchangeclusterthickness02.cpp" line="92"/>
        <location filename="../step/onf_stepchangeclusterthickness02.cpp" line="107"/>
        <source>Cluster (Grp)</source>
        <translation>Cluster (Grp)</translation>
    </message>
    <message>
        <location filename="../step/onf_stepchangeclusterthickness02.cpp" line="93"/>
        <location filename="../step/onf_stepchangeclusterthickness02.cpp" line="108"/>
        <source>Points</source>
        <translation>Points</translation>
    </message>
    <message>
        <location filename="../step/onf_stepchangeclusterthickness02.cpp" line="100"/>
        <source>Epaisseur en Z  :</source>
        <translation>Z-thickness:</translation>
    </message>
</context>
<context>
    <name>ONF_StepComputeLAI2000Data</name>
    <message>
        <location filename="../step/onf_stepcomputelai2000data.cpp" line="40"/>
        <source>Calcul de données LAI-2000</source>
        <translation>Computing of LAI 2000 data</translation>
    </message>
    <message>
        <location filename="../step/onf_stepcomputelai2000data.cpp" line="45"/>
        <source>No detailled description for this step</source>
        <translation>Pas de description détaillée pour cette étape</translation>
    </message>
    <message>
        <location filename="../step/onf_stepcomputelai2000data.cpp" line="59"/>
        <location filename="../step/onf_stepcomputelai2000data.cpp" line="62"/>
        <source>Scène</source>
        <translation>Scene</translation>
    </message>
    <message>
        <location filename="../step/onf_stepcomputelai2000data.cpp" line="68"/>
        <location filename="../step/onf_stepcomputelai2000data.cpp" line="71"/>
        <source>Données LAI2000</source>
        <translation>LAI2000 data</translation>
    </message>
    <message>
        <location filename="../step/onf_stepcomputelai2000data.cpp" line="61"/>
        <location filename="../step/onf_stepcomputelai2000data.cpp" line="70"/>
        <source>Groupe</source>
        <translation>Group</translation>
    </message>
</context>
<context>
    <name>ONF_StepComputeSphereVoxels</name>
    <message>
        <location filename="../step/onf_stepcomputespherevoxels.cpp" line="48"/>
        <source>Calul d&apos;une densité de points corrigée / sphère</source>
        <translation>Computing of corrected points density by sphere</translation>
    </message>
    <message>
        <location filename="../step/onf_stepcomputespherevoxels.cpp" line="53"/>
        <source>No detailled description for this step</source>
        <translation>Pas de description détaillée pour cette étape</translation>
    </message>
    <message>
        <location filename="../step/onf_stepcomputespherevoxels.cpp" line="67"/>
        <location filename="../step/onf_stepcomputespherevoxels.cpp" line="70"/>
        <source>Scène</source>
        <translation>Scene</translation>
    </message>
    <message>
        <location filename="../step/onf_stepcomputespherevoxels.cpp" line="79"/>
        <source>Sphère</source>
        <translation>Sphere</translation>
    </message>
    <message>
        <location filename="../step/onf_stepcomputespherevoxels.cpp" line="76"/>
        <source>Sphères</source>
        <translation>Spheres</translation>
    </message>
    <message>
        <location filename="../step/onf_stepcomputespherevoxels.cpp" line="69"/>
        <location filename="../step/onf_stepcomputespherevoxels.cpp" line="78"/>
        <source>Groupe</source>
        <translation>Group</translation>
    </message>
</context>
<context>
    <name>ONF_StepConvertSceneToCluster</name>
    <message>
        <location filename="../step/onf_stepconvertscenetocluster.cpp" line="31"/>
        <source>Conversion d&apos;une scène en cluster ordonné</source>
        <translation>Conversion of a scene in ordered cluster</translation>
    </message>
    <message>
        <location filename="../step/onf_stepconvertscenetocluster.cpp" line="37"/>
        <source>No detailled description for this step</source>
        <translation>Pas de description détaillée pour cette étape</translation>
    </message>
</context>
<context>
    <name>ONF_StepDetectSection06</name>
    <message>
        <source>Aggrégation verticale de clusters en billon</source>
        <translation type="vanished">Vertical merging of clusters in logs</translation>
    </message>
    <message>
        <location filename="../step/onf_stepdetectsection06.cpp" line="96"/>
        <source>Groupe</source>
        <translation>Group</translation>
    </message>
    <message>
        <location filename="../step/onf_stepdetectsection06.cpp" line="75"/>
        <source>Cette étape prend en entrée des couches horizontales (layers) contenant des clusters.&lt;br&gt;Ce type de structure peut par exemple être produite par l&apos;étape &lt;em&gt;ONF_StepHorizontalClustering&lt;/em&gt;.&lt;br&gt;Les clusters adjacents verticalement sont regroupés en billons (groupes). Pour ce faire :&lt;ul&gt;&lt;li&gt; Les clusters dont la &lt;b&gt;distance verticale&lt;/b&gt; les séparant est inférieure au seuil choisi sont comparés deux à deux.&lt;/li&gt;&lt;li&gt;Si leurs boites englobantes s&apos;intersectent dans le plan XY, les clusters sont regroupés dans la même billon.&lt;/li&gt;&lt;/ul&gt;</source>
        <translation>This step need  horizontal layers containing points clusters as input. &lt;br&gt; This sort of data could for example be produced by the &lt;em&gt;ONF_StepHorizontalClustering&lt;/em&gt;.&lt;br&gt;The clusters which are vertically paralels are gathered in logs (groups). To do it:&lt;ul&gt;&lt;li&gt;Clusters which have a &lt;b&gt;vertical distance&lt;/b&gt; between then lesser than the specified threshold are compared two by two.&lt;/li&gt;&lt;li&gt;If their bounding boxes are intersecting in XY plane, clusters are bring together in the same log.&lt;/li&gt;&lt;/ul&gt;</translation>
    </message>
    <message>
        <location filename="../step/onf_stepdetectsection06.cpp" line="70"/>
        <source>Aggrégation verticale de clusters en billon (Ancienne Version)</source>
        <translation>Vertical merging of clusters in logs (Old Version)</translation>
    </message>
    <message>
        <location filename="../step/onf_stepdetectsection06.cpp" line="94"/>
        <source>Clusters</source>
        <translation>Clusters</translation>
    </message>
    <message>
        <location filename="../step/onf_stepdetectsection06.cpp" line="97"/>
        <source>Points</source>
        <translation>Points</translation>
    </message>
    <message>
        <location filename="../step/onf_stepdetectsection06.cpp" line="106"/>
        <source>Distance en z (en + et en -) maximum entre deux groupes de points à comparer</source>
        <translation>Maximum Z distance (+ or -) between 2 points clusters to compare</translation>
    </message>
    <message>
        <location filename="../step/onf_stepdetectsection06.cpp" line="145"/>
        <source>Billon</source>
        <translation>Log</translation>
    </message>
    <message>
        <location filename="../step/onf_stepdetectsection06.cpp" line="149"/>
        <source>Billons</source>
        <translation>Logs</translation>
    </message>
</context>
<context>
    <name>ONF_StepDetectSection07</name>
    <message>
        <location filename="../step/onf_stepdetectsection07.cpp" line="73"/>
        <source>Aggrégation verticale de clusters en billon</source>
        <translation>Vertical merging of clusters in logs</translation>
    </message>
    <message>
        <location filename="../step/onf_stepdetectsection07.cpp" line="78"/>
        <source>Cette étape prend en entrée des couches horizontales (layers) contenant des clusters.&lt;br&gt;Ce type de structure peut par exemple être produite par l&apos;étape &lt;em&gt;ONF_StepHorizontalClustering&lt;/em&gt;.&lt;br&gt;Les clusters adjacents verticalement sont regroupés en billons (groupes). Pour ce faire :&lt;ul&gt;&lt;li&gt; Les clusters dont la &lt;b&gt;distance verticale&lt;/b&gt; les séparant est inférieure au seuil choisi sont comparés deux à deux.&lt;/li&gt;&lt;li&gt;Si leurs boites englobantes s&apos;intersectent dans le plan XY, les clusters sont regroupés dans la même billon.&lt;/li&gt;&lt;/ul&gt;N.B. : Les clusters ayant la plus grande boite englobante XY sont prioritaires.</source>
        <translation>This step need  horizontal layers containing points clusters as input. &lt;br&gt; This sort of data could for example be produced by the &lt;em&gt;ONF_StepHorizontalClustering&lt;/em&gt;.&lt;br&gt;The clusters which are vertically paralels are gathered in logs (groups). To do it:&lt;ul&gt;&lt;li&gt;Clusters which have a &lt;b&gt;vertical distance&lt;/b&gt; between then lesser than the specified threshold are compared two by two.&lt;/li&gt;&lt;li&gt;If their bounding boxes are intersecting in XY plane, clusters are bring together in the same log.&lt;/li&gt;&lt;/ul&gt;N.B.: Clusters with the higher bounding box have the priority.</translation>
    </message>
    <message>
        <location filename="../step/onf_stepdetectsection07.cpp" line="98"/>
        <source>Clusters</source>
        <translation>Clusters</translation>
    </message>
    <message>
        <location filename="../step/onf_stepdetectsection07.cpp" line="100"/>
        <source>Niveau Z (Grp)</source>
        <translation>Z-Level (Grp)</translation>
    </message>
    <message>
        <location filename="../step/onf_stepdetectsection07.cpp" line="101"/>
        <source>Cluster (Grp)</source>
        <translation>Cluster (Grp)</translation>
    </message>
    <message>
        <location filename="../step/onf_stepdetectsection07.cpp" line="102"/>
        <source>Points</source>
        <translation>Points</translation>
    </message>
    <message>
        <location filename="../step/onf_stepdetectsection07.cpp" line="111"/>
        <source>Distance en z (en + et en -) maximum entre deux groupes de points à comparer</source>
        <translation>Maximum Z distance (+ or -) between 2 points clusters to compare</translation>
    </message>
    <message>
        <location filename="../step/onf_stepdetectsection07.cpp" line="150"/>
        <source>Billon</source>
        <translation>Log</translation>
    </message>
    <message>
        <location filename="../step/onf_stepdetectsection07.cpp" line="154"/>
        <source>Billons</source>
        <translation>Logs</translation>
    </message>
</context>
<context>
    <name>ONF_StepExtractDiametersFromCylinders</name>
    <message>
        <location filename="../step/onf_stepextractdiametersfromcylinders.cpp" line="69"/>
        <source>Calcul d&apos;un diamètre moyen des cylindres / billon</source>
        <translation>Computing of cylinders mean diameter by log</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractdiametersfromcylinders.cpp" line="74"/>
        <source>No detailled description for this step</source>
        <translation>Pas de description détaillée pour cette étape</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractdiametersfromcylinders.cpp" line="87"/>
        <source>Billons contenant des cylindres</source>
        <translation>Logs containing cylinders</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractdiametersfromcylinders.cpp" line="89"/>
        <source>Billon (Grp)</source>
        <translation>Log (Grp)</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractdiametersfromcylinders.cpp" line="90"/>
        <source>Cordonnée MNT</source>
        <translation>DTM coordinate</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractdiametersfromcylinders.cpp" line="91"/>
        <source>Groupe</source>
        <translation>Group</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractdiametersfromcylinders.cpp" line="92"/>
        <source>Cylindre</source>
        <translation>Cylinder</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractdiametersfromcylinders.cpp" line="99"/>
        <source>Hauteur de référence : </source>
        <translation>Reference height:</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractdiametersfromcylinders.cpp" line="100"/>
        <source>Hauteur minimale d&apos;évaluation :</source>
        <translation>Bottom height for evaluation:</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractdiametersfromcylinders.cpp" line="101"/>
        <source>Hauteur maximale d&apos;évaluation : </source>
        <translation>Top height for evaluation:</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractdiametersfromcylinders.cpp" line="102"/>
        <source>Décroissance métrique maximale : </source>
        <translation>Maximal taper:</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractdiametersfromcylinders.cpp" line="103"/>
        <source>Nombre de cylindres minimum pour ajuster un cercle : </source>
        <translation>Minimum number of cylinder to adjust a circle:</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractdiametersfromcylinders.cpp" line="109"/>
        <source>Diamètre à 1.30m</source>
        <translation>Diameter at 1.30 m</translation>
    </message>
</context>
<context>
    <name>ONF_StepExtractPlot</name>
    <message>
        <location filename="../step/onf_stepextractplot.cpp" line="71"/>
        <source>Extraction d&apos;une placette</source>
        <translation>Extraction of plot</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplot.cpp" line="76"/>
        <source>Cette étape permet d&apos;extraire les points de la scène d&apos;entrée contenus dans une placette circulaire.&lt;br&gt;On définit dans les paramètres son &lt;b&gt;centre (X,Y)&lt;/b&gt;, son &lt;b&gt;rayon&lt;/b&gt; (maximal), le &lt;b&gt;niveau Z minimum&lt;/b&gt; et le &lt;b&gt;niveau Z maximum&lt;/b&gt;.&lt;br&gt;Si on définit un &lt;b&gt;rayon de début de placette&lt;/b&gt;, cela permet d&apos;obtenir une placette annulaire.&lt;br&gt;On peut également définir un &lt;b&gt;azimut de début&lt;/b&gt; et un &lt;b&gt;azimut de fin&lt;/b&gt;, pour obtenir un secteur.</source>
        <translation>This step extracts a circular plot in the input points scene.&lt;br&gt;It defines the following parameters: &lt;b&gt;plot center (X,Y)&lt;/b&gt;, &lt;b&gt;plot radius&lt;/b&gt; (maximum), the &lt;b&gt;minimum Z level&lt;/b&gt; and the &lt;b&gt;maximum Z level&lt;/b&gt;.&lt;br&gt;The following optional parameters are also available: &lt;b&gt;beginning radius of the plot&lt;/b&gt; allowing to obtain an annulus, a &lt;b&gt;beginning azimut&lt;/b&gt; and an &lt;b&gt;ending azimut&lt;/b&gt; giving a sector. </translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplot.cpp" line="92"/>
        <source>Scène(s)</source>
        <translation>Scene(s)</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplot.cpp" line="96"/>
        <source>Scène</source>
        <translation>Scene</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplot.cpp" line="102"/>
        <source>Scène(s) extraites</source>
        <translation>Exctracted scene(s)</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplot.cpp" line="105"/>
        <source>Scène extraite</source>
        <translation>Extracted scene</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplot.cpp" line="112"/>
        <source>Coordonnée X du centre de la placette :</source>
        <translation>X coordinate of the plot center:</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplot.cpp" line="113"/>
        <source>Coordonnée Y du centre de la placette :</source>
        <translation>Y coordinate of the plot center:</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplot.cpp" line="114"/>
        <source>Rayon de début de la placette :</source>
        <translation>Beginning radius of plot:</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplot.cpp" line="115"/>
        <source>Rayon de la placette (maximum) :</source>
        <translation>Ending radius of plot:</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplot.cpp" line="116"/>
        <source>Azimut début (Nord = axe Y) :</source>
        <translation>Beginning azimut of plot (North = Y axis):</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplot.cpp" line="116"/>
        <location filename="../step/onf_stepextractplot.cpp" line="117"/>
        <source>Grades</source>
        <translation>Grades</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplot.cpp" line="117"/>
        <source>Azimut fin (Nord = axe Y) :</source>
        <translation>Ending azimut of plot (North = Y axis):</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplot.cpp" line="118"/>
        <source>Niveau Z minimum :</source>
        <translation>Minimum Z for plot:</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplot.cpp" line="119"/>
        <source>Niveau Z maximum :</source>
        <translation>Maximum Z for plot:</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplot.cpp" line="135"/>
        <source>La scène d&apos;entrée comporte %1 points.</source>
        <translation>The input scene contains %1 points.</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplot.cpp" line="237"/>
        <source>La scène extraite comporte %1 points.</source>
        <translation>The extracted scene contains %1 points.</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplot.cpp" line="240"/>
        <source>Aucun point n&apos;est dans l&apos;emprise choisie</source>
        <translation>Not point in selected plot</translation>
    </message>
</context>
<context>
    <name>ONF_StepExtractPlotBasedOnDTM</name>
    <message>
        <location filename="../step/onf_stepextractplotbasedondtm.cpp" line="76"/>
        <source>Extraction d&apos;une placette // MNT (cpy)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplotbasedondtm.cpp" line="81"/>
        <source>Cette étape permet d&apos;extraire les points de la scène d&apos;entrée contenus dans une placette circulaire.&lt;br&gt;On définit dans les paramètres son &lt;b&gt;centre (X,Y)&lt;/b&gt;, son &lt;b&gt;rayon&lt;/b&gt; (maximal), le &lt;b&gt;niveau Z minimum&lt;/b&gt; et le &lt;b&gt;niveau Z maximum&lt;/b&gt;.&lt;br&gt;Si on définit un &lt;b&gt;rayon de début de placette&lt;/b&gt;, cela permet d&apos;obtenir une placette annulaire.&lt;br&gt;On peut également définir un &lt;b&gt;azimut de début&lt;/b&gt; et un &lt;b&gt;azimut de fin&lt;/b&gt;, pour obtenir un secteur.&lt;br&gt;Cette étape fonctionne comme &lt;em&gt;ONF_StepExtractPlot&lt;/em&gt;, mais les niveaux Z sont spécifiés sous forme de hauteurs par rapport au MNT de référence choisi.</source>
        <translation type="unfinished">This step extracts a circular plot in the input points scene.&lt;br&gt;It defines the following parameters: &lt;b&gt;plot center (X,Y)&lt;/b&gt;, &lt;b&gt;plot radius&lt;/b&gt; (maximum), the &lt;b&gt;minimum Z level&lt;/b&gt; and the &lt;b&gt;maximum Z level&lt;/b&gt;.&lt;br&gt;The following optional parameters are also available: &lt;b&gt;beginning radius of the plot&lt;/b&gt; allowing to obtain an annulus, a &lt;b&gt;beginning azimut&lt;/b&gt; and an &lt;b&gt;ending azimut&lt;/b&gt; giving a sector.&lt;br&gt;This step work like &lt;em&gt;ONF_StepExtractPlot&lt;/em&gt;, but Z levels are specified as an height from choosen reference DTM. </translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplotbasedondtm.cpp" line="98"/>
        <source>MNT (Raster)</source>
        <translation type="unfinished">DTM (raster)</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplotbasedondtm.cpp" line="101"/>
        <source>Modèle Numérique de Terrain</source>
        <translation type="unfinished">Digital Terrain Model</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplotbasedondtm.cpp" line="103"/>
        <source>Scène(s)</source>
        <translation type="unfinished">Scene(s)</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplotbasedondtm.cpp" line="106"/>
        <source>Scène</source>
        <translation type="unfinished">Scene</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplotbasedondtm.cpp" line="113"/>
        <source>Coordonnee X du centre de la placette :</source>
        <translation type="unfinished">X coordinate of the plot center:</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplotbasedondtm.cpp" line="114"/>
        <source>Coordonnee Y du centre de la placette :</source>
        <translation type="unfinished">Y coordinate of the plot center:</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplotbasedondtm.cpp" line="115"/>
        <source>Rayon de debut de la placette</source>
        <translation type="unfinished">Beginning radius of plot:</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplotbasedondtm.cpp" line="116"/>
        <source>Rayon de la placette :</source>
        <translation type="unfinished">Ending radius of plot:</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplotbasedondtm.cpp" line="117"/>
        <source>Azimut debut (Nord = axe Y) :</source>
        <translation type="unfinished">Beginning azimut of plot (North = Y axis):</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplotbasedondtm.cpp" line="118"/>
        <source>Azimut fin (Nord = axe Y) :</source>
        <translation type="unfinished">Ending azimut of plot (North = Y axis):</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplotbasedondtm.cpp" line="119"/>
        <source>Z minimum :</source>
        <translation type="unfinished">Minimum Z for plot:</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplotbasedondtm.cpp" line="120"/>
        <source>Z maximum :</source>
        <translation type="unfinished">Maximum Z for plot:</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplotbasedondtm.cpp" line="128"/>
        <source>Scène extraite</source>
        <translation type="unfinished">Extracted scene</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplotbasedondtm.cpp" line="155"/>
        <source>La scène d&apos;entrée comporte %1 points.</source>
        <translation type="unfinished">The input scene contains %1 points.</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplotbasedondtm.cpp" line="259"/>
        <source>La scène extraite comporte %1 points.</source>
        <translation type="unfinished">The extracted scene contains %1 points.</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplotbasedondtm.cpp" line="263"/>
        <source>Aucun point n&apos;est dans l&apos;emprise choisie</source>
        <translation type="unfinished">Not point in selected plot</translation>
    </message>
</context>
<context>
    <name>ONF_StepExtractPlotBasedOnMNT02</name>
    <message>
        <source>Extraction d&apos;une&apos; placette // MNT</source>
        <translation type="vanished">Extraction of plot // DTM</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplotbasedonmnt02.cpp" line="80"/>
        <source>Extraction d&apos;une&apos; placette // MNT (ancienne version)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplotbasedonmnt02.cpp" line="85"/>
        <source>Cette étape permet d&apos;extraire les points de la scène d&apos;entrée contenus dans une placette circulaire.&lt;br&gt;On définit dans les paramètres son &lt;b&gt;centre (X,Y)&lt;/b&gt;, son &lt;b&gt;rayon&lt;/b&gt; (maximal), le &lt;b&gt;niveau Z minimum&lt;/b&gt; et le &lt;b&gt;niveau Z maximum&lt;/b&gt;.&lt;br&gt;Si on définit un &lt;b&gt;rayon de début de placette&lt;/b&gt;, cela permet d&apos;obtenir une placette annulaire.&lt;br&gt;On peut également définir un &lt;b&gt;azimut de début&lt;/b&gt; et un &lt;b&gt;azimut de fin&lt;/b&gt;, pour obtenir un secteur.&lt;br&gt;Cette étape fonctionne comme &lt;em&gt;ONF_StepExtractPlot&lt;/em&gt;, mais les niveaux Z sont spécifiés sous forme de hauteurs par rapport au MNT de référence choisi.</source>
        <translation>This step extracts a circular plot in the input points scene.&lt;br&gt;It defines the following parameters: &lt;b&gt;plot center (X,Y)&lt;/b&gt;, &lt;b&gt;plot radius&lt;/b&gt; (maximum), the &lt;b&gt;minimum Z level&lt;/b&gt; and the &lt;b&gt;maximum Z level&lt;/b&gt;.&lt;br&gt;The following optional parameters are also available: &lt;b&gt;beginning radius of the plot&lt;/b&gt; allowing to obtain an annulus, a &lt;b&gt;beginning azimut&lt;/b&gt; and an &lt;b&gt;ending azimut&lt;/b&gt; giving a sector.&lt;br&gt;This step work like &lt;em&gt;ONF_StepExtractPlot&lt;/em&gt;, but Z levels are specified as an height from choosen reference DTM. </translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplotbasedonmnt02.cpp" line="102"/>
        <source>MNT (Raster)</source>
        <translation>DTM (raster)</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplotbasedonmnt02.cpp" line="105"/>
        <source>Modèle Numérique de Terrain</source>
        <translation>Digital Terrain Model</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplotbasedonmnt02.cpp" line="107"/>
        <source>Scène(s)</source>
        <translation>Scene(s)</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplotbasedonmnt02.cpp" line="110"/>
        <location filename="../step/onf_stepextractplotbasedonmnt02.cpp" line="133"/>
        <source>Scène</source>
        <translation>Scene</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplotbasedonmnt02.cpp" line="117"/>
        <source>Coordonnee X du centre de la placette :</source>
        <translation>X coordinate of the plot center:</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplotbasedonmnt02.cpp" line="118"/>
        <source>Coordonnee Y du centre de la placette :</source>
        <translation>Y coordinate of the plot center:</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplotbasedonmnt02.cpp" line="119"/>
        <source>Rayon de debut de la placette</source>
        <translation>Beginning radius of plot:</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplotbasedonmnt02.cpp" line="120"/>
        <source>Rayon de la placette :</source>
        <translation>Ending radius of plot:</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplotbasedonmnt02.cpp" line="121"/>
        <source>Azimut debut (Nord = axe Y) :</source>
        <translation>Beginning azimut of plot (North = Y axis):</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplotbasedonmnt02.cpp" line="122"/>
        <source>Azimut fin (Nord = axe Y) :</source>
        <translation>Ending azimut of plot (North = Y axis):</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplotbasedonmnt02.cpp" line="123"/>
        <source>Z minimum :</source>
        <translation>Minimum Z for plot:</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplotbasedonmnt02.cpp" line="124"/>
        <source>Z maximum :</source>
        <translation>Maximum Z for plot:</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplotbasedonmnt02.cpp" line="131"/>
        <source>Scène(s) extraites</source>
        <translation>Exctracted scene(s)</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplotbasedonmnt02.cpp" line="162"/>
        <source>La scène d&apos;entrée comporte %1 points.</source>
        <translation>The input scene contains %1 points.</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplotbasedonmnt02.cpp" line="272"/>
        <source>La scène extraite comporte %1 points.</source>
        <translation>The extracted scene contains %1 points.</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractplotbasedonmnt02.cpp" line="276"/>
        <source>Aucun point n&apos;est dans l&apos;emprise choisie</source>
        <translation>Not point in selected plot</translation>
    </message>
</context>
<context>
    <name>ONF_StepExtractSoil03</name>
    <message>
        <location filename="../step/onf_stepextractsoil03.cpp" line="94"/>
        <source>Séparation sol / végétation -&gt; MNT</source>
        <translation>Soil / vegetation segmentation -&gt; DTM</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractsoil03.cpp" line="99"/>
        <source>Cette étape permet de séparer les points Sol et Végétation, et de générer :&lt;ul&gt;&lt;li&gt;Le Modèle Numérique de Terrain (MNT)&lt;/li&gt;&lt;li&gt;Le Modèle Numérique de Surface (MNS)&lt;/li&gt;&lt;li&gt;Le Modèle Numérique de Hauteur (MNH)&lt;/li&gt;&lt;/ul&gt;&lt;br&gt;Etapes de l&apos;extraction du sol et de la création du MNT :&lt;ul&gt;&lt;li&gt;Une grille Zmin est créée à la &lt;b&gt;résolution&lt;/b&gt; spécifiée&lt;/li&gt;&lt;li&gt;La densité de points situés entre Zmin et (Zmin + &lt;b&gt;épaisseur du sol&lt;/b&gt;) est calculée pour chaque case&lt;/li&gt;&lt;li&gt;La valeur NA est affectée à toute case dont la densité est inférieure à la &lt;b&gt;densité minimum&lt;/b&gt;&lt;/li&gt;&lt;li&gt;Un test de cohérence des Zmin restants est réalisé pour chaque case sur le &lt;b&gt;voisinage&lt;/b&gt; spécifié (nombre de cases). La valeur NA est affectée aux cases incohérentes&lt;/li&gt;&lt;li&gt;Si l&apos; &lt;b&gt;interpolation&lt;/b&gt; est activée, les valeur NA sont remplacées par une moyenne des voisins natuels dans la grille (triangulation de Delaunay en 2D, fournie en sortie)&lt;/li&gt;&lt;li&gt;Si le &lt;b&gt;lissage&lt;/b&gt; est activé, chaque cellule est tranformée en la moyenne du k-voisinnage, avec k = &lt;b&gt;voisinnage de lissage&lt;/b&gt;&lt;/li&gt;&lt;/ul&gt;&lt;br&gt;Le MNT est la grille résultante (interpolée et/ou lissée selon les options cochées).&lt;br&gt;Le MNS est simplement une grille Zmax de la même &lt;b&gt;résolution&lt;/b&gt;.&lt;br&gt;Le MNH est la soutraction MNS-MNT.&lt;br&gt;Les points Sol sont tous les points dont Z &amp;lt; (hauteur MNT + &lt;b&gt;épaisseur du sol&lt;/b&gt;).&lt;br&gt;Les points Végétation sont tous les points non classés sol.</source>
        <translation>This step allows the separation of Soil and Vegetation points and generate:
&lt;ul&gt;
&lt;li&gt;Digital Terrain Model (DTM)&lt;/li&gt;
&lt;li&gt;Digital Surface Model (DSM)&lt;/li&gt;
&lt;li&gt;Digital Height Model (DHM)&lt;/li&gt;
&lt;/ul&gt;
&lt;br&gt;Steps for soil extraction and DTM creation:
&lt;ul&gt;
&lt;li&gt;Zmin grid is created with the specified &lt;b&gt;resolution&lt;/b&gt;&lt;/li&gt;
&lt;li&gt;the density of points between Zmin and (Zmin + &lt;b&gt;soil depth&lt;/b&gt;) is calculated for each cell&lt;/li&gt;
&lt;li&gt;the value NA is assigned to each cell whose density is less than the &lt;b&gt;minimum density&lt;/b&gt;&lt;/li&gt;
&lt;li&gt;a consistency check of remaining Zmin is performed for each cell on the specified &lt;b&gt;neighborhood&lt;/b&gt; (number of cells). The NA value is assigned to inconsistent cells&lt;/li&gt;
&lt;li&gt;if &lt;b&gt;interpolation&lt;/b&gt; is enabled, the NA value is replaced by an average of natural neighbors in the grid (2D Delaunay triangulation, given as output)&lt;/li&gt;
&lt;li&gt;if &lt;b&gt;smoothing&lt;/b&gt; is enabled, each cellvalue  is remplaced by the average k-Neighborhood, k = &lt;b&gt;smoothing neighborhood&lt;/b&gt;&lt;/li&gt;
&lt;/ul&gt;
&lt;br&gt;
the DTM is the resulting grid (interpolated and / or smoothed depending on the checked options).&lt;br&gt;
DSM is simply a grid containing Zmax for the same &lt;b&gt;resolution&lt;/b&gt;.&lt;br&gt;
DHM is the soutraction: DSM-DTM&lt;br&gt;
Soil points are all points for which Z &amp;lt; (DTM height + &lt;b&gt;soil depth&lt;/b&gt;).&lt;br&gt;
Vegetation points are all points not classified as soil.</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractsoil03.cpp" line="131"/>
        <source>Scène(s)</source>
        <translation>Scene(s)</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractsoil03.cpp" line="135"/>
        <source>Scène</source>
        <translation>Scene</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractsoil03.cpp" line="142"/>
        <source>Résolution de la grille :</source>
        <translation>Grid resolution:</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractsoil03.cpp" line="143"/>
        <source>Epaisseur du sol :</source>
        <translation>Soil thickness:</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractsoil03.cpp" line="144"/>
        <source>Densité minimum :</source>
        <translation>Minimum density:</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractsoil03.cpp" line="145"/>
        <source>Voisinage (points isolés) :</source>
        <translation>Neighbourhood (isolated points):</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractsoil03.cpp" line="146"/>
        <source>Interpolation</source>
        <translation>Interpolation</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractsoil03.cpp" line="147"/>
        <source>Lissage</source>
        <translation>Smoothing</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractsoil03.cpp" line="148"/>
        <source>Voisinage de lissage :</source>
        <translation>Smoothing neighbourhood:</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractsoil03.cpp" line="155"/>
        <source>Points végétation</source>
        <translation>Vegetation points</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractsoil03.cpp" line="157"/>
        <source>Scène végétation</source>
        <translation>Vegetation scene</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractsoil03.cpp" line="159"/>
        <source>Points sol</source>
        <translation>Soil points</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractsoil03.cpp" line="161"/>
        <source>Scène sol</source>
        <translation>Soil scene</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractsoil03.cpp" line="163"/>
        <location filename="../step/onf_stepextractsoil03.cpp" line="165"/>
        <source>Triangulation 2D</source>
        <translation>2D triangulation</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractsoil03.cpp" line="167"/>
        <source>Modèle Numérique de terrain</source>
        <translation>Digital Terrain Model</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractsoil03.cpp" line="169"/>
        <source>MNT (Raster)</source>
        <translation>DTM (raster)</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractsoil03.cpp" line="171"/>
        <source>Modèle Numérique de Surface</source>
        <translation>Digital Surface Model</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractsoil03.cpp" line="173"/>
        <source>MNS (Raster)</source>
        <translation>DSM (raster)</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractsoil03.cpp" line="175"/>
        <source>Modèle Numérique de Hauteur</source>
        <translation>Digital Height Model</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractsoil03.cpp" line="177"/>
        <source>MNH (Raster)</source>
        <translation>DHM (raster)</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractsoil03.cpp" line="179"/>
        <source>Densité de points sol</source>
        <translation>Soil points density</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractsoil03.cpp" line="181"/>
        <source>Densité pts sol (Raster)</source>
        <translation>Soil point density (raster)</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractsoil03.cpp" line="227"/>
        <source>La scène d&apos;entrée %2 comporte %1 points.</source>
        <translation>The %2 input scene contains %1 points.</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractsoil03.cpp" line="273"/>
        <source>Grille Zmin et MNS créés</source>
        <translation>Zmin grid and DSM created</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractsoil03.cpp" line="297"/>
        <source>Filtrage sur la densité terminé</source>
        <translation>Filtering on density achieved</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractsoil03.cpp" line="343"/>
        <source>Test de cohérence de voisinnage terminé</source>
        <translation>Neighbourhood consistency test achieved</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractsoil03.cpp" line="366"/>
        <source>Triangulation des cases conservées terminée</source>
        <translation>Kept cells triangulation achieved</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractsoil03.cpp" line="425"/>
        <source>Interpolation du MNT terminée</source>
        <translation>DTM interpolation achieved</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractsoil03.cpp" line="459"/>
        <source>Lissage du MNT terminé</source>
        <translation>DTM smoothing achieved</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractsoil03.cpp" line="483"/>
        <source>Création du MNH terminée</source>
        <translation>DHM creation achieved</translation>
    </message>
    <message>
        <location filename="../step/onf_stepextractsoil03.cpp" line="559"/>
        <source>Scène %3 : Création des scènes sol (%1 points) et végétation (%2 points) terminée</source>
        <translation>Scene %3: soil (%1 points) and vegetation (%2 points) scenes creation achieved</translation>
    </message>
</context>
<context>
    <name>ONF_StepFilterClustersBySize</name>
    <message>
        <location filename="../step/onf_stepfilterclustersbysize.cpp" line="53"/>
        <source>Filtrage de clusters / nb. de points</source>
        <translation>Filtering of clusters by points number</translation>
    </message>
    <message>
        <location filename="../step/onf_stepfilterclustersbysize.cpp" line="58"/>
        <source>Cette étape filtre des clusters (tout item contenant des points).&lt;br&gt;Tout cluster ayant un nombre de points strictement inférieur au &lt;b&gt;nombre de points minimum&lt;/b&gt; spécifié est éliminé.</source>
        <translation>This step filters clusters (any item containing points).&lt;br&gt;
All cluster with a number of points strictly below the specified &lt;b&gt;minimum number of points&lt;/b&gt; are removed.</translation>
    </message>
    <message>
        <location filename="../step/onf_stepfilterclustersbysize.cpp" line="72"/>
        <source>Clusters</source>
        <translation>Clusters</translation>
    </message>
    <message>
        <location filename="../step/onf_stepfilterclustersbysize.cpp" line="75"/>
        <source>Points</source>
        <translation>Points</translation>
    </message>
    <message>
        <location filename="../step/onf_stepfilterclustersbysize.cpp" line="82"/>
        <source>Nombre de points minimum dans un cluster</source>
        <translation>Minimum points number in one cluster</translation>
    </message>
    <message>
        <location filename="../step/onf_stepfilterclustersbysize.cpp" line="125"/>
        <source>Nombre de clusters avant filtrage : %1</source>
        <translation>Number of clusters before filtering: %1</translation>
    </message>
    <message>
        <location filename="../step/onf_stepfilterclustersbysize.cpp" line="126"/>
        <source>Nombre de clusters éliminés : %1</source>
        <translation>Number of droped clusters: %1</translation>
    </message>
</context>
<context>
    <name>ONF_StepFilterGroupsByGroupsNumber</name>
    <message>
        <location filename="../step/onf_stepfiltergroupsbygroupsnumber.cpp" line="53"/>
        <source>Filtrage de groupes niv.1 / nb. de groupes niv.2</source>
        <translation>Filtering lvl 1 groups / number of lvl 2 groups</translation>
    </message>
    <message>
        <location filename="../step/onf_stepfiltergroupsbygroupsnumber.cpp" line="58"/>
        <source>Cette étape très générique travaille sur deux niveau de groupes.&lt;br&gt;Tout groupe du niveau 1 contenant &lt;b&gt;nombre de groupes&lt;/b&gt; de niveau 2 insuffisant est éliminé.&lt;br&gt;Un usage de cette étape est d&apos;éliminer des groupes de niveau 1 ne contenant pas assez de groupes de niveau 2.&lt;br&gt;Comme par exemple après une étape ONF_StepDetectSection.</source>
        <translation>This very generic step is working on two group levels.&lt;br&gt;
Any group of level 1 containing an to lower &lt;b&gt;number of groups&lt;/b&gt; of level 2 is eliminated.&lt;br&gt;
So this step is eliminate groups of level 1 not containing enough groups of level 2, for example after a ONF_StepDetectSection step.</translation>
    </message>
    <message>
        <location filename="../step/onf_stepfiltergroupsbygroupsnumber.cpp" line="75"/>
        <source>Groupes niv.1 (à filter)</source>
        <translation>Lvl 1 groups (to filter)</translation>
    </message>
    <message>
        <location filename="../step/onf_stepfiltergroupsbygroupsnumber.cpp" line="77"/>
        <source>Groupe niv.1 (à filter)</source>
        <translation>Lvl 1 group (to filter)</translation>
    </message>
    <message>
        <location filename="../step/onf_stepfiltergroupsbygroupsnumber.cpp" line="78"/>
        <source>Groupe niv.2 (à dénombrer)</source>
        <translation>Lvl 2 group (to count)</translation>
    </message>
    <message>
        <location filename="../step/onf_stepfiltergroupsbygroupsnumber.cpp" line="85"/>
        <source>Nombre de groupes minimum de niveau 2 dans un groupe de niveau 1</source>
        <translation>Minimum number of lvl 2 groups in a lvl 1 group</translation>
    </message>
</context>
<context>
    <name>ONF_StepFitAndFilterCylindersInSections</name>
    <message>
        <location filename="../step/onf_stepfitandfiltercylindersinsections.cpp" line="79"/>
        <source>Ajustement/Filtrage des cylindres / billon</source>
        <translation>Fitting / Filtering of cylinders by log</translation>
    </message>
    <message>
        <location filename="../step/onf_stepfitandfiltercylindersinsections.cpp" line="84"/>
        <source>No detailled description for this step</source>
        <translation>Pas de description détaillée pour cette étape</translation>
    </message>
    <message>
        <location filename="../step/onf_stepfitandfiltercylindersinsections.cpp" line="100"/>
        <source>Groupe</source>
        <translation>Group</translation>
    </message>
    <message>
        <location filename="../step/onf_stepfitandfiltercylindersinsections.cpp" line="97"/>
        <source>Billons</source>
        <translation>Logs</translation>
    </message>
    <message>
        <location filename="../step/onf_stepfitandfiltercylindersinsections.cpp" line="99"/>
        <source>Billon (Grp)</source>
        <translation>Log (Grp)</translation>
    </message>
    <message>
        <location filename="../step/onf_stepfitandfiltercylindersinsections.cpp" line="101"/>
        <source>Points</source>
        <translation>Points</translation>
    </message>
    <message>
        <location filename="../step/onf_stepfitandfiltercylindersinsections.cpp" line="102"/>
        <source>Point de référence</source>
        <translation>Reference point</translation>
    </message>
    <message>
        <location filename="../step/onf_stepfitandfiltercylindersinsections.cpp" line="109"/>
        <source>Rayon minimum  :</source>
        <translation>Minimum radius:</translation>
    </message>
    <message>
        <location filename="../step/onf_stepfitandfiltercylindersinsections.cpp" line="110"/>
        <source>Rayon maximum  :</source>
        <translation>Maximum radius:</translation>
    </message>
    <message>
        <location filename="../step/onf_stepfitandfiltercylindersinsections.cpp" line="111"/>
        <source>Filtrer les cylindres sur l&apos;erreur absolue</source>
        <translation>Filter cylinders on absolute error</translation>
    </message>
    <message>
        <location filename="../step/onf_stepfitandfiltercylindersinsections.cpp" line="112"/>
        <source>Erreur maximum :</source>
        <translation>Maximum error:</translation>
    </message>
    <message>
        <location filename="../step/onf_stepfitandfiltercylindersinsections.cpp" line="113"/>
        <source>Filtrer les cylindres sur l&apos;erreur relative</source>
        <translation>Filter cylinders on relative error</translation>
    </message>
    <message>
        <location filename="../step/onf_stepfitandfiltercylindersinsections.cpp" line="114"/>
        <source>Erreur maximum relative au diamètre :</source>
        <translation>Maximum relative (to diameter) error:</translation>
    </message>
    <message>
        <location filename="../step/onf_stepfitandfiltercylindersinsections.cpp" line="115"/>
        <source>Filtrer les cylindres sur leur verticalité</source>
        <translation>Filter cylinders on verticallity</translation>
    </message>
    <message>
        <location filename="../step/onf_stepfitandfiltercylindersinsections.cpp" line="116"/>
        <source>Angle maximal à la verticale (depuis de zénith) :</source>
        <translation>Maximum angle from vertical (zenithal angle):</translation>
    </message>
    <message>
        <location filename="../step/onf_stepfitandfiltercylindersinsections.cpp" line="122"/>
        <source>Cylindre</source>
        <translation>Cylinder</translation>
    </message>
</context>
<context>
    <name>ONF_StepFitCirclesAndFilter</name>
    <message>
        <location filename="../step/onf_stepfitcirclesandfilter.cpp" line="54"/>
        <source>Ajustement/Filtrage des cercles / cluster</source>
        <translation>Fitting / Filtering of circles by log</translation>
    </message>
    <message>
        <location filename="../step/onf_stepfitcirclesandfilter.cpp" line="59"/>
        <source>Cette étape ajoute un cercle dans chaque cluster d&apos;entrée.&lt;br&gt;Les cercles sont ajustés par moindres carrés sur les groupes de points.&lt;br&gt;Les paramètres de l&apos;étape permettent d&apos;activer optionnellement un  &lt;b&gt;filtrage&lt;/b&gt; de cercles.&lt;br&gt;Les criètres de filtrages sont le &lt;b&gt;rayon minimum&lt;/b&gt;, le &lt;b&gt;rayon maximum&lt;/b&gt; et l&apos; &lt;b&gt;erreur d&apos;ajustement du cercle&lt;/b&gt; maximale autorisée.</source>
        <translation>This step adds a circle in each input cluster.&lt;br&gt;
Circles are adjusted by least squares on clusters of points.&lt;br&gt;
Parameters for this stepallow to optionally activate the &lt;b&gt;filtering&lt;/b &gt; of circles.&lt;br&gt;
Filtering criterions are &lt;b&gt;minimum radius&lt;/b&gt;, &lt;b&gt;maximum radius&lt;/b&gt; and maximum allowed &lt;b&gt;adjustment error of the circle&lt;/b&gt;.</translation>
    </message>
    <message>
        <location filename="../step/onf_stepfitcirclesandfilter.cpp" line="75"/>
        <source>Clusters</source>
        <translation>Clusters</translation>
    </message>
    <message>
        <location filename="../step/onf_stepfitcirclesandfilter.cpp" line="77"/>
        <source>Cluster (Grp)</source>
        <translation>Cluster (Grp)</translation>
    </message>
    <message>
        <location filename="../step/onf_stepfitcirclesandfilter.cpp" line="78"/>
        <source>Points</source>
        <translation>Points</translation>
    </message>
    <message>
        <location filename="../step/onf_stepfitcirclesandfilter.cpp" line="85"/>
        <source>Filtrer les cercles sur les critres suivants</source>
        <translation>Filter circles on  following criterion</translation>
    </message>
    <message>
        <location filename="../step/onf_stepfitcirclesandfilter.cpp" line="86"/>
        <source>Rayon minimum  :</source>
        <translation>Minimum radius:</translation>
    </message>
    <message>
        <location filename="../step/onf_stepfitcirclesandfilter.cpp" line="87"/>
        <source>Rayon maximum  :</source>
        <translation>Maximum radius:</translation>
    </message>
    <message>
        <location filename="../step/onf_stepfitcirclesandfilter.cpp" line="88"/>
        <source>Erreur maximum :</source>
        <translation>Maximum error:</translation>
    </message>
    <message>
        <location filename="../step/onf_stepfitcirclesandfilter.cpp" line="95"/>
        <source>Cercle</source>
        <translation>Circle</translation>
    </message>
</context>
<context>
    <name>ONF_StepHorizontalClustering04</name>
    <message>
        <location filename="../step/onf_stephorizontalclustering04.cpp" line="73"/>
        <source>Clustering / tranches horizontales</source>
        <translation>Clustering by horizontal slices</translation>
    </message>
    <message>
        <location filename="../step/onf_stephorizontalclustering04.cpp" line="78"/>
        <source>Cette étape vise à constituer de petits groupes de points aggrégés. L&apos;idée est d&apos;obtenir, dans le cas de troncs d&apos;arbres des arcs de cercle peu épais.&lt;br&gt;Pour ce faire, l&apos;étape fonctionne en deux étapes :&lt;ul&gt;&lt;li&gt; La scène est découpée en tranches horizontales (Layers) de l&apos; &lt;b&gt;épaisseur&lt;/b&gt; choisie&lt;/li&gt;&lt;li&gt; Dans chacune des tranches, les points sont aggrégés en clusters en fonction de leur espacement en (x,y)&lt;/li&gt;&lt;/ul&gt;&lt;br&gt;La &lt;b&gt;distance maximale séparant deux points d&apos;un même groupe&lt;/b&gt; est spécifiée en paramètre.</source>
        <translation>This step create points clusters. In the case of tree trunks, the idea is to obtain thin circles arcs&lt;br&gt;
To do this, the step works in two phases:
&lt;Ul&gt;
&lt;li&gt;The scene is sliced in horizontal layers of defined &lt;b&gt;thickness&lt;/b&gt;&lt;/li&gt;
&lt;li&gt;In each slice, the points are aggregated into clusters according to their spacing in (x, y)&lt;/li&gt;
&lt;/ul&gt;
The &lt;b&gt;maximum distance between two points within a group&lt;/b&gt; is specified as a parameter.</translation>
    </message>
    <message>
        <location filename="../step/onf_stephorizontalclustering04.cpp" line="98"/>
        <source>Scène(s)</source>
        <translation>Scene(s)</translation>
    </message>
    <message>
        <location filename="../step/onf_stephorizontalclustering04.cpp" line="100"/>
        <source>Groupe</source>
        <translation>Group</translation>
    </message>
    <message>
        <location filename="../step/onf_stephorizontalclustering04.cpp" line="101"/>
        <source>Scène à clusteriser</source>
        <translation>Scene to clusterize</translation>
    </message>
    <message>
        <location filename="../step/onf_stephorizontalclustering04.cpp" line="108"/>
        <source>Distance maximum pour intégrer un point à un groupe :</source>
        <translation>Maximum distance to integrate a point in a group:</translation>
    </message>
    <message>
        <location filename="../step/onf_stephorizontalclustering04.cpp" line="109"/>
        <source>Epaisseur des tranches horizontales :</source>
        <translation>Thickness of horizontal slices:</translation>
    </message>
    <message>
        <location filename="../step/onf_stephorizontalclustering04.cpp" line="114"/>
        <source>Scène clusterisée</source>
        <translation>Clusterized scene</translation>
    </message>
    <message>
        <location filename="../step/onf_stephorizontalclustering04.cpp" line="116"/>
        <source>Niveau Z (Grp)</source>
        <translation>Z-Level (Grp)</translation>
    </message>
    <message>
        <location filename="../step/onf_stephorizontalclustering04.cpp" line="117"/>
        <source>Cluster (Grp))</source>
        <translation>Cluster (Grp)</translation>
    </message>
    <message>
        <location filename="../step/onf_stephorizontalclustering04.cpp" line="118"/>
        <source>Points</source>
        <translation>Points</translation>
    </message>
    <message>
        <location filename="../step/onf_stephorizontalclustering04.cpp" line="156"/>
        <source>La scène à clusteriser comporte %1 points.</source>
        <translation>The scene to clusterize contains %1 points.</translation>
    </message>
    <message>
        <location filename="../step/onf_stephorizontalclustering04.cpp" line="238"/>
        <source>L&apos;étape a généré %1 couches horizontales.</source>
        <translation>The step has generated %1 horizontal slices.</translation>
    </message>
</context>
<context>
    <name>ONF_StepHorizontalClustering05</name>
    <message>
        <location filename="../step/onf_stephorizontalclustering05.cpp" line="71"/>
        <source>Clustering / tranches horizontales / scène (v5)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../step/onf_stephorizontalclustering05.cpp" line="76"/>
        <source>Cette étape vise à constituer de petits groupes de points aggrégés. L&apos;idée est d&apos;obtenir, dans le cas de troncs d&apos;arbres des arcs de cercle peu épais.&lt;br&gt;Pour ce faire, l&apos;étape fonctionne en deux étapes :&lt;ul&gt;&lt;li&gt; La scène est découpée en tranches horizontales (Layers) de l&apos; &lt;b&gt;épaisseur&lt;/b&gt; choisie&lt;/li&gt;&lt;li&gt; Dans chacune des tranches, les points sont aggrégés en clusters en fonction de leur espacement en (x,y)&lt;/li&gt;&lt;/ul&gt;&lt;br&gt;La &lt;b&gt;distance maximale séparant deux points d&apos;un même groupe&lt;/b&gt; est spécifiée en paramètre.&lt;br&gt;Cette version 05 de l&apos;étape permet de traiter séparément chaque scène d&apos;entrée.&lt;br&gt;De plus dans cette version, le résultat d&apos;entrée est en copie.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../step/onf_stephorizontalclustering05.cpp" line="98"/>
        <source>Scène(s)</source>
        <translation type="unfinished">Scene(s)</translation>
    </message>
    <message>
        <location filename="../step/onf_stephorizontalclustering05.cpp" line="100"/>
        <source>Groupe</source>
        <translation type="unfinished">Group</translation>
    </message>
    <message>
        <location filename="../step/onf_stephorizontalclustering05.cpp" line="101"/>
        <source>Scène à clusteriser</source>
        <translation type="unfinished">Scene to clusterize</translation>
    </message>
    <message>
        <location filename="../step/onf_stephorizontalclustering05.cpp" line="108"/>
        <source>Distance maximum pour intégrer un point à un groupe :</source>
        <translation type="unfinished">Maximum distance to integrate a point in a group:</translation>
    </message>
    <message>
        <location filename="../step/onf_stephorizontalclustering05.cpp" line="109"/>
        <source>Epaisseur des tranches horizontales :</source>
        <translation type="unfinished">Thickness of horizontal slices:</translation>
    </message>
    <message>
        <location filename="../step/onf_stephorizontalclustering05.cpp" line="116"/>
        <source>Niveau Z (Grp)</source>
        <translation type="unfinished">Z-Level (Grp)</translation>
    </message>
    <message>
        <location filename="../step/onf_stephorizontalclustering05.cpp" line="117"/>
        <source>Cluster (Grp)</source>
        <translation type="unfinished">Cluster (Grp)</translation>
    </message>
    <message>
        <location filename="../step/onf_stephorizontalclustering05.cpp" line="118"/>
        <source>Points</source>
        <translation type="unfinished">Points</translation>
    </message>
    <message>
        <location filename="../step/onf_stephorizontalclustering05.cpp" line="159"/>
        <source>La scène à clusteriser comporte %1 points.</source>
        <translation type="unfinished">The scene to clusterize contains %1 points.</translation>
    </message>
    <message>
        <location filename="../step/onf_stephorizontalclustering05.cpp" line="241"/>
        <source>L&apos;étape a généré %1 couches horizontales.</source>
        <translation type="unfinished">The step has generated %1 horizontal slices.</translation>
    </message>
</context>
<context>
    <name>ONF_StepHorizontalClustering3D</name>
    <message>
        <location filename="../step/onf_stephorizontalclustering3d.cpp" line="71"/>
        <source>Clustering 3D</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../step/onf_stephorizontalclustering3d.cpp" line="76"/>
        <source>Cette étape vise à constituer de petits groupes de points aggrégés. L&apos;idée est d&apos;obtenir, dans le cas de troncs d&apos;arbres des arcs de cercle peu épais.&lt;br&gt;Pour ce faire, l&apos;étape fonctionne en deux étapes :&lt;ul&gt;&lt;li&gt; La scène est découpée en tranches horizontales (Layers) de l&apos; &lt;b&gt;épaisseur&lt;/b&gt; choisie&lt;/li&gt;&lt;li&gt; Dans chacune des tranches, les points sont aggrégés en clusters en fonction de leur espacement en (x,y)&lt;/li&gt;&lt;/ul&gt;&lt;br&gt;La &lt;b&gt;distance maximale séparant deux points d&apos;un même groupe&lt;/b&gt; est spécifiée en paramètre.&lt;br&gt;Cette version 05 de l&apos;étape permet de traiter séparément chaque scène d&apos;entrée.&lt;br&gt;De plus dans cette version, le résultat d&apos;entrée est en copie.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../step/onf_stephorizontalclustering3d.cpp" line="98"/>
        <source>Scène(s)</source>
        <translation type="unfinished">Scene(s)</translation>
    </message>
    <message>
        <location filename="../step/onf_stephorizontalclustering3d.cpp" line="100"/>
        <source>Groupe</source>
        <translation type="unfinished">Group</translation>
    </message>
    <message>
        <location filename="../step/onf_stephorizontalclustering3d.cpp" line="101"/>
        <source>Scène à clusteriser</source>
        <translation type="unfinished">Scene to clusterize</translation>
    </message>
    <message>
        <location filename="../step/onf_stephorizontalclustering3d.cpp" line="108"/>
        <source>Distance maximum pour intégrer un point à un groupe :</source>
        <translation type="unfinished">Maximum distance to integrate a point in a group:</translation>
    </message>
    <message>
        <location filename="../step/onf_stephorizontalclustering3d.cpp" line="109"/>
        <source>Epaisseur des tranches horizontales :</source>
        <translation type="unfinished">Thickness of horizontal slices:</translation>
    </message>
    <message>
        <location filename="../step/onf_stephorizontalclustering3d.cpp" line="116"/>
        <source>Niveau Z (Grp)</source>
        <translation type="unfinished">Z-Level (Grp)</translation>
    </message>
    <message>
        <location filename="../step/onf_stephorizontalclustering3d.cpp" line="117"/>
        <source>Cluster (Grp)</source>
        <translation type="unfinished">Cluster (Grp)</translation>
    </message>
    <message>
        <location filename="../step/onf_stephorizontalclustering3d.cpp" line="118"/>
        <source>Points</source>
        <translation type="unfinished">Points</translation>
    </message>
    <message>
        <location filename="../step/onf_stephorizontalclustering3d.cpp" line="159"/>
        <source>La scène à clusteriser comporte %1 points.</source>
        <translation type="unfinished">The scene to clusterize contains %1 points.</translation>
    </message>
    <message>
        <location filename="../step/onf_stephorizontalclustering3d.cpp" line="241"/>
        <source>L&apos;étape a généré %1 couches horizontales.</source>
        <translation type="unfinished">The step has generated %1 horizontal slices.</translation>
    </message>
</context>
<context>
    <name>ONF_StepMergeEndToEndSections04</name>
    <message>
        <location filename="../step/onf_stepmergeendtoendsections04.cpp" line="72"/>
        <source>Fusion de billons alignés</source>
        <translation>Merging of aligned logs</translation>
    </message>
    <message>
        <location filename="../step/onf_stepmergeendtoendsections04.cpp" line="77"/>
        <source>No detailled description for this step</source>
        <translation>Pas de description détaillée pour cette étape</translation>
    </message>
    <message>
        <location filename="../step/onf_stepmergeendtoendsections04.cpp" line="90"/>
        <source>Billons</source>
        <translation>Logs</translation>
    </message>
    <message>
        <location filename="../step/onf_stepmergeendtoendsections04.cpp" line="92"/>
        <location filename="../step/onf_stepmergeendtoendsections04.cpp" line="111"/>
        <source>Billon (Grp)</source>
        <translation>Log (Grp)</translation>
    </message>
    <message>
        <location filename="../step/onf_stepmergeendtoendsections04.cpp" line="93"/>
        <location filename="../step/onf_stepmergeendtoendsections04.cpp" line="112"/>
        <source>Cluster (Grp)</source>
        <translation>Cluster (Grp)</translation>
    </message>
    <message>
        <location filename="../step/onf_stepmergeendtoendsections04.cpp" line="94"/>
        <location filename="../step/onf_stepmergeendtoendsections04.cpp" line="113"/>
        <source>Points</source>
        <translation>Points</translation>
    </message>
    <message>
        <location filename="../step/onf_stepmergeendtoendsections04.cpp" line="101"/>
        <source>Epaisseur des groupes en Z  :</source>
        <translation>Z-thickness of groups:</translation>
    </message>
    <message>
        <location filename="../step/onf_stepmergeendtoendsections04.cpp" line="102"/>
        <source>Distance maximale entre extremités de billons à fusionner :</source>
        <translation>Maximum distance beetween extremities of logs to merge:</translation>
    </message>
    <message>
        <location filename="../step/onf_stepmergeendtoendsections04.cpp" line="103"/>
        <source>Nombre de barycentres a considerer aux extremites :</source>
        <translation>Number of barycenters to consider at extremities:</translation>
    </message>
    <message>
        <location filename="../step/onf_stepmergeendtoendsections04.cpp" line="104"/>
        <source>Facteur multiplicatif de maxDist :</source>
        <translation>Multiplicative factor for maxDist:</translation>
    </message>
    <message>
        <location filename="../step/onf_stepmergeendtoendsections04.cpp" line="105"/>
        <source>Chevauchement toléré en Z :</source>
        <translation>Tolerated Z overlapping:</translation>
    </message>
    <message>
        <location filename="../step/onf_stepmergeendtoendsections04.cpp" line="110"/>
        <source>Billons Fusionnées</source>
        <translation>Merged logs</translation>
    </message>
    <message>
        <location filename="../step/onf_stepmergeendtoendsections04.cpp" line="114"/>
        <source>Barycentre</source>
        <translation>Barycenter</translation>
    </message>
</context>
<context>
    <name>ONF_StepMergeNeighbourSections04</name>
    <message>
        <location filename="../step/onf_stepmergeneighboursections04.cpp" line="73"/>
        <source>Fusion de billons parallèles</source>
        <translation>Merging of parallel logs</translation>
    </message>
    <message>
        <location filename="../step/onf_stepmergeneighboursections04.cpp" line="78"/>
        <source>Cette étape prend en entrée une liste de billons. Chaque billon est composée d&apos;une séquence de clusters. &lt;br&gt;Un cluster est caractérisé par :&lt;ul&gt;&lt;li&gt;Une liste de points&lt;/li&gt;&lt;li&gt;Un barycentre (le barycentre des points)&lt;/li&gt;&lt;li&gt;Une valeur &lt;em&gt;buffer&lt;/em&gt;, égale à la distance entre le barycentre et le point le plus éloigné du barycentre&lt;/li&gt;&lt;/ul&gt;&lt;br&gt;Ces billons sont issues d&apos;une étape précédente telle que &lt;em&gt;ONF_StepDetectSection&lt;/em&gt;. Cependant, en début d&apos;étape elles sont remaniées de façon à ce que les clusters aient l&apos; &lt;b&gt;épaisseur&lt;/b&gt; choisie en paramètre de l&apos;étape.&lt;br&gt;Au sein de chaque billon ce remaniement consiste à prendre tous les points de tous les clusters, afin de recréer des clusters de l&apos; &lt;b&gt;épaisseur&lt;/b&gt; choisie.&lt;br&gt;Ensuite, pour chaque cluster créé, on en détermine le barycentre et le buffer.&lt;br&gt;&lt;b&gt;Le but de cette étape est de fusionner des billons appartenant dans la réalité au même arbre&lt;/b&gt;.&lt;br&gt;Elle traite spécifiquement le cas des billons se chevauchant verticalement. Elle est complétée par &lt;em&gt;ONF_StepMergeEndToEndSections&lt;/em&gt;.&lt;br&gt;En plus de l&apos; &lt;b&gt;épaisseur de cluster&lt;/b&gt;, cette étape utilise les paramètres suivants :&lt;ul&gt;&lt;li&gt;Une &lt;b&gt;distance de recherche de voisinnage&lt;/b&gt; (paramètre d&apos;optimisation des calculs)&lt;/li&gt;&lt;li&gt;Une distance &lt;b&gt;deltaZ&lt;/b&gt; : écart vertical maximal entre deux barycentres comparés&lt;/li&gt;&lt;li&gt;Un critère &lt;b&gt;distMax&lt;/b&gt; : distance XY maximum entre deux barycentres de billons à fusionner&lt;/li&gt;&lt;li&gt;Un critère &lt;b&gt;ratioMax&lt;/b&gt; : accroissement maximal du buffer accepté en cas de fusion&lt;/li&gt;&lt;/ul&gt;&lt;br&gt;Le fonctionnement de l&apos;étape est le suivant. Les billons sont comparées deux à deux par ordre décroissant de longueur selon Z.A chaque itération, on compare une billon A (la plus longue) constituée de n clusters ayant des barycentres Ai (i = 1 à n), avec une billon B constituée de m clusters ayant des barycentres Bj (j = 1 à m).&lt;br&gt;Pour ce faire on commence par calculer &lt;b&gt;medBuffer&lt;/b&gt; : la médiane des distances buffers des barycentres Ai.&lt;br&gt;Pour que A et B soient fusionnées, il faut que pour tout i et j tels que la distance verticale |Ai - Bj|z &lt; &lt;b&gt;deltaZ&lt;/b&gt;&lt;ul&gt;&lt;li&gt;Qu&apos;aucune distance horizontale |Ai - Bj|xy ne soit supérieure à &lt;b&gt;distMax&lt;/b&gt;&lt;/li&gt;&lt;li&gt;Qu&apos;aucune distance horizontale |Ai - Bj|xy ne soit supérieure à &lt;b&gt;medDist&lt;/b&gt;&lt;/li&gt;&lt;li&gt;Qu&apos;au moins pour un couple Ai / Bj, le ratio |Ai - Bj| / MAX(buffer Ai, buffer Bj) soit inférieur à &lt;b&gt;ratioMax&lt;/b&gt;&lt;/ul&gt;En cas de fusion, les clusters et les barycentres sont recréés à partir de tous les points des deux billons sources pour former une nouvelle billon C.&lt;br&gt;La billon C devient la de facto la plus longue : elle est donc aussitôt utilisée dans l&apos;itération suivant dans la comparaison avec la prochaine billon (plus petite) de la liste.</source>
        <translation>This step takes as input a list of logs. Each log is composed of a sequence of clusters.&lt;br&gt;
A cluster is characterized by:
&lt;ul&gt;
&lt;li&gt;A list of points&lt;/li&gt;
&lt;li&gt;A centroid (the centroid points)&lt;/li&gt;
&lt;li&gt;A value of &lt;em&gt;buffer&lt;/em &gt; equal to the distance between the centroid and the farthest point from the centroid&lt;/li&gt;
&lt;/ul&gt; 
These logs are created in a previous step like &lt;em&gt;ONF_StepDetectSection&lt;/em&gt;. However, in the early stage they are processed so that the clusters have the selected &lt;b&gt;thickness&lt;/b&gt; at the beginning of the step.&lt;br&gt;
Within each log this process takes all points of all clusters, abd recreate clusters of the chosen &lt;b&gt;thickness&lt;/b&gt;.&lt;br&gt;
After, for each created cluster, we the centroid and the buffer are computed too.&lt;br&gt;
&lt;b&gt;The purpose of this step is to merge logs which in reality belongs to the same tree&lt;/b&gt;.&lt;br&gt;
It deals specifically with the case of vertically overlapping logs (parallels). It is supplemented by &lt;em&gt;ONF_StepMergeEndToEndSections &lt;/em&gt;. &lt;br&gt;
In addition to the &lt;b&gt;thickness of clusters&lt;/b&gt;, this step uses the following parameters:
&lt;ul&gt; 
&lt;li&gt; &lt;b&gt;Neighborhoud searching distance&lt;/b&gt; (optimization parameter)&lt;/li&gt;
&lt;li&gt;Distance &lt;b&gt;DeltaZ&lt;/b&gt;: Maximum vertical distance between two compared centroids&lt;/li&gt;
&lt;li&gt;A criterion &lt;b&gt;distMax&lt;/b&gt;: XY maximum distance between two centroids in logs to merge&lt;/li&gt;
&lt;li&gt;A criterion &lt;b&gt;ratioMax&lt;/b&gt;: Maximum increase of buffer accepted when merging&lt;/li&gt;
&lt;/ul&gt;
&lt;br&gt;The step works as follows. Logs are compared in pairs in order of decreasing Z length. At each iteration, we compare a log A (longest) consisting of n clusters with centroids Ai (i = 1 to n), with a log B consisting of m clusters with centroids Bj (j = 1 to m)&lt;br&gt;
To do this we first calculate &lt;b&gt;medBuffer&lt;/b&gt; the median buffers of Ai clusters&lt;br&gt;
A and B are merged, if for all i and j whith a vertical distance |Ai - Bj|z &amp;lt; &lt;b&gt;DeltaZ&lt;/b&gt;
&lt;ul&gt; 
&lt;li&gt;- no horizontal distance |Ai - Bj|xy is greater than &lt;b&gt;distMax&lt;/b&gt;&lt;/li&gt;
&lt;li&gt;- no horizontal distance |Ai - Bj|xy is greater than &lt;b&gt;medDist&lt;/b&gt;&lt;/li&gt;
&lt;li&gt;- at least for a couple Ai / Bj, the ratio |Ai - Bj| / MAX (buffer Ai, buffer Bj) is less than &lt;b&gt;ratioMax&lt;/b&gt;
&lt;/ul&gt;
If A and B are merged, clusters and centroids are recreated from all points of the logs A and B to form a new log C.&lt;br&gt;
Log C becomes the longest log: it is immediately used in the next iteration in comparison with the next log (smaller) of the list.</translation>
    </message>
    <message>
        <location filename="../step/onf_stepmergeneighboursections04.cpp" line="122"/>
        <source>Billons</source>
        <translation>Logs</translation>
    </message>
    <message>
        <location filename="../step/onf_stepmergeneighboursections04.cpp" line="124"/>
        <location filename="../step/onf_stepmergeneighboursections04.cpp" line="143"/>
        <source>Billon (Grp)</source>
        <translation>Log (Grp)</translation>
    </message>
    <message>
        <location filename="../step/onf_stepmergeneighboursections04.cpp" line="125"/>
        <location filename="../step/onf_stepmergeneighboursections04.cpp" line="144"/>
        <source>Cluster (Grp)</source>
        <translation>Cluster (Grp)</translation>
    </message>
    <message>
        <location filename="../step/onf_stepmergeneighboursections04.cpp" line="126"/>
        <location filename="../step/onf_stepmergeneighboursections04.cpp" line="145"/>
        <source>Points</source>
        <translation>Points</translation>
    </message>
    <message>
        <location filename="../step/onf_stepmergeneighboursections04.cpp" line="133"/>
        <source>Epaisseur (en Z) des clusters  :</source>
        <translation>Z-thickness of clusters:</translation>
    </message>
    <message>
        <location filename="../step/onf_stepmergeneighboursections04.cpp" line="134"/>
        <source>Distance de recherche de voisinage :</source>
        <translation>Distance for neighbourood search:</translation>
    </message>
    <message>
        <location filename="../step/onf_stepmergeneighboursections04.cpp" line="135"/>
        <source>Distance XY maximum entre barycentres de clusters de billons à fusionner :</source>
        <translation>Maximum XY distance between barycenter of clusters for logs to merge:</translation>
    </message>
    <message>
        <location filename="../step/onf_stepmergeneighboursections04.cpp" line="136"/>
        <source>Distance Z maximum entre barycentres de clusters de billons à fusionner :</source>
        <translation>Maximum Z distance between barycenters of clusters for logs to merge:</translation>
    </message>
    <message>
        <location filename="../step/onf_stepmergeneighboursections04.cpp" line="137"/>
        <source>fois</source>
        <translation>times</translation>
    </message>
    <message>
        <location filename="../step/onf_stepmergeneighboursections04.cpp" line="137"/>
        <source>Facteur d&apos;accroissement maximal des distances XY entre barycentres de clusters de billons à fusionner&apos; :</source>
        <translation>Maximum increasing factor for XY distances between barycenters of cluster for logs to merge:</translation>
    </message>
    <message>
        <location filename="../step/onf_stepmergeneighboursections04.cpp" line="142"/>
        <source>Billons Fusionnées</source>
        <translation>Merged logs</translation>
    </message>
    <message>
        <location filename="../step/onf_stepmergeneighboursections04.cpp" line="146"/>
        <source>Barycentre</source>
        <translation>Barycenter</translation>
    </message>
</context>
<context>
    <name>ONF_StepRefPointFromArcCenter</name>
    <message>
        <location filename="../step/onf_steprefpointfromarccenter.cpp" line="55"/>
        <source>Création de points de réf. à partir d&apos;arcs</source>
        <translation>Creation of ref. points from arcs</translation>
    </message>
    <message>
        <location filename="../step/onf_steprefpointfromarccenter.cpp" line="60"/>
        <source>No detailled description for this step</source>
        <translation>Pas de description détaillée pour cette étape</translation>
    </message>
    <message>
        <location filename="../step/onf_steprefpointfromarccenter.cpp" line="73"/>
        <source>Polyline(s)</source>
        <translation>Polyline(s)</translation>
    </message>
    <message>
        <location filename="../step/onf_steprefpointfromarccenter.cpp" line="77"/>
        <source>Polyligne</source>
        <translation>Polyline</translation>
    </message>
    <message>
        <location filename="../step/onf_steprefpointfromarccenter.cpp" line="83"/>
        <source>Barycentre</source>
        <translation>Barycenter</translation>
    </message>
</context>
<context>
    <name>ONF_StepRefPointFromBarycenter02</name>
    <message>
        <location filename="../step/onf_steprefpointfrombarycenter02.cpp" line="56"/>
        <source>Création de points de réf. à partir de barycentres</source>
        <translation>Creation of ref. points from barycenters</translation>
    </message>
    <message>
        <location filename="../step/onf_steprefpointfrombarycenter02.cpp" line="61"/>
        <source>No detailled description for this step</source>
        <translation>Pas de description détaillée pour cette étape</translation>
    </message>
    <message>
        <location filename="../step/onf_steprefpointfrombarycenter02.cpp" line="74"/>
        <source>Polylignes</source>
        <translation>Polylines</translation>
    </message>
    <message>
        <location filename="../step/onf_steprefpointfrombarycenter02.cpp" line="78"/>
        <source>Polyligne</source>
        <translation>Polyline</translation>
    </message>
    <message>
        <location filename="../step/onf_steprefpointfrombarycenter02.cpp" line="84"/>
        <source>Barycentre</source>
        <translation>Barycenter</translation>
    </message>
</context>
<context>
    <name>ONF_StepSetFootCoordinatesVertically</name>
    <message>
        <location filename="../step/onf_stepsetfootcoordinatesvertically.cpp" line="57"/>
        <source>Ajout d&apos;une coordonnée de base / billon // MNT</source>
        <translation>Addition od a base coordinante by log // DTM</translation>
    </message>
    <message>
        <location filename="../step/onf_stepsetfootcoordinatesvertically.cpp" line="62"/>
        <source>No detailled description for this step</source>
        <translation>Pas de description détaillée pour cette étape</translation>
    </message>
    <message>
        <location filename="../step/onf_stepsetfootcoordinatesvertically.cpp" line="77"/>
        <source>MNT (Raster)</source>
        <translation>DTM (raster)</translation>
    </message>
    <message>
        <location filename="../step/onf_stepsetfootcoordinatesvertically.cpp" line="80"/>
        <source>Modèle Numérique de Terrain</source>
        <translation>Digital Terrain Model</translation>
    </message>
    <message>
        <location filename="../step/onf_stepsetfootcoordinatesvertically.cpp" line="84"/>
        <source>Billons</source>
        <translation>Logs</translation>
    </message>
    <message>
        <location filename="../step/onf_stepsetfootcoordinatesvertically.cpp" line="86"/>
        <source>Billon (Grp)</source>
        <translation>Log (Grp)</translation>
    </message>
    <message>
        <location filename="../step/onf_stepsetfootcoordinatesvertically.cpp" line="87"/>
        <source>Cluster (Grp)</source>
        <translation>Cluster (Grp)</translation>
    </message>
    <message>
        <location filename="../step/onf_stepsetfootcoordinatesvertically.cpp" line="88"/>
        <source>Point de référence</source>
        <translation>Reference point</translation>
    </message>
    <message>
        <location filename="../step/onf_stepsetfootcoordinatesvertically.cpp" line="98"/>
        <source>Coordonnée MNT</source>
        <translation>DTM coordinate</translation>
    </message>
</context>
<context>
    <name>ONF_StepSmoothSkeleton</name>
    <message>
        <location filename="../step/onf_stepsmoothskeleton.cpp" line="54"/>
        <source>Lissage d&apos;une séquence de points de référence</source>
        <translation>Smoothing of sequence of ref. points</translation>
    </message>
    <message>
        <location filename="../step/onf_stepsmoothskeleton.cpp" line="59"/>
        <source>No detailled description for this step</source>
        <translation>Pas de description détaillée pour cette étape</translation>
    </message>
    <message>
        <location filename="../step/onf_stepsmoothskeleton.cpp" line="72"/>
        <source>Billons / Clusters / Points de référence</source>
        <translation>Logs / Clusters / Reference points</translation>
    </message>
    <message>
        <location filename="../step/onf_stepsmoothskeleton.cpp" line="74"/>
        <source>Billon (Grp)</source>
        <translation>Log (Grp)</translation>
    </message>
    <message>
        <location filename="../step/onf_stepsmoothskeleton.cpp" line="75"/>
        <source>Cluster (Grp)</source>
        <translation>Cluster (Grp)</translation>
    </message>
    <message>
        <location filename="../step/onf_stepsmoothskeleton.cpp" line="86"/>
        <source>Point de référence (lissé)</source>
        <translation>Reference point (smoothed)</translation>
    </message>
    <message>
        <location filename="../step/onf_stepsmoothskeleton.cpp" line="76"/>
        <source>Point de référence</source>
        <translation>Reference point</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="../itemdrawable/onf_lvoxsphere.h" line="109"/>
        <source>Name</source>
        <translation>Nom</translation>
    </message>
    <message>
        <location filename="../itemdrawable/onf_lvoxsphere.h" line="110"/>
        <source>Xs</source>
        <translation>Xs</translation>
    </message>
    <message>
        <location filename="../itemdrawable/onf_lvoxsphere.h" line="111"/>
        <source>Ys</source>
        <translation>Ys</translation>
    </message>
    <message>
        <location filename="../itemdrawable/onf_lvoxsphere.h" line="112"/>
        <source>Zs</source>
        <translation>Zs</translation>
    </message>
    <message>
        <location filename="../itemdrawable/onf_lvoxsphere.h" line="113"/>
        <source>Radius</source>
        <translation>Rayon</translation>
    </message>
    <message>
        <location filename="../itemdrawable/onf_lvoxsphere.h" line="114"/>
        <source>ExclusionRadius</source>
        <translation>RayonExclusion</translation>
    </message>
    <message>
        <location filename="../itemdrawable/onf_lvoxsphere.h" line="115"/>
        <source>Ni</source>
        <translation>Ni</translation>
    </message>
    <message>
        <location filename="../itemdrawable/onf_lvoxsphere.h" line="116"/>
        <source>Nb</source>
        <translation>Nb</translation>
    </message>
    <message>
        <location filename="../itemdrawable/onf_lvoxsphere.h" line="117"/>
        <source>Nt</source>
        <translation>Nt</translation>
    </message>
    <message>
        <location filename="../itemdrawable/onf_lvoxsphere.h" line="118"/>
        <source>N_excluded</source>
        <translation>N_exclus</translation>
    </message>
</context>
</TS>
