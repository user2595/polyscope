// Copyright 2017-2019, Nicholas Sharp and the Polyscope contributors. http://polyscope.run.
#include "polyscope/point_cloud_frame_quantity.h"

#include "polyscope/file_helpers.h"
#include "polyscope/polyscope.h"
#include "polyscope/render/shaders.h"

#include "imgui.h"

#include <fstream>
#include <iostream>


using std::cout;
using std::endl;

namespace polyscope {

PointCloudFrameQuantity::PointCloudFrameQuantity(std::string name, std::vector<std::array<glm::vec3, 3>> frames_,
                                                 PointCloud& pointCloud_, bool cross_, VectorType vectorType_)

    : PointCloudQuantity(name, pointCloud_), cross(cross_), vectorType(vectorType_), frames(frames_),
      frameLengthMult(uniquePrefix() + "#frameLengthMult",
                      vectorType == VectorType::AMBIENT ? absoluteValue(1.0) : relativeValue(0.02)),
      frameRadius(uniquePrefix() + "#frameRadius", relativeValue(0.0025)),
      frameColorX(uniquePrefix() + "#frameColorX", getNextUniqueColor()),
      frameColorY(uniquePrefix() + "#frameColorY", getNextUniqueColor()),
      frameColorZ(uniquePrefix() + "#frameColorZ", getNextUniqueColor()), showX(uniquePrefix() + "#frameShowX", true),
      showY(uniquePrefix() + "#frameShowY", true), showZ(uniquePrefix() + "#frameShowZ", true),
      drawCube(uniquePrefix() + "#frameDrawCube", false), material(uniquePrefix() + "#material", "clay") {

  if (frames.size() != parent.points.size()) {
    polyscope::error("Point cloud frame quantity " + name + " does not have same number of values (" +
                     std::to_string(frames.size()) + ") as point cloud size (" + std::to_string(parent.points.size()) +
                     ")");
  }

  std::vector<glm::vec3> totalVectorList;
  totalVectorList.reserve(3 * frames.size());
  for (std::array<glm::vec3, 3> f : frames) {
    totalVectorList.push_back(f[0]);
    totalVectorList.push_back(f[1]);
    totalVectorList.push_back(f[2]);
  }

  // Create a mapper (default mapper is identity)
  if (vectorType == VectorType::AMBIENT) {
    mapper.setMinMax(totalVectorList);
  } else {
    mapper = AffineRemapper<glm::vec3>(totalVectorList, DataType::MAGNITUDE);
  }
}

void PointCloudFrameQuantity::draw() {
  if (!isEnabled()) return;

  if (programX == nullptr) {
    createProgram();
  }


  if (drawCube.get()) {
    parent.setTransformUniforms(*programCube);

    if (vectorType == VectorType::AMBIENT) {
      programCube->setUniform("u_lengthMult", 1.0);
    } else {
      programCube->setUniform("u_lengthMult", frameLengthMult.get().asAbsolute());
    }

    programCube->setUniform("u_baseColorX", frameColorX.get());
    programCube->setUniform("u_baseColorY", frameColorY.get());
    programCube->setUniform("u_baseColorZ", frameColorZ.get());

    programCube->draw();
  } else {
    // Set uniforms
    parent.setTransformUniforms(*programX);
    parent.setTransformUniforms(*programY);
    parent.setTransformUniforms(*programZ);

    programX->setUniform("u_radius", frameRadius.get().asAbsolute());
    programY->setUniform("u_radius", frameRadius.get().asAbsolute());
    programZ->setUniform("u_radius", frameRadius.get().asAbsolute());

    if (cross) {
      programX->setUniform("u_baseColor", frameColorX.get());
      programY->setUniform("u_baseColor", frameColorX.get());
      programZ->setUniform("u_baseColor", frameColorX.get());
    } else {
      programX->setUniform("u_baseColor", frameColorX.get());
      programY->setUniform("u_baseColor", frameColorY.get());
      programZ->setUniform("u_baseColor", frameColorZ.get());
    }

    if (vectorType == VectorType::AMBIENT) {
      programX->setUniform("u_lengthMult", 1.0);
      programY->setUniform("u_lengthMult", 1.0);
      programZ->setUniform("u_lengthMult", 1.0);
    } else {
      programX->setUniform("u_lengthMult", frameLengthMult.get().asAbsolute());
      programY->setUniform("u_lengthMult", frameLengthMult.get().asAbsolute());
      programZ->setUniform("u_lengthMult", frameLengthMult.get().asAbsolute());
    }

    glm::mat4 P = view::getCameraPerspectiveMatrix();
    glm::mat4 Pinv = glm::inverse(P);
    programX->setUniform("u_invProjMatrix", glm::value_ptr(Pinv));
    programY->setUniform("u_invProjMatrix", glm::value_ptr(Pinv));
    programZ->setUniform("u_invProjMatrix", glm::value_ptr(Pinv));
    programX->setUniform("u_viewport", render::engine->getCurrentViewport());
    programY->setUniform("u_viewport", render::engine->getCurrentViewport());
    programZ->setUniform("u_viewport", render::engine->getCurrentViewport());

    if (showX.get()) {
      programX->draw();
    }
    if (showY.get()) {
      programY->draw();
    }
    if (showZ.get()) {
      programZ->draw();
    }
  }
}

void PointCloudFrameQuantity::createProgram() {
  programX = render::engine->generateShaderProgram(
      {render::PASSTHRU_VECTOR_VERT_SHADER, render::VECTOR_GEOM_SHADER, render::VECTOR_FRAG_SHADER}, DrawMode::Points);
  programY = render::engine->generateShaderProgram(
      {render::PASSTHRU_VECTOR_VERT_SHADER, render::VECTOR_GEOM_SHADER, render::VECTOR_FRAG_SHADER}, DrawMode::Points);
  programZ = render::engine->generateShaderProgram(
      {render::PASSTHRU_VECTOR_VERT_SHADER, render::VECTOR_GEOM_SHADER, render::VECTOR_FRAG_SHADER}, DrawMode::Points);
  programCube = render::engine->generateShaderProgram(
      {render::PASSTHRU_CUBE_VERT_SHADER, render::CUBE_GEOM_SHADER, render::CUBE_FRAG_SHADER}, DrawMode::Points);

  // Fill buffers
  std::vector<glm::vec3> mappedFrames1;
  std::vector<glm::vec3> mappedFrames2;
  std::vector<glm::vec3> mappedFrames3;
  for (std::array<glm::vec3, 3> f : frames) {
    mappedFrames1.push_back(mapper.map(f[0]));
    mappedFrames2.push_back(mapper.map(f[1]));
    mappedFrames3.push_back(mapper.map(f[2]));
  }

  programX->setAttribute("a_vector", mappedFrames1);
  programY->setAttribute("a_vector", mappedFrames2);
  programZ->setAttribute("a_vector", mappedFrames3);
  programX->setAttribute("a_position", parent.points);
  programY->setAttribute("a_position", parent.points);
  programZ->setAttribute("a_position", parent.points);

  programCube->setAttribute("a_vector_x", mappedFrames1);
  programCube->setAttribute("a_vector_y", mappedFrames2);
  programCube->setAttribute("a_vector_z", mappedFrames3);
  programCube->setAttribute("a_position", parent.points);

  render::engine->setMaterial(*programX, getMaterial());
  render::engine->setMaterial(*programY, getMaterial());
  render::engine->setMaterial(*programZ, getMaterial());
  render::engine->setMaterial(*programCube, getMaterial());
}

void PointCloudFrameQuantity::geometryChanged() {
  programX.reset();
  programY.reset();
  programZ.reset();
}

void PointCloudFrameQuantity::buildCustomUI() {

  ImGui::Checkbox("Draw Cube", &drawCube.get());

  if (cross) {
    if (ImGui::ColorEdit3("Color", &frameColorX.get()[0], ImGuiColorEditFlags_NoInputs))
      setFrameColors(getFrameColors());
  } else {

    if (!drawCube.get()) {
      ImGui::Checkbox("Draw X", &showX.get());
      ImGui::SameLine();
    }
    if (ImGui::ColorEdit3("Color X", &frameColorX.get()[0], ImGuiColorEditFlags_NoInputs))
      setFrameColors(getFrameColors());
    if (!drawCube.get()) {
      ImGui::Checkbox("Draw Y", &showY.get());
      ImGui::SameLine();
    }
    if (ImGui::ColorEdit3("Color Y", &frameColorY.get()[0], ImGuiColorEditFlags_NoInputs))
      setFrameColors(getFrameColors());
    if (!drawCube.get()) {
      ImGui::Checkbox("Draw Z", &showZ.get());
      ImGui::SameLine();
    }
    if (ImGui::ColorEdit3("Color Z", &frameColorZ.get()[0], ImGuiColorEditFlags_NoInputs))
      setFrameColors(getFrameColors());
  }

  // === Options popup
  if (ImGui::Button("Options")) {
    ImGui::OpenPopup("OptionsPopup");
  }
  if (ImGui::BeginPopup("OptionsPopup")) {
    if (ImGui::MenuItem("Write to file")) writeToFile();
    ImGui::EndPopup();
  }

  // Only get to set length for non-ambient frames
  if (vectorType != VectorType::AMBIENT) {
    if (ImGui::SliderFloat("Length", frameLengthMult.get().getValuePtr(), 0.0, .1, "%.5f", 3.)) {
      frameLengthMult.manuallyChanged();
      requestRedraw();
    }
  }

  if (!drawCube.get()) {
    if (ImGui::SliderFloat("Radius", frameRadius.get().getValuePtr(), 0.0, .1, "%.5f", 3.)) {
      frameRadius.manuallyChanged();
      requestRedraw();
    }
  }

  { // Draw max and min magnitude
    ImGui::TextUnformatted(mapper.printBounds().c_str());
  }
}

void PointCloudFrameQuantity::buildPickUI(size_t ind) {
  ImGui::TextUnformatted(name.c_str());
  ImGui::NextColumn();

  std::stringstream buffer;
  buffer << frames[ind][0] << ", " << frames[ind][1] << ", " << frames[ind][2];
  ImGui::TextUnformatted(buffer.str().c_str());

  ImGui::NextColumn();
  ImGui::NextColumn();
  ImGui::Text("magnitude: %g, %g, %g", glm::length(frames[ind][0]), glm::length(frames[ind][1]),
              glm::length(frames[ind][2]));
  ImGui::NextColumn();
}

void PointCloudFrameQuantity::writeToFile(std::string filename) {

  if (filename == "") {
    filename = promptForFilename();
    if (filename == "") {
      return;
    }
  }

  cout << "Writing surface frame quantity " << name << " to file " << filename << endl;

  std::ofstream outFile(filename);
  outFile << "#Frames written by polyscope from Point Cloud Frame Quantity " << name << endl;
  outFile << "#displayradius " << frameRadius.get().asAbsolute() << endl;
  outFile << "#displaylength " << frameLengthMult.get().asAbsolute() << endl;

  for (size_t i = 0; i < frames.size(); i++) {
    if (glm::length2(frames[i][0]) > 0 || glm::length2(frames[i][1]) > 0 || glm::length2(frames[i][2]) > 0) {
      outFile << parent.points[i] << " " << frames[i][0] << ", " << frames[i][1] << ", " << frames[i][2] << endl;
    }
  }

  outFile.close();
}


PointCloudFrameQuantity* PointCloudFrameQuantity::setFrameLengthScale(double newLength, bool isRelative) {
  frameLengthMult = ScaledValue<double>(newLength, isRelative);
  requestRedraw();
  return this;
}
double PointCloudFrameQuantity::getFrameLengthScale() { return frameLengthMult.get().asAbsolute(); }
PointCloudFrameQuantity* PointCloudFrameQuantity::setFrameRadius(double val, bool isRelative) {
  frameRadius = ScaledValue<double>(val, isRelative);
  requestRedraw();
  return this;
}
double PointCloudFrameQuantity::getFrameRadius() { return frameRadius.get().asAbsolute(); }
PointCloudFrameQuantity* PointCloudFrameQuantity::setFrameColors(std::array<glm::vec3, 3> colors) {
  frameColorX = colors[0];
  frameColorY = colors[1];
  frameColorZ = colors[2];

  requestRedraw();
  return this;
}
std::array<glm::vec3, 3> PointCloudFrameQuantity::getFrameColors() {
  return std::array<glm::vec3, 3>{frameColorX.get(), frameColorY.get(), frameColorZ.get()};
}

PointCloudFrameQuantity* PointCloudFrameQuantity::setMaterial(std::string m) {
  material = m;
  if (programX) {
    render::engine->setMaterial(*programX, getMaterial());
    render::engine->setMaterial(*programY, getMaterial());
    render::engine->setMaterial(*programZ, getMaterial());
  }
  requestRedraw();
  return this;
}
std::string PointCloudFrameQuantity::getMaterial() { return material.get(); }

std::string PointCloudFrameQuantity::niceName() { return name + " (frame)"; }

} // namespace polyscope
