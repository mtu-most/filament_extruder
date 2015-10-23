/************************************************************************************

filament_extruder.scad - various parts for the MOST filament extruder V10000
Copyright 2015 Jerry Anzalone
Author: Jerry Anzalone <gcanzalo@mtu.edu>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.

************************************************************************************/

// hopper for filament extruder
include <fasteners.scad>
include <threads.scad>
$fn = 48;
od_pipe = 22; // measured 21.52
l_gap = 10;
h_gap = 1.5 * 25.4;
pad_pipe = 5;
od_pipe_engagement = od_pipe + 2 * pad_pipe;
h_pipe_engagement = h_gap + 2 * pad_pipe;
od_hopper_engagement = 24;
l_hopper_base = 70;
w_hopper_base = 25;
t_hopper_walls = 4;
w_clamp = od_pipe_engagement / 2 + 8;
l_clamp = 8;
y_offset_hopper = 45 + od_hopper_engagement / 2;
z_offset_hopper = 80;

cc_flange_mounts = 7 * 25.4 / 4;
d_flange_mounts = 5 * 25.4 / 16;
h_outer_race = 20;
d_outer_race = 50.4;
d_inner = 41;

pitch_hopper = 4.00;
d_hopper = 39; //36.5;
// for the jug adapter:
a_hopper = 38;
pad_threads = 5;
h_threads = 13.1;
id_throat = 30;


//hopper_base();
//windshield_hopper();

//trailer_bearing_block();

//feed_chute();

//jug_chute(review = false);

//jug_chute_mate();

// for the printable bearing bushing
// bushing will have a 1/2" flat washer in front to distribute load over face of bushing
h_bushing = 20;
h_bushing_flange = 5;
id_bearing = 25.35;
od_bearing_flange = 39.8;
r_id_bearing = 2.5; // the inner race has a radiused shoulder
d_auger_shank = 12.7;
dmax_auger_shank = 13.25;
h_auger_taper = 6;

difference() {
	union() {
		cylinder(r = id_bearing / 2, h = h_bushing);
	
		cylinder(r = od_bearing_flange / 2, h = h_bushing_flange);
		
		translate([0, 0, h_bushing_flange])
			cylinder(r = od_bearing_flange / 2 - 2 * r_id_bearing, h = r_id_bearing);
	}
	
	translate([0, 0, h_bushing_flange + r_id_bearing])
		rotate_extrude(convexity = 10)
			translate([id_bearing / 2 + r_id_bearing, 0, 0])
				circle(r = r_id_bearing);
	
	translate([0, 0, -1])
		cylinder(r = d_auger_shank / 2, h = h_bushing + 2);
	
	translate([0, 0, -0.01])
#		cylinder(r1 = dmax_auger_shank / 2, r2 = d_auger_shank / 2, h = h_auger_taper);
}




module jug_chute_mate() {
	difference() {
			pipe_engagement();

	//		translate([0, 0, -h_gap / 2])
	//			cylinder(r1 = od_pipe / 2, r2 = od_pipe / 2 + pad_pipe + 10 - pad_pipe, h = 10);
	
	//		translate([0, 0, 9.9 - h_gap / 2])
	//			cylinder(r = od_pipe / 2 + pad_pipe + 10.1 - pad_pipe, h = h_gap - 10);

			cylinder(r = od_pipe / 2, h = 200, center = true);
		
				translate([-30, 0, 0])
					cube([60, 60, h_gap + 300], center = true);
	}
}

module jug_chute(review = false) {
	difference() {
		union() {
			pipe_engagement();

			translate([0, 0, -8])
				jug_engagement();
		}
	
		// feed gap
		translate([0, 0, -h_gap / 2])
			cylinder(r1 = od_pipe / 2, r2 = od_pipe / 2 + 10, h = 10 * tan(90 - a_hopper));
	
		translate([0, 0, 9.9 * tan(90 - a_hopper) - h_gap / 2])
			cylinder(r = od_pipe / 2 + pad_pipe + 10.1 - pad_pipe, h = h_gap - 10 * tan(90 - a_hopper));

		cylinder(r = od_pipe / 2, h = 200, center = true);

//		translate([od_pipe / 2 + pad_pipe + 10, 0, 0]) {
//			hull()
//				for (i = [-1, 0])
//					translate([i * (od_pipe / 2 + pad_pipe + 10.1 - pad_pipe), 0, 0])
//						sphere(r = id_throat / 2);

			translate([0, 0, -8])
			rotate([0, a_hopper, 0]) {
				cylinder(r = id_throat / 2, h = 95);
		
				if (review)
					translate([0, 0, 47])
						cylinder(r = d_hopper / 2, h = h_threads);
				else
					translate([0, 0, 47])
						metric_thread(diameter = d_hopper, pitch = pitch_hopper, length = h_threads, internal = true, n_starts = 1);

			}
//		}
		
//		translate([0, 0, (h_gap + 25) / 2 - 4.25])
//			difference() {
//				cylinder(r = od_pipe / 2 + pad_pipe + 3, h = 9, center = true);

//				cylinder(r = od_pipe / 2 + pad_pipe, h = 10, center = true);
//			}
		
		translate([-30, 0, 0])
			cube([60, 60, h_gap + 300], center = true);
	}
}

module pipe_engagement() {
	union() {
		cylinder(r = od_pipe / 2 + pad_pipe + 10, h = h_gap + 8, center = true);
		
		cylinder(r = od_pipe / 2 + pad_pipe, h = h_gap + 25, center = true);
		
		cylinder(r = od_pipe / 2 + pad_pipe, h = 60);
	}
}

module jug_engagement() {
	union() {
		// jug engagement
//		translate([od_pipe / 2 + pad_pipe + 10, 0, 0]) {
//			sphere(r = (h_gap + 8) / 2);
			
			rotate([0, a_hopper, 0])
				cylinder(r = (h_gap + 8) / 2, h = 60);
//		}
	}
}

module windshield_hopper_old() {
	
		difference() {
			union() {
	//			cylinder(r = od_hopper_engagement / 2 + 5, h = 15);
			
	//			translate([0, 0, 5])
					rotate([0, a_hopper, 0]) {
						cylinder(r = od_hopper_engagement / 2, h = 45);
					
						translate([0, 0, -(d_hopper / 2 + pad_threads) * sin(a_hopper) - 1])
							cylinder(r = d_hopper / 2 + pad_threads, h = 35);
					}
			}

			translate([0, 0, -1])
				metric_thread(diameter = d_hopper, pitch = pitch_hopper, length = h_threads, internal = true, n_starts = 1);
//			cylinder(r = d_hopper / 2, h = h_threads);


				translate([0, 0, -20])
					cylinder(r = od_hopper_engagement / 2 + pad_threads + 10, h = 20);

				rotate([0, a_hopper, 0])
					translate([0, 0, 5]) {
						cylinder(r = od_hopper_engagement / 2 - t_hopper_walls, h = 45);

						translate([0, 0, -1])
							cylinder(r1 = d_hopper / 2 - 2, r2 = od_hopper_engagement / 2 - t_hopper_walls, h = 25);
					}
		}
}

module trailer_bearing_block() {
	difference() {
		hull()
			for (i = [-1, 1])
				for (j = [-1, 1])
					translate([i * cc_flange_mounts / 2, j * cc_flange_mounts / 2, 0])
						cylinder(r = d_flange_mounts / 2 + 4, h = h_outer_race, center = true);
	
			for (i = [-1, 1])
				for (j = [-1, 1])
					translate([i * cc_flange_mounts / 2, j * cc_flange_mounts / 2, 0])
						cylinder(r = d_flange_mounts / 2, h = h_outer_race + 1, center = true);

		translate([0, 0, 1])
			cylinder(r = d_outer_race / 2, h = h_outer_race, center = true);

		cylinder(r = d_inner / 2, h = h_outer_race + 1, center = true);
	}
}

module feed_chute() {
	difference() {
		union() {
			difference() {
				union() {
					hull() {
						cylinder(r = od_pipe_engagement / 2, h = h_pipe_engagement, center = true);

						translate([0, y_offset_hopper, z_offset_hopper + h_pipe_engagement / 4])
							cylinder(r = od_hopper_engagement / 2 + t_hopper_walls, h = h_pipe_engagement / 2, center = true);
					}
		//			translate([0, offset_hopper, -offset_hopper])
		//				hull() {
		//					cylinder(r = od_pipe_engagement / 2, h = h_pipe_engagement, center = true);
			
		//					translate([0, 30, 0])
		//						cube([l_hopper_base, w_hopper_base, h_pipe_engagement], center = true);
		//				}

					translate([0, y_offset_hopper, z_offset_hopper + h_pipe_engagement / 4 + 4]) {
						cylinder(r = od_hopper_engagement / 2 + t_hopper_walls, h = h_pipe_engagement / 2 + 8, center = true);

						translate([0, (od_hopper_engagement / 2 + t_hopper_walls + 8) / 2, 0])		
							cube([l_clamp, od_hopper_engagement / 2 + t_hopper_walls + 8, h_pipe_engagement / 2 + 8], center = true);
					}
					
					translate([0, -od_pipe_engagement / 2, 0])
						cube([l_clamp, od_hopper_engagement / 2 + t_hopper_walls, h_pipe_engagement], center = true);
				
	//				translate([0, -w_clamp / 2, 0])		
	//					cube([l_clamp, w_clamp, h_pipe_engagement], center = true);
			
	//				translate([0, od_pipe_engagement / 2 + y_offset_hopper / 2 + 2, (z_offset_hopper - h_pipe_engagement / 2) / 2 - 2])
	//					rotate([0, 90, 0])
	//						cylinder(r = 8, h = l_clamp, center = true);
				}
	
				cylinder(r = od_pipe / 2, h = h_pipe_engagement + 2 * z_offset_hopper, center = true);
	
		//		translate([0, offset_hopper, -offset_hopper])
		//			hull() {
		//				cylinder(r = l_gap / 2, h = h_gap, center = true);
		
		//				translate([0, 31, 0])
		//					cube([l_hopper_base - 2 * pad_pipe, w_hopper_base + 1, h_gap], center = true);
		//			}

				hull() {
					cylinder(r = l_gap / 2, h = h_gap, center = true);

					translate([0, y_offset_hopper, z_offset_hopper + h_gap / 4])
						cylinder(r = od_hopper_engagement / 2, h = h_gap / 2, center = true);
				}

				translate([0, y_offset_hopper, z_offset_hopper + h_pipe_engagement / 2])
					cylinder(r = od_hopper_engagement / 2, h = h_pipe_engagement, center = true);
	
				rotate([0, 90, 0])
					for (i = [-1, 1])
						translate([i * h_pipe_engagement / 3, -(od_pipe + pad_pipe) / 2 - 6, 0]) {
							cylinder(r = d_M3_screw / 2, h = od_pipe_engagement, center = true);

//							translate([0, 0, -pad_pipe - 7])
//							cylinder(r = d_M3_nut / 2 + 0.5, h = 12);
						}
			
					for (i = [-1, 1])
						translate([0, y_offset_hopper + (od_hopper_engagement / 2 + t_hopper_walls + 8) / 2 + 7, z_offset_hopper + h_gap / 4 + i * h_pipe_engagement / 6 + 6])		
							rotate([0, 90, 0])
								cylinder(r = d_M3_screw / 2, h = l_clamp + 1, center = true);
	
	//				translate([0, od_pipe_engagement / 2 + y_offset_hopper / 2 + 3, (z_offset_hopper - h_pipe_engagement) / 2 - 3])
	//					rotate([0, 90, 0])
	//						cylinder(r = d_M3_screw / 2, h = l_clamp + 1, center = true);
			}

			// supports:
	//		translate([0, 0, -h_pipe_engagement / 2])
	//			cylinder(r = od_pipe_engagement / 2, h = 0.5);
			
	//		translate([0, y_offset_hopper, z_offset_hopper + (h_gap + 9) / 2 + 5 - 0.5 / 2])
	//			cylinder(r = od_hopper_engagement / 2, h = 0.5, center = true);

		}
	
			translate([0, -(od_pipe_engagement + 3 * y_offset_hopper) / 2, -(h_pipe_engagement + 2 * z_offset_hopper) / 3])			
				cube([od_pipe_engagement, od_pipe_engagement + 3 * y_offset_hopper, h_pipe_engagement + 2 * z_offset_hopper]);
	}
}
