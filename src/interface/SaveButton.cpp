#include <iostream>

#include "SaveButton.h"
#include "search/Save.h"
#include "Graphics.h"
#include "Global.h"
#include "Engine.h"
#include "client/Client.h"

namespace ui {

SaveButton::SaveButton(Point position, Point size, Save * save):
	Component(position, size),
	save(save),
	thumbnail(NULL),
	isMouseInside(false),
	isButtonDown(false),
	actionCallback(NULL),
	voteColour(255, 0, 0)
{
	if(save->votesUp==0)
		voteRatio = 0.0f;
	else if(save->votesDown==0)
		voteRatio = 1.0f;
	else
		voteRatio = 1.0f-(float)(((float)(save->votesDown))/((float)(save->votesUp)));
	if(voteRatio < 0.0f)
		voteRatio = 0.0f;
	if(voteRatio > 1.0f)	//Not possible, but just in case the server were to give a negative value or something
		voteRatio = 1.0f;


	voteColour.Red = (1.0f-voteRatio)*255;
	voteColour.Green = voteRatio*255;

}

SaveButton::~SaveButton()
{
	if(thumbnail)
		delete thumbnail;
	if(actionCallback)
		delete actionCallback;
	if(save)
		delete save;
}

void SaveButton::Tick(float dt)
{
	Thumbnail * tempThumb;
	float scaleFactorY = 1.0f, scaleFactorX = 1.0f;
	if(!thumbnail)
	{
		tempThumb = Client::Ref().GetThumbnail(save->GetID(), 0);
		if(tempThumb)
		{
			thumbnail = new Thumbnail(*tempThumb); //Store a local copy of the thumbnail
			if(thumbnail->Data)
			{
				if(thumbnail->Size.Y > (Size.Y-25))
				{
					scaleFactorY = ((float)(Size.Y-25))/((float)thumbnail->Size.Y);
				}
				if(thumbnail->Size.X > Size.X-3)
				{
					scaleFactorX = ((float)Size.X-3)/((float)thumbnail->Size.X);
				}
				if(scaleFactorY < 1.0f || scaleFactorX < 1.0f)
				{
					float scaleFactor = scaleFactorY < scaleFactorX ? scaleFactorY : scaleFactorX;
					pixel * thumbData = thumbnail->Data;
					thumbnail->Data = Graphics::resample_img(thumbData, thumbnail->Size.X, thumbnail->Size.Y, thumbnail->Size.X * scaleFactor, thumbnail->Size.Y * scaleFactor);
					thumbnail->Size.X *= scaleFactor;
					thumbnail->Size.Y *= scaleFactor;
					free(thumbData);
				}
			}
		}
	}
}

void SaveButton::Draw(const Point& screenPos)
{
	Graphics * g = ui::Engine::Ref().g;
	float scaleFactor;
	ui::Point thumbBoxSize(0, 0);

	if(thumbnail)
	{
		thumbBoxSize = ui::Point(thumbnail->Size.X, thumbnail->Size.Y);
		if(save->id)
			g->draw_image(thumbnail->Data, screenPos.X-3+(Size.X-thumbBoxSize.X)/2, screenPos.Y+(Size.Y-25-thumbBoxSize.Y)/2, thumbnail->Size.X, thumbnail->Size.Y, 255);
		else
			g->draw_image(thumbnail->Data, screenPos.X+(Size.X-thumbBoxSize.X)/2, screenPos.Y+(Size.Y-25-thumbBoxSize.Y)/2, thumbnail->Size.X, thumbnail->Size.Y, 255);
	}
	else
	{
		scaleFactor = (Size.Y-25)/((float)YRES);
		thumbBoxSize = ui::Point(((float)XRES)*scaleFactor, ((float)YRES)*scaleFactor);
	}
	if(save->id)
	{
		g->drawrect(screenPos.X-3+(Size.X-thumbBoxSize.X)/2, screenPos.Y+(Size.Y-21-thumbBoxSize.Y)/2, thumbBoxSize.X, thumbBoxSize.Y, 180, 180, 180, 255);
		g->drawrect(screenPos.X-3+thumbBoxSize.X+(Size.X-thumbBoxSize.X)/2, screenPos.Y+(Size.Y-21-thumbBoxSize.Y)/2, 6, thumbBoxSize.Y, 180, 180, 180, 255);

		int voteBar = max(10.0f, ((float)(thumbBoxSize.Y))*voteRatio);
		g->fillrect(screenPos.X-3+thumbBoxSize.X+(Size.X-thumbBoxSize.X)/2, screenPos.Y+(thumbBoxSize.Y-voteBar)+(Size.Y-21-thumbBoxSize.Y)/2, 6, voteBar, voteColour.Red, voteColour.Green, voteColour.Blue, 255);
	}
	else
	{
		g->drawrect(screenPos.X+(Size.X-thumbBoxSize.X)/2, screenPos.Y+(Size.Y-21-thumbBoxSize.Y)/2, thumbBoxSize.X, thumbBoxSize.Y, 180, 180, 180, 255);
	}

	if(isMouseInside)
	{
		g->drawrect(screenPos.X, screenPos.Y, Size.X, Size.Y, 255, 255, 255, 255);
		g->drawtext(screenPos.X+(Size.X-Graphics::textwidth((char *)save->name.c_str()))/2, screenPos.Y+Size.Y - 21, save->name, 255, 255, 255, 255);
		g->drawtext(screenPos.X+(Size.X-Graphics::textwidth((char *)save->userName.c_str()))/2, screenPos.Y+Size.Y - 10, save->userName, 200, 230, 255, 255);
	}
	else
	{
		g->drawtext(screenPos.X+(Size.X-Graphics::textwidth((char *)save->name.c_str()))/2, screenPos.Y+Size.Y - 21, save->name, 180, 180, 180, 255);
		g->drawtext(screenPos.X+(Size.X-Graphics::textwidth((char *)save->userName.c_str()))/2, screenPos.Y+Size.Y - 10, save->userName, 100, 130, 160, 255);
	}
}

void SaveButton::OnMouseUnclick(int x, int y, unsigned int button)
{
	if(button != 1)
	{
		return; //left click only!
	}

	if(isButtonDown)
	{
		DoAction();
	}

	isButtonDown = false;
}

void SaveButton::OnMouseClick(int x, int y, unsigned int button)
{
	if(button != 1) return; //left click only!
	isButtonDown = true;
}

void SaveButton::OnMouseEnter(int x, int y)
{
	isMouseInside = true;
}

void SaveButton::OnMouseLeave(int x, int y)
{
	isMouseInside = false;
}

void SaveButton::DoAction()
{
	if(actionCallback)
		actionCallback->ActionCallback(this);
}

void SaveButton::SetActionCallback(SaveButtonAction * action)
{
	actionCallback = action;
}

} /* namespace ui */