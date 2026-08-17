///////////////////////////////////////////////////////////////////////////////
// shadermanager.cpp
// ============
// manage the loading and rendering of 3D scenes
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
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
	/*** STUDENTS - add the code BELOW for defining object materials. ***/
	/*** There is no limit to the number of object materials that can ***/
	/*** be defined. Refer to the code in the OpenGL Sample for help  ***/
	OBJECT_MATERIAL metalMaterial;
	metalMaterial.ambientColor = glm::vec3(0.01f, 0.01f, 0.01f);
	metalMaterial.ambientStrength = 0.1f;
	metalMaterial.diffuseColor = glm::vec3(0.2f, 0.2f, 0.2f);
	metalMaterial.specularColor = glm::vec3(0.3f, 0.3f, 0.3f);
	metalMaterial.shininess = 75.0;
	metalMaterial.tag = "metal";
	m_objectMaterials.push_back(metalMaterial);

	OBJECT_MATERIAL asphaltMaterial;
	asphaltMaterial.ambientColor = glm::vec3(0.05f, 0.05f, 0.05f);
	asphaltMaterial.ambientStrength = 0.1f;
	asphaltMaterial.diffuseColor = glm::vec3(0.2f, 0.2f, 0.2f);
	asphaltMaterial.specularColor = glm::vec3(0.1f, 0.1f, 0.1f);
	asphaltMaterial.shininess = 0.3;
	asphaltMaterial.tag = "asphalt";
	m_objectMaterials.push_back(asphaltMaterial);

	OBJECT_MATERIAL cementMaterial;
	cementMaterial.ambientColor = glm::vec3(0.2f, 0.2f, 0.2f);
	cementMaterial.ambientStrength = 0.2f;
	cementMaterial.diffuseColor = glm::vec3(0.5f, 0.5f, 0.5f);
	cementMaterial.specularColor = glm::vec3(0.4f, 0.4f, 0.4f);
	cementMaterial.shininess = 0.1;
	cementMaterial.tag = "cement";
	m_objectMaterials.push_back(cementMaterial);

	OBJECT_MATERIAL woodMaterial;
	woodMaterial.ambientColor = glm::vec3(0.4f, 0.3f, 0.1f);
	woodMaterial.ambientStrength = 0.2f;
	woodMaterial.diffuseColor = glm::vec3(0.3f, 0.2f, 0.1f);
	woodMaterial.specularColor = glm::vec3(0.1f, 0.1f, 0.1f);
	woodMaterial.shininess = 0.3;
	woodMaterial.tag = "wood";
	m_objectMaterials.push_back(woodMaterial);

	OBJECT_MATERIAL stoneMaterial;
	stoneMaterial.ambientColor = glm::vec3(0.4f, 0.3f, 0.3f);
	stoneMaterial.ambientStrength = 0.2f;
	stoneMaterial.diffuseColor = glm::vec3(0.7f, 0.7f, 0.6f);
	stoneMaterial.specularColor = glm::vec3(0.4f, 0.5f, 0.6f);
	stoneMaterial.shininess = 10.0;
	stoneMaterial.tag = "stone";
	m_objectMaterials.push_back(stoneMaterial);

	OBJECT_MATERIAL glassMaterial;
	glassMaterial.ambientColor = glm::vec3(0.4f, 0.5f, 0.5f);
	glassMaterial.ambientStrength = 0.1f;
	glassMaterial.diffuseColor = glm::vec3(0.5f, 0.6f, 0.8f);
	glassMaterial.specularColor = glm::vec3(0.6f, 0.6f, 0.6f);
	glassMaterial.shininess = 80.0;
	glassMaterial.tag = "glass";
	m_objectMaterials.push_back(glassMaterial);

	OBJECT_MATERIAL plantMaterial;
	plantMaterial.ambientColor = glm::vec3(0.1f, 0.1f, 0.1f);
	plantMaterial.ambientStrength = 0.3f;
	plantMaterial.diffuseColor = glm::vec3(0.1f, 0.3f, 0.1f);
	plantMaterial.specularColor = glm::vec3(0.1f, 0.5f, 0.1f);
	plantMaterial.shininess = 0.3;
	plantMaterial.tag = "plant";
	m_objectMaterials.push_back(plantMaterial);
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

	/*** STUDENTS - add the code BELOW for setting up light sources ***/
	/*** Up to four light sources can be defined. Refer to the code ***/
	/*** in the OpenGL Sample for help                              ***/

	// Four light sources set in a square shape above the models to represent sunlight. Plain white light

	m_pShaderManager->setVec3Value("lightSources[0].position", 50.0f, 50.0f, 50.0f);
	m_pShaderManager->setVec3Value("lightSources[0].ambientColor", 0.01f, 0.01f, 0.01f);
	m_pShaderManager->setVec3Value("lightSources[0].diffuseColor", 0.4f, 0.4f, 0.4f);
	m_pShaderManager->setVec3Value("lightSources[0].specularColor", 1.0f, 1.0f, 1.0f);
	m_pShaderManager->setFloatValue("lightSources[0].focalStrength", 32.0f);
	m_pShaderManager->setFloatValue("lightSources[0].specularIntensity", 0.1f);

	m_pShaderManager->setVec3Value("lightSources[1].position", 50.0f, 50.0f, -50.0f);
	m_pShaderManager->setVec3Value("lightSources[1].ambientColor", 0.01f, 0.01f, 0.01f);
	m_pShaderManager->setVec3Value("lightSources[1].diffuseColor", 0.4f, 0.4f, 0.4f);
	m_pShaderManager->setVec3Value("lightSources[1].specularColor", 1.0f, 1.0f, 1.0f);
	m_pShaderManager->setFloatValue("lightSources[1].focalStrength", 32.0f);
	m_pShaderManager->setFloatValue("lightSources[1].specularIntensity", 0.1f);

	m_pShaderManager->setVec3Value("lightSources[2].position", -50.0f, 50.0f, -50.0f);
	m_pShaderManager->setVec3Value("lightSources[2].ambientColor", 0.01f, 0.01f, 0.01f);
	m_pShaderManager->setVec3Value("lightSources[2].diffuseColor", 0.4f, 0.4f, 0.4f);
	m_pShaderManager->setVec3Value("lightSources[2].specularColor", 1.0f, 1.0f, 1.0f);
	m_pShaderManager->setFloatValue("lightSources[2].focalStrength", 32.0f);
	m_pShaderManager->setFloatValue("lightSources[2].specularIntensity", 0.1f);

	m_pShaderManager->setVec3Value("lightSources[3].position", -50.0f, 50.0f, 50.0f);
	m_pShaderManager->setVec3Value("lightSources[3].ambientColor", 0.01f, 0.01f, 0.01f);
	m_pShaderManager->setVec3Value("lightSources[3].diffuseColor", 0.4f, 0.4f, 0.4f);
	m_pShaderManager->setVec3Value("lightSources[3].specularColor", 1.0f, 1.0f, 1.0f);
	m_pShaderManager->setFloatValue("lightSources[3].focalStrength", 32.0f);
	m_pShaderManager->setFloatValue("lightSources[3].specularIntensity", 0.1f);

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
	/*** STUDENTS - add the code BELOW for loading the textures that ***/
	/*** will be used for mapping to objects in the 3D scene. Up to  ***/
	/*** 16 textures can be loaded per scene. Refer to the code in   ***/
	/*** the OpenGL Sample for help.                                 ***/

	bool bReturn = false; 
	
	bReturn = CreateGLTexture(
		"../../Utilities/textures/asphalt.jpg",
		"asphalt");

	bReturn = CreateGLTexture(
		"../../Utilities/textures/concrete.jpg",
		"concrete");

	bReturn = CreateGLTexture(
		"../../Utilities/textures/grass.jpg",
		"grass");

	bReturn = CreateGLTexture(
		"../../Utilities/textures/stainless.jpg",
		"stainless");

	bReturn = CreateGLTexture(
		"../../Utilities/textures/stainless_end.jpg",
		"stainless_end");

	bReturn = CreateGLTexture(
		"../../Utilities/textures/knife_handle.jpg",
		"wood");

	bReturn = CreateGLTexture(
		"../../Utilities/textures/drywall.jpg",
		"glass");

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
	RenderGround();
	RenderPlanters();
	RenderBollards();
	RenderBuilding();
	RenderDoors();
	RenderWindows();
}

void SceneManager::RenderGround()
{
	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;
	/******************************************************************/
	/***									  					    ***/
	/***						Ground		  					    ***/
	/***									  					    ***/
	/******************************************************************/
	/***						Road		  					    ***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(30.0f, 1.0f, 5.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, 0.0f, 5.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	//SetShaderColor(0.3, 0.3, 0.3, 1);

	// scale texture to match box size
	SetTextureUVScale(30.0, 5.0);

	// apply the asphalt texture to the plane
	SetShaderTexture("asphalt");
	// apply the asphalt material to the plane
	SetShaderMaterial("asphalt");

	// draw the mesh with transformation values
	m_basicMeshes->DrawPlaneMesh();
	/******************************************************************/
	/***						Sidewalk Close  				    ***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(30.0f, 1.0f, 5.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, 0.0f, 15.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	//SetShaderColor(0.3, 0.3, 0.3, 1);

	// scale texture to match box size
	SetTextureUVScale(30.0, 5.0);

	// apply the asphalt texture to the plane
	SetShaderTexture("asphalt");
	// apply the cement material to the plane
	SetShaderMaterial("cement");

	// draw the mesh with transformation values
	m_basicMeshes->DrawPlaneMesh();
	/******************************************************************/
	/***						Sidewalk Far  					    ***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(30.0f, 1.0f, 30.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, 0.0f, -30.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	//SetShaderColor(0.3, 0.3, 0.3, 1);

	// scale texture to match box size
	SetTextureUVScale(30.0, 30.0);

	// apply the asphalt texture to the plane
	SetShaderTexture("asphalt");
	// apply the cement material to the plane
	SetShaderMaterial("cement");

	// draw the mesh with transformation values
	m_basicMeshes->DrawPlaneMesh();
}
void SceneManager::RenderPlanters()
{
	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;
	/****************************************************************/
	/***														  ***/
	/***														  ***/
	/***						Planter Boxes					  ***/
	/***														  ***/
	/***														  ***/
	/****************************************************************/
	/***														  ***/
	/***					Planter Box Concrete				  ***/
	/***														  ***/
	/****************************************************************/
	/***					Planter Box L Concrete				  ***/
	/****************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(6.0f, 1.0f, 1.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-12.0, 0.5f, -1.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.6, 0.6, 0.6, 1);

	// scale texture to match box size
	SetTextureUVScale(6.0, 1.0);

	// apply the concrete texture to the planter box
	SetShaderTexture("concrete");
	// apply the cement material to the planter box
	SetShaderMaterial("cement");

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	/***					Planter Box C Concrete				  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0, 0.5f, -1.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	/***					Planter Box R Concrete				  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(12.0, 0.5f, -1.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	/***														  ***/
	/***					Planter Box Grass Bed				  ***/
	/***														  ***/
	/****************************************************************/
	/***					Planter Box L Grass Bed				  ***/
	/****************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(6.0f, 0.05f, 1.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-12.0f, 1.025f, -1.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.2, 0.7, 0.2, 1);

	// scale texture to match box size
	SetTextureUVScale(1.0, 4.0);

	// apply the grass texture to the box
	SetShaderTexture("grass");
	// apply the plant material to the sphere
	SetShaderMaterial("plant");

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	/***					Planter Box C Grass Bed				  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, 1.025f, -1.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	/***					Planter Box R Grass Bed				  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(12.0f, 1.025f, -1.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	/***														  ***/
	/***					Planter Box Bushes					  ***/
	/***														  ***/
	/****************************************************************/
	/***					Planter Box L Bush 1	 			  ***/
	/****************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(0.5f, 0.5f, 0.5f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-14.5f, 1.0f, -1.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.2, 0.65, 0.2, 1);

	// apply the grass texture to the sphere
	SetShaderTexture("grass");
	// apply the plant material to the sphere
	SetShaderMaterial("plant");

	// draw the mesh with transformation values
	m_basicMeshes->DrawSphereMesh();
	/****************************************************************/
	/***					Planter Box L Bush 2	 			  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-13.5f, 1.0f, -1.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawSphereMesh();
	/****************************************************************/
	/***					Planter Box L Bush 3	 			  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-12.5f, 1.0f, -1.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawSphereMesh();
	/****************************************************************/
	/***					Planter Box L Bush 4	 			  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-11.5f, 1.0f, -1.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawSphereMesh();
	/****************************************************************/
	/***					Planter Box L Bush 5	 			  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-10.5f, 1.0f, -1.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawSphereMesh();
	/****************************************************************/
	/***					Planter Box L Bush 6	 			  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-9.5f, 1.0f, -1.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawSphereMesh();
	/****************************************************************/
	/***					Planter Box C Bush 1	 			  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-2.5f, 1.0f, -1.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawSphereMesh();
	/****************************************************************/
	/***					Planter Box C Bush 2	 			  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-1.5f, 1.0f, -1.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawSphereMesh();
	/****************************************************************/
	/***					Planter Box C Bush 3	 			  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-0.5f, 1.0f, -1.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawSphereMesh();
	/****************************************************************/
	/***					Planter Box C Bush 4	 			  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.5f, 1.0f, -1.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawSphereMesh();
	/****************************************************************/
	/***					Planter Box C Bush 5	 			  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(1.5f, 1.0f, -1.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawSphereMesh();
	/****************************************************************/
	/***					Planter Box C Bush 6	 			  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(2.5f, 1.0f, -1.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawSphereMesh();
	/****************************************************************/
	/***					Planter Box R Bush 1	 			  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(9.5f, 1.0f, -1.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawSphereMesh();
	/****************************************************************/
	/***					Planter Box R Bush 2	 			  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(10.5f, 1.0f, -1.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawSphereMesh();
	/****************************************************************/
	/***					Planter Box R Bush 3	 			  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(11.5f, 1.0f, -1.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawSphereMesh();
	/****************************************************************/
	/***					Planter Box R Bush 4	 			  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(12.5f, 1.0f, -1.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawSphereMesh();
	/****************************************************************/
	/***					Planter Box R Bush 5	 			  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(13.5f, 1.0f, -1.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawSphereMesh();
	/****************************************************************/
	/***					Planter Box R Bush 6	 			  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(14.5f, 1.0f, -1.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawSphereMesh();
}
void SceneManager::RenderBollards()
{
	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;
	/****************************************************************/
	/***														  ***/
	/***														  ***/
	/***						Bollards						  ***/
	/***														  ***/
	/***														  ***/
	/****************************************************************/
	/***						Bollard 1						  ***/
	/****************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(0.25f, 0.8f, 0.25f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-8.5f, 0.0f, -1.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// apply the concrete texture to the cylinder side
	SetShaderTexture("stainless");
	// scale texture to match cylinder size
	SetTextureUVScale(1.0, 1.0);
	// apply the cement material to the cylinder
	SetShaderMaterial("metal");

	// draw the cylinder sides
	m_basicMeshes->DrawCylinderMesh(false, false, true);

	// apply the concrete texture to the cylinder ends
	SetShaderTexture("stainless_end");
	// scale texture to match cylinder size
	SetTextureUVScale(1.0, 1.0);

	// draw the cylinder ends
	m_basicMeshes->DrawCylinderMesh(true, true, false);
	/****************************************************************/
	/***						Bollard 2						  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-7.5f, 0.0f, -1.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// apply the concrete texture to the cylinder side
	SetShaderTexture("stainless");

	// draw the cylinder sides
	m_basicMeshes->DrawCylinderMesh(false, false, true);

	// apply the concrete texture to the cylinder ends
	SetShaderTexture("stainless_end");

	// draw the cylinder ends
	m_basicMeshes->DrawCylinderMesh(true, true, false);
	/****************************************************************/
	/***						Bollard 3						  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-6.5f, 0.0f, -1.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// apply the concrete texture to the cylinder side
	SetShaderTexture("stainless");

	// draw the cylinder sides
	m_basicMeshes->DrawCylinderMesh(false, false, true);

	// apply the concrete texture to the cylinder ends
	SetShaderTexture("stainless_end");

	// draw the cylinder ends
	m_basicMeshes->DrawCylinderMesh(true, true, false);
	/****************************************************************/
	/***						Bollard 4						  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-5.5f, 0.0f, -1.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// apply the concrete texture to the cylinder side
	SetShaderTexture("stainless");

	// draw the cylinder sides
	m_basicMeshes->DrawCylinderMesh(false, false, true);

	// apply the concrete texture to the cylinder ends
	SetShaderTexture("stainless_end");

	// draw the cylinder ends
	m_basicMeshes->DrawCylinderMesh(true, true, false);
	/****************************************************************/
	/***						Bollard 5						  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-4.5f, 0.0f, -1.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// apply the concrete texture to the cylinder side
	SetShaderTexture("stainless");

	// draw the cylinder sides
	m_basicMeshes->DrawCylinderMesh(false, false, true);

	// apply the concrete texture to the cylinder ends
	SetShaderTexture("stainless_end");

	// draw the cylinder ends
	m_basicMeshes->DrawCylinderMesh(true, true, false);
	/****************************************************************/
	/***						Bollard 6						  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-3.5f, 0.0f, -1.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// apply the concrete texture to the cylinder side
	SetShaderTexture("stainless");

	// draw the cylinder sides
	m_basicMeshes->DrawCylinderMesh(false, false, true);

	// apply the concrete texture to the cylinder ends
	SetShaderTexture("stainless_end");

	// draw the cylinder ends
	m_basicMeshes->DrawCylinderMesh(true, true, false);
	/****************************************************************/
	/***						Bollard 7						  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(3.5f, 0.0f, -1.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// apply the concrete texture to the cylinder side
	SetShaderTexture("stainless");

	// draw the cylinder sides
	m_basicMeshes->DrawCylinderMesh(false, false, true);

	// apply the concrete texture to the cylinder ends
	SetShaderTexture("stainless_end");

	// draw the cylinder ends
	m_basicMeshes->DrawCylinderMesh(true, true, false);
	/****************************************************************/
	/***						Bollard 8						  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(4.5f, 0.0f, -1.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// apply the concrete texture to the cylinder side
	SetShaderTexture("stainless");

	// draw the cylinder sides
	m_basicMeshes->DrawCylinderMesh(false, false, true);

	// apply the concrete texture to the cylinder ends
	SetShaderTexture("stainless_end");

	// draw the cylinder ends
	m_basicMeshes->DrawCylinderMesh(true, true, false);
	/****************************************************************/
	/***						Bollard 9						  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(5.5f, 0.0f, -1.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// apply the concrete texture to the cylinder side
	SetShaderTexture("stainless");

	// draw the cylinder sides
	m_basicMeshes->DrawCylinderMesh(false, false, true);

	// apply the concrete texture to the cylinder ends
	SetShaderTexture("stainless_end");

	// draw the cylinder ends
	m_basicMeshes->DrawCylinderMesh(true, true, false);
	/****************************************************************/
	/***						Bollard 10						  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(6.5f, 0.0f, -1.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// apply the concrete texture to the cylinder side
	SetShaderTexture("stainless");

	// draw the cylinder sides
	m_basicMeshes->DrawCylinderMesh(false, false, true);

	// apply the concrete texture to the cylinder ends
	SetShaderTexture("stainless_end");

	// draw the cylinder ends
	m_basicMeshes->DrawCylinderMesh(true, true, false);
	/****************************************************************/
	/***						Bollard 11						  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(7.5f, 0.0f, -1.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// apply the concrete texture to the cylinder side
	SetShaderTexture("stainless");

	// draw the cylinder sides
	m_basicMeshes->DrawCylinderMesh(false, false, true);

	// apply the concrete texture to the cylinder ends
	SetShaderTexture("stainless_end");

	// draw the cylinder ends
	m_basicMeshes->DrawCylinderMesh(true, true, false);
	/****************************************************************/
	/***						Bollard 12						  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(8.5f, 0.0f, -1.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// apply the concrete texture to the cylinder side
	SetShaderTexture("stainless");

	// draw the cylinder sides
	m_basicMeshes->DrawCylinderMesh(false, false, true);

	// apply the concrete texture to the cylinder ends
	SetShaderTexture("stainless_end");

	// draw the cylinder ends
	m_basicMeshes->DrawCylinderMesh(true, true, false);
}
void SceneManager::RenderBuilding()
{
	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;
	/****************************************************************/
	/***														  ***/
	/***						Main Building					  ***/
	/***														  ***/
	/****************************************************************/
	/***						Main Body						  ***/
	/****************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(40.0f, 20.0f, 1.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0, 10.0f, -10.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// scale texture to match box size
	SetTextureUVScale(40.0, 30.0);

	// apply the concrete texture to the planter box
	SetShaderTexture("concrete");
	// apply the cement material to the planter box
	SetShaderMaterial("stone");

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	/***														  ***/
	/***						Pillar Roof						  ***/
	/***														  ***/
	/****************************************************************/
	/***					Pillar Roof Bottom					  ***/
	/****************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(17.0f, 3.0f, 3.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0, 12.0f, -8.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// scale texture to match box size
	SetTextureUVScale(17.0, 3.0);

	// apply the concrete texture to the building
	SetShaderTexture("concrete");
	// apply the cement material to the building
	SetShaderMaterial("stone");

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	/***					Pillar Roof Top						  ***/
	/****************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(18.0f, 4.0f, 3.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = -90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 180.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0, 15.0f, -8.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// scale texture to match box size
	SetTextureUVScale(18.0, 3.0);

	// draw the mesh with transformation values
	m_basicMeshes->DrawPrismMesh();
	/****************************************************************/
	/***														  ***/
	/***						Pillars							  ***/
	/***														  ***/
	/****************************************************************/
	/***						Pillar 1					   	  ***/
	/****************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(0.7f, 12.0f, 0.7f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-7.5, 0.0f, -7.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// scale texture to match box size
	SetTextureUVScale(1.0, 12.0);

	// draw the mesh with transformation values
	m_basicMeshes->DrawCylinderMesh();
	/****************************************************************/
	/***						Pillar 2					   	  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-4.5, 0.0f, -7.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawCylinderMesh();
	/****************************************************************/
	/***						Pillar 3					   	  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-1.5, 0.0f, -7.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawCylinderMesh();
	/****************************************************************/
	/***						Pillar 4					   	  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(1.5, 0.0f, -7.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawCylinderMesh();
	/****************************************************************/
	/***						Pillar 5					   	  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(4.5, 0.0f, -7.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawCylinderMesh();
	/****************************************************************/
	/***						Pillar 6					   	  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(7.5, 0.0f, -7.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawCylinderMesh();
}
void SceneManager::RenderDoors()
{
	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;
	/****************************************************************/
	/***														  ***/
	/***						Doors							  ***/
	/***														  ***/
	/****************************************************************/
	/***						Door 1						  	  ***/
	/****************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(2.0f, 3.0f, 0.1f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-6.0, 1.5f, -9.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// apply the wood texture to the door
	SetShaderTexture("wood");
	// scale texture to match box size
	SetTextureUVScale(4.0, 6.0);
	// apply the wood material to the door
	SetShaderMaterial("wood");

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	/***						Door 2						  	  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-3.0, 1.5f, -9.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	/***						Door 3						  	  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0, 1.5f, -9.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	/***						Door 4						  	  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(3.0, 1.5f, -9.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	/***						Door 5						  	  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(6.0, 1.5f, -9.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
}

void SceneManager::RenderWindows()
{
	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;
	/****************************************************************/
	/***														  ***/
	/***						Windows							  ***/
	/***														  ***/
	/****************************************************************/
	/***						Window 1,1					  	  ***/
	/****************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(2.0f, 2.5f, 0.1f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-18.0, 1.75f, -9.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// apply the glass texture to the window
	SetShaderTexture("glass");
	// scale texture to match box size
	SetTextureUVScale(4.0, 5.0);
	// apply the glass material to the window
	SetShaderMaterial("glass");

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	/***						Window 1,2					  	  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-18.0, 5.5f, -9.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	/***						Window 1,3					  	  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-18.0, 8.5f, -9.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	/***						Window 2,1					  	  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-15.0, 1.75f, -9.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	/***						Window 2,2					  	  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-15.0, 5.5f, -9.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	/***						Window 2,3					  	  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-15.0, 8.5f, -9.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	/***						Window 3,1					  	  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-12.0, 1.75f, -9.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	/***						Window 3,2					  	  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-12.0, 5.5f, -9.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	/***						Window 3,3					  	  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-12.0, 8.5f, -9.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	/***						Window 4,1					  	  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-9.0, 1.75f, -9.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	/***						Window 4,2					  	  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-9.0, 5.5f, -9.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	/***						Window 4,3					  	  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-9.0, 8.5f, -9.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	/***						Window 5,2					  	  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-6.0, 5.5f, -9.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	/***						Window 5,3					  	  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-6.0, 8.5f, -9.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	/***						Window 6,2					  	  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-3.0, 5.5f, -9.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	/***						Window 6,3					  	  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-3.0, 8.5f, -9.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	/***						Window 7,2					  	  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0, 5.5f, -9.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	/***						Window 7,3					  	  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0, 8.5f, -9.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	/***						Window 8,2					  	  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(3.0, 5.5f, -9.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	/***						Window 8,3					  	  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(3.0, 8.5f, -9.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	/***						Window 9,2					  	  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(6.0, 5.5f, -9.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	/***						Window 9,3					  	  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(6.0, 8.5f, -9.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	/***						Window 10,1					  	  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(9.0, 1.75f, -9.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	/***						Window 10,2					  	  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(9.0, 5.5f, -9.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	/***						Window 10,3					  	  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(9.0, 8.5f, -9.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	/***						Window 11,1					  	  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(12.0, 1.75f, -9.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	/***						Window 11,2					  	  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(12.0, 5.5f, -9.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	/***						Window 11,3					  	  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(12.0, 8.5f, -9.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	/***						Window 12,1					  	  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(15.0, 1.75f, -9.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	/***						Window 12,2					  	  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(15.0, 5.5f, -9.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	/***						Window 12,3					  	  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(15.0, 8.5f, -9.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	/***						Window 13,1					  	  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(18.0, 1.75f, -9.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	/***						Window 13,2					  	  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(18.0, 5.5f, -9.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
	/***						Window 13,3					  	  ***/
	/****************************************************************/
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(18.0, 8.5f, -9.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
}
