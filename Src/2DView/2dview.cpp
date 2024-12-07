
#include "stdafx.h"
#include "2dview.h"

using namespace graphic;
using namespace framework;


c2DView::c2DView()
	: framework::cDockWindow("2DView")
	, m_camera2d("2d")
{
}

c2DView::~c2DView()
{
	Clear();
}


// initialize
bool c2DView::Init(graphic::cRenderer& renderer)
{
	const sRectf viewRect = GetWindowSizeAvailible();
	const Vector3 eyePos2D(0, 30, 0);
	const Vector3 upVector2D(0, 0, 1.f);
	m_camera2d.m_zoom = 3.f;
	m_camera2d.m_minZoom = 1.0f;
	m_camera2d.m_maxZoom = 400.f;
	m_camera2d.SetCamera(eyePos2D * m_camera2d.m_zoom, upVector2D);
	m_camera2d.SetProjectionOrthogonal(viewRect.Width(), viewRect.Height(), 1, 100000.f);
	{
		const Vector3 _min(0, 0, 0);
		const Vector3 _max(1200, 10000, 960);
		Vector3 center = (_min + _max) * 0.5f;
		center.y = 0;
		const float radius = _min.Distance(center);
		m_camera2d.m_boundingType = graphic::cCamera::eBoundingType::BOX;
		m_camera2d.m_boundingBox.SetBoundingBox(Vector3(600, 0, 480), Vector3(600, 20000, 480), Quaternion());
	}

	GetMainLight().Init(graphic::cLight::LIGHT_DIRECTIONAL);
	GetMainLight().SetDirection(Vector3(-1, -2, -1.3f).Normal());

	sf::Vector2u size((uint)m_rect.Width() - 15, (uint)m_rect.Height() - 50);
	cViewport vp = renderer.m_viewPort;
	vp.m_vp.Width = (float)size.x;
	vp.m_vp.Height = (float)size.y;
	m_renderTarget.Create(renderer, vp, DXGI_FORMAT_R8G8B8A8_UNORM, true, true
		, DXGI_FORMAT_D24_UNORM_S8_UINT);

	m_gridLine.Create(renderer, 1000, 1000, 1.f, 1.f
		, (eVertexType::POSITION | eVertexType::COLOR));

	m_axis.Create(renderer);
	cBoundingBox bbox3(Vector3(0, 0, 0), Vector3(10, 0, 10), Quaternion());
	m_axis.SetAxis(0.025f, bbox3, false);

	return true;
}


void c2DView::OnUpdate(const float deltaSeconds)
{
}


void c2DView::OnRender(const float deltaSeconds)
{
	ImVec2 pos = ImGui::GetCursorScreenPos();
	m_viewPos = { (int)(pos.x), (int)(pos.y) };
	m_viewRect = { pos.x + 5, pos.y, pos.x + m_rect.Width() - 30, pos.y + m_rect.Height() - 42 };

	// HUD
	bool isOpen = true;
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration
		| ImGuiWindowFlags_NoBackground
		;

	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
	ImGui::Image(m_renderTarget.m_resolvedSRV, ImVec2(m_rect.Width() - 15, m_rect.Height() - 42));

	// Render Information
	ImGui::SetNextWindowPos(ImVec2(pos.x, pos.y));
	ImGui::SetNextWindowBgAlpha(0.f);
	ImGui::SetNextWindowSize(ImVec2(min(m_viewRect.Width(), 350.f), m_viewRect.Height()));
	if (ImGui::Begin("2DView", &isOpen, flags))
	{
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	}
	ImGui::End();
	ImGui::PopStyleColor();
}


void c2DView::OnPreRender(const float deltaSeconds)
{
	cRenderer& renderer = GetRenderer();

	cAutoCam cam(&m_camera2d);
	const XMMATRIX parentTm = GetMainCamera().GetZoomMatrix().GetMatrixXM();

	renderer.UnbindShaderAll();
	renderer.UnbindTextureAll();
	GetMainCamera().Bind(renderer);
	GetMainLight().Bind(renderer);

	if (m_renderTarget.Begin(renderer))
	{
		renderer.UnbindShaderAll();
		renderer.UnbindTextureAll();
		GetMainCamera().Bind(renderer);
		GetMainLight().Bind(renderer);
		renderer.GetDevContext()->RSSetState(renderer.m_renderState.CullCounterClockwise());
		RenderScene(renderer, deltaSeconds, parentTm);
	}
	m_renderTarget.End(renderer);
	renderer.UnbindTextureAll();
}


// render shared data
void c2DView::RenderScene(graphic::cRenderer& renderer, const float deltaSeconds
	, const XMMATRIX& parentTm //= XMIdentity
)
{
	m_gridLine.Render(renderer, parentTm);
	m_axis.Render(renderer, parentTm);

	renderer.m_dbgSphere.m_transform.scale = Vector3::Ones * 0.1f;
	renderer.m_dbgSphere.m_transform.pos = m_mousePickPos;
	renderer.m_dbgSphere.Render(renderer, parentTm);
}


// return picking pos with original object space position
Vector3 c2DView::GetPickPosOrignal(const int mousePosX, const int mousePosY)
{
	Vector3 reVal;
	const Ray ray = m_camera2d.GetRay(mousePosX, mousePosY);
	reVal = ray.orig / m_camera2d.m_zoom;
	reVal.y = 0;
	return reVal;
}


// return picking pos with camera zoomed object space position
Vector3 c2DView::GetPickPosReal(const int mousePosX, const int mousePosY)
{
	Vector3 reVal;
	const Ray ray = m_camera2d.GetRay(mousePosX, mousePosY);
	reVal = ray.orig;
	reVal.y = 0;
	return reVal;
}


void c2DView::OnResizeEnd(const framework::eDockResize::Enum type, const sRectf& rect)
{
	if (type == eDockResize::DOCK_WINDOW)
		m_owner->RequestResetDeviceNextFrame();
}


void c2DView::UpdateLookAt()
{
	GetMainCamera().MoveCancel();

	const float centerX = GetMainCamera().m_width / 2;
	const float centerY = GetMainCamera().m_height / 2;
	const Ray ray = GetMainCamera().GetRay((int)centerX, (int)centerY);
	const Plane groundPlane(Vector3(0, 1, 0), 0);
	const float distance = groundPlane.Collision(ray.dir);
	if (distance < -0.2f)
	{
		GetMainCamera().m_lookAt = groundPlane.Pick(ray.orig, ray.dir);
	}
	else
	{ // horizontal viewing
		const Vector3 lookAt = GetMainCamera().m_eyePos + GetMainCamera().GetDirection() * 5.f;
		GetMainCamera().m_lookAt = lookAt;
	}

	GetMainCamera().UpdateViewMatrix();
}


void c2DView::OnWheelMove(const float delta, const POINT mousePt)
{
	UpdateLookAt();

	cCamera2D& cam2D = m_camera2d;

	const Ray ray = cam2D.GetRay(mousePt.x, mousePt.y);
	const Vector3 pickPos = ray.orig;
	const int hw = (int)(cam2D.m_width / 2.f);
	const int hh = (int)(cam2D.m_height / 2.f);

	Vector3 offset(float(mousePt.x - hw), 0, float(hh - mousePt.y));
	Quaternion q;
	q.SetRotationArc(Vector3(0, 0, 1), cam2D.m_up);
	Matrix44 mRot = q.GetMatrix();
	offset *= mRot;

	const float oldZoom = cam2D.m_zoom;
	cam2D.Zoom((delta > 0) ? 1.1f : 0.9f);
	const float curZoom = cam2D.m_zoom;
	const float zoom = curZoom / oldZoom;
	const Vector3 camPos = (pickPos * zoom) - offset;
	cam2D.SetEyePos(camPos);
}


// Handling Mouse Move Event
void c2DView::OnMouseMove(const POINT mousePt)
{
	const POINT delta = { mousePt.x - m_mousePos.x, mousePt.y - m_mousePos.y };
	m_mousePos = mousePt;

	if (m_mouseDown[0])
	{
		const float z = m_camera2d.m_zoom;
		GetMainCamera().MoveUp(delta.y * min(1.f, (1000.f / z) * 0.4f));
		GetMainCamera().MoveRight(-delta.x * min(1.f, (1000.f / z) * 0.4f));
	}
	else if (m_mouseDown[1])
	{
	}
	else if (m_mouseDown[2])
	{
	}
}


// Handling Mouse Button Down Event
void c2DView::OnMouseDown(const sf::Mouse::Button& button, const POINT mousePt)
{
	m_mousePos = mousePt;
	m_mouseClickPos = mousePt;
	UpdateLookAt();
	SetCapture();

	const Ray ray = GetMainCamera().GetRay(mousePt.x, mousePt.y);
	m_rotateLen = ray.orig.y * 0.9f;// (target - ray.orig).Length();
	m_mousePickPos = GetPickPosOrignal(mousePt.x, mousePt.y);

	switch (button)
	{
	case sf::Mouse::Left:
		m_mouseDown[0] = true;
		break;
	case sf::Mouse::Right:
		m_mouseDown[1] = true;
		break;
	case sf::Mouse::Middle:
		m_mouseDown[2] = true;
		break;
	}
}


void c2DView::OnMouseUp(const sf::Mouse::Button& button, const POINT mousePt)
{
	const POINT delta = { mousePt.x - m_mousePos.x, mousePt.y - m_mousePos.y };
	m_mousePos = mousePt;
	ReleaseCapture();

	switch (button)
	{
	case sf::Mouse::Left:
		m_mouseDown[0] = false;
		break;
	case sf::Mouse::Right:
	{
		m_mouseDown[1] = false;
		const int dx = m_mouseClickPos.x - mousePt.x;
		const int dy = m_mouseClickPos.y - mousePt.y;
		if (sqrt(dx * dx + dy * dy) > 10)
			break; // move long distance, do not show popup menu
	}
	break;
	case sf::Mouse::Middle:
		m_mouseDown[2] = false;
		break;
	}
}


void c2DView::OnEventProc(const sf::Event& evt)
{
	ImGuiIO& io = ImGui::GetIO();
	switch (evt.type)
	{
	case sf::Event::KeyPressed:
		break;

	case sf::Event::MouseMoved:
	{
		cAutoCam cam(&m_camera2d);

		POINT curPos;
		GetCursorPos(&curPos); // sf::event mouse position has noise so we use GetCursorPos() function
		ScreenToClient(m_owner->getSystemHandle(), &curPos);
		POINT pos = { curPos.x - m_viewPos.x, curPos.y - m_viewPos.y };
		OnMouseMove(pos);
	}
	break;

	case sf::Event::MouseButtonPressed:
	case sf::Event::MouseButtonReleased:
	{
		cAutoCam cam(&m_camera2d);

		POINT curPos;
		GetCursorPos(&curPos); // sf::event mouse position has noise so we use GetCursorPos() function
		ScreenToClient(m_owner->getSystemHandle(), &curPos);
		const POINT pos = { curPos.x - m_viewPos.x, curPos.y - m_viewPos.y };
		const sRectf viewRect = GetWindowSizeAvailible(true);

		if (sf::Event::MouseButtonPressed == evt.type)
		{
			if (viewRect.IsIn((float)pos.x, (float)pos.y))
				OnMouseDown(evt.mouseButton.button, pos);
		}
		else
		{
			if (viewRect.IsIn((float)pos.x, (float)pos.y)
				|| (this == GetCapture()))
				OnMouseUp(evt.mouseButton.button, pos);
		}
	}
	break;

	case sf::Event::MouseWheelScrolled:
	{
		cAutoCam cam(&m_camera2d);

		POINT curPos;
		GetCursorPos(&curPos); // sf::event mouse position has noise so we use GetCursorPos() function
		ScreenToClient(m_owner->getSystemHandle(), &curPos);
		const POINT pos = { curPos.x - m_viewPos.x, curPos.y - m_viewPos.y };
		OnWheelMove(evt.mouseWheelScroll.delta, pos);
	}
	break;
	}
}


void c2DView::OnResetDevice()
{
	cRenderer& renderer = GetRenderer();

	// update viewport
	sRectf viewRect = { 0, 0, m_rect.Width() - 15, m_rect.Height() - 50 };
	m_camera2d.SetViewPort(viewRect.Width(), viewRect.Height());

	cViewport vp = GetRenderer().m_viewPort;
	vp.m_vp.Width = viewRect.Width();
	vp.m_vp.Height = viewRect.Height();
	m_renderTarget.Create(renderer, vp, DXGI_FORMAT_R8G8B8A8_UNORM, true, true, DXGI_FORMAT_D24_UNORM_S8_UINT);
}


// clear
void c2DView::Clear()
{
}

