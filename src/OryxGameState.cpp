//---------------------------------------------------------------------------
//(C) Copyright Riley Adams 2010

//This file is part of Oryx Engine.

// Oryx Engine is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// Oryx Engine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with Oryx Engine. If not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------

#include "Oryx.h"
#include "OryxGameState.h"
#include "OryxEngine.h"

namespace Oryx
{
	GameState::GameState()
		:mEngine(Engine::getPtr()),mUserPtr(0){}
	//-----------------------------------------------------------------------

	void GameState::setReturnParam(String param, String value)
	{
		mReturnParams[param] = value;
	}
	//-----------------------------------------------------------------------

	void GameState::setReturnPtr(void* ptr)
	{
		mUserPtr = ptr;
	}
	//-----------------------------------------------------------------------

	void* GameState::getReturnPtr()
	{
		return mUserPtr;
	}
	//-----------------------------------------------------------------------

	std::map<String, String> GameState::getReturnParams()
	{
		return mReturnParams;
	}
	//-----------------------------------------------------------------------

	void GameState::setInputParams(std::map<String, String> in, void* inptr)
	{
		mInputParams = in;
		mInputPtr = inptr;
	}
	//-----------------------------------------------------------------------
}
