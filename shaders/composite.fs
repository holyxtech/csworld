#version 460 core

#define Point2 vec2
#define Point3 vec3
#define Vector2 vec2
#define Vector3 vec3
#define Vector4 vec4
#define float2 vec2
#define float3 vec3
#define int2 ivec2

layout (location = 0) out vec4 color;
layout (location = 1) out vec4 brightColor;

uniform sampler2D mainColor;
uniform sampler2D mainDepth;

uniform sampler2D waterColor;
uniform sampler2D waterDepth;
uniform sampler2D waterPosition;
uniform sampler2D waterNormal;

uniform mat4 uPixelProjection;

// -*- c++ -*-
// Copyright (c) 2014, Morgan McGuire and Michael Mara
// All rights reserved.
//
// From McGuire and Mara, Efficient GPU Screen-Space Ray Tracing, 
// Journal of Computer Graphics Techniques, 2014
//
// This software is open source under the "BSD 2-clause license":
//
//    Redistribution and use in source and binary forms, with or
//    without modification, are permitted provided that the following
//    conditions are met:
//
//    1. Redistributions of source code must retain the above
//    copyright notice, this list of conditions and the following
//    disclaimer.
//
//    2. Redistributions in binary form must reproduce the above
//    copyright notice, this list of conditions and the following
//    disclaimer in the documentation and/or other materials provided
//    with the distribution.
//
//    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
//    CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
//    INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
//    MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
//    CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
//    USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
//    AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
//    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
//    THE POSSIBILITY OF SUCH DAMAGE.

void swap(in out float a, in out float b) {
  float temp = a;
  a = b;
  b = temp;
}

float distanceSquared(vec2 a, vec2 b) {
  a -= b;
  return dot(a, a);
}

float linearizeDepth(float depth, float near, float far) {
  return 2.0 * near * far / (far + near - (2.0 * depth - 1.0) * (far - near));
}

/**
  \param csOrigin Camera-space ray origin, which must be 
  within the view volume and must have z > 0.01 and project within the valid screen rectangle

  \param csDirection Unit length camera-space ray direction

  \param projectToPixelMatrix A projection matrix that maps to pixel coordinates (not [-1, +1] normalized device coordinates)

  \param csZBuffer The depth or camera-space Z buffer, depending on the value of \a csZBufferIsHyperbolic

  \param csZThickness Camera space thickness to ascribe to each pixel in the depth buffer

  \param nearPlaneZ Positive number in LH coordinates (0.1...)

  \param stride Step in horizontal or vertical pixels between samples. This is a float
   because integer math is slow on GPUs, but should be set to an integer >= 1

  \param jitterFraction  Number between 0 and 1 for how far to bump the ray in stride units
    to conceal banding artifacts

  \param maxSteps Maximum number of iterations. Higher gives better images but may be slow

  \param maxRayTraceDistance Maximum camera-space distance to trace before returning a miss

  \param hitPixel Pixel coordinates of the first intersection with the scene

  \param csHitPoint Camera space location of the ray hit

  Single-layer

 */
bool traceScreenSpaceRay (
  Point3        csOrigin, 
  Vector3       csDirection,
  mat4x4        projectToPixelMatrix,
  sampler2D     csZBuffer,
  float         csZThickness,
  float         nearPlaneZ,
  float         stride,
  float         jitterFraction,
  float         maxSteps,
  in float      maxRayTraceDistance,
  out Point2    hitPixel,
  out Point3    csHitPoint) {
  
  // Clip ray to a near plane in 3D (doesn't have to be *the* near plane, although that would be a good idea)
  float rayLength =
    ((csOrigin.z + csDirection.z * maxRayTraceDistance) < nearPlaneZ) ?
    (nearPlaneZ - csOrigin.z) / csDirection.z :
    maxRayTraceDistance;
  Point3 csEndPoint = csDirection * rayLength + csOrigin;

  // Project into screen space
  Vector4 H0 = projectToPixelMatrix * Vector4(csOrigin, 1.0);
  Vector4 H1 = projectToPixelMatrix * Vector4(csEndPoint, 1.0);

  // There are a lot of divisions by w that can be turned into multiplications
  // at some minor precision loss...and we need to interpolate these 1/w values
  // anyway.
  //
  // Because the caller was required to clip to the near plane,
  // this homogeneous division (projecting from 4D to 2D) is guaranteed 
  // to succeed. 
  float k0 = 1.0 / H0.w;
  float k1 = 1.0 / H1.w;

  // Switch the original points to values that interpolate linearly in 2D
  Point3 Q0 = csOrigin * k0; 
  Point3 Q1 = csEndPoint * k1;

  // Screen-space endpoints
  Point2 P0 = H0.xy * k0;
  Point2 P1 = H1.xy * k1;

  // [Optional clipping to frustum sides here]

  // Initialize to off screen
  hitPixel = Point2(-1.0, -1.0);

  // If the line is degenerate, make it cover at least one pixel
  // to avoid handling zero-pixel extent as a special case later
  P1 += vec2((distanceSquared(P0, P1) < 0.0001) ? 0.01 : 0.0);

  Vector2 delta = P1 - P0;

  // Permute so that the primary iteration is in x to reduce
  // large branches later
  bool permute = false;
  if (abs(delta.x) < abs(delta.y)) {
    // More-vertical line. Create a permutation that swaps x and y in the output
    permute = true;

    // Directly swizzle the inputs
    delta = delta.yx;
    P1 = P1.yx;
    P0 = P0.yx;      
  }
  
  // From now on, "x" is the primary iteration direction and "y" is the secondary one

  float stepDirection = sign(delta.x);
  float invdx = stepDirection / delta.x;
  Vector2 dP = Vector2(stepDirection, invdx * delta.y);

  // Track the derivatives of Q and k
  Vector3 dQ = (Q1 - Q0) * invdx;
  float   dk = (k1 - k0) * invdx;

  // Scale derivatives by the desired pixel stride
  dP *= stride; dQ *= stride; dk *= stride;

  // Offset the starting values by the jitter fraction
  P0 += dP * jitterFraction; Q0 += dQ * jitterFraction; k0 += dk * jitterFraction;

  // Slide P from P0 to P1, (now-homogeneous) Q from Q0 to Q1, and k from k0 to k1
  Point3 Q = Q0;
  float  k = k0;

  // We track the ray depth at +/- 1/2 pixel to treat pixels as clip-space solid 
  // voxels. Because the depth at -1/2 for a given pixel will be the same as at 
  // +1/2 for the previous iteration, we actually only have to compute one value 
  // per iteration.
  float prevZMaxEstimate = csOrigin.z;
  float stepCount = 0.0;
  float rayZMax = prevZMaxEstimate, rayZMin = prevZMaxEstimate;
  float sceneZMax = rayZMax + 1e4;

  // P1.x is never modified after this point, so pre-scale it by 
  // the step direction for a signed comparison
  float end = P1.x * stepDirection;

  // We only advance the z field of Q in the inner loop, since
  // Q.xy is never used until after the loop terminates.

  for (
    Point2 P = P0;
    ((P.x * stepDirection) <= end) && 
    (stepCount < maxSteps) &&
    ((rayZMax < sceneZMax - csZThickness) ||
    (rayZMin > sceneZMax)) &&
    (sceneZMax != 0.0);
    P += dP, Q.z += dQ.z, k += dk, stepCount += 1.0) {
        
    hitPixel = permute ? P.yx : P;
    // The depth range that the ray covers within this loop
    // iteration.  Assume that the ray is moving in increasing z
    // and swap if backwards.  Because one end of the interval is
    // shared between adjacent iterations, we track the previous
    // value and then swap as needed to ensure correct ordering
    rayZMin = prevZMaxEstimate;
    // Compute the value at 1/2 pixel into the future
    rayZMax = (dQ.z * 0.5 + Q.z) / (dk * 0.5 + k);
    prevZMaxEstimate = rayZMax;
    if (rayZMin > rayZMax) { swap(rayZMin, rayZMax); }

    // Camera-space z of the background
    sceneZMax = texelFetch(csZBuffer, int2(hitPixel), 0).r;
    sceneZMax = linearizeDepth(sceneZMax, 0.1, 4000);
  } // pixel on ray

  Q.xy += dQ.xy * stepCount;
  csHitPoint = Q * (1.0 / k);

  // Matches the new loop condition:
  return (rayZMax >= sceneZMax - csZThickness) && (rayZMin <= sceneZMax);
}

void main() {
  ivec2 texcoord = ivec2(floor(gl_FragCoord.xy));
  vec4 main_depth = texelFetch(mainDepth, texcoord, 0);
  vec4 water_depth = texelFetch(waterDepth, texcoord, 0);
  vec4 main_color = texelFetch(mainColor, texcoord, 0);
 
  if (main_depth.r <= water_depth.r) {
    color = texelFetch(mainColor, texcoord, 0);
  } else {
    vec4 water_color = texelFetch(waterColor, texcoord, 0);
    vec3 waterPosition = texelFetch(waterPosition, texcoord, 0).rgb;
    vec3 waterNormal = texelFetch(waterNormal, texcoord, 0).rgb;  
  
    vec3 direction = normalize(reflect(waterPosition, normalize(waterNormal)));

    Point2 hitPixel;
    Point3 hitPoint;
    bool hit = traceScreenSpaceRay(
      waterPosition, direction, uPixelProjection, mainDepth, 0.01, 1, 10.0, 0.5, 100.0, 3000.0,
      hitPixel, hitPoint);
    
    if (hit) {
      ivec2 hitCoord = ivec2(floor(hitPixel));
      color = texelFetch(mainColor, hitCoord, 0);
    } else {
      color = water_color;
    }
    color = mix(main_color, color, 0.4);
    
  }

  float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
  if (brightness > 1.0)
    brightColor = vec4(color.rgb, 1.0);
  else
    brightColor = vec4(0.0, 0.0, 0.0, 1.0); 
}

