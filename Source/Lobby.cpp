/**
* @file Lobby.cpp
* @author Nat Ryall
* @date 22/05/2008
* @brief The online lobby screen implementation.
*
* Copyright � SAPIAN
*/

//##############################################################################

// Global.
#include <Global.h>

// Local.
#include <Lobby.h>

//##############################################################################

// =============================================================================
CLobbyScreen::CLobbyScreen() : CScreen(ScreenIndex_LobbyScreen),
	m_iState(LobbyState_None),
	m_bPublic(true),
	m_pSession(NULL)
{
}

// =============================================================================
CLobbyScreen::~CLobbyScreen()
{
}

// =============================================================================
void CLobbyScreen::OnLoad()
{
    // Initialise the render resources.
	m_pBackground = new CBackgroundImage("Lobby-Background");

	// Load.
	m_pJoinInterface = new CJoinInterface();
	m_pStatusBox = new CStatusBox();

	m_pJoinInterface->m_pJoinButton->SetClickCallback(xbind(this, &CLobbyScreen::OnJoinClicked));

	// Initialise,
	m_pPeerFont = new CFont(_FONT("Lobby-Peer"));

	for (xint iA = 0; iA < MATCH_SESSION_LIMIT; ++iA)
		m_pSessionBoxes[iA] = NULL;

	m_pJoinInterface->m_pAddressBox->SetText("127.0.0.1");
}

// =============================================================================
void CLobbyScreen::OnUnload()
{
	delete m_pJoinInterface;
	delete m_pStatusBox;

	delete m_pPeerFont;

	DeleteSessionBoxes();
}

// =============================================================================
void CLobbyScreen::OnActivate()
{
	// Reset the lobby states.
	m_iState = LobbyState_None;

	// Initialise the layers.
	m_xRenderView = new CRenderView(LobbyLayerIndex_Max);

	m_xRenderView->GetLayer(LobbyLayerIndex_Background)->AttachRenderable(m_pBackground);
	m_xRenderView->GetLayer(LobbyLayerIndex_Listing)->SetRenderOverride(xbind(this, &CLobbyScreen::RenderLobby));
	m_xRenderView->GetLayer(LobbyLayerIndex_Interface)->SetRenderOverride(xbind(&InterfaceManager, &CInterfaceManager::Render));
}

// =============================================================================
void CLobbyScreen::OnDeactivate()
{
	m_iState = LobbyState_None;

	delete m_xRenderView;
	m_xRenderView = NULL;
}

// =============================================================================
void CLobbyScreen::OnWake()
{
	RenderManager.SetRenderView(m_xRenderView);
	InterfaceManager.SetCursorVisible(true);

	switch (m_iState)
	{
	case LobbyState_Game:
		{
			EndGame();
			Stop();
		}
		break;
	}
}

// =============================================================================
void CLobbyScreen::OnSleep()
{
	RenderManager.ClearRenderView();
	InterfaceManager.SetCursorVisible(false);
}

// =============================================================================
void CLobbyScreen::OnUpdate()
{
	if (_HGE->Input_KeyUp(HGEK_ESCAPE))
		Stop();

	ParentUpdate();

	switch (m_iState)
	{
	case LobbyState_Lobby:
		{
			UpdateLobby();
		}
		break;

	case LobbyState_Closing:
		{
			if (!MatchManager.IsBusy() && m_pSession)
				MatchManager.CloseSession(m_pSession, xbind(this, &CLobbyScreen::OnCloseSessionCompleted));
		}
		break;
	}

	m_pBackground->Update();
}

// =============================================================================
void CLobbyScreen::OnPreRender()
{
	ParentPreRender();
}

// =============================================================================
void CLobbyScreen::OnPostRender()
{
}

// =============================================================================
void CLobbyScreen::UpdateLobby()
{
	if (NetworkManager.IsHosting())
	{
		// Ping the matchmaking service on set intervals to keep the session alive.
		if (m_pSession && !MatchManager.IsBusy() && m_xPingTimer.IsExpired())
		{
			MatchManager.PingSession(m_pSession, NULL);
			m_xPingTimer.ExpireAfter(MATCH_PING_INTERVAL);
		}

		// Start the game when ENTER is pressed (debug).
		if (_HGE->Input_KeyUp(HGEK_ENTER) && NetworkManager.IsEveryoneVerified())
		{
			NetworkManager.Broadcast(NULL, NetworkStreamType_StartGame, NULL, HIGH_PRIORITY, RELIABLE_ORDERED);
			StartGame();
		}
	}
}

// =============================================================================
void CLobbyScreen::RenderLobby(CRenderLayer* pLayer)
{
	xint iPeerOffset = 0;

	XEN_LIST_FOREACH(t_NetworkPeerList, ppPeer, NetworkManager.GetVerifiedPeers())
	{
		CNetworkGamerCard* pCard = GetGamerCard(*ppPeer);
		
		m_pPeerFont->Render(pCard->m_cNickname, xpoint(80, 100 + iPeerOffset), HGETEXT_LEFT);
		iPeerOffset += 40;
	}

	if (NetworkManager.IsHosting())
		m_pPeerFont->Render("You are the Host!", xpoint(10, 10), HGETEXT_LEFT);

	//Global.m_pGameFont->Render(XFORMAT("%d", NetworkManager.GetLastPing()), xpoint(2, 0), HGETEXT_LEFT);
}

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
			SetState(LobbyState_Searching);
			MatchManager.ListSessions(xbind(this, &CLobbyScreen::OnListSessionsCompleted));
		}
		break;

	case LobbyStartMode_JoinPrivate:
		{
			SetState(LobbyState_Specify);
		}
		break;
	
	case LobbyStartMode_CreatePublic:
		{
			SetState(LobbyState_Creating);
			m_pSession = MatchManager.CreateSession(4, "PikPik Beta Server", xbind(this, &CLobbyScreen::OnCreateSessionCompleted));
		}
		break;

	case LobbyStartMode_CreatePrivate:
		{
			SetState(LobbyState_Creating);
			CreateLobby();
		}
		break;
	}

	m_iStartMode = iStartMode;
}

// =============================================================================
void CLobbyScreen::Stop()
{
	if (NetworkManager.IsRunning())
	{
		SetState(LobbyState_Leaving);
		NetworkManager.RequestStop();
	}
	else
		OnNetworkStop();
}

// =============================================================================
void CLobbyScreen::SetState(t_LobbyState iState)
{
	InterfaceCanvas.Clear();

	switch (iState)
	{
	case LobbyState_List:
		{
			for (int iA = 0; iA < m_iSessionCount; ++iA)
				InterfaceCanvas.Attach(m_pSessionBoxes[iA]);
		}
		break;

	case LobbyState_Searching:
		{
			m_pStatusBox->AttachElements();
			m_pStatusBox->m_pLabel->SetText(_LOCALE("Status_Searching"));
		}
		break;

	case LobbyState_Specify:
		{
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

	case LobbyState_Verifying:
		{
			m_pStatusBox->AttachElements();
			m_pStatusBox->m_pLabel->SetText(_LOCALE("Status_Verifying"));
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
void CLobbyScreen::InitialiseNetwork()
{
	// Initialise the local player's gamer card.
	static const char* s_pNames[] =
	{
		"PeterParker",
		"Krakken",
		"w0nd3rw0man",
		"<M00MIN>",
		"slygamer123",
		"Middas",
		"kazii",
		"Malus",
		"robspear",
		"KRM-Gaute",
		"|6P|_Pickis",
		"The-Guvna",
		"Firefoot",
		"Shaks",
		"Rlan",
		"cadron_9000",
		"billBo_bacons",
		"STALLONE [AH]",
		"TMWNN",
		"bkt"
	};

	strcpy_s(m_xGamerCard.m_cNickname, _MAXNAMELEN, s_pNames[rand() % 20]);
	m_xGamerCard.m_iSeed = rand() % 4096;

	NetworkManager.SetGamerCard(&m_xGamerCard, sizeof(CNetworkGamerCard));

	// Initialise the verification info.
	NetworkManager.SetVerificationInfo(_GID, String::Length(_GID) + 1);

	// Bind the stream type callbacks.
	NetworkManager.BindReceiveCallback(NetworkStreamType_StartGame, xbind(this, &CLobbyScreen::OnReceiveStartGame));
	NetworkManager.BindReceiveCallback(NetworkStreamType_PlayerUpdate, &CPlayer::OnReceivePlayerUpdate);

	// Bind all event callbacks.
	NetworkManager.m_xCallbacks.m_fpNetworkStarted = xbind(this, &CLobbyScreen::OnNetworkStart);
	NetworkManager.m_xCallbacks.m_fpNetworkStopped = xbind(this, &CLobbyScreen::OnNetworkStop);
	NetworkManager.m_xCallbacks.m_fpConnectionCompleted = xbind(this, &CLobbyScreen::OnConnectionCompleted);
	NetworkManager.m_xCallbacks.m_fpConnectionLost = xbind(this, &CLobbyScreen::OnConnectionLost);
	NetworkManager.m_xCallbacks.m_fpVerifyPeer = xbind(this, &CLobbyScreen::OnVerifyPeer);
	NetworkManager.m_xCallbacks.m_fpVerificationCompleted = xbind(this, &CLobbyScreen::OnConnectionVerified);
	NetworkManager.m_xCallbacks.m_fpPeerJoined = xbind(this, &CLobbyScreen::OnPeerJoined);
	NetworkManager.m_xCallbacks.m_fpPeerLeaving = xbind(this, &CLobbyScreen::OnPeerLeaving);
}

// =============================================================================
void CLobbyScreen::CreateLobby()
{
	NetworkManager.Reset();

	InitialiseNetwork();

	NetworkManager.StartHost(4, _HOSTPORT);

	SetState(LobbyState_Lobby);
}

// =============================================================================
void CLobbyScreen::JoinLobby(const xchar* pHostAddress)
{
	NetworkManager.Reset();

	InitialiseNetwork();

	NetworkManager.StartClient(pHostAddress, _HOSTPORT);

	SetState(LobbyState_Connecting);
}

// =============================================================================
void CLobbyScreen::StartGame()
{
	//SetState(LobbyState_Starting);

	// Load the map.
	MapManager.SetCurrentMap("M009");

	// Initialise the players.
	if (NetworkManager.IsHosting())
		PlayerManager.InitialisePlayers(PlayerLogicType_AI);
	else
		PlayerManager.InitialisePlayers(PlayerLogicType_Remote);

	xint iPlayerIndex = 0;

	XEN_LIST_FOREACH(t_NetworkPeerList, ppPeer, NetworkManager.GetVerifiedPeers())
	{
		CNetworkPeer* pPeer = *ppPeer;
		CNetworkPeerInfo* pInfo = GetPeerInfo(pPeer);

		pInfo->m_pPlayer = PlayerManager.GetActivePlayer(iPlayerIndex++);
		pInfo->m_pPlayer->SetLogicType(pPeer->m_bLocal ? PlayerLogicType_Local : PlayerLogicType_Remote);

		if (pPeer->m_bLocal)
			PlayerManager.SetLocalPlayer(iPlayerIndex);
	}

	// Start the game.
	SetState(LobbyState_Game);
	ScreenManager.Push(ScreenIndex_GameScreen);
}

// =============================================================================
void CLobbyScreen::EndGame()
{
	MapManager.ClearCurrentMap();
}

// =============================================================================
void CLobbyScreen::OnJoinClicked(CButtonComponent* pButton, xpoint xOffset)
{
	JoinLobby(m_pJoinInterface->m_pAddressBox->GetText());
}

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
		Stop();
}

// =============================================================================
void CLobbyScreen::OnCreateSessionCompleted(t_MatchResultError iError, CSession* pSession)
{
	if (iError == MatchResultError_Success)
		CreateLobby();
	else
		Stop();
}

// =============================================================================
void CLobbyScreen::OnCloseSessionCompleted(t_MatchResultError iError, CSession* pSession)
{
	delete m_pSession;
	m_pSession = NULL;

	OnNetworkStop();
}

// =============================================================================
xbool CLobbyScreen::OnVerifyPeer(CNetworkPeer* pPeer, void* pData, xint iDataLength)
{
	return iDataLength && pData && String::IsMatch(_GID, (const xchar*)pData) && m_iState == LobbyState_Lobby;
}

// =============================================================================
void CLobbyScreen::OnNetworkStart()
{
#if XDEBUG
	NetworkManager.GetInterface()->ApplyNetworkSimulator(XKB(56), 80, 40);
#endif
}

// =============================================================================
void CLobbyScreen::OnNetworkStop()
{
	if (m_iState != LobbyState_None)
	{
		// If we have a session still open, close that and return here later.
		if (NetworkManager.IsHosting() && m_pSession)
		{
			if (m_iState != LobbyState_Closing)
				SetState(LobbyState_Closing);
		}
		// Otherwise, we're ready to exit so let's leave.
		else
		{
			SetState(LobbyState_None);
			ScreenManager.Pop();
		}
	}
}

// =============================================================================
void CLobbyScreen::OnConnectionCompleted(xbool bSuccess)
{
	if (bSuccess)
		SetState(LobbyState_Verifying);
	else
		Stop();
}

// =============================================================================
void CLobbyScreen::OnConnectionVerified(xbool bSuccess)
{
	if (bSuccess)
		SetState(LobbyState_Lobby);
	else
		Stop();
}

// =============================================================================
void CLobbyScreen::OnConnectionLost()
{
	OnNetworkStop();
}

// =============================================================================
void CLobbyScreen::OnPeerJoined(CNetworkPeer* pPeer)
{
	XLOG("[LobbyScreen] '%s' joined the game with the ID %d.", GetGamerCard(pPeer)->m_cNickname, pPeer->m_iID);

	CNetworkPeerInfo* pInfo = new CNetworkPeerInfo();
	pInfo->m_pPlayer = NULL;

	pPeer->m_pData = pInfo;

	NetworkManager.SortPeers();
}

// =============================================================================
void CLobbyScreen::OnPeerLeaving(CNetworkPeer* pPeer)
{
	XLOG("[LobbyScreen] '%s' with the ID %d is about to leave the game.", GetGamerCard(pPeer)->m_cNickname, pPeer->m_iID);

	CNetworkPeerInfo* pInfo = GetPeerInfo(pPeer);

	if (m_iState == LobbyState_Game)
		pInfo->m_pPlayer->SetLogicType(NetworkManager.IsHosting() ? PlayerLogicType_AI : PlayerLogicType_Remote);

	if (pInfo)
		delete pInfo;
}

// =============================================================================
void CLobbyScreen::OnReceiveStartGame(CNetworkPeer* pFrom, BitStream* pStream)
{
	XLOG("[LobbyScreen] Received notification to start the game.");

	StartGame();
}

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
CStatusBox::~CStatusBox()
{
	InterfaceCanvas.Detach(m_pStatusBox);
	delete m_pStatusBox;

	InterfaceCanvas.Detach(m_pLabel);
	delete m_pLabel;
}

//##############################################################################

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
CJoinInterface::~CJoinInterface()
{
	InterfaceCanvas.Detach(m_pAddressBox);
	delete m_pAddressBox;

	InterfaceCanvas.Detach(m_pJoinButton);
	delete m_pJoinButton;
}

//##############################################################################

// =============================================================================
CSessionBox::CSessionBox(xint iIndex, CSession* pSession) : CImageComponent(_SPRITE("Menu-Session")),
	m_iIndex(iIndex),
	m_pSession(pSession)
{
	m_iType = ElementType_SessionBox;

	m_pTitleFont = new CFont(_FONT("Menu-Session-Title"));
	m_pInfoFont = new CFont(_FONT("Menu-Session-Info"));

	xpoint xSize = m_pSprite->GetImageSize();
	SetPosition(xpoint(_HSWIDTH - (xSize.m_tX / 2), 40 + (m_iIndex * (xSize.m_tY + 10))));
}

// =============================================================================
CSessionBox::~CSessionBox()
{
	delete m_pTitleFont;
	delete m_pInfoFont;
}

// =============================================================================
void CSessionBox::OnRender()
{
	CImageComponent::OnRender();

	m_pTitleFont->Render
	(
		m_pSession->m_sTitle.c_str(), 
		m_pSprite->GetMetadata()->FindArea("Title")->m_xRect + GetPosition(), 
		HGETEXT_CENTER | HGETEXT_MIDDLE
	);

	m_pInfoFont->Render
	(
		XFORMAT("%d Playing", m_pSession->m_iUsedSlots), 
		m_pSprite->GetMetadata()->FindArea("Info")->m_xRect + GetPosition(), 
		HGETEXT_CENTER | HGETEXT_MIDDLE
	);
}

// =============================================================================
xbool CSessionBox::OnMouseUp(xpoint xPosition)
{
	CLobbyScreen* pLobby = (CLobbyScreen*)ScreenManager.FindScreen(ScreenIndex_LobbyScreen);
	pLobby->JoinLobby(m_pSession->m_sIP.c_str());

	return true;
}

//##############################################################################
