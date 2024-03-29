﻿module GameComponentDestroyer;
import RigidBodyFactory;
import ColliderFactory;
using std::shared_ptr;

namespace BEbraEngine {

	GameComponentDestroyer::GameComponentDestroyer(
		RenderObjectFactory& renderFactory,
		ColliderFactory& colliderFactory,
		RigidBodyFactory& rigidBodyFactory,
		TextureFactory& textureFactory)
	{
		this->renderFactory = &renderFactory;
		this->colliderFactory = &colliderFactory;
		this->rigidBodyFactory = &rigidBodyFactory;
		this->textureFactory = &textureFactory;
	}

	void GameComponentDestroyer::destroyRenderComponent(RenderObject& comp)
	{
		renderFactory->destroyObject(comp);
	}

	void GameComponentDestroyer::destroyRigidBodyComponent(RigidBody& comp)
	{
		rigidBodyFactory->destroy(comp);
	}

	void GameComponentDestroyer::destroyColliderComponent(Collider& comp)
	{
		colliderFactory->destroyCollider(comp);

	}
	void GameComponentDestroyer::destroyTextureComponent(Texture& comp)
	{
		//std::visit([&](CTextureFactory auto& textureFactory) {
		//	textureFactory.destroyTextureAsync(comp);
		//}, *textureFactory);
	}
	void GameComponentDestroyer::destroyMaterialComponent(Material& comp)
	{
		if (comp.color)
			destroyTextureComponentAsync(comp.color);
		if (comp.specular)
			destroyTextureComponentAsync(comp.specular);
		if (comp.normal)
			destroyTextureComponentAsync(comp.normal);
	}
	void GameComponentDestroyer::destroyTransformComponent(Transform& comp)
	{

	}

	void GameComponentDestroyer::destroyPointLightComponent(Light& comp)
	{
		renderFactory->destroyPointLight(comp);
	}

	void GameComponentDestroyer::destroyDirectionLightComponent(DirectionLight& comp)
	{

	}
	void GameComponentDestroyer::destroyCameraComponent(SimpleCamera& comp)
	{
		renderFactory->destroyCamera(comp);
	}

	void GameComponentDestroyer::destroyTextureComponentAsync(shared_ptr<Texture> comp)
	{
		//std::visit([&](CTextureFactory auto& textureFactory) {
		//	textureFactory.destroyTextureAsync(comp);
		//}, *textureFactory);
		

	}

	void GameComponentDestroyer::destroyGameObject(GameObject& comp)
	{

	}
}