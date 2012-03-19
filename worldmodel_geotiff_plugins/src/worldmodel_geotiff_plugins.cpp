//=================================================================================================
// Copyright (c) 2012, Johannes Meyer, TU Darmstadt
// All rights reserved.

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the Flight Systems and Automatic Control group,
//       TU Darmstadt, nor the names of its contributors may be used to
//       endorse or promote products derived from this software without
//       specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//=================================================================================================

#include <geotiff_writer/map_writer_interface.h>
#include <geotiff_writer/map_writer_plugin_interface.h>

#include <ros/ros.h>
#include <worldmodel_msgs/GetObjectModel.h>

#include <pluginlib/class_loader.h>

namespace worldmodel_geotiff_plugins {

using namespace hector_geotiff;

class VictimMapWriter : public MapWriterPluginInterface
{
public:
  virtual ~VictimMapWriter() {}

  virtual void initialize(const std::string& name);
  virtual void draw(MapWriterInterface *interface);

private:
  ros::NodeHandle nh_;
  ros::ServiceClient service_client_;

  bool draw_all_objects_;
  std::string class_id_;
};

void VictimMapWriter::initialize(const std::string& name)
{
  ros::NodeHandle plugin_nh("~/" + name);
  std::string service_name_;

  plugin_nh.param("service_name", service_name_, std::string("worldmodel/get_object_model"));
  plugin_nh.param("draw_all_objects", draw_all_objects_, false);
  plugin_nh.param("class_id", class_id_, std::string());

  service_client_ = nh_.serviceClient<worldmodel_msgs::GetObjectModel>(service_name_);
}

void VictimMapWriter::draw(MapWriterInterface *interface)
{
  worldmodel_msgs::GetObjectModel data;

  if (!service_client_.call(data)) {
    ROS_ERROR_NAMED("VictimMapWriter", "Cannot draw victims, service %s failed", service_client_.getService().c_str());
    return;
  }

  int counter = 0;
  for(worldmodel_msgs::ObjectModel::_objects_type::const_iterator it = data.response.model.objects.begin(); it != data.response.model.objects.end(); ++it) {
    const worldmodel_msgs::Object& object = *it;
    if (!draw_all_objects_ && object.state.state != worldmodel_msgs::ObjectState::CONFIRMED) continue;
    if (!class_id_.empty() && object.info.class_id != class_id_) continue;

    Eigen::Vector2f coords;
    coords.x() = object.pose.pose.position.x;
    coords.y() = object.pose.pose.position.y;
    interface->drawVictim(coords, ++counter);
  }
}

} // namespace

//register this planner as a MapWriterPluginInterface plugin
#include <pluginlib/class_list_macros.h>
PLUGINLIB_DECLARE_CLASS(worldmodel_geotiff_plugins, VictimMapWriter, worldmodel_geotiff_plugins::VictimMapWriter, hector_geotiff::MapWriterPluginInterface)
