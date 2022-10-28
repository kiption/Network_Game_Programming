//-----------------------------------------------------------------------------
// File: CScene.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Scene.h"

CScene::CScene()
{
}

CScene::~CScene()
{
}

void CScene::BuildDefaultLightsAndMaterials()
{
	m_nLights = 4;
	m_pLights = new LIGHT[m_nLights];
	::ZeroMemory(m_pLights, sizeof(LIGHT) * m_nLights);

	m_xmf4GlobalAmbient = XMFLOAT4(0.2f, 0.2f, 0.4f, 1.0f);

	m_pLights[0].m_nType = DIRECTIONAL_LIGHT;
	m_pLights[0].m_fRange = 500.0;
	m_pLights[0].m_xmf4Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.5f);
	m_pLights[0].m_xmf4Diffuse = XMFLOAT4(0.6f, 0.6f, 0.5f, 0.5f);
	m_pLights[0].m_xmf4Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 0.0f);
	m_pLights[0].m_xmf3Position = XMFLOAT3(0.0f, 0.5f, 0.0f);
	m_pLights[0].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_pLights[0].m_xmf3Attenuation = XMFLOAT3(0.5f, 0.5f, 0.5f);
	m_pLights[0].m_fFalloff = 7.0f;
	m_pLights[0].m_fPhi = (float)cos(XMConvertToRadians(120.0f));
	m_pLights[0].m_fTheta = (float)cos(XMConvertToRadians(60.0f));
	m_pLights[0].m_bEnable = true;

	m_pLights[1].m_nType = SPOT_LIGHT;
	m_pLights[1].m_fRange = 400.0f;
	m_pLights[1].m_xmf4Ambient = XMFLOAT4(0.5f, 0.3f, 0.3f, 1.0f);
	m_pLights[1].m_xmf4Diffuse = XMFLOAT4(0.2f, 0.6f, 0.3f, 0.8f);
	m_pLights[1].m_xmf4Specular = XMFLOAT4(0.2f, 0.2f, 0.3f, 0.0f);
	m_pLights[1].m_xmf3Position = XMFLOAT3(0.0f, 30.0f, 1.0f);
	m_pLights[1].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_pLights[1].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights[1].m_fFalloff =5.0f;
	m_pLights[1].m_fPhi = (float)cos(XMConvertToRadians(32.0f));
	m_pLights[1].m_fTheta = (float)cos(XMConvertToRadians(20.0f));
	m_pLights[1].m_bEnable = true;

	m_pLights[2].m_nType = SPOT_LIGHT;
	m_pLights[2].m_fRange = 400.0f;
	m_pLights[2].m_xmf4Ambient = XMFLOAT4(0.5f, 0.3f, 0.3f, 1.0f);
	m_pLights[2].m_xmf4Diffuse = XMFLOAT4(0.2f, 0.6f, 0.3f, 0.8f);
	m_pLights[2].m_xmf4Specular = XMFLOAT4(0.2f, 0.2f, 0.3f, 0.0f);
	m_pLights[2].m_xmf3Position = XMFLOAT3(0.0f, 30.0f, 1.0f);
	m_pLights[2].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_pLights[2].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights[2].m_fFalloff = 5.0f;
	m_pLights[2].m_fPhi = (float)cos(XMConvertToRadians(32.0f));
	m_pLights[2].m_fTheta = (float)cos(XMConvertToRadians(20.0f));
	m_pLights[2].m_bEnable = true;

	m_pLights[3].m_nType = SPOT_LIGHT;
	m_pLights[3].m_fRange = 400.0f;
	m_pLights[3].m_xmf4Ambient = XMFLOAT4(0.5f, 0.3f, 0.3f, 1.0f);
	m_pLights[3].m_xmf4Diffuse = XMFLOAT4(0.2f, 0.6f, 0.3f, 0.8f);
	m_pLights[3].m_xmf4Specular = XMFLOAT4(0.2f, 0.2f, 0.3f, 0.0f);
	m_pLights[3].m_xmf3Position = XMFLOAT3(0.0f, 30.0f, 1.0f);
	m_pLights[3].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_pLights[3].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights[3].m_fFalloff = 5.0f;
	m_pLights[3].m_fPhi = (float)cos(XMConvertToRadians(32.0f));
	m_pLights[3].m_fTheta = (float)cos(XMConvertToRadians(20.0f));
	m_pLights[3].m_bEnable = true;

}


uniform_real_distribution<> uidx(200.0, 1000.0); uniform_real_distribution<> uidz(200.0, 2300.0);
uniform_real_distribution<> uidy(190.0, 195.0); uniform_real_distribution<> uid(0.0, 10.0);
void CScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);
	BuildDefaultLightsAndMaterials();

	m_ppShaderObjcet = new CShader * [3];

	m_ppShaderObjcet[0] = new CIlluminatedShader();
	CMaterial::PrepareShaders(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, &m_ppShaderObjcet[0]);
	m_ppShaderObjcet[0]->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

	CTerrainShader* pTerrainShader = new CTerrainShader();
	pTerrainShader->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

	CMaterialColors* pMaterialColors = new CMaterialColors();
	pMaterialColors->m_xmf4Ambient = XMFLOAT4(0.1f, 0.15f, 0.1f, 0.5f);
	pMaterialColors->m_xmf4Diffuse = XMFLOAT4(0.53f, 0.8f, 0.1f, 1.0f);
	pMaterialColors->m_xmf4Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f); //(r,g,b,a=power)
	pMaterialColors->m_xmf4Emissive = XMFLOAT4(0.2f, 0.5f, 0.2f, 1.0f);

	CMaterial* pMaterial = new CMaterial();
	pMaterial->SetShader(pTerrainShader);
	pMaterial->SetMaterialColors(pMaterialColors);

	XMFLOAT3 xmf3Scale(5.0f, 2.0f, 5.0f);
	XMFLOAT3 xmf3Pos(0.0, 0.0f, 0.0);
	XMFLOAT4 xmf4Color(0.0f, 0.2f, 0.0f, 0.0f);
	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Assets/Image/Terrain/terrain2.raw"), 513, 513, 8, 8, xmf3Scale, xmf4Color);

	m_pTerrain->SetPosition(xmf3Pos);
	m_pTerrain->SetMaterial(pMaterial);
	m_pTerrain->Rotate(0.0, 0.0, 0.0);

	m_nGameObjects = 3;
	m_ppGameObjects = new CGameObjcet * [m_nGameObjects];

	CGameObjcet* pTreeModel = CGameObjcet::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/NEBOOM.bin");

	CObstacleObject* pTree = NULL;
	pTree = new CObstacleObject();
	pTree->SetChild(pTreeModel, true);
	pTree->OnInitialize();
	pTree->SetScale(5.0, 5.0, 10.0);
	pTree->SetPosition(386.7, -15.0, 732.1);
	pTree->Rotate(0, 90.f, 0.f);
	m_ppGameObjects[0] = pTree;

	CGameObjcet* pPlayerCars = CGameObjcet::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/PoliceCar.bin");

	CPlayerObject* P2 = NULL;
	P2 = new CPlayerObject();
	P2->SetChild(pPlayerCars, true);
	P2->OnInitialize();
	P2->SetScale(5.0, 5.0, 7.0);
	P2->SetPosition(200.0, 2.0, 900.1);
	m_ppGameObjects[1] = P2;

	CPlayerObject* P3 = NULL;
	P3 = new CPlayerObject();
	P3->SetChild(pPlayerCars, true);
	P3->OnInitialize();
	P3->SetScale(5.0, 5.0, 7.0);
	P3->SetPosition(230.0, 2.0, 950.0);
	m_ppGameObjects[2] = P3;

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CScene::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();

	if (m_ppGameObjects)
	{
		for (int i = 0; i < m_nGameObjects; i++) m_ppGameObjects[i]->Release();
		delete[] m_ppGameObjects;
	}

	if (m_pTerrain) delete m_pTerrain;
	if (m_pLights) delete[] m_pLights;


	for (int i = 0; i < 2; i++)
	{
		m_ppShaderObjcet[i]->Release();
	}
	if (m_ppShaderObjcet) delete[] m_ppShaderObjcet;
}

ID3D12RootSignature* CScene::CreateGraphicsRootSignature(ID3D12Device* pd3dDevice)
{
	ID3D12RootSignature* pd3dGraphicsRootSignature = NULL;

	D3D12_ROOT_PARAMETER pd3dRootParameters[4];

	//Camera
	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[0].Descriptor.ShaderRegister = 1;
	pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	//GameObject
	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[1].Constants.Num32BitValues = 32;
	pd3dRootParameters[1].Constants.ShaderRegister = 2;
	pd3dRootParameters[1].Constants.RegisterSpace = 0;
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	//terrain
	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[2].Constants.Num32BitValues = 16;
	pd3dRootParameters[2].Constants.ShaderRegister = 3;
	pd3dRootParameters[2].Constants.RegisterSpace = 0;
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	//Lights
	pd3dRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[3].Descriptor.ShaderRegister = 4;
	pd3dRootParameters[3].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = 0;
	d3dRootSignatureDesc.pStaticSamplers = NULL;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob* pd3dSignatureBlob = NULL;
	ID3DBlob* pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void**)&pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	return(pd3dGraphicsRootSignature);
}

void CScene::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(LIGHTS) + 255) & ~255); //256ÀÇ ¹è¼ö
	m_pd3dcbLights = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbLights->Map(0, NULL, (void**)&m_pcbMappedLights);
}

void CScene::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	::memcpy(m_pcbMappedLights->m_pLights, m_pLights, sizeof(LIGHT) * m_nLights);
	::memcpy(&m_pcbMappedLights->m_xmf4GlobalAmbient, &m_xmf4GlobalAmbient, sizeof(XMFLOAT4));
	::memcpy(&m_pcbMappedLights->m_nLights, &m_nLights, sizeof(int));
}

void CScene::ReleaseShaderVariables()
{
	if (m_pd3dcbLights)
	{
		m_pd3dcbLights->Unmap(0, NULL);
		m_pd3dcbLights->Release();
	}
	if (m_ppShaderObjcet)
	{
		for (int i = 0; i < 3; i++)
			m_ppShaderObjcet[i]->Release();
	}
}

void CScene::ReleaseUploadBuffers()
{

	for (int i = 0; i < m_nGameObjects; i++) m_ppGameObjects[i]->ReleaseUploadBuffers();
	if (m_pTerrain) m_pTerrain->ReleaseUploadBuffers();

}

bool CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return(false);
}

bool CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{

	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_SPACE:
			((CMyPlayer*)m_pPlayer)->FireBullet(NULL);
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	return(false);
}

bool CScene::ProcessInput(UCHAR* pKeysBuffer)
{
	return(false);
}

void CScene::PickingToPlayer()
{


}
void CScene::CheckObjectByBulletCollisions()
{

}

void CScene::CheckObjectByPlayerCollisions(float fTimeElapsed)
{

	if (m_pTerrain->oobb.Intersects(m_pPlayer->oobb))
	{
		if (m_pPlayer->m_xmf3Position.z > 1200.0 && m_pPlayer->m_xmf3Position.z < 2100.0
			&& m_pPlayer->m_xmf3Position.x > 200.0 && m_pPlayer->m_xmf3Position.x < 5500.0)
		{
			m_bCollisionCheck = true;
			m_fSavePosition = m_pPlayer->m_xmf3Position.z;
		}
	}
	if (m_bCollisionCheck == true)
	{

		m_pPlayer->m_xmf3Position.z -= m_fCollisionVelocity;
		if (m_pPlayer->m_xmf3Position.z < m_fSavePosition - m_fCollisionRange)
		{
			m_pPlayer->m_xmf3Position.z += 1.0f;
			m_bCollisionCheck = false;

		}
	}
}

void CScene::AnimateObjects(float fTimeElapsed)
{

	m_fElapsedTime = fTimeElapsed;
	for (int i = 0; i < m_nGameObjects; i++) m_ppGameObjects[i]->Animate(fTimeElapsed, NULL);

	if (m_pLights)
	{
		XMFLOAT3 offset = XMFLOAT3(0, 20, 50);
		m_pLights[1].m_xmf3Position.z = m_pPlayer->GetPosition().z;
		m_pLights[1].m_xmf3Position.y = m_pPlayer->GetPosition().y + 10.0;
		m_pLights[1].m_xmf3Position.x = m_pPlayer->GetPosition().x;
		m_pLights[1].m_xmf3Direction = m_pPlayer->GetLookVector();
		XMStoreFloat3(&offset, XMVectorAdd(XMLoadFloat3(&m_pPlayer->GetPosition()), XMLoadFloat3(&offset)));

		XMFLOAT3 offsetp2 = XMFLOAT3(0, 20, 50);
		m_pLights[2].m_xmf3Position.z = m_ppGameObjects[1]->GetPosition().z;
		m_pLights[2].m_xmf3Position.y = m_ppGameObjects[1]->GetPosition().y + 10.0;
		m_pLights[2].m_xmf3Position.x = m_ppGameObjects[1]->GetPosition().x;
		m_pLights[2].m_xmf3Direction = m_ppGameObjects[1]->GetLook();
		XMStoreFloat3(&offset, XMVectorAdd(XMLoadFloat3(&m_ppGameObjects[1]->GetPosition()), XMLoadFloat3(&offset)));

		XMFLOAT3 offsetp3 = XMFLOAT3(0, 20, 50);
		m_pLights[3].m_xmf3Position.z = m_ppGameObjects[2]->GetPosition().z;
		m_pLights[3].m_xmf3Position.y = m_ppGameObjects[2]->GetPosition().y + 10.0;
		m_pLights[3].m_xmf3Position.x = m_ppGameObjects[2]->GetPosition().x;
		m_pLights[3].m_xmf3Direction = m_ppGameObjects[2]->GetLook();
		XMStoreFloat3(&offset, XMVectorAdd(XMLoadFloat3(&m_ppGameObjects[2]->GetPosition()), XMLoadFloat3(&offset)));

	}

	CheckObjectByPlayerCollisions(m_fElapsedTime);
	CheckObjectByBulletCollisions();
	PickingToPlayer();
}

void CScene::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	UpdateShaderVariables(pd3dCommandList);

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(3, d3dcbLightsGpuVirtualAddress); //Lights

	if (m_pTerrain) m_pTerrain->Render(pd3dCommandList, pCamera);

	for (int i = 0; i < m_nGameObjects; i++)
	{
		if (m_ppGameObjects[i])
		{
			m_ppGameObjects[i]->Animate(m_fElapsedTime, NULL);
			m_ppGameObjects[i]->UpdateTransform(NULL);
			m_ppGameObjects[i]->Render(pd3dCommandList, pCamera);
		}
	}
}


