#include "Skybox.h"
#include "Texture/TextureManager.h"
#include "SkyboxRenderer.h"

void Skybox::SetCubemap(const std::string& tag) {
	D3D12_GPU_DESCRIPTOR_HANDLE handle = TextureManager::GetInstance()->GetTextureHandle(tag);
	if (handle.ptr == 0) {
		isLoaded_ = false;
		return;
	}
	cubemapHandle_ = handle;
	isLoaded_ = true;
}

void Skybox::Draw() const {
	if (!isLoaded_) return;
	SkyboxRenderer::GetInstance()->Draw(cubemapHandle_, color_);
}
