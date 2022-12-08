//-----------------------------------------------------------------------------
// File: CPlayer.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Player.h"
#include "Shader.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CPlayer

CPlayer::CPlayer()
{
	m_pCamera = NULL;

	m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Gravity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_fMaxVelocityXZ = 0.0f;
	m_fMaxVelocityY = 0.0f;
	m_fFriction = 0.0f;

	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;

	m_pPlayerUpdatedContext = NULL;
	m_pCameraUpdatedContext = NULL;
}

CPlayer::~CPlayer()
{
	ReleaseShaderVariables();

	if (m_pCamera) delete m_pCamera;
}

void CPlayer::SetTerrain(LPVOID pPlayerUpdatedContext)
{
	m_pPlayerUpdatedContext = pPlayerUpdatedContext;
}



void CPlayer::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pCamera) m_pCamera->CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CPlayer::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CPlayer::ReleaseShaderVariables()
{
	if (m_pCamera) m_pCamera->ReleaseShaderVariables();
}

void CPlayer::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
{
	if (dwDirection)
	{
		XMFLOAT3 xmf3Shift = XMFLOAT3(0.0, 0, 0);
		if (dwDirection & DIR_FORWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, fDistance * 0.35f);
		if (dwDirection & DIR_BACKWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -fDistance * 0.35f);
		if (dwDirection & DIR_RIGHT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, fDistance * 0.35f);
		if (dwDirection & DIR_LEFT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -fDistance * 0.35f);
		if (dwDirection & DIR_UP) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, fDistance * 0.35f);
		if (dwDirection & DIR_DOWN) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, -fDistance * 0.35f);

		Move(xmf3Shift, bUpdateVelocity);
	}
}

void CPlayer::Move(const XMFLOAT3& xmf3Shift, bool bUpdateVelocity)
{
	if (bUpdateVelocity)
	{
		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Shift);
	}
	else
	{

		m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
		m_pCamera->Move(xmf3Shift);
	}
}

void CPlayer::Rotate(float x, float y, float z)
{
	DWORD nCurrentCameraMode = m_pCamera->GetMode();
	
	if (nCurrentCameraMode == THIRD_PERSON_CAMERA)
	{
		if (y != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
		m_pCamera->Rotate(x, y, z);

	}
	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);
}

void CPlayer::Update(float fTimeElapsed)
{

	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, m_xmf3Gravity);
	float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);
	float fMaxVelocityXZ = m_fMaxVelocityXZ;
	if (fLength > m_fMaxVelocityXZ)
	{
		m_xmf3Velocity.x *= (fMaxVelocityXZ / fLength);
		m_xmf3Velocity.z *= (fMaxVelocityXZ / fLength);
	}
	float fMaxVelocityY = m_fMaxVelocityY;
	fLength = sqrtf(m_xmf3Velocity.y * m_xmf3Velocity.y);
	if (fLength > m_fMaxVelocityY) m_xmf3Velocity.y *= (fMaxVelocityY / fLength);

	XMFLOAT3 xmf3Velocity = Vector3::ScalarProduct(m_xmf3Velocity, fTimeElapsed, true);
	Move(xmf3Velocity, false);

	if (m_pPlayerUpdatedContext) OnPlayerUpdateCallback(fTimeElapsed);

	DWORD nCurrentCameraMode = m_pCamera->GetMode();
	if (nCurrentCameraMode == THIRD_PERSON_CAMERA) m_pCamera->Update(m_xmf3Position, fTimeElapsed);
	if (m_pCameraUpdatedContext) OnCameraUpdateCallback(fTimeElapsed);
	if (nCurrentCameraMode == THIRD_PERSON_CAMERA) m_pCamera->SetLookAt(m_xmf3Position);

	m_pCamera->RegenerateViewMatrix();

	fLength = Vector3::Length(m_xmf3Velocity);
	float fDeceleration = (m_fFriction * fTimeElapsed);
	if (fDeceleration > fLength) fDeceleration = fLength;
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Velocity, -fDeceleration, true));
}

void CPlayer::OnPlayerUpdateCallback(float fTimeElapsed)
{
}

void CMyPlayer::OnPlayerUpdateCallback(float fTimeElapsed)
{
	XMFLOAT3 xmf3PlayerPosition = GetPosition();
	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)m_pPlayerUpdatedContext;
	/*지형에서 플레이어의 현재 위치 (x, z)의 지형 높이(y)를 구한다. 그리고 플레이어 메쉬의 높이가 12이고 플레이어의
	중심이 직육면체의 가운데이므로 y 값에 메쉬의 높이의 절반을 더하면 플레이어의 위치가 된다.*/

	float fHeight = pTerrain->GetHeight(xmf3PlayerPosition.x, xmf3PlayerPosition.z) + 2.0f;
	/*플레이어의 위치 벡터의 y-값이 음수이면(예를 들어, 중력이 적용되는 경우) 플레이어의 위치 벡터의 y-값이 점점
	작아지게 된다. 이때 플레이어의 현재 위치 벡터의 y 값이 지형의 높이(실제로 지형의 높이 + 6)보다 작으면 플레이어
	의 일부가 지형 아래에 있게 된다. 이러한 경우를 방지하려면 플레이어의 속도 벡터의 y 값을 0으로 만들고 플레이어
	의 위치 벡터의 y-값을 지형의 높이(실제로 지형의 높이 + 6)로 설정한다. 그러면 플레이어는 항상 지형 위에 있게 된다.*/
	if (xmf3PlayerPosition.y < fHeight)
	{
		XMFLOAT3 xmf3PlayerVelocity = GetVelocity();
		xmf3PlayerVelocity.y = 0.0f;
		SetVelocity(xmf3PlayerVelocity);
		xmf3PlayerPosition.y = fHeight;
		SetPosition(xmf3PlayerPosition);
	}


}

CCamera* CPlayer::OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode)
{
	CCamera* pNewCamera = NULL;
	switch (nNewCameraMode)
	{
	case THIRD_PERSON_CAMERA:
		pNewCamera = new CThirdPersonCamera(m_pCamera);
		break;
	}
	
	if ((nNewCameraMode == THIRD_PERSON_CAMERA) && m_pCamera)
	{
		m_xmf3Right = m_pCamera->GetRightVector();
		m_xmf3Up = m_pCamera->GetUpVector();
		m_xmf3Look = m_pCamera->GetLookVector();
	}

	if (pNewCamera)
	{
		pNewCamera->SetMode(nNewCameraMode);
		pNewCamera->SetPlayer(this);
	}

	if (m_pCamera) delete m_pCamera;

	return(pNewCamera);
}

void CPlayer::OnPrepareRender()
{
	m_xmf4x4Transform._11 = m_xmf3Right.x; m_xmf4x4Transform._12 = m_xmf3Right.y; m_xmf4x4Transform._13 = m_xmf3Right.z;
	m_xmf4x4Transform._21 = m_xmf3Up.x; m_xmf4x4Transform._22 = m_xmf3Up.y; m_xmf4x4Transform._23 = m_xmf3Up.z;
	m_xmf4x4Transform._31 = m_xmf3Look.x; m_xmf4x4Transform._32 = m_xmf3Look.y; m_xmf4x4Transform._33 = m_xmf3Look.z;
	m_xmf4x4Transform._41 = m_xmf3Position.x; m_xmf4x4Transform._42 = m_xmf3Position.y; m_xmf4x4Transform._43 = m_xmf3Position.z;

	UpdateTransform(NULL);
}

void CPlayer::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	DWORD nCameraMode = (pCamera) ? pCamera->GetMode() : 0x00;
	if (nCameraMode == THIRD_PERSON_CAMERA) CGameObjcet::Render(pd3dCommandList, pCamera);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 

CMyPlayer::CMyPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{

	m_pCamera = ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);


	CGameObjcet* pBulletMesh = CGameObjcet::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Roket.bin");

	for (int i = 0; i < BULLETS; i++)
	{
		pBulletObject = new CBulletObject(m_fBulletEffectiveRange);
		pBulletObject->SetChild(pBulletMesh, true);
		pBulletObject->SetRotationSpeed(40.0f);
		pBulletObject->SetMovingSpeed(300.0f);
		pBulletObject->SetActive(false);
		m_ppBullets[i] = pBulletObject;

	}

	CGameObjcet* pTrapModel = CGameObjcet::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Roket.bin");

	m_pTrapObject = new CTrapObject();
	m_pTrapObject->SetChild(pTrapModel, true);
	m_pTrapObject->SetRotationSpeed(90.0f);
	m_pTrapObject->SetMovingSpeed(20.0f);
	m_pTrapObject->SetActive(false);


	
	CGameObjcet* pGameObject = CGameObjcet::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/race.bin");

	SetChild(pGameObject, true);
	OnInitialize();
	pGameObject->Rotate(0.0f, 0.0f, 0.0f);
	pGameObject->SetScale(6.0f, 6.0, 6.0);
	pGameObject->SetPosition(0.0, -1.0, 0.0);
	m_pPlayerObejct = (CPlayerObject*)pGameObject;

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

CMyPlayer::~CMyPlayer()
{
	for (int i = 0; i < BULLETS; i++) if (m_ppBullets[i]) delete m_ppBullets[i];
	if (m_pTrapObject) delete m_pTrapObject;
}

void CMyPlayer::OnInitialize()
{
	m_WheelBack_Left = FindFrame("wheel_backLeft");
	m_WheelBack_Right = FindFrame("wheel_backRight");
	m_WheelFront_Left = FindFrame("wheel_frontLeft");
	m_WheelFront_Right = FindFrame("wheel_frontRight");
	m_fPos = m_WheelBack_Right->m_xmf4x4Transform._42;
}

void CMyPlayer::Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent)
{
	if (m_WheelBack_Left)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationX(XMConvertToRadians(360.0f * 4.0) * fTimeElapsed);
		m_WheelBack_Left->m_xmf4x4Transform = Matrix4x4::Multiply(xmmtxRotate, m_WheelBack_Left->m_xmf4x4Transform);
	}
	if (m_WheelBack_Right)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationX(XMConvertToRadians(360.0f * 4.0) * fTimeElapsed);
		m_WheelBack_Right->m_xmf4x4Transform = Matrix4x4::Multiply(xmmtxRotate, m_WheelBack_Right->m_xmf4x4Transform);
	}
	if (m_WheelFront_Left)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationX(XMConvertToRadians(360.0f * 4.0) * fTimeElapsed);
		m_WheelFront_Left->m_xmf4x4Transform = Matrix4x4::Multiply(xmmtxRotate, m_WheelFront_Left->m_xmf4x4Transform);
	}
	if (m_WheelFront_Right)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationX(XMConvertToRadians(360.0f * 4.0) * fTimeElapsed);
		m_WheelFront_Right->m_xmf4x4Transform = Matrix4x4::Multiply(xmmtxRotate, m_WheelFront_Right->m_xmf4x4Transform);
	}

	if (m_bWheelAnimation == false)
	{
		m_WheelBack_Left->m_xmf4x4Transform._42 += 0.008f;m_WheelBack_Right->m_xmf4x4Transform._42 += 0.008f;m_WheelFront_Left->m_xmf4x4Transform._42 += 0.008f;m_WheelFront_Right->m_xmf4x4Transform._42 += 0.008f;
		if (m_WheelBack_Right->m_xmf4x4Transform._42 > m_fPos + 0.03f) m_bWheelAnimation = true;
	}
	if (m_bWheelAnimation == true)
	{
		m_WheelBack_Left->m_xmf4x4Transform._42 -= 0.008f;m_WheelBack_Right->m_xmf4x4Transform._42 -= 0.008f;m_WheelFront_Left->m_xmf4x4Transform._42 -= 0.008f;m_WheelFront_Right->m_xmf4x4Transform._42 -= 0.008f;
		if (m_WheelBack_Right->m_xmf4x4Transform._42 < m_fPos - 0.01f) m_bWheelAnimation = false;
	}

	for (int i = 0; i < BULLETS; i++)
	{
		if (m_ppBullets[i]->m_bActive) {
			m_ppBullets[i]->Animate(fTimeElapsed);
			m_ppBullets[i]->Rotate(0.0, 20.0, 0.0);
		}
	}

	if (m_pTrapObject->m_bActive) 
	{
		m_pTrapObject->Animate(fTimeElapsed);
	}

	if (m_bbsAct == true)
	{
		m_pCamera->m_xmf3Position.z -= 1.5f;
	}
	

	CPlayer::Animate(fTimeElapsed, pxmf4x4Parent);
	m_Boobb = BoundingOrientedBox(GetPosition(), XMFLOAT3(15.0, 10.0, 30.0), XMFLOAT4(0.0, 0.0, 0.0, 1.0));
}

void CMyPlayer::OnPrepareRender()
{
	CPlayer::OnPrepareRender();
}

void CMyPlayer::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	OnPrepareRender();
	CGameObjcet::Render(pd3dCommandList, pCamera);
	for (int i = 0; i < BULLETS; i++) if (m_ppBullets[i]->m_bActive) m_ppBullets[i]->Render(pd3dCommandList, pCamera);
	if (m_pTrapObject->m_bActive) m_pTrapObject->Render(pd3dCommandList, pCamera);
}


void CMyPlayer::MissileMode(CGameObjcet* pLockedObject)
{

	CBulletObject* pBulletObject = NULL;
	for (int i = 0; i < BULLETS; i++)
	{
		if (!m_ppBullets[i]->m_bActive)
		{
			pBulletObject = m_ppBullets[i];
			break;
		}
	}

	XMFLOAT3 PlayerPos = GetLookVector();
	XMFLOAT3 CameraPos = m_pCamera->GetLookVector();
	XMFLOAT3 TotalLookVector = Vector3::Normalize(Vector3::Add(PlayerPos, CameraPos));

	if (pBulletObject)
	{

		XMFLOAT3 xmf3Position = GetPosition();
		XMFLOAT3 xmf3Direction = TotalLookVector;
		XMFLOAT3 xmf3FirePosition = Vector3::Add(xmf3Position, Vector3::ScalarProduct(xmf3Direction, 6.0f, true));
		pBulletObject->m_xmf4x4Transform = m_xmf4x4World;
		pBulletObject->SetFirePosition(xmf3FirePosition);
		pBulletObject->SetMovingDirection(GetLookVector());
		pBulletObject->Rotate(90.0f, 0.0, 0.0);
		pBulletObject->SetScale(700.0, 200.0, 700.0);
		pBulletObject->SetActive(true);
	}
}

void CMyPlayer::TrapMode()
{
	XMFLOAT3 PlayerPos = GetLookVector();
	XMFLOAT3 CameraPos = m_pCamera->GetLookVector();
	XMFLOAT3 TotalLookVector = Vector3::Normalize(Vector3::Add(PlayerPos, CameraPos));
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Direction = TotalLookVector;
	XMFLOAT3 xmf3FirePosition = Vector3::Add(xmf3Position, Vector3::ScalarProduct(xmf3Direction, 6.0f, true));
	m_pTrapObject->m_xmf4x4Transform = m_xmf4x4World;
	m_pTrapObject->SetCreateTrapPosition(xmf3FirePosition);
	m_pTrapObject->SetMovingDirection(GetLookVector());
	m_pTrapObject->Rotate(0.0f, 0.0, 0.0);
	m_pTrapObject->SetScale(8.3,8.3,8.3);
	m_pTrapObject->SetActive(true);
	
}

void CMyPlayer::BoosterMode()
{
	m_bbsAct = true;
}

CCamera* CMyPlayer::ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed)
{
	DWORD nCurrentCameraMode = (m_pCamera) ? m_pCamera->GetMode() : 0x00;
	if (nCurrentCameraMode == nNewCameraMode) return(m_pCamera);
	switch (nNewCameraMode)
	{
	case THIRD_PERSON_CAMERA:
		SetFriction(50.5f);
		SetGravity(XMFLOAT3(0.0f, -100.0f, 0.0f));
		SetMaxVelocityXZ(20.5f);
		SetMaxVelocityY(2000.0f);
		m_pCamera = OnChangeCamera(THIRD_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.8f);
		m_pCamera->SetOffset(XMFLOAT3(-0.0f, 20.0f, -65.0f));
		m_pCamera->SetPosition(Vector3::Add(m_xmf3Position, m_pCamera->GetOffset()));
		m_pCamera->GenerateProjectionMatrix(1.01f, 6000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	default:
		break;
	}


	Update(fTimeElapsed);

	return(m_pCamera);
}

