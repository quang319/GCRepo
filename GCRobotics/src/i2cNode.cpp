#include "ros/ros.h"
#include "std_msgs/String.h"
#include "GCRobotics/i2cData.h"
#include "/home/gcr/GCRepo/roboio/Include/roboard.h"
#include <iostream>

using namespace std;


void i2cSend(const GCRobotics::i2cData::ConstPtr&);

ros::Publisher pub;

void i2cSendCallback(const GCRobotics::i2cData::ConstPtr& msg)
{
	// Roboard i2cSend functions here
	ROS_INFO("I heard things");
	cout << "Here\n";
	std_msgs::String stringmsg;
	stringmsg.data = "i2cDataIsHere";
	pub.publish(stringmsg);
	if (msg->expectDataBack == 1)
	{
		// Roboard i2cRead functions here, put result in pub.publish()
		pub.publish(stringmsg);
	}
	else
	{
		i2cSend(msg);
	}
	
	return;
}


int main(int argc, char **argv)
{
	roboio_SetRBVer(RB_100RD);
	ros::init(argc, argv,"i2cNode");
	ros::NodeHandle n;
	ros::Subscriber sub = n.subscribe("i2cSend", 100, i2cSendCallback);
	//i2c_Init(I2CMODE_AUTO, 1000);
	pub = n.advertise<std_msgs::String>("i2cReceived",100);
		
	ros::spin();

	return 0;
}

void i2cSend(const GCRobotics::i2cData::ConstPtr& msg)
{
	if (i2c_Init(I2CMODE_AUTO, 1000) == false)
	{
		ROS_INFO("%s", roboio_GetErrMsg());
		return;	
	}
	unsigned char buf[2] = {msg->messageType, msg->messageData};

	i2c_Send(0x10,buf, 2);

	i2c_Close();
	return;	
}
