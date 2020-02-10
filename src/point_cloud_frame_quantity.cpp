// Copyright 2017-2019, Nicholas Sharp and the Polyscope contributors. http://polyscope.run.
#include "polyscope/point_cloud_frame_quantity.h"

#include "polyscope/file_helpers.h"
#include "polyscope/gl/materials/materials.h"
#include "polyscope/gl/shaders.h"
#include "polyscope/gl/shaders/frame_shaders.h"
#include "polyscope/polyscope.h"

#include "imgui.h"

#include <fstream>
#include <iostream>


using std::cout;
using std::endl;

namespace polyscope {

PointCloudFrameQuantity::PointCloudFrameQuantity(std::string name, std::vector<std::array<glm::vec3, 3>> frames_,
                                                 PointCloud& pointCloud_, bool cross_, VectorType vectorType_)

    : PointCloudQuantity(name, pointCloud_), cross(cross_), vectorType(vectorType_), frames(frames_) {

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

  // Default viz settings
  if (vectorType != VectorType::AMBIENT) {
    lengthMult = .02;
  } else {
    lengthMult = 1.0;
  }
  radiusMult = .0005;

  if (cross) {
    frameColors[0] = getNextUniqueColor();
  } else {
    frameColors = std::array<glm::vec3, 3>{getNextUniqueColor(), getNextUniqueColor(), getNextUniqueColor()};
  }
}

void PointCloudFrameQuantity::draw() {
  if (!enabled) return;

  if (program == nullptr) {
    createProgram();
  }

  // Set uniforms
  parent.setTransformUniforms(*program);

  program->setUniform("u_radius", radiusMult * state::lengthScale);
  if (cross) {
    program->setUniform("u_color_x", frameColors[0]);
    program->setUniform("u_color_y", frameColors[0]);
    program->setUniform("u_color_z", frameColors[0]);
  } else {
    program->setUniform("u_color_x", frameColors[0]);
    program->setUniform("u_color_y", frameColors[1]);
    program->setUniform("u_color_z", frameColors[2]);
  }
  program->setUniform("u_cross", cross);

  if (vectorType == VectorType::AMBIENT) {
    program->setUniform("u_lengthMult", 1.0);
  } else {
    program->setUniform("u_lengthMult", lengthMult * state::lengthScale);
  }

  program->draw();
}

void PointCloudFrameQuantity::createProgram() {
  program.reset(new gl::GLProgram(&gl::PASSTHRU_FRAME_VERT_SHADER, &gl::FRAME_GEOM_SHADER, &gl::SHINY_FRAME_FRAG_SHADER,
                                  gl::DrawMode::Points));

  // Fill buffers
  std::vector<glm::vec3> mappedFrames1;
  std::vector<glm::vec3> mappedFrames2;
  std::vector<glm::vec3> mappedFrames3;
  for (std::array<glm::vec3, 3> f : frames) {
    mappedFrames1.push_back(mapper.map(f[0]));
    mappedFrames2.push_back(mapper.map(f[1]));
    mappedFrames3.push_back(mapper.map(f[2]));
  }

  program->setAttribute("a_vector1", mappedFrames1);
  program->setAttribute("a_vector2", mappedFrames2);
  program->setAttribute("a_vector3", mappedFrames3);
  program->setAttribute("a_position", parent.points);

  setMaterialForProgram(*program, "wax");
}

void PointCloudFrameQuantity::geometryChanged() { program.reset(); }

void PointCloudFrameQuantity::buildCustomUI() {
  ImGui::SameLine();

  if (cross) {
    ImGui::ColorEdit3("Color X", (float*)&frameColors[0], ImGuiColorEditFlags_NoInputs);
  } else {
    ImGui::ColorEdit3("Color X", (float*)&frameColors[0], ImGuiColorEditFlags_NoInputs);
    ImGui::ColorEdit3("Color Y", (float*)&frameColors[1], ImGuiColorEditFlags_NoInputs);
    ImGui::ColorEdit3("Color Z", (float*)&frameColors[2], ImGuiColorEditFlags_NoInputs);
  }
  ImGui::SameLine();

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
    ImGui::SliderFloat("Length", &lengthMult, 0.0, 1.0, "%.5f", 3.);
  }

  ImGui::SliderFloat("Radius", &radiusMult, 0.0, .1, "%.5f", 3.);

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
  outFile << "#displayradius " << (radiusMult * state::lengthScale) << endl;
  outFile << "#displaylength " << (lengthMult * state::lengthScale) << endl;

  for (size_t i = 0; i < frames.size(); i++) {
    if (glm::length2(frames[i][0]) > 0 || glm::length2(frames[i][1]) > 0 || glm::length2(frames[i][2]) > 0) {
      outFile << parent.points[i] << " " << frames[i][0] << ", " << frames[i][1] << ", " << frames[i][2] << endl;
    }
  }

  outFile.close();
}

std::string PointCloudFrameQuantity::niceName() { return name + " (frame)"; }

} // namespace polyscope
