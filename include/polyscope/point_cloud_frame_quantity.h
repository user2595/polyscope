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
  float lengthMult; // longest frame will be this fraction of lengthScale (if not ambient)
  float radiusMult; // radius is this fraction of lengthScale
  std::array<glm::vec3, 3> frameColors;


  // The map that takes values to [0,1] for drawing
  AffineRemapper<glm::vec3> mapper;

  void writeToFile(std::string filename = "");

  void createProgram();
  std::unique_ptr<gl::GLProgram> program;
};

} // namespace polyscope
