#pragma once
#include "DataTypes.h"
#include "../vendor/glm/gtc/quaternion.hpp"
#include "../vendor/glm/gtx/quaternion.hpp"
#include <btBulletDynamicsCommon.h>
namespace Lava{
	class RigidBody{
		btCollisionShape *shape;
		bool convex;
		bool staticPlane;
		btRigidBody *rigidBody;
		btDefaultMotionState* motionState;
		public:
			void createShapeFromVertices(
				std::vector<Vertex> vertices,
				bool Convex
			){
				convex = Convex;
				shape = new btConvexHullShape;
				if (convex) {
					for (size_t i = 0; i < vertices.size(); i++) {
						((btConvexHullShape*)shape)->addPoint(btVector3(vertices[i].pos.x, vertices[i].pos.y, vertices[i].pos.z), true);
					}
				}
				else {
					btTriangleMesh* mesh = new btTriangleMesh;
					for (int i = 0; i < vertices.size(); i += 3) {
						Vertex v1 = vertices[i];
						Vertex v2 = vertices[i + 1];
						Vertex v3 = vertices[i + 2];

						btVector3 bv1 = btVector3(v1.pos.x, v1.pos.y, v1.pos.z);
						btVector3 bv2 = btVector3(v2.pos.x, v2.pos.y, v2.pos.z);
						btVector3 bv3 = btVector3(v3.pos.x, v3.pos.y, v3.pos.z);

						mesh->addTriangle(bv1, bv2, bv3);
					}
					shape = new btConvexTriangleMeshShape(mesh);
				}
			}
			void createPlaneShape(){
				shape = new btStaticPlaneShape(btVector3(0.,1,0.),0);
				
			}

		void createBodyWithMass(
			btScalar mass,
			float friction,
			float restitution,
			btDynamicsWorld *dynamicsWorld,
			int userIndex
		){
			btTransform transform;
			transform.setIdentity();
			transform.setOrigin(btVector3(0, 0, 0));
			
			motionState = new btDefaultMotionState(transform);
			btScalar bodyMass = mass;
			btVector3 bodyInertia;
			
			if (mass == 0) {
				bodyInertia = btVector3(0,0,0);
			
			}else{
				shape->calculateLocalInertia(bodyMass, bodyInertia);
			}
			
			btRigidBody::btRigidBodyConstructionInfo constructionInfo(
				bodyMass,
				motionState,
				shape,
				bodyInertia
			);
			if(mass == 0){
				motionState = 0;
			}
		    constructionInfo.m_restitution = restitution;
			constructionInfo.m_friction = friction;
	
			rigidBody = new btRigidBody(constructionInfo);
			
			if (mass == 0){
				rigidBody->setActivationState(ISLAND_SLEEPING);
				
			}else{
				rigidBody->setActivationState(DISABLE_DEACTIVATION);
			}
			
			rigidBody->setUserPointer(this);
			rigidBody->setUserIndex(userIndex);

			dynamicsWorld->addRigidBody(rigidBody);
			
			
		}
	
		btRigidBody* getRigidBody(){
			return rigidBody;
		}
		btCollisionShape* getShape(){
			return shape;
		}
		void destroy(btDynamicsWorld *dynamicsWorld){
			delete motionState;
		    delete shape;
			dynamicsWorld->removeRigidBody(rigidBody);
			delete rigidBody;
		}



	};
}