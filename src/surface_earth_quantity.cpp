// Copyright 2017-2019, Nicholas Sharp and the Polyscope contributors. http://polyscope.run.
#include "polyscope/surface_earth_quantity.h"

#include "polyscope/file_helpers.h"
#include "polyscope/gl/materials/materials.h"
#include "polyscope/gl/shaders.h"
#include "polyscope/gl/shaders/earth_shaders.h"
#include "polyscope/polyscope.h"

#include "imgui.h"
#include "stb_image.h"

using std::cout;
using std::endl;

namespace polyscope {

SurfaceEarthQuantity::SurfaceEarthQuantity(std::string name, SurfaceMesh& mesh_, std::vector<glm::vec3> values_,
                                           DataType dataType_)
    : SurfaceMeshQuantity(name, mesh_, true), dataType(dataType_), values(values_) {
  for (size_t i = 0; i < values.size(); ++i) {
    scaleFactors.emplace_back(0);
  }
}
SurfaceEarthQuantity::SurfaceEarthQuantity(std::string name, SurfaceMesh& mesh_, std::vector<glm::vec3> values_,
                                           std::vector<double> scaleFactors_, DataType dataType_)
    : SurfaceMeshQuantity(name, mesh_, true), dataType(dataType_), values(values_) {
  for (size_t i = 0; i < values.size(); ++i) {
    scaleFactors.emplace_back((float)scaleFactors_[i]);
  }
}

void SurfaceEarthQuantity::draw() {
  if (!enabled) return;

  if (program == nullptr) {
    createProgram();
  }

  // Set uniforms
  parent.setTransformUniforms(*program);
  program->setUniform("u_projectiveInterpolate", (int)projectiveInterpolate);

  program->draw();
}

// Update range uniforms
void SurfaceEarthQuantity::setProgramTextures(gl::GLProgram& program) {

  // Load the texture
  int w, h, comp;
  // unsigned char* image = nullptr;
  std::array<unsigned char*, 6> images;
  std::array<std::string, 6> filenames;
  filenames[0] = "/Users/mark/Desktop/EarthCube/R_earth-cube.png";
  filenames[1] = "/Users/mark/Desktop/EarthCube/L_earth-cube.png";
  filenames[2] = "/Users/mark/Desktop/EarthCube/U_earth-cube.png";
  filenames[3] = "/Users/mark/Desktop/EarthCube/D_earth-cube.png";
  filenames[4] = "/Users/mark/Desktop/EarthCube/F_earth-cube.png";
  filenames[5] = "/Users/mark/Desktop/EarthCube/B_earth-cube.png";

  for (size_t i = 0; i < 6; ++i) {
    // stbi_set_flip_vertically_on_load(true);
    images[i] = stbi_load(filenames[i].c_str(), &w, &h, &comp, STBI_rgb);
    if (images[i] == nullptr) throw std::logic_error("Failed to load " + filenames[i]);
  }
  program.setTextureCube("t_earth", images, w, false, false, false);
}

void SurfaceEarthQuantity::buildCustomUI() { ImGui::Checkbox("Projective Interpolate", &projectiveInterpolate); }

void SurfaceEarthQuantity::geometryChanged() { program.reset(); }

void SurfaceEarthQuantity::createProgram() {
  // Create the program to draw this quantity
  program.reset(new gl::GLProgram(&gl::EARTH_DRAW_VERT_SHADER, &gl::EARTH_DRAW_FRAG_SHADER, gl::DrawMode::Triangles));

  // Fill color buffers
  parent.fillGeometryBuffers(*program);
  fillPositionBuffers(*program);
  setProgramTextures(*program);

  setMaterialForProgram(*program, "wax");
}


void SurfaceEarthQuantity::fillPositionBuffers(gl::GLProgram& p) {
  std::vector<glm::vec3> texCoord0, texCoord1, texCoord2, baryCoords, scaleFactor;
  texCoord0.reserve(3 * parent.nFacesTriangulation());
  texCoord1.reserve(3 * parent.nFacesTriangulation());
  texCoord2.reserve(3 * parent.nFacesTriangulation());
  baryCoords.reserve(3 * parent.nFacesTriangulation());
  scaleFactor.reserve(3 * parent.nFacesTriangulation());

  glm::vec3 zero{0.f, 0.f, 0.f};

  for (size_t iF = 0; iF < parent.nFaces(); iF++) {
    auto& face = parent.faces[iF];
    size_t D = face.size();

    // implicitly triangulate from root
    size_t vRoot = face[0];
    for (size_t j = 1; (j + 1) < D; j++) {
      size_t vB = face[j];
      size_t vC = face[(j + 1) % D];

      // val.push_back(values[vRoot]);
      // val.push_back(values[vB]);
      // val.push_back(values[vC]);
      texCoord0.emplace_back(values[vRoot]);
      texCoord0.emplace_back(zero);
      texCoord0.emplace_back(zero);

      texCoord1.emplace_back(zero);
      texCoord1.emplace_back(values[vB]);
      texCoord1.emplace_back(zero);

      texCoord2.emplace_back(zero);
      texCoord2.emplace_back(zero);
      texCoord2.emplace_back(values[vC]);

      baryCoords.emplace_back(glm::vec3{1.f, 0.f, 0.f});
      baryCoords.emplace_back(glm::vec3{0.f, 1.f, 0.f});
      baryCoords.emplace_back(glm::vec3{0.f, 0.f, 1.f});

      scaleFactor.emplace_back(glm::vec3{scaleFactors[vRoot], 0.f, 0.f});
      scaleFactor.emplace_back(glm::vec3{0.f, scaleFactors[vB], 0.f});
      scaleFactor.emplace_back(glm::vec3{0.f, 0.f, scaleFactors[vC]});
    }
  }

  // Store data in buffers
  p.setAttribute("a_barycoord", baryCoords);
  p.setAttribute("a_scale_factor", scaleFactor);
  p.setAttribute("a_texture_coord0", texCoord0);
  p.setAttribute("a_texture_coord1", texCoord1);
  p.setAttribute("a_texture_coord2", texCoord2);
}

void SurfaceEarthQuantity::writeToFile(std::string filename) {
  polyscope::warning("Writing to file not yet implemented for this datatype");
}
std::string SurfaceEarthQuantity::niceName() { return name; }

} // namespace polyscope
