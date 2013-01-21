//
// Copyright (c) 2010-2011 Matthew Jack and Doug Binks
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#pragma once

#ifndef CONSOLE_INCLUDED
#define CONSOLE_INCLUDED

#include "../../RuntimeCompiler/FileSystemUtils.h"

#include "../../RuntimeCompiler/IFileChangeNotifier.h"
#include "../../RuntimeObjectSystem/ObjectInterface.h"

#include <Rocket/Core.h>
#include <Rocket/Controls.h>

class Environment;


class Console : public IFileChangeListener, public Rocket::Core::EventListener
{
public:
	
	Console(Environment* pEnv, Rocket::Core::Context* pRocketContext);
	~Console();

	// IFileChangeListener

	void OnFileChange(const IAUDynArray<const char*>& filelist);

	// ~IFileChangeListener

	// EventListener

	virtual void ProcessEvent(Rocket::Core::Event& event);

	// ~EventListener


	void OnCompileDone(bool bSuccess);

	void ToggleGUI();

	void DestroyContext();

private:

	void InitFileChangeNotifier();
	bool CreateConsoleContextFile();
	void CreateConsoleContext();
	void WriteConsoleContext();
	void WriteConsoleContext(const std::string& text);
	void ExecuteConsoleContext();

	void InitGUI();
	void ReloadGUI();
	void InitGUIReferences();
	void InitGUIEvents();
	void ApplyCurrentGUIState();
	void StoreGUITextInHistory();
	void FocusOnTextArea();
	void ApplyGUIView();
	void ApplyGUIViewType();
	void ApplyGUIClear();
	void ApplyGUIHistoryPosition();
	void ApplyGUIExecute();
	void ApplyGUIFinishExecute();

	// Private Members

	enum ETextAreaType
	{
		ETAT_SINGLE = 0,
		ETAT_MULTI,
		ETAT_COUNT
	};

	typedef std::vector<std::string> THistory;
	struct STextAreaParams
	{
		THistory history;
		int position;
		Rocket::Controls::ElementFormControl* pElement;
	};

	STextAreaParams m_textAreaParams[ETAT_COUNT]; 

	Environment* m_pEnv;
	Rocket::Core::Context* m_pRocketContext;
	ObjectId m_contextId;
	FileSystemUtils::Path m_inputFile;
	FileSystemUtils::Path m_contextFile;

	bool m_bWaitingForCompile;
	bool m_bCurrentContextFromGUI;
	bool m_bGUIVisible;
	bool m_bGUIViewMulti;

	Rocket::Core::ElementDocument* m_pDocument;
	Rocket::Controls::ElementFormControl* m_pViewButton;
	Rocket::Controls::ElementFormControl* m_pClearButton;
	Rocket::Controls::ElementFormControl* m_pBackButton;
	Rocket::Controls::ElementFormControl* m_pForwardButton;
	Rocket::Controls::ElementFormControl* m_pCloseButton;
	Rocket::Controls::ElementFormControl* m_pExecuteButton;
	Rocket::Controls::ElementFormControl* m_pSingleLineArea;
	Rocket::Controls::ElementFormControl* m_pMultiLineArea;
};

#endif // CONSOLE_INCLUDED
