/************************************************************************************

filament_winder_renderer.scad - makes it easy to render various parts for the MOST filament extruder V10000
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

include<filament_winder.scad>

render_part(part_to_render = 17);

module render_part(part_to_render) {
	if (part_to_render == 3) sensor_mount();
	
	if (part_to_render == 4) fan_mount();
	
	if (part_to_render == 6) qc_diameter();
	
	if (part_to_render == 7) qc_length(outfeed = false);
	
	if (part_to_render == 8) qc_mount(assembly = false, mount_template = true);
	
	if (part_to_render == 9) sensor_shroud();
	
	if (part_to_render == 10) fan_shroud();
	
	if (part_to_render == 11) spool_drive_bearing_block();
	
	if (part_to_render == 12) gear_small();
	
	if (part_to_render == 13) gear_large();
	
	if (part_to_render == 14) idler_cone();
	
	if (part_to_render == 15) servo_mount();
	
	if (part_to_render == 16) thread_holder();
	
	if (part_to_render == 17) mainboard_mount();
	
	if (part_to_render == 18) filament_guide();
	
	if (part_to_render == 19) winder_motor_lock();

	if (part_to_render == 99) sandbox();
}


w_al_strip = 25.4 * 2;
t_al_strip = 25.4 / 8;
spool_drive_cc = 40; // spool drive gear c-c
n_teeth_small = 11;
n_teeth_large = 44;
t_gear = 12;
pitch_spool_threads = 6;
d_spool_threads = d_small_cone - 6;

cp = fit_spur_gears(n1 = n_teeth_small, n2 = n_teeth_large, spacing = spool_drive_cc);

module sandbox() {
	sensor_mount_prototype();
}

module filament_guide_arm() {
	cc_winder_arm = 100; // distance between servo pivot and filament guide hole
	d_servo_shaft = 6;
	d_filament_guide = 6;
	t_arm = 4;
	t_rib = 6;
	
	difference() {
		union() {
			translate([0, 0, t_arm])
				hull() {
					cube([cc_winder_arm, t_arm, 0.1], center = true);
				
					translate([0, 0, t_rib - t_arm / 2])
						rotate([0, 90, 0])
							cylinder(r = t_arm / 2, h = cc_winder_arm, center = true);
				}
			
			translate([0, 0, t_arm / 2])
				hull() {
					translate([-cc_winder_arm / 2, 0, 0])
						cylinder(r = d_servo_shaft / 2 + 8, h = t_arm, center = true);
					
					translate([cc_winder_arm / 2, 0, 0])
						cylinder(r = d_filament_guide / 2 + 3, h = t_arm, center = true);
				}
		}
		
		translate([-cc_winder_arm / 2, 0, 0]) {
			translate([0, 0, -1])
				cylinder(r = d_servo_shaft / 2, h = t_arm - 1);
			
			translate([0, 0, t_arm - 1 + layer_height])
				cylinder(r = d_M3_screw / 2, h = t_arm + 1);
			
			translate([0, 0, t_arm])
				cylinder(r = d_servo_shaft, h = t_rib + 1);
		}

		translate([cc_winder_arm / 2, 0, t_rib - 1])
			cylinder(r = d_filament_guide / 2, h = 2 * t_rib, center = true);
	}
}

module sensor_shroud_prototype() {
	h_shroud = 10;
	
	difference() {
		cube([13, 36, h_shroud]);
		
		translate([hole1[1], hole1[0], -1]) {
			cylinder(r = d_mounts / 2, h = h_shroud + 2);
			
			translate([0, 0, 4])
				cylinder(r = d_M3_cap / 2, h = h_shroud + 2);
		}

		translate([hole2[1], hole2[0], -1]) {
			cylinder(r = d_mounts / 2, h = h_shroud + 2);
			
			translate([0, 0, 4])
				cylinder(r = d_M3_cap / 2, h = h_shroud + 2);
		}
		
		translate([hole1[1] + 0.05 * 25.4, hole1[0] + 0.4 * 25.4, -2.8])
			for (i = [0:2])
				translate([0, i * 0.3 * 25.4, 0]) {
					cylinder(r = 3, h = h_shroud);
					
					translate([0, 0, h_shroud])
						sphere(r = 3);
				}
	}
}

module sensor_mount_prototype() {
	offset_board = 4; // offset sensor board so pins extend off edge of part
	difference() {
		union() {
			sensor_mount_body();
			
			translate([l_sensor_mount / 2 - 4, 3, (w_sensor_mount + w_sensor_board - offset_board - 1) / 2])
				cube([8, l_sensor_mount, w_sensor_board - offset_board + 1], center = true);
		}
		
		translate([l_sensor_mount / 2 - 4, 3, w_sensor_mount / 2 + w_sensor_board - 2 * offset_board])
			for (i = [-57.15 / 2, 57.15 / 2])
				translate([0, i, 0])
					rotate([0, 90, 0])
						cylinder(r = 1.05, h = l_light_path + 20, center = true);
	}
}

module sensor_mount_body() {
	difference() {
		translate([0, 3, 0])
			cube([l_sensor_mount, l_sensor_mount, w_sensor_mount], center = true);

		translate([0, (l_sensor_mount - l_sensor_board), 0])
			cube([l_light_path, l_sensor_mount, w_sensor_mount + 1], center = true);

		for (i = [-1, 1])
			translate([i * cc_strut_slots / 2, 3, 0])
				rotate([90, 0, 0])
					cylinder(r = d_sensor_threaded_rod / 2 + 0.25, h = l_sensor_mount + 1, center = true);
		}
}

module winder_motor_stay() {
	difference() {
		union() {
			difference() {
				rotate_extrude(convexity = 10)
					translate([(d_motor_mount_circle - 3 * d_mounts) / 2, 0, 0])
						square([3 * d_mounts, 4]);
			
				translate([0, -d_motor_mount_circle, -1])
					cube([2 * d_motor_mount_circle, 2 * d_motor_mount_circle, 8]);
			}

			for (i = [-1, 1])
				translate([0, i * d_motor_mount_circle / 2, 0])
					cylinder(r = 3 * d_mounts / 2, h = 4);
			
		}
		translate([0, 0, -1]) {
			difference() {
				rotate_extrude(convexity = 10)
					translate([(d_motor_mount_circle - d_mounts) / 2, 0, 0])
						square([d_mounts, 6]);
			
				translate([0, -d_motor_mount_circle, -1])
					cube([2 * d_motor_mount_circle, 2 * d_motor_mount_circle, 8]);
			}
			
			for (i = [-1, 1])
				translate([0, i * d_motor_mount_circle / 2, -1])
					cylinder(r = d_mounts / 2, h = 8);
		}
	}
}

module winder_motor_lock() {
	t_stay= 4;
	difference() {
		cube([30, 7, t_stay], center = true);
		
		for (j = [-1, 1])
		translate([j * 8.25, 0, 0])
			hull()
				for(i = [-2, 2])
					translate([i, 0, 0])
						cylinder(r = d_mounts / 2 + 0.51, h = t_stay + 1, center = true);
	}
}

module filament_guide() {
	difference() {
		hull() {
			translate([-(qc_bearings[0] + id_tubing)/ 2, 0, -qc_bearings[2] / 2])
				rotate([90, 0, 0])
					cylinder(r = od_tubing / 2 + pad_tubing, h = 40, center = true);
		
					translate([0, 0, -qc_bearings[2] / 2])
						cylinder(r = qc_bearings[0] / 2 - 1.5, h = od_tubing + 2 * pad_tubing, center = true);
		}
		
		// nut relief
		translate([-qc_bearings[0], 0, -2])
			hull() {
				cylinder(r = qc_bearings[0] / 2 + 0.5, h = od_tubing + 2 * pad_tubing + 2, center = true);
				
				translate([-20, 0, 0])
					cylinder(r = qc_bearings[0] / 2 + 10, h = od_tubing + 2 * pad_tubing + 2, center = true);
			}

		// filament path
		translate([-(qc_bearings[0] + id_tubing)/ 2, 0, -qc_bearings[2] / 2])
			rotate([90, 0, 0]) {
				hull()
					for (i = [-qc_max_displacement / 2, qc_max_displacement / 2])
						translate([i, 0, 0])
							cylinder(r = qc_max_displacement / 2 + 0.1, h = 41, center = true);
				
				// tubing engagement
				difference() {
					cylinder(r = od_tubing / 2, h = 41, center = true);
					
					cylinder(r = od_tubing / 2 + 1, h = 25, center = true);
				}
			}
		
		// idler bearing shaft
		cylinder(r = d_M5_screw / 2, h = 20, center = true);
		
		// idler bearing
		translate([0, 0, -10]) // change this position to alter where filament lands on bearings
			difference() {
				rotate([0, 0, 18])
					hull() {
						cylinder(r = qc_bearings[0] / 2 + 0.5, h = 11);
					
						translate([-20, 0, 0])
							cylinder(r = qc_bearings[0] / 2 + 5, h = 11);
					}
				
				translate([0, 0, 11])
					cylinder(r = qc_bearings[0] / 2 - 1.5, h = 2, center = true);
			}
		// clearance for pivot
		translate([-15, -20.50, -8])
			cube([10, 41, 10], center = true);
	}
}

module idler_cone() {
	difference() {
		union() {
			translate([0, 0, 15])
				cylinder(r1 = d_spool_threads / 2 - 6, r2 = d_large_cone / 2 + 2, h = 50);

			metric_thread(diameter = d_spool_threads - 1, pitch = pitch_spool_threads, length = 20, internal = false, n_starts = 1);
		}

		// taper the bottom so starting threads is easier
		translate([0, 0, -4])
			rotate_extrude(convexity = 10)
				translate([(d_spool_threads - 1) / 2, 0, 0])
					rotate([0, 0, 45])
						square([5, 5]);
		
		translate([0, 0, 19])
			difference() {
				union() {
					cylinder(r1 = d_spool_threads / 2 - 6, r2 = d_large_cone / 2 + 2, h = 50);

					translate([0, 0, -20])
						cylinder(r = d_spool_threads / 2 - 8, h = 26);
				}				
//			translate([0, 0, -1])
//				cylinder(r = 7.1, h = 25);
			
//			for (i = [0, 90])
//				rotate([0, 0, i])
				
				translate([-(d_large_cone + 1) / 2, -1, 0])
					cube([d_large_cone + 1, 2, 55]);
					
				translate([-1, -(d_large_cone + 1) / 2, 2 * layer_height])
					cube([2, d_large_cone + 1, 55]);
			}

	}
}

module gear_large() {
	or_large_gear = outer_radius(teeth = n_teeth_large, circular_pitch = cp);
	d_bore = 8.2;
	echo(str("Large gear diameter (mm) = ", 2 * or_large_gear));
	
	difference() {
		union() {
			gear (
				number_of_teeth=n_teeth_large,
				circular_pitch=cp, diametral_pitch=false,
				pressure_angle=28,
				clearance = 0.2,
				gear_thickness=t_gear,
				rim_thickness=t_gear,
				rim_width=5,
				hub_thickness=t_gear,
				hub_diameter=15,
				bore_diameter=0,
				circles=0,
				backlash=0,
				twist=0,
				involute_facets=0
			);
			
			// spool cone
			translate([0, 0, t_gear - 0.1])
				cylinder(r1 = d_large_cone / 2, r2 = d_small_cone / 2, h = h_cone);
		}
		
		// chamfer the gear
		for (i = [-3, t_gear - 1])
			translate([0, 0, i])
				rotate_extrude(convexity = 10)
					translate([or_large_gear, 0, 0])
						rotate([0, 0, 45])
							square([3, 3]);
		
		// pocket for magnet
		translate([od_608 / 2 + 15, 0, -1])
			cylinder(r = d_magnet / 2, h = 30);
		
		// relief for 608zz
		translate([0, 0, -1])
			cylinder(r = od_608 / 2 + 1, h = 3);
		
		// nut pocket and threaded liner
		translate([0, 0, 5]) {
			cylinder(r = d_M8_nut / 2, h = h_M8_nut + 1, $fn = 6);

			// shaft
			translate([0, 0, layer_height - 3])
				cylinder(r = 4.2, h = 3);
			
			translate([0, 0, h_M8_nut])
				metric_thread(diameter = d_spool_threads, pitch = pitch_spool_threads, length = 20, internal = true, n_starts = 1);
//				cylinder(r = d_spool_threads / 2, h = 20);
		}
	}
}

module gear_small(n_teeth = n_teeth_small) {
	or_small_gear = outer_radius(teeth = n_teeth, circular_pitch = cp);
	echo(str("Small gear diameter (mm) = ", 2 * or_small_gear));
	difference() {
		gear (
			number_of_teeth=n_teeth,
			circular_pitch=cp, diametral_pitch=false,
			pressure_angle=28,
			clearance = 0.2,
			gear_thickness=t_gear,
			rim_thickness=t_gear,
			rim_width=5,
			hub_thickness=t_gear + 3,
			hub_diameter=15,
			bore_diameter=6,
			circles=0,
			backlash=0,
			twist=0,
			involute_facets=0
		);
		
		// chamfer the gear
		for (i = [-2, t_gear - 1])
			translate([0, 0, i])
				rotate_extrude(convexity = 10)
					translate([or_small_gear, 0, 0])
						rotate([0, 0, 45])
							square([2, 2]);

		translate([0, 0, 12])
			rotate([0, -90, 0]) {
				cylinder(r = d_M3_screw / 2 - 0.4, h = 20);

//				translate([0, 0, 4.5])
//					hull()
//						for (i = [0, 10])
//							translate([i, 0, 0])
//								cylinder(r = d_M3_nut / 2, h = h_M3_nut, center = true, $fn = 6);
			}
	}
}

// two required
module spool_drive_bearing_block() {
	difference() {
		hull() {
			translate([w_al_strip / 2, 0, 0])
				cylinder(r = 6, h = 6, center = true);
		
			cylinder(r = od_608 / 2 + 3, h = 6, center = true);
		
			translate([-w_al_strip / 2, 0, 0])
				cylinder(r = 6, h = 6, center = true);
		}
		
		translate([0, 0, -4])
			cylinder(r = od_608 / 2, h = 5);

		translate([0, 0, 1 + layer_height])
			cylinder(r = 7, h = 10);
			
		translate([0, 0, 4])
			cube([w_al_strip, od_608 + 6, 4], center = true);
	}
}

d_fan_blade = 38;
d_fan_hub = 25;
h_fan_mount = d_sensor_threaded_rod + 8;
cc_fan_mounts = 32;
d_fan_shroud_outlet = d_tensioner + 4;
offset_fan_entry = 33;

module fillet(
	t_web,
	r_fillet,
	height,
	width,
	center = false)
{
	
	translate([0, r_fillet, 0])
		for (i = [-1, 1])
			translate([i * (r_fillet + t_web / 2), 0, 0])
				hull()
					for (j = [0, 1])
						translate([0, j * width, 0])
							cylinder(r = r_fillet, h = height, center = center);
}

module fan_shroud() {
	difference() {
		hull() {
			cube([40, 4, 40], center = true);
			
			translate([0, offset_fan_entry, 0])
				cylinder(r = d_fan_shroud_outlet / 2, h = 40, center = true);
		}

		translate([0, -10, 0])
			fillet(
				t_web = d_fan_shroud_outlet,
				r_fillet = 45,
				height = 41,
				width = offset_fan_entry,
				center = true);

		difference() {
			hull() {
				translate([0, -0.5, 0])
					cube([36, 5, 36], center = true);
			
				translate([0, offset_fan_entry, 0])
					cylinder(r = d_fan_shroud_outlet / 2 - 2, h = 36, center = true);
			}

			translate([0, -10, 0])
				fillet(
					t_web = d_fan_shroud_outlet - 4,
					r_fillet = 45,
					height = 37,
					width = offset_fan_entry,
					center = true);

			rotate([-90, 0, 0])
				for (i = [-1, 1])
					for (j = [-1, 1])
						translate([i * cc_fan_mounts / 2, j * cc_fan_mounts / 2, -4])
							hull()
								for (k = [0, 10])
									translate([i * k, 0, 0])
										cylinder(r = d_M3_screw / 2 + 2, h = 30);
			
			translate([0, offset_fan_entry, -18])
				cylinder(r = d_fan_shroud_outlet / 2, h = 4);

	
			// floor for upper mounts
			translate([0, 2, 20 - d_M3_screw - 4 - layer_height])
				cube([40, 8, 2 * layer_height], center = true);
		}
			
		rotate([-90, 0, 0])
			for (i = [-1, 1])
				for (j = [-1, 1])
					translate([i * cc_fan_mounts / 2, j * cc_fan_mounts / 2, 0]) {
						translate([0, 0, 3])
							cylinder(r = d_M3_cap / 2, h = 30);
						
						translate([0, 0, -5])
							cylinder(r = d_M3_screw / 2, h = 30);
					}

		translate([0, offset_fan_entry, 0]) {
			translate([0, 0, -21])
				cylinder(r = d_fan_shroud_outlet / 2 - 2, h = 8);
			
			translate([0, d_tensioner / 2 + 0.5, 6])
				cube([d_fan_shroud_outlet - 4, d_fan_shroud_outlet - 4 + 1, 40], center = true);
			
			translate([0, 0, 18 + layer_height])
				hull()
					for (i = [-1, 1])
						translate([0, i * 8, 0])
							cylinder(r = 10, h = 10);
		}
	}
}

module fan_mount() {
//	cylinder(r = 12.5, h = 30);
	difference() {
		union() {
			difference() {
				cube([l_sensor_mount, 40, d_sensor_threaded_rod + 8], center = true);
		
				translate([0, 0, h_fan_mount / 2 + 3])
					rotate_extrude(convexity = 10)
						translate([0.76 * (d_fan_blade - (d_fan_blade - d_fan_hub) / 2), 0, 0])
							scale([2, 1])
								circle(r = (d_fan_blade - d_fan_hub) / 2);
		
				for (i = [-1, 1])
					translate([i * cc_strut_slots / 2, 0, 0])
						rotate([90, 0, 0])
							cylinder(r = d_sensor_threaded_rod / 2 + 0.25, h = l_sensor_board + 13, center = true);

				translate([0, 0, -(d_sensor_threaded_rod + 8) / 2 - 1])
					cube([cc_strut_slots - d_sensor_threaded_rod - 8, 41, d_sensor_threaded_rod + 8], center = true);
			}
		
			for (i = [-1, 1])
				for (j = [-1, 1])
					translate([i * cc_fan_mounts / 2, j * cc_fan_mounts / 2, h_fan_mount / 2 - (d_fan_blade - d_fan_hub) / 2 + 2])
						cylinder(r = d_M3_screw / 2 + 2, h = (d_fan_blade - d_fan_hub) / 2 - 2);
		}
		
		for (i = [-1, 1])
			for (j = [-1, 1])
				translate([i * cc_fan_mounts / 2, j * cc_fan_mounts / 2, -1 + layer_height])
					cylinder(r = d_M3_screw / 2 - 0.15, h = h_fan_mount);
	}
}

