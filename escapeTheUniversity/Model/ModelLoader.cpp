#include <assimp\postprocess.h>
#include "ModelLoader.hpp"
#include "Node\Node.hpp"
#include "Node\ModelNode.hpp"
#include "Node\LightNode.hpp"
#include "Node\TransformationNode.hpp"
#include "Node\AnimatNode.hpp"
#include "Mesh\Mesh.hpp"
#include <IL\il.h>
#include <IL\ilu.h>  // for image creation and manipulation funcs.
#include "..\Debug\Debugger.hpp"

using namespace std;

ModelLoader::~ModelLoader()
{
	delete root;
	delete animator;
	delete importer;
}

/*Loads recusrive the model with its objects, meshs, and textures.*/
void ModelLoader::load(string path)
{
	if (loadModels)
	{
		path = MODEL_DIR + path;
		loadModels = false; // Makes sure in the hole game that the models are loaded only once!
		importer = new Assimp::Importer();// Read file via ASSIMP
		const aiScene* scene = importer->ReadFile(path, aiProcess_Triangulate | aiProcess_LimitBoneWeights);

		if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // Error check, if not zero
			Debugger::getInstance()->pauseExit("ASSIMP " + *importer->GetErrorString());

		this->directory = path.substr(0, path.find_last_of('\\') + 1);// Retrieve the directory path of the filepath
		this->linkLightUBO(); // Link light UBO in shader
		
		if (scene->HasAnimations())
			animator = new Animator(scene, 0);

		root = this->processNode(nullptr, scene->mRootNode, scene);// Process ASSIMP's root node recursively		
		
		loadedTextures.clear();
		loadedMaterials.clear();
	}
}

void ModelLoader::linkLightUBO()
{
		glGenBuffers(1, &lightUBO);
		glBindBuffer(GL_UNIFORM_BUFFER, lightUBO);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(LightNode::Light), NULL, GL_DYNAMIC_DRAW); // Play with last param 4 performance
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		//glUniformBlockBinding(shader->programHandle, xxx,xxx); // done by index attribute in shader
}

/*Processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).*/
Node* ModelLoader::processNode(Node* parent, aiNode* node, const aiScene* scene)
{
	string name = node->mName.C_Str();

    if (string::npos != name.find(LIGHT_SUFFIX)) //Light node
		return processLightNode(&name, parent, node, scene);
	else // Normal node and transformation processing
	{
		ModelNode* current = nullptr;
		
		if (string::npos != name.find(ANGLE_SUFFIX))
			current = new TransformationNode();
		else if (string::npos != name.find(ANIMATION_SUFFIX))
			current = new AnimatNode();
		else
			current = new ModelNode();

		current->name = name;

		if (string::npos != name.find(BOUNDING_SUFFIX))
			current->bounding = true;

		if (string::npos != name.find(SPHERE_01_NAME))
			sphere01 = current;

		current->parent = parent;
		processMeshesAndChildren(current, node, scene);
		return current;
	}

}

/*Processes Meshes and Child nodes recursively in depth traversal.*/
void ModelLoader::processMeshesAndChildren(Node* current, aiNode* node, const aiScene* scene)
{
	ModelNode* mn = (ModelNode*)current;

	if (mn != 0)
	{
		mn->position = getTransformationVec(&node->mTransformation);
		mn->setModelMatrix(); // Already translate modelMatrix
	}

	for (unsigned int i = 0; i < node->mNumMeshes; i++)// Process each mesh located at the current node
	{
		// The node object only contains indices to index the actual objects in the scene. 
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		// The scene contains all the data, node is just to keep stuff organized (like relations between nodes).
		
		if (mn != 0)
			mn->meshes.push_back(processMesh(mesh, i, scene, node, mn));
	}

	// After we've processed all of the meshes (if any) we then recursively process each of the children nodes
	for (unsigned int i = 0; i < node->mNumChildren; i++)
		current->children.push_back(processNode(current, node->mChildren[i], scene));
}

/*Transforms the blender vec4 location into a vec3 for further processing.*/
glm::vec3 ModelLoader::getTransformationVec(aiMatrix4x4* transformation)
{
	return glm::vec3(transformation->a4, transformation->b4, transformation->c4);
}

/*Performs the special light node coordinates readout.*/
LightNode* ModelLoader::processLightNode(string* name, Node* parent, aiNode* node, const aiScene* scene)
{
	LightNode * ln = nullptr;

	for (unsigned int i = 0; i < scene->mNumLights; i++)
	{
		aiLight* lightNode = scene->mLights[i]; // Go through all lights

		if (lightNode->mName.C_Str() == *name) // Node found
		{
			ln = new LightNode(lightUBO, i); // vec4 in method signature causes __declspec(align('16')) won't be aligned

			// Get light node information
			float lightType = 0.0f;// w=0 point light, w=1 directional light in mesh.frag

			if (lightNode->mType == aiLightSourceType::aiLightSource_POINT)
				lightType = 0.0f;
			else if (lightNode->mType == aiLightSourceType::aiLightSource_DIRECTIONAL)
				lightType = 1.0f;
			else
				Debugger::getInstance()->pauseExit("Malformed light type " + lightSourceTypeToString(lightNode->mType) + " found in light with name " + *name + ".");
			
			ln->light.position = glm::vec4(lightNode->mPosition.x, lightNode->mPosition.y, lightNode->mPosition.z, lightType); 

			if (ln->light.position.x == 0.0f && ln->light.position.y == 0.0f && ln->light.position.z == 0.0f) // Position in light node often not set, but in the normal node representation it is...
				ln->light.position = glm::vec4(getTransformationVec(&node->mTransformation), ln->light.position.w);

			ln->light.diffuse = glm::vec4(lightNode->mColorDiffuse.r, lightNode->mColorDiffuse.g, lightNode->mColorDiffuse.b, ln->light.diffuse.a);// Ambient light is in the shader ambient = diffuse.rbg * diffuse.a where .a is a simple coefficient
			ln->light.shiConLinQua = glm::vec4(64.0f, lightNode->mAttenuationConstant, lightNode->mAttenuationLinear, lightNode->mAttenuationQuadratic);
			// Shin,Lin, Qua values with distance: http://www.ogre3d.org/tikiwiki/tiki-index.php?page=-Point+Light+Attenuation

			ln->name = *name;
			ln->parent = parent;
			break;
		}
	}

	if (ln == nullptr)
		Debugger::getInstance()->pauseExit("Malfunction: Light node " + *name + " not found.");

	lights.push_back(ln);

	return ln;
}

std::string ModelLoader::lightSourceTypeToString(aiLightSourceType type)
{
	// Undefined light type
	if (type == aiLightSourceType::aiLightSource_UNDEFINED)
		return "aiLightSource_UNDEFINED";
	//! A directional light source has a well-defined direction
	//! but is infinitely far away. That's quite a good
	//! approximation for sun light.
	else if (type == aiLightSourceType::aiLightSource_DIRECTIONAL)
		return "aiLightSource_DIRECTIONAL";
	//! A point light source has a well-defined position
	//! in space but no direction - it emits light in all
	//! directions. A normal bulb is a point light.
	else if (type == aiLightSourceType::aiLightSource_POINT)
		return "aiLightSource_POINT";
	//! A spot light source emits light in a specific
	//! angle. It has a position and a direction it is pointing to.
	//! A good example for a spot light is a light spot in
	//! sport arenas.
	else if (type == aiLightSourceType::aiLightSource_SPOT)
		return "aiLightSource_SPOT";
	//! The generic light level of the world, including the bounces
	//! of all other light sources.
	//! Typically, there's at most one ambient light in a scene.
	//! This light type doesn't have a valid position, direction, or
	//! other properties, just a color.
	else if (type == aiLightSourceType::aiLightSource_AMBIENT)
		return "aiLightSource_AMBIENT";
	//! An area light is a rectangle with predefined size that uniformly
	//! emits light from one of its sides. The position is center of the
	//! rectangle and direction is its normal vector.
	else if (type == aiLightSourceType::aiLightSource_AREA)
		return "aiLightSource_AREA";
	else
		return "Unknown light enum type";
}

Mesh* ModelLoader::processMesh(aiMesh* assimpMesh, unsigned int meshIndex, const aiScene* scene, aiNode* assimpNode, ModelNode* modelNode)
{
	Mesh* mesh = new Mesh(); // Our own mesh
	mesh->modelNode = modelNode;

	// Loop through each of the mesh's vertices
	for (unsigned int i = 0; i < assimpMesh->mNumVertices; i++)
	{
		Mesh::Vertex vertex;
		// Positions
		vertex.position.x = assimpMesh->mVertices[i].x;
		vertex.position.y = assimpMesh->mVertices[i].y;
		vertex.position.z = assimpMesh->mVertices[i].z;

		if (fabs(vertex.position.x) > modelNode->radius)
			modelNode->radius = fabs(vertex.position.x);
		else if (fabs(vertex.position.y) > modelNode->radius)
			modelNode->radius = fabs(vertex.position.y);
		else if (fabs(vertex.position.z) > modelNode->radius)
			modelNode->radius = fabs(vertex.position.z);

		// Normals
		vertex.normal.x = assimpMesh->mNormals[i].x;
		vertex.normal.y = assimpMesh->mNormals[i].y;
		vertex.normal.z = assimpMesh->mNormals[i].z;
		// Texture Coordinates
		if (assimpMesh->mTextureCoords[0]) // Does the mesh contain texture coordinates?
		{
			// A vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
			// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
			vertex.texCoords.x = assimpMesh->mTextureCoords[0][i].x;
			vertex.texCoords.y = assimpMesh->mTextureCoords[0][i].y;
		}
		else
			vertex.texCoords = glm::vec2(0.0f, 0.0f);

		mesh->vertices.push_back(vertex);
	}

	// Process the meshes bones
	for (unsigned int boneIndex = 0; boneIndex < assimpMesh->mNumBones; boneIndex++)
	{
		aiBone * pBone = assimpMesh->mBones[boneIndex];

		for (unsigned int weightIndex = 0; weightIndex < pBone->mNumWeights; weightIndex++)
		{
			aiVertexWeight assimpVertexWeight = pBone->mWeights[weightIndex];
			glm::vec4* pBoneWeight = &mesh->vertices.at(assimpVertexWeight.mVertexId).boneWeights;
			glm::uvec4* pBoneIndices = &mesh->vertices.at(assimpVertexWeight.mVertexId).boneIndices;

			if (pBoneWeight->x == 0.0f)
			{
				pBoneIndices->x = boneIndex;
				pBoneWeight->x = assimpVertexWeight.mWeight;
			}
			else if (pBoneWeight->y == 0.0f)
			{
				pBoneIndices->y = boneIndex;
				pBoneWeight->y = assimpVertexWeight.mWeight;
			}
			else if (pBoneWeight->z == 0.0f)
			{
				pBoneIndices->z = boneIndex;
				pBoneWeight->z = assimpVertexWeight.mWeight;
			}
			else if (pBoneWeight->w == 0.0f)
			{
				pBoneIndices->w = boneIndex;
				pBoneWeight->w = assimpVertexWeight.mWeight;
			}
			else
			{ // Not more than four influcences on a bone are allowed
				string nodeName = "The node " + modelNode->name + " has more than four weights in the bone " + pBone->mName.C_Str() + ".";
				Debugger::getInstance()->pause(nodeName.c_str());
			}
		}

		if(mesh->assimpBoneNode == nullptr && assimpNode != nullptr)
			mesh->assimpBoneNode = assimpNode; // To be able to use the animator.cpp to the the correct for skinned pose matrices
		mesh->meshIndex = meshIndex;
	}
	

	// Now loop through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
	for (unsigned int i = 0; i < assimpMesh->mNumFaces; i++)
	{
		aiFace face = assimpMesh->mFaces[i];
		// Retrieve all indices of the face and store them in the indices vector
		for (GLuint j = 0; j < face.mNumIndices; j++)
			mesh->indices.push_back(face.mIndices[j]);
	}
	// Process textures and materials

	if (assimpMesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[assimpMesh->mMaterialIndex];
		vector<Mesh::Texture> text;
		text = loadMaterialTextures(material, aiTextureType_AMBIENT, "textureAmbient", &mesh->materials);
		mesh->textures.insert(mesh->textures.end(), text.begin(), text.end());
		text.clear();
		text = loadMaterialTextures(material, aiTextureType_DIFFUSE, "textureDiffuse", &mesh->materials);
		mesh->textures.insert(mesh->textures.end(), text.begin(), text.end());
		text.clear();
		text = loadMaterialTextures(material, aiTextureType_SPECULAR, "textureSpecular", &mesh->materials);
		mesh->textures.insert(mesh->textures.end(), text.begin(), text.end());
		text.clear();
	}

	mesh->link();

	return mesh; 
}

// Checks all material textures of a given type and loads the textures if they're not loaded yet.
vector<Mesh::Texture> ModelLoader::loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName, vector<glm::vec4>* materials)
{
	//Read materials
	float shininess = 0; // For new only one material! Has already standard values set if none found

	for (unsigned int i = 0; i < mat->mNumProperties; i++)
	{
		aiMaterialProperty* prop = mat->mProperties[i];
		if (prop->mType == aiPTI_Float)
		{
			int found = -1;
			string key = prop->mKey.C_Str();

			if (key.find("shininess") != string::npos) // Search key only once!
				found = 0;
			else if (key.find("ambient") != string::npos) // Not used in shaders
				found = 1;		
			else if (key.find("diffuse") != string::npos)
				found = 2;		
			else if (key.find("specular") != string::npos)
				found = 3;

			if (found == 0 || found == 2)  // for now only diffuse and shininess of it
			{// Extract float only if correct property is found
				float* data = reinterpret_cast<float*>(prop->mData);
				int length = (prop->mDataLength * sizeof(char)) / sizeof(float);

				if (length == 1 && data[0] != 0.0f && found == 0) // Shininess
					shininess = data[0];
				else if (length == 4 && data[0] != 0.0f && data[1] != 0.0f && data[2] != 0.0f)
				{
					if (found == 1) // Ambient
					{ // Not used in shaders and mesh.cpp
						//m.ambient = glm::vec4(data[0], data[1], data[2], 1.0f); // No 4th element, is okay!
					}
					else if (found == 2) // Diffuse, UNUSED IN SHADER! 
					{
						//m.r = data[0];
						//m.g = data[1];
						//m.b = data[2];
					}
					else if (found == 3) // Specular
					{ // Not used in shaders and mesh.cpp
						// m.specular = glm::vec4(data[0], data[1], data[2], 1.0f);
					}
				}
			}
		}
	}

	materials->push_back(glm::vec4(0, 0 , 0, shininess));

	//Read and load textures
	vector<Mesh::Texture> textures;
	bool loaded = false;

	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);
		string path = directory + str.C_Str();
		loaded = false;
		// Check if texture was loaded before and if so, continue to next iteration: skip loading a new texture but add the already loaded textures in the correct indices
		for (unsigned int j = 0; !loaded && j < loadedTextures.size(); j++)
			if (loadedTextures[j].path == path)
			{
				textures.push_back(loadedTextures[j]);
				loaded = true;
			}

		if (!loaded)
		{   // If texture hasn't been loaded already, load it
			Mesh::Texture t;

			t.id = loadPicture(path);
			t.name = typeName;
			t.path = path;
			textures.push_back(t);
			loadedTextures.push_back(t);// Store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
		}
	}

	return textures;
}

/*Loads one image and returns the gl texture handle for it.*/
unsigned int ModelLoader::loadPicture(string path)
{
	ilInit();
	ILuint ilHandle;
	ilGenImages(1, &ilHandle);
	ilBindImage(ilHandle);

	ILboolean error = ilLoadImage((ILstring) path.c_str());// load  the image and check to see if everything went OK

	if (error != IL_TRUE) {
		ilDeleteImages(1, &ilHandle);
		string msg = "Texture: Failed to load the image from " + path + " , error code: " + to_string(error) + ".";
		Debugger::getInstance()->pauseExit(msg);
	}

	ilEnable(IL_ORIGIN_SET);// match image origin to OpenGL’s
	ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
	ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);// Convert image to RGBA with unsigned byte data type

	unsigned int glHandle;
	glGenTextures(1, &glHandle);
	glBindTexture(GL_TEXTURE_2D, glHandle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 0, GL_RGBA, GL_UNSIGNED_BYTE, ilGetData());
	// This SRGB and RGB for only one gamma correction can cause color errors, diffuse specular, and normal maps should work finde though
    // Source: http://learnopengl.com/#!Advanced-Lighting/Gamma-Correction
	
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//Repeat texture if outside of the border
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // The combination of both parameters can create trilinear filtering, see https://www.informatik-forum.at/showthread.php?107156-Textur-Sampling-Mip-Mapping
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	ilDeleteImages(1, &ilHandle); // Because we have already copied image data into texture data we can release memory used by image.
	return glHandle;
}

void ModelLoader::setTextureState(Node* current, int paramMin, int paramMax) {
	ModelNode* mn = dynamic_cast<ModelNode*>(current);

	if (mn && mn->meshes.size() > 0)
		for (Mesh* me : mn->meshes)
			for (Mesh::Texture t : me->textures)
			{
				glBindTexture(GL_TEXTURE_2D, t.id);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, paramMin); // The combination of both parameters can create trilinear filtering, see https://www.informatik-forum.at/showthread.php?107156-Textur-Sampling-Mip-Mapping
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, paramMax);
				glBindTexture(GL_TEXTURE_2D, 0);
			}

	for (Node* child : current->children)
		setTextureState(child, paramMin, paramMax);
}