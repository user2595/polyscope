#pragma once

#include "polyscope/affine_remapper.h"
#include "polyscope/render/engine.h"
#include "polyscope/ribbon_artist.h"
#include "polyscope/surface_mesh.h"
#include "polyscope/surface_mesh_enums.h"

#include "tet_mesh.h"
#include "tet_mesh_quantity.h"

namespace polyscope {

// Forward declare TetMesh
class TetMesh;

// ==== Common base class

// Represents a general vector field associated with a tet mesh, including
// R3 fields in the ambient space and R2 fields embedded in the tet
class TetVectorQuantity : public TetMeshQuantity {
public:
  TetVectorQuantity(std::string name, TetMesh& mesh_, MeshElement definedOn_,
                    VectorType vectorType_ = VectorType::STANDARD);


  virtual void draw() override;
  virtual void buildCustomUI() override;
  virtual void geometryChanged() override;
  virtual bool skipVector(size_t iV) = 0;

  // Allow children to append to the UI
  virtual void drawSubUI();

  // === Members
  const VectorType vectorType;
  std::vector<glm::vec3> vectorRoots;
  std::vector<glm::vec3> vectors;


  // === Option accessors

  //  The vectors will be scaled such that the longest vector is this long
  TetVectorQuantity* setVectorLengthScale(double newLength, bool isRelative = true);
  double getVectorLengthScale();

  // The radius of the vectors
  TetVectorQuantity* setVectorRadius(double val, bool isRelative = true);
  double getVectorRadius();

  // The color of the vectors
  TetVectorQuantity* setVectorColor(glm::vec3 color);
  glm::vec3 getVectorColor();

  // Material
  TetVectorQuantity* setMaterial(std::string name);
  std::string getMaterial();

  // Enable the ribbon visualization
  TetVectorQuantity* setRibbonEnabled(bool newVal);
  bool isRibbonEnabled();

  // Clip vector field when clipping tets
  bool hideWithMesh = true;

  void writeToFile(std::string filename = "");

  // GL things
  void prepareProgram();
  std::shared_ptr<render::ShaderProgram> program;

protected:
  // === Visualization options
  PersistentValue<ScaledValue<float>> vectorLengthMult;
  PersistentValue<ScaledValue<float>> vectorRadius;
  PersistentValue<glm::vec3> vectorColor;
  PersistentValue<std::string> material;

  // The map that takes values to [0,1] for drawing
  AffineRemapper<glm::vec3> mapper;

  MeshElement definedOn;

  // A ribbon viz that is appropriate for some fields
  std::unique_ptr<RibbonArtist> ribbonArtist;
  PersistentValue<bool> ribbonEnabled;

  // Set up the mapper for vectors
  void prepareVectorMapper();
};


// ==== R3 vectors at vertices

class TetVertexVectorQuantity : public TetVectorQuantity {
public:
  TetVertexVectorQuantity(std::string name, std::vector<glm::vec3> vectors_, TetMesh& mesh_,
                          VectorType vectorType_ = VectorType::STANDARD);

  std::vector<glm::vec3> vectorField;
  virtual std::string niceName() override;
  virtual void buildVertexInfoGUI(size_t vInd) override;
  virtual bool skipVector(size_t iV) override;
};

// ==== R3 vectors at faces

class TetFaceVectorQuantity : public TetVectorQuantity {
public:
  TetFaceVectorQuantity(std::string name, std::vector<glm::vec3> vectors_, TetMesh& mesh_,
                        VectorType vectorType_ = VectorType::STANDARD);

  std::vector<glm::vec3> vectorField;

  virtual std::string niceName() override;
  virtual void buildFaceInfoGUI(size_t fInd) override;
  virtual bool skipVector(size_t iV) override;
};

// ==== R3 vectors at tets

class TetTetVectorQuantity : public TetVectorQuantity {
public:
  TetTetVectorQuantity(std::string name, std::vector<glm::vec3> vectors_, TetMesh& mesh_,
                       VectorType vectorType_ = VectorType::STANDARD);

  std::vector<glm::vec3> vectorField;

  virtual std::string niceName() override;
  virtual void buildTetInfoGUI(size_t tInd) override;
  virtual bool skipVector(size_t iV) override;
};

} // namespace polyscope
