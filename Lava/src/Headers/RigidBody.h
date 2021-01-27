#pragma once
#include	"DataTypes.h"
#include	"Mesh.h"
#include	"../vendor/glm/gtc/quaternion.hpp"
#include	"../vendor/glm/gtx/quaternion.hpp"

namespace Lava{
	enum RigidBodyShapeType{
		LAVA_RIGIDBODY_SHAPE_TYPE_PLANE,
		LAVA_RIGIDBODY_SHAPE_TYPE_CUBE,
		LAVA_RIGIDBODY_SHAPE_TYPE_MESH
	
	};

	class RigidBody	{

		RigidBodyShapeType    ShapeType;
		btConvexHullShape *   PShape;
		btRigidBody *		  vRigidBody;
		glm::vec3			  PosOffset;
		glm::vec3			  RotationOffset;
		btDefaultMotionState* MotionState;
		WireframeMesh		  DebugMesh;

		private:
		//«агрузка .obj меша
		std::vector<glm::vec3> *LoadVertices(std::string modelPath){
			std::unordered_map<DataTypes::Vertex, uint32_t> _UniqueVertices{};
			tinyobj::attrib_t attrib;
			std::vector<glm::vec3> *vertices = new std::vector<glm::vec3>;
			std::vector<tinyobj::material_t> materials;
			std::vector<tinyobj::shape_t> shapes;
			std::string warn, err;

			if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.c_str())) throw std::runtime_error(warn + err);

			for (size_t i = 0; i < shapes.size(); i++) {
				size_t offset = 0;
				for (size_t j = 0; j < shapes[i].mesh.num_face_vertices.size(); j++) {
					for (size_t k = 0; k < shapes[i].mesh.num_face_vertices[j]; k++) {

						DataTypes::Vertex vertex{};

						vertex.pos = {

						attrib.vertices[3 * shapes[i].mesh.indices[k + offset].vertex_index + 0],
						attrib.vertices[3 * shapes[i].mesh.indices[k + offset].vertex_index + 1],
						attrib.vertices[3 * shapes[i].mesh.indices[k + offset].vertex_index + 2]

						};

						if (_UniqueVertices.count(vertex) == 0) {
							_UniqueVertices[vertex] = static_cast<uint32_t>(vertices->size());
							vertices->push_back(vertex.pos);
						}
					}
					offset += shapes[i].mesh.num_face_vertices[j];
				}
			}
		}


		//—оздание формы твердого тела 
		void CreateShape(Mesh *mesh) {
			ShapeType = LAVA_RIGIDBODY_SHAPE_TYPE_MESH;
			if (ENABLE_RIGIDBODY_MESH) {
				DebugMesh.CreateMesh(mesh->GetMeshPath(), glm::vec3(0.0f, 1.0f, 0.0f));
			}

			PShape = new btConvexHullShape;
			for (size_t i = 0; i < mesh->GetVertices()->size(); i++) {
				((btConvexHullShape*)PShape)->addPoint(btVector3(
					mesh->GetVertices()->at(i).pos.x,
					mesh->GetVertices()->at(i).pos.y,
					mesh->GetVertices()->at(i).pos.z),
					true);
			}
			((btConvexHullShape*)PShape)->optimizeConvexHull();
			((btConvexHullShape*)PShape)->initializePolyhedralFeatures();
			((btConvexHullShape*)PShape)->recalcLocalAabb();
		}

		//—оздание формы твердого тела
		void CreateShape(RigidBodyShapeType shapeType){
			ShapeType = shapeType;

			if (shapeType == LAVA_RIGIDBODY_SHAPE_TYPE_MESH) {
				if (ENABLE_RIGIDBODY_MESH) {
					DebugMesh.CreateMesh("CoreAssets/cube.obj", glm::vec3(1.0f, 0.0f, 0.0f));
					//—оздание формы твердого тела в виде куба
					//“акже будет создан отладочный меш
					PShape = new btConvexHullShape;
					for (size_t i = 0; i < DebugMesh.GetVertices()->size(); i++) {
						((btConvexHullShape*)PShape)->addPoint(btVector3(
							DebugMesh.GetVertices()->at(i).pos.x,
							DebugMesh.GetVertices()->at(i).pos.y,
							DebugMesh.GetVertices()->at(i).pos.z),
							true);
					}
					
				}
				else {
					//—оздание формы твердого тела в виде куба
					//без отладочного меша
					PShape = new btConvexHullShape;
					std::vector<glm::vec3>* vertices = LoadVertices("CoreAssets/cube.obj");
					for (size_t i = 0; i < vertices->size(); i++) {
						((btConvexHullShape*)PShape)->addPoint(btVector3(
							vertices->at(i).x,
							vertices->at(i).y,
							vertices->at(i).z),
							true);
					}
					delete vertices;
					
				}
				return;
			}

			switch (shapeType) {
			case LAVA_RIGIDBODY_SHAPE_TYPE_CUBE:
			{
				if (ENABLE_RIGIDBODY_MESH) {
					DebugMesh.CreateMesh("CoreAssets/cube.obj", glm::vec3(1.0f, 0.0f, 0.0f));
					//—оздание твердого тела в форме куба
					//отладочный меш будет создан
					PShape = new btConvexHullShape;
					for (size_t i = 0; i < DebugMesh.GetVertices()->size(); i++) {
						((btConvexHullShape*)PShape)->addPoint(btVector3(
							DebugMesh.GetVertices()->at(i).pos.x,
							DebugMesh.GetVertices()->at(i).pos.y,
							DebugMesh.GetVertices()->at(i).pos.z),
							true);
					}
				}
				else {
					//—оздание твердого тела в форме куба
					//Ѕез отладочного меша
					PShape = new btConvexHullShape;
					std::vector<glm::vec3>* vertices = LoadVertices("CoreAssets/cube.obj");
					for (size_t i = 0; i < vertices->size(); i++) {
						((btConvexHullShape*)PShape)->addPoint(btVector3(
							vertices->at(i).x,
							vertices->at(i).y,
							vertices->at(i).z),
							true);
					}
					delete vertices;
				}
			}
			break;

			case LAVA_RIGIDBODY_SHAPE_TYPE_PLANE:
			{
				if (ENABLE_RIGIDBODY_MESH) {
					DebugMesh.CreateMesh("CoreAssets/plane.obj", glm::vec3(1.0f, 0.0f, 0.0f));
					//—оздание твердого тела в форме плоскости
					//отладочный меш будет создан
					PShape = new btConvexHullShape;
					for (size_t i = 0; i < DebugMesh.GetVertices()->size(); i++) {
						((btConvexHullShape*)PShape)->addPoint(btVector3(
							DebugMesh.GetVertices()->at(i).pos.x,
							DebugMesh.GetVertices()->at(i).pos.y,
							DebugMesh.GetVertices()->at(i).pos.z),
							true);
					}
				}
				else {
					//—оздание твердого тела в форме плоскости
					//отладочный меш не будет создан
					PShape = new btConvexHullShape;
					std::vector<glm::vec3>* vertices = LoadVertices("CoreAssets/plane.obj");
					for (size_t i = 0; i < vertices->size(); i++) {
						((btConvexHullShape*)PShape)->addPoint(btVector3(
							vertices->at(i).x,
							vertices->at(i).y,
							vertices->at(i).z),
							true);
					}
					delete vertices;
				}
			}
			break;
			default:
				break;
			}
		}

		//—оздание твердого тела
		void CreateBodyWithMass(btScalar mass, float friction, float restitution,
			btDynamicsWorld* dynamicsWorld, int userIndex) {

			btTransform transform;
			transform.setIdentity();
			transform.setOrigin(btVector3(0, 0, 0));

			btVector3 bodyInertia;
			btScalar bodyMass = mass;

			if (bodyMass == 0) {
				MotionState = 0;
				PShape->calculateLocalInertia(bodyMass, bodyInertia);
			}else{
				MotionState = new btDefaultMotionState(transform);
				PShape->calculateLocalInertia(bodyMass, bodyInertia);
			}
			
			btRigidBody::btRigidBodyConstructionInfo constructionInfo(
				bodyMass,
				MotionState,
				PShape,
				bodyInertia
			);

			constructionInfo.m_restitution = restitution;
			constructionInfo.m_friction = friction;

			vRigidBody = new btRigidBody(constructionInfo);

			if (bodyMass != 0) {
				vRigidBody->setActivationState(DISABLE_DEACTIVATION);
			}

			vRigidBody->setUserPointer(this);
			vRigidBody->setUserIndex(userIndex);

			dynamicsWorld->addRigidBody(vRigidBody);
		}

		public:
		bool IsCreated;

		public:
		void SetRigidBodyTransform(Transform &transform) {
			if (vRigidBody != nullptr) {
				btVector3 pos(
					transform.gPosition().x,
					transform.gPosition().y,
					transform.gPosition().z
				);

				btQuaternion rot;
				rot.setX(transform.gQuaternion().x);
				rot.setY(transform.gQuaternion().y);
				rot.setZ(transform.gQuaternion().z);
				rot.setW(transform.gQuaternion().w);

				btTransform worldTransform(rot, pos);
				vRigidBody->setCenterOfMassTransform(worldTransform);

				DebugMesh.Transform.Translate(transform.gPosition());
				DebugMesh.Transform.Rotate(transform.gEulerAngles());
			}
		}
		
		void SetRigidbodyScale(glm::vec3 scaleVal){
			PShape->setLocalScaling(btVector3(
				scaleVal.x,
				scaleVal.y,
				scaleVal.z
			));
			PShape->recalcLocalAabb();
			DebugMesh.Transform.Scale(scaleVal);
		}

		public:
		WireframeMesh* GetDebugMesh() {
			return &DebugMesh;
		}

		btRigidBody* GetBulletRigidBody() {
			return vRigidBody;
		}

		btCollisionShape* GetBulletShape() {
			return PShape;
		}

		public:
		void CreateRigidBody(Mesh *mesh, float mass, float restitution, 
			float friction, btDynamicsWorld* dynamicsWorld, int id) {

			CreateShape(mesh);

			CreateBodyWithMass(
				mass,
				friction,
				restitution,
				dynamicsWorld,
				id
			);

			IsCreated = true;}

		void CreateRigidBody(RigidBodyShapeType shapeType, float mass, float restitution,
			float friction, btDynamicsWorld* dynamicsWorld, int id){

			CreateShape(shapeType);

			CreateBodyWithMass(
				mass,
				friction,
				restitution,
				dynamicsWorld,
				id
			);

			IsCreated = true;
		}
		
		void Destroy(btDynamicsWorld *dynamicsWorld){
			if (vRigidBody != nullptr) {
				DebugMesh.Destroy();
				dynamicsWorld->removeRigidBody(vRigidBody);
				delete MotionState;
				delete PShape;
				delete vRigidBody;
				IsCreated = false;
			}
		}
	};
}