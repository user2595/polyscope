// Copyright 2017-2019, Nicholas Sharp and the Polyscope contributors. http://polyscope.run.
#pragma once

#include "polyscope/gl/shaders.h"

namespace polyscope {
namespace gl {

// clang-format off

static const VertShader SURFACE_TEXTURE_VERT_SHADER =  {

    // uniforms
    {
       {"u_modelView", GLData::Matrix44Float},
       {"u_projMatrix", GLData::Matrix44Float},
    },

    // attributes
    {
        {"a_position", GLData::Vector3Float},
        {"a_normal", GLData::Vector3Float},
        {"a_coord", GLData::Vector2Float},
    },

    // source
    POLYSCOPE_GLSL(150,
      uniform mat4 u_modelView;
      uniform mat4 u_projMatrix;
      in vec3 a_position;
      in vec3 a_normal;
      in vec2 a_coord;
      out vec3 Normal;
      out vec2 Coord;

      void main()
      {
          Normal = mat3(u_modelView) * a_normal;
          Coord = a_coord;
          gl_Position = u_projMatrix * u_modelView * vec4(a_position,1.);
      }
    )
};

static const FragShader SURFACE_TEXTURE_FRAG_SHADER = {

    // uniforms
    {
    },

    // attributes
    {
    },

    // textures
    {
     {"t_mat_r", TextureTarget::TwoD},
     {"t_mat_g", TextureTarget::TwoD},
     {"t_mat_b", TextureTarget::TwoD},
     {"t_image", TextureTarget::TwoD},
    },

    // output location
    "outputF",

    // source
    POLYSCOPE_GLSL(150,
      uniform sampler2D t_image;
      uniform sampler2D t_mat_r;
      uniform sampler2D t_mat_g;
      uniform sampler2D t_mat_b;
      in vec3 Normal;
      in vec2 Coord;
      out vec4 outputF;

      // Forward declarations of methods from <shaders/common.h>
      vec4 lightSurfaceMat(vec3 normal, vec3 color, sampler2D t_mat_r, sampler2D t_mat_g, sampler2D t_mat_b);

      void main()
      {
        vec3 color = texture(t_image, Coord).rgb;
        outputF = lightSurfaceMat(Normal, color, t_mat_r, t_mat_g, t_mat_b);
        /* outputF = texture(t_image, Coord); */
        /* outputF = vec4(Coord.x, Coord.y, 0.f, 1.f); */
      }

    )
};

// clang-format on

} // namespace gl
} // namespace polyscope
