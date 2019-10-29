// *****************************************************************************
//
// Copyright (c) 2014, Southwest Research Institute® (SwRI®)
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

#include <string>

#include <cv_bridge/cv_bridge.h>
#include <image_transport/image_transport.h>
#include <opencv2/core/core.hpp>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>

namespace swri_image_util
{
  class DrawPolygonNode : public rclcpp::Node
  {
  public:
    explicit DrawPolygonNode(const rclcpp::NodeOptions& options) :
        rclcpp::Node("draw_polygon", options),
        thickness_(-1),
        r_(0),
        g_(0),
        b_(0)
    {
      this->get_parameter_or("thickness", thickness_, thickness_);
      this->get_parameter_or("r", r_, r_);
      this->get_parameter_or("g", g_, g_);
      this->get_parameter_or("b", b_, b_);

      std::map<std::string, std::vector<int64_t> > points;

      this->get_parameters("polygon", points);
      if (points.find("x") == points.end() ||
          points.find("y") == points.end() ||
          points["x"].size() != points["y"].size())
      {
        RCLCPP_FATAL(this->get_logger(), "'polygon' param must have an equal number of 'x' and 'y' points.");
      }
      std::vector<int64_t>& x_points = points["x"];
      std::vector<int64_t>& y_points = points["y"];
      for (size_t i = 0; i < x_points.size(); i++)
      {
        polygon_.emplace_back(cv::Point(x_points[i], y_points[i]));
      }

      auto callback = [this](const sensor_msgs::msg::Image::ConstSharedPtr& image) -> void
      {
        cv_bridge::CvImagePtr cv_image = cv_bridge::toCvCopy(image);

        const cv::Point* points[1] = {&polygon_[0]};
        int count = polygon_.size();

        if (thickness_ < 1)
        {
          cv::fillPoly(cv_image->image, points, &count, 1, cv::Scalar(b_, g_, r_));
        }
        else
        {
          cv::polylines(cv_image->image, points, &count, 1, true, cv::Scalar(b_, g_, r_), thickness_);
        }

        image_pub_.publish(cv_image->toImageMsg());
      };

      image_transport::ImageTransport it(shared_from_this());
      image_pub_ = it.advertise("image_out", 1);
      image_sub_ = it.subscribe("image_in", 1, callback);
    }

  private:
    int thickness_;
    int r_;
    int g_;
    int b_;

    std::vector<cv::Point> polygon_;

    image_transport::Subscriber image_sub_;
    image_transport::Publisher image_pub_;
  };
}

#include <rclcpp_components/register_node_macro.hpp>
RCLCPP_COMPONENTS_REGISTER_NODE(swri_image_util::DrawPolygonNode)