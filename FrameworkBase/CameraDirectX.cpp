/*=============================================================================

カメラ(DirectX)[ CameraDirectX.cpp ]

-------------------------------------------------------------------------------
■  Author
	Ohno Takuya

■  Create
	2017/10/16
=============================================================================*/

/*-----------------------------------------------------------------------------
Include Files
-----------------------------------------------------------------------------*/
#include "Common.h"
#include "RendererDirectX.h"
#include "Game.h"
#include "InputDirectX.h"
#include "CameraDirectX.h"
#include "../CameraState.h"

/*-----------------------------------------------------------------------------
Macro
-----------------------------------------------------------------------------*/
#define CAMERA_NEAR ( 1.0f )	//	near
#define CAMERA_FAR ( 500.0f )	//	far
#define CAMERA_SPEED (2.0f)

/*-----------------------------------------------------------------------------
Static
-----------------------------------------------------------------------------*/
#ifdef _DEBUG
	bool CameraDirectX::m_DebugCameraFlag = true;
#endif


CameraDirectX::CameraDirectX()
	:m_PositionEye(D3DXVECTOR3(0.0f, 60.0f, -120.0f))
	,m_PositionAt(D3DXVECTOR3(0.0f, 0.0f, 0.0f))
	,m_VecUp(D3DXVECTOR3(0.0f, 1.0f, 0.0f))
	,m_FrontVector(D3DXVECTOR3(0.0f, 0.0f, 0.0f))
	,m_RightVector(D3DXVECTOR3(0.0f, 0.0f, 0.0f))
{
	m_State = new CameraStateNull;
}

void CameraDirectX::Uninitialize(void)
{
	SAFE_DELETE(m_State);
}

void CameraDirectX::Update(void)
{
	NormalizeCamera();

#ifdef _DEBUG
	if(m_DebugCameraFlag)
	{
		DebugCamera();
	}
#endif

	m_State->Update();

	SetProjection();
}

void CameraDirectX::SetProjection(void)
{
	//	デバイスの取得
	LPDIRECT3DDEVICE9 Device = RendererDirectX::GetDevice();

	//	ワールド座標変換
	//	ワールドマトリクスの作成
	D3DXMATRIX mtxWorld;

	D3DXMatrixIdentity(&mtxWorld);	//	行列を単位行列にする

	//	デバイスにワールド変換行列を設定
	Device->SetTransform(D3DTS_WORLD, &mtxWorld);

	//	ビュー座標変換
	D3DXMatrixLookAtLH(&m_MtxView, &m_PositionEye, &m_PositionAt, &m_VecUp);

	//	デバイスにビュー変換行列を設定
	Device->SetTransform(D3DTS_VIEW, &m_MtxView);

	//	プロジェクション変換

	//	パースペクティブ行列
	D3DXMatrixPerspectiveFovLH(&m_mtxProj,
		D3DX_PI / 3.0f,	//	画角
		(float)SCREEN_WIDTH / SCREEN_HEIGHT,	//	アスペクト比
		CAMERA_NEAR,	//	near	値を絶対0以下にしない
		CAMERA_FAR);	//	far
	Device->SetTransform(D3DTS_PROJECTION, &m_mtxProj);
}

void CameraDirectX::NormalizeCamera(void)
{
	m_FrontVector = m_PositionAt - m_PositionEye;

	D3DXVec3Normalize(&m_FrontVector, &m_FrontVector);
	D3DXVec3Cross(&m_RightVector, &m_VecUp, &m_FrontVector);	//	外積
	D3DXVec3Normalize(&m_RightVector, &m_RightVector);
}

/*-----------------------------------------------------------------------------
ビュー行列の逆行列取得
-----------------------------------------------------------------------------*/
const D3DXMATRIX CameraDirectX::GetInversedView(void)const
{
	D3DXMATRIX mtxViewInverse;

	//	転置行列
	D3DXMatrixTranspose(&mtxViewInverse, &m_MtxView);

	//	平行移動成分をカット
	mtxViewInverse._14 = 0.0f;
	mtxViewInverse._24 = 0.0f;
	mtxViewInverse._34 = 0.0f;

	return mtxViewInverse;
}

#ifdef _DEBUG
/*-----------------------------------------------------------------------------
デバッグ用の操作
-----------------------------------------------------------------------------*/
void CameraDirectX::DebugCamera(void)
{
	DirectXKeyboard* m_Keyboard = Game::GetInstance()->GetKeyboard();
	BOOL leftSift = m_Keyboard->GetKeyPress(DIK_LSHIFT);
	BOOL moveForward = m_Keyboard->GetKeyPress(DIK_W);
	BOOL moveBack = m_Keyboard->GetKeyPress(DIK_S);
	BOOL moveLeft = m_Keyboard->GetKeyPress(DIK_A);
	BOOL moveRight = m_Keyboard->GetKeyPress(DIK_D);
	BOOL up = m_Keyboard->GetKeyPress(DIK_Q);
	BOOL down = m_Keyboard->GetKeyPress(DIK_E);

	if(leftSift && moveForward)
	{
		m_PositionEye += CAMERA_SPEED * m_FrontVector;
		m_PositionAt += CAMERA_SPEED * m_FrontVector;
	}
	if(leftSift && moveBack)
	{
		m_PositionEye -= CAMERA_SPEED * m_FrontVector;
		m_PositionAt -= CAMERA_SPEED * m_FrontVector;
	}
	if(leftSift && moveLeft)
	{
		m_PositionEye -= CAMERA_SPEED * m_RightVector;
		m_PositionAt -= CAMERA_SPEED * m_RightVector;
	}
	if(leftSift && moveRight)
	{
		m_PositionEye += CAMERA_SPEED * m_RightVector;
		m_PositionAt += CAMERA_SPEED * m_RightVector;
	}

	if(leftSift && up)
	{
		m_PositionEye.y += CAMERA_SPEED;
		m_PositionAt.y += CAMERA_SPEED;
	}
	else if(leftSift && down)
	{
		m_PositionEye.y -= CAMERA_SPEED;
		m_PositionAt.y -= CAMERA_SPEED;
	}
}
#endif

void CameraDirectX::SetState(CameraState* state)
{
	SAFE_DELETE(m_State);

	m_State = state;
}

D3DXMATRIX CameraDirectX::GetViewProjectionMatrix(void)
{
	D3DXMATRIX matrix;

	D3DXMatrixIdentity(&matrix);
	D3DXMatrixMultiply(&matrix, &matrix, &m_MtxView);
	D3DXMatrixMultiply(&matrix, &matrix, &m_mtxProj);

	return matrix;
}