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
#
# takes a bbbike[1] streetfile as input and converts it to some osm xml file format.
#
# bbbike files are usually not in polar coordinate format, you will have to convert them first, e.g.
# like this (from within your local bbbike checkout):
#
#  perl ./miscsrc/convert_coordsys standard polar < data/strassen > /tmp/strassen.polar
#
#  [1]:	bbbike is the free Berlin bike map system: http://bbbike.sourceforge.net/
#  		use the cvs download from sourceforge to retrieve the scripts and data.
#
use Switch;
$created_by="dotslashs crufty bbbike converter";
$node_string="";
$seg_string="";
$way_string="";
$id=1;
( $sec, $min, $hour, $day, $month, $year ) = ( localtime ) [ 0, 1, 2, 3, 4, 5 ];
%translate = ("HH","primary","H","secondary","N","residential", "NN","residential");
$timestamp =sprintf("%04d-%02d-%02d %02d:%02d:%02d",1900+$year,$month,$day, $hour,$min,$sec);
print "<?xml version='1.0' encoding='UTF-8'?>\n";
print "<osm version='0.3' generator='$created_by'>\n";

while (<>) {
	if (/^\#/) {
		next;
	}
	@array=split(/\t/,$_);
	$name=$array[0];
	$name=~ s/\'//eg;	# remove bad '
	$name=~ s/([\x80-\xFF])/chr(0xC0|ord($1)>>6).chr(0x80|ord($1)&0x3F)/eg;	#latin1 -> utf8
	($type, @coords)=split(/[\s]/,$array[1]);
#		$type=shift(@coords);			# actually the first entry isn't some coord ..
	if ($type eq"Pl") {
		($lo, $la)=split(/,/,$coords[0]);
		$node_string.=" <node id='$id' timestamp='$timestamp' lat='$la' lon='$lo'>\n";
		$node_string.="   <tag k='created_by' v='$created_by' />\n";
		$node_string.="   <tag k='amenity' v='foo' />\n";
		$node_string.="   <tag k='name' v='$name' />\n";
		$node_string.=" </node>\n";
			$id++;
	} else {
		@nodeid=(0,0);
		@segs=();
		foreach (@coords) {
			($lo, $la)=split(/,/,$_);
			$node_string.=" <node id='$id' timestamp='$timestamp' lat='$la' lon='$lo' />\n";
			$nodeid[1]=$nodeid[0];
			$nodeid[0]=$id;
			$id++;
			if ($nodeid[1]>0) {
				$seg_string.=" <segment id='$id' from='".$nodeid[1]."' to='".$nodeid[0]."' />\n";
				push(@segs,$id);
				$id++;
			}
		}

		$way_string.=" <way id='$id' timestamp='$timestamp'>\n";
		foreach (@segs) {	$way_string.="   <seg id='$_' />\n";	}
		$way_string.="   <tag k='name' v='$name' />\n";
		$way_string.="   <tag k='motorcar' v='yes' />\n";
		$way_string.="   <tag k='bicycle' v='yes' />\n";
		$way_string.="   <tag k='foot' v='yes' />\n";
		$way_string.="   <tag k='class' v='motorway' />\n";
		$way_string.="   <tag k='highway' v='".$translate{$type}."' />\n";
		$way_string.="   <tag k='created_by' v='$created_by' />\n";

		$way_string.=" </way>\n";
	}
}

print $node_string;
print $seg_string;
print $way_string;
print "</osm>\n";
