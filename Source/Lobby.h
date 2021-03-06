#pragma once

/**
* @file Lobby.h
* @author Nat Ryall
* @date 22/05/2008
* @brief The online lobby screen implementation.
*
* Copyright � SAPIAN
*/

//##############################################################################

// Global.
#include <Global.h>

// Other.
#include <Component.h>
#include <Network.h>
#include <Match.h>
#include <Menu.h>
#include <Player.h>

//##############################################################################

// Predeclare.
class CStatusBox;
class CJoinInterface;
class CSessionBox;

// The lobby state.
enum t_LobbyState
{
	LobbyState_None,
	LobbyState_Searching,
	LobbyState_List,
	LobbyState_Specify,
	LobbyState_Creating,
	LobbyState_Connecting,
	LobbyState_Verifying,
	LobbyState_Joining,
	LobbyState_Lobby,
	LobbyState_Starting,
	LobbyState_Game,
	LobbyState_Ending,
	LobbyState_Leaving,
	LobbyState_Closing,
};

// The message display mode.
enum t_MessageDisplayMode
{
	MessageDisplayMode_None,
	MessageDisplayMode_Error,
	MessageDisplayMode_Confirm,
};

//##############################################################################
class CNetworkGamerCard
{
public:
	// The peer's nickname/gamertag with a terminating NULL.
	char m_cNickname[_MAXNAMECHARS];

	// The player's random seed.
	xint m_iSeed;
};

//##############################################################################
class CNetworkPeerInfo
{
public:
	// The player character assigned to the peer.
	union 
	{
		// Base.
		CPlayer* m_pPlayer;

		// Pacman.
		CPacman* m_pPacman;

		// Ghost.
		CGhost* m_pGhost;
	};
};

//##############################################################################
class CLobbyScreen : public CScreen
{
public:
	// Friends.
	friend class CSessionBox;

	// Initialise.
	CLobbyScreen();

	// Deinitialise
	virtual ~CLobbyScreen();

	// Start and initialise the lobby using the specified mode.
	void Start(t_LobbyStartMode iStartMode);

	// Stop and close the lobby.
	void Stop();

	// Get the gamer card from a peer structure.
	inline CNetworkGamerCard* GetGamerCard(CNetworkPeer* pPeer)
	{
		return (CNetworkGamerCard*)pPeer->m_pGamerCard;
	}

	// Get the local peer info from a peer structure.
	inline CNetworkPeerInfo* GetPeerInfo(CNetworkPeer* pPeer)
	{
		return (CNetworkPeerInfo*)pPeer->m_pData;
	}

protected:
	// Called to load the screen resources.
	virtual void OnLoad();

	// Called to unload the screen resources.
	virtual void OnUnload();

	// Called when the screen is first added to the screen stack.
	virtual void OnActivate();

	// Called when the screen is removed from the screen stack.
	virtual void OnDeactivate();

	// Called once when the screen is placed at the top of the stack.
	virtual void OnWake();

	// Called once when the screen goes out of scope either through destruction or another screen is placed above this on the stack.
	virtual void OnSleep();

	// Called each frame to update the screen when active.
	virtual void OnUpdate();

	// Called immediately before the render manager is executed.	
	virtual void OnPreRender();

	// Called immediately after the render manager has executed.	
	virtual void OnPostRender();

	// OnUpdate the main lobby screen.
	void UpdateLobby();

	// Render the main lobby screen.
	void RenderLobby(CRenderLayer* pLayer);

	// Set the internal state.
	void SetState(t_LobbyState iState);

	// Initialise the network settings.
	void InitialiseNetwork();

	// Create a new lobby and act as the host.
	void CreateLobby();

	// Join an existing lobby and act as a client.
	void JoinLobby(const xchar* pHostAddress);

	// Start the game from the lobby.
	void StartGame();

	// End the game.
	void EndGame();

	// Close the lobby down and disconnect from the network.
	void CloseLobby();

	// Delete any session box instances we have.
	void DeleteSessionBoxes();

	// Callback for when the join button is clicked in the join interface.
	void OnJoinClicked(CButtonComponent* pButton, xpoint xOffset);

	// Callback for when the listing of available sessions has completed.
	void OnListSessionsCompleted(t_MatchResultError iError, xint iSessionCount, CSession* pSessions);

	// Callback for when the session has created.
	void OnCreateSessionCompleted(t_MatchResultError iError, CSession* pSession);

	// Callback for when the session is closed.
	void OnCloseSessionCompleted(t_MatchResultError iError, CSession* pSession);

	// Callback to verify the client data on the host.
	xbool OnVerifyPeer(CNetworkPeer* pPeer, void* pData, xint iDataLength);

	// Callback for when the network is started.
	void OnNetworkStart();

	// Callback for when the network is stopped.
	void OnNetworkStop();

	// Callback for when the network has successfully started.
	void OnConnectionCompleted(xbool bSuccess);

	// Callback for when the network has veen verified.
	void OnConnectionVerified(xbool bSuccess);

	// Callback for when the network connection has been lost.
	void OnConnectionLost();

	// Callback for when a remote peer joins the game.
	void OnPeerJoined(CNetworkPeer* pPeer);

	// Callback for when a remote peer is just about to leave the game.
	void OnPeerLeaving(CNetworkPeer* pPeer);

	// Callback for when a start game placet is received.
	void OnReceiveStartGame(CNetworkPeer* pFrom, BitStream* pStream);

	// The lobby startup mode.
	t_LobbyStartMode m_iStartMode;

	// The current lobby state.
	t_LobbyState m_iState;

	// Specifies if the lobby is public or private.
	xbool m_bPublic;

	// The local active session.
	CSession* m_pSession;

	// The local gamer card.
	CNetworkGamerCard m_xGamerCard;

    // The background image to scroll.
	CBackgroundImage* m_pBackground;

	// The join interface.
	CJoinInterface* m_pJoinInterface;

	// The status box.
	CStatusBox* m_pStatusBox;

	// The session list object.
	CSessionBox* m_pSessionBoxes[MATCH_SESSION_LIMIT];

	// The lobby peer font.
	CFont* m_pPeerFont;

	// The session list count.
	xint m_iSessionCount;

	// The session list.
	CSession* m_pSessionList;

	// The ping timer.
	CTimer m_xPingTimer;

	// The screen's render view.
	CRenderView* m_xRenderView;
};

//##############################################################################
class CStatusBox
{
public:
	// Friends.
	friend class CLobbyScreen;

	// Constructor.
	CStatusBox();

	// Destructor.
	virtual ~CStatusBox();

	// Attach all elements to the interface.
	inline void AttachElements()
	{
		InterfaceCanvas.Attach(m_pStatusBox);
		InterfaceCanvas.Attach(m_pLabel);
	}

protected:
	// The status screen display box.
	CProgressComponent* m_pStatusBox;

	// The status text.
	CLabelComponent* m_pLabel;
};

//##############################################################################
class CMessageInterface
{
public:
	// Friends.
	friend class CLobbyScreen;

	// Constructor.
	CMessageInterface();

	// Destructor.
	virtual ~CMessageInterface();

	// Attach all elements to the interface.
	inline void AttachElements()
	{
		InterfaceCanvas.Attach(m_pAcceptLink);
		InterfaceCanvas.Attach(m_pCancelLink);
	}

	// Render the message interface.
	void Render();

protected:
	// The display mode.
	t_MessageDisplayMode m_iMode;

	// The "accept" text link.
	CHyperlinkComponent* m_pAcceptLink;

	// The "cancel" text link.
	CHyperlinkComponent* m_pCancelLink;
};

//##############################################################################
class CJoinInterface
{
public:
	// Friends.
	friend class CLobbyScreen;

	// Constructor.
	CJoinInterface();

	// Destructor.
	virtual ~CJoinInterface();

	// Attach all elements to the interface.
	inline void AttachElements()
	{
		InterfaceCanvas.Attach(m_pAddressBox);
		InterfaceCanvas.Attach(m_pJoinButton);
	}

protected:
	// The join screen address box.
	CInputComponent* m_pAddressBox;

	// The join screen join button.
	CButtonComponent* m_pJoinButton;
};

//##############################################################################
class CSessionBox : CImageComponent
{
public:
	// Friends.
	friend class CLobbyScreen;

	// Constructor.
	CSessionBox(xint iIndex, CSession* pSession);

	// Destructor.
	virtual ~CSessionBox();

	// Render the element.
	virtual void OnRender();

protected:
	// Triggered when the left mouse-button is released within the element area.
	virtual xbool OnMouseUp(xpoint xPosition);

	// The title font.
	CFont* m_pTitleFont;

	// The info font.
	CFont* m_pInfoFont;

	// The index of the session box.
	xint m_iIndex;
		
	// The session bound to this box.
	CSession* m_pSession;
};

//##############################################################################