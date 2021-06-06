#pragma once
#include "../EngineProject/src/Headers/Entities.h"
class Floor : public Engine::GameObject {
public:
    void Start() {
    }
    void Update() {
        Transform.SetRotation(glm::vec3(0.0f, Transform.GetEulerAngles().y + 2.0f * Engine::Globals::DeltaTime, 0.0f));
        ApplyEntityTransformToRigidbody();
    }
};