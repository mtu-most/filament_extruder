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

render_part(part_to_render = 99);

module render_part(part_to_render) {
	if (part_to_render == 2) qc_length_rotor();

	if (part_to_render == 3) sensor_mount();

	if (part_to_render == 4) fan_mount();

	if (part_to_render == 6) qc_diameter();

	if (part_to_render == 7) qc_length(outfeed = false, encoder = false);

	if (part_to_render == 8) qc_mount(assembly = false, mount_template = true);

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

module sandbox() {
	difference() {
		union() {
			// bracket
			for (j = [-1, 1])
				translate([0, j * (l_sensor + pad_sensor) / 2, 0])
					hull() {
						translate([-cc_strut_slots / 2, 0, 0])
							rotate([90, 0, 0])
								cylinder(r = d_sensor_threaded_rod / 2 + 4, h = pad_sensor, center = true);
				
						translate([-(l_light_path - t_sensor_mount) / 2, 0, offset_sensor - (w_sensor / 2 + pad_sensor) + 0.5])
							cube([t_sensor + t_sensor_mount, pad_sensor, 1], center = true);

						translate([-cc_strut_slots / 2 + d_sensor_threaded_rod / 2 + 2 , 0, offset_sensor - (w_sensor / 2 + pad_sensor) + 0.5])
							cube([4, pad_sensor, 1], center = true);
					}

			for (j = [-1, 1])
				translate([0, j * (l_sensor + pad_sensor) / 2, 0])
					hull() {
						translate([cc_strut_slots / 2, 0, 0])
							rotate([90, 0, 0])
								cylinder(r = d_sensor_threaded_rod / 2 + 4, h = pad_sensor, center = true);
				
							translate([(l_light_path - t_light_source_mount / 2) / 2, 0, offset_sensor - (w_sensor + 2 * pad_sensor + 3 * l_light_path * w_slot / t_sensor_mount) / 2 + 0.5])
								cube([t_light_source_mount, pad_sensor, 1], center = true);

							translate([cc_strut_slots / 2 - d_sensor_threaded_rod / 2 - 2, 0, offset_sensor])
								cube([4, pad_sensor, 1], center = true);
					}

			for (i = [-1, 1])
				translate([i * cc_strut_slots / 2, 0, 0])
					rotate([90, 0, 0])
						cylinder(r = d_sensor_threaded_rod / 2 + 4, h = l_sensor + 2 * pad_sensor, center = true);

			translate([0, 0, offset_sensor])
				difference() {
					hull() {
						translate([-(l_light_path + t_sensor) / 2, 0, 0])
							cube([0.05, l_sensor + 2 * pad_sensor, w_sensor + 2 * pad_sensor], center = true);
			
						translate([(l_light_path + t_sensor) / 2, 0, 0])
							cube([0.05, l_sensor + 2 * pad_sensor, w_sensor + 2 * pad_sensor + 3 * l_light_path * w_slot / t_sensor_mount], center = true);
					}
		
					translate([t_sensor_mount - t_light_source_mount, 0, 0])
						cube([l_light_path - t_sensor_mount - t_light_source_mount, l_sensor, w_sensor + 2 * pad_sensor + 3 * l_light_path * w_slot / t_sensor_mount], center = true);
		
					translate([-(l_light_path + t_sensor) / 2, 0, 0]) {
						// sensor pocket
						cube([2 * t_sensor, l_sensor, w_sensor], center = true);

						translate([t_sensor_mount + t_sensor - 1, 0, offset_sensor_centerline]) {
							// slot for light
							cube([2 * t_sensor_mount, l_slot, w_slot], center = true);

							// sensor mounting holes
							for (i = [-1, 1])
								translate([-t_sensor_mount, i * cc_sensor_mounts / 2, 0])
									rotate([0, 90, 0, 0])
										cylinder(r = d_sensor_mounts / 2, h = t_sensor_mount);
							}
					}
		
					// light source
					translate([(l_light_path + t_sensor) / 2, 0, 0])
						rotate([0, 90, 0]) {
							translate([0, 0, 1])
								cylinder(r = d_light_source / 2, h = t_light_source_mount, center = true);

							cylinder(r = d_light_source_pinhole / 2, h = t_light_source_mount + 1, center = true);
						}
				}
		}

		// threaded rod openings
		for (i = [-1, 1])
			translate([i * cc_strut_slots / 2, 0, 0])
				rotate([90, 0, 0])
					cylinder(r = d_sensor_threaded_rod / 2, h = l_sensor + 2 * pad_sensor + 1, center = true);
	}
}
