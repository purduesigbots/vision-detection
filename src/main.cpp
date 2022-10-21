#include "main.h"
#include "pros/llemu.hpp"
#include "pros/vision.h"
#include "visiondetect/classes.hpp"

/**
 * A callback function for LLEMU's center button.
 *
 * When this callback is fired, it will toggle line 2 of the LCD text between
 * "I was pressed!" and nothing.
 */
void on_center_button() {
	static bool pressed = false;
	pressed = !pressed;
	if (pressed) {
		pros::lcd::set_text(2, "I was pressed!");
	} else {
		pros::lcd::clear_line(2);
	}
}

/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */
void initialize() {
	printf("lkfjalfkjs\n");
	pros::lcd::initialize();
	pros::lcd::set_text(1, "Hello PROS User!");

	pros::lcd::register_btn1_cb(on_center_button);



}

/**
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competition Switch, following either autonomous or opcontrol. When
 * the robot is enabled, this task will exit.
 */
void disabled() {}

/**
 * Runs after initialize(), and before autonomous when connected to the Field
 * Management System or the VEX Competition Switch. This is intended for
 * competition-specific initialization routines, such as an autonomous selector
 * on the LCD.
 *
 * This task will exit when the robot is enabled and autonomous or opcontrol
 * starts.
 */
void competition_initialize() {}

/**
 * Runs the user autonomous code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the autonomous
 * mode. Alternatively, this function may be called in initialize or opcontrol
 * for non-competition testing purposes.
 *
 * If the robot is disabled or communications is lost, the autonomous task
 * will be stopped. Re-enabling the robot will restart the task, not re-start it
 * from where it left off.
 */
void autonomous() {}

/**
 * Runs the operator control code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the operator
 * control mode.
 *
 * If no competition control is connected, this function will run immediately
 * following initialize().
 *
 * If the robot is disabled or communications is lost, the
 * operator control task will be stopped. Re-enabling the robot will restart the
 * task, not resume it from where it left off.
 */

void increase() {
	int i=0;
	while(true) {
		pros::lcd::set_text(4, "Current Count: " + std::to_string(++i));
		pros::delay(50);
	}
}

void opcontrol() {
	pros::Task t(increase);
	visiondetect::Object yellow_disk = visiondetect::Object(
	pros::Vision::signature_from_utility(1, -369, 105, -132, -4493, -3939, -4216, 3.000, 0),
	1,
	5,
	52,
	50,
	0.15
	);

	visiondetect::Object yellow_goal = visiondetect::Object(
	pros::Vision::signature_from_utility(2, 1437, 1743, 1590, -3563, -3207, -3385, 3.000, 0),
	2,
	5,
	15,
	100,
	0.30
	);


	printf("Created objects\n");
	pros::delay(100);
	printf("kfjalsdjfsfa\n");
	visiondetect::Vision advanced_vision = visiondetect::Vision(
		1,
		5,
		2,
		5,
		false
		);
	printf("EEEEE\n");
	visiondetect::detected_object_s_t found_disk;
	visiondetect::detected_object_s_t found_goal;
	printf("made object arrays\n");
	pros::delay(1000);
	printf("Created vision object\n");
	while (true) {

		// copy output of visiondetect::Vision::find_objects() to found_disk
		printf("Finding yellow disks\n");
		bool found_yellow_disk = advanced_vision.detect_object(yellow_disk, &found_disk);
		if(found_yellow_disk)
			printf("Found Disk Data: { area: %d, x: %d, y: %d, width: %d, height: %d, apprx_distance: %f }\n", found_disk.area, found_disk.x_px, found_disk.y_px, found_disk.width, found_disk.height, found_disk.apprx_distance);
		else
			printf("No Disk Found\n");

		printf("Finding yellow goals\n");

		pros::delay(20);

		// copy output of visiondetect::Vision::find_objects() to found_goal
		/*
		bool found_yellow_goal = advanced_vision.detect_object(yellow_goal, &found_goal);
		
		if(found_yellow_goal)
			printf("Found Goal Data: { area: %d, x: %d, y: %d, width: %d, height: %d, apprx_distance: %f }\n", found_goal.area, found_goal.x_px, found_goal.y_px, found_goal.width, found_goal.height, found_goal.apprx_distance);
		else
			printf("No Goal Found\n");

		pros::lcd::set_text(0, "Disk: " + std::to_string(found_disk.area));
		pros::lcd::set_text(1, "Goal: " + std::to_string(found_goal.area));
	*/
		printf("lkajfljssssssssss\n");
		pros::delay(20);
		printf("dkdddddddddddddddddd\n");
	}
}
