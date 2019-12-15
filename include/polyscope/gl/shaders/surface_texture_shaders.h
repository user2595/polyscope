// Copyright 2017-2019, Nicholas Sharp and the Polyscope contributors. http://polyscope.run.
#pragma once

#include "polyscope/gl/shaders.h"

// clang-format off

namespace polyscope {
namespace gl {

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
        {"a_tcoord", GLData::Vector2Float},
    },

    // source
    POLYSCOPE_GLSL(150,
      uniform mat4 u_modelView;
      uniform mat4 u_projMatrix;
      in vec3 a_position;
      in vec3 a_normal;
      in vec2 a_tcoord;
      out vec2 tCoord;
      out vec3 Normal;

      void main()
      {
        Normal = mat3(u_modelView) * a_normal;
        gl_Position = u_projMatrix * u_modelView * vec4(a_position,1.);
        tCoord = a_tcoord;
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
      in vec2 tCoord;
      out vec4 outputF;

      // Forward declarations of methods from <shaders/common.h>
      vec4 lightSurfaceMat(vec3 normal, vec3 color, sampler2D t_mat_r, sampler2D t_mat_g, sampler2D t_mat_b);

      void main()
      {
        vec3 color = texture(t_image, tCoord).rgb;
        outputF = lightSurfaceMat(Normal, color, t_mat_r, t_mat_g, t_mat_b);
        outputF = texture(t_image, tCoord * 100.f);
      }

    )
};

// clang-format on

} // namespace gl
} // namespace polyscope
