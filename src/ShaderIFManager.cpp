#include "ShaderIFManager.h"

ShaderIF* ShaderIFManager::get(const std::string& shaderName) { return shaders[shaderName]; }
bool ShaderIFManager::createShader(const std::string& name, ShaderIF::ShaderSpec shaderSpecs[],
                                   size_t n) {
    ShaderIF* newShader = new ShaderIF(shaderSpecs, n);
    if (newShader)
        return shaders.emplace(name, newShader).second;
    return false;
}
