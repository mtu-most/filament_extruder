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

include<filament_extruder.scad>

render_part(part_to_render = 99);

module render_part(part_to_render) {
	if (part_to_render == 1) display_mount();

	if (part_to_render == 2) qc_length_rotor();

	if (part_to_render == 3) sensor_mount();

	if (part_to_render == 4) fan_mount();

	if (part_to_render == 6) qc_diameter();

	if (part_to_render == 7) qc_length(outfeed = false, encoder = false);

	if (part_to_render == 8) qc_mount(assembly = false, mount_template = true);

	if (part_to_render == 9) encoder_knob();

	if (part_to_render == 10) fan_shroud();

	if (part_to_render == 11) spool_drive_bearing_block();

	if (part_to_render == 12) gear_small();

	if (part_to_render == 13) gear_large();

	if (part_to_render == 14) idler_cone();

	if (part_to_render == 15) servo_mount();

	if (part_to_render == 16) thread_holder();

	if (part_to_render == 17) mainboard_mount();

	if (part_to_render == 18) filament_guide();

	if (part_to_render == 19) winder_motor_stay();
	
	if (part_to_render == 20) filament_guide_arm();
	
	if (part_to_render == 21) bearing_bushing();
	
	if (part_to_render == 22) trailer_bearing_block();
	
	if (part_to_render == 23) feed_chute();
	
	if (part_to_render == 24) jug_adapter();

	if (part_to_render == 99) sandbox();
}

module sandbox() {
	rotate([a_hopper, 0, 0])
	difference() {
			difference() {
				hull() {
					cylinder(r = od_pipe_engagement / 2, h = h_pipe_engagement, center = true);

					translate([0, y_offset_hopper, z_offset_hopper + h_pipe_engagement / 4])
						rotate([-a_hopper, 0, 0])
							cylinder(r = d_hopper / 2 + pad_threads, h = h_threads);
				}
	
				// barrel opening
				cylinder(r = od_pipe / 2, h = h_pipe_engagement + 2 * z_offset_hopper, center = true);

				// interior chute
				hull() {
					cylinder(r = l_gap / 2, h = h_gap, center = true);

					translate([0, y_offset_hopper, z_offset_hopper + h_pipe_engagement / 4])
						rotate([-a_hopper, 0, 0])
							cylinder(r = d_hopper / 2 - 2, h = h_threads);
				}

				translate([0, y_offset_hopper, z_offset_hopper + h_pipe_engagement / 4])
					rotate([-a_hopper, 0, 0])
						translate([0, 0, 0.1])
							metric_thread(diameter = d_hopper, pitch = pitch_hopper, length = h_threads, internal = true, n_starts = 1);
//							cylinder(r = d_hopper / 2, h = h_threads);
				
				// screw two halves together
				for (i = [-1, 1], j = [-1, 1])
					translate([i * (od_pipe + od_pipe_engagement) / 4, 0, j * h_pipe_engagement / 3])
						rotate([90, 0, 0]) {
							translate([0, 0, -13])
								cylinder(r = d_M3_screw / 2 - 0.16, h = 30);
							
							cylinder(r = d_M3_screw / 2 + 0.2, h = 20);

							translate([0, 0, 5])
								cylinder(r = d_M3_cap / 2 + 0.5, h = 16);
							}
			}
		
		translate([0, -40, 0])
			cube([80, 80, 300], center = true);
	}

}
