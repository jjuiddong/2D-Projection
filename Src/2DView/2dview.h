//
// 2024-12-74, jjuiddong
// 2d view using parallel projection
//
#pragma once


class c2DView : public framework::cDockWindow
{
public:
	c2DView();
	virtual ~c2DView();

	bool Init(graphic::cRenderer& renderer);
	virtual void OnUpdate(const float deltaSeconds) override;
	virtual void OnRender(const float deltaSeconds) override;
	virtual void OnPreRender(const float deltaSeconds) override;
	virtual void OnResizeEnd(const framework::eDockResize::Enum type, const sRectf& rect) override;
	virtual void OnEventProc(const sf::Event& evt) override;
	virtual void OnResetDevice() override;
	void Clear();


protected:
	void RenderScene(graphic::cRenderer& renderer, const float deltaSeconds
		, const XMMATRIX& parentTm = graphic::XMIdentity);
	Vector3 GetPickPosOrignal(const int mousePosX, const int mousePosY);
	Vector3 GetPickPosReal(const int mousePosX, const int mousePosY);

	void UpdateLookAt();
	void OnWheelMove(const float delta, const POINT mousePt);
	void OnMouseMove(const POINT mousePt);
	void OnMouseDown(const sf::Mouse::Button& button, const POINT mousePt);
	void OnMouseUp(const sf::Mouse::Button& button, const POINT mousePt);


public:
	graphic::cRenderTarget m_renderTarget;
	graphic::cGridLine m_gridLine;
	graphic::cDbgAxis m_axis;
	graphic::cCamera2D m_camera2d; // parallal projection camera

	// MouseMove Variable
	POINT m_viewPos;
	sRectf m_viewRect; // detect mouse event area
	POINT m_mousePos; // window 2d mouse pos
	POINT m_mouseClickPos; // window 2d mouse pos
	Vector3 m_mousePickPos; // mouse cursor pos in ground picking
	bool m_mouseDown[3]; // Left, Right, Middle
	float m_rotateLen;
};
