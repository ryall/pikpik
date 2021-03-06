//##############################################################################

// Global.
#include <Global.h>

// Local.
#include <Splash.h>

// Other.
#include <Renderer.h>

//##############################################################################
#define SPLASH_FADETIME 500
#define SPLASH_DISPLAYTIME 1000

//##############################################################################

// =============================================================================
CFadeScreen::CFadeScreen(t_ScreenIndex iIndex, xuint iFadeTime, xuint iDisplayTime) : CScreen(iIndex),
	m_iFadeTime(iFadeTime),
	m_iDisplayTime(iDisplayTime)
{
	Reset();
}

// =============================================================================
void CFadeScreen::Reset()
{
	m_iStage = FadeScreenStage_FadeIn;
	m_iElapsedTime = 0;
	m_iTotalTime = 0;
	m_fAlpha = 0.0f;
}

// =============================================================================
void CFadeScreen::OnUpdate()
{
	m_iElapsedTime += _TIMEDELTA;
	m_iTotalTime += _TIMEDELTA;

	switch (m_iStage)
	{
	case FadeScreenStage_FadeIn:
		{
			if (m_iElapsedTime > m_iFadeTime)
			{
				m_iElapsedTime -= m_iFadeTime;

				m_fAlpha = 1.0f;
				m_iStage = FadeScreenStage_Display;
			}
			else
				m_fAlpha = (float)m_iElapsedTime / (float)m_iFadeTime;
		}
		break;

	case FadeScreenStage_Display:
		{
			if (m_iElapsedTime > m_iDisplayTime)
			{
				m_iElapsedTime -= m_iDisplayTime;
				m_iStage = FadeScreenStage_FadeOut;
			}
		}
		break;

	case FadeScreenStage_FadeOut:
		if (m_iElapsedTime > m_iFadeTime)
		{
			m_iElapsedTime = 0;

			m_fAlpha = 0.0f;
			m_iStage = FadeScreenStage_Complete;

			OnFadeComplete();
		}
		else
			m_fAlpha = 1.0f - ((float)m_iElapsedTime / (float)m_iFadeTime);
		break;

	case FadeScreenStage_Complete:
		break;
	}
}

//##############################################################################