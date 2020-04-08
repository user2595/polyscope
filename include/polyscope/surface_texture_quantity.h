// Copyright 2017-2019, Nicholas Sharp and the Polyscope contributors. http://polyscope.run.
#pragma once

#include "polyscope/surface_mesh.h"

namespace polyscope {

class SurfaceTextureQuantity : public SurfaceMeshQuantity {
public:
  SurfaceTextureQuantity(std::string name, SurfaceMesh& mesh_, std::vector<glm::vec2> coords, DataType dataType);

  virtual void draw() override;
  virtual void buildCustomUI() override;
  virtual std::string niceName() override;
  virtual void geometryChanged() override;

  // === Members
  const DataType dataType;


  void fillColorBuffers(render::ShaderProgram& p);
  void buildHalfedgeInfoGUI(size_t heInd) override;
  SurfaceTextureQuantity* setTexture(std::string filename);

  // === Members
  std::vector<glm::vec2> coords;

protected:
  const std::string definedOn;
  std::shared_ptr<render::ShaderProgram> program;
  unsigned char* img;
  bool imageLoaded = false;
  int imgWidth, imgHeight, imgComp;

  // Helpers
  virtual void createProgram();
  void setProgramUniforms(render::ShaderProgram& program);
};

} // namespace polyscope
