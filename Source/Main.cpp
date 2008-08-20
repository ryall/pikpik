//##############################################################################
//
//                                   INCLUDE
//
//##############################################################################

// Global.
#include <Global.h>

// Local.
#include <Main.h>

// Other.
#include <Splash.h>
#include <Menu.h>
#include <Game.h>
#include <Lobby.h>
#include <Windows.h>
#include <Resource.h>
#include <Network.h>
#include <Character.h>
#include <Match.h>
#include <Collision.h>
#include <Player.h>
#include <Visor.h>
#include <Crypt.h>

// Crypto.
#include <Crypto/cryptlib.h>

//##############################################################################

//##############################################################################
//
//                                   TYPES
//
//##############################################################################

// Lists.
typedef xlist<CScreen*> t_ScreenList;

//##############################################################################

//##############################################################################
//
//                                   STATIC
//
//##############################################################################

// HGE.
static HGE* s_pInterface = NULL;

// Sound.
static FMOD::System* s_pSoundSystem = NULL;

// Flow Control.
static xbool s_bTerminate = false;

// Timer.
static xuint s_iTimeDelta = 0;

// Screen List.
::t_ScreenList s_lpScreens;

//##############################################################################

//##############################################################################
//
//                                    MAIN
//
//##############################################################################

// =============================================================================
// Nat Ryall                                                         23-Mar-2008
// =============================================================================
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	s_pInterface = hgeCreate(HGE_VERSION);

	s_pInterface->System_SetState(HGE_FRAMEFUNC, &Application::Update);
	s_pInterface->System_SetState(HGE_RENDERFUNC, &Application::Render);
	s_pInterface->System_SetState(HGE_FOCUSLOSTFUNC, &Application::OnBlur);
	s_pInterface->System_SetState(HGE_FOCUSGAINFUNC, &Application::OnFocus);
	s_pInterface->System_SetState(HGE_TITLE, "PikPik");
	s_pInterface->System_SetState(HGE_USESOUND, false);
	s_pInterface->System_SetState(HGE_WINDOWED, true);
	s_pInterface->System_SetState(HGE_SCREENWIDTH, _SWIDTH);
	s_pInterface->System_SetState(HGE_SCREENHEIGHT, _SHEIGHT);
	s_pInterface->System_SetState(HGE_SCREENBPP, 32);
	s_pInterface->System_SetState(HGE_SHOWSPLASH, false);
	s_pInterface->System_SetState(HGE_FPS, 60);
	s_pInterface->System_SetState(HGE_DONTSUSPEND, true);

	if (FMOD::System_Create(&s_pSoundSystem) == FMOD_OK)
	{
		FMOD_RESULT eSoundResult = s_pSoundSystem->init(32, FMOD_INIT_NORMAL, NULL);

		if (eSoundResult == FMOD_OK && s_pInterface->System_Initiate())
		{
			HWND hWindow = s_pInterface->System_GetState(HGE_HWND);

			try
			{
				Application::Initialise();

				s_pInterface->System_Start();
			}
			catch (Xen::CException xException)
			{
				_TERMINATE;

				MessageBox(hWindow, xException.GetDetailedMessage().c_str(), "Application Error", MB_OK | MB_ICONERROR | MB_APPLMODAL);
				XLOG(xException.GetDetailedMessage().c_str());
			}
			catch (CryptoPP::Exception xException)
			{
				_TERMINATE;

				MessageBox(hWindow, xException.what(), "Cryptographic Error", MB_OK | MB_ICONERROR | MB_APPLMODAL);
				XLOG(xException.what());
			}

			s_pInterface->System_Shutdown();

			Application::Deinitialise();
		}

		s_pSoundSystem->release();
		s_pSoundSystem = NULL;
	}

	s_pInterface->Release();
	s_pInterface = NULL;

	return 0;
}

//##############################################################################

//##############################################################################
//
//                                 DECLARATION
//
//##############################################################################

// =============================================================================
// Nat Ryall                                                          8-Apr-2008
// =============================================================================
void Application::Initialise()
{
	// Seed the random number generator.
	srand(_TIMEMS);

	// Initialise global vars.
	Global.m_bWindowFocused = true;
	Global.m_pActiveMap = NULL;
	Global.m_pLocalPlayer = NULL;
	Global.m_fMusicEnergy = 0.f;
	Global.m_fWorldAlpha = 1.f;

	// Load all relevant metadata.
	ResourceManager::RegisterMetadata(new CMetadata(".\\Metadata\\Sprites" _METAEXT, _METAKEY, true));
	ResourceManager::RegisterMetadata(new CMetadata(".\\Metadata\\Font" _METAEXT, _METAKEY, true));

	// Initialise the game font.
	Global.m_pGameFont = new CFont(_FONT("Default"));

	// Load the locale.
	Global.m_pLocale = new CMetadata(".\\Metadata\\Strings" _METAEXT, _METAKEY, true);

	// Add all required modules to the game.
	XMODULE(&Network);
	XMODULE(&Match);
	XMODULE(&MapManager);
	XMODULE(&ScreenManager);
	XMODULE(&CollisionManager);
	XMODULE(&RenderManager);
	XMODULE(&Interface);

	// Initialise all modules.
	ModuleManager.Initialise();

	// Create all the screen instances.
	s_lpScreens.push_back(new CLogoScreen);
	s_lpScreens.push_back(new CWarningScreen);
	s_lpScreens.push_back(new CMenuScreen);
	s_lpScreens.push_back(new CGameScreen);
	s_lpScreens.push_back(new CLobbyScreen);
	s_lpScreens.push_back(new CCharacterScreen);
	s_lpScreens.push_back(new CVisorScreen);

	// Load all screen instances and set go to the logo screen.
	ScreenManager.LoadScreens();
	ScreenManager.Set(ScreenIndex_LogoScreen, true);

	// Create all the available players.
	Global.m_lpPlayers.push_back(new CPacman());
	Global.m_lpPlayers.push_back(new CGhost(0xFF40F0F0));
	Global.m_lpPlayers.push_back(new CGhost(0xFFF04040));
	Global.m_lpPlayers.push_back(new CGhost(0xFF4040F0));
	Global.m_lpPlayers.push_back(new CGhost(0xFFF0F040));
	Global.m_lpPlayers.push_back(new CGhost(0xFFF040F0));
	
	// Execute the first frame update.
	Update();
}

// =============================================================================
// Nat Ryall                                                          8-Apr-2008
// =============================================================================
void Application::Deinitialise()
{
	// Free the string metadata.
	delete Global.m_pLocale;

	// Free the font.
	delete Global.m_pGameFont;

	// Unload all loaded screens.
	ScreenManager.UnloadScreens();

	// Deinitialise all modules.
	ModuleManager.Deinitialise();

	// Free all screen instances.
	XEN_LIST_ERASE_ALL(s_lpScreens);

	// Free all the players.
	XEN_LIST_ERASE_ALL(Global.m_lpPlayers);
}

// =============================================================================
// Nat Ryall                                                         23-Mar-2008
// =============================================================================
xbool Application::Update()
{
	if (!s_bTerminate)
	{
		s_iTimeDelta = (xuint)(_TIMEDELTAF * 1000.f);

		s_pSoundSystem->update();

		ModuleManager.Update();

		hgeInputEvent hgEvent;

		while (s_pInterface->Input_GetEvent(&hgEvent))
			ScreenManager.Event(hgEvent.type, &hgEvent);
	}

	return s_bTerminate;
}

// =============================================================================
// Nat Ryall                                                         23-Mar-2008
// =============================================================================
xbool Application::Render()
{
	if (!s_bTerminate)
	{
		s_pInterface->Gfx_BeginScene();

		ModuleManager.Render();

		s_pInterface->Gfx_EndScene();
	}
		
	return false;
}

// =============================================================================
// Nat Ryall                                                         30-Jul-2008
// =============================================================================
xbool Application::OnBlur()
{
	Global.m_bWindowFocused = false;

	return false;
}

// =============================================================================
// Nat Ryall                                                         30-Jul-2008
// =============================================================================
xbool Application::OnFocus()
{
	Global.m_bWindowFocused = true;

	return false;
}

// =============================================================================
// Nat Ryall                                                          6-Apr-2008
// =============================================================================
void Application::Terminate()
{
	s_bTerminate = true;
}

// =============================================================================
// Nat Ryall                                                          6-Apr-2008
// =============================================================================
HGE* Application::GetInterface()
{
	return s_pInterface;
}

// =============================================================================
// Nat Ryall                                                          8-Apr-2008
// =============================================================================
xuint Application::GetTimeDelta()
{
	return s_iTimeDelta;
}

// =============================================================================
// Nat Ryall                                                          2-Jun-2008
// =============================================================================
FMOD::System* Application::GetSoundSystem()
{
	return s_pSoundSystem;
}

//##############################################################################


