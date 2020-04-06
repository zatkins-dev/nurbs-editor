#pragma once

#include "ShaderIF.h"

#include <map>
using std::map;

class ShaderIFManager {
  public:
    ShaderIF* get(const std::string&);
    bool createShader(const std::string&, ShaderIF::ShaderSpec[], size_t);

  private:
    map<std::string, ShaderIF*> shaders;
};