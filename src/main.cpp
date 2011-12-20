//stl includes
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

//ROS includes
#include <ros/ros.h>
#include <ros/common.h>
#include <nav_msgs/Path.h>
#include <geometry_msgs/PoseStamped.h>


void updatePathMsgs(int alg_id, long m_seconds, double x, double y, std::vector<nav_msgs::PathPtr> *message_vect)
{
        std::cout << "entering update function" << std::endl;
        geometry_msgs::PoseStamped new_pose_stamped;

        //setup timestamp
        std::cout << "update timestamp" << std::endl;
        ros::Time *new_timestamp = &(new_pose_stamped.header.stamp);
        new_timestamp->fromNSec(m_seconds * 10);

        //set up Pose
        std::cout << "update pose" << std::endl;
        geometry_msgs::Pose *new_pose = &(new_pose_stamped.pose);

        new_pose->position.x = x;
        new_pose->position.y = y;
        new_pose->position.z = 0;

        std::cout << "adding new pose to message vector" << std::endl;
        (*message_vect)[alg_id]->poses.push_back(new_pose_stamped);

}


int main(int argc, char **argv)
{
        //invalid value for coordinates
        static const double invalid = -1000;

        if(argc < 2)
        {
                std::cout << "you need to specify an input file" << std::endl;
                return -1;
        }

        std::ifstream data_file (argv[1], std::ifstream::in);

        std::stringstream ss(std::stringstream::in | std::stringstream::out);
        std::string line;

        std::vector<nav_msgs::PathPtr> message_vector;

        double x,y;
        unsigned int algorithm_id;
        long mikro_seconds;
        char dummy;

        //parse file line by line
        while(std::getline(data_file, line))
        {
                //reset stringstream
                ss.clear();
                ss.str("");

                //add new line to stringstream
                ss << line;

                //parse timestamp
                ss >> mikro_seconds;

                algorithm_id = 0;

                //parse point coordinates for each algorithm
                while(ss >> dummy >> x >> dummy >> y >> dummy)
                {
                        std::cout << "parsing point as \t algorithm id: " << algorithm_id << " timestamp: " << mikro_seconds << " x: " << x << " y: " << y << std::endl;

                        //increase message vector size if necessary
                        while(message_vector.size() <= algorithm_id)
                        {
                                std::cout << "increasing message vector size by 1" << std::endl;
                                message_vector.push_back(nav_msgs::PathPtr(new nav_msgs::Path));
                        }

                        //only add the point if it contains valid data
                        if((x != invalid) && (y != invalid))
                        {

                                std::cout << "the point is considered a valid point" << std::endl;
                                updatePathMsgs(algorithm_id, mikro_seconds, x, y, &message_vector);
                        }

                        ++algorithm_id;
                }
        }

        //close file stream
        data_file.close();

        //start ROS related activities
        ros::init(argc, argv, "pathmessenger");
        ros::NodeHandle node_handle;

        std::vector<ros::Publisher> publisher_vect;
        ros::Publisher pub;
        //create publisher for each entry in message_vector
        for(unsigned int i = 0; i < message_vector.size(); ++i)
        {
                ss.clear();
                ss.str("");
                ss << "algorithm" << i;

                pub = node_handle.advertise<nav_msgs::Path>(ss.str().c_str(), 100);
                publisher_vect.push_back(pub);

        }

        ros::Rate r(1); // spin at 1 Hz
        while (ros::ok())
        {
                //publish path messages
                for(unsigned int i = 0; i < publisher_vect.size(); ++i)
                {
                        publisher_vect[i].publish(*(message_vector[i]));
                }

                ros::spinOnce();
                r.sleep();
        }

    return 0;
}
