#!/usr/bin/env python
import roslib; roslib.load_manifest('GCRobotics')
import rospy
from std_msgs.msg import String
from GCRobotics.msg import i2cData

def talker():
	pub = rospy.Publisher('i2cSend',i2cData)
	rospy.init_node('talker')
	while not rospy.is_shutdown():
		msg = i2cData()
		msg.address = 0x05
		msg.messageType = 0x06
		msg.messageData = 0x07
		pub.publish(msg)
		rospy.sleep(1.0)

if __name__ == '__main__':
	try:
		talker()
	except rospy.ROSInterruptException:
		pass
