#ifndef LOCALPLANNER
#define LOCALPLANNER

#include <iostream>
#include <cmath>
#include "GCRobotics/simpleVelocity.h"
#include "GCRobotics/Pose_msg.h"
#include "ros/ros.h"
#define PI 3.14159265
using namespace std;

class LocalPlanner
{
private:
	int current_location[3]; // x, y, z  have a slot for z, but it will not be used
	float current_heading; // the read in value is in radians
	int current_velocity; // may not need velocity
	float new_heading;
	int destination[2]; //x, y

	// Speed control tables
	int table_distance[11];
	float table_heading[11];
	void initialize_tables();
	
	

	// Move to Point methods
	void get_current_location();
	void get_destination();
	void get_current_velocity(); // may not need velocity
	void calculate_new_heading();
		int calculate_quadrant();
		float calculate_theta(int);
	void resolve_heading();
		int check_heading_difference();
		void rotate_clockwise(float);
		void rotate_counterclockwise(float);
	void resolve_distance();
		int calculate_distance_difference();

	// Debugging functions
	void print_output();

public:

	// Published variables
	int velocity_out;
	float direction_out;
	/***********************
	0 = forward
	1 = right
	2 = backward
	3 = left
	4 = clockwise
	5 = counterclockwise
	6 = stop
	***********************/

	ros::NodeHandle n;
	ros::Subscriber CurrentPoseSub;
	ros::Subscriber NextPoseSub;
	ros::Publisher pub;
	void init(int argc, char **argv);
	void CurrentPositionCallback(const GCRobotics::Pose_msg::ConstPtr& msg);
	void NextPositionCallback(const GCRobotics::Pose_msg::ConstPtr& msg);
	
	
	LocalPlanner();
	void move_to_point();
	void halt();
};

#endif
