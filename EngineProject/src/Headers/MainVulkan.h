#ifndef mainvulkan_h
#define mainvulkan_h

#include "../../vendor/imgui.h"
#include "../../vendor/imgui_impl_glfw.h"
#include "../../vendor/imgui_impl_vulkan.h"
#include "../../vendor/imgui_impl_win32.h"
#include "../../vendor/glm/glm.hpp"
#include "../../vendor/glm/gtc/matrix_transform.hpp"
#include "../../vendor/glm/gtc/quaternion.hpp"
#include "../../vendor/glm/gtx/quaternion.hpp"
#include "../../vendor/volk.h"

#include <array>
#include <iostream>
#include <vector>
#include <fstream>
#include <chrono>

#include "Renderer/VkInstance.h"
#include "Renderer/DataTypes.h"
#include "Events.h"

#include "Renderer/Buffers.h"

#include "Entities.h"

#include "Scene.h"

static void imguiErrFunction(const VkResult result) {
    if (result != VK_SUCCESS) {
        std::cout << "Imgui vulkan errcode: " << result << std::endl;
        throw std::runtime_error("Imgui fatal error");
    }
}

namespace Engine {
    //static glm::vec3 castRay(double mouseX,double mouseY){
    //	double normalizedX = 2*mouseX/ Globals::gWidth - 1;
    //	double normalizedY = 2*mouseY / Globals::gHeight - 1;
    //	glm::vec4 deviceCoord = glm::vec4(normalizedX, normalizedY,-1.0f,1.0f);
    //	glm::mat4 invertedProj = glm::inverse(glm::perspective(glm::radians(80.0f), (Globals::gWidth / (float)Globals::gHeight), 0.1f, 1000.0f));
    //	glm::vec4 eyeCoords = invertedProj * deviceCoord;
    //	eyeCoords = glm::vec4(eyeCoords.x,eyeCoords.y,-1.0f,0.0f);
    //	glm::mat4 viewMatrix = Globals::debugCamera.GetView();
    //	eyeCoords = glm::inverse(viewMatrix) * eyeCoords;
    //	return glm::normalize(glm::vec3(eyeCoords.x,eyeCoords.y,eyeCoords.z));
    //}
}

#endif