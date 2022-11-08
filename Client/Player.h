#pragma once

#define DIR_FORWARD				0x01
#define DIR_BACKWARD			0x02
#define DIR_LEFT				0x04
#define DIR_RIGHT				0x08
#define DIR_UP					0x10
#define DIR_DOWN				0x20

#include "Object.h"
#include "Camera.h"

class CPlayer : public CGameObjcet
{
protected:
	XMFLOAT3					m_xmf3Right;
	XMFLOAT3					m_xmf3Up;

	XMFLOAT3					m_xmf3Look = XMFLOAT3(0,0,1);
	XMFLOAT3     				m_xmf3Gravity;
	float           			m_fMaxVelocityXZ;
	float           			m_fMaxVelocityY;
	float           			m_fFriction;

	LPVOID						m_pCameraUpdatedContext;

public:

	XMFLOAT3					m_xmf3Velocity;
	float           			m_fPitch;
	float           			m_fYaw;
	float           			m_fRoll;
	float						m_RotateAngle=0.0;
	CCamera						*m_pCamera = NULL;
	LPVOID						m_pPlayerUpdatedContext;
	XMFLOAT3					m_xmf3Position;
	int							m_iItemVal = 0;
	
	CPlayer();
	virtual ~CPlayer();
	virtual XMFLOAT3 GetPosition() { return(m_xmf3Position); }
	virtual XMFLOAT3 GetLookVector() { return(m_xmf3Look); }
	virtual XMFLOAT3 GetUpVector() { return(m_xmf3Up); }
	virtual XMFLOAT3 GetRightVector() { return(m_xmf3Right); }
	

	void SetTerrain(LPVOID pPlayerUpdatedContext);
	void SetFriction(float fFriction) { m_fFriction = fFriction; }
	void SetGravity(const XMFLOAT3& xmf3Gravity) { m_xmf3Gravity = xmf3Gravity; }
	void SetMaxVelocityXZ(float fMaxVelocity) { m_fMaxVelocityXZ = fMaxVelocity; }
	void SetMaxVelocityY(float fMaxVelocity) { m_fMaxVelocityY = fMaxVelocity; }
	void SetVelocity(const XMFLOAT3& xmf3Velocity) { m_xmf3Velocity = xmf3Velocity; }
	void SetPosition(const XMFLOAT3& xmf3Position) { Move(XMFLOAT3(xmf3Position.x - m_xmf3Position.x, xmf3Position.y - m_xmf3Position.y, xmf3Position.z - m_xmf3Position.z), false); }
	const XMFLOAT3& GetVelocity() const { return(m_xmf3Velocity); }
	float GetYaw() const { return(m_fYaw); }
	float GetPitch() const { return(m_fPitch); }
	float GetRoll() const { return(m_fRoll); }
	
	CCamera *GetCamera() { return(m_pCamera); }
	void SetCamera(CCamera *pCamera) { m_pCamera = pCamera; }

	void Move(ULONG nDirection, float fDistance, bool bVelocity = false);
	void Move(const XMFLOAT3& xmf3Shift, bool bVelocity = false);
	void Move(float fxOffset = 0.0f, float fyOffset = 0.0f, float fzOffset = 0.0f);
	void Rotate(float x, float y, float z);

	void Update(float fTimeElapsed);
	virtual void OnPlayerUpdateCallback(float fTimeElapsed);
	
	void SetPlayerUpdatedContext(LPVOID pContext) { m_pPlayerUpdatedContext = pContext; }

	virtual void OnCameraUpdateCallback(float fTimeElapsed) {  }
	void SetCameraUpdatedContext(LPVOID pContext) { m_pCameraUpdatedContext = pContext; }

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);

	CCamera *OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode);

	virtual CCamera *ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed) { return(NULL); }
	virtual void OnPrepareRender();
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = NULL);
};

#define BULLETS					6

class CMyPlayer : public CPlayer
{
public:
	CMyPlayer(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature);
	virtual ~CMyPlayer();

public:
	float							m_fBulletEffectiveRange = 300.0f;
	CBulletObject*					pBulletObject = NULL;
	CTrapObject*					m_pTrapObject = NULL;
	CPlayerObject*					m_pPlayerObejct = NULL;
	CBulletObject* m_ppBullets[BULLETS];

public:
	float m_fPos = 0.0;
	bool m_bWheelAnimation = false;
	CGameObjcet* m_WheelBack_Left;
	CGameObjcet* m_WheelBack_Right;
	CGameObjcet* m_WheelFront_Left;
	CGameObjcet* m_WheelFront_Right;

	
private:
	virtual void OnInitialize();
	virtual void Animate(float fTimeElapsed, XMFLOAT4X4 *pxmf4x4Parent = NULL);
	virtual void OnPlayerUpdateCallback(float fTimeElapsed);

public:
	virtual CCamera *ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed);
	virtual void OnPrepareRender();
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

public:
	void MissileMode(CGameObjcet* pLockedObject);
	void TrapMode();
	void BoosterMode();
	bool m_bbsAct = false;
};


