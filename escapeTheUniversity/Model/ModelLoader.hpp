#pragma 

#include <assimp/scene.h>
#include <GL\glew.h>
#include <string>
#include <vector>
#include "Mesh/Mesh.hpp"
#include "Mesh/Animator.hpp"

class Node;
class ModelNode;
class LightNode;
class DoorNode;

/*Loads and contains all models.*/
class ModelLoader
{
private:
	const std::string DOOR_SUFFIX = "_door"; // Door xxx names
	const std::string ANGLE_SUFFIX = "_angle";
	const std::string LIGHT_SUFFIX = "_light"; // Light nodes
	const std::string BOUNDING_SUFFIX = "_bounding"; // Suffix to calculate bounding volumes for

	bool loadModels = true; // Set this variable only once!
	std::string directory;// Relative path to all models
	std::vector<Mesh::Texture> loadedTextures; // Contains all already loaded textures
	std::vector<glm::vec4> loadedMaterials; // Contains all already loaded materials

	ModelLoader(void){}; // Private constructor to allow only one instance
	ModelLoader(ModelLoader const&); // Private constructor to prevent copies
	void operator=(ModelLoader const&); // Private constructor to prevent assignments

	Node* ModelLoader::processNode(Node* parent, aiNode* node, const aiScene* scene); // Processes a node
	LightNode* processLightNode(string* name, Node* parent, aiNode* node, const aiScene* scene); // Processes a light node
	void processMeshesAndChildren(Node* current, aiNode* node, const aiScene* scene);
	glm::vec3 getTransformationVec(aiMatrix4x4* transformation); // Transforms the blender 4x4 matrix into a xyz vec3
	std::string lightSourceTypeToString(aiLightSourceType type); // Transforms the enum type into a string
	
	Mesh* processMesh(aiMesh* mesh, const aiScene* scene, ModelNode* modelNode); // Processes the mesh
	std::vector<Mesh::Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName, std::vector<glm::vec4>* materials);// Loads all materials and the textures
	void linkLightUBO(); // Generates the Light UBO handle
public:
	const std::string SMALL_WING = "SmallWing";
	const std::string BIG_WING = "BigWing";
	Node* root; // Root node of the scene graph
	Animator* animator; // Is the animator for  vertex skinning animations

	// Resource top folder directories
	const std::string MODEL_DIR = ".\\Model\\";
	// Number of lights 
	const unsigned int LIGHT_NUMBER = 10;

	/*Returns the pointer to the unique instance of this class.*/
	static ModelLoader* ModelLoader::getInstance()
	{
		static ModelLoader instance;// lazy singleton, instantiated on first use
		return &instance;
	}
	~ModelLoader();

	void load(std::string path); // Loads all models and generates the scene graph
	unsigned int ModelLoader::loadPicture(std::string path); // Loads a single picture
	void setTextureState(Node* current, int paramMin, int paramMax); // Sets the texture sampling state of the current node

	// Light
	const unsigned int lightBinding = 2; // In the deferredShading.frag
	unsigned int lightUBO = 0; // Handle for the Light UBO in deferredShading.frag
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
