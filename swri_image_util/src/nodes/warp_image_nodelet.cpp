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

#include <cv_bridge/cv_bridge.h>
#include <image_transport/image_transport.h>
#include <image_transport/publisher.h>
#include <image_transport/subscriber.h>
#include <opencv2/core/core.hpp>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <swri_roscpp/parameters.h>

namespace swri_image_util
{
class WarpImageNodelet : public rclcpp::Node
  {
    public:
      explicit WarpImageNodelet(const rclcpp::NodeOptions& options) :
        rclcpp::Node("warp_image", options),
        use_input_size_(false)
      {
        std::vector<double> transform;
        if (this->has_parameter("width") && this->has_parameter("height"))
        {
          use_input_size_ = false;
          this->get_parameter("width", output_size_.width);
          this->get_parameter("height", output_size_.height);
        }
        else
        {
          use_input_size_ = true;
          RCLCPP_INFO(this->get_logger(),
              "No ~width and ~height parameters given. Output images will be same size as input.");
        }
        this->get_parameter_or("transform", transform, transform);
        if (transform.size() != 9)
        {
          RCLCPP_FATAL(this->get_logger(),
              "~transform must be a 9-element list of doubles (3x3 matrix, row major)");
          // Return without setting up callbacks
          // Don't shut down, because that would bring down all other nodelets as well
          return;
        }
        m_ = cv::Mat(transform, true).reshape(0, 3);
        std::stringstream matstring;
        matstring << m_;
        RCLCPP_INFO(this->get_logger(), "Transformation matrix: %s", matstring.str().c_str());

        auto callback = [this](const sensor_msgs::msg::Image::ConstSharedPtr& image) -> void {
          cv_bridge::CvImageConstPtr cv_image = cv_bridge::toCvShare(image);

          cv_bridge::CvImagePtr cv_warped = std::make_shared<cv_bridge::CvImage>();
          if (use_input_size_)
          {
            output_size_ = cv_image->image.size();
          }
          cv::warpPerspective(cv_image->image, cv_warped->image, m_, output_size_, CV_INTER_LANCZOS4);

          cv_warped->encoding = cv_image->encoding;
          cv_warped->header = cv_image->header;

          image_pub_.publish(cv_warped->toImageMsg());
        };

        image_transport::ImageTransport it(shared_from_this());
        image_pub_ = it.advertise("warped_image", 1);
        image_sub_ = it.subscribe("image", 1, callback);
      }

    private:
      image_transport::Subscriber image_sub_;
      image_transport::Publisher image_pub_;
      cv::Mat m_;
      bool use_input_size_;
      cv::Size output_size_;
  };
}

#include <rclcpp_components/register_node_macro.hpp>
RCLCPP_COMPONENTS_REGISTER_NODE(swri_image_util::WarpImageNodelet)