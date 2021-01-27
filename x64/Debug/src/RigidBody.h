#pragma once
#include	"DataTypes.h"
#include	"Mesh.h"
#include	"../vendor/glm/gtc/quaternion.hpp"
#include	"../vendor/glm/gtx/quaternion.hpp"

namespace Lava{
	class RigidBody{
		WireframeMesh debugMesh;
		btCollisionShape *shape;

		bool convex;
		bool staticPlane;

		btRigidBody *rigidBody;

		btDefaultMotionState* motionState;

		void createShapeFromMesh(Mesh mesh) {
			debugMesh.createMesh(mesh.getMeshPath(), glm::vec3(0.0f, 1.0f, 0.0f));
			shape = new btConvexHullShape;
			
			for (size_t i = 0; i < mesh.getVertices()->size(); i++) {
				((btConvexHullShape*)shape)->addPoint(btVector3(
					mesh.getVertices()->at(i).pos.x, 
					mesh.getVertices()->at(i).pos.y,
					mesh.getVertices()->at(i).pos.z),
				true);
			}
		}

		void createStaticPlaneShape() {
			debugMesh.createMesh("assets/plane.obj",glm::vec3(1.0f,0.0f,0.0f));
			shape = new btConvexHullShape;

			for (size_t i = 0; i < debugMesh.getVertices()->size(); i++) {
				((btConvexHullShape*)shape)->addPoint(btVector3(
					debugMesh.getVertices()->at(i).pos.x,
					debugMesh.getVertices()->at(i).pos.y,
					debugMesh.getVertices()->at(i).pos.z),
					true);
			}
		}
		
		void createBodyWithMass(
			btScalar mass,
			float friction,
			float restitution,
			btDynamicsWorld* dynamicsWorld,
			int userIndex
		) {
			btTransform transform;
			transform.setIdentity();
			transform.setOrigin(btVector3(0, 0, 0));
			btVector3 bodyInertia;
			btScalar bodyMass = mass;
			if (bodyMass == 0) {
				motionState = 0;
				bodyInertia = btVector3(0, 0, 0);
			}else{
				motionState = new btDefaultMotionState(transform);
				shape->calculateLocalInertia(bodyMass, bodyInertia);
			}
			
			btRigidBody::btRigidBodyConstructionInfo constructionInfo(
				bodyMass,
				motionState,
				shape,
				bodyInertia
			);
			
			constructionInfo.m_restitution = restitution;
			constructionInfo.m_friction = friction;

			rigidBody = new btRigidBody(constructionInfo);

			if (mass == 0) {
				rigidBody->setActivationState(ISLAND_SLEEPING);
			}
			else {
				rigidBody->setActivationState(DISABLE_DEACTIVATION);
			}

			rigidBody->setUserPointer(this);
			rigidBody->setUserIndex(userIndex);

			dynamicsWorld->addRigidBody(rigidBody);
		}

		public:
		void setRigidBodyTransformFromMesh(Mesh mesh) {
			if (rigidBody != nullptr) {

				btVector3 pos(
					mesh.transform.getPosition().x,
					mesh.transform.getPosition().y,
					mesh.transform.getPosition().z
				);

				btQuaternion rot;
				rot.setX(mesh.transform.getQuaternion().x);
				rot.setY(mesh.transform.getQuaternion().y);
				rot.setZ(mesh.transform.getQuaternion().z);
				rot.setW(mesh.transform.getQuaternion().w);

				btTransform worldTransform(rot, pos);
				rigidBody->setWorldTransform(worldTransform);
				shape->setLocalScaling(btVector3(
					mesh.transform.getScale().x,
					mesh.transform.getScale().y,
					mesh.transform.getScale().z
				));

				debugMesh.transform.translate(mesh.transform.getPosition());
				debugMesh.transform.setQuaternion(mesh.transform.getQuaternion());
				debugMesh.transform.scale(mesh.transform.getScale());
			}
		}
		
		void scaleRigidBody(glm::vec3 scaleVal){
			shape->setLocalScaling(btVector3(
				scaleVal.x,
				scaleVal.y,
				scaleVal.z
			));

	
			debugMesh.transform.scale(scaleVal);
		}

		void createRigidBodyFromMesh(
			Mesh mesh,
			float mass,
			float restitution,
			float friction,
			btDynamicsWorld* dynamicsWorld,
			int id
		) {
			createShapeFromMesh(mesh);
			createBodyWithMass(
				mass,
				friction,
				restitution,
				dynamicsWorld,
				id
			);
		}
		
		void createStaticRigidBodyWithPlaneShape(
			btDynamicsWorld* dynamicsWorld,
			int id
		) {
			createStaticPlaneShape();
			createBodyWithMass(
				0.0f,
				0.0f,
				0.0f,
				dynamicsWorld,
				id
			);
		}
	
		WireframeMesh *getDebugMesh(){
			return &debugMesh;
		}

		btRigidBody* getBulletRigidBody(){
			return rigidBody;
		}

		btCollisionShape* getBulletShape(){
			return shape;
		}

		void destroy(btDynamicsWorld *dynamicsWorld){
			if (rigidBody != nullptr) {
				debugMesh.destroy();
				dynamicsWorld->removeRigidBody(rigidBody);
				delete motionState;
				delete shape;
				delete rigidBody;
			}
		}
	};
}