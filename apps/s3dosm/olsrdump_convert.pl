#!/usr/bin/env perl
#
#
# Copyright (C) 2006-2008 Simon Wunderlich <dotslash@packetmixer.de>
#
# This file is part of s3dosm, a gps card application for s3d.
# See http://s3d.berlios.de/ for more updates.
#
# s3dosm is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# s3dosm is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with s3dosm; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
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
