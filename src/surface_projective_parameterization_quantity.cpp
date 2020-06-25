// Copyright 2017-2019, Nicholas Sharp and the Polyscope contributors. http://polyscope.run.
#include "polyscope/surface_projective_parameterization_quantity.h"

#include "polyscope/file_helpers.h"
#include "polyscope/polyscope.h"
#include "polyscope/render/materials.h"
#include "polyscope/render/shaders.h"

#include "imgui.h"

using std::cout;
using std::endl;

namespace polyscope {

// ==============================================================
// ================  Base ProjectiveParameterization  =====================
// ==============================================================

SurfaceProjectiveParameterizationQuantity::SurfaceProjectiveParameterizationQuantity(std::string name,
                                                                                     ParamCoordsType type_,
                                                                                     ParamVizStyle style_,
                                                                                     SurfaceMesh& mesh_)
    : SurfaceMeshQuantity(name, mesh_, true), coordsType(type_), checkerSize(uniquePrefix() + "#checkerSize", 0.02),
      vizStyle(uniquePrefix() + "#vizStyle", style_), checkColor1(uniquePrefix() + "#checkColor1", render::RGB_PINK),
      checkColor2(uniquePrefix() + "#checkColor2", glm::vec3(.976, .856, .885)),
      gridLineColor(uniquePrefix() + "#gridLineColor", render::RGB_WHITE),
      gridBackgroundColor(uniquePrefix() + "#gridBackgroundColor", render::RGB_PINK),
      cMap(uniquePrefix() + "#cMap", "phase") {}


void SurfaceProjectiveParameterizationQuantity::draw() {
  if (!isEnabled()) return;

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

  switch (getStyle()) {
  case ParamVizStyle::CHECKER:
    program = render::engine->generateShaderProgram(
        {render::PROJECTIVE_PARAM_SURFACE_VERT_SHADER, render::PROJECTIVE_PARAM_CHECKER_SURFACE_FRAG_SHADER},
        DrawMode::Triangles);
    break;
  case ParamVizStyle::GRID:
    program = render::engine->generateShaderProgram(
        {render::PROJECTIVE_PARAM_SURFACE_VERT_SHADER, render::PROJECTIVE_PARAM_GRID_SURFACE_FRAG_SHADER},
        DrawMode::Triangles);
    break;
  case ParamVizStyle::LOCAL_CHECK:
    program = render::engine->generateShaderProgram(
        {render::PROJECTIVE_PARAM_SURFACE_VERT_SHADER, render::PROJECTIVE_PARAM_LOCAL_CHECKER_SURFACE_FRAG_SHADER},
        DrawMode::Triangles);
    program->setTextureFromColormap("t_colormap", cMap.get());
    break;
  case ParamVizStyle::LOCAL_RAD:
    program = render::engine->generateShaderProgram(
        {render::PROJECTIVE_PARAM_SURFACE_VERT_SHADER, render::PROJECTIVE_PARAM_LOCAL_RAD_SURFACE_FRAG_SHADER},
        DrawMode::Triangles);
    program->setTextureFromColormap("t_colormap", cMap.get());
    break;
  }

  // Fill color buffers
  parent.fillGeometryBuffers(*program);
  fillPositionBuffers(*program);

  render::engine->setMaterial(*program, parent.getMaterial());
}


// Update range uniforms
void SurfaceProjectiveParameterizationQuantity::setProgramUniforms(render::ShaderProgram& program) {

  // Interpretatin of modulo parameter depends on data type
  switch (coordsType) {
  case ParamCoordsType::UNIT:
    program.setUniform("u_modLen", getCheckerSize());
    break;
  case ParamCoordsType::WORLD:
    program.setUniform("u_modLen", getCheckerSize() * state::lengthScale);
    break;
  }

  // Set other uniforms needed
  switch (getStyle()) {
  case ParamVizStyle::CHECKER:
    program.setUniform("u_color1", getCheckerColors().first);
    program.setUniform("u_color2", getCheckerColors().second);
    break;
  case ParamVizStyle::GRID:
    program.setUniform("u_gridLineColor", getGridColors().first);
    program.setUniform("u_gridBackgroundColor", getGridColors().second);
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
  if (ImGui::BeginCombo("style", styleName(getStyle()).c_str())) {
    for (ParamVizStyle s :
         {ParamVizStyle::CHECKER, ParamVizStyle::GRID, ParamVizStyle::LOCAL_CHECK, ParamVizStyle::LOCAL_RAD}) {
      if (ImGui::Selectable(styleName(s).c_str(), s == getStyle())) {
        setStyle(s);
      }
    }
    ImGui::EndCombo();
  }


  // Modulo stripey width
  if (ImGui::DragFloat("period", &checkerSize.get(), .001, 0.0001, 1.0, "%.4f", 2.0)) {
    setCheckerSize(getCheckerSize());
  }


  ImGui::PopItemWidth();

  switch (getStyle()) {
  case ParamVizStyle::CHECKER:
    if (ImGui::ColorEdit3("##colors2", &checkColor1.get()[0], ImGuiColorEditFlags_NoInputs))
      setCheckerColors(getCheckerColors());
    ImGui::SameLine();
    if (ImGui::ColorEdit3("colors", &checkColor2.get()[0], ImGuiColorEditFlags_NoInputs))
      setCheckerColors(getCheckerColors());
    break;
  case ParamVizStyle::GRID:
    if (ImGui::ColorEdit3("base", &gridBackgroundColor.get()[0], ImGuiColorEditFlags_NoInputs))
      setGridColors(getGridColors());
    ImGui::SameLine();
    if (ImGui::ColorEdit3("line", &gridLineColor.get()[0], ImGuiColorEditFlags_NoInputs))
      setGridColors(getGridColors());
    break;
  case ParamVizStyle::LOCAL_CHECK:
  case ParamVizStyle::LOCAL_RAD: {
    // Angle slider
    ImGui::PushItemWidth(100);
    ImGui::SliderAngle("angle shift", &localRot, -180, 180); // displays in degrees, works in radians
    ImGui::PopItemWidth();

    // Set colormap
    if (render::buildColormapSelector(cMap.get())) {
      setColorMap(getColorMap());
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

ParamVizStyle SurfaceProjectiveParameterizationQuantity::getStyle() { return vizStyle.get(); }

SurfaceProjectiveParameterizationQuantity*
SurfaceProjectiveParameterizationQuantity::setCheckerColors(std::pair<glm::vec3, glm::vec3> colors) {
  checkColor1 = colors.first;
  checkColor2 = colors.second;
  requestRedraw();
  return this;
}

std::pair<glm::vec3, glm::vec3> SurfaceProjectiveParameterizationQuantity::getCheckerColors() {
  return std::make_pair(checkColor1.get(), checkColor2.get());
}

SurfaceProjectiveParameterizationQuantity*
SurfaceProjectiveParameterizationQuantity::setGridColors(std::pair<glm::vec3, glm::vec3> colors) {
  gridLineColor = colors.first;
  gridBackgroundColor = colors.second;
  requestRedraw();
  return this;
}

std::pair<glm::vec3, glm::vec3> SurfaceProjectiveParameterizationQuantity::getGridColors() {
  return std::make_pair(gridLineColor.get(), gridBackgroundColor.get());
}

SurfaceProjectiveParameterizationQuantity* SurfaceProjectiveParameterizationQuantity::setCheckerSize(double newVal) {
  checkerSize = newVal;
  requestRedraw();
  return this;
}

double SurfaceProjectiveParameterizationQuantity::getCheckerSize() { return checkerSize.get(); }

SurfaceProjectiveParameterizationQuantity* SurfaceProjectiveParameterizationQuantity::setColorMap(std::string name) {
  cMap = name;
  program.reset();
  requestRedraw();
  return this;
}
std::string SurfaceProjectiveParameterizationQuantity::getColorMap() { return cMap.get(); }

void SurfaceProjectiveParameterizationQuantity::geometryChanged() { program.reset(); }

// ==============================================================
// ===============  Corner ProjectiveParameterization  ====================
// ==============================================================

SurfaceCornerProjectiveParameterizationQuantity::SurfaceCornerProjectiveParameterizationQuantity(
    std::string name, std::vector<glm::vec3> coords_, ParamCoordsType type_, SurfaceMesh& mesh_)
    : SurfaceProjectiveParameterizationQuantity(name, type_, ParamVizStyle::CHECKER, mesh_),
      coords(std::move(coords_)) {}

std::string SurfaceCornerProjectiveParameterizationQuantity::niceName() { return name + " (corner parameterization)"; }

void SurfaceCornerProjectiveParameterizationQuantity::fillPositionBuffers(render::ShaderProgram& p) {
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

      glm::vec3 vRootVal, vBVal, vCVal;
      vRootVal = coords[cornerCounter];
      vBVal = coords[cornerCounter + j];
      vCVal = coords[cornerCounter + j + 1];

      texCoord.emplace_back(vRootVal);
      texCoord.emplace_back(vBVal);
      texCoord.emplace_back(vCVal);
    }
    cornerCounter += D;
  }

  // Store data in buffers
  p.setAttribute("a_coord", texCoord);
}

void SurfaceCornerProjectiveParameterizationQuantity::buildHalfedgeInfoGUI(size_t heInd) {
  ImGui::TextUnformatted(name.c_str());
  ImGui::NextColumn();
  ImGui::Text("<%g,%g>", coords[heInd].x, coords[heInd].y);
  ImGui::NextColumn();
}

} // namespace polyscope
