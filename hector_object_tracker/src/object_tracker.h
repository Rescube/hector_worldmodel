#ifndef OBJECT_TRACKER_OBJECT_TRACKER_H
#define OBJECT_TRACKER_OBJECT_TRACKER_H

#include <ros/ros.h>
#include <std_msgs/String.h>
#include <std_msgs/Float32.h>
#include <worldmodel_msgs/ImagePercept.h>
#include <worldmodel_msgs/PosePercept.h>
#include <worldmodel_msgs/SetObjectState.h>
#include <worldmodel_msgs/SetObjectName.h>
#include <worldmodel_msgs/AddObject.h>
#include <worldmodel_msgs/GetObjectModel.h>

#include <tf/transform_listener.h>
#include <image_geometry/pinhole_camera_model.h>

#include <hector_marker_drawing/HectorDrawings.h>

#include "ObjectModel.h"

namespace hector_object_tracker {

class ObjectTracker {
public:
  ObjectTracker();
  virtual ~ObjectTracker();

protected:
  void sysCommandCb(const std_msgs::StringConstPtr &);
  void imagePerceptCb(const worldmodel_msgs::ImagePerceptConstPtr &);
  void posePerceptCb(const worldmodel_msgs::PosePerceptConstPtr &);
  void objectAgeingCb(const std_msgs::Float32ConstPtr &);

  void modelUpdateCb(const worldmodel_msgs::ObjectModelConstPtr &);

  bool setObjectStateCb(worldmodel_msgs::SetObjectState::Request& request, worldmodel_msgs::SetObjectState::Response& response);
  bool setObjectNameCb(worldmodel_msgs::SetObjectName::Request& request, worldmodel_msgs::SetObjectName::Response& response);
  bool addObjectCb(worldmodel_msgs::AddObject::Request& request, worldmodel_msgs::AddObject::Response& response);
  bool getObjectModelCb(worldmodel_msgs::GetObjectModel::Request& request, worldmodel_msgs::GetObjectModel::Response& response);

  ObjectModel& getModel()             { return model; }
  const ObjectModel& getModel() const { return model; }

  void publishModelEvent(const ros::TimerEvent&);
  void publishModel();

protected:
  bool transformPose(const geometry_msgs::Pose& from, geometry_msgs::Pose &to, std_msgs::Header &header, tf::StampedTransform *transform = 0);
  bool transformPose(const geometry_msgs::PoseWithCovariance& from, geometry_msgs::PoseWithCovariance &to, std_msgs::Header &header);
  bool mapToNextObstacle(const geometry_msgs::Pose& source, const std_msgs::Header &header, const worldmodel_msgs::ObjectInfo &info, geometry_msgs::Pose &mapped);

private:
  ros::NodeHandle nh;
  ros::NodeHandle priv_nh;
  ros::Subscriber imagePerceptSubscriber;
  ros::Subscriber posePerceptSubscriber;
  ros::Subscriber sysCommandSubscriber;
  ros::Subscriber objectAgeingSubscriber;

  ros::Publisher modelPublisher;
  ros::Publisher modelUpdatePublisher;
  ros::Publisher poseDebugPublisher;
  ros::Publisher pointDebugPublisher;
  ros::Subscriber modelUpdateSubscriber;

  ros::ServiceServer setObjectState;
  ros::ServiceServer addObject;
  ros::ServiceServer getObjectModel;
  ros::ServiceServer setObjectName;

  ros::Timer publishTimer;

  typedef std::pair<ros::ServiceClient,XmlRpc::XmlRpcValue> VerificationService;
  std::map<std::string,std::map<std::string,std::vector<VerificationService> > > verificationServices;

  tf::TransformListener tf;

  HectorDrawings drawings;

  ObjectModel model;
  typedef boost::shared_ptr<image_geometry::PinholeCameraModel> CameraModelPtr;
  std::map<std::string,CameraModelPtr> cameraModels;

  struct MergedModelData {
    std::string prefix;
    ObjectModel model;
    ros::Subscriber subscriber;
  };
  std::vector<MergedModelData> merged_models;
  void mergeModelCallback(const worldmodel_msgs::ObjectModelConstPtr &new_model, MergedModelData& model);

  std::string _frame_id;
  std::string _worldmodel_ns;

  double _ageing_threshold;
  double _publish_interval;
};

} // namespace hector_object_tracker

#endif // OBJECT_TRACKER_OBJECT_TRACKER_H
