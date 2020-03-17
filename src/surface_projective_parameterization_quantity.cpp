// Copyright 2017-2019, Nicholas Sharp and the Polyscope contributors. http://polyscope.run.
#include "polyscope/surface_projective_parameterization_quantity.h"

#include "polyscope/file_helpers.h"
#include "polyscope/gl/materials/materials.h"
#include "polyscope/gl/shaders.h"
#include "polyscope/gl/shaders/projective_parameterization_shaders.h"
#include "polyscope/polyscope.h"

#include "imgui.h"

using std::cout;
using std::endl;

namespace polyscope {

// ==============================================================
// ================  Base ProjectiveParameterization  =====================
// ==============================================================

SurfaceProjectiveParameterizationQuantity::SurfaceProjectiveParameterizationQuantity(std::string name,
                                                                                     ParamCoordsType type_,
                                                                                     SurfaceMesh& mesh_)
    : SurfaceMeshQuantity(name, mesh_, true), coordsType(type_) {

  // Set default colormap
  cMap = gl::ColorMapID::PHASE;

  // Set a checker color
  checkColor1 = gl::RGB_PINK;
  glm::vec3 checkColor1HSV = RGBtoHSV(checkColor1);
  checkColor1HSV.y *= 0.15; // very light
  checkColor2 = HSVtoRGB(checkColor1HSV);

  // set grid color
  gridLineColor = gl::RGB_WHITE;
  gridBackgroundColor = gl::RGB_PINK;
}

void SurfaceProjectiveParameterizationQuantity::draw() {
  if (!enabled) return;

  if (program == nullptr) {
    createProgram();
  }

  // Set uniforms
  parent.setTransformUniforms(*program);
  setProgramUniforms(*program);

  program->draw();
}

void SurfaceProjectiveParameterizationQuantity::createProgram() {
  // Create the program to draw this quantity

  switch (vizStyle) {
  case ParamVizStyle::CHECKER:
    program.reset(new gl::GLProgram(&gl::PROJECTIVE_PARAM_SURFACE_VERT_SHADER,
                                    &gl::PROJECTIVE_PARAM_CHECKER_SURFACE_FRAG_SHADER, gl::DrawMode::Triangles));
    break;
  case ParamVizStyle::GRID:
    program.reset(new gl::GLProgram(&gl::PROJECTIVE_PARAM_SURFACE_VERT_SHADER,
                                    &gl::PROJECTIVE_PARAM_GRID_SURFACE_FRAG_SHADER, gl::DrawMode::Triangles));
    break;
  case ParamVizStyle::LOCAL_CHECK:
    program.reset(new gl::GLProgram(&gl::PROJECTIVE_PARAM_SURFACE_VERT_SHADER,
                                    &gl::PROJECTIVE_PARAM_LOCAL_CHECKER_SURFACE_FRAG_SHADER, gl::DrawMode::Triangles));
    program->setTextureFromColormap("t_colormap", gl::getColorMap(cMap));
    break;
  case ParamVizStyle::LOCAL_RAD:
    program.reset(new gl::GLProgram(&gl::PROJECTIVE_PARAM_SURFACE_VERT_SHADER,
                                    &gl::PROJECTIVE_PARAM_LOCAL_RAD_SURFACE_FRAG_SHADER, gl::DrawMode::Triangles));
    program->setTextureFromColormap("t_colormap", gl::getColorMap(cMap));
    break;
  }

  // Fill color buffers
  parent.fillGeometryBuffers(*program);
  fillPositionBuffers(*program);

  setMaterialForProgram(*program, "wax");
}


// Update range uniforms
void SurfaceProjectiveParameterizationQuantity::setProgramUniforms(gl::GLProgram& program) {

  // Interpretatin of modulo parameter depends on data type
  switch (coordsType) {
  case ParamCoordsType::UNIT:
    program.setUniform("u_modLen", modLen);
    break;
  case ParamCoordsType::WORLD:
    program.setUniform("u_modLen", modLen * state::lengthScale);
    break;
  }

  // Set other uniforms needed
  switch (vizStyle) {
  case ParamVizStyle::CHECKER:
    program.setUniform("u_color1", checkColor1);
    program.setUniform("u_color2", checkColor2);
    break;
  case ParamVizStyle::GRID:
    program.setUniform("u_gridLineColor", gridLineColor);
    program.setUniform("u_gridBackgroundColor", gridBackgroundColor);
    break;
  case ParamVizStyle::LOCAL_CHECK:
  case ParamVizStyle::LOCAL_RAD:
    program.setUniform("u_angle", localRot);
    break;
  }
}

namespace {
// Helper to name styles
std::string styleName(ParamVizStyle v) {
  switch (v) {
  case ParamVizStyle::CHECKER:
    return "checker";
    break;
  case ParamVizStyle::GRID:
    return "grid";
    break;
  case ParamVizStyle::LOCAL_CHECK:
    return "local grid";
    break;
  case ParamVizStyle::LOCAL_RAD:
    return "local dist";
    break;
  }
  throw std::runtime_error("broken");
}

} // namespace

void SurfaceProjectiveParameterizationQuantity::buildCustomUI() {
  ImGui::PushItemWidth(100);

  ImGui::SameLine(); // put it next to enabled

  // Choose viz style
  if (ImGui::BeginCombo("style", styleName(vizStyle).c_str())) {
    for (ParamVizStyle s :
         {ParamVizStyle::CHECKER, ParamVizStyle::GRID, ParamVizStyle::LOCAL_CHECK, ParamVizStyle::LOCAL_RAD}) {
      if (ImGui::Selectable(styleName(s).c_str(), s == vizStyle)) {
        setStyle(s);
      }
    }
    ImGui::EndCombo();
  }


  // Modulo stripey width
  ImGui::DragFloat("period", &modLen, .001, 0.0001, 1000.0, "%.4f", 2.0);


  ImGui::PopItemWidth();

  switch (vizStyle) {
  case ParamVizStyle::CHECKER:
    ImGui::ColorEdit3("##colors2", (float*)&checkColor1, ImGuiColorEditFlags_NoInputs);
    ImGui::SameLine();
    ImGui::ColorEdit3("colors", (float*)&checkColor2, ImGuiColorEditFlags_NoInputs);
    break;
  case ParamVizStyle::GRID:
    ImGui::ColorEdit3("base", (float*)&gridBackgroundColor, ImGuiColorEditFlags_NoInputs);
    ImGui::SameLine();
    ImGui::ColorEdit3("line", (float*)&gridLineColor, ImGuiColorEditFlags_NoInputs);
    break;
  case ParamVizStyle::LOCAL_CHECK:
  case ParamVizStyle::LOCAL_RAD: {
    // Angle slider
    ImGui::PushItemWidth(100);
    ImGui::SliderAngle("angle shift", &localRot, -180, 180); // displays in degrees, works in radians
    ImGui::PopItemWidth();

    // Set colormap
    if (buildColormapSelector(cMap)) {
      program.reset();
    }
  }

  break;
  }
  if (ImGui::Checkbox("Projective Interpolate", &projectiveInterpolate)) {
    fillPositionBuffers(*program);
  }
}


SurfaceProjectiveParameterizationQuantity* SurfaceProjectiveParameterizationQuantity::setStyle(ParamVizStyle newStyle) {
  vizStyle = newStyle;
  program.reset();
  requestRedraw();
  return this;
}

void SurfaceProjectiveParameterizationQuantity::geometryChanged() { program.reset(); }

// ==============================================================
// ===============  Corner ProjectiveParameterization  ====================
// ==============================================================


SurfaceCornerProjectiveParameterizationQuantity::SurfaceCornerProjectiveParameterizationQuantity(
    std::string name, std::vector<glm::vec2> coords_, ParamCoordsType type_, SurfaceMesh& mesh_)
    : SurfaceProjectiveParameterizationQuantity(name, type_, mesh_), coords(std::move(coords_)) {
  cornerScaleFactors.reserve(coords.size());
  for (size_t iC = 0; iC < coords.size(); ++iC) {
    cornerScaleFactors.push_back(0);
  }
}

SurfaceCornerProjectiveParameterizationQuantity::SurfaceCornerProjectiveParameterizationQuantity(
    std::string name, std::vector<glm::vec2> coords_, std::vector<double> cornerScaleFactors_, ParamCoordsType type_,
    SurfaceMesh& mesh_)
    : SurfaceProjectiveParameterizationQuantity(name, type_, mesh_), coords(std::move(coords_)),
      cornerScaleFactors(std::move(cornerScaleFactors_)) {}

std::string SurfaceCornerProjectiveParameterizationQuantity::niceName() { return name + " (corner parameterization)"; }

void SurfaceCornerProjectiveParameterizationQuantity::fillPositionBuffers(gl::GLProgram& p) {
  std::vector<glm::vec3> texCoord;
  texCoord.reserve(3 * parent.nFacesTriangulation());

  glm::vec3 zero{0.f, 0.f, 0.f};

  size_t cornerCounter = 0;
  for (size_t iF = 0; iF < parent.nFaces(); iF++) {
    auto& face = parent.faces[iF];
    size_t D = face.size();

    // implicitly triangulate from root
    size_t vRoot = face[0];
    for (size_t j = 1; (j + 1) < D; j++) {

      glm::vec2 vRootVal, vBVal, vCVal;
      double vRootSF, vBSF, vCSF;
      vRootVal = coords[cornerCounter];
      vBVal = coords[cornerCounter + j];
      vCVal = coords[cornerCounter + j + 1];
      vRootSF = (projectiveInterpolate) ? cornerScaleFactors[cornerCounter] : 0;
      vBSF = (projectiveInterpolate) ? cornerScaleFactors[cornerCounter + j] : 0;
      vCSF = (projectiveInterpolate) ? cornerScaleFactors[cornerCounter + j + 1] : 0;

      glm::vec3 projRoot = glm::vec3{vRootVal.x, vRootVal.y, 1} * (float)exp(-vRootSF);
      glm::vec3 projB = glm::vec3{vBVal.x, vBVal.y, 1} * (float)exp(-vBSF);
      glm::vec3 projC = glm::vec3{vCVal.x, vCVal.y, 1} * (float)exp(-vCSF);

      texCoord.emplace_back(projRoot);
      texCoord.emplace_back(projB);
      texCoord.emplace_back(projC);
    }
    cornerCounter += D;
  }

  // Store data in buffers
  p.setAttribute("a_texture_coord", texCoord);
}

void SurfaceCornerProjectiveParameterizationQuantity::buildHalfedgeInfoGUI(size_t heInd) {
  ImGui::TextUnformatted(name.c_str());
  ImGui::NextColumn();
  ImGui::Text("<%g,%g>", coords[heInd].x, coords[heInd].y);
  ImGui::NextColumn();
}

} // namespace polyscope
