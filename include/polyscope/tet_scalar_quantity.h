#pragma once

#include "polyscope/affine_remapper.h"
#include "polyscope/histogram.h"
#include "polyscope/render/color_maps.h"
#include "polyscope/render/engine.h"

#include "tet_mesh.h"
#include "tet_mesh_quantity.h"

namespace polyscope {

// Forward declare TetMesh
class TetMesh;

class TetScalarQuantity : public TetMeshQuantity {
public:
  TetScalarQuantity(std::string name, TetMesh& mesh_, std::string definedOn, DataType dataType);

  virtual void draw() override;
  virtual void buildCustomUI() override;
  virtual std::string niceName() override;
  virtual void geometryChanged() override;

  virtual void writeToFile(std::string filename = "");

  // === Members
  const DataType dataType;

  // === Get/set visualization parameters

  // The color map
  TetScalarQuantity* setColorMap(std::string val);
  std::string getColorMap();

  // Data limits mapped in to colormap
  TetScalarQuantity* setMapRange(std::pair<double, double> val);
  std::pair<double, double> getMapRange();
  TetScalarQuantity* resetMapRange(); // reset to full range

protected:
  // Affine data maps and limits
  std::pair<float, float> vizRange;
  std::pair<double, double> dataRange;
  Histogram hist;


  // UI internals
  PersistentValue<std::string> cMap;
  const std::string definedOn;
  std::shared_ptr<render::ShaderProgram> program;

  // Helpers
  virtual void createProgram() = 0;
  void setProgramUniforms(render::ShaderProgram& program);
};

// ========================================================
// ==========           Vertex Scalar            ==========
// ========================================================

class TetVertexScalarQuantity : public TetScalarQuantity {
public:
  TetVertexScalarQuantity(std::string name, std::vector<double> values_, TetMesh& mesh_,
                          DataType dataType_ = DataType::STANDARD);

  virtual void createProgram() override;

  void fillColorBuffers(render::ShaderProgram& p);

  void draw() override;

  void buildVertexInfoGUI(size_t vInd) override;
  virtual void writeToFile(std::string filename = "") override;

  // === Members
  std::vector<double> values;
};

// ========================================================
// ==========            Face Scalar             ==========
// ========================================================

class TetFaceScalarQuantity : public TetScalarQuantity {
public:
  TetFaceScalarQuantity(std::string name, std::vector<double> values_, TetMesh& mesh_,
                        DataType dataType_ = DataType::STANDARD);

  virtual void createProgram() override;

  void fillColorBuffers(render::ShaderProgram& p);

  void buildFaceInfoGUI(size_t fInd) override;

  // === Members
  std::vector<double> values;
};


} // namespace polyscope
