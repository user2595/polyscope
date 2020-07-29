#include "polyscope/tet_vector_quantity.h"

#include "polyscope/file_helpers.h"
#include "polyscope/polyscope.h"
#include "polyscope/render/engine.h"
#include "polyscope/render/shaders.h"
#include "polyscope/trace_vector_field.h"

#include "imgui.h"

#include <complex>
#include <fstream>
#include <iostream>

using std::cout;
using std::endl;

namespace polyscope {

TetVectorQuantity::TetVectorQuantity(std::string name, TetMesh& mesh_, MeshElement definedOn_, VectorType vectorType_)
    : TetMeshQuantity(name, mesh_), vectorType(vectorType_),
      vectorLengthMult(uniquePrefix() + name + "#vectorLengthMult",
                       vectorType == VectorType::AMBIENT ? absoluteValue(1.0) : relativeValue(0.02)),
      vectorRadius(uniquePrefix() + name + "#vectorRadius", relativeValue(0.0025)),
      vectorColor(uniquePrefix() + "#vectorColor", getNextUniqueColor()),
      material(uniquePrefix() + "#material", "clay"), definedOn(definedOn_),
      ribbonEnabled(uniquePrefix() + "#ribbonEnabled", false) {}

void TetVectorQuantity::prepareVectorMapper() {

  // Create a mapper (default mapper is identity)
  if (vectorType == VectorType::AMBIENT) {
    mapper.setMinMax(vectors);
  } else {
    mapper = AffineRemapper<glm::vec3>(vectors, DataType::MAGNITUDE);
  }
}

void TetVectorQuantity::draw() {
  if (!isEnabled()) return;

  if (program == nullptr) {
    prepareProgram();
  }

  // Set uniforms
  parent.setTransformUniforms(*program);

  program->setUniform("u_radius", getVectorRadius());
  program->setUniform("u_baseColor", getVectorColor());

  if (vectorType == VectorType::AMBIENT) {
    program->setUniform("u_lengthMult", 1.0);
  } else {
    program->setUniform("u_lengthMult", getVectorLengthScale());
  }

  program->draw();
}

void TetVectorQuantity::geometryChanged() { program.reset(); }

void TetVectorQuantity::prepareProgram() {

  program = render::engine->generateShaderProgram(
      {render::PASSTHRU_VECTOR_VERT_SHADER, render::VECTOR_GEOM_SHADER, render::VECTOR_FRAG_SHADER}, DrawMode::Points);

  // Fill buffers
  std::vector<glm::vec3> mappedVectors, mappedRoots;
  for (size_t iV = 0; iV < vectors.size(); ++iV) {
    if (!hideWithMesh || !skipVector(iV)) {
      mappedVectors.push_back(mapper.map(vectors[iV]));
      mappedRoots.push_back(vectorRoots[iV]);
    }
  }

  program->setAttribute("a_vector", mappedVectors);
  program->setAttribute("a_position", mappedRoots);

  render::engine->setMaterial(*program, getMaterial());
}

bool TetVertexVectorQuantity::skipVector(size_t iV) {
  glm::vec3 pos = parent.vertices[iV];
  return glm::dot(pos, parent.sliceNormal) > parent.sliceDist;
}

bool TetFaceVectorQuantity::skipVector(size_t iF) {
  size_t iT = iF / 4;
  glm::vec3 pos = parent.tetCenters[iT];
  return glm::dot(pos, parent.sliceNormal) > parent.sliceDist;
}

bool TetTetVectorQuantity::skipVector(size_t iT) {
  glm::vec3 pos = parent.tetCenters[iT];
  return glm::dot(pos, parent.sliceNormal) > parent.sliceDist;
}


void TetVectorQuantity::buildCustomUI() {
  ImGui::SameLine();
  ImGui::ColorEdit3("Color", (float*)&vectorColor, ImGuiColorEditFlags_NoInputs);
  ImGui::SameLine();


  // === Options popup
  if (ImGui::Button("Options")) {
    ImGui::OpenPopup("OptionsPopup");
  }
  if (ImGui::BeginPopup("OptionsPopup")) {
    if (ImGui::MenuItem("Write to file")) writeToFile();
    ImGui::EndPopup();
  }


  // Only get to set length for non-ambient vectors
  if (vectorType != VectorType::AMBIENT) {
    if (ImGui::SliderFloat("Length", vectorLengthMult.get().getValuePtr(), 0.0, .1, "%.5f", 3.)) {
      vectorLengthMult.manuallyChanged();
      requestRedraw();
    }
  }

  if (ImGui::SliderFloat("Radius", vectorRadius.get().getValuePtr(), 0.0, .1, "%.5f", 3.)) {
    vectorRadius.manuallyChanged();
    requestRedraw();
  }

  { // Draw max and min magnitude
    ImGui::TextUnformatted(mapper.printBounds().c_str());
  }

  if (ImGui::Checkbox("hide with mesh", &hideWithMesh)) {
    geometryChanged();
  }

  drawSubUI();
}

void TetVectorQuantity::drawSubUI() {}

TetVectorQuantity* TetVectorQuantity::setVectorLengthScale(double newLength, bool isRelative) {
  vectorLengthMult = ScaledValue<double>(newLength, isRelative);
  requestRedraw();
  return this;
}
double TetVectorQuantity::getVectorLengthScale() { return vectorLengthMult.get().asAbsolute(); }

TetVectorQuantity* TetVectorQuantity::setVectorRadius(double val, bool isRelative) {
  vectorRadius = ScaledValue<double>(val, isRelative);
  requestRedraw();
  return this;
}
double TetVectorQuantity::getVectorRadius() { return vectorRadius.get().asAbsolute(); }

TetVectorQuantity* TetVectorQuantity::setVectorColor(glm::vec3 color) {
  vectorColor = color;
  requestRedraw();
  return this;
}
glm::vec3 TetVectorQuantity::getVectorColor() { return vectorColor.get(); }

TetVectorQuantity* TetVectorQuantity::setMaterial(std::string m) {
  material = m;
  if (program) render::engine->setMaterial(*program, getMaterial());
  if (ribbonArtist && ribbonArtist->program) render::engine->setMaterial(*ribbonArtist->program, material.get());
  requestRedraw();
  return this;
}
std::string TetVectorQuantity::getMaterial() { return material.get(); }

TetVectorQuantity* TetVectorQuantity::setRibbonEnabled(bool val) {
  ribbonEnabled = val;
  requestRedraw();
  return this;
}
bool TetVectorQuantity::isRibbonEnabled() { return ribbonEnabled.get(); }

void TetVectorQuantity::writeToFile(std::string filename) {

  if (filename == "") {
    filename = promptForFilename();
    if (filename == "") {
      return;
    }
  }

  if (options::verbosity > 0) {
    cout << "Writing tet vector quantity " << name << " to file " << filename << endl;
  }

  std::ofstream outFile(filename);
  outFile << "#Vectors written by polyscope from Tet Vector Quantity " << name << endl;
  outFile << "#displayradius " << (vectorRadius.get().asAbsolute()) << endl;
  outFile << "#displaylength " << (vectorLengthMult.get().asAbsolute()) << endl;

  for (size_t i = 0; i < vectors.size(); i++) {
    if (glm::length(vectors[i]) > 0) {
      outFile << vectorRoots[i] << " " << vectors[i] << endl;
    }
  }

  outFile.close();
}

// ========================================================
// ==========           Vertex Vector            ==========
// ========================================================

TetVertexVectorQuantity::TetVertexVectorQuantity(std::string name, std::vector<glm::vec3> vectors_, TetMesh& mesh_,
                                                 VectorType vectorType_)

    : TetVectorQuantity(name, mesh_, MeshElement::VERTEX, vectorType_), vectorField(vectors_) {

  size_t i = 0;
  vectorRoots = parent.vertices;
  vectors = vectorField;

  prepareVectorMapper();
}

void TetVertexVectorQuantity::buildVertexInfoGUI(size_t iV) {
  ImGui::TextUnformatted(name.c_str());
  ImGui::NextColumn();

  std::stringstream buffer;
  buffer << vectorField[iV];
  ImGui::TextUnformatted(buffer.str().c_str());

  ImGui::NextColumn();
  ImGui::NextColumn();
  ImGui::Text("magnitude: %g", glm::length(vectorField[iV]));
  ImGui::NextColumn();
}

std::string TetVertexVectorQuantity::niceName() { return name + " (vertex vector)"; }

// ========================================================
// ==========            Face Vector             ==========
// ========================================================

TetFaceVectorQuantity::TetFaceVectorQuantity(std::string name, std::vector<glm::vec3> vectors_, TetMesh& mesh_,
                                             VectorType vectorType_)
    : TetVectorQuantity(name, mesh_, MeshElement::FACE, vectorType_), vectorField(vectors_) {

  // Copy the vectors
  vectors = vectorField;
  vectorRoots.resize(parent.nFaces());
  for (size_t iF = 0; iF < parent.nFaces(); iF++) {
    auto& face = parent.faces[iF];
    glm::vec3 faceCenter = parent.faceCenter(iF);
    vectorRoots[iF] = faceCenter;
  }

  prepareVectorMapper();
}

void TetFaceVectorQuantity::buildFaceInfoGUI(size_t iF) {
  ImGui::TextUnformatted(name.c_str());
  ImGui::NextColumn();

  std::stringstream buffer;
  buffer << vectorField[iF];
  ImGui::TextUnformatted(buffer.str().c_str());

  ImGui::NextColumn();
  ImGui::NextColumn();
  ImGui::Text("magnitude: %g", glm::length(vectorField[iF]));
  ImGui::NextColumn();
}

std::string TetFaceVectorQuantity::niceName() { return name + " (face vector)"; }

// ========================================================
// ==========            Tet Vector             ==========
// ========================================================

TetTetVectorQuantity::TetTetVectorQuantity(std::string name, std::vector<glm::vec3> vectors_, TetMesh& mesh_,
                                           VectorType vectorType_)
    // TODO: fix enum to remove hack
    : TetVectorQuantity(name, mesh_, MeshElement::CORNER, vectorType_), vectorField(vectors_) {

  // Copy the vectors
  vectors = vectorField;
  vectorRoots.resize(parent.nTets());
  for (size_t iT = 0; iT < parent.nTets(); iT++) {
    auto& tet = parent.tets[iT];
    glm::vec3 tetCenter = parent.tetCenters[iT];
    vectorRoots[iT] = tetCenter;
  }

  prepareVectorMapper();
}

void TetTetVectorQuantity::buildTetInfoGUI(size_t iT) {
  ImGui::TextUnformatted(name.c_str());
  ImGui::NextColumn();

  std::stringstream buffer;
  buffer << vectorField[iT];
  ImGui::TextUnformatted(buffer.str().c_str());

  ImGui::NextColumn();
  ImGui::NextColumn();
  ImGui::Text("magnitude: %g", glm::length(vectorField[iT]));
  ImGui::NextColumn();
}

std::string TetTetVectorQuantity::niceName() { return name + " (tet vector)"; }
} // namespace polyscope
