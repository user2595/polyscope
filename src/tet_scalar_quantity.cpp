#include "polyscope/tet_scalar_quantity.h"

#include "polyscope/file_helpers.h"
#include "polyscope/polyscope.h"
#include "polyscope/render/engine.h"
#include "polyscope/render/shaders.h"

#include "imgui.h"

using std::cout;
using std::endl;

namespace polyscope {

TetScalarQuantity::TetScalarQuantity(std::string name, TetMesh& mesh_, std::string definedOn_, DataType dataType_)
    : TetMeshQuantity(name, mesh_, true), dataType(dataType_),
      cMap(uniquePrefix() + name + "#cmap", defaultColorMap(dataType)), definedOn(definedOn_) {}

void TetScalarQuantity::draw() {
  if (!isEnabled()) return;

  if (program == nullptr) {
    createProgram();
  }

  // Set uniforms
  parent.setTransformUniforms(*program);
  setProgramUniforms(*program);

  program->draw();
}
void TetScalarQuantity::geometryChanged() { program.reset(); }

TetScalarQuantity* TetScalarQuantity::setColorMap(std::string name) {
  cMap = name;
  hist.updateColormap(cMap.get());
  requestRedraw();
  return this;
}
std::string TetScalarQuantity::getColorMap() { return cMap.get(); }

TetScalarQuantity* TetScalarQuantity::setMapRange(std::pair<double, double> val) {
  vizRange = val;
  requestRedraw();
  return this;
}
std::pair<double, double> TetScalarQuantity::getMapRange() { return vizRange; }

void TetScalarQuantity::writeToFile(std::string filename) {
  polyscope::warning("Writing to file not yet implemented for this datatype");
}


// Update range uniforms
void TetScalarQuantity::setProgramUniforms(render::ShaderProgram& program) {
  program.setUniform("u_rangeLow", vizRange.first);
  program.setUniform("u_rangeHigh", vizRange.second);
}

TetScalarQuantity* TetScalarQuantity::resetMapRange() {
  switch (dataType) {
  case DataType::STANDARD:
    vizRange = dataRange;
    break;

  case DataType::SYMMETRIC: {
    double absRange = std::max(std::abs(dataRange.first), std::abs(dataRange.second));
    vizRange = std::make_pair(-absRange, absRange);
  } break;
  case DataType::MAGNITUDE:
    vizRange = std::make_pair(0., dataRange.second);
    break;
  }

  requestRedraw();
  return this;
}

void TetScalarQuantity::buildCustomUI() {
  ImGui::SameLine();

  // == Options popup
  if (ImGui::Button("Options")) {
    ImGui::OpenPopup("OptionsPopup");
  }
  if (ImGui::BeginPopup("OptionsPopup")) {

    if (ImGui::MenuItem("Write to file")) writeToFile();
    if (ImGui::MenuItem("Reset colormap range")) resetMapRange();

    ImGui::EndPopup();
  }

  if (render::buildColormapSelector(cMap.get())) {
    program.reset();
    setColorMap(getColorMap());
  }

  // Draw the histogram of values
  hist.colormapRange = vizRange;
  hist.buildUI();

  // Data range
  // Note: %g specifies are generally nicer than %e, but here we don't
  // acutally have a choice. ImGui (for somewhat valid reasons) links the
  // resolution of the slider to the decimal width of the formatted number.
  // When %g formats a number with few decimal places, sliders can break.
  // There is no way to set a minimum number of decimal places with %g,
  // unfortunately.
  {
    switch (dataType) {
    case DataType::STANDARD:
      ImGui::DragFloatRange2("", &vizRange.first, &vizRange.second, (dataRange.second - dataRange.first) / 100.,
                             dataRange.first, dataRange.second, "Min: %.3e", "Max: %.3e");
      break;
    case DataType::SYMMETRIC: {
      float absRange = std::max(std::abs(dataRange.first), std::abs(dataRange.second));
      ImGui::DragFloatRange2("##range_symmetric", &vizRange.first, &vizRange.second, absRange / 100., -absRange,
                             absRange, "Min: %.3e", "Max: %.3e");
    } break;
    case DataType::MAGNITUDE: {
      ImGui::DragFloatRange2("##range_mag", &vizRange.first, &vizRange.second, vizRange.second / 100., 0.0,
                             dataRange.second, "Min: %.3e", "Max: %.3e");
    } break;
    }
  }
}

std::string SurfaceScalarQuantity::getColorMap() { return cMap.get(); }
std::string TetScalarQuantity::niceName() { return name + " (" + definedOn + " scalar)"; }

// ========================================================
// ==========           Vertex Scalar            ==========
// ========================================================

TetVertexScalarQuantity::TetVertexScalarQuantity(std::string name, std::vector<double> values_, TetMesh& mesh_,
                                                 DataType dataType_)
    : TetScalarQuantity(name, mesh_, "vertex", dataType_), values(std::move(values_))

{
  hist.updateColormap(cMap.get());
  hist.buildHistogram(values, parent.vertexVolumes);

  dataRange = robustMinMax(values, 1e-5);
  resetMapRange();
}

void TetVertexScalarQuantity::createProgram() {
  // Create the program to draw this quantity
  program = render::engine->generateShaderProgram(
      {render::VERTCOLOR_SURFACE_VERT_SHADER, render::VERTCOLOR_SURFACE_FRAG_SHADER}, DrawMode::Triangles);

  // Fill color buffers
  parent.fillGeometryBuffers(*program);
  fillColorBuffers(*program);
  render::engine->setMaterial(*program, parent.getMaterial());
}

void TetVertexScalarQuantity::draw() {
  if (!isEnabled()) return;

  if (program == nullptr) {
    createProgram();
  }

  // Set uniforms
  parent.setTransformUniforms(*program);
  setProgramUniforms(*program);

  program->draw();
}

void TetVertexScalarQuantity::fillColorBuffers(render::ShaderProgram& p) {
  std::vector<double> colorval;
  colorval.reserve(3 * parent.visibleFaces.size());

  for (size_t iF = 0; iF < parent.visibleFaces.size(); ++iF) {
    std::vector<EdgePt> face = parent.visibleFaces[iF];
    size_t D = face.size();

    // implicitly triangulate from root
    double vRoot = parent.interp(face[0], values);
    for (size_t j = 1; (j + 1) < D; j++) {
      double vB = parent.interp(face[j], values);
      double vC = parent.interp(face[j + 1], values);

      colorval.push_back(vRoot);
      colorval.push_back(vB);
      colorval.push_back(vC);
    }
  }

  // Store data in buffers
  p.setAttribute("a_colorval", colorval);
  p.setTextureFromColormap("t_colormap", cMap.get());
}

void TetVertexScalarQuantity::writeToFile(std::string filename) { throw std::runtime_error("not implemented"); }


void TetVertexScalarQuantity::buildVertexInfoGUI(size_t vInd) {
  ImGui::TextUnformatted(name.c_str());
  ImGui::NextColumn();
  ImGui::Text("%g", values[vInd]);
  ImGui::NextColumn();
}

// ========================================================
// ==========            Face Scalar             ==========
// ========================================================

TetFaceScalarQuantity::TetFaceScalarQuantity(std::string name, std::vector<double> values_, TetMesh& mesh_,
                                             DataType dataType_)
    : TetScalarQuantity(name, mesh_, "face", dataType_), values(std::move(values_))

{
  hist.updateColormap(cMap.get());
  hist.buildHistogram(values, parent.faceAreas);

  dataRange = robustMinMax(values, 1e-5);
  resetMapRange();
}

void TetFaceScalarQuantity::createProgram() {
  // Create the program to draw this quantity
  program = render::engine->generateShaderProgram(
      {render::VERTCOLOR_SURFACE_VERT_SHADER, render::VERTCOLOR_SURFACE_FRAG_SHADER}, DrawMode::Triangles);

  // Fill color buffers
  parent.fillGeometryBuffers(*program);
  fillColorBuffers(*program);
  render::engine->setMaterial(*program, parent.getMaterial());
}

void TetFaceScalarQuantity::fillColorBuffers(render::ShaderProgram& p) {

  std::vector<double> colorval;
  std::cout << "NOT FACE COLORS IMPLEMENTED YET" << std::endl;
  colorval.reserve(3 * parent.visibleFaces.size());

  for (size_t iF = 0; iF < parent.visibleFaces.size(); ++iF) {
    std::vector<EdgePt> face = parent.visibleFaces[iF];
    size_t D = face.size();

    // implicitly triangulate from root
    double vRoot = parent.interp(face[0], values);
    for (size_t j = 1; (j + 1) < D; j++) {
      double vB = parent.interp(face[j], values);
      double vC = parent.interp(face[j + 1], values);

      colorval.push_back(values[iF]);
      colorval.push_back(values[iF]);
      colorval.push_back(values[iF]);
    }
  }

  // Store data in buffers
  p.setAttribute("a_colorval", colorval);
  p.setTextureFromColormap("t_colormap", cMap.get());
}

void TetFaceScalarQuantity::buildFaceInfoGUI(size_t fInd) {
  ImGui::TextUnformatted(name.c_str());
  ImGui::NextColumn();
  ImGui::Text("%g", values[fInd]);
  ImGui::NextColumn();
}


} // namespace polyscope
