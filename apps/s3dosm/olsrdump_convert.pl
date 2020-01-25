#!/usr/bin/env perl
#
# SPDX-License-Identifier: GPL-2.0-or-later
# SPDX-FileCopyrightText: 2006-2015  Simon Wunderlich <sw@simonwunderlich.de>
#
#
# converts freifunk database dump to osm format, which can be loaded with s3dosm.
# this is no useful script, so don't package it or something. :)
#
$created_by="dotslash";
$node_string="";
$seg_string="";
$way_string="";
$id=-1;
( $sec, $min, $hour, $day, $month, $year ) = ( localtime ) [ 0, 1, 2, 3, 4, 5 ];
$timestamp =sprintf("%04d-%02d-%02d %02d:%02d:%02d",1900+$year,$month,$day, $hour,$min,$sec);

print "<?xml version='1.0' encoding='UTF-8'?>\n";
print "<osm version='0.3' generator='$created_by'>\n";

while (<>) {
	@a=split(",");
	$ip=$a[1];
	$name=$a[4];
	@co=split("[(\ )]",$a[6]);
	$lo=$co[2];
	$la=$co[3];
	if ($a[0]=~/^INSERT/) {
		$node_string.=" <node id='$id' timestamp='$timestamp' lat='$la' lon='$lo'>\n";
		$node_string.="   <tag k='created_by' v='$created_by' />\n";
		$node_string.="   <tag k='amenity' v='accesspoint' />\n";
		$node_string.="   <tag k='accesspoint' v='freifunk' />\n";
		$node_string.="   <tag k='name' v=$name />\n";
		$node_string.="   <tag k='ip' v=$ip />\n";
		$node_string.=" </node>\n";
	}
	$id--;
}
print $node_string;
print "</osm>\n";
