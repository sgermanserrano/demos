// Copyright 2017 Open Source Robotics Foundation, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_components/register_node_macro.hpp"

#include "rcutils/cmdline_parser.h"
#include "rcutils/snprintf.h"

#include "std_msgs/msg/string.hpp"

#include "rmw/serialized_message.h"

#include "demo_nodes_cpp/visibility_control.h"

using namespace std::chrono_literals;

namespace demo_nodes_cpp
{

class SerializedMessageTalker : public rclcpp::Node
{
public:
  DEMO_NODES_CPP_PUBLIC
  explicit SerializedMessageTalker(const rclcpp::NodeOptions & options)
  : Node("serialized_message_talker", options)
  {
    // In this example we send serialized data (serialized data).
    // For this we initially allocate a container message
    // which can hold the data.
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);
    std::vector<std::string> args = options.arguments();
    if (find_command_option(args, "-h")) {
      print_usage();
      rclcpp::shutdown();
    } else {
      std::string tmptopic = get_command_option(args, "-t");
      if (!tmptopic.empty()) {
        topic_name_ = tmptopic;
      }
      serialized_msg_ = rmw_get_zero_initialized_serialized_message();
      auto allocator = rcutils_get_default_allocator();
      auto initial_capacity = 0u;
      auto ret = rmw_serialized_message_init(
        &serialized_msg_,
        initial_capacity,
        &allocator);
      if (ret != RCL_RET_OK) {
        throw std::runtime_error("failed to initialize serialized message");
      }

      // Create a function for when messages are to be sent.
      auto publish_message =
        [this]() -> void
        {
          // In this example we send a std_msgs/String as serialized data.
          // This is the manual CDR serialization of a string message with the content of
          // Hello World: <count_> equivalent to talker example.
          // The serialized data is composed of a 8 Byte header
          // plus the length of the actual message payload.
          // If we were to compose this serialized message by hand, we would execute the following:
          // rcutils_snprintf(
          //   serialized_msg_.buffer, serialized_msg_.buffer_length, "%c%c%c%c%c%c%c%c%s %zu",
          //   0x00, 0x01, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, "Hello World:", count_++);

          // In order to ease things up, we call the rmw_serialize function,
          // which can do the above conversion for us.
          // For this, we initially fill up a std_msgs/String message and fill up its content
          auto string_msg = std::make_shared<std_msgs::msg::String>();
          string_msg->data = "Hello World:" + std::to_string(count_++);

          // We know the size of the data to be sent, and thus can pre-allocate the
          // necessary memory to hold all the data.
          // This is specifically interesting to do here, because this means
          // no dynamic memory allocation has to be done down the stack.
          // If we don't allocate enough memory, the serialized message will be
          // dynamically allocated before sending it to the wire.
          auto message_header_length = 8u;
          auto message_payload_length = static_cast<size_t>(string_msg->data.size());
          auto ret = rmw_serialized_message_resize(
            &serialized_msg_, message_header_length + message_payload_length);
          if (ret != RCL_RET_OK) {
            throw std::runtime_error("failed to resize serialized message");
          }

          auto string_ts =
            rosidl_typesupport_cpp::get_message_type_support_handle<std_msgs::msg::String>();
          // Given the correct typesupport, we can convert our ROS2 message into
          // its binary representation (serialized_msg)
          ret = rmw_serialize(string_msg.get(), string_ts, &serialized_msg_);
          if (ret != RMW_RET_OK) {
            fprintf(stderr, "failed to serialize serialized message\n");
            return;
          }

          // For demonstration we print the ROS2 message format
          printf("ROS message:\n");
          printf("%s\n", string_msg->data.c_str());
          // And after the corresponding binary representation
          printf("serialized message:\n");
          for (size_t i = 0; i < serialized_msg_.buffer_length; ++i) {
            printf("%02x ", serialized_msg_.buffer[i]);
          }
          printf("\n");

          pub_->publish(serialized_msg_);
        };

      rclcpp::QoS qos(rclcpp::KeepLast(7));
      pub_ = this->create_publisher<std_msgs::msg::String>(topic_name_, qos);

      // Use a timer to schedule periodic message publishing.
      timer_ = this->create_wall_timer(1s, publish_message);
    }
  }

  DEMO_NODES_CPP_PUBLIC
  ~SerializedMessageTalker()
  {
    auto ret = rmw_serialized_message_fini(&serialized_msg_);
    if (ret != RCL_RET_OK) {
      fprintf(stderr, "could not clean up memory for serialized message");
    }
  }

private:
  DEMO_NODES_CPP_LOCAL
  void print_usage()
  {
    printf("Usage for talker app:\n");
    printf("talker [-t topic_name] [-h]\n");
    printf("options:\n");
    printf("-h : Print this help function.\n");
    printf("-t topic_name : Specify the topic on which to publish. Defaults to chatter.\n");
  }

  DEMO_NODES_CPP_LOCAL
  bool find_command_option(const std::vector<std::string> & args, const std::string & option)
  {
    return std::find(args.begin(), args.end(), option) != args.end();
  }

  DEMO_NODES_CPP_LOCAL
  std::string get_command_option(const std::vector<std::string> & args, const std::string & option)
  {
    auto it = std::find(args.begin(), args.end(), option);
    if (it != args.end() && ++it != args.end()) {
      return *it;
    }
    return std::string();
  }

  size_t count_ = 1;
  rcl_serialized_message_t serialized_msg_;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_;
  rclcpp::TimerBase::SharedPtr timer_;
  std::string topic_name_ = "chatter";
};

}  // namespace demo_nodes_cpp

RCLCPP_COMPONENTS_REGISTER_NODE(demo_nodes_cpp::SerializedMessageTalker)
