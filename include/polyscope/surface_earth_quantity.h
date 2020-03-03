// Copyright 2017-2019, Nicholas Sharp and the Polyscope contributors. http://polyscope.run.
#pragma once

#include "polyscope/affine_remapper.h"
#include "polyscope/gl/color_maps.h"
#include "polyscope/histogram.h"
#include "polyscope/surface_mesh.h"

namespace polyscope {

class SurfaceEarthQuantity : public SurfaceMeshQuantity {
public:
  SurfaceEarthQuantity(std::string name, SurfaceMesh& mesh_, std::vector<glm::vec3> values_,
                       DataType dataType_ = DataType::STANDARD);
  SurfaceEarthQuantity(std::string name, SurfaceMesh& mesh_, std::vector<glm::vec3> values_,
                       std::vector<double> scaleFactors_, DataType dataType_ = DataType::STANDARD);

  virtual void draw() override;
  virtual void buildCustomUI() override;
  virtual std::string niceName() override;
  virtual void geometryChanged() override;

  virtual void writeToFile(std::string filename = "");

  virtual void createProgram();
  void fillPositionBuffers(gl::GLProgram& p);

  // === Members
  const DataType dataType;
  std::vector<glm::vec3> values;
  std::vector<float> scaleFactors;
  bool projectiveInterpolate = true;
  bool cornerData = false;

protected:
  // UI internals
  std::unique_ptr<gl::GLProgram> program;

  // Helpers
  void setProgramTextures(gl::GLProgram& program);
};

} // namespace polyscope
