// Copyright 2017-2019, Nicholas Sharp and the Polyscope contributors. http://polyscope.run.
#include "polyscope/surface_normal_quantity.h"

#include "polyscope/file_helpers.h"
#include "polyscope/polyscope.h"
#include "polyscope/render/materials.h"
#include "polyscope/render/shaders.h"

#include "imgui.h"

using std::cout;
using std::endl;

namespace polyscope {

SurfaceNormalQuantity::SurfaceNormalQuantity(std::string name, SurfaceMesh& mesh_, std::string definedOn_)
    : SurfaceMeshQuantity(name, mesh_, true), definedOn(definedOn_) {}

void SurfaceNormalQuantity::draw() {
  if (!isEnabled()) return;

  if (program == nullptr) {
    createProgram();
  }

  // Set uniforms
  parent.setTransformUniforms(*program);
  program->setUniform("u_basecolor", parent.getSurfaceColor());
  setProgramUniforms(*program);


  program->draw();
}

void SurfaceNormalQuantity::writeToFile(std::string filename) {
  polyscope::warning("Writing to file not yet implemented for this datatype");
}


// Update range uniforms
void SurfaceNormalQuantity::setProgramUniforms(render::ShaderProgram& program) {}

void SurfaceNormalQuantity::buildCustomUI() {
  ImGui::SameLine();

  // == Options popup
  if (ImGui::Button("Options")) {
    ImGui::OpenPopup("OptionsPopup");
  }
  if (ImGui::BeginPopup("OptionsPopup")) {

    if (ImGui::MenuItem("Write to file")) writeToFile();

    ImGui::EndPopup();
  }
}

void SurfaceNormalQuantity::geometryChanged() { program.reset(); }

std::string SurfaceNormalQuantity::niceName() { return name + " (" + definedOn + " normal)"; }

// ========================================================
// ==========           Vertex Normal            ==========
// ========================================================

SurfaceVertexNormalQuantity::SurfaceVertexNormalQuantity(std::string name, std::vector<glm::vec3> normalValues_,
                                                         SurfaceMesh& mesh_)
    : SurfaceNormalQuantity(name, mesh_, "vertex"), normalValues(std::move(normalValues_))

{}

void SurfaceVertexNormalQuantity::createProgram() {
  // Create the program to draw this quantity
  program = render::engine->generateShaderProgram(
      {render::PLAIN_SURFACE_VERT_SHADER, render::PLAIN_SURFACE_FRAG_SHADER}, DrawMode::Triangles);

  // Fill geometry buffers
  fillGeometryBuffers(*program);

  render::engine->setMaterial(*program, parent.getMaterial());
}

void SurfaceVertexNormalQuantity::fillGeometryBuffers(render::ShaderProgram& p) {
  std::vector<glm::vec3> positions;
  std::vector<glm::vec3> normals;
  std::vector<glm::vec3> bcoord;

  bool wantsBary = p.hasAttribute("a_barycoord");

  // Reserve space
  positions.reserve(3 * parent.nFacesTriangulation());
  normals.reserve(3 * parent.nFacesTriangulation());
  if (wantsBary) {
    bcoord.reserve(3 * parent.nFacesTriangulation());
  }

  for (size_t iF = 0; iF < parent.nFaces(); iF++) {
    auto& face = parent.faces[iF];
    size_t D = face.size();

    // implicitly triangulate from root
    size_t vRoot = face[0];
    glm::vec3 pRoot = parent.vertices[vRoot];
    for (size_t j = 1; (j + 1) < D; j++) {
      size_t vB = face[j];
      glm::vec3 pB = parent.vertices[vB];
      size_t vC = face[(j + 1) % D];
      glm::vec3 pC = parent.vertices[vC];

      positions.push_back(pRoot);
      positions.push_back(pB);
      positions.push_back(pC);

      normals.push_back(normalValues[vRoot]);
      normals.push_back(normalValues[vB]);
      normals.push_back(normalValues[vC]);

      if (wantsBary) {
        bcoord.push_back(glm::vec3{1., 0., 0.});
        bcoord.push_back(glm::vec3{0., 1., 0.});
        bcoord.push_back(glm::vec3{0., 0., 1.});
      }
    }
  }

  // Store data in buffers
  p.setAttribute("a_position", positions);
  p.setAttribute("a_normal", normals);
  if (wantsBary) {
    p.setAttribute("a_barycoord", bcoord);
  }
}

void SurfaceVertexNormalQuantity::writeToFile(std::string filename) { throw std::runtime_error("not implemented"); }

void SurfaceVertexNormalQuantity::buildVertexInfoGUI(size_t vInd) {
  ImGui::TextUnformatted(name.c_str());
  ImGui::NextColumn();
  ImGui::Text("(%g, %g, %g)", normalValues[vInd].x, normalValues[vInd].y, normalValues[vInd].z);
  ImGui::NextColumn();
}

// ========================================================
// ==========            Face Normal             ==========
// ========================================================

SurfaceFaceNormalQuantity::SurfaceFaceNormalQuantity(std::string name, std::vector<glm::vec3> normalValues_,
                                                     SurfaceMesh& mesh_)
    : SurfaceNormalQuantity(name, mesh_, "face"), normalValues(std::move(normalValues_))

{}

void SurfaceFaceNormalQuantity::createProgram() {
  // Create the program to draw this quantity
  program = render::engine->generateShaderProgram(
      {render::PLAIN_SURFACE_VERT_SHADER, render::PLAIN_SURFACE_FRAG_SHADER}, DrawMode::Triangles);

  // Fill color buffers
  fillGeometryBuffers(*program);

  render::engine->setMaterial(*program, parent.getMaterial());
}

void SurfaceFaceNormalQuantity::fillGeometryBuffers(render::ShaderProgram& p) {
  std::vector<glm::vec3> positions;
  std::vector<glm::vec3> normals;
  std::vector<glm::vec3> bcoord;

  bool wantsBary = p.hasAttribute("a_barycoord");

  positions.reserve(3 * parent.nFacesTriangulation());
  normals.reserve(3 * parent.nFacesTriangulation());
  if (wantsBary) {
    bcoord.reserve(3 * parent.nFacesTriangulation());
  }

  for (size_t iF = 0; iF < parent.nFaces(); iF++) {
    auto& face = parent.faces[iF];
    size_t D = face.size();
    glm::vec3 faceN = normalValues[iF];

    // implicitly triangulate from root
    size_t vRoot = face[0];
    glm::vec3 pRoot = parent.vertices[vRoot];
    for (size_t j = 1; (j + 1) < D; j++) {
      size_t vB = face[j];
      glm::vec3 pB = parent.vertices[vB];
      size_t vC = face[(j + 1) % D];
      glm::vec3 pC = parent.vertices[vC];

      positions.push_back(pRoot);
      positions.push_back(pB);
      positions.push_back(pC);

      normals.push_back(faceN);
      normals.push_back(faceN);
      normals.push_back(faceN);

      if (wantsBary) {
        bcoord.push_back(glm::vec3{1., 0., 0.});
        bcoord.push_back(glm::vec3{0., 1., 0.});
        bcoord.push_back(glm::vec3{0., 0., 1.});
      }
    }
  }


  // Store data in buffers
  p.setAttribute("a_position", positions);
  p.setAttribute("a_normal", normals);
  if (wantsBary) {
    p.setAttribute("a_barycoord", bcoord);
  }
}

void SurfaceFaceNormalQuantity::buildFaceInfoGUI(size_t fInd) {
  ImGui::TextUnformatted(name.c_str());
  ImGui::NextColumn();
  ImGui::Text("(%g, %g, %g)", normalValues[fInd].x, normalValues[fInd].y, normalValues[fInd].z);
  ImGui::NextColumn();
}

} // namespace polyscope
