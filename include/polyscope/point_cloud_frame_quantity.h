// Copyright 2017-2019, Nicholas Sharp and the Polyscope contributors. http://polyscope.run.
#pragma once

#include "polyscope/affine_remapper.h"
#include "polyscope/point_cloud.h"

#include <array>

namespace polyscope {

// Represents a general frame field associated with a point cloud
class PointCloudFrameQuantity : public PointCloudQuantity {
public:
  PointCloudFrameQuantity(std::string name, std::vector<std::array<glm::vec3, 3>> vectors, PointCloud& pointCloud_,
                          bool cross_ = false, VectorType vectorType_ = VectorType::STANDARD);

  virtual void draw() override;
  virtual void buildCustomUI() override;
  virtual void buildPickUI(size_t ind) override;
  virtual std::string niceName() override;
  virtual void geometryChanged() override;

  // === Members
  bool cross;
  const VectorType vectorType;
  std::vector<std::array<glm::vec3, 3>> frames;

  // === Option accessors

  //  The frames will be scaled such that all vectors are this long
  PointCloudFrameQuantity* setFrameLengthScale(double newLength, bool isRelative = true);
  double getFrameLengthScale();

  // The radius of the frames
  PointCloudFrameQuantity* setFrameRadius(double val, bool isRelative = true);
  double getFrameRadius();

  // The color of the frames
  PointCloudFrameQuantity* setFrameColors(std::array<glm::vec3, 3> color);
  std::array<glm::vec3, 3> getFrameColors();

  // Material
  PointCloudFrameQuantity* setMaterial(std::string name);
  std::string getMaterial();

  void writeToFile(std::string filename = "");

private:
  // === Visualization options
  PersistentValue<ScaledValue<float>> frameLengthMult;
  PersistentValue<ScaledValue<float>> frameRadius;
  PersistentValue<glm::vec3> frameColorX;
  PersistentValue<glm::vec3> frameColorY;
  PersistentValue<glm::vec3> frameColorZ;
  PersistentValue<bool> showX;
  PersistentValue<bool> showY;
  PersistentValue<bool> showZ;
  PersistentValue<std::string> material;

  // The map that takes values to [0,1] for drawing
  AffineRemapper<glm::vec3> mapper;

  void createProgram();
  std::shared_ptr<render::ShaderProgram> programX;
  std::shared_ptr<render::ShaderProgram> programY;
  std::shared_ptr<render::ShaderProgram> programZ;
};

} // namespace polyscope
