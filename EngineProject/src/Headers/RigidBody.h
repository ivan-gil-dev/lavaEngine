#  pragma warning( push )
#  pragma warning( disable: 4251 )
#ifndef rigidbody_h
#define rigidbody_h

#include	"Renderer/DataTypes.h"
#include	"Mesh.h"
#include	"../../vendor/glm/gtc/quaternion.hpp"
#include	"../../vendor/glm/gtx/quaternion.hpp"

namespace Engine{
	enum RigidBodyShapeType{
		RIGIDBODY_SHAPE_TYPE_PLANE,
		RIGIDBODY_SHAPE_TYPE_CUBE,
		RIGIDBODY_SHAPE_TYPE_MESH,
		RIGIDBODY_SHAPE_TYPE_SPHERE
	};

	class EngineAPI_Export RigidBody	{
		RigidBodyShapeType    ShapeType;
		btConvexShape *       pShape;
		btRigidBody *		  pRigidBody;
		
		btDefaultMotionState* pMotionState;
		WireframeMesh		  DebugMesh;

		float				  mass,
							  restitution,
							  friction;
		
        glm::vec3			  rigidbodyOffset;
        glm::vec3			  rigidbodyRotation;
		glm::vec3			  rigidbodyScale;
		private:
		std::vector<glm::vec3> *LoadVertices(std::string modelPath);

		void CreateShape(Mesh *mesh);

		void CreateShape(RigidBodyShapeType shapeType);

		//Создание твердого тела
		void CreateBodyWithMass(btDynamicsWorld* dynamicsWorld, int userIndex);

		public:
			RigidBody();

		void SetRigidBodyTransform(Transform &transform);
		
		void SetRigidbodyScale(glm::vec3 scaleVal);

		glm::vec3 GetRigidbodyScale();

		WireframeMesh* pGetDebugMesh();

		btRigidBody* GetBulletRigidBody();

		btCollisionShape* GetBulletShape();

		void CreateRigidBody(Mesh *mesh, btDynamicsWorld* dynamicsWorld, int id);

		void CreateRigidBody(RigidBodyShapeType shapeType, btDynamicsWorld* dynamicsWorld, int id);
		
		void Destroy(btDynamicsWorld *dynamicsWorld);
		
		RigidBodyShapeType GetShapeType();

		float GetMass() const;
		void SetMass(float val);
        float GetRestitution() const;
        void SetRestitution(float val);
        float GetFriction() const;
        void SetFriction(float val);
	};
}

#endif
#  pragma warning( pop )