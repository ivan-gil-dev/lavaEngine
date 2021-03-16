#include "RigidBody.h"



std::vector<glm::vec3>* Engine::RigidBody::LoadVertices(std::string modelPath) {
	std::unordered_map<DataTypes::MeshVertex_t, uint32_t> _UniqueVertices{};
	tinyobj::attrib_t attrib;
	std::vector<glm::vec3>* vertices = new std::vector<glm::vec3>;
	std::vector<tinyobj::material_t> materials;
	std::vector<tinyobj::shape_t> shapes;
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.c_str())) throw std::runtime_error(warn + err);

	for (size_t i = 0; i < shapes.size(); i++) {
		size_t offset = 0;
		for (size_t j = 0; j < shapes[i].mesh.num_face_vertices.size(); j++) {
			for (size_t k = 0; k < shapes[i].mesh.num_face_vertices[j]; k++) {

				DataTypes::MeshVertex_t vertex{};

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
	return vertices;
}

void Engine::RigidBody::CreateShape(RigidBodyShapeType shapeType) {
	ShapeType = shapeType;

	if (shapeType == RIGIDBODY_SHAPE_TYPE_MESH) {
		if (ENABLE_RIGIDBODY_MESH) {
			DebugMesh.CreateMesh("CoreAssets/cube.obj", glm::vec3(1.0f, 0.0f, 0.0f));
			//Физическая модель в форме куба с графическим объектом
			pShape = new btConvexHullShape;
			for (size_t i = 0; i < DebugMesh.GetVertices().size(); i++) {
				((btConvexHullShape*)pShape)->addPoint(btVector3(
					DebugMesh.GetVertices().at(i).pos.x,
					DebugMesh.GetVertices().at(i).pos.y,
					DebugMesh.GetVertices().at(i).pos.z),
					true);
			}

		}
		else {
			//Физическая модель в форме куба без графического объекта
			pShape = new btConvexHullShape;
			std::vector<glm::vec3>* vertices = LoadVertices("CoreAssets/cube.obj");
			for (size_t i = 0; i < vertices->size(); i++) {
				((btConvexHullShape*)pShape)->addPoint(btVector3(
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
	case RIGIDBODY_SHAPE_TYPE_CUBE:
	{
		if (ENABLE_RIGIDBODY_MESH) {
			DebugMesh.CreateMesh("CoreAssets/cube.obj", glm::vec3(1.0f, 0.0f, 0.0f));
			pShape = new btConvexHullShape;
			for (size_t i = 0; i < DebugMesh.GetVertices().size(); i++) {
				((btConvexHullShape*)pShape)->addPoint(btVector3(
					DebugMesh.GetVertices().at(i).pos.x,
					DebugMesh.GetVertices().at(i).pos.y,
					DebugMesh.GetVertices().at(i).pos.z),
					true);
			}
		}
		else {
			pShape = new btConvexHullShape;
			std::vector<glm::vec3>* vertices = LoadVertices("CoreAssets/cube.obj");
			for (size_t i = 0; i < vertices->size(); i++) {
				((btConvexHullShape*)pShape)->addPoint(btVector3(
					vertices->at(i).x,
					vertices->at(i).y,
					vertices->at(i).z),
					true);
			}
			delete vertices;
		}
	}
	break;

	case RIGIDBODY_SHAPE_TYPE_PLANE:
	{
		if (ENABLE_RIGIDBODY_MESH) {
			DebugMesh.CreateMesh("CoreAssets/plane.obj", glm::vec3(1.0f, 0.0f, 0.0f));
			pShape = new btConvexHullShape;
			for (size_t i = 0; i < DebugMesh.GetVertices().size(); i++) {
				((btConvexHullShape*)pShape)->addPoint(btVector3(
					DebugMesh.GetVertices().at(i).pos.x,
					DebugMesh.GetVertices().at(i).pos.y,
					DebugMesh.GetVertices().at(i).pos.z),
					true);
			}
		}
		else {
			pShape = new btConvexHullShape;
			std::vector<glm::vec3>* vertices = LoadVertices("CoreAssets/plane.obj");
			for (size_t i = 0; i < vertices->size(); i++) {
				((btConvexHullShape*)pShape)->addPoint(btVector3(
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

void Engine::RigidBody::CreateShape(Mesh* mesh) {
	ShapeType = RIGIDBODY_SHAPE_TYPE_MESH;
	if (ENABLE_RIGIDBODY_MESH) {
		DebugMesh.CreateMesh(mesh->pMeshPath(), glm::vec3(0.0f, 1.0f, 0.0f));
	}

	pShape = new btConvexHullShape;
	for (size_t i = 0; i < mesh->GetVertices()->size(); i++) {
		((btConvexHullShape*)pShape)->addPoint(btVector3(
			mesh->GetVertices()->at(i).pos.x,
			mesh->GetVertices()->at(i).pos.y,
			mesh->GetVertices()->at(i).pos.z),
			true);
	}
	((btConvexHullShape*)pShape)->optimizeConvexHull();
	((btConvexHullShape*)pShape)->initializePolyhedralFeatures();
	((btConvexHullShape*)pShape)->recalcLocalAabb();
}

void Engine::RigidBody::CreateBodyWithMass(btScalar mass, float friction, float restitution, btDynamicsWorld* dynamicsWorld, int userIndex) {
	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(btVector3(0, 0, 0));

	btVector3 bodyInertia;
	btScalar bodyMass = mass;

	if (bodyMass == 0) {
		pMotionState = 0;
		pShape->calculateLocalInertia(bodyMass, bodyInertia);
	}
	else {
		pMotionState = new btDefaultMotionState(transform);
		pShape->calculateLocalInertia(bodyMass, bodyInertia);
	}

	btRigidBody::btRigidBodyConstructionInfo constructionInfo(
		bodyMass,
		pMotionState,
		pShape,
		bodyInertia
	);

	constructionInfo.m_restitution = restitution;
	constructionInfo.m_friction = friction;

	pRigidBody = new btRigidBody(constructionInfo);
	pRigidBody->setActivationState(DISABLE_DEACTIVATION);
	pRigidBody->setUserPointer(this);
	pRigidBody->setUserIndex(userIndex);

	dynamicsWorld->addRigidBody(pRigidBody);
}

void Engine::RigidBody::SetRigidBodyTransform(Transform& transform) {
	if (pRigidBody != nullptr) {
		btVector3 pos(
			transform.GetPosition().x,
			transform.GetPosition().y,
			transform.GetPosition().z
		);

		btQuaternion rot;
		rot.setX(transform.GetQuaternion().x);
		rot.setY(transform.GetQuaternion().y);
		rot.setZ(transform.GetQuaternion().z);
		rot.setW(transform.GetQuaternion().w);

		btTransform worldTransform(rot, pos);
		pRigidBody->setCenterOfMassTransform(worldTransform);

		DebugMesh.Transform.Translate(transform.GetPosition());
		DebugMesh.Transform.Rotate(transform.GetEulerAngles());
	}
}

void Engine::RigidBody::SetRigidbodyScale(glm::vec3 scaleVal) {
	pShape->setLocalScaling(btVector3(
		scaleVal.x,
		scaleVal.y,
		scaleVal.z
	));
	pShape->recalcLocalAabb();
	DebugMesh.Transform.Scale(scaleVal);
}

Engine::WireframeMesh* Engine::RigidBody::pGetDebugMesh() {
	return &DebugMesh;
}

btRigidBody* Engine::RigidBody::GetBulletRigidBody() {
	return pRigidBody;
}

btCollisionShape* Engine::RigidBody::GetBulletShape() {
	return pShape;
}

void Engine::RigidBody::CreateRigidBody(RigidBodyShapeType shapeType, float mass, 
	float restitution, float friction, btDynamicsWorld* dynamicsWorld, int id) {
	CreateShape(shapeType);

	CreateBodyWithMass(
		mass,
		friction,
		restitution,
		dynamicsWorld,
		id
	);
}

void Engine::RigidBody::CreateRigidBody(Mesh* mesh, float mass, 
	float restitution, float friction, btDynamicsWorld* dynamicsWorld, int id) {
	CreateShape(mesh);

	CreateBodyWithMass(
		mass,
		friction,
		restitution,
		dynamicsWorld,
		id
	);
}

void Engine::RigidBody::Destroy(btDynamicsWorld* dynamicsWorld) {
	if (pRigidBody != nullptr) {
		DebugMesh.Destroy();
		dynamicsWorld->removeRigidBody(pRigidBody);
		delete pMotionState;
		delete pShape;
		delete pRigidBody;
	}
}
