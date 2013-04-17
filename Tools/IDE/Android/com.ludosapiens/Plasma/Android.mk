MY_LOCAL_PATH := $(call my-dir)

include ../CommonAndroid.mk

# Plasma
include $(CLEAR_VARS)

$(DEV_LOCAL_DEFAULTS)

LOCAL_PATH := $(DEV_PATH)/Projects

LOCAL_MODULE := Plasma

LOCAL_SRC_FILES := \
	Plasma/Action/Action.cpp \
	Plasma/Animation/AnimationGraph.cpp \
	Plasma/Animation/AnimationNode.cpp \
	Plasma/Animation/AnimationRail.cpp \
	Plasma/Animation/BasicNode.cpp \
	Plasma/Animation/JumpNode.cpp \
	Plasma/Animation/Puppeteer.cpp \
	Plasma/Animation/SkeletalAnimation.cpp \
	Plasma/Animation/Skeleton.cpp \
	Plasma/Animation/TravelNode.cpp \
	Plasma/Geometry/Geometry.cpp \
	Plasma/Geometry/Material.cpp \
	Plasma/Geometry/MaterialSet.cpp \
	Plasma/Geometry/MetaGeometry.cpp \
	Plasma/Geometry/MetaNode.cpp \
	Plasma/Geometry/MetaSurface.cpp \
	Plasma/Geometry/ParametricPatch.cpp \
	Plasma/Geometry/SurfaceGeometry.cpp \
	Plasma/Geometry/SilhouetteGeometry.cpp \
	Plasma/Geometry/MeshGeometry.cpp \
	Plasma/Intersector.cpp \
	Plasma/Manipulator/CameraManipulator.cpp \
	Plasma/Manipulator/FlyByManipulator.cpp \
	Plasma/Manipulator/Manipulator.cpp \
	Plasma/Particle/ParticleAnimator.cpp \
	Plasma/Particle/ParticleGenerator.cpp \
	Plasma/Particle/ParticleGroup.cpp \
	Plasma/Particle/ParticleManager.cpp \
	Plasma/Particle/ParticleSource.cpp \
	Plasma/Particle/Smoke.cpp \
	Plasma/Particle/Spark.cpp \
	Plasma/Physics/CollisionShapeGenerator.cpp \
	Plasma/Physics/PlasmaAttractors.cpp \
	Plasma/Plasma.cpp \
	Plasma/Procedural/Boundary.cpp \
	Plasma/Procedural/BoundaryPolygon.cpp \
	Plasma/Procedural/BSP2.cpp \
	Plasma/Procedural/BSP3.cpp \
	Plasma/Procedural/Component.cpp \
	Plasma/Procedural/ProceduralGeometry.cpp \
	Plasma/Procedural/ProceduralMaterial.cpp \
	Plasma/Procedural/ProceduralMesh.cpp \
	Plasma/Procedural/ProceduralWorld.cpp \
	Plasma/Render/DeferredRenderer.cpp \
	Plasma/Render/ForwardRenderer.cpp \
	Plasma/Render/ForwardRendererFixed.cpp \
	Plasma/Render/ForwardRendererHDR.cpp \
	Plasma/Render/PlasmaBaker.cpp \
	Plasma/Render/Renderer.cpp \
	Plasma/Renderable/BasicsRenderable.cpp \
	Plasma/Renderable/DebugRenderable.cpp \
	Plasma/Renderable/PrimitiveRenderable.cpp \
	Plasma/Renderable/Renderable.cpp \
	Plasma/Resource/ResExporter.cpp \
	Plasma/Resource/ResManager.cpp \
	Plasma/Resource/Serializer.cpp \
	Plasma/Widget/PlasmaBrowser.cpp \
	Plasma/Widget/PlasmaScreen.cpp \
	Plasma/World/Brain.cpp \
	Plasma/World/Camera.cpp \
	Plasma/World/Entity.cpp \
	Plasma/World/EntityGroup.cpp \
	Plasma/World/Light.cpp \
	Plasma/World/RigidEntity.cpp \
	Plasma/World/Selection.cpp \
	Plasma/World/SkeletalEntity.cpp \
	Plasma/World/Viewport.cpp \
	Plasma/World/World.cpp \
	Plasma/World/WorldVM.cpp \

include $(BUILD_STATIC_LIBRARY)
