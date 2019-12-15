// Copyright 2017-2019, Nicholas Sharp and the Polyscope contributors. http://polyscope.run.
#pragma once

#include "polyscope/gl/color_maps.h"
#include "polyscope/surface_mesh.h"

namespace polyscope {

class SurfaceTextureQuantity : public SurfaceMeshQuantity {
public:
  SurfaceTextureQuantity(std::string name, SurfaceMesh& mesh_, std::vector<Vector2> values, DataType dataType);

  virtual void draw() override;
  virtual void buildCustomUI() override;
  virtual std::string niceName() override;
  virtual void geometryChanged() override;

  virtual void writeToFile(std::string filename = "");

  // === Members
  const DataType dataType;


  void fillColorBuffers(gl::GLProgram& p);
  void buildVertexInfoGUI(size_t vInd) override;

  // === Members
  std::vector<double> values;

protected:
  const std::string definedOn;
  std::unique_ptr<gl::GLProgram> program;

  // Helpers
  virtual void createProgram();
  void setProgramUniforms(gl::GLProgram& program);
};

} // namespace polyscope
