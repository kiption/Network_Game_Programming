//-----------------------------------------------------------------------------
// File: CScene.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Scene.h"
//#include "ObjINFO.h"
random_device ItemVal;
default_random_engine ItemRanVal(ItemVal());
uniform_int_distribution<>PresentItemVal(1, 3);

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
	m_pLights[1].m_fFalloff = 5.0f;
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
	m_pLights[2].m_fFalloff = 7.0f;
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
random_device CoordRd;
default_random_engine CoordDre(CoordRd());

uniform_real_distribution<> uidx(200.0, 1000.0);
uniform_real_distribution<> uidy(190.0, 195.0);
uniform_real_distribution<> uidz(200.0, 2300.0);
uniform_real_distribution<> uid(0.0, 10.0);
uniform_real_distribution<> RandomBoxCorX(330.0f, 810.0f);
uniform_real_distribution<> RandomBoxCorZ(799.0f, 805.0f);
float Random(float fMin, float fMax)
{
	float fRandomValue = (float)rand();
	if (fRandomValue < fMin) fRandomValue = fMin;
	if (fRandomValue > fMax) fRandomValue = fMax;
	return(fRandomValue);
}

float Random()
{
	return(rand() / float(RAND_MAX));
}

XMFLOAT3 RandomPositionInSphere(XMFLOAT3 xmf3Center, float fRadius, int nColumn, int nColumnSpace)
{
	float fAngle = Random() * 360.0f * (2.0f * 3.14159f / 360.0f);

	XMFLOAT3 xmf3Position;
	xmf3Position.x = xmf3Center.x + fRadius * sin(fAngle);
	xmf3Position.y = xmf3Center.y - (nColumn * float(nColumnSpace) / 2.0f) + (nColumn * nColumnSpace) + Random();
	xmf3Position.z = xmf3Center.z + fRadius * cos(fAngle);

	return(xmf3Position);
}
void CScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	for (int degree = 1; degree <= 360; degree++)
	{
		radian = XMConvertToRadians(degree);
	}

	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);
	BuildDefaultLightsAndMaterials();

	m_ppShaderObjcet = new CShader * [3];

	m_ppShaderObjcet[0] = new CIlluminatedShader();
	CMaterial::PrepareShaders(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, &m_ppShaderObjcet[0]);
	m_ppShaderObjcet[0]->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

	CTerrainShader* pTerrainShader = new CTerrainShader();
	pTerrainShader->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	CMaterialColors* pMaterialColors = new CMaterialColors();
	pMaterialColors->m_xmf4Ambient = XMFLOAT4(0.1f, 0.2f, 0.1f, 0.5f);
	pMaterialColors->m_xmf4Diffuse = XMFLOAT4(0.15f, 0.45f, 0.05f, 1.0f);
	pMaterialColors->m_xmf4Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	pMaterialColors->m_xmf4Emissive = XMFLOAT4(0.2f, 0.5f, 0.2f, 1.0f);
	CMaterial* pMaterial = new CMaterial();
	pMaterial->SetShader(pTerrainShader);
	pMaterial->SetMaterialColors(pMaterialColors);
	XMFLOAT3 xmf3Scale(5.0f, 4.0f, 5.0f);
	XMFLOAT3 xmf3Pos(0.0f, 5.0f, 0.0f);
	XMFLOAT4 xmf4Color(0.0f, 0.2f, 0.0f, 0.0f);
	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Assets/Image/Terrain/SquareMap.raw"), 513, 513, 16, 16, xmf3Scale, xmf4Color);
	m_pTerrain->SetPosition(xmf3Pos);
	m_pTerrain->SetMaterial(pMaterial);


	CTerrainShader* pCollisionTerrainShader = new CTerrainShader();
	pCollisionTerrainShader->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	CMaterialColors* pCollisionMaterialColors = new CMaterialColors();
	pCollisionMaterialColors->m_xmf4Ambient = XMFLOAT4(0.2f, 0.3f, 0.5f, 0.5f);
	pCollisionMaterialColors->m_xmf4Diffuse = XMFLOAT4(0.53f, 0.8f, 0.1f, 1.0f);
	pCollisionMaterialColors->m_xmf4Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	pCollisionMaterialColors->m_xmf4Emissive = XMFLOAT4(0.2f, 0.5f, 0.2f, 1.0f);
	CMaterial* pCollisonMaterial = new CMaterial();
	pCollisonMaterial->SetShader(pCollisionTerrainShader);
	pCollisonMaterial->SetMaterialColors(pCollisionMaterialColors);
	XMFLOAT3 xmf3collisionScale(5.0f, 2.0f, 5.0f);
	XMFLOAT3 xmf3collisionPos(0.0f, 0.0f, 0.0f);
	m_pCollisionTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Assets/Image/Terrain/SquareMap.raw"), 513, 513, 16, 16, xmf3collisionScale, xmf4Color);
	m_pCollisionTerrain->SetPosition(xmf3collisionPos);
	m_pCollisionTerrain->SetMaterial(pCollisonMaterial);


	m_nGameObjects = 190;
	m_ppGameObjects = new CGameObjcet * [m_nGameObjects];

	CGameObjcet* pPlayerCars2 = CGameObjcet::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/race.bin");
	CPlayerObject* P2 = NULL;
	P2 = new CPlayerObject();
	P2->SetChild(pPlayerCars2, true);
	P2->OnInitialize();
	pPlayerCars2->SetScale(0.95f, 0.95f, 0.95f);
	P2->SetPosition(450.0f, 14.0f, 480.0f);
	m_ppGameObjects[0] = P2;

	CGameObjcet* pPlayerCars3 = CGameObjcet::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/race.bin");
	CPlayerObject* P3 = NULL;
	P3 = new CPlayerObject();
	P3->SetChild(pPlayerCars3, true);
	P3->OnInitialize();
	pPlayerCars3->SetScale(0.95f, 0.95f, 0.95f);
	P3->SetPosition(500.0, 14.0, 480.0f);
	m_ppGameObjects[1] = P3;

	CGameObjcet* pBoxModel = CGameObjcet::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Mystery_box.bin");

	for (int i = 2; i < 14; i++)
	{
		m_ppGameObjects[i] = new CObstacleObject();
		m_ppGameObjects[i]->SetChild(pBoxModel, true);
		m_ppGameObjects[i]->SetScale(80.0f, 80.0f, 80.0f);
		m_ppGameObjects[i]->SetPosition(350.0f + (i * 40), 20.0f, 800.0f);
		m_ppGameObjects[i]->Rotate(45.0f, 0.0f, 0.0f);
	}

	CGameObjcet* pArrowModel = CGameObjcet::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Arrow5.bin");
	CObstacleObject* pArrow1 = NULL;
	pArrow1 = new CObstacleObject();
	pArrow1->SetChild(pArrowModel, true);
	pArrow1->OnInitialize();
	pArrow1->SetScale(3.0f, 3.0f, 3.0f);
	pArrow1->SetPosition(305.0f, 25.0f, 2400.0f);
	pArrow1->Rotate(0.0f, 180.0f, 0.0f);
	m_ppGameObjects[14] = pArrow1;

	CObstacleObject* pArrow2 = NULL;
	pArrow2 = new CObstacleObject();
	pArrow2->SetChild(pArrowModel, true);
	pArrow2->OnInitialize();
	pArrow2->SetScale(3.0f, 3.0f, 3.0f);
	pArrow2->SetPosition(2330, 25.0f, 2200.0f);
	pArrow2->Rotate(0.0f, 270.0f, 0.0f);
	m_ppGameObjects[15] = pArrow2;

	CObstacleObject* pArrow3 = NULL;
	pArrow3 = new CObstacleObject();
	pArrow3->SetChild(pArrowModel, true);
	pArrow3->OnInitialize();
	pArrow3->SetScale(3.0f, 3.0f, 3.0f);
	pArrow3->SetPosition(2225.0f, 25.0f, 230.0f);
	pArrow3->Rotate(0.0f, 0.0f, 0.0f);
	m_ppGameObjects[16] = pArrow3;

	CObstacleObject* pArrow4 = NULL;
	pArrow4 = new CObstacleObject();
	pArrow4->SetChild(pArrowModel, true);
	pArrow4->OnInitialize();
	pArrow4->SetScale(3.0f, 3.0f, 3.0f);
	pArrow4->SetPosition(330.0f, 25.0f, 340.0f);
	pArrow4->Rotate(0.0f, 90.0f, 0.0f);
	m_ppGameObjects[17] = pArrow4;


	CGameObjcet* pMiddleLineModel = CGameObjcet::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Tree.bin");
	for (int i = 18; i < 188; i++)
	{
		m_ppGameObjects[i] = new CObstacleObject();
		m_ppGameObjects[i]->SetChild(pMiddleLineModel, true);
		m_ppGameObjects[i]->SetScale(3.0f, 10.0f, 3.0f);
	}
	for (int i = 18; i < 59; i++)m_ppGameObjects[i]->SetPosition(280.0, 6.0f, 0.0f + (float)i * 45);
	for (int i = 59; i < 65; i++)m_ppGameObjects[i]->SetPosition(((float)(i - 50) * 25) + 220.0 + cos(radian) * 70.0, 6.0f, ((float)(i - 50) * 25) + 2230.0 + sin(radian) * 70.0);
	for (int i = 65; i < 106; i++)m_ppGameObjects[i]->SetPosition(450.0 + (float)((i - 56) * 45.0), 6.0f, 2350.0);
	for (int i = 106; i < 147; i++)m_ppGameObjects[i]->SetPosition(2300.0, 6.0f, 0.0f + 280.0 + (float)((i - 97) * 45));
	for (int i = 147; i < 188; i++)m_ppGameObjects[i]->SetPosition(450.0 + (float)((i - 138) * 45.0), 6.0f, 250.0);
	

	CGameObjcet* pMissileModel = CGameObjcet::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Roket.bin");
	CObstacleObject* pMissiles = NULL;
	pMissiles = new CObstacleObject();
	pMissiles->SetChild(pMissileModel, true);
	pMissiles->OnInitialize();
	pMissiles->SetScale(300.0f, 300.0f, 500.0f);
	pMissiles->SetPosition(0.0f, 0.0f,0.0f);
	pMissiles->Rotate(90.0f, 0.0f, 0.0f);
	m_ppGameObjects[188] = pMissiles;

	CGameObjcet* pTrapModel = CGameObjcet::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/stone_largeB.bin");
	CObstacleObject* pTrap = NULL;
	pTrap = new CObstacleObject();
	pTrap->SetChild(pTrapModel, true);
	pTrap->OnInitialize();
	pTrap->SetScale(0.1, 0.1, 0.1);
	pTrap->SetPosition(0.0f, 0.0f, 0.0f);
	pTrap->Rotate(0.0f, 0.0f, 0.0f);
	m_ppGameObjects[189] = pTrap;

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
	if (m_pCollisionTerrain) delete m_pCollisionTerrain;
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
	UINT ncbElementBytes = ((sizeof(LIGHTS) + 255) & ~255); //256�� ���
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
	if (m_pCollisionTerrain) m_pCollisionTerrain->ReleaseUploadBuffers();

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
			if (m_bMissileActive == true) { ((CMyPlayer*)m_pPlayer)->MissileMode(NULL); }
			//if (m_bBoosterActive == true) { ((CMyPlayer*)m_pPlayer)->BoosterMode(); }
			if (m_bTrapActive == true) { ((CMyPlayer*)m_pPlayer)->TrapMode(); }
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

void CScene::CheckPlayerByRandomBoxCollisions()
{
//	//objinfo.m_xoobb.PlayerOOBB = BoundingOrientedBox(objinfo.GetPosition(), XMFLOAT3(4.0, 4.0, 2.0), XMFLOAT4(0.0, 0.0, 0.0, 1.0));
//	//objinfo.m_xoobb.ItemboxOOBB = BoundingOrientedBox(XMFLOAT3(350.0f + (2 * 40), 20.0f, 800.0f), XMFLOAT3(4.0, 4.0, 2.0), XMFLOAT4(0.0, 0.0, 0.0, 1.0));
//
//	for (int i = 2; i < 3; ++i) {
//
//		m_ppGameObjects[2]->SetRotationSpeed(2.0f);
//		m_ppGameObjects[2]->Rotate(0, m_ppGameObjects[2]->m_fRotationSpeed, 0);
//
//		//if (objinfo.m_xoobb.ItemboxOOBB.Intersects(objinfo.m_xoobb.PlayerOOBB)) {
//		//	m_ppGameObjects[2]->m_bObjectCollideCheck = true;
//		//}
//		if (m_ppGameObjects[2]->m_bObjectCollideCheck) {
//			m_ppGameObjects[2]->m_xmf4x4Transform._42 -= 0.5f;
//			if (m_ppGameObjects[2]->m_xmf4x4Transform._42 < -100.0) {
//				m_ppGameObjects[2]->m_bObjectCollideCheck = false;
//				m_ppGameObjects[2]->m_bObjectRising = true;
//				int RandomItemVal = PresentItemVal(ItemRanVal);
//				m_pPlayer->m_iItemVal = RandomItemVal;
//			}
//		}
//
//		else {
//			if (m_ppGameObjects[2]->m_bObjectRising) {
//				m_ppGameObjects[2]->m_xmf4x4Transform._42 += 1.5f;
//			}
//			if (m_ppGameObjects[2]->m_xmf4x4Transform._42 >= 20.0f) {
//				m_ppGameObjects[2]->m_xmf4x4Transform._42 = 20.0f;
//				m_ppGameObjects[2]->m_bObjectRising = false;
//			}
//		}
//	}
//
//	if (m_pPlayer->m_iItemVal == 1)
//	{
//		m_bMissileActive = true;
//		m_bTrapActive = false;
//		m_bBoosterActive = false;
//		MissileProcess();
//	}
//	else if (m_pPlayer->m_iItemVal == 2)
//	{
//		m_bTrapActive = true;
//		m_bMissileActive = false;
//		m_bBoosterActive = false;
//		TrapProcess();
//	}
//	else if (m_pPlayer->m_iItemVal == 3)
//	{
//		m_bBoosterActive = true;
//		m_bMissileActive = false;
//		m_bTrapActive = false;
//		BoosterProcess();
//	}
//
}

void CScene::CheckWallByPlayerCollisions(float fTimeElapsed)
{

	/*if (m_pPlayer->m_xmf3Position.y < 12.0)
	{
		m_pPlayer->m_xmf3Position.y -= 0.05f;
		m_bCollisionCheck = true;
	}
	else
	{
		m_bCollisionCheck = false;
	}*/

}

void CScene::CheckPlayerByPlayerCollisions(float fTimeElapsed)
{
}

void CScene::MissileProcess()
{
	cout << "Get Missile" << endl;
	m_pPlayer->m_iItemVal = 0;
}

void CScene::CheckMissileByPlayerCollisions(float fTimeElapsed)
{

	//CBulletObject** ppBullets = ((CMyPlayer*)m_pPlayer)->m_ppBullets;
	////objinfo.m_xoobb.PlayerOOBB = BoundingOrientedBox(objinfo.GetPosition(), XMFLOAT3(4.0, 4.0, 2.0), XMFLOAT4(0.0, 0.0, 0.0, 1.0));

	//for (int i = 0; i < 2; i++) {
	//	for (int j = 0; j < BULLETS; j++)
	//	{
	//		// Coordinate None Recv
	//		// ppBullets[j]->m_Boobb = objinfo.m_xoobb.MissileOOBB = BoundingOrientedBox(objinfo.GetPosition(), XMFLOAT3(2.0, 2.0, 4.0), XMFLOAT4(0.0, 0.0, 0.0, 1.0));

	//		//if (ppBullets[j]->m_bActive && (objinfo.m_xoobb.PlayerOOBB.Intersects(ppBullets[j]->m_Boobb)))
	//		//{
	//		//	cout << "Collision!!-Missile" << endl;
	//		//	m_bMissileCollision = true;

	//		//}
	//	}
	//}

	/*if (m_bMissileCollision == true)
	{
		m_ppGameObjects[0]->m_xmf4x4Transform._42 += 80.0 * fTimeElapsed;
		m_ppGameObjects[0]->Rotate(20.0, 0.0, 0.0);
		if (m_ppGameObjects[0]->m_xmf4x4Transform._42 > 80.0) m_bMissileCollision = false;
	}

	if (m_bMissileCollision == false)
	{
		if (m_ppGameObjects[0]->m_xmf4x4Transform._42 < 14.0 + 0.1) {
			m_ppGameObjects[0]->m_xmf4x4Transform._42 = 14.0;
			m_ppGameObjects[0]->Rotate(0.0, 0.0, 0.0);
		}
		else
		{
			m_ppGameObjects[0]->m_xmf4x4Transform._42 -= 80.0 * fTimeElapsed;
			m_ppGameObjects[0]->Rotate(-20.5, 0.0, 0.0);
		}
	}*/

}

void CScene::TrapProcess()
{
	cout << "Get Trap" << endl;
	m_pPlayer->m_iItemVal = 0;
}


void CScene::CheckTrapByPlayerCollisions(float fTimeElapsed)
{
	//CTrapObject* ppTrap = ((CMyPlayer*)m_pPlayer)->m_pTrapObject;
	////objinfo.m_xoobb.PlayerOOBB = BoundingOrientedBox(objinfo.GetPosition(), XMFLOAT3(4.0, 4.0, 2.0), XMFLOAT4(0.0, 0.0, 0.0, 1.0));
	////for (int i = 0; i < 2; i++)
	////{
	////	if ((objinfo.m_xoobb.PlayerOOBB.Intersects(ppTrap->m_Boobb)))
	////	{
	////		m_bTrapCollision = true;
	////	}
	////}

	//if (m_bTrapCollision == true)
	//{
	//	m_ppGameObjects[0]->m_xmf4x4Transform._41 += 10.0 * fTimeElapsed;
	//	m_ppGameObjects[0]->m_xmf4x4Transform._42 += 30.0 * fTimeElapsed;
	//	m_ppGameObjects[0]->Rotate(0.0, 10.0, 0.0);
	//	if (m_ppGameObjects[0]->m_xmf4x4Transform._42 > 8.0)
	//	{
	//		m_bTrapCollision = false;
	//	}
	//}

	//if (m_bTrapCollision == false)
	//{
	//	if (m_ppGameObjects[0]->m_xmf4x4Transform._42 < 2.0 + 0.1) {
	//		m_ppGameObjects[0]->Rotate(0.0, 0.0, 0.0);
	//		m_ppGameObjects[0]->m_xmf4x4Transform._42 = 2.0f;
	//	}
	//	else
	//	{
	//		m_ppGameObjects[0]->m_xmf4x4Transform._42 -= 30.0 * fTimeElapsed;
	//	}
	//}
}

void CScene::BoosterProcess()
{
	cout << "Get Booster" << endl;
	m_pPlayer->m_iItemVal = 0;
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
		m_pLights[2].m_xmf3Position.z = m_ppGameObjects[0]->GetPosition().z;
		m_pLights[2].m_xmf3Position.y = m_ppGameObjects[0]->GetPosition().y + 10.0;
		m_pLights[2].m_xmf3Position.x = m_ppGameObjects[0]->GetPosition().x;
		m_pLights[2].m_xmf3Direction = m_ppGameObjects[0]->GetLookVector();
		XMStoreFloat3(&offset, XMVectorAdd(XMLoadFloat3(&m_ppGameObjects[0]->GetPosition()), XMLoadFloat3(&offset)));

		XMFLOAT3 offsetp3 = XMFLOAT3(0, 20, 50);
		m_pLights[3].m_xmf3Position.z = m_ppGameObjects[1]->GetPosition().z;
		m_pLights[3].m_xmf3Position.y = m_ppGameObjects[1]->GetPosition().y + 10.0;
		m_pLights[3].m_xmf3Position.x = m_ppGameObjects[1]->GetPosition().x;
		m_pLights[3].m_xmf3Direction = m_ppGameObjects[1]->GetLookVector();
		XMStoreFloat3(&offset, XMVectorAdd(XMLoadFloat3(&m_ppGameObjects[1]->GetPosition()), XMLoadFloat3(&offset)));

	}

	CheckPlayerByPlayerCollisions(m_fElapsedTime);
	CheckMissileByPlayerCollisions(m_fElapsedTime);
	CheckTrapByPlayerCollisions(m_fElapsedTime);
	CheckWallByPlayerCollisions(m_fElapsedTime);
	CheckPlayerByRandomBoxCollisions();


}

void CScene::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	UpdateShaderVariables(pd3dCommandList);

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(3, d3dcbLightsGpuVirtualAddress); //Lights

	if (m_pCollisionTerrain) m_pCollisionTerrain->Render(pd3dCommandList, pCamera);
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

