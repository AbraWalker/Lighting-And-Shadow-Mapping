//--------------------------------------------------------------------------------------
// Scene geometry and layout preparation
// Scene rendering & update
//--------------------------------------------------------------------------------------

#include "Scene.h"
#include "Mesh.h"
#include "Model.h"
#include "Camera.h"
#include "State.h"
#include "Shader.h"
#include "Input.h"
#include "Common.h"

#include "CVector2.h" 
#include "CVector3.h" 
#include "CMatrix4x4.h"
#include "MathHelpers.h"     // Helper functions for maths
#include "GraphicsHelpers.h" // Helper functions to unclutter the code here

#include "ColourRGBA.h" 

#include <sstream>
#include <memory>


//--------------------------------------------------------------------------------------
// Scene Data
//--------------------------------------------------------------------------------------
// Addition of Mesh, Model and Camera classes have greatly simplified this section
// Geometry data has gone to Mesh class. Positions, rotations, matrices have gone to Model and Camera classes

// Constants controlling speed of movement/rotation (measured in units per second because we're using frame time)
const float ROTATION_SPEED = 2.0f;  // 2 radians per second for rotation
const float MOVEMENT_SPEED = 50.0f; // 50 units per second for movement (what a unit of length is depends on 3D model - i.e. an artist decision usually)


// Meshes, models and cameras, same meaning as TL-Engine. Meshes prepared in InitGeometry function, Models & camera in InitScene
Mesh* gCharacterMesh;
Mesh* gCrateMesh;
Mesh* gGroundMesh;
Mesh* gSphereMesh;
Mesh* gParaSphereMesh;
Mesh* gCubeMesh;
Mesh* gNormalCubeMesh;
Mesh* gCloudCubeMesh;
Mesh* gGolbezCubeMesh;
Mesh* gTrollMesh;
Mesh* gLightMesh;
Mesh* gLightMesh2;

Model* gCharacter;
Model* gCrate;
Model* gGround;
Model* gSphere;
Model* gParaSphere;
Model* gCube;
Model* gNormalCube;
Model* gCloudCube;
Model* gGolbezCube;
Model* gTroll;

Camera* gCamera;


// Store lights in an array in this exercise
const int NUM_LIGHTS = 2;
struct Light
{
    Model*   model;
    CVector3 colour;
    float    strength;
};
Light gLights[NUM_LIGHTS]; 


// Additional light information
CVector3 gAmbientColour = { 0.8f, 0.7f, 0.75f }; // Background level of light (slightly bluish to match the far background, which is dark blue)
float    gSpecularPower = 256; // Specular power controls shininess - same for all models in this app

ColourRGBA gBackgroundColor = { 0.2f, 0.2f, 0.3f, 1.0f };

// Variables controlling light1's orbiting of the cube
const float gLightOrbit = 20.0f;
const float gLightOrbitSpeed = 0.7f;

// Spotlight data - using spotlights in this lab because shadow mapping needs to treat each light as a camera, which is easy with spotlights
float gSpotlightConeAngle = 90.0f; // Spot light cone angle (degrees), like the FOV (field-of-view) of the spot light

								   // Cell shading data
CVector3 OutlineColour = { 0, 50, 0 };
float    OutlineThickness = 0.015f;

//--------------------------------------------------------------------------------------
//**** Shadow Texture  ****//
//--------------------------------------------------------------------------------------
// This texture will have the scene from the point of view of the light renderered on it. This texture is then used for shadow mapping

// Dimensions of shadow map texture - controls quality of shadows
int gShadowMapSize  = 1024;

// The shadow texture - effectively a depth buffer of the scene **from the light's point of view**
//                      Each frame it is rendered to, then the texture is used to help the per-pixel lighting shader identify pixels in shadow
ID3D11Texture2D*          gShadowMap1Texture      = nullptr; // This object represents the memory used by the texture on the GPU
ID3D11DepthStencilView*   gShadowMap1DepthStencil = nullptr; // This object is used when we want to render to the texture above **as a depth buffer**
ID3D11ShaderResourceView* gShadowMap1SRV          = nullptr; // This object is used to give shaders access to the texture above (SRV = shader resource view)

ID3D11Texture2D*          gShadowMap2Texture = nullptr;
ID3D11DepthStencilView*   gShadowMap2DepthStencil = nullptr;
ID3D11ShaderResourceView* gShadowMap2SRV = nullptr;

//*********************//



//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------
// Variables sent over to the GPU each frame
// The structures are now in Common.h
// IMPORTANT: Any new data you add in C++ code (CPU-side) is not automatically available to the GPU
//            Anything the shaders need (per-frame or per-model) needs to be sent via a constant buffer

PerFrameConstants gPerFrameConstants;      // The constants that need to be sent to the GPU each frame (see common.h for structure)
ID3D11Buffer*     gPerFrameConstantBuffer; // The GPU buffer that will recieve the constants above

PerModelConstants gPerModelConstants;      // As above, but constant that change per-model (e.g. world matrix)
ID3D11Buffer*     gPerModelConstantBuffer; // --"--



//--------------------------------------------------------------------------------------
// Textures
//--------------------------------------------------------------------------------------

// DirectX objects controlling textures used in this lab
ID3D11Resource*           gCharacterDiffuseSpecularMap    = nullptr; // This object represents the memory used by the texture on the GPU
ID3D11ShaderResourceView* gCharacterDiffuseSpecularMapSRV = nullptr; // This object is used to give shaders access to the texture above (SRV = shader resource view)

ID3D11Resource*           gCrateDiffuseSpecularMap    = nullptr;
ID3D11ShaderResourceView* gCrateDiffuseSpecularMapSRV = nullptr;

ID3D11Resource*           gSphereDiffuseSpecularMap = nullptr;
ID3D11ShaderResourceView* gSphereDiffuseSpecularMapSRV = nullptr;

ID3D11Resource*           gParaSphereDiffuseSpecularMap = nullptr;
ID3D11ShaderResourceView* gParaSphereDiffuseSpecularMapSRV = nullptr;

ID3D11Resource*           gParaSphereMap = nullptr;
ID3D11ShaderResourceView* gParaSphereMapSRV = nullptr;

ID3D11Resource*           gCubeDiffuseSpecularMap = nullptr;
ID3D11ShaderResourceView* gCubeDiffuseSpecularMapSRV = nullptr;

ID3D11Resource*           gCubeDiffuseSpecularMap2 = nullptr;
ID3D11ShaderResourceView* gCubeDiffuseSpecularMapSRV2 = nullptr;

ID3D11Resource*           gNormalCubeDiffuseSpecularMap = nullptr;
ID3D11ShaderResourceView* gNormalCubeDiffuseSpecularMapSRV = nullptr;

ID3D11Resource*           gCloudCubeDiffuseSpecularMap = nullptr;
ID3D11ShaderResourceView* gCloudCubeDiffuseSpecularMapSRV = nullptr;

ID3D11Resource*           gGolbezCubeDiffuseSpecularMap = nullptr;
ID3D11ShaderResourceView* gGolbezCubeDiffuseSpecularMapSRV = nullptr;

ID3D11Resource*           gTrollDiffuseMap = nullptr;
ID3D11ShaderResourceView* gTrollDiffuseMapSRV = nullptr;

ID3D11Resource*           gCellMap = nullptr;
ID3D11ShaderResourceView* gCellMapSRV = nullptr;

ID3D11Resource*           gNormalCubeMap = nullptr;
ID3D11ShaderResourceView* gNormalCubeMapSRV = nullptr;

ID3D11Resource*           gGroundDiffuseSpecularMap    = nullptr;
ID3D11ShaderResourceView* gGroundDiffuseSpecularMapSRV = nullptr;

ID3D11Resource*           gLightDiffuseMap    = nullptr;
ID3D11ShaderResourceView* gLightDiffuseMapSRV = nullptr;

float gParallaxDepth = 0.2f;
bool gUseParallax = true;

//--------------------------------------------------------------------------------------
// Light Helper Functions
//--------------------------------------------------------------------------------------

// Get "camera-like" view matrix for a spotlight
CMatrix4x4 CalculateLightViewMatrix(int lightIndex)
{
    return InverseAffine(gLights[lightIndex].model->WorldMatrix());
}

// Get "camera-like" projection matrix for a spotlight
CMatrix4x4 CalculateLightProjectionMatrix(int lightIndex)
{
    return MakeProjectionMatrix(1.0f, ToRadians(gSpotlightConeAngle)); // Helper function in Utility\GraphicsHelpers.cpp
}


//--------------------------------------------------------------------------------------
// Initialise scene geometry, constant buffers and states
//--------------------------------------------------------------------------------------

// Prepare the geometry required for the scene
// Returns true on success
bool InitGeometry()
{
    // Load mesh geometry data, just like TL-Engine this doesn't create anything in the scene. Create a Model for that.
    // IMPORTANT NOTE: Will only keep the first object from the mesh - multipart objects will have parts missing - see later lab for more robust loader
    try 
    {
        gCharacterMesh = new Mesh("teapot.x");
        gCrateMesh     = new Mesh("CargoContainer.x");
        gGroundMesh    = new Mesh("Ground.x");
		gSphereMesh	   = new Mesh("Sphere.x");
		gParaSphereMesh= new Mesh("Cube.x", true);
		gCubeMesh	   = new Mesh("Cube.x");
		gNormalCubeMesh= new Mesh("Cube.x", true);
		gCloudCubeMesh = new Mesh("Cube.x");
		gGolbezCubeMesh = new Mesh("Cube.x");
		gTrollMesh     = new Mesh("Troll.x");
        gLightMesh     = new Mesh("Light.x");
		gLightMesh2    = new Mesh("Light.x");
    }
    catch (std::runtime_error e)  // Constructors cannot return error messages so use exceptions to catch mesh errors (fairly standard approach this)
    {
        gLastError = e.what(); // This picks up the error message put in the exception (see Mesh.cpp)
        return false;
    }


    // Load the shaders required for the geometry we will use (see Shader.cpp / .h)
    if (!LoadShaders())
    {
        gLastError = "Error loading shaders";
        return false;
    }


    // Create GPU-side constant buffers to receive the gPerFrameConstants and gPerModelConstants structures above
    // These allow us to pass data from CPU to shaders such as lighting information or matrices
    // See the comments above where these variable are declared and also the UpdateScene function
    gPerFrameConstantBuffer = CreateConstantBuffer(sizeof(gPerFrameConstants));
    gPerModelConstantBuffer = CreateConstantBuffer(sizeof(gPerModelConstants));
    if (gPerFrameConstantBuffer == nullptr || gPerModelConstantBuffer == nullptr)
    {
        gLastError = "Error creating constant buffers";
        return false;
    }


    //// Load / prepare textures on the GPU ////

    // Load textures and create DirectX objects for them
    // The LoadTexture function requires you to pass a ID3D11Resource* (e.g. &gCubeDiffuseMap), which manages the GPU memory for the
    // texture and also a ID3D11ShaderResourceView* (e.g. &gCubeDiffuseMapSRV), which allows us to use the texture in shaders
    // The function will fill in these pointers with usable data. The variables used here are globals found near the top of the file.
    if (!LoadTexture("MetalDiffuseSpecular.dds", &gCharacterDiffuseSpecularMap, &gCharacterDiffuseSpecularMapSRV) ||
        !LoadTexture("CargoA.dds",               &gCrateDiffuseSpecularMap,     &gCrateDiffuseSpecularMapSRV    ) ||
        !LoadTexture("CobbleDiffuseSpecular.dds", &gGroundDiffuseSpecularMap,    &gGroundDiffuseSpecularMapSRV   )||
		!LoadTexture("BrainDiffuseSpecular.dds", &gSphereDiffuseSpecularMap,    &gSphereDiffuseSpecularMapSRV   ) ||
		!LoadTexture("PatternDiffuseSpecular.dds", &gParaSphereDiffuseSpecularMap, &gParaSphereDiffuseSpecularMapSRV) ||
		!LoadTexture("PatternNormalHeight.dds", &gParaSphereMap, &gParaSphereMapSRV)								  ||
		!LoadTexture("StoneDiffuseSpecular.dds", &gCubeDiffuseSpecularMap,		&gCubeDiffuseSpecularMapSRV     ) ||
		!LoadTexture("WoodDiffuseSpecular.dds",  &gCubeDiffuseSpecularMap2,     &gCubeDiffuseSpecularMapSRV2)     ||
		!LoadTexture("PatternDiffuseSpecular.dds", &gNormalCubeDiffuseSpecularMap,    &gNormalCubeDiffuseSpecularMapSRV)      ||
		!LoadTexture("PatternNormal.dds", &gNormalCubeMap, &gNormalCubeMapSRV) ||
		!LoadTexture("Cloud.png", &gCloudCubeDiffuseSpecularMap, &gCloudCubeDiffuseSpecularMapSRV) ||
		!LoadTexture("Golbez.png", &gGolbezCubeDiffuseSpecularMap, &gGolbezCubeDiffuseSpecularMapSRV) ||
		!LoadTexture("Red.png", &gTrollDiffuseMap, &gTrollDiffuseMapSRV) ||
		!LoadTexture("CellGradient.png", &gCellMap, &gCellMapSRV) ||
        !LoadTexture("Flare.jpg",                &gLightDiffuseMap,             &gLightDiffuseMapSRV))
    {
        gLastError = "Error loading textures";
        return false;
    }



	//**** Create Shadow Map texture ****//

	// We also need a depth buffer to go with our portal
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width  = gShadowMapSize; // Size of the shadow map determines quality / resolution of shadows
	textureDesc.Height = gShadowMapSize;
	textureDesc.MipLevels = 1; // 1 level, means just the main texture, no additional mip-maps. Usually don't use mip-maps when rendering to textures (or we would have to render every level)
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32_TYPELESS; // The shadow map contains a single 32-bit value [tech gotcha: have to say typeless because depth buffer and shaders see things slightly differently]
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D10_BIND_DEPTH_STENCIL | D3D10_BIND_SHADER_RESOURCE; // Indicate we will use texture as a depth buffer and also pass it to shaders
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	if (FAILED(gD3DDevice->CreateTexture2D(&textureDesc, NULL, &gShadowMap1Texture) ))
	{
		gLastError = "Error creating shadow map texture";
		return false;
	}
	if (FAILED(gD3DDevice->CreateTexture2D(&textureDesc, NULL, &gShadowMap2Texture)))
	{
		gLastError = "Error creating shadow map texture";
		return false;
	}

	// Create the depth stencil view, i.e. indicate that the texture just created is to be used as a depth buffer
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT; // See "tech gotcha" above. The depth buffer sees each pixel as a "depth" float
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
    dsvDesc.Flags = 0;
	if (FAILED(gD3DDevice->CreateDepthStencilView(gShadowMap1Texture, &dsvDesc, &gShadowMap1DepthStencil) ))
	{
		gLastError = "Error creating shadow map depth stencil view";
		return false;
	}
	if (FAILED(gD3DDevice->CreateDepthStencilView(gShadowMap2Texture, &dsvDesc, &gShadowMap2DepthStencil)))
	{
		gLastError = "Error creating shadow map depth stencil view";
		return false;
	}

   
 	// We also need to send this texture (resource) to the shaders. To do that we must create a shader-resource "view"
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT; // See "tech gotcha" above. The shaders see textures as colours, so shadow map pixels are not seen as depths
                                           // but rather as "red" floats (one float taken from RGB). Although the shader code will use the value as a depth
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	if (FAILED(gD3DDevice->CreateShaderResourceView(gShadowMap1Texture, &srvDesc, &gShadowMap1SRV) ))
	{
		gLastError = "Error creating shadow map shader resource view";
		return false;
	}
	if (FAILED(gD3DDevice->CreateShaderResourceView(gShadowMap2Texture, &srvDesc, &gShadowMap2SRV)))
	{
		gLastError = "Error creating shadow map shader resource view";
		return false;
	}


   //*****************************//


  	// Create all filtering modes, blending modes etc. used by the app (see State.cpp/.h)
	if (!CreateStates())
	{
		gLastError = "Error creating states";
		return false;
	}

	return true;
}


// Prepare the scene
// Returns true on success
bool InitScene()
{
    //// Set up scene ////

    gCharacter = new Model(gCharacterMesh);
    gCrate     = new Model(gCrateMesh);
    gGround    = new Model(gGroundMesh);
	gSphere = new Model(gSphereMesh);
	gParaSphere = new Model(gParaSphereMesh);
	gCube = new Model(gCubeMesh);
	gNormalCube = new Model(gNormalCubeMesh);
	gCloudCube = new Model(gCloudCubeMesh);
	gGolbezCube = new Model(gGolbezCubeMesh);
	gTroll = new Model(gTrollMesh);

	// Initial positions
	gCharacter->SetPosition({ 15, 0, 0 });
    gCharacter->SetScale(1);
    gCharacter->SetRotation({ 0, ToRadians(215.0f), 0 });
	gCrate-> SetPosition({ 40, 0, 30 });
	gCrate-> SetScale(6);
	gCrate-> SetRotation({ 0.0f, ToRadians(-20.0f), 0.0f });
	gSphere->SetPosition({-20, 10, 0});
	gSphere->SetScale(0.5);
	gParaSphere->SetPosition({ -20, 20, 0 });
	gParaSphere->SetScale(0.5);
	gCube->SetPosition({ 40, 5, -10 });
	gCube->SetScale(0.75);
	gNormalCube->SetPosition({ 25, 20, -20 });
	gNormalCube->SetScale(0.5);
	gCloudCube->SetPosition({ 40, 30, 30 });
	gGolbezCube->SetPosition({ 40, 30, 40 });
	gTroll->SetPosition({ -20, 0, 10 });


    // Light set-up - using an array this time
    for (int i = 0; i < NUM_LIGHTS; ++i)
    {
        gLights[i].model = new Model(gLightMesh);
		//gLights[i].model = new Model(gLightMesh2);
    }

    gLights[0].colour = { 0.8f, 0.8f, 1.0f };
    gLights[0].strength = 50;
    gLights[0].model->SetPosition({ 30, 20, 0 });
    gLights[0].model->SetScale(pow(gLights[0].strength, 0.7f)); // Convert light strength into a nice value for the scale of the light - equation is ad-hoc.
	gLights[0].model->FaceTarget(gCharacter->Position());

    gLights[1].colour = { 1.0f, 0.8f, 0.2f };
    gLights[1].strength = 40;
    gLights[1].model->SetPosition({ -20, 30, 20 });
    gLights[1].model->SetScale(pow(gLights[1].strength, 0.7f));
	gLights[1].model->FaceTarget({ 0, 0, 0 });


    //// Set up camera ////

    gCamera = new Camera();
    gCamera->SetPosition({ 15, 30,-70 });
    gCamera->SetRotation({ ToRadians(13), 0, 0 });

    return true;
}


// Release the geometry and scene resources created above
void ReleaseResources()
{
    ReleaseStates();

    if (gShadowMap1DepthStencil)  gShadowMap1DepthStencil->Release();
    if (gShadowMap1SRV)           gShadowMap1SRV->Release();
    if (gShadowMap1Texture)       gShadowMap1Texture->Release();

	if (gShadowMap2DepthStencil)  gShadowMap2DepthStencil->Release();
	if (gShadowMap2SRV)           gShadowMap2SRV->Release();
	if (gShadowMap2Texture)       gShadowMap2Texture->Release();

    if (gLightDiffuseMapSRV)             gLightDiffuseMapSRV->Release();
    if (gLightDiffuseMap)                gLightDiffuseMap->Release();
    if (gGroundDiffuseSpecularMapSRV)    gGroundDiffuseSpecularMapSRV->Release();
    if (gGroundDiffuseSpecularMap)       gGroundDiffuseSpecularMap->Release();
    if (gCrateDiffuseSpecularMapSRV)     gCrateDiffuseSpecularMapSRV->Release();
    if (gCrateDiffuseSpecularMap)        gCrateDiffuseSpecularMap->Release();
    if (gCharacterDiffuseSpecularMapSRV) gCharacterDiffuseSpecularMapSRV->Release();
    if (gCharacterDiffuseSpecularMap)    gCharacterDiffuseSpecularMap->Release();

    if (gPerModelConstantBuffer)  gPerModelConstantBuffer->Release();
    if (gPerFrameConstantBuffer)  gPerFrameConstantBuffer->Release();

    ReleaseShaders();

    // See note in InitGeometry about why we're not using unique_ptr and having to manually delete
    for (int i = 0; i < NUM_LIGHTS; ++i)
    {
        delete gLights[i].model;  gLights[i].model = nullptr;
    }
    delete gCamera;    gCamera    = nullptr;
    delete gGround;    gGround    = nullptr;
    delete gCrate;     gCrate     = nullptr;
    delete gCharacter; gCharacter = nullptr;
	delete gSphere; gSphere = nullptr;
	delete gCube; gCube = nullptr;
	delete gNormalCube; gNormalCube = nullptr;
	delete gLightMesh;     gLightMesh     = nullptr;
	delete gLightMesh2;	   gLightMesh2    = nullptr;
    delete gGroundMesh;    gGroundMesh    = nullptr;
    delete gCrateMesh;     gCrateMesh     = nullptr;
    delete gCharacterMesh; gCharacterMesh = nullptr;
}



//--------------------------------------------------------------------------------------
// Scene Rendering
//--------------------------------------------------------------------------------------

// Render the scene from the given light's point of view. Only renders depth buffer
void RenderDepthBufferFromLight(int lightIndex)
{
    // Get camera-like matrices from the spotlight, seet in the constant buffer and send over to GPU
    gPerFrameConstants.viewMatrix           = CalculateLightViewMatrix(lightIndex);
    gPerFrameConstants.projectionMatrix     = CalculateLightProjectionMatrix(lightIndex);
    gPerFrameConstants.viewProjectionMatrix = gPerFrameConstants.viewMatrix * gPerFrameConstants.projectionMatrix;
    UpdateConstantBuffer(gPerFrameConstantBuffer, gPerFrameConstants);

    // Indicate that the constant buffer we just updated is for use in the vertex shader (VS) and pixel shader (PS)
    gD3DContext->VSSetConstantBuffers(0, 1, &gPerFrameConstantBuffer); // First parameter must match constant buffer number in the shader 
    gD3DContext->PSSetConstantBuffers(0, 1, &gPerFrameConstantBuffer);


    //// Only render models that cast shadows ////

    // Use special depth-only rendering shaders
    gD3DContext->VSSetShader(gBasicTransformVertexShader, nullptr, 0);
    gD3DContext->PSSetShader(gDepthOnlyPixelShader,       nullptr, 0);
    
    // States - no blending, normal depth buffer and culling
    gD3DContext->OMSetBlendState(gNoBlendingState, nullptr, 0xffffff);
    gD3DContext->OMSetDepthStencilState(gUseDepthBufferState, 0);
    gD3DContext->RSSetState(gCullFrontState);

    // Render models - no state changes required between each object in this situation (no textures used in this step)
    gGround->Render();
    gCharacter->Render();
    gCrate->Render();
	gNormalCube->Render();
	gParaSphere->Render();
	gTroll->Render();
	gCloudCube->Render();
	gGolbezCube->Render();


	gD3DContext->VSSetShader(gSphereVertexShader, nullptr, 0);
	gD3DContext->PSSetShader(gSpherePixelShader, nullptr, 0);

	gSphere->Render();

	gD3DContext->VSSetShader(gCubeVertexShader, nullptr, 0);
	gD3DContext->PSSetShader(gCubePixelShader, nullptr, 0);

	gCube->Render();
	
}



// Render everything in the scene from the given camera
// This code is common between rendering the main scene and rendering the scene in the portal
// See RenderScene function below
void RenderSceneFromCamera(Camera* camera)
{
    // Set camera matrices in the constant buffer and send over to GPU
    gPerFrameConstants.viewMatrix           = camera->ViewMatrix();
    gPerFrameConstants.projectionMatrix     = camera->ProjectionMatrix();
    gPerFrameConstants.viewProjectionMatrix = camera->ViewProjectionMatrix();
	gPerFrameConstants.outlineColour = OutlineColour;
	gPerFrameConstants.outlineThickness = OutlineThickness;
	gPerFrameConstants.light3Colour = CVector3{ 0.0f, 1.0f, 0.5f };
	gPerFrameConstants.light3Direction = CVector3{ -0.5, -1, 0 };

    UpdateConstantBuffer(gPerFrameConstantBuffer, gPerFrameConstants);

    // Indicate that the constant buffer we just updated is for use in the vertex shader (VS) and pixel shader (PS)
    gD3DContext->VSSetConstantBuffers(0, 1, &gPerFrameConstantBuffer); // First parameter must match constant buffer number in the shader 
    gD3DContext->PSSetConstantBuffers(0, 1, &gPerFrameConstantBuffer);


    //// Render lit models ////

    // Select which shaders to use next
    gD3DContext->VSSetShader(gPixelLightingVertexShader, nullptr, 0);
    gD3DContext->PSSetShader(gPixelLightingPixelShader,  nullptr, 0);
    
    // States - no blending, normal depth buffer and culling
    gD3DContext->OMSetBlendState(gNoBlendingState, nullptr, 0xffffff);
    gD3DContext->OMSetDepthStencilState(gUseDepthBufferState, 0);
    gD3DContext->RSSetState(gCullBackState);

    // Select the approriate textures and sampler to use in the pixel shader
    gD3DContext->PSSetShaderResources(0, 1, &gGroundDiffuseSpecularMapSRV); // First parameter must match texture slot number in the shader
    gD3DContext->PSSetSamplers(0, 1, &gAnisotropic4xSampler);

    // Render model - it will update the model's world matrix and send it to the GPU in a constant buffer, then it will call
    // the Mesh render function, which will set up vertex & index buffer before finally calling Draw on the GPU
    gGround->Render();

    // Render other lit models, only change textures for each onee
    gD3DContext->PSSetShaderResources(0, 1, &gCharacterDiffuseSpecularMapSRV); 
    gCharacter->Render();

    gD3DContext->PSSetShaderResources(0, 1, &gCrateDiffuseSpecularMapSRV);
    gCrate->Render();

	gD3DContext->PSSetShader(gCubePixelShader, nullptr, 0);
	gD3DContext->PSSetShaderResources(0, 1, &gCubeDiffuseSpecularMapSRV);
	gD3DContext->PSSetShaderResources(1, 1, &gCubeDiffuseSpecularMapSRV2);
	gCube->Render();

	gD3DContext->VSSetShader(gSphereVertexShader, nullptr, 0);
	gD3DContext->PSSetShader(gSpherePixelShader, nullptr, 0);

	gD3DContext->PSSetShaderResources(0, 1, &gSphereDiffuseSpecularMapSRV);
	gSphere->Render();

	gD3DContext->VSSetShader(gParallaxMappingVertexShader, nullptr, 0);
	gD3DContext->PSSetShader(gParallaxMappingPixelShader, nullptr, 0);

	gD3DContext->PSSetShaderResources(0, 1, &gParaSphereDiffuseSpecularMapSRV);
	gD3DContext->PSSetShaderResources(1, 1, &gParaSphereMapSRV);

	gParaSphere->Render();

	gD3DContext->VSSetShader(gNormalCubeVertexShader, nullptr, 0);
	gD3DContext->PSSetShader(gNormalCubePixelShader, nullptr, 0);

	gD3DContext->PSSetShaderResources(0, 1, &gNormalCubeDiffuseSpecularMapSRV);
	gD3DContext->PSSetShaderResources(1, 1, &gNormalCubeMapSRV);
	gNormalCube->Render();

	gD3DContext->VSSetShader(gCSOVertexShader, nullptr, 0);
	gD3DContext->PSSetShader(gCSOPixelShader, nullptr, 0);

	gD3DContext->RSSetState(gCullFrontState);
	
	gTroll->Render();

	gD3DContext->VSSetShader(gCSVertexShader, nullptr, 0);
	gD3DContext->PSSetShader(gCSPixelShader, nullptr, 0);

	gD3DContext->RSSetState(gCullBackState);

	//// Also, cell shading uses a special 1D "cell map", which uses point sampling
	gD3DContext->PSSetShaderResources(0, 1, &gTrollDiffuseMapSRV); // First parameter must match texture slot number in the shaer
	gD3DContext->PSSetSamplers(0, 1, &gAnisotropic4xSampler);
	gD3DContext->PSSetShaderResources(1, 1, &gCellMapSRV); // First parameter must match texture slot number in the shaer
	gD3DContext->PSSetSamplers(1, 1, &gPointSampler);
	gTroll->Render();

	gD3DContext->VSSetShader(gPixelLightingVertexShader, nullptr, 0);
	gD3DContext->PSSetShader(gPixelLightingPixelShader, nullptr, 0);
	gD3DContext->PSSetShaderResources(0, 1, &gCloudCubeDiffuseSpecularMapSRV);

	gD3DContext->OMSetBlendState(gAlphaBlendingState, nullptr, 0xffffff);
	gD3DContext->OMSetDepthStencilState(gDepthReadOnlyState, 0);
	gD3DContext->RSSetState(gCullBackState);

	gCloudCube->Render();

	gD3DContext->PSSetShaderResources(0, 1, &gGolbezCubeDiffuseSpecularMapSRV);
	gD3DContext->OMSetBlendState(gMultiplicativeBlendingState, nullptr, 0xffffff);
	gD3DContext->OMSetDepthStencilState(gDepthReadOnlyState, 0);
	gD3DContext->RSSetState(gCullBackState);

	gGolbezCube->Render();

    //// Render lights ////

    // Select which shaders to use next
    gD3DContext->VSSetShader(gBasicTransformVertexShader, nullptr, 0);
    gD3DContext->PSSetShader(gLightModelPixelShader,      nullptr, 0);

    // Select the texture and sampler to use in the pixel shader
    gD3DContext->PSSetShaderResources(0, 1, &gLightDiffuseMapSRV); // First parameter must match texture slot number in the shaer
    gD3DContext->PSSetSamplers(0, 1, &gAnisotropic4xSampler);

    // States - additive blending, read-only depth buffer and no culling (standard set-up for blending
    gD3DContext->OMSetBlendState(gAdditiveBlendingState, nullptr, 0xffffff);
    gD3DContext->OMSetDepthStencilState(gDepthReadOnlyState, 0);
    gD3DContext->RSSetState(gCullNoneState);

    // Render all the lights in the array
    for (int i = 0; i < NUM_LIGHTS; ++i)
    {
        gPerModelConstants.objectColour = gLights[i].colour; // Set any per-model constants apart from the world matrix just before calling render (light colour here)
		if (i == 0 && gLights[0].strength > 1 || i == 1)
		{
			gLights[i].model->Render();
		}
		

    }
}




// Rendering the scene now renders everything twice. First it renders the scene for the portal into a texture.
// Then it renders the main scene using the portal texture on a model.
void RenderScene()
{
    //// Common settings ////

    // Set up the light information in the constant bufferh
    // Don't send to the GPU yet, the function RenderSceneFromCamera will do that
   gPerFrameConstants.light1Colour   = gLights[0].colour * gLights[0].strength;
   gPerFrameConstants.light1Position = gLights[0].model->Position();
   gPerFrameConstants.light1Facing   = Normalise(gLights[0].model->WorldMatrix().GetZAxis());    // Additional lighting information for spotlights
   gPerFrameConstants.light1CosHalfAngle = cos(ToRadians(gSpotlightConeAngle / 2)); // --"--
   gPerFrameConstants.light1ViewMatrix       = CalculateLightViewMatrix(0);         // Calculate camera-like matrices for...
   gPerFrameConstants.light1ProjectionMatrix = CalculateLightProjectionMatrix(0);   //...lights to support shadow mapping

	gPerFrameConstants.light2Colour = gLights[1].colour * gLights[1].strength;
	gPerFrameConstants.light2Position = gLights[1].model->Position();
	gPerFrameConstants.light2Facing = Normalise(gLights[1].model->WorldMatrix().GetZAxis());    // Additional lighting information for spotlights
	gPerFrameConstants.light2CosHalfAngle = cos(ToRadians(gSpotlightConeAngle / 2)); // --"--
	gPerFrameConstants.light2ViewMatrix = CalculateLightViewMatrix(1);         // Calculate camera-like matrices for...
	gPerFrameConstants.light2ProjectionMatrix = CalculateLightProjectionMatrix(1);   //...lights to support shadow mapping

	gPerFrameConstants.light3Colour = CVector3{ 0.0f, 1.0f, 0.0f };
	gPerFrameConstants.light3Direction = CVector3{ 0, -1, 0 };

    gPerFrameConstants.ambientColour  = gAmbientColour;
    gPerFrameConstants.specularPower  = gSpecularPower;
    gPerFrameConstants.cameraPosition = gCamera->Position();
	gPerFrameConstants.parallaxDepth = (gUseParallax ? gParallaxDepth : 0);

    //***************************************//
    //// Render from light's point of view ////
    
    // Only rendering from light 1 to begin with

    // Setup the viewport to the size of the shadow map texture
    D3D11_VIEWPORT vp;
    vp.Width  = static_cast<FLOAT>(gShadowMapSize);
    vp.Height = static_cast<FLOAT>(gShadowMapSize);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    gD3DContext->RSSetViewports(1, &vp);

    // Select the shadow map texture as the current depth buffer. We will not be rendering any pixel colours
    // Also clear the the shadow map depth buffer to the far distance
    gD3DContext->OMSetRenderTargets(0, nullptr, gShadowMap1DepthStencil);
    gD3DContext->ClearDepthStencilView(gShadowMap1DepthStencil, D3D11_CLEAR_DEPTH, 1.0f, 0);

    // Render the scene from the point of view of light 1 (only depth values written)
    RenderDepthBufferFromLight(0);

	gD3DContext->OMSetRenderTargets(0, nullptr, gShadowMap2DepthStencil);
	gD3DContext->ClearDepthStencilView(gShadowMap2DepthStencil, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// Render the scene from the point of view of light 1 (only depth values written)
	RenderDepthBufferFromLight(1);

    //**************************//


    //// Main scene rendering ////

    // Set the back buffer as the target for rendering and select the main depth buffer.
    // When finished the back buffer is sent to the "front buffer" - which is the monitor.
    gD3DContext->OMSetRenderTargets(1, &gBackBufferRenderTarget, gDepthStencil);

    // Clear the back buffer to a fixed colour and the depth buffer to the far distance
    gD3DContext->ClearRenderTargetView(gBackBufferRenderTarget, &gBackgroundColor.r);
    gD3DContext->ClearDepthStencilView(gDepthStencil, D3D11_CLEAR_DEPTH, 1.0f, 0);

    // Setup the viewport to the size of the main window
    vp.Width  = static_cast<FLOAT>(gViewportWidth);
    vp.Height = static_cast<FLOAT>(gViewportHeight);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    gD3DContext->RSSetViewports(1, &vp);

    // Set shadow maps in shaders
    // First parameter is the "slot", must match the Texture2D declaration in the HLSL code
    // In this app the diffuse map uses slot 0, the shadow maps use slots 1 onwards. If we were using other maps (e.g. normal map) then
    // we might arrange things differently
    gD3DContext->PSSetShaderResources(1, 1, &gShadowMap1SRV);
	gD3DContext->PSSetShaderResources(2, 1, &gShadowMap2SRV);
    gD3DContext->PSSetSamplers(1, 1, &gPointSampler);

    // Render the scene for the main window
    RenderSceneFromCamera(gCamera);

    // Unbind shadow maps from shaders - prevents warnings from DirectX when we try to render to the shadow maps again next frame
    ID3D11ShaderResourceView* nullView = nullptr;
    gD3DContext->PSSetShaderResources(1, 1, &nullView);

    //// Scene completion ////

    // When drawing to the off-screen back buffer is complete, we "present" the image to the front buffer (the screen)
    gSwapChain->Present(0, 0);
}


//--------------------------------------------------------------------------------------
// Scene Update
//--------------------------------------------------------------------------------------

// Update models and camera. frameTime is the time passed since the last frame
void UpdateScene(float frameTime)
{
	// Control sphere (will update its world matrix)
	gCharacter->Control(frameTime, Key_I, Key_K, Key_J, Key_L, Key_U, Key_O, Key_Period, Key_Comma );
	
	static float wiggleChange = 6.0f;
	gPerModelConstants.wiggle += wiggleChange * frameTime;

	if (KeyHeld(Key_Add))
	{
		wiggleChange += 0.001f;
	}
	if (KeyHeld(Key_Subtract))
	{
		wiggleChange -= 0.001f;
	}

    // Orbit the light - a bit of a cheat with the static variable [ask the tutor if you want to know what this is]
	static float rotate = 0.0f;
    static bool go = true;
	gLights[0].model->SetPosition( gCharacter->Position() + CVector3{ cos(rotate) * gLightOrbit, 10, sin(rotate) * gLightOrbit } );
	gLights[0].model->FaceTarget(gCharacter->Position());
	gLights[0].strength -= frameTime * 25;
	gLights[0].model->SetScale(gLights[0].model->Scale() * (1 - (frameTime * 25)));
	if (gLights[0].strength < 0)
	{
		gLights[0].strength = 25;
		gLights[0].model->SetScale(pow(gLights[1].strength, 0.7f));
	}
	gLights[1].colour.x += frameTime;
	gLights[1].colour.z += frameTime;
	if (gLights[1].colour.x > 1)
	{
		gLights[1].colour.x = 0;
		gLights[1].colour.y = 0;
	}
    if (go)  rotate -= gLightOrbitSpeed * frameTime;
    if (KeyHit(Key_1))  go = !go;

	// Troll movement
	float trollSpeed = 50.0f;
	static CVector3 posChange = CVector3{0.0f, 0.0f, 0.0f};

	if (KeyHit(Key_Space))
	{
		posChange.x = rand() % static_cast<int>(trollSpeed*2) - trollSpeed;
		posChange.z = rand() % static_cast<int>(trollSpeed*2) - trollSpeed;
		
	}

	gTroll->SetPosition(gTroll->Position() += posChange * frameTime);



	// Control camera (will update its view matrix)
	gCamera->Control(frameTime, Key_Up, Key_Down, Key_Left, Key_Right, Key_W, Key_S, Key_A, Key_D );


    // Show frame time / FPS in the window title //
    const float fpsUpdateTime = 0.5f; // How long between updates (in seconds)
    static float totalFrameTime = 0;
    static int frameCount = 0;
    totalFrameTime += frameTime;
    ++frameCount;
    if (totalFrameTime > fpsUpdateTime)
    {
        // Displays FPS rounded to nearest int, and frame time (more useful for developers) in milliseconds to 2 decimal places
        float avgFrameTime = totalFrameTime / frameCount;
        std::ostringstream frameTimeMs;
        frameTimeMs.precision(2);
        frameTimeMs << std::fixed << avgFrameTime * 1000;
        std::string windowTitle = "CO2409 Week 20: Shadow Mapping - Frame Time: " + frameTimeMs.str() +
                                  "ms, FPS: " + std::to_string(static_cast<int>(1 / avgFrameTime + 0.5f));
        SetWindowTextA(gHWnd, windowTitle.c_str());
        totalFrameTime = 0;
        frameCount = 0;
    }
}
