cmake_minimum_required(VERSION 2.8.3)
project(swri_image_util)

find_package(catkin REQUIRED COMPONENTS 
  roscpp
  rospy
  roslib
  std_msgs
  nodelet
  cv_bridge
  tf
  cmake_modules
  geometry_msgs
  image_geometry
  nav_msgs
  camera_calibration_parsers
  swri_math_util
  swri_opencv_util
  image_transport
)

find_package(OpenCV)
find_package(Qt4 REQUIRED)
find_package(Boost REQUIRED COMPONENTS filesystem system random)

# The Boost Random library headers and namespaces changed between version
# 1.46 and 1.47
if(${Boost_LIB_VERSION} MATCHES "1_46*")
  add_definitions( -DBOOST_1_46 )
endif(${Boost_LIB_VERSION} MATCHES "1_46*")

find_package(Eigen REQUIRED)
add_definitions(${EIGEN_DEFINITIONS})

include(${QT_USE_FILE})

catkin_package(
  INCLUDE_DIRS include
  LIBRARIES ${PROJECT_NAME}
  CATKIN_DEPENDS 
    roscpp
    roscpp
    rospy
    roslib
    std_msgs
    nodelet
    cv_bridge
    tf
    geometry_msgs
    image_geometry
    nav_msgs
    camera_calibration_parsers
    swri_math_util
    swri_opencv_util
    image_transport
)

include_directories(include ${catkin_INCLUDE_DIRS} ${QT_INCLUDE_DIR} ${OpenCV_INCLUDE_DIRS} ${EIGEN_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})

add_library(${PROJECT_NAME} 
  src/image_normalization.cpp 
  src/rolling_normalization.cpp 
  src/image_matching.cpp
  src/draw_util.cpp
  src/geometry_util.cpp
  src/image_warp_util.cpp)
target_link_libraries(${PROJECT_NAME} ${catkin_LIBRARIES} ${QT_LIBRARIES} ${OpenCV_LIBRARIES} ${Boost_LIBRARIES})

add_library(${PROJECT_NAME}_nodelets
  src/nodelets/rotate_image_nodelet.cpp
  src/nodelets/contrast_stretch_nodelet.cpp
  src/nodelets/normalize_response_nodelet.cpp
  src/nodelets/scale_image_nodelet.cpp
  src/nodelets/draw_text_nodelet.cpp)
target_link_libraries(${PROJECT_NAME}_nodelets ${PROJECT_NAME})

add_executable(normalization_image_generator_node src/nodes/normalization_image_generator_node.cpp)
target_link_libraries(normalization_image_generator_node ${PROJECT_NAME})

add_executable(dummy_image_publisher src/nodes/dummy_image_publisher.cpp)
target_link_libraries(dummy_image_publisher ${catkin_LIBRARIES})

add_executable(rotate_image src/nodes/rotate_image.cpp)
target_link_libraries(rotate_image ${catkin_LIBRARIES})

add_executable(scale_image src/nodes/scale_image.cpp)
target_link_libraries(scale_image ${catkin_LIBRARIES})

add_executable(draw_text src/nodes/draw_text.cpp)
target_link_libraries(draw_text ${catkin_LIBRARIES})

add_executable(contrast_stretch src/nodes/contrast_stretch.cpp)
target_link_libraries(contrast_stretch ${catkin_LIBRARIES})

add_executable(normalize_response src/nodes/normalize_response.cpp)
target_link_libraries(normalize_response ${catkin_LIBRARIES})

if(CATKIN_ENABLE_TESTING)
  find_package(rostest REQUIRED)
  add_rostest_gtest(test_geometry_util test/geometry_util.test test/test_geometry_util.cpp)
  target_link_libraries(test_geometry_util ${PROJECT_NAME})
endif()

install(DIRECTORY include/${PROJECT_NAME}/
  DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}
  FILES_MATCHING PATTERN "*.h"
)

install(TARGETS ${PROJECT_NAME} 
    ${PROJECT_NAME}_nodelets
    normalization_image_generator_node 
    dummy_image_publisher
    rotate_image
    scale_image
    draw_text
    contrast_stretch
    normalize_response
  RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
  LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
)

install(FILES nodelet_plugins.xml
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
)

