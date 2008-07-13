/**
* @file Lobby.cpp
* @author Nat Ryall
* @date 22/05/2008
* @brief The online lobby screen implementation.
*
* Copyright � SAPIAN
*/

//##############################################################################
//
//                                   INCLUDE
//
//##############################################################################

// Global.
#include <Global.h>

// Local.
#include <Lobby.h>

//##############################################################################

//##############################################################################
//
//                                LOBBY SCREEN
//
//##############################################################################

// =============================================================================
// Nat Ryall                                                         22-May-2008
// =============================================================================
CLobbyScreen::CLobbyScreen() : CScreen(ScreenIndex_LobbyScreen),
	m_iState(LobbyState_None),
	m_bPublic(true),
	m_pSession(NULL)
{
	_GLOBAL.pLobby = this;
}

// =============================================================================
// Nat Ryall                                                         22-May-2008
// =============================================================================
CLobbyScreen::~CLobbyScreen()
{
	_GLOBAL.pLobby = NULL;
}

// =============================================================================
// Nat Ryall                                                         09-Jun-2008
// =============================================================================
void CLobbyScreen::Load()
{
	// Load interfaces.
	m_pJoinInterface = new CJoinInterface();
	m_pStatusBox = new CStatusBox();

	m_pJoinInterface->m_pJoinButton->SetClickCallback(xbind(this, &CLobbyScreen::OnJoinClicked));

	// Load fonts.
	m_pPeerFont = new CFont(_FONT("Lobby-Peer"));

	// Initialise components.
	m_pJoinInterface->m_pAddressBox->SetText("127.0.0.1");

	// Session list.
	for (xint iA = 0; iA < MATCH_SESSION_LIMIT; ++iA)
		m_pSessionBoxes[iA] = NULL;
}

// =============================================================================
// Nat Ryall                                                         09-Jun-2008
// =============================================================================
void CLobbyScreen::Unload()
{
	delete m_pJoinInterface;
	delete m_pStatusBox;

	delete m_pPeerFont;

	DeleteSessionBoxes();
}

// =============================================================================
// Nat Ryall                                                         17-Jun-2008
// =============================================================================
void CLobbyScreen::QuitCheck()
{
	if (_HGE->Input_KeyUp(HGEK_ESCAPE))
	{
		switch (m_iState)
		{
		case LobbyState_List:
		case LobbyState_Join:
			{
				SetState(LobbyState_None);
				ScreenManager::Pop();
			}
			break;

		case LobbyState_Lobby:
			{
				if (m_bPublic && Network.m_bHosting)
				{
					Match.CloseSession(m_pSession, xbind(this, &CLobbyScreen::OnCloseSessionCompleted));
					SetState(LobbyState_Closing);
				}
				else
					CloseLobby();
			}
			break;
		}
	}
}

// =============================================================================
// Nat Ryall                                                         09-Jun-2008
// =============================================================================
void CLobbyScreen::Update()
{
	QuitCheck();
	UpdateParent();

	switch (m_iState)
	{
	case LobbyState_Lobby:
		UpdateLobby();
		break;
	}
}

// =============================================================================
// Nat Ryall                                                         17-Jun-2008
// =============================================================================
void CLobbyScreen::UpdateLobby()
{
	if (!Match.IsBusy())
	{
		if (Network.m_bHosting)
		{
			if (m_xPingTimer.IsExpired())
			{
				Match.PingSession(m_pSession, NULL);
				m_xPingTimer.ExpireAfter(MATCH_PING_INTERVAL);
			}
		}
	}
}

// =============================================================================
// Nat Ryall                                                         09-Jun-2008
// =============================================================================
void CLobbyScreen::Render()
{
	RenderParent();

	switch (m_iState)
	{
	case LobbyState_List:
		RenderList();
		break;

	case LobbyState_Lobby:
		RenderLobby();
		break;
	}
}

// =============================================================================
// Nat Ryall                                                         09-Jul-2008
// =============================================================================
void CLobbyScreen::RenderList()
{	
}

// =============================================================================
// Nat Ryall                                                         17-Jun-2008
// =============================================================================
void CLobbyScreen::RenderLobby()
{
	xint iPeerOffset = 0;

	XEN_LIST_FOREACH(t_NetworkPeerList, ppPeer, Network.m_lpPeers)
	{
		m_pPeerFont->Render(XFORMAT("Peer #%d", (*ppPeer)->m_iID), xpoint(50, 50 + iPeerOffset), HGETEXT_LEFT);
		iPeerOffset += 40;
	}
}

// =============================================================================
// Nat Ryall                                                         09-Jun-2008
// =============================================================================
void CLobbyScreen::Start(t_LobbyStartMode iStartMode)
{
	m_xPingTimer.Reset();
	
	m_bPublic = (iStartMode == LobbyStartMode_JoinPublic || iStartMode == LobbyStartMode_CreatePublic);
	m_pSession = NULL;

	switch (iStartMode)
	{
	case LobbyStartMode_JoinPublic:
		{
			Match.ListSessions(xbind(this, &CLobbyScreen::OnListSessionsCompleted));
			SetState(LobbyState_Searching);
		}
		break;

	case LobbyStartMode_JoinPrivate:
		{
			SetState(LobbyState_Join);
		}
		break;
	
	case LobbyStartMode_CreatePublic:
		{
			m_pSession = Match.CreateSession(4, xbind(this, &CLobbyScreen::OnCreateSessionCompleted));
			SetState(LobbyState_Creating);
		}
		break;

	case LobbyStartMode_CreatePrivate:
		{
			CreateLobby();
		}
		break;
	}

	m_iStartMode = iStartMode;
}

// =============================================================================
// Nat Ryall                                                         18-Jun-2008
// =============================================================================
void CLobbyScreen::Stop()
{
	Network.RequestStop();
}

// =============================================================================
// Nat Ryall                                                         15-Jun-2008
// =============================================================================
void CLobbyScreen::SetState(t_LobbyState iState)
{
	Interface.GetScreen()->DetachAll();

	switch (iState)
	{
	case LobbyState_List:
		{
			for (int iA = 0; iA < m_iSessionCount; ++iA)
				Interface.GetScreen()->Attach(m_pSessionBoxes[iA]);
		}
		break;

	case LobbyState_Searching:
		{
			m_pStatusBox->AttachElements();
			m_pStatusBox->m_pLabel->SetText(_LOCALE("Status_Searching"));
		}
		break;

	case LobbyState_Join:
		{
			if (Network.IsRunning())
				Network.RequestStop();

			m_pJoinInterface->AttachElements();
		}
		break;

	case LobbyState_Creating:
		{
			m_pStatusBox->AttachElements();
			m_pStatusBox->m_pLabel->SetText(_LOCALE("Status_Creating"));
		}
		break;

	case LobbyState_Connecting:
		{
			m_pStatusBox->AttachElements();
			m_pStatusBox->m_pLabel->SetText(_LOCALE("Status_Connecting"));
		}
		break;

	case LobbyState_Joining:
		{
			m_pStatusBox->AttachElements();
			m_pStatusBox->m_pLabel->SetText(_LOCALE("Status_Joining"));
		}
		break;

	case LobbyState_Leaving:
		{
			m_pStatusBox->AttachElements();
			m_pStatusBox->m_pLabel->SetText(_LOCALE("Status_Leaving"));
		}
		break;

	case LobbyState_Closing:
		{
			m_pStatusBox->AttachElements();
			m_pStatusBox->m_pLabel->SetText(_LOCALE("Status_Closing"));
		}
		break;
	}

	m_iState = iState;
}

// =============================================================================
// Nat Ryall                                                         10-Jul-2008
// =============================================================================
void CLobbyScreen::CreateLobby()
{
	Network.Reset();

	Network.m_xCallbacks.m_fpPeerJoined = xbind(this, &CLobbyScreen::OnPeerJoined);
	Network.m_xCallbacks.m_fpPeerLeaving = xbind(this, &CLobbyScreen::OnPeerLeaving);

	BindPacketCallbacks();

	Network.StartHost(16, _HOSTPORT);

	SetState(LobbyState_Lobby);
}

// =============================================================================
// Nat Ryall                                                         10-Jul-2008
// =============================================================================
void CLobbyScreen::JoinLobby(const xchar* pHostAddress)
{
	Network.Reset();

	Network.m_xCallbacks.m_fpConnectionCompleted = xbind(this, &CLobbyScreen::OnConnectionCompleted);
	Network.m_xCallbacks.m_fpConnectionLost = xbind(this, &CLobbyScreen::OnConnectionLost);
	Network.m_xCallbacks.m_fpPeerJoined = xbind(this, &CLobbyScreen::OnPeerJoined);
	Network.m_xCallbacks.m_fpPeerLeaving = xbind(this, &CLobbyScreen::OnPeerLeaving);

	Network.StartClient(pHostAddress, _HOSTPORT);

	BindPacketCallbacks();

	SetState(LobbyState_Connecting);
}

// =============================================================================
// Nat Ryall                                                         10-Jul-2008
// =============================================================================
void CLobbyScreen::CloseLobby()
{
	if (m_bPublic || Network.m_bHosting)
	{
		SetState(LobbyState_None);
		ScreenManager::Pop();
	}
	else
		SetState(LobbyState_Join);

	Network.Stop();
}

// =============================================================================
// Nat Ryall                                                         09-Jul-2008
// =============================================================================
void CLobbyScreen::OnListSessionsCompleted(t_MatchResultError iError, xint iSessionCount, CSession* pSessions)
{
	if (iError == MatchResultError_Success)
	{
		DeleteSessionBoxes();

		m_iSessionCount = iSessionCount;
		m_pSessionList = pSessions;

		for (xint iA = 0; iA < iSessionCount; ++iA)
			m_pSessionBoxes[iA] = new CSessionBox(iA, &m_pSessionList[iA]);

		SetState(LobbyState_List);
	}
	else
	{
		SetState(LobbyState_None);
		ScreenManager::Pop();
	}
}

// =============================================================================
// Nat Ryall                                                         08-Jul-2008
// =============================================================================
void CLobbyScreen::OnCreateSessionCompleted(t_MatchResultError iError, CSession* pSession)
{
	if (iError == MatchResultError_Success)
		CreateLobby();
	else
	{
		SetState(LobbyState_None);
		ScreenManager::Pop();
	}
}

// =============================================================================
// Nat Ryall                                                         09-Jul-2008
// =============================================================================
void CLobbyScreen::OnCloseSessionCompleted(t_MatchResultError iError, CSession* pSession)
{
	delete m_pSession;
	m_pSession = NULL;

	CloseLobby();
}

// =============================================================================
// Nat Ryall                                                         15-Jun-2008
// =============================================================================
void CLobbyScreen::OnJoinClicked(CButtonComponent* pButton, xpoint xOffset)
{
	JoinLobby(m_pJoinInterface->m_pAddressBox->GetText());
}

// =============================================================================
// Nat Ryall                                                         17-Jun-2008
// =============================================================================
void CLobbyScreen::OnConnectionCompleted(xbool bSuccess)
{
	if (bSuccess)
		SetState(LobbyState_Lobby);
	else
		SetState(LobbyState_Join);
}

// =============================================================================
// Nat Ryall                                                         17-Jun-2008
// =============================================================================
void CLobbyScreen::OnConnectionLost()
{
	SetState(LobbyState_Join);
}

// =============================================================================
// Nat Ryall                                                         18-Jun-2008
// =============================================================================
void CLobbyScreen::OnPeerJoined(CNetworkPeer* pPeer)
{
	XLOG("[LobbyScreen] Peer #%d joined the game.", pPeer->m_iID);
}

// =============================================================================
// Nat Ryall                                                         18-Jun-2008
// =============================================================================
void CLobbyScreen::OnPeerLeaving(CNetworkPeer* pPeer)
{
	XLOG("[LobbyScreen] Peer #%d is about to leave the game.", pPeer->m_iID);
}

// =============================================================================
// Nat Ryall                                                         18-Jun-2008
// =============================================================================
void CLobbyScreen::BindPacketCallbacks()
{
	Network.BindReceiveCallback(NetworkStreamType_PlayerInfo, xbind(this, &CLobbyScreen::OnReceivePlayerInfo));
}

// =============================================================================
// Nat Ryall                                                         18-Jun-2008
// =============================================================================
void CLobbyScreen::OnReceivePlayerInfo(CNetworkPeer* pFrom, BitStream* pStream)
{
	XLOG("[LobbyScreen] Received player info from peer #%d.", pFrom->m_iID);
}

// =============================================================================
// Nat Ryall                                                         13-Jul-2008
// =============================================================================
void CLobbyScreen::DeleteSessionBoxes()
{
	for (xint iA = 0; iA < MATCH_SESSION_LIMIT; ++iA)
	{
		if (m_pSessionBoxes[iA])
		{
			delete m_pSessionBoxes[iA];
			m_pSessionBoxes[iA] = NULL;
		}
	}
}

//##############################################################################

//##############################################################################
//
//                               STATUS SCREEN
//
//##############################################################################

// =============================================================================
// Nat Ryall                                                         09-Jun-2008
// =============================================================================
CStatusBox::CStatusBox()
{
	m_pStatusBox = new CProgressComponent(_SPRITE("Menu-Status"));
	m_pStatusBox->SetInnerWidth(500);
	m_pStatusBox->SetPosition(xpoint(_HSWIDTH - (m_pStatusBox->GetWidth() / 2), _HSHEIGHT - (m_pStatusBox->GetHeight() / 2)));
	m_pStatusBox->SetEnabled(false);

	CFontMetadata* pFont = _FONT("Menu-Status");

	m_pLabel = new CLabelComponent(pFont);
	m_pLabel->SetAlignment(HGETEXT_CENTER);
	m_pLabel->SetPosition(xpoint(_HSWIDTH, _HSHEIGHT - ((xint)pFont->GetFont()->GetHeight() / 2)));
}

// =============================================================================
// Nat Ryall                                                         09-Jun-2008
// =============================================================================
CStatusBox::~CStatusBox()
{
	Interface.GetScreen()->Detach(m_pStatusBox);
	delete m_pStatusBox;

	Interface.GetScreen()->Detach(m_pLabel);
	delete m_pLabel;
}

//##############################################################################

//##############################################################################
//
//                             MESSAGE INTERFACE
//
//##############################################################################

//##############################################################################

//##############################################################################
//
//                                 JOIN SCREEN
//
//##############################################################################

// =============================================================================
// Nat Ryall                                                         15-Jun-2008
// =============================================================================
CJoinInterface::CJoinInterface()
{
	m_pAddressBox = new CInputComponent(_SPRITE("Menu-Input"), _FONT("Menu-Input"));
	m_pAddressBox->SetInnerWidth(300);

	m_pJoinButton = new CButtonComponent(_SPRITE("Menu-Button"), _FONT("Menu-Button"));
	m_pJoinButton->SetInnerWidth(100);
	m_pJoinButton->SetText(_LOCALE("Button_Join"));

	m_pAddressBox->SetPosition(xpoint(_HSWIDTH - ((m_pAddressBox->GetWidth() + m_pJoinButton->GetWidth() + 10) / 2), _HSHEIGHT - (m_pAddressBox->GetHeight() / 2)));
	m_pJoinButton->SetPosition(m_pAddressBox->GetPosition() + xpoint(m_pAddressBox->GetWidth() + 5, 0));
}

// =============================================================================
// Nat Ryall                                                         15-Jun-2008
// =============================================================================
CJoinInterface::~CJoinInterface()
{
	Interface.GetScreen()->Detach(m_pAddressBox);
	delete m_pAddressBox;

	Interface.GetScreen()->Detach(m_pJoinButton);
	delete m_pJoinButton;
}

//##############################################################################

//##############################################################################
//
//                                 SESSION BOX
//
//##############################################################################

// =============================================================================
// Nat Ryall                                                         13-Jul-2008
// =============================================================================
CSessionBox::CSessionBox(xint iIndex, CSession* pSession) : CImageComponent(_SPRITE("Menu-Session")),
	m_iIndex(iIndex),
	m_pSession(pSession)
{
	m_iType = ElementType_SessionBox;

	m_pTitleFont = new CFont(_FONT("Menu-Session-Title"));
	m_pInfoFont = new CFont(_FONT("Menu-Session-Info"));

	xpoint xSize = m_pSprite->GetImageSize();
	SetPosition(xpoint(_HSWIDTH - (xSize.iX / 2), 40 + (m_iIndex * (xSize.iY + 10))));
}

// =============================================================================
// Nat Ryall                                                         13-Jul-2008
// =============================================================================
CSessionBox::~CSessionBox()
{
	delete m_pTitleFont;
	delete m_pInfoFont;
}

// =============================================================================
// Nat Ryall                                                         13-Jul-2008
// =============================================================================
void CSessionBox::Render()
{
	CImageComponent::Render();

	m_pTitleFont->Render
	(
		m_pSession->m_sTitle.c_str(), 
		m_pSprite->GetMetadata()->FindArea("Title")->xRect + GetPosition(), 
		HGETEXT_CENTER | HGETEXT_MIDDLE
	);

	m_pInfoFont->Render
	(
		XFORMAT("%d Playing", m_pSession->m_iUsedSlots), 
		m_pSprite->GetMetadata()->FindArea("Info")->xRect + GetPosition(), 
		HGETEXT_CENTER | HGETEXT_MIDDLE
	);
}

// =============================================================================
// Nat Ryall                                                         13-Jul-2008
// =============================================================================
void CSessionBox::OnMouseUp(xpoint xPosition)
{
	_GLOBAL.pLobby->JoinLobby(m_pSession->m_sIP.c_str());
}

//##############################################################################