#include "helpers.h"
#include <iostream>
#include <iomanip>

glm::mat4 CAD::translate(glm::mat4 matrix, glm::vec3 vector) {
  glm::mat4 translationMatrix = glm::mat4(1.0f);
  translationMatrix[3] = glm::vec4(vector, 1.0f);
  return translationMatrix * matrix;
}

glm::mat4 CAD::createXrotationMatrix(float angle) {
  glm::mat4 result = glm::mat4(1.0f);
  result[1][1] = cos(angle);
  result[1][2] = sin(angle);
  result[2][1] = -sin(angle);
  result[2][2] = cos(angle);
  return result;
}

glm::mat4 CAD::createYrotationMatrix(float angle) {
  glm::mat4 result = glm::mat4(1.0f);
  result[0][0] = cos(angle);
  result[0][2] = -sin(angle);
  result[2][0] = sin(angle);
  result[2][2] = cos(angle);
  return result;
}

glm::mat4 CAD::createZrotationMatrix(float angle) {
  glm::mat4 result = glm::mat4(1.0f);
  result[0][0] = cos(angle);
  result[0][1] = sin(angle);
  result[1][0] = -sin(angle);
  result[1][1] = cos(angle);
  return result;
}

glm::mat4 CAD::rotate(glm::mat4 matrix, glm::vec3 angle) {
  return CAD::createZrotationMatrix(angle.z) *
         CAD::createYrotationMatrix(angle.y) *
         CAD::createXrotationMatrix(angle.x) * matrix;
}

glm::mat4 CAD::projection(float fov, float ratio, float near,
                                 float far) {
  glm::mat4 result = glm::mat4(0.0f);
  result[0][0] = 1.0f / (tan(fov / 2.0f) * ratio);
  result[1][1] = 1.0f / tan(fov / 2.0f);
  result[2][2] = -(far + near) / (far - near);
  result[3][2] = (-2.0f * far * near) / (far - near);
  result[2][3] = -1.0f;
  result[3][3] = 0.0f;
  return result;
}

glm::mat4 CAD::scaling(glm::mat4 matrix, glm::vec3 scale) {
  glm::mat4 scaleMatrix = glm::mat4(1.0f);
  scaleMatrix[0][0] = scale.x;
  scaleMatrix[1][1] = scale.y;
  scaleMatrix[2][2] = scale.z;
  return scaleMatrix * matrix;
}

void CAD::printVector(glm::vec3 vec) {
  std::cout << "{ " << vec.x << " " << vec.y << " " << vec.z << " }"
            << std::endl;
}


glm::mat4 CAD::lookAt(glm::vec3 position, glm::vec3 target, glm::vec3 up) {

  glm::vec3 f = glm::vec3(glm::normalize(target - position));
  glm::vec3 r = glm::vec3(glm::normalize(glm::cross(f, up)));
  glm::vec3 u = glm::vec3(glm::cross(r, f));
  glm::vec3 t = glm::vec3(-glm::dot(r, position), -glm::dot(u, position),
                          glm::dot(f, position));

  glm::mat4 result = glm::mat4(0.f);

  result[0][0] = r.x;
  result[1][0] = r.y;
  result[2][0] = r.z;

  result[0][1] = u.x;
  result[1][1] = u.y;
  result[2][1] = u.z;

  result[0][2] = -f.x;
  result[1][2] = -f.y;
  result[2][2] = -f.z;

  result[3] = glm::vec4(t, 1.f);
  return result;
}

void CAD::printMatrix(glm::mat4 mat) 
{
  for (int i = 0; i < 4; i++) {
    std::cout << "| ";
    for (int j = 0; j < 4; j++) {
      std::cout << std::left << std::setfill(' ') << std::setw(10) << mat[j][i]
                << " ";
    }
    std::cout << " |" << std::endl;
  }
}

float CAD::angleBetweenVectors(glm::vec3 u, glm::vec3 v) 
{ 
    return glm::acos(glm::clamp(glm::dot(u, v) / (glm::length(u) * glm::length(v)),-1.f,1.f));
}

std::string CAD::printPosition(glm::vec3 pos, std::string name) {
  return name + "X: " + std::to_string(pos.x) +
         ", Y:" + std::to_string(pos.y) + ", Z:" + std::to_string(pos.z);
}

