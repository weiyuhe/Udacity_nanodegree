#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    // TODO: Request a service and pass the velocities to it to drive the robot
    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;

    if(!client.call(srv)){
        ROS_ERROR("Failed to call ball_chaser/command_robot!");
    }
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{

    int white_pixel = 255;
    int imgStep = img.step;
    int imgHeight = img.height;
    int imgWidth = img.width;
    int image_data = 0;
    bool found = false;


/*# This message contains an uncompressed image
# (0, 0) is at top-left corner of image
#

Header header        # Header timestamp should be acquisition time of image
                     # Header frame_id should be optical frame of camera
                     # origin of frame should be optical center of camera
                     # +x should point to the right in the image
                     # +y should point down in the image
                     # +z should point into to plane of the image
                     # If the frame_id here and the frame_id of the CameraInfo
                     # message associated with the image conflict
                     # the behavior is undefined

uint32 height         # image height, that is, number of rows
uint32 width          # image width, that is, number of columns

# The legal values for encoding are in file src/image_encodings.cpp
# If you want to standardize a new string format, join
# ros-users@lists.sourceforge.net and send an email proposing a new encoding.

string encoding       # Encoding of pixels -- channel meaning, ordering, size
                      # taken from the list of strings in include/sensor_msgs/image_encodings.h

uint8 is_bigendian    # is this data bigendian?
uint32 step           # Full row length in bytes
uint8[] data          # actual matrix data, size is (step * rows)  */

    // TODO: Loop through each pixel in the image and check if there's a bright white one
    // Then, identify if this pixel falls in the left, mid, or right side of the image
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera

    //find location of the white pixels
    int curr_pixel = 0;
    for(int i =0; i< imgHeight && !found; i++){
        for(int j = 0; j < imgStep && !found; j+=3){
            image_data = i*imgStep + j;
            if(img.data[image_data] == white_pixel && img.data[image_data+1] == white_pixel &&img.data[image_data+2] == white_pixel){
                curr_pixel = j/3;
                found = true;
                break;
            }

        }
        
    }

    float x = 0.0;
    float beta = 0.0;

    if(curr_pixel < imgWidth/3 && found){ 
        x = 0.2;
        beta = 0.2;
        ROS_INFO("Left");

    }else if(curr_pixel >= imgWidth/3 && curr_pixel < 2*imgWidth/3 && found){
        x = 0.4;
        beta = 0.0;
        ROS_INFO("Mid");

    }else if(curr_pixel >= 2*imgWidth/3 && found ){ //curr_pixel >= 2*imgWidth/3
        x = 0.2;
        beta = -0.2;
        ROS_INFO("Right");

    }else{
        x = 0.0;
        beta = 0.0;
        ROS_INFO("Not Found");
    }

    drive_robot(x,beta);

    

}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}