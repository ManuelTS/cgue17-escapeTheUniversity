#pragma 

#include <assimp/scene.h>
#include <GL\glew.h>
#include <string>
#include <vector>
#include "Mesh.hpp"

class Node;
class ModelNode;
class LightNode;
class DoorNode;

/*Loads and contains all models.*/
class ModelLoader
{
private:
	// General 
	const std::string ANGLE_SUFFIX = "_Angle";
	const std::string DOOR_PREFIX = "Door_";

	bool loadModels = true; // Set this variable only once!
	std::string directory;// Relative path to all models
	std::vector<Mesh::Texture> loadedTextures; // Contains all already loaded textures
	std::vector<glm::vec4> loadedMaterials; // Contains all already loaded materials

	ModelLoader(void){}; // Private constructor to allow only one instance
	ModelLoader(ModelLoader const&); // Private constructor to prevent copies
	void operator=(ModelLoader const&); // Private constructor to prevent assignments
	Node* ModelLoader::processNode(Node* parent, aiNode* node, const aiScene* scene); // Processes a node
	Mesh* processMesh(aiMesh* mesh, const aiScene* scene); // Processes the mesh
	std::vector<Mesh::Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName, std::vector<glm::vec4>* materials);// Loads all materials and the textures
	glm::vec3 getTransformationVec(aiMatrix4x4* transformation); // Transforms the blender 4x4 matrix into a xyz vec3
	void processMeshesAndChildren(Node* current, aiNode* node, const aiScene* scene);

	// Light
	const std::string LIGHT_SUFFIX = "_Licht";
	const unsigned int MAX_LIGHTS = 10; // Correlates with mesh.frag#MAX_LIGHTS

	LightNode* processLightNode(aiLight* lightNode, aiNode* node, const aiScene* scene, unsigned int arrayIndex); // Processes a light node
	void linkLightUBO(); // Generates the Light UBO handle
public:
	// General
	Node* root;

	/*Returns the pointer to the unique instance of this class.*/
	static ModelLoader* ModelLoader::getInstance()
	{
		static ModelLoader instance;// lazy singleton, instantiated on first use
		return &instance;
	}
	~ModelLoader();

	void load(std::string path); // Loads all models and generates the scene graph
	unsigned int ModelLoader::loadPicture(std::string path); // Loads a single picture
	std::vector<Node*> getAllNodes(); // Traverses the scenegraph and puts all nodes in the vector

	// Light
	const unsigned int lightBinding = 2; // In the Mesh.frag
	unsigned int lightUBO = 0; // Handle for the Light UBO in mesh.frag
	std::vector<LightNode*>lights;
};

inline bool operator == (const glm::vec4& a, const glm::vec4& b)
{// Lazy evaluation says thank you
	return
		a.x == b.x &&
		a.y == b.y &&
		a.z == b.z &&
		a.w == b.w;
}
