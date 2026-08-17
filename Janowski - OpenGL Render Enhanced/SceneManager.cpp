///////////////////////////////////////////////////////////////////////////////
// shadermanager.cpp
// ============
// manage the loading and rendering of 3D scenes
//
//  AUTHOR: Eryk Janowski
//	Created for CS-499 August 2nd, 2026
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include <glm/gtx/transform.hpp>

// declaration of global variables
namespace
{
	const char* g_ModelName = "model";
	const char* g_ColorValueName = "objectColor";
	const char* g_TextureValueName = "objectTexture";
	const char* g_UseTextureName = "bUseTexture";
	const char* g_UseLightingName = "bUseLighting";
}

/***********************************************************
 *  SceneManager()
 *
 *  The constructor for the class
 ***********************************************************/
SceneManager::SceneManager(ShaderManager *pShaderManager)
{
	m_pShaderManager = pShaderManager;
	m_basicMeshes = new ShapeMeshes();
}

/***********************************************************
 *  ~SceneManager()
 *
 *  The destructor for the class
 ***********************************************************/
SceneManager::~SceneManager()
{
	m_pShaderManager = NULL;
	delete m_basicMeshes;
	m_basicMeshes = NULL;
}

/***********************************************************
 *  CreateGLTexture()
 *
 *  This method is used for loading textures from image files,
 *  configuring the texture mapping parameters in OpenGL,
 *  generating the mipmaps, and loading the read texture into
 *  the next available texture slot in memory.
 ***********************************************************/
bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
	int width = 0;
	int height = 0;
	int colorChannels = 0;
	GLuint textureID = 0;

	// indicate to always flip images vertically when loaded
	stbi_set_flip_vertically_on_load(true);

	// try to parse the image data from the specified image file
	unsigned char* image = stbi_load(
		filename,
		&width,
		&height,
		&colorChannels,
		0);

	// if the image was successfully read from the image file
	if (image)
	{
		std::cout << "Successfully loaded image:" << filename << ", width:" << width << ", height:" << height << ", channels:" << colorChannels << std::endl;

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// if the loaded image is in RGB format
		if (colorChannels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		// if the loaded image is in RGBA format - it supports transparency
		else if (colorChannels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			std::cout << "Not implemented to handle image with " << colorChannels << " channels" << std::endl;
			return false;
		}

		// generate the texture mipmaps for mapping textures to lower resolutions
		glGenerateMipmap(GL_TEXTURE_2D);

		// free the image data from local memory
		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		// register the loaded texture and associate it with the special tag string
		m_textureIDs[m_loadedTextures].ID = textureID;
		m_textureIDs[m_loadedTextures].tag = tag;
		m_loadedTextures++;

		return true;
	}

	std::cout << "Could not load image:" << filename << std::endl;

	// Error loading the image
	return false;
}

/***********************************************************
 *  BindGLTextures()
 *
 *  This method is used for binding the loaded textures to
 *  OpenGL texture memory slots.  There are up to 16 slots.
 ***********************************************************/
void SceneManager::BindGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  DestroyGLTextures()
 *
 *  This method is used for freeing the memory in all the
 *  used texture memory slots.
 ***********************************************************/
void SceneManager::DestroyGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glGenTextures(1, &m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  FindTextureID()
 *
 *  This method is used for getting an ID for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureID(std::string tag)
{
	int textureID = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureID = m_textureIDs[index].ID;
			bFound = true;
		}
		else
			index++;
	}

	return(textureID);
}

/***********************************************************
 *  FindTextureSlot()
 *
 *  This method is used for getting a slot index for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureSlot(std::string tag)
{
	int textureSlot = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureSlot = index;
			bFound = true;
		}
		else
			index++;
	}

	return(textureSlot);
}

/***********************************************************
 *  FindMaterial()
 *
 *  This method is used for getting a material from the previously
 *  defined materials list that is associated with the passed in tag.
 ***********************************************************/
bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
	if (m_objectMaterials.size() == 0)
	{
		return(false);
	}

	int index = 0;
	bool bFound = false;
	while ((index < m_objectMaterials.size()) && (bFound == false))
	{
		if (m_objectMaterials[index].tag.compare(tag) == 0)
		{
			bFound = true;
			material.ambientColor = m_objectMaterials[index].ambientColor;
			material.ambientStrength = m_objectMaterials[index].ambientStrength;
			material.diffuseColor = m_objectMaterials[index].diffuseColor;
			material.specularColor = m_objectMaterials[index].specularColor;
			material.shininess = m_objectMaterials[index].shininess;
		}
		else
		{
			index++;
		}
	}

	return(true);
}

/***********************************************************
 *  SetTransformations()
 *
 *  This method is used for setting the transform buffer
 *  using the passed in transformation values.
 ***********************************************************/
void SceneManager::SetTransformations(
	glm::vec3 scaleXYZ,
	float XrotationDegrees,
	float YrotationDegrees,
	float ZrotationDegrees,
	glm::vec3 positionXYZ)
{
	// variables for this method
	glm::mat4 modelView;
	glm::mat4 scale;
	glm::mat4 rotationX;
	glm::mat4 rotationY;
	glm::mat4 rotationZ;
	glm::mat4 translation;

	// set the scale value in the transform buffer
	scale = glm::scale(scaleXYZ);
	// set the rotation values in the transform buffer
	rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	// set the translation value in the transform buffer
	translation = glm::translate(positionXYZ);

	modelView = translation * rotationX * rotationY * rotationZ * scale;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(g_ModelName, modelView);
	}
}

/***********************************************************
 *  SetShaderColor()
 *
 *  This method is used for setting the passed in color
 *  into the shader for the next draw command
 ***********************************************************/
void SceneManager::SetShaderColor(
	float redColorValue,
	float greenColorValue,
	float blueColorValue,
	float alphaValue)
{
	// variables for this method
	glm::vec4 currentColor;

	currentColor.r = redColorValue;
	currentColor.g = greenColorValue;
	currentColor.b = blueColorValue;
	currentColor.a = alphaValue;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, false);
		m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
	}
}

/***********************************************************
 *  SetShaderTexture()
 *
 *  This method is used for setting the texture data
 *  associated with the passed in ID into the shader.
 ***********************************************************/
void SceneManager::SetShaderTexture(
	std::string textureTag)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, true);

		int textureID = -1;
		textureID = FindTextureSlot(textureTag);
		m_pShaderManager->setSampler2DValue(g_TextureValueName, textureID);
	}
}

/***********************************************************
 *  SetTextureUVScale()
 *
 *  This method is used for setting the texture UV scale
 *  values into the shader.
 ***********************************************************/
void SceneManager::SetTextureUVScale(float u, float v)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
	}
}

/***********************************************************
 *  SetShaderMaterial()
 *
 *  This method is used for passing the material values
 *  into the shader.
 ***********************************************************/
void SceneManager::SetShaderMaterial(
	std::string materialTag)
{
	if (m_objectMaterials.size() > 0)
	{
		OBJECT_MATERIAL material;
		bool bReturn = false;

		bReturn = FindMaterial(materialTag, material);
		if (bReturn == true)
		{
			m_pShaderManager->setVec3Value("material.ambientColor", material.ambientColor);
			m_pShaderManager->setFloatValue("material.ambientStrength", material.ambientStrength);
			m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor);
			m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
			m_pShaderManager->setFloatValue("material.shininess", material.shininess);
		}
	}
}

/**************************************************************/
/*** STUDENTS CAN MODIFY the code in the methods BELOW for  ***/
/*** preparing and rendering their own 3D replicated scenes.***/
/*** Please refer to the code in the OpenGL sample project  ***/
/*** for assistance.                                        ***/
/**************************************************************/

/***********************************************************
  *  DefineObjectMaterials()
  *
  *  This method is used for configuring the various material
  *  settings for all of the objects within the 3D scene.
  ***********************************************************/
void SceneManager::DefineObjectMaterials()
{
	// Vector to store OBJECT_MATERIAL structs
	std::vector<OBJECT_MATERIAL> materialData = {
	//	AmbientIntensity	AmbientColor				DiffuseColor			SpecularColor			Shininess	tag
		{ 0.1f,				{ 0.01f, 0.01f, 0.01f },	{ 0.2f, 0.2f, 0.2f },	{ 0.3f, 0.3f, 0.3f },	75.0f,		"metal"		},
		{ 0.1f,				{ 0.05f, 0.05f, 0.05f },	{ 0.2f, 0.2f, 0.2f },	{ 0.1f, 0.1f, 0.1f },	0.3f,		"asphalt"	},
		{ 0.2f,				{ 0.2f,  0.2f,  0.2f  },	{ 0.5f, 0.5f, 0.5f },	{ 0.4f, 0.4f, 0.4f },	0.1f,		"cement"	},
		{ 0.2f,				{ 0.4f,  0.4f,  0.4f  },	{ 0.3f, 0.3f, 0.3f },	{ 0.1f, 0.1f, 0.1f },	0.3f,		"wood"		},
		{ 0.2f,				{ 0.4f,  0.3f,  0.3f  },	{ 0.7f, 0.7f, 0.6f },	{ 0.4f, 0.5f, 0.6f },	10.0f,		"stone"		},
		{ 0.1f,				{ 0.4f,  0.5f,  0.5f  },	{ 0.5f, 0.6f, 0.5f },	{ 0.6f, 0.6f, 0.6f },	80.0f,		"glass"		},
		{ 0.3f,				{ 0.1f,  0.1f,  0.1f  },	{ 0.1f, 0.3f, 0.1f },	{ 0.1f, 0.5f, 0.1f },	 0.3f,		"plant"		}
	};

	for (const auto& material : materialData) {
		m_objectMaterials.push_back(material);
	}
}

/***********************************************************
 *  SetupSceneLights()
 *
 *  This method is called to add and configure the light
 *  sources for the 3D scene.  There are up to 4 light sources.
 ***********************************************************/
void SceneManager::SetupSceneLights()
{
	// this line of code is NEEDED for telling the shaders to render 
	// the 3D scene with custom lighting, if no light sources have
	// been added then the display window will be black - to use the 
	// default OpenGL lighting then comment out the following line
	//m_pShaderManager->setBoolValue(g_UseLightingName, true);

	// Four light sources set in a square shape above the models to represent sunlight. Plain white light

	// Struct to hold values for lights
	struct Light {
		std::string id;
		glm::vec3 position;
		glm::vec3 ambientColor;
		glm::vec3 diffuseColor;
		glm::vec3 specularColor;
		float focalStrength;
		float specularIntensity;
	};

	// Vector to hold Light structs
	std::vector<Light> lightData = {
		{
			"0",
			{50.0f, 50.0f, 50.0f},
			{0.01f, 0.01f, 0.01f},
			{0.4f,  0.4f,  0.4f},
			{1.0f,  1.0f,  1.0f},
			32.0f,
			1.0f
		},
		{
			"1",
			{50.0f, 50.0f, -50.0f},
			{0.01f, 0.01f, 0.01f},
			{0.4f,  0.4f,  0.4f},
			{1.0f,  1.0f,  1.0f},
			32.0f,
			1.0f
		},
		{
			"2",
			{-50.0f, 50.0f, -50.0f},
			{0.01f, 0.01f, 0.01f},
			{0.4f,  0.4f,  0.4f},
			{1.0f,  1.0f,  1.0f},
			32.0f,
			1.0f
		},
		{
			"3",
			{-50.0f, 50.0f, 50.0f},
			{0.01f, 0.01f, 0.01f},
			{0.4f,  0.4f,  0.4f},
			{1.0f,  1.0f,  1.0f},
			32.0f,
			1.0f
		}
	};

	for (const auto& light : lightData) {
		m_pShaderManager->setVec3Value("lightSources[" + light.id + "].position", light.position.x, light.position.y, light.position.z);
		m_pShaderManager->setVec3Value("lightSources[" + light.id + "].ambientColor", light.ambientColor.x, light.ambientColor.y, light.ambientColor.z);
		m_pShaderManager->setVec3Value("lightSources[" + light.id + "].diffuseColor", light.diffuseColor.x, light.diffuseColor.y, light.diffuseColor.z);
		m_pShaderManager->setVec3Value("lightSources[" + light.id + "].specularColor", light.specularColor.x, light.specularColor.y, light.specularColor.z);
		m_pShaderManager->setFloatValue("lightSources[" + light.id + "].focalStrength", light.focalStrength);
		m_pShaderManager->setFloatValue("lightSources[" + light.id + "].specularIntensity", light.specularIntensity);
	}

	m_pShaderManager->setBoolValue("bUseLighting", true);
}

/***********************************************************
  *  LoadSceneTextures()
  *
  *  This method is used for preparing the 3D scene by loading
  *  the shapes, textures in memory to support the 3D scene
  *  rendering
  ***********************************************************/
void SceneManager::LoadSceneTextures()
{
	/*** Load up to 16 textures from the textureData vector	***/

	bool bReturn = false; 
	
	std::vector<std::pair<std::string, std::string>> textureData = {
		{"../../Utilities/textures/asphalt.jpg", "asphalt"},
		{"../../Utilities/textures/concrete.jpg", "concrete"},
		{"../../Utilities/textures/grass.jpg", "grass"},
		{"../../Utilities/textures/stainless.jpg", "stainless"},
		{"../../Utilities/textures/stainless_end.jpg", "stainless_end"},
		{"../../Utilities/textures/knife_handle.jpg", "wood"},
		{"../../Utilities/textures/drywall.jpg", "glass"}
	};

	for (const auto& texture : textureData) {
		bReturn = CreateGLTexture(texture.first.c_str(), texture.second);
	}

	// after the texture image data is loaded into memory, the
	// loaded textures need to be bound to texture slots - there
	// are a total of 16 available slots for scene textures
	BindGLTextures();
}

/***********************************************************
 *  PrepareScene()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene 
 *  rendering
 ***********************************************************/
void SceneManager::PrepareScene()
{
	// load the textures for the 3D scene
	LoadSceneTextures();

	// define materials and set up lights
	DefineObjectMaterials();
	SetupSceneLights();

	// only one instance of a particular mesh needs to be
	// loaded in memory no matter how many times it is drawn
	// in the rendered 3D scene

	m_basicMeshes->LoadPlaneMesh();
	m_basicMeshes->LoadBoxMesh();
	m_basicMeshes->LoadSphereMesh();
	m_basicMeshes->LoadCylinderMesh();
	m_basicMeshes->LoadPrismMesh();
}

/***********************************************************
 *  RenderScene()
 *
 *  This method is used for rendering the 3D scene by 
 *  transforming and drawing the basic 3D shapes
 ***********************************************************/
void SceneManager::RenderScene()
{
	struct Mesh {
		glm::vec3 scale;
		glm::vec3 position;
		glm::vec3 rotation;
		glm::vec2 UVScale;
		std::string texture;
		std::string material;
		int id;
		glm::vec3 cylinder;
	};

	// Data structure to contain all object mesh rendering data
	std::vector<Mesh> meshData = {
		// Scale				Posiiton					Rotation				UV Scale		Texture			Material		ID	CylinderBools
		{ {30.0f, 1.0f, 5.0f},	{0.0f, 0.0f, 5.0f},			{0.0f, 0.0f, 0.0f},		{30.0, 5.0},	"asphalt",		"asphalt",		0,	{} }, // Road
		{ {30.0f, 1.0f, 5.0f},	{0.0f, 0.0f, 15.0f},		{0.0f, 0.0f, 0.0f},		{30.0, 5.0},	"asphalt",		"cement",		0,	{} }, // Sidewalk Close
		{ {30.0f, 1.0f, 30.0f}, {0.0f, 0.0f, -30.0f},		{0.0f, 0.0f, 0.0f},		{30.0, 30.0},	"asphalt",		"cement",		0,	{} }, // Sidewalk Far
		{ {6.0f, 1.0f, 1.0f},	{-12.0f, 0.5f, -1.0f},		{0.0f, 0.0f, 0.0f},		{6.0, 1.0},		"concrete",		"cement",		1,	{} }, // Planter Box L Concrete
		{ {6.0f, 1.0f, 1.0f},	{0.0f, 0.5f, -1.0f},		{0.0f, 0.0f, 0.0f},		{6.0, 1.0},		"concrete",		"cement",		1,	{} }, // Planter Box C Concrete
		{ {6.0f, 1.0f, 1.0f},	{12.0f, 0.5f, -1.0f},		{0.0f, 0.0f, 0.0f},		{6.0, 1.0},		"concrete",		"cement",		1,	{} }, // Planter Box R Concrete
		{ {6.0f, 0.05f, 1.0f},	{-12.0f, 1.025f, -1.0f},	{0.0f, 0.0f, 0.0f},		{1.0, 4.0},		"grass",		"plant",		1,	{} }, // Planter Box L Grass
		{ {6.0f, 0.05f, 1.0f},	{0.0f, 1.025f, -1.0f},		{0.0f, 0.0f, 0.0f},		{1.0, 4.0},		"grass",		"plant",		1,	{} }, // Planter Box C Grass
		{ {6.0f, 0.05f, 1.0f},	{12.0f, 1.025f, -1.0f},		{0.0f, 0.0f, 0.0f},		{1.0, 4.0},		"grass",		"plant",		1,	{} }, // Planter Box R Grass
		{ {0.5f, 0.5f, 0.5f},	{-14.5f, 1.0f, -1.0f},		{0.0f, 0.0f, 0.0f},		{1.0, 4.0},		"grass",		"plant",		3,	{} }, // Planter Box L Bush 1
		{ {0.5f, 0.5f, 0.5f},	{-13.5f, 1.0f, -1.0f},		{0.0f, 0.0f, 0.0f},		{1.0, 4.0},		"grass",		"plant",		3,	{} }, // Planter Box L Bush 2
		{ {0.5f, 0.5f, 0.5f},	{-12.5f, 1.0f, -1.0f},		{0.0f, 0.0f, 0.0f},		{1.0, 4.0},		"grass",		"plant",		3,	{} }, // Planter Box L Bush 3
		{ {0.5f, 0.5f, 0.5f},	{-11.5f, 1.0f, -1.0f},		{0.0f, 0.0f, 0.0f},		{1.0, 4.0},		"grass",		"plant",		3,	{} }, // Planter Box L Bush 4
		{ {0.5f, 0.5f, 0.5f},	{-10.5f, 1.0f, -1.0f},		{0.0f, 0.0f, 0.0f},		{1.0, 4.0},		"grass",		"plant",		3,	{} }, // Planter Box L Bush 5
		{ {0.5f, 0.5f, 0.5f},	{-9.5f, 1.0f, -1.0f},		{0.0f, 0.0f, 0.0f},		{1.0, 4.0},		"grass",		"plant",		3,	{} }, // Planter Box L Bush 6
		{ {0.5f, 0.5f, 0.5f},	{-2.5f, 1.0f, -1.0f},		{0.0f, 0.0f, 0.0f},		{1.0, 4.0},		"grass",		"plant",		3,	{} }, // Planter Box C Bush 1
		{ {0.5f, 0.5f, 0.5f},	{-1.5f, 1.0f, -1.0f},		{0.0f, 0.0f, 0.0f},		{1.0, 4.0},		"grass",		"plant",		3,	{} }, // Planter Box C Bush 2
		{ {0.5f, 0.5f, 0.5f},	{-0.5f, 1.0f, -1.0f},		{0.0f, 0.0f, 0.0f},		{1.0, 4.0},		"grass",		"plant",		3,	{} }, // Planter Box C Bush 3
		{ {0.5f, 0.5f, 0.5f},	{0.5f, 1.0f, -1.0f},		{0.0f, 0.0f, 0.0f},		{1.0, 4.0},		"grass",		"plant",		3,	{} }, // Planter Box C Bush 4
		{ {0.5f, 0.5f, 0.5f},	{1.5f, 1.0f, -1.0f},		{0.0f, 0.0f, 0.0f},		{1.0, 4.0},		"grass",		"plant",		3,	{} }, // Planter Box C Bush 5
		{ {0.5f, 0.5f, 0.5f},	{2.5f, 1.0f, -1.0f},		{0.0f, 0.0f, 0.0f},		{1.0, 4.0},		"grass",		"plant",		3,	{} }, // Planter Box C Bush 6
		{ {0.5f, 0.5f, 0.5f},	{9.5f, 1.0f, -1.0f},		{0.0f, 0.0f, 0.0f},		{1.0, 4.0},		"grass",		"plant",		3,	{} }, // Planter Box R Bush 1
		{ {0.5f, 0.5f, 0.5f},	{10.5f, 1.0f, -1.0f},		{0.0f, 0.0f, 0.0f},		{1.0, 4.0},		"grass",		"plant",		3,	{} }, // Planter Box R Bush 2
		{ {0.5f, 0.5f, 0.5f},	{11.5f, 1.0f, -1.0f},		{0.0f, 0.0f, 0.0f},		{1.0, 4.0},		"grass",		"plant",		3,	{} }, // Planter Box R Bush 3
		{ {0.5f, 0.5f, 0.5f},	{12.5f, 1.0f, -1.0f},		{0.0f, 0.0f, 0.0f},		{1.0, 4.0},		"grass",		"plant",		3,	{} }, // Planter Box R Bush 4
		{ {0.5f, 0.5f, 0.5f},	{13.5f, 1.0f, -1.0f},		{0.0f, 0.0f, 0.0f},		{1.0, 4.0},		"grass",		"plant",		3,	{} }, // Planter Box R Bush 5
		{ {0.5f, 0.5f, 0.5f},	{14.5f, 1.0f, -1.0f},		{0.0f, 0.0f, 0.0f},		{1.0, 4.0},		"grass",		"plant",		3,	{} }, // Planter Box R Bush 6
		{ {0.25f, 0.8f, 0.25f},	{-8.5f, 0.0f, -1.0f},		{0.0f, 0.0f, 0.0f},		{1.0, 1.0},		"stainless",	"metal",		2,	{false, false, true}	}, // Bollard 1 Body
		{ {0.25f, 0.8f, 0.25f},	{-8.5f, 0.0f, -1.0f},		{0.0f, 0.0f, 0.0f},		{1.0, 1.0},		"stainless_end","metal",		2,	{true, true, false}		}, // Bollard 1 Top
		{ {0.25f, 0.8f, 0.25f},	{-7.5f, 0.0f, -1.0f},		{0.0f, 0.0f, 0.0f},		{1.0, 1.0},		"stainless",	"metal",		2,	{false, false, true}	}, // Bollard 2 Body
		{ {0.25f, 0.8f, 0.25f},	{-7.5f, 0.0f, -1.0f},		{0.0f, 0.0f, 0.0f},		{1.0, 1.0},		"stainless_end","metal",		2,	{true, true, false}		}, // Bollard 2 Top
		{ {0.25f, 0.8f, 0.25f},	{-6.5f, 0.0f, -1.0f},		{0.0f, 0.0f, 0.0f},		{1.0, 1.0},		"stainless",	"metal",		2,	{false, false, true}	}, // Bollard 3 Body
		{ {0.25f, 0.8f, 0.25f},	{-6.5f, 0.0f, -1.0f},		{0.0f, 0.0f, 0.0f},		{1.0, 1.0},		"stainless_end","metal",		2,	{true, true, false}		}, // Bollard 3 Top
		{ {0.25f, 0.8f, 0.25f},	{-5.5f, 0.0f, -1.0f},		{0.0f, 0.0f, 0.0f},		{1.0, 1.0},		"stainless",	"metal",		2,	{false, false, true}	}, // Bollard 4 Body
		{ {0.25f, 0.8f, 0.25f},	{-5.5f, 0.0f, -1.0f},		{0.0f, 0.0f, 0.0f},		{1.0, 1.0},		"stainless_end","metal",		2,	{true, true, false}		}, // Bollard 4 Top
		{ {0.25f, 0.8f, 0.25f},	{-4.5f, 0.0f, -1.0f},		{0.0f, 0.0f, 0.0f},		{1.0, 1.0},		"stainless",	"metal",		2,	{false, false, true}	}, // Bollard 5 Body
		{ {0.25f, 0.8f, 0.25f},	{-4.5f, 0.0f, -1.0f},		{0.0f, 0.0f, 0.0f},		{1.0, 1.0},		"stainless_end","metal",		2,	{true, true, false}		}, // Bollard 5 Top
		{ {0.25f, 0.8f, 0.25f},	{-3.5f, 0.0f, -1.0f},		{0.0f, 0.0f, 0.0f},		{1.0, 1.0},		"stainless",	"metal",		2,	{false, false, true}	}, // Bollard 6 Body
		{ {0.25f, 0.8f, 0.25f},	{-3.5f, 0.0f, -1.0f},		{0.0f, 0.0f, 0.0f},		{1.0, 1.0},		"stainless_end","metal",		2,	{true, true, false}		}, // Bollard 6 Top
		{ {0.25f, 0.8f, 0.25f},	{3.5f, 0.0f, -1.0f},		{0.0f, 0.0f, 0.0f},		{1.0, 1.0},		"stainless",	"metal",		2,	{false, false, true}	}, // Bollard 7 Body
		{ {0.25f, 0.8f, 0.25f},	{3.5f, 0.0f, -1.0f},		{0.0f, 0.0f, 0.0f},		{1.0, 1.0},		"stainless_end","metal",		2,	{true, true, false}		}, // Bollard 7 Top
		{ {0.25f, 0.8f, 0.25f},	{4.5f, 0.0f, -1.0f},		{0.0f, 0.0f, 0.0f},		{1.0, 1.0},		"stainless",	"metal",		2,	{false, false, true}	}, // Bollard 8 Body
		{ {0.25f, 0.8f, 0.25f},	{4.5f, 0.0f, -1.0f},		{0.0f, 0.0f, 0.0f},		{1.0, 1.0},		"stainless_end","metal",		2,	{true, true, false}		}, // Bollard 8 Top
		{ {0.25f, 0.8f, 0.25f},	{5.5f, 0.0f, -1.0f},		{0.0f, 0.0f, 0.0f},		{1.0, 1.0},		"stainless",	"metal",		2,	{false, false, true}	}, // Bollard 9 Body
		{ {0.25f, 0.8f, 0.25f},	{5.5f, 0.0f, -1.0f},		{0.0f, 0.0f, 0.0f},		{1.0, 1.0},		"stainless_end","metal",		2,	{true, true, false}		}, // Bollard 9Top
		{ {0.25f, 0.8f, 0.25f},	{6.5f, 0.0f, -1.0f},		{0.0f, 0.0f, 0.0f},		{1.0, 1.0},		"stainless",	"metal",		2,	{false, false, true}	}, // Bollard 10 Body
		{ {0.25f, 0.8f, 0.25f},	{6.5f, 0.0f, -1.0f},		{0.0f, 0.0f, 0.0f},		{1.0, 1.0},		"stainless_end","metal",		2,	{true, true, false}		}, // Bollard 10 Top
		{ {0.25f, 0.8f, 0.25f},	{7.5f, 0.0f, -1.0f},		{0.0f, 0.0f, 0.0f},		{1.0, 1.0},		"stainless",	"metal",		2,	{false, false, true}	}, // Bollard 11 Body
		{ {0.25f, 0.8f, 0.25f},	{7.5f, 0.0f, -1.0f},		{0.0f, 0.0f, 0.0f},		{1.0, 1.0},		"stainless_end","metal",		2,	{true, true, false}		}, // Bollard 11 Top
		{ {0.25f, 0.8f, 0.25f},	{8.5f, 0.0f, -1.0f},		{0.0f, 0.0f, 0.0f},		{1.0, 1.0},		"stainless",	"metal",		2,	{false, false, true}	}, // Bollard 12 Body
		{ {0.25f, 0.8f, 0.25f},	{8.5f, 0.0f, -1.0f},		{0.0f, 0.0f, 0.0f},		{1.0, 1.0},		"stainless_end","metal",		2,	{true, true, false}		}, // Bollard 12 Top
		{ {40.0f, 20.0f, 1.0f},	{0.0f, 10.0f, -10.0f},		{0.0f, 0.0f, 0.0f},		{40.0, 30.0},	"concrete",		"stone",		1,	{} }, // Main Building
		{ {17.0f, 3.0f, 3.0f},	{0.0f, 12.0f, -8.0f},		{0.0f, 0.0f, 0.0f},		{17.0, 3.0},	"concrete",		"stone",		1,	{} }, // Pillar Roof Bottom
		{ {18.0f, 4.0f, 3.0f},	{0.0f, 15.0f, -8.0f},		{-90.0f, 0.0f, 180.0f},	{18.0, 3.0},	"concrete",		"stone",		4,	{} }, // Pillar Roof Top
		{ {0.7f, 12.0f, 0.7f},	{-7.5, 0.0f, -7.5f},		{0.0f, 0.0f, 0.0f},		{18.0, 3.0},	"concrete",		"stone",		2,	{true, true, true}		}, // Pillar 1
		{ {0.7f, 12.0f, 0.7f},	{-4.5, 0.0f, -7.5f},		{0.0f, 0.0f, 0.0f},		{18.0, 3.0},	"concrete",		"stone",		2,	{true, true, true}		}, // Pillar 2
		{ {0.7f, 12.0f, 0.7f},	{-1.5, 0.0f, -7.5f},		{0.0f, 0.0f, 0.0f},		{18.0, 3.0},	"concrete",		"stone",		2,	{true, true, true}		}, // Pillar 3
		{ {0.7f, 12.0f, 0.7f},	{1.5, 0.0f, -7.5f},			{0.0f, 0.0f, 0.0f},		{18.0, 3.0},	"concrete",		"stone",		2,	{true, true, true}		}, // Pillar 4
		{ {0.7f, 12.0f, 0.7f},	{4.5, 0.0f, -7.5f},			{0.0f, 0.0f, 0.0f},		{18.0, 3.0},	"concrete",		"stone",		2,	{true, true, true}		}, // Pillar 5
		{ {0.7f, 12.0f, 0.7f},	{7.5, 0.0f, -7.5f},			{0.0f, 0.0f, 0.0f},		{18.0, 3.0},	"concrete",		"stone",		2,	{true, true, true}		}, // Pillar 6
		{ {2.0f, 3.0f, 0.1f},	{-6.0, 1.5f, -9.5f},		{0.0f, 0.0f, 0.0f},		{4.0, 6.0},		"wood",			"wood",			1,	{} }, // Door 1
		{ {2.0f, 3.0f, 0.1f},	{-3.0, 1.5f, -9.5f},		{0.0f, 0.0f, 0.0f},		{4.0, 6.0},		"wood",			"wood",			1,	{} }, // Door 2
		{ {2.0f, 3.0f, 0.1f},	{0.0, 1.5f, -9.5f},			{0.0f, 0.0f, 0.0f},		{4.0, 6.0},		"wood",			"wood",			1,	{} }, // Door 3
		{ {2.0f, 3.0f, 0.1f},	{3.0, 1.5f, -9.5f},			{0.0f, 0.0f, 0.0f},		{4.0, 6.0},		"wood",			"wood",			1,	{} }, // Door 4
		{ {2.0f, 3.0f, 0.1f},	{6.0, 1.5f, -9.5f},			{0.0f, 0.0f, 0.0f},		{4.0, 6.0},		"wood",			"wood",			1,	{} }, // Door 5
		{ {2.0f, 2.5f, 0.1f},	{-18.0, 1.75f, -9.5f},		{0.0f, 0.0f, 0.0f},		{4.0, 5.0},		"glass",		"glass",		1,	{} }, // Window 1, 1
		{ {2.0f, 2.5f, 0.1f},	{-18.0, 5.5f, -9.5f},		{0.0f, 0.0f, 0.0f},		{4.0, 5.0},		"glass",		"glass",		1,	{} }, // Window 1, 2
		{ {2.0f, 2.5f, 0.1f},	{-18.0, 8.5f, -9.5f},		{0.0f, 0.0f, 0.0f},		{4.0, 5.0},		"glass",		"glass",		1,	{} }, // Window 1, 3
		{ {2.0f, 2.5f, 0.1f},	{-15.0, 1.75f, -9.5f},		{0.0f, 0.0f, 0.0f},		{4.0, 5.0},		"glass",		"glass",		1,	{} }, // Window 2, 1
		{ {2.0f, 2.5f, 0.1f},	{-15.0, 5.5f, -9.5f},		{0.0f, 0.0f, 0.0f},		{4.0, 5.0},		"glass",		"glass",		1,	{} }, // Window 2, 2
		{ {2.0f, 2.5f, 0.1f},	{-15.0, 8.5f, -9.5f},		{0.0f, 0.0f, 0.0f},		{4.0, 5.0},		"glass",		"glass",		1,	{} }, // Window 2, 3
		{ {2.0f, 2.5f, 0.1f},	{-12.0, 1.75f, -9.5f},		{0.0f, 0.0f, 0.0f},		{4.0, 5.0},		"glass",		"glass",		1,	{} }, // Window 3, 1
		{ {2.0f, 2.5f, 0.1f},	{-12.0, 5.5f, -9.5f},		{0.0f, 0.0f, 0.0f},		{4.0, 5.0},		"glass",		"glass",		1,	{} }, // Window 3, 2
		{ {2.0f, 2.5f, 0.1f},	{-12.0, 8.5f, -9.5f},		{0.0f, 0.0f, 0.0f},		{4.0, 5.0},		"glass",		"glass",		1,	{} }, // Window 3, 3
		{ {2.0f, 2.5f, 0.1f},	{-9.0, 1.75f, -9.5f},		{0.0f, 0.0f, 0.0f},		{4.0, 5.0},		"glass",		"glass",		1,	{} }, // Window 4, 1
		{ {2.0f, 2.5f, 0.1f},	{-9.0, 5.5f, -9.5f},		{0.0f, 0.0f, 0.0f},		{4.0, 5.0},		"glass",		"glass",		1,	{} }, // Window 4, 2
		{ {2.0f, 2.5f, 0.1f},	{-9.0, 8.5f, -9.5f},		{0.0f, 0.0f, 0.0f},		{4.0, 5.0},		"glass",		"glass",		1,	{} }, // Window 4, 3
		{ {2.0f, 2.5f, 0.1f},	{-6.0, 5.5f, -9.5f},		{0.0f, 0.0f, 0.0f},		{4.0, 5.0},		"glass",		"glass",		1,	{} }, // Window 5, 2
		{ {2.0f, 2.5f, 0.1f},	{-6.0, 8.5f, -9.5f},		{0.0f, 0.0f, 0.0f},		{4.0, 5.0},		"glass",		"glass",		1,	{} }, // Window 5, 3
		{ {2.0f, 2.5f, 0.1f},	{-3.0, 5.5f, -9.5f},		{0.0f, 0.0f, 0.0f},		{4.0, 5.0},		"glass",		"glass",		1,	{} }, // Window 6, 2
		{ {2.0f, 2.5f, 0.1f},	{-3.0, 8.5f, -9.5f},		{0.0f, 0.0f, 0.0f},		{4.0, 5.0},		"glass",		"glass",		1,	{} }, // Window 6, 3
		{ {2.0f, 2.5f, 0.1f},	{0.0, 5.5f, -9.5f},			{0.0f, 0.0f, 0.0f},		{4.0, 5.0},		"glass",		"glass",		1,	{} }, // Window 7, 2
		{ {2.0f, 2.5f, 0.1f},	{0.0, 8.5f, -9.5f},			{0.0f, 0.0f, 0.0f},		{4.0, 5.0},		"glass",		"glass",		1,	{} }, // Window 7, 3
		{ {2.0f, 2.5f, 0.1f},	{3.0, 5.5f, -9.5f},			{0.0f, 0.0f, 0.0f},		{4.0, 5.0},		"glass",		"glass",		1,	{} }, // Window 8, 2
		{ {2.0f, 2.5f, 0.1f},	{3.0, 8.5f, -9.5f},			{0.0f, 0.0f, 0.0f},		{4.0, 5.0},		"glass",		"glass",		1,	{} }, // Window 8, 3
		{ {2.0f, 2.5f, 0.1f},	{6.0, 5.5f, -9.5f},			{0.0f, 0.0f, 0.0f},		{4.0, 5.0},		"glass",		"glass",		1,	{} }, // Window 9, 2
		{ {2.0f, 2.5f, 0.1f},	{6.0, 8.5f, -9.5f},			{0.0f, 0.0f, 0.0f},		{4.0, 5.0},		"glass",		"glass",		1,	{} }, // Window 9, 3
		{ {2.0f, 2.5f, 0.1f},	{9.0, 1.75f, -9.5f},		{0.0f, 0.0f, 0.0f},		{4.0, 5.0},		"glass",		"glass",		1,	{} }, // Window 10, 1
		{ {2.0f, 2.5f, 0.1f},	{9.0, 5.5f, -9.5f},			{0.0f, 0.0f, 0.0f},		{4.0, 5.0},		"glass",		"glass",		1,	{} }, // Window 10, 2
		{ {2.0f, 2.5f, 0.1f},	{9.0, 8.5f, -9.5f},			{0.0f, 0.0f, 0.0f},		{4.0, 5.0},		"glass",		"glass",		1,	{} }, // Window 10, 3
		{ {2.0f, 2.5f, 0.1f},	{12.0, 1.75f, -9.5f},		{0.0f, 0.0f, 0.0f},		{4.0, 5.0},		"glass",		"glass",		1,	{} }, // Window 11, 1
		{ {2.0f, 2.5f, 0.1f},	{12.0, 5.5f, -9.5f},		{0.0f, 0.0f, 0.0f},		{4.0, 5.0},		"glass",		"glass",		1,	{} }, // Window 11, 2
		{ {2.0f, 2.5f, 0.1f},	{12.0, 8.5f, -9.5f},		{0.0f, 0.0f, 0.0f},		{4.0, 5.0},		"glass",		"glass",		1,	{} }, // Window 11, 3
		{ {2.0f, 2.5f, 0.1f},	{15.0, 1.75f, -9.5f},		{0.0f, 0.0f, 0.0f},		{4.0, 5.0},		"glass",		"glass",		1,	{} }, // Window 12, 1
		{ {2.0f, 2.5f, 0.1f},	{15.0, 5.5f, -9.5f},		{0.0f, 0.0f, 0.0f},		{4.0, 5.0},		"glass",		"glass",		1,	{} }, // Window 12, 2
		{ {2.0f, 2.5f, 0.1f},	{15.0, 8.5f, -9.5f},		{0.0f, 0.0f, 0.0f},		{4.0, 5.0},		"glass",		"glass",		1,	{} }, // Window 12, 3
		{ {2.0f, 2.5f, 0.1f},	{18.0, 1.75f, -9.5f},		{0.0f, 0.0f, 0.0f},		{4.0, 5.0},		"glass",		"glass",		1,	{} }, // Window 13, 1
		{ {2.0f, 2.5f, 0.1f},	{18.0, 5.5f, -9.5f},		{0.0f, 0.0f, 0.0f},		{4.0, 5.0},		"glass",		"glass",		1,	{} }, // Window 13, 2
		{ {2.0f, 2.5f, 0.1f},	{18.0, 8.5f, -9.5f},		{0.0f, 0.0f, 0.0f},		{4.0, 5.0},		"glass",		"glass",		1,	{} }, // Window 13, 3
	};

	for (const auto& mesh : meshData) {
		RenderMesh( mesh.scale, mesh.position, mesh.rotation, mesh.UVScale, mesh.texture, mesh.material, mesh.id, mesh.cylinder);
	}
}

/***********************************************************
 *  RenderMesh()
 *  Render the scene by processing every call from RenderScene using meshes from the meshData vector
 ***********************************************************/
void SceneManager::RenderMesh(
	const glm::vec3 scaleArray,
	const glm::vec3 posArray,
	const glm::vec3 rotArray,
	const glm::vec2 UVArray,
	const std::string texture,
	const std::string material,
	const int meshID,
	const glm::vec3 cylArray)
{
	glm::vec3 scaleXYZ = scaleArray;
	glm::vec3 positionXYZ = posArray;

	float XrotationDegrees = rotArray.x;
	float YrotationDegrees = rotArray.y;
	float ZrotationDegrees = rotArray.z;

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);

	SetTextureUVScale(UVArray.x, UVArray.y);
	SetShaderTexture(texture);
	SetShaderMaterial(material);

	// 0=Plane, 1=Box, 2=Cylinder, 3=Sphere, 4=Prism
	switch (meshID) {
	case 0:
		m_basicMeshes->DrawPlaneMesh();
		break;
	case 1:
		m_basicMeshes->DrawBoxMesh();
		break;
	case 2:
		m_basicMeshes->DrawCylinderMesh(cylArray.x, cylArray.y, cylArray.z);
		break;
	case 3:
		m_basicMeshes->DrawSphereMesh();
		break;
	case 4:
		m_basicMeshes->DrawPrismMesh();
		break;
	default:
		break;
	}
}