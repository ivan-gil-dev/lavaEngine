#ifndef rigidbody_h
#define rigidbody_h

#include	"DataTypes.h"
#include	"Mesh.h"
#include	"../../vendor/glm/gtc/quaternion.hpp"
#include	"../../vendor/glm/gtx/quaternion.hpp"

namespace Engine{
	enum RigidBodyShapeType{
		RIGIDBODY_SHAPE_TYPE_PLANE,
		RIGIDBODY_SHAPE_TYPE_CUBE,
		RIGIDBODY_SHAPE_TYPE_MESH
	
	};

	class RigidBody	{
		RigidBodyShapeType    ShapeType;
		btConvexHullShape *   pShape;
		btRigidBody *		  pRigidBody;
		glm::vec3			  PosOffset;
		glm::vec3			  RotationOffset;
		btDefaultMotionState* pMotionState;
		WireframeMesh		  DebugMesh;

		private:
		std::vector<glm::vec3> *LoadVertices(std::string modelPath);

		void CreateShape(Mesh *mesh);

		void CreateShape(RigidBodyShapeType shapeType);

		//Создание твердого тела
		void CreateBodyWithMass(btScalar mass, float friction, float restitution,
			btDynamicsWorld* dynamicsWorld, int userIndex);

		public:
		void SetRigidBodyTransform(Transform &transform);
		
		void SetRigidbodyScale(glm::vec3 scaleVal);

		WireframeMesh* pGetDebugMesh();

		btRigidBody* GetBulletRigidBody();

		btCollisionShape* GetBulletShape();

		void CreateRigidBody(Mesh *mesh, float mass, float restitution, 
			float friction, btDynamicsWorld* dynamicsWorld, int id);

		void CreateRigidBody(RigidBodyShapeType shapeType, float mass, float restitution,
			float friction, btDynamicsWorld* dynamicsWorld, int id);
		
		void Destroy(btDynamicsWorld *dynamicsWorld);
		
	};
}

#endif