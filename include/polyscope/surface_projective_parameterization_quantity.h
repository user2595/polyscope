// Copyright 2017-2019, Nicholas Sharp and the Polyscope contributors. http://polyscope.run.
#pragma once

#include "polyscope/affine_remapper.h"
#include "polyscope/histogram.h"
#include "polyscope/surface_mesh.h"
#include "polyscope/surface_parameterization_enums.h"


namespace polyscope {


// ==============================================================
// ================  Base ProjectiveParameterization  =====================
// ==============================================================


class SurfaceProjectiveParameterizationQuantity : public SurfaceMeshQuantity {

public:
  SurfaceProjectiveParameterizationQuantity(std::string name, ParamCoordsType type_, ParamVizStyle style_,
                                            SurfaceMesh& mesh_);

  void draw() override;
  virtual void buildCustomUI() override;

  virtual void geometryChanged() override;


  // === Members
  ParamCoordsType coordsType;

  // === Viz stuff
  // to keep things simple, has settings for all of the viz styles, even though not all are used at all times


  // === Getters and setters for visualization options

  // What visualization scheme to use
  SurfaceProjectiveParameterizationQuantity* setStyle(ParamVizStyle newStyle);
  ParamVizStyle getStyle();

  // Colors for checkers
  SurfaceProjectiveParameterizationQuantity* setCheckerColors(std::pair<glm::vec3, glm::vec3> colors);
  std::pair<glm::vec3, glm::vec3> getCheckerColors();

  // Colors for checkers
  SurfaceProjectiveParameterizationQuantity* setGridColors(std::pair<glm::vec3, glm::vec3> colors);
  std::pair<glm::vec3, glm::vec3> getGridColors();

  // The size of checkers / stripes
  SurfaceProjectiveParameterizationQuantity* setCheckerSize(double newVal);
  double getCheckerSize();

  // Color map for radial visualization
  SurfaceProjectiveParameterizationQuantity* setColorMap(std::string val);
  std::string getColorMap();

protected:
  // === Visualiztion options
  PersistentValue<float> checkerSize;
  PersistentValue<ParamVizStyle> vizStyle;
  PersistentValue<glm::vec3> checkColor1, checkColor2;           // for checker (two colors to use)
  PersistentValue<glm::vec3> gridLineColor, gridBackgroundColor; // for GRID (two colors to use)

  PersistentValue<std::string> cMap;
  float localRot = 0.; // for LOCAL (angular shift, in radians)
  std::shared_ptr<render::ShaderProgram> program;
  bool projectiveInterpolate = true; // TODO: make persistent

  // Helpers
  void createProgram();
  void setProgramUniforms(render::ShaderProgram& program);
  virtual void fillPositionBuffers(render::ShaderProgram& p) = 0;
};


// ==============================================================
// ===============  Corner ProjectiveParameterization  ====================
// ==============================================================

class SurfaceCornerProjectiveParameterizationQuantity : public SurfaceProjectiveParameterizationQuantity {

public:
  SurfaceCornerProjectiveParameterizationQuantity(std::string name, std::vector<glm::vec2> values_,
                                                  ParamCoordsType type_, SurfaceMesh& mesh_);
  SurfaceCornerProjectiveParameterizationQuantity(std::string name, std::vector<glm::vec2> values_,
                                                  std::vector<double> cornerScaleFactors_, ParamCoordsType type_,
                                                  SurfaceMesh& mesh_);

  virtual void buildHalfedgeInfoGUI(size_t heInd) override;
  virtual std::string niceName() override;

  // === Members
  std::vector<glm::vec2> coords; // on corners
  std::vector<double> cornerScaleFactors;

protected:
  virtual void fillPositionBuffers(render::ShaderProgram& p) override;
};

} // namespace polyscope
