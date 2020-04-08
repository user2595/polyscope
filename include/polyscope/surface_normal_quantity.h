// Copyright 2017-2019, Nicholas Sharp and the Polyscope contributors. http://polyscope.run.
#pragma once

#include "polyscope/affine_remapper.h"
#include "polyscope/histogram.h"
#include "polyscope/surface_mesh.h"

namespace polyscope {

class SurfaceNormalQuantity : public SurfaceMeshQuantity {
public:
  SurfaceNormalQuantity(std::string name, SurfaceMesh& mesh_, std::string definedOn);

  virtual void draw() override;
  virtual void buildCustomUI() override;
  virtual std::string niceName() override;
  virtual void geometryChanged() override;

  virtual void writeToFile(std::string filename = "");

protected:
  const std::string definedOn;
  std::shared_ptr<render::ShaderProgram> program;

  // Helpers
  virtual void createProgram() = 0;
  void setProgramUniforms(render::ShaderProgram& program);
};

// ========================================================
// ==========           Vertex Normal            ==========
// ========================================================

class SurfaceVertexNormalQuantity : public SurfaceNormalQuantity {
public:
  SurfaceVertexNormalQuantity(std::string name, std::vector<glm::vec3> normalValues_, SurfaceMesh& mesh_);

  virtual void createProgram() override;
  void fillGeometryBuffers(render::ShaderProgram& p);

  void buildVertexInfoGUI(size_t vInd) override;
  virtual void writeToFile(std::string filename = "") override;

  // === Members
  std::vector<glm::vec3> normalValues;
};


// ========================================================
// ==========            Face Normal             ==========
// ========================================================

class SurfaceFaceNormalQuantity : public SurfaceNormalQuantity {
public:
  SurfaceFaceNormalQuantity(std::string name, std::vector<glm::vec3> normalValues_, SurfaceMesh& mesh_);

  virtual void createProgram() override;
  void fillGeometryBuffers(render::ShaderProgram& p);

  void buildFaceInfoGUI(size_t fInd) override;

  // === Members
  std::vector<glm::vec3> normalValues;
};
} // namespace polyscope
