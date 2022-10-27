//------------------------------------------------------- ----------------------
// File: Object.h
//-----------------------------------------------------------------------------

#pragma once

#include "Mesh.h"
#include "Camera.h"

#define DIR_FORWARD					0x01
#define DIR_BACKWARD				0x02
#define DIR_LEFT					0x04
#define DIR_RIGHT					0x08
#define DIR_UP						0x10
#define DIR_DOWN					0x20

class CShader;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#define MATERIAL_ALBEDO_MAP			0x01
#define MATERIAL_SPECULAR_MAP		0x02
#define MATERIAL_NORMAL_MAP			0x04
#define MATERIAL_METALLIC_MAP		0x08
#define MATERIAL_EMISSION_MAP		0x10
#define MATERIAL_DETAIL_ALBEDO_MAP	0x20
#define MATERIAL_DETAIL_NORMAL_MAP	0x40

struct MATERIALLOADINFO
{
	XMFLOAT4						m_xmf4AlbedoColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4						m_xmf4EmissiveColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4						m_xmf4SpecularColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	float							m_fGlossiness = 0.0f;
	float							m_fSmoothness = 0.0f;
	float							m_fSpecularHighlight = 0.0f;
	float							m_fMetallic = 0.0f;
	float							m_fGlossyReflection = 0.0f;

	UINT							m_nType = 0x00;
};

struct MATERIALSLOADINFO
{
	int								m_nMaterials = 0;
	MATERIALLOADINFO* m_pMaterials = NULL;
};

class CMaterialColors
{
private:
	int								m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

public:
	CMaterialColors() { }
	CMaterialColors(MATERIALLOADINFO* pMaterialInfo);
	virtual ~CMaterialColors() { }

	XMFLOAT4						m_xmf4Ambient = XMFLOAT4(0.2f, 0.2f, 0.1f, 0.7f);
	XMFLOAT4						m_xmf4Diffuse = XMFLOAT4(0.1f, 0.0f, 0.2f, 0.6f);
	XMFLOAT4						m_xmf4Specular = XMFLOAT4(0.5f, 0.2f, 0.1f, 0.5f); //(r,g,b,a=power)
	XMFLOAT4						m_xmf4Emissive = XMFLOAT4(0.6f, 0.0f, 0.4f, 1.0f);
};

class CMaterial
{
public:
	CMaterial();
	virtual ~CMaterial();

private:
	int								m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList);
	void SetMaterialColors(CMaterialColors* pMaterialColors);
	void SetIlluminatedShader() { SetShader(m_pIlluminatedShader); }
	void SetShader(CShader* pShader);

public:
	static CShader* m_pIlluminatedShader;
	CMaterialColors* m_pMaterialColors = NULL;
	CShader* m_pShader = NULL;

public:
	static void CMaterial::PrepareShaders(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CShader** shader);
};



class CGameObjcet
{
private:
	int								m_nReferences = 0;

public:
	void AddRef();
	void Release();

public:
	CGameObjcet();
	virtual ~CGameObjcet();

public:
	bool m_bActive = true;
	void SetMovingDirection(const XMFLOAT3& xmf3MovingDirection);
	void SetActive(bool bActive) { m_bActive = bActive; }

public:
	char							m_pstrFrameName[64];

	CMesh*							m_pMesh = NULL;
	CMesh**							m_ppMeshes = NULL;
	int								m_nMeshes = 0;

	CMaterial**						m_ppMaterials = NULL;
	int								m_nMaterials = 0;


	XMFLOAT4X4						m_xmf4x4Transform;
	XMFLOAT4X4						m_xmf4x4Position;
	XMFLOAT4X4						m_xmf4x4World;

	CGameObjcet* m_pParent = NULL;
	CGameObjcet* m_pChild = NULL;
	CGameObjcet* m_pSibling = NULL;

	virtual void SetMesh(CMesh* pMesh);
	virtual void SetShader(CShader* pShader);
	virtual void SetShader(int nMaterial, CShader* pShader);
	virtual void SetMaterial(int nMaterial, CMaterial* pMaterial);
	virtual void SetPosition(float x, float y, float z);
	virtual void SetPosition(XMFLOAT3 xmf3Position);
	void SetChild(CGameObjcet* pChild, bool bReferenceUpdate = false);
	
	virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL);

	virtual void OnPrepareRender() { }
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);

	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World);
	virtual void ReleaseShaderVariables();
	virtual void ReleaseUploadBuffers();

	void SetScale(float x, float y, float z);
	XMFLOAT3 GetPosition();
	XMFLOAT3 GetLook();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRight();
	float x, y, z;
	void Rotate(float fPitch, float fYaw, float fRoll);
	void Rotate(XMFLOAT3* pxmf3Axis, float fAngle);
	void Rotate(XMFLOAT4* pxmf4Quaternion);
	virtual void SetRotationAxis(XMFLOAT3& xmf3RotationAxis);
	CGameObjcet* GetParent() { return(m_pParent); }
	void UpdateTransform(XMFLOAT4X4* pxmf4x4Parent = NULL);
	CGameObjcet* FindFrame(char* pstrFrameName);

	UINT GetMeshType() { return((m_pMesh) ? m_pMesh->GetType() : 0); }
	CPlayer* m_pPlayer = NULL;
	CCamera* m_pCamera = NULL;
	CShader* m_pShader = NULL;
public:
	static MATERIALSLOADINFO* LoadMaterialsInfoFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, FILE* pInFile);
	static CMeshLoadInfo* LoadMeshInfoFromFile(FILE* pInFile);

	static CGameObjcet* LoadFrameHierarchyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, FILE* pInFile);
	static CGameObjcet* LoadGeometryFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, char* pstrFileName);

	static void PrintFrameInfo(CGameObjcet* pGameObject, CGameObjcet* pParent);
public:


	float m_fMovingSpeed = 0.0f;
	float m_fMovingRange = 0.0f;
	float m_fRotationSpeed = 0.0f;
	XMFLOAT3 m_xmf3RotationAxis = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMFLOAT3 m_xmf3MovingDirection = XMFLOAT3(0.0f, 1.0f, 0.0f);
	void SetRotationSpeed(float fSpeed) { m_fRotationSpeed = fSpeed; }
	void SetMovingSpeed(float fSpeed) { m_fMovingSpeed = fSpeed; }
public:
	BoundingOrientedBox			oobb = BoundingOrientedBox();
	void UpdateBoundingBox();
};

class CPlayerObject : public CGameObjcet
{
public:
	CPlayerObject();
	virtual ~CPlayerObject();
	CGameObjcet* m_pMainRotorFrame = NULL;
	CGameObjcet* m_pTailRotorFrame = NULL;

public:
	virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL);
	virtual void OnInitialize();

};

class CObstacleObject :public CGameObjcet
{
public:
	CObstacleObject() {};
	virtual ~CObstacleObject() {};
	void GenerateRayForPicking(XMVECTOR& xmvPickPosition, XMMATRIX& xmmtxView, XMVECTOR& xmvPickRayOrigin, XMVECTOR& xmvPickRayDirection);
	int PickObjectByRayIntersection(XMVECTOR& xmvPickPosition, XMMATRIX& xmmtxView, float* pfHitDistance);
	CGameObjcet* m_pMainRotorFrame = NULL;
	CGameObjcet* m_pTailRotorFrame = NULL;


public:
	virtual void OnInitialize();
	virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL);
};

class CTerrainObject : public CGameObjcet
{
private:
	int m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

public:
	CTerrainObject(int nMeshes = 1);
	virtual ~CTerrainObject();
	CMesh** m_ppMeshes = NULL;
	int m_nMeshes = 0;

protected:
	XMFLOAT4X4 m_xmf4x4World;
	CShader* m_pShader = NULL;
	CMaterial* m_pMaterial = NULL;

public:
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void SetMesh(int nIndex, CHeightMapGridMesh* pMesh);
	virtual void SetShader(CShader* pShader);
	virtual void SetMaterial(CMaterial* pMaterial) { m_pMaterial = pMaterial; }
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
};

class CHeightMapTerrain : public CTerrainObject
{
public:
	CHeightMapTerrain(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, LPCTSTR pFileName, int nWidth, int nLength, int nBlockWidth, int nBlockLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color);
	virtual ~CHeightMapTerrain();

private:
	XMFLOAT3 m_xmf3Scale;
	CHeightMapImage* m_pHeightMapImage;
	int m_nWidth;
	int m_nLength;

public:
	XMFLOAT3 GetNormal(float x, float z) {return(m_pHeightMapImage->GetHeightMapNormal(int(x / m_xmf3Scale.x), int(z / m_xmf3Scale.z)));}
	XMFLOAT3 GetScale() { return(m_xmf3Scale); }
	
	int GetHeightMapWidth() { return(m_pHeightMapImage->GetHeightMapWidth()); }
	int GetHeightMapLength() { return(m_pHeightMapImage->GetHeightMapLength()); }
	
	float GetHeight(float x, float z) { return(m_pHeightMapImage->GetHeight(x / m_xmf3Scale.x, z / m_xmf3Scale.z) * m_xmf3Scale.y); }
	float GetWidth() { return(m_nWidth * m_xmf3Scale.x); }
	float GetLength() { return(m_nLength * m_xmf3Scale.z); }
};

class CBulletObject : public CGameObjcet
{
public:
	CBulletObject(float fEffectiveRange);
	virtual ~CBulletObject();

public:
	virtual void Animate(float fElapsedTime);

	float						m_fBulletEffectiveRange = 700.0f;
	float						m_fMovingDistance = 0.0f;
	float						m_fRotationAngle = 0.0f;
	XMFLOAT3					m_xmf3FirePosition = XMFLOAT3(0.0f, 0.0f, 1.0f);

	float						m_fElapsedTimeAfterFire = 0.0f;
	float						m_fLockingDelayTime = 0.3f;
	float						m_fLockingTime = 5.0f;
	CGameObjcet* m_pLockedObject = NULL;
	CCamera* m_pCamera = NULL;

	void SetFirePosition(XMFLOAT3 xmf3FirePosition);
	void Reset();
};
