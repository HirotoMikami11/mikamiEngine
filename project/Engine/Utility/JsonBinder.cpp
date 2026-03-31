#include "JsonBinder.h"
#include "JsonSettings.h"
#include "ImGui/ImGuiManager.h"

JsonBinder::JsonBinder(const std::string& groupName)
    : groupName_(groupName)
{
    auto* js = JsonSettings::GetInstance();
    js->CreateGroup({ groupName_ });
    js->LoadFile(groupName_);  // ファイルがなければ何もしない（Logger に記録）
}

//-----------------------------------------------------------------------
// 基本型 Bind
//-----------------------------------------------------------------------

void JsonBinder::Bind(const std::string& key, float* ptr, float defaultValue, float speed)
{
    auto* js = JsonSettings::GetInstance();
    js->AddItem({ groupName_ }, key, defaultValue);
    *ptr = js->GetFloatValue({ groupName_ }, key);

    drawItems_.push_back([this, key, ptr, speed]() {
        if (ImGui::DragFloat(key.c_str(), ptr, speed)) {
            JsonSettings::GetInstance()->SetValue({ groupName_ }, key, *ptr);
        }
    });
}

void JsonBinder::Bind(const std::string& key, int32_t* ptr, int32_t defaultValue, float speed)
{
    auto* js = JsonSettings::GetInstance();
    js->AddItem({ groupName_ }, key, defaultValue);
    *ptr = js->GetIntValue({ groupName_ }, key);

    drawItems_.push_back([this, key, ptr, speed]() {
        int v = static_cast<int>(*ptr);
        if (ImGui::DragInt(key.c_str(), &v, speed)) {
            *ptr = static_cast<int32_t>(v);
            JsonSettings::GetInstance()->SetValue({ groupName_ }, key, *ptr);
        }
    });
}

void JsonBinder::Bind(const std::string& key, bool* ptr, bool defaultValue)
{
    auto* js = JsonSettings::GetInstance();
    js->AddItem({ groupName_ }, key, defaultValue);
    *ptr = js->GetBoolValue({ groupName_ }, key);

    drawItems_.push_back([this, key, ptr]() {
        if (ImGui::Checkbox(key.c_str(), ptr)) {
            JsonSettings::GetInstance()->SetValue({ groupName_ }, key, *ptr);
        }
    });
}

void JsonBinder::Bind(const std::string& key, Vector2* ptr, const Vector2& defaultValue, float speed)
{
    auto* js = JsonSettings::GetInstance();
    js->AddItem({ groupName_ }, key, defaultValue);
    *ptr = js->GetVector2Value({ groupName_ }, key);

    drawItems_.push_back([this, key, ptr, speed]() {
        if (ImGui::DragFloat2(key.c_str(), &ptr->x, speed)) {
            JsonSettings::GetInstance()->SetValue({ groupName_ }, key, *ptr);
        }
    });
}

void JsonBinder::Bind(const std::string& key, Vector3* ptr, const Vector3& defaultValue, float speed)
{
    auto* js = JsonSettings::GetInstance();
    js->AddItem({ groupName_ }, key, defaultValue);
    *ptr = js->GetVector3Value({ groupName_ }, key);

    drawItems_.push_back([this, key, ptr, speed]() {
        if (ImGui::DragFloat3(key.c_str(), &ptr->x, speed)) {
            JsonSettings::GetInstance()->SetValue({ groupName_ }, key, *ptr);
        }
    });
}

void JsonBinder::Bind(const std::string& key, Vector4* ptr, const Vector4& defaultValue, float speed)
{
    auto* js = JsonSettings::GetInstance();
    js->AddItem({ groupName_ }, key, defaultValue);
    *ptr = js->GetVector4Value({ groupName_ }, key);

    drawItems_.push_back([this, key, ptr, speed]() {
        if (ImGui::DragFloat4(key.c_str(), &ptr->x, speed)) {
            JsonSettings::GetInstance()->SetValue({ groupName_ }, key, *ptr);
        }
    });
}

void JsonBinder::BindColor(const std::string& key, Vector4* ptr, const Vector4& defaultValue)
{
    auto* js = JsonSettings::GetInstance();
    js->AddItem({ groupName_ }, key, defaultValue);
    *ptr = js->GetVector4Value({ groupName_ }, key);

    drawItems_.push_back([this, key, ptr]() {
        if (ImGui::ColorEdit4(key.c_str(), &ptr->x)) {
            JsonSettings::GetInstance()->SetValue({ groupName_ }, key, *ptr);
        }
    });
}

//-----------------------------------------------------------------------
// 複合型 Bind
//-----------------------------------------------------------------------

void JsonBinder::BindTransform3D(const std::string& sectionName, Transform3D* transform)
{
    auto* js = JsonSettings::GetInstance();
    std::vector<std::string> path = { groupName_, sectionName };

    js->CreateGroup(path);
    js->AddItem(path, "Position", transform->GetPosition());
    js->AddItem(path, "Rotation", transform->GetRotation());
    js->AddItem(path, "Scale",    transform->GetScale());

    // JSON の値をオブジェクトに適用
    transform->SetPosition(js->GetVector3Value(path, "Position"));
    transform->SetRotation(js->GetVector3Value(path, "Rotation"));
    transform->SetScale(   js->GetVector3Value(path, "Scale"));

    drawItems_.push_back([sectionName, transform, path]() {
        if (ImGui::CollapsingHeader(sectionName.c_str())) {
            auto* js = JsonSettings::GetInstance();

            Vector3 pos = transform->GetPosition();
            if (ImGui::DragFloat3("Position", &pos.x, 0.1f)) {
                transform->SetPosition(pos);
                js->SetValue(path, "Position", pos);
            }

            Vector3 rot = transform->GetRotation();
            if (ImGui::DragFloat3("Rotation", &rot.x, 0.01f)) {
                transform->SetRotation(rot);
                js->SetValue(path, "Rotation", rot);
            }

            Vector3 scale = transform->GetScale();
            if (ImGui::DragFloat3("Scale", &scale.x, 0.01f, 0.01f, 100.0f)) {
                transform->SetScale(scale);
                js->SetValue(path, "Scale", scale);
            }
        }
    });
}

void JsonBinder::BindMaterial(const std::string& sectionName, Material* material)
{
    auto* js = JsonSettings::GetInstance();
    std::vector<std::string> path = { groupName_, sectionName };

    js->CreateGroup(path);
    js->AddItem(path, "Color",        material->GetColor());
    js->AddItem(path, "LightingMode", static_cast<int32_t>(material->GetLightingMode()));

    // JSON の値をオブジェクトに適用
    material->SetColor(       js->GetVector4Value(path, "Color"));
    material->SetLightingMode(static_cast<LightingMode>(js->GetIntValue(path, "LightingMode")));

    drawItems_.push_back([sectionName, material, path]() {
        if (ImGui::CollapsingHeader(sectionName.c_str())) {
            auto* js = JsonSettings::GetInstance();

            Vector4 color = material->GetColor();
            if (ImGui::ColorEdit4("Color", &color.x)) {
                material->SetColor(color);
                js->SetValue(path, "Color", color);
            }

            int mode = static_cast<int>(material->GetLightingMode());
            const char* lightingItems[] = { "None", "Lambert", "HalfLambert", "PhongSpecular" };
            if (ImGui::Combo("LightingMode", &mode, lightingItems, 4)) {
                material->SetLightingMode(static_cast<LightingMode>(mode));
                js->SetValue(path, "LightingMode", static_cast<int32_t>(mode));
            }
        }
    });
}

//-----------------------------------------------------------------------
// 表示・保存
//-----------------------------------------------------------------------

void JsonBinder::ImGui()
{
#ifdef USEIMGUI
    for (auto& draw : drawItems_) {
        draw();
    }
    if (::ImGui::Button(("Save##" + groupName_).c_str())) {
        Save();
    }
#endif
}

void JsonBinder::Save()
{
    JsonSettings::GetInstance()->SaveFile({ groupName_ });
}
