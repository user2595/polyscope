// Copyright 2017-2019, Nicholas Sharp and the Polyscope contributors. http://polyscope.run.
#pragma once

#include "polyscope/gl/shaders.h"

namespace polyscope {
namespace gl {

// clang-format off


static const VertShader PASSTHRU_FRAME_VERT_SHADER = {
    // uniforms
    {
    }, 

    // attributes
    {
        {"a_position", GLData::Vector3Float},
        {"a_vector1", GLData::Vector3Float},
        {"a_vector2", GLData::Vector3Float},
        {"a_vector3", GLData::Vector3Float},
    },

    // source
    POLYSCOPE_GLSL(150,
        in vec3 a_position;
        in vec3 a_vector1;
        in vec3 a_vector2;
        in vec3 a_vector3;

        out vec3 vector1;
        out vec3 vector2;
        out vec3 vector3;

        void main()
        {
            gl_Position = vec4(a_position,1.0);
            vector1 = a_vector1;
            vector2 = a_vector2;
            vector3 = a_vector3;
        }
    )
};



static const GeomShader FRAME_GEOM_SHADER = {

    // uniforms
    {
        {"u_modelView", GLData::Matrix44Float},
        {"u_projMatrix", GLData::Matrix44Float},
        {"u_lengthMult", GLData::Float},
        {"u_radius", GLData::Float},
        {"u_color_x", GLData::Vector3Float},
        {"u_color_y", GLData::Vector3Float},
        {"u_color_z", GLData::Vector3Float},
        {"u_cross", GLData::Int},
    },

    // attributes
    {
    },

    // source
    POLYSCOPE_GLSL(150,
        layout(points) in;
        layout(triangle_strip, max_vertices=240) out;
        in vec3 vector1[];
        in vec3 vector2[];
        in vec3 vector3[];
        uniform mat4 u_modelView;
        uniform mat4 u_projMatrix;
        uniform float u_lengthMult;
        uniform float u_radius;
        uniform vec3 u_color_x;
        uniform vec3 u_color_y;
        uniform vec3 u_color_z;
        uniform int u_cross;
        out vec3 cameraNormal;
        out vec3 color;

        void drawVector(vec3 vector, vec3 vecColor) {
            mat4 PV = u_projMatrix * u_modelView;

            const int nTheta = 8;
            const float PI = 3.14159;
            const float delTheta = 2.*PI / nTheta;

            // Points along the central axis
            vec3 rootP = gl_in[0].gl_Position.xyz;
            vec3 capP = rootP + .8 * vector * u_lengthMult;
            vec3 tipP = rootP + vector * u_lengthMult;

            // Orthogonal basis
            const vec3 arbVec = vec3(0.129873, -.70892, .58972);
            vec3 radX = normalize(cross(vector, arbVec));
            vec3 radY = normalize(cross(vector, radX));

            // Generate each panel around the vector
            for(int iTheta = 0; iTheta < nTheta; iTheta++) {

                float theta0 = delTheta * iTheta;
                float theta1 = delTheta * (iTheta+1);

                float x0 = cos(theta0);
                float y0 = sin(theta0);
                float x1 = cos(theta1);
                float y1 = sin(theta1);

                vec3 norm0 = (x0 * radX + y0 * radY);
                vec3 norm1 = (x1 * radX + y1 * radY);

                { // Lower left
                    vec4 worldPos = vec4(rootP + norm0 * u_radius, 1.);
                    gl_Position = PV * worldPos;
                    cameraNormal = mat3(u_modelView) * norm0;
                    color = vecColor;
                    EmitVertex();
                }

                { // Lower right
                    vec4 worldPos = vec4(rootP + norm1 * u_radius, 1.);
                    gl_Position = PV * worldPos;
                    cameraNormal = mat3(u_modelView) * norm1;
                    color = vecColor;
                    EmitVertex();
                }

                { // Upper left
                    vec4 worldPos = vec4(capP + norm0 * u_radius, 1.);
                    gl_Position = PV * worldPos;
                    cameraNormal = mat3(u_modelView) * norm0;
                    color = vecColor;
                    EmitVertex();
                }

                { // Upper right
                    vec4 worldPos = vec4(capP + norm1 * u_radius, 1.);
                    gl_Position = PV * worldPos;
                    cameraNormal = mat3(u_modelView) * norm1;
                    color = vecColor;
                    EmitVertex();
                }

                { // Tip
                    vec3 tipNormal = normalize(norm0 + norm1);
                    vec4 worldPos = vec4(tipP, 1.);
                    gl_Position = PV * worldPos;
                    cameraNormal = mat3(u_modelView) * tipNormal;
                    color = vecColor;
                    EmitVertex();
                }

                EndPrimitive();
            }

        }

        void main()   {
          drawVector(vector1[0], u_color_x);
          drawVector(vector2[0], u_color_y);
          drawVector(vector3[0], u_color_z);
          if (u_cross > 0) {
            drawVector(-vector1[0], u_color_x);
            drawVector(-vector2[0], u_color_y);
            drawVector(-vector3[0], u_color_z);
          }
        }
    )
};



static const FragShader SHINY_FRAME_FRAG_SHADER = {
    
    // uniforms
    {
    }, 

    // attributes
    {
    },
    
    // textures 
    {
        {"t_mat_r", 2},
        {"t_mat_g", 2},
        {"t_mat_b", 2},
    },
    
    // output location
    "outputF",
 
    // source
    POLYSCOPE_GLSL(150,
        uniform sampler2D t_mat_r;
        uniform sampler2D t_mat_g;
        uniform sampler2D t_mat_b;
        in vec3 cameraNormal;
        in vec3 color;
        out vec4 outputF;

        // Forward declarations of methods from <shaders/common.h>
        vec4 lightSurfaceMat(vec3 normal, vec3 color, sampler2D t_mat_r, sampler2D t_mat_g, sampler2D t_mat_b);

        void main()
        {
           outputF = lightSurfaceMat(cameraNormal, color, t_mat_r, t_mat_g, t_mat_b);
        }
    )
};

// clang-format on

} // namespace gl
} // namespace polyscope
