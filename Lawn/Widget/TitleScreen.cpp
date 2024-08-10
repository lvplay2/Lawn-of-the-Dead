#include "TitleScreen.h"
#include "../../SexyAppFramework/HyperlinkWidget.h"
#include "../../SexyAppFramework/WidgetManager.h"
#include "../../LawnApp.h"
#include "../../Resources.h"
#include "../../Sexy.TodLib/TodCommon.h"
#include "../../SexyAppFramework/SexyMatrix.h"
#include "../../Sexy.TodLib/TodStringFile.h"
#include "../../Sexy.TodLib/EffectSystem.h"
#include "../../Sexy.TodLib/TodDebug.h"
#include "../../Sexy.TodLib/Reanimator.h"
#include "../../GameConstants.h"
#include "../System/Music.h"

TitleScreen::TitleScreen(LawnApp* theApp)
{
	mCurBarWidth = 0.0f;
	mTotalBarWidth = 314.0f;
	mBarVel = 0.2f;
	mBarStartProgress = 0.0f;
	mPrevLoadingPercent = 0.0f;
	mApp = theApp;
	mTitleAge = 0;
	mNeedRegister = false;
	mRegisterClicked = false;
	mNeedShowRegisterBox = false;
	mLoadingThreadComplete = false;
	mDrawnYet = false;
	mNeedToInit = true;
	mQuickLoadKey = KeyCode::KEYCODE_UNKNOWN;
	mDisplayPartnerLogo = mApp->GetBoolean("DisplayPartnerLogo", false);
	mTitleState = TitleState::TITLESTATE_WAITING_FOR_FIRST_DRAW;
	mTitleStateDuration = 0;
	mTitleStateCounter = 0;
	mLoaderScreenIsLoaded = false;

	mStartButton = new Sexy::HyperlinkWidget(0, this);
	mStartButton->mColor = Sexy::Color(218, 184, 33);
	mStartButton->mOverColor = Sexy::Color(250, 90, 15);
	mStartButton->mUnderlineSize = 0;
	mStartButton->mDisabled = true;
	mStartButton->mVisible = false;

	mApp->mDetails = "In the Title Screen";
}

TitleScreen::~TitleScreen()
{
	if (mStartButton)
	{
		delete mStartButton;
	}
}

void TitleScreen::DrawToPreload(Graphics* g)
{
	g->DrawImageF(IMAGE_PLANTSHADOW, 1000.0f, 0.0f);
}

void TitleScreen::Draw(Graphics* g)
{
	g->SetLinearBlend(true);

	if (mTitleState == TitleState::TITLESTATE_WAITING_FOR_FIRST_DRAW)
	{
		g->SetColor(Color::Black);
		g->FillRect(0, 0, mWidth, mHeight);

		if (!mDrawnYet)
		{
			TodTraceAndLog("First Draw Time: %d ms\n", GetTickCount() - mApp->mTimeLoaded);
			TodHesitationTrace("TitleScreen First Draw");
			mDrawnYet = true;
		}

		return;
	}
	
	if (mTitleState == TitleState::TITLESTATE_POPCAP_LOGO)
	{
		g->SetColor(Color::Black);
		g->FillRect(0, 0, mWidth, mHeight);

		int anAlpha = 255;
		if (mTitleStateCounter < mTitleStateDuration - 50)
		{
			if (!mDisplayPartnerLogo)
			{
				anAlpha = TodAnimateCurve(50, 0, mTitleStateCounter, 255, 0, TodCurves::CURVE_LINEAR);
			}
		}
		else
		{
			anAlpha = TodAnimateCurve(mTitleStateDuration, mTitleStateDuration - 50, mTitleStateCounter, 0, 255, TodCurves::CURVE_LINEAR);
		}
		g->SetColorizeImages(true);
		g->SetColor(Color(255, 255, 255, anAlpha));
		g->DrawImage(IMAGE_POPCAP_LOGO, (mWidth - IMAGE_POPCAP_LOGO->mWidth) / 2, (mHeight - IMAGE_POPCAP_LOGO->mHeight) / 2);
		g->SetColorizeImages(false);

		return;
	}

	if (mTitleState == TitleState::TITLESTATE_PARTNER_LOGO)
	{
		g->SetColor(Color::Black);
		g->FillRect(0, 0, mWidth, mHeight);

		g->SetColorizeImages(true);
		int anAlpha = 255;
		if (mTitleStateCounter >= mTitleStateDuration - 35)
		{
			anAlpha = TodAnimateCurve(mTitleStateDuration, mTitleStateDuration - 35, mTitleStateCounter, 0, 255, TodCurves::CURVE_LINEAR);
			g->SetColor(Color(255, 255, 255, 255 - anAlpha));
			g->DrawImage(IMAGE_POPCAP_LOGO, (mWidth - IMAGE_POPCAP_LOGO->mWidth) / 2, (mHeight - IMAGE_POPCAP_LOGO->mHeight) / 2);
		}
		else
		{
			anAlpha = TodAnimateCurve(35, 0, mTitleStateCounter, 255, 0, TodCurves::CURVE_LINEAR);
		}
		g->SetColor(Color(255, 255, 255, anAlpha));
		g->DrawImage(IMAGE_PARTNER_LOGO, (mWidth - IMAGE_PARTNER_LOGO->mWidth) / 2, (mHeight - IMAGE_PARTNER_LOGO->mHeight) / 2);
		g->SetColorizeImages(false);

		return;
	}

	if (!mLoaderScreenIsLoaded)
	{
		g->SetColor(Color::Black);
		g->FillRect(0, 0, mWidth, mHeight);
		return;
	}

	g->DrawImage(IMAGE_TITLESCREEN, 0, 0);
	if (mNeedToInit)
	{
		return;
	}

	int aLogoY;
	if (mTitleStateCounter > 60)
	{
		aLogoY = TodAnimateCurve(100, 60, mTitleStateCounter, -150, 10, CURVE_EASE_IN);
	}
	else
	{
		aLogoY = TodAnimateCurve(60, 50, mTitleStateCounter, 10, 15, CURVE_BOUNCE);
	}
	g->DrawImage(IMAGE_PVZ_LOGO, mWidth / 2 - IMAGE_PVZ_LOGO->mWidth / 2, aLogoY);

	int aGrassX = mStartButton->mX + 30;
	int aGrassY = mStartButton->mY;

	Graphics aClipG(*g);
	aClipG.ClipRect(mWidth / 2 - IMAGE_LOADERBAR->mWidth, aGrassY, mCurBarWidth, IMAGE_LOADERBAR->mHeight);
	if (mCurBarWidth = mTotalBarWidth)
	{
		aClipG.DrawImage(IMAGE_LOADERBAROVER, aGrassX, aGrassY);
		aClipG.DrawImage(IMAGE_LOADER_PLAY, aGrassX + 50, aGrassY);
	}
	else
	{
		aClipG.DrawImage(IMAGE_LOADERBAR, aGrassX, aGrassY);
		aClipG.DrawImage(IMAGE_LOADERBARLOADING, aGrassX + 50, aGrassY);
	}

	Reanimation* aReanim = nullptr;
	while (mApp->mEffectSystem->mReanimationHolder->mReanimations.IterateNext(aReanim))
	{
		aReanim->Draw(g);
	}
}

void TitleScreen::Update()
{
	Widget::Update();
	if (mApp->mShutdown)
	{
		return;
	}

	MarkDirty();
	if (!mDrawnYet)
	{
		return;
	}

	if (mTitleState == TitleState::TITLESTATE_WAITING_FOR_FIRST_DRAW)
	{
		mApp->mMusic->MusicTitleScreenInit();
		mApp->StartLoadingThread();

		mTitleState = TitleState::TITLESTATE_POPCAP_LOGO;
		if (mDisplayPartnerLogo)
		{
			mTitleStateDuration = 150;
		}
		else
		{
			mTitleStateDuration = 200;
		}
		
		mTitleStateCounter = mTitleStateDuration;
	}

	if (mQuickLoadKey != KeyCode::KEYCODE_UNKNOWN && mTitleState != TitleState::TITLESTATE_SCREEN)
	{
		mTitleState = TitleState::TITLESTATE_SCREEN;
		mTitleStateDuration = 0;
		mTitleStateCounter = 100;
	}

	mTitleAge++;
	if (mTitleStateCounter > 0)
	{
		mTitleStateCounter--;
	}

	if (mTitleState == TitleState::TITLESTATE_POPCAP_LOGO)
	{
		if (mTitleStateCounter == 0)
		{
			if (mDisplayPartnerLogo)
			{
				mTitleState = TitleState::TITLESTATE_SCREEN;
				mTitleStateDuration = 200;
				mTitleStateCounter = 200;
			}
			else
			{
				mTitleState = TitleState::TITLESTATE_SCREEN;
				mTitleStateDuration = 100;
				mTitleStateCounter = 100;
			}
		}

		return;
	}
	else if (mTitleState == TitleState::TITLESTATE_PARTNER_LOGO)
	{
		if (mTitleStateCounter == 0)
		{
			mTitleState = TitleState::TITLESTATE_SCREEN;
			mTitleStateDuration = 100;
			mTitleStateCounter = 100;
		}

		return;
	}

	if (!mLoaderScreenIsLoaded)
	{
		return;
	}

	float aCurrentProgress = mApp->GetLoadingThreadProgress();
	if (mNeedToInit)
	{
		mNeedToInit = false;

		mStartButton->SetFont(FONT_BRIANNETOD16);
		mStartButton->Resize(mWidth / 2 - IMAGE_LOADERBAR->mWidth / 2, mHeight / 2 - IMAGE_LOADERBAR->mHeight / 2, mTotalBarWidth, 50);
		mStartButton->mVisible = true;

		float aEstimatedTotalLoadTime;
		if (aCurrentProgress > 0.000001f)
		{
			aEstimatedTotalLoadTime = mTitleAge / aCurrentProgress;
		}
		else
		{
			aEstimatedTotalLoadTime = 3000;
		}

		float aLoadTime = aEstimatedTotalLoadTime * (1 - aCurrentProgress);
		aLoadTime = ClampFloat(aLoadTime, 100, 3000);
		mBarVel = mTotalBarWidth / aLoadTime;
		mBarStartProgress = min(aCurrentProgress, 0.9f);
	}

	float aLoadingPercent = (aCurrentProgress - mBarStartProgress) / (1 - mBarStartProgress);

	mStartButton->Resize(mWidth / 2 - IMAGE_LOADERBAR->mWidth, mHeight - (IMAGE_LOADERBAR->mHeight * 8), mTotalBarWidth, mStartButton->mHeight);

	if (mTitleStateCounter > 0)
	{
		return;
	}

	mApp->mEffectSystem->Update();

	float aPrevWidth = mCurBarWidth;
	mCurBarWidth += mBarVel;
	if (!mLoadingThreadComplete)
	{
		if (mCurBarWidth > mTotalBarWidth * 0.99f)
		{
			mCurBarWidth = mTotalBarWidth * 0.99f;
		}
	}
	else if (mCurBarWidth > mTotalBarWidth)
	{
		mCurBarWidth = mTotalBarWidth;
	}

	if (aLoadingPercent > mPrevLoadingPercent + 0.01f || mLoadingThreadComplete)
	{
		float aBarWidth = TodAnimateCurveFloatTime(0, 1, aLoadingPercent, 0, mTotalBarWidth, TodCurves::CURVE_EASE_IN);
		float aDiff = aBarWidth - mCurBarWidth;
		float aAcceleration = TodAnimateCurveFloatTime(0, 1, aLoadingPercent, 0.0001f, 0.00001f, TodCurves::CURVE_LINEAR);
		if (mLoadingThreadComplete)
		{
			aAcceleration = 0.0001f;
		}
		mBarVel += aDiff * abs(aDiff) * aAcceleration;

		float aMinVelocity = TodAnimateCurveFloatTime(0, 1, aLoadingPercent, 0.2f, 0.01f, TodCurves::CURVE_LINEAR);
		float aMaxVelocity = 2;
		if (mApp->mTodCheatKeys)
		{
			aMinVelocity = 0;
			aMaxVelocity = 100;
		}

		if (mBarVel < aMinVelocity)
		{
			mBarVel = aMinVelocity;
		}
		else if (mBarVel > aMaxVelocity)
		{
			mBarVel = aMaxVelocity;
		}

		mPrevLoadingPercent = aLoadingPercent;
	}

	if (!mLoadingThreadComplete && mApp->mLoadingThreadCompleted)
	{
		mLoadingThreadComplete = true;
		mStartButton->SetDisabled(false);

		if (mQuickLoadKey == KeyCode::KEYCODE_ASCIIEND)
		{
			mApp->FastLoad(GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN);
		}
		else if (mQuickLoadKey == (KeyCode)0x4D)
		{
			mApp->LoadingCompleted();
		}
		else if (mQuickLoadKey == (KeyCode)0x53)
		{
			mApp->LoadingCompleted();
			mApp->KillGameSelector();
			mApp->ShowChallengeScreen(ChallengePage::CHALLENGE_PAGE_SURVIVAL);
		}
		else if (mQuickLoadKey == (KeyCode)0x43)
		{
			mApp->LoadingCompleted();
			mApp->KillGameSelector();
			mApp->ShowChallengeScreen(ChallengePage::CHALLENGE_PAGE_CHALLENGE);
		}
		else if (mQuickLoadKey == (KeyCode)0x55)
		{
			mApp->LoadingCompleted();
			mApp->KillGameSelector();
			mApp->PreNewGame(GameMode::GAMEMODE_UPSELL, false);
		}
		else if (mQuickLoadKey == (KeyCode)0x49)
		{
			mApp->LoadingCompleted();
			mApp->KillGameSelector();
			mApp->PreNewGame(GameMode::GAMEMODE_INTRO, false);
		}
		else if (mQuickLoadKey == (KeyCode)0x50)
		{
			mApp->LoadingCompleted();
			mApp->KillGameSelector();
			mApp->ShowChallengeScreen(ChallengePage::CHALLENGE_PAGE_PUZZLE);
		}
		else if (mQuickLoadKey == (KeyCode)0x52)
		{
			mApp->LoadingCompleted();
			mApp->KillGameSelector();
			mApp->ShowCreditScreen();
		}
		else if (mApp->mTodCheatKeys && mApp->mPlayerInfo && mQuickLoadKey == (KeyCode)0x54)
		{
			mApp->FastLoad(GameMode::GAMEMODE_ADVENTURE);
		}
		else
		{
			mStartButton->SetVisible(true);
		}
	}
}

void TitleScreen::Resize(int theX, int theY, int theWidth, int theHeight)
{
	Widget::Resize(theX, theY, theWidth, theHeight);
}

void TitleScreen::AddedToManager(Sexy::WidgetManager* theWidgetManager)
{
	Widget::AddedToManager(theWidgetManager);
	theWidgetManager->AddWidget(mStartButton);
}

void TitleScreen::RemovedFromManager(Sexy::WidgetManager* theWidgetManager)
{
	Widget::RemovedFromManager(theWidgetManager);
	theWidgetManager->RemoveWidget(mStartButton);
}

void TitleScreen::ButtonPress(int theId)
{
	mApp->PlaySample(Sexy::SOUND_BUTTONCLICK);
}

void TitleScreen::ButtonDepress(int theId)
{
	switch (theId)
	{
	case TitleScreen::TitleScreen_Start:
		mApp->LoadingCompleted();
		break;

	case TitleScreen::TitleScreen_Register:
		mRegisterClicked = true;
		break;
	}
}

void TitleScreen::MouseDown(int x, int y, int theClickCount)
{
	if (mLoadingThreadComplete)
	{
		mApp->PlaySample(Sexy::SOUND_BUTTONCLICK);
		mApp->LoadingCompleted();
	}
}

void TitleScreen::KeyDown(KeyCode theKey)
{
	if (mLoadingThreadComplete)
	{
		mApp->PlaySample(Sexy::SOUND_BUTTONCLICK);
		mApp->LoadingCompleted();
	}

	if (mApp->mTodCheatKeys && mApp->mPlayerInfo)
	{
		mQuickLoadKey = theKey;
	}
}
