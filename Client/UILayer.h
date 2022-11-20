#pragma once


struct TextBlock
{
    WCHAR                           m_pstrText[256];    // 내가 화면에 출력하고 싶은 문자열들을
    D2D1_RECT_F                     m_d2dLayoutRect;    // 내가 정한 사각형 영역에
    IDWriteTextFormat* m_pdwFormat;        // 이런 형태로 출력해달라
    ID2D1SolidColorBrush* m_pd2dTextBrush;
};

class UILayer
{
public:
    // 생성자에서 모든 Direct2D 관련처리를 하겠다
    UILayer(UINT nFrames, UINT nTextBlocks, ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, ID3D12Resource** ppd3dRenderTargets, UINT nWidth, UINT nHeight);

    // 인덱스에 대한 정보를 바꾸는 것(폰트,텍스트,사각형 ... )
    void UpdateTextOutputs(UINT nIndex, WCHAR* pstrUIText, D2D1_RECT_F* pd2dLayoutRect, IDWriteTextFormat* pdwFormat, ID2D1SolidColorBrush* pd2dTextBrush);

    void Render(UINT nFrame);
    void ReleaseResources();

    // 여러가지 브러쉬나 포맷을 리턴
    ID2D1SolidColorBrush* CreateBrush(D2D1::ColorF d2dColor);
    IDWriteTextFormat* CreateTextFormat(WCHAR* pszFontName, float fFontSize);

public:
    // 다이렉트2D Device를 생성한다
    void InitializeDevice(ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, ID3D12Resource** ppd3dRenderTargets);

    float                           m_fWidth = 0.0f;
    float                           m_fHeight = 0.0f;

    ID3D11DeviceContext* m_pd3d11DeviceContext = NULL;
    ID3D11On12Device* m_pd3d11On12Device = NULL;
    IDWriteFactory* m_pd2dWriteFactory = NULL;
    ID2D1Factory3* m_pd2dFactory = NULL;
    ID2D1Device2* m_pd2dDevice = NULL;
    ID2D1DeviceContext2* m_pd2dDeviceContext = NULL;

    UINT                            m_nRenderTargets = 0;
    ID3D11Resource** m_ppd3d11WrappedRenderTargets = NULL;
    ID2D1Bitmap1** m_ppd2dRenderTargets = NULL;

    // 텍스트 블럭을 배열형태로
    UINT                            m_nTextBlocks = 0;
    TextBlock* m_pTextBlocks = NULL;
};