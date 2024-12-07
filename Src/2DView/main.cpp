//
// 2024-12-07, jjuiddong
// 2d projection project
//
#include "stdafx.h"
#include "2dview.h"

using namespace graphic;
using namespace framework;
class cApp : public framework::cGameMain2
{
public:
	cApp();
	virtual ~cApp();
	virtual bool OnInit() override;
	virtual void OnEventProc(const sf::Event& evt) override;
};
INIT_FRAMEWORK3(cApp);


cApp::cApp()
{
	m_windowName = L"2D Projection";
	m_isLazyMode = true;
	//const RECT r = { 0, 0, 1024, 768 };
	//const RECT r = { 0, 0, 1224, 768 };
	//const RECT r = { 0, 0, 1424, 768 };
	const RECT r = { 0, 0, 1280, 960 };
	m_windowRect = r;
	graphic::cResourceManager::Get()->SetMediaDirectory("./media/");
}

cApp::~cApp()
{
}


bool cApp::OnInit()
{
	c2DView* view = new c2DView();
	view->Create(eDockState::DOCKWINDOW, eDockSlot::TAB, this, NULL);
	view->Init(m_renderer);

	m_gui.SetContext();
	m_gui.SetStyleColorsDark();
	return true;
}


void cApp::OnEventProc(const sf::Event& evt)
{
	if (sf::Event::KeyPressed == evt.type)
		if (sf::Keyboard::Escape == evt.key.cmd)
			close();
}
