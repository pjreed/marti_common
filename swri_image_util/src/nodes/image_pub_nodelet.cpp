// *****************************************************************************
//
// Copyright (c) 2017, Southwest Research Institute® (SwRI®)
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of Southwest Research Institute® (SwRI®) nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// *****************************************************************************

#include <algorithm>

#include <boost/make_shared.hpp>

#include <cv_bridge/cv_bridge.h>
#include <image_transport/image_transport.h>
#include <image_transport/publisher.h>
#include <image_transport/subscriber.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <swri_roscpp/parameters.h>

namespace swri_image_util
{
class ImagePubNodelet : public rclcpp::Node
  {
    public:
      explicit ImagePubNodelet(const rclcpp::NodeOptions& options) :
        rclcpp::Node("image_pub", options)
      {
        std::string image_file;
        this->get_parameter_or("image_file", image_file, image_file);

        std::string mode;
        const std::string bgr8(sensor_msgs::image_encodings::BGR8);
        this->get_parameter_or("mode", mode, bgr8);

        double rate = 1;
        this->get_parameter_or("rate", rate, rate);
        rate = std::max(0.1, rate);

        cv_image.header.stamp = rclcpp::Clock().now();
        if (mode == sensor_msgs::image_encodings::BGR8)
        {
          cv_image.image = cv::imread(image_file, CV_LOAD_IMAGE_COLOR);
          cv_image.encoding = sensor_msgs::image_encodings::BGR8;
        }
        else
        {
          cv_image.image = cv::imread(image_file, CV_LOAD_IMAGE_GRAYSCALE);
          cv_image.encoding = sensor_msgs::image_encodings::MONO8;
        }

        if (!cv_image.image.empty())
        {
          image_transport::ImageTransport it(shared_from_this());
          image_pub_ = it.advertise("image", 2, true);
          pub_timer_ = this->create_wall_timer(
             std::chrono::duration<float>(1.0 / rate),
                 std::bind(&ImagePubNodelet::publish, this));
        }
        else
        {
          RCLCPP_FATAL(this->get_logger(), "Failed to load image.");
          rclcpp::shutdown();
        }
      }

      void publish()
      {
        cv_image.header.stamp = rclcpp::Clock().now();
        image_pub_.publish(cv_image.toImageMsg());
      }

    private:
      rclcpp::TimerBase::SharedPtr pub_timer_;
      image_transport::Publisher image_pub_;

      cv_bridge::CvImage cv_image;
  };
}

#include <rclcpp_components/register_node_macro.hpp>
RCLCPP_COMPONENTS_REGISTER_NODE(swri_image_util::ImagePubNodelet)