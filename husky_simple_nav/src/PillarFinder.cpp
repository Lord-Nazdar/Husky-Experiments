#include "husky_simple_nav/PillarFinder.h"
#include <math.h>

PillarFinder::PillarFinder(const ros::Publisher &pubHusky, const ros::Publisher &pubRViz):
    PubHusky(pubHusky),
    PubRViz(pubRViz)
{
}

void PillarFinder::Monitor(const sensor_msgs::LaserScan::ConstPtr& msg) {
    uint16_t rangeID = GetClosestLaser(msg);

    // Get the angle between laser and closest hit
    double hitAngle = msg->angle_min + (rangeID * msg->angle_increment);

    geometry_msgs::Vector3 hitLocation = ExtractLaserHitLocation(msg->ranges[rangeID], hitAngle);

    geometry_msgs::Twist twistInfo;

    twistInfo.linear.x = 2;
    twistInfo.angular.z = atan2(hitLocation.y, hitLocation.x);

    PubHusky.publish(twistInfo);
}

uint16_t PillarFinder::GetClosestLaser(const sensor_msgs::LaserScan::ConstPtr& msg) {
    float rangeMin = msg->range_max;
    uint iMin = 0;
    for(uint i = 0; i < msg->ranges.size(); ++i){
        if(msg->ranges[i]<rangeMin) {
            rangeMin = msg->ranges[i];
            iMin = i;
        }
    }

    return iMin;
}

geometry_msgs::Vector3 PillarFinder::ExtractLaserHitLocation(const double range, const double angle) {
    geometry_msgs::Vector3 hitLocationScannerSpace;
    hitLocationScannerSpace.x = std::cos(angle) * range;
    hitLocationScannerSpace.y = std::sin(angle) * range;
    tf2_ros::TransformListener tfListener(tfBuffer);

    // Transform scanner space to base space
    geometry_msgs::Vector3 returnVal;
    geometry_msgs::TransformStamped ts;
    try {
        ts = tfBuffer.lookupTransform("base_link", "base_laser", ros::Time(0));

        returnVal.x = hitLocationScannerSpace.x + ts.transform.translation.x;
        returnVal.y = -hitLocationScannerSpace.y + ts.transform.translation.y;
    } catch(tf2::TransformException &ex) {
        ROS_WARN("%s", ex.what());
        ros::Duration(1.0).sleep();
    }

    return returnVal;
}
