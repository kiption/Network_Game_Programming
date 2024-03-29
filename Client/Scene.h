//-----------------------------------------------------------------------------
// File: Scene.h
//-----------------------------------------------------------------------------

#pragma once

#include "Shader.h"
#include "Player.h"
#include <random>
#include <array>
#include <utility>

#define MAX_LIGHTS			16 

#define POINT_LIGHT			1
#define SPOT_LIGHT			2
#define DIRECTIONAL_LIGHT	3

static std::random_device rd;
static std::default_random_engine dre(rd());

struct LIGHT
{
	XMFLOAT4				m_xmf4Ambient;
	XMFLOAT4				m_xmf4Diffuse;
	XMFLOAT4				m_xmf4Specular;
	XMFLOAT3				m_xmf3Position;
	float 					m_fFalloff;
	XMFLOAT3				m_xmf3Direction;
	float 					m_fTheta; //cos(m_fTheta)
	XMFLOAT3				m_xmf3Attenuation;
	float					m_fPhi; //cos(m_fPhi)
	bool					m_bEnable;
	int						m_nType;
	float					m_fRange;
	float					padding;
};

struct LIGHTS
{
	LIGHT					m_pLights[MAX_LIGHTS];
	XMFLOAT4				m_xmf4GlobalAmbient;
	int						m_nLights;
};

class CScene
{
public:
	CScene();
	~CScene();

	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	void BuildDefaultLightsAndMaterials();
	void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void ReleaseObjects();

	ID3D12RootSignature* CreateGraphicsRootSignature(ID3D12Device* pd3dDevice);
	ID3D12RootSignature* GetGraphicsRootSignature() { return(m_pd3dGraphicsRootSignature); }

	bool ProcessInput(UCHAR* pKeysBuffer);
	void AnimateObjects(float fTimeElapsed);
	void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);
	void ReleaseUploadBuffers();

public:
	void CheckPlayerByRandomBoxCollisions();
	void CheckWallByPlayerCollisions(float fTimeElapsed);
	void CheckPlayerByPlayerCollisions(float fTimeElapsed);
	void MissileProcess();
	void CheckMissileByPlayerCollisions(float fTimeElapsed);
	void TrapProcess();
	void CheckTrapByPlayerCollisions(float fTimeElapsed);
	void BoosterProcess();
public:
	float						m_fElapsedTime = 0.0f;
	
	ID3D12RootSignature			*m_pd3dGraphicsRootSignature = NULL;
	
	CGameObjcet					**m_ppGameObjects = NULL;
	int							m_nGameObjects=0;

	int							m_nLights = 0;
	LIGHT						*m_pLights = NULL;
	LIGHTS						*m_pcbMappedLights = NULL;
	ID3D12Resource*				m_pd3dcbLights = NULL;
	XMFLOAT4					m_xmf4GlobalAmbient;

	CPlayer*					m_pPlayer = NULL;
	CCamera*					m_pCamera = NULL;
	CHeightMapTerrain*			m_pTerrain = NULL;
	CHeightMapTerrain*			m_pCollisionTerrain = NULL;

	CBulletObject*				pBulletObject = NULL;
	CShader*					m_pShader = NULL;

	float						m_fSavePosition = 0.0;
	float						m_fCollisionRange = 100.0;
	float						m_fCollisionVelocity = 2.5;
	bool						m_bCollisionCheck = false;

	float						m_fBoxCoordinateX;
	float						m_fBoxCoordinateZ;
	
	bool						m_bObjectCollideCheck = false;
	double radian;

public:

	//Active
	bool m_bMissileActive = false;
	bool m_bTrapActive = false;
	bool m_bBoosterActive = false;

	//Check
	bool m_bMissileCollision = false;
	bool m_bTrapCollision = false;

protected:
	CTerrainShader* m_pTerrainShader=NULL;
	CShader** m_ppShaderObjcet=NULL;

};
