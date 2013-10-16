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

#include "Console.h"

#include "../../RuntimeCompiler/BuildTool.h"
#include "../../RuntimeCompiler/ICompilerLogger.h"
#include "../../RuntimeObjectSystem/ObjectInterface.h"
#include "../../RuntimeObjectSystem/IObjectFactorySystem.h"
#include "../../RuntimeObjectSystem/IRuntimeObjectSystem.h"
#include "../../RuntimeObjectSystem/IObject.h"
#include "../../Systems/ILogSystem.h"
#include "../../Systems/IEntitySystem.h"
#include "../../Systems/IGame.h"
#include "../../Systems/SystemTable.h"
#include "../../RuntimeObjectSystem/RuntimeProtector.h"
#include "Environment.h"
#include "IConsoleContext.h"
#include "IObjectUtils.h"

#include <assert.h>
#include <fstream>
#include <algorithm>
#ifndef _WIN32
    #include <string.h>
    int stricmp( const char* pS1, const char* pS2 )
    {
        return strcasecmp( pS1, pS2 );
    }
#endif

#define CONSOLE_INPUT_FILE "Console.txt"
#define CONSOLE_CONTEXT_FILE "ConsoleContext.cpp"

// Remove windows.h define of GetObject which conflicts with EntitySystem GetObject
#if defined _WINDOWS_ && defined GetObject
#undef GetObject
#endif

using FileSystemUtils::Path;


static const char* CONTEXT_HEADER = 
	"// Generated/modified by Console.cpp during runtime - safe to delete \n"
	"#include \"../../RuntimeObjectSystem/ObjectInterfacePerModule.h\" \n"
	"#include \"../../Systems/SystemTable.h\" \n"
	"#include \"../../RuntimeObjectSystem/IObjectFactorySystem.h\" \n"
	"#include \"../../Systems/ILogSystem.h\" \n"
	"#include \"ConsoleContext.h\" \n\n"
	"REGISTERCLASS(ConsoleContext); \n\n"
	"void ConsoleContext::Execute(SystemTable* sys) \n"
	"{ \n";

static const char* CONTEXT_FOOTER =
	"\n}";

Console::Console(Environment* pEnv, Rocket::Core::Context* pRocketContext) 
	: m_pEnv(pEnv)
	, m_pRocketContext(pRocketContext)
	, m_bWaitingForCompile(false)
	, m_bCurrentContextFromGUI(false)
	, m_bGUIVisible(false)
	, m_bGUIViewMulti(false)
	, m_pDocument(0)
	, m_pViewButton(0)
	, m_pBackButton(0)
	, m_pForwardButton(0)
	, m_pCloseButton(0)
	, m_pExecuteButton(0)
	, m_pSingleLineArea(0)
	, m_pMultiLineArea(0)
{
	AU_ASSERT(m_pEnv && m_pRocketContext);

	Path basepath = Path(__FILE__).ParentPath();
    basepath = m_pEnv->sys->pRuntimeObjectSystem->FindFile( basepath );
	m_inputFile = basepath / Path(CONSOLE_INPUT_FILE);
	m_contextFile = basepath / Path(CONSOLE_CONTEXT_FILE); 
	
    m_contextFile.ToOSCanonicalCase();

	if( CreateConsoleContextFile() )
	{
		InitFileChangeNotifier();
	}	

	m_textAreaParams[ETAT_SINGLE].history.push_back("");
	m_textAreaParams[ETAT_SINGLE].position = 0;
	m_textAreaParams[ETAT_MULTI].history.push_back("");
	m_textAreaParams[ETAT_MULTI].position = 0;

	InitGUI();
}

Console::~Console()
{
	m_contextFile.Remove();

	// Call just in case it wasn't already called
	DestroyContext();
}

void Console::DestroyContext()
{
	if (m_contextId.IsValid())
	{
		delete m_pEnv->sys->pObjectFactorySystem->GetObject( m_contextId );
		m_contextId = ObjectId(); // set to invalid value
	}
}

void Console::OnFileChange(const IAUDynArray<const char*>& filelist)
{
	if (!stricmp(filelist[0], m_inputFile.c_str()))
	{
		// This is a console compilation notification

		if (!m_bWaitingForCompile)
		{
			m_bWaitingForCompile = true;
			m_bCurrentContextFromGUI = false;
			WriteConsoleContext();
		}
		else
		{
			m_pEnv->sys->pLogSystem->Log(eLV_WARNINGS, "Received console code while still waiting for last compile to complete\n");
		}
	}
	else
	{
		// This is a GUI file change notification

		ReloadGUI();
	}	
}

void Console::OnCompileDone(bool bSuccess)
{
	if (m_bWaitingForCompile)
	{
		m_bWaitingForCompile = false;

		// Remove temp context file from game runtime file list so it isn't included in full recompiles
		// This must be done every time ConsoleContext.cpp gets recompiled because it will get registered again
		m_pEnv->sys->pRuntimeObjectSystem->RemoveFromRuntimeFileList(m_contextFile.c_str());

		if (bSuccess)
		{
			// Create console context on first compile
			if (m_contextId.m_PerTypeId == InvalidId)
			{
				CreateConsoleContext();
			}

			ExecuteConsoleContext();	
		}	

		if (m_bCurrentContextFromGUI)
		{
			if (bSuccess)
			{
				ApplyGUIClear();
			}

			ApplyGUIFinishExecute();
		}
	}
}

void Console::InitFileChangeNotifier()
{
	// Make filechangenotifier monitor console code file and notify here
	m_pEnv->sys->pFileChangeNotifier->Watch(m_inputFile.c_str(), this);

	// Make filechangenotifier watch temp context file for changes and notify regular listener for recompile
	m_pEnv->sys->pRuntimeObjectSystem->AddToRuntimeFileList( m_contextFile.c_str() );
	//m_pEnv->sys->pFileChangeNotifier->Watch(m_contextFile.string().c_str(), m_pEnv->sys->pRuntimeObjectSystem);

	// Make filechangenotifier watch console RML/RCSS files
	Path basepath = Path(__FILE__).ParentPath();
	std::string filename = (basepath / Path("/../../Assets/GUI/console.rml")).m_string;
	m_pEnv->sys->pFileChangeNotifier->Watch(filename.c_str(), this);
	filename = (basepath / Path("/../../Assets/GUI/console.rcss")).m_string;
	m_pEnv->sys->pFileChangeNotifier->Watch(filename.c_str(), this);
}

void Console::InitGUI()
{
	// Load document but don't show it yet
	Rocket::Core::ElementDocument* pDocument = m_pRocketContext->LoadDocument("/GUI/console.rml");
	if (pDocument != NULL)
	{
		pDocument->SetId( "Console" );
		pDocument->RemoveReference();

		InitGUIReferences();
		InitGUIEvents();
		ApplyCurrentGUIState();		
	}
}

void Console::ReloadGUI()
{
	// Clear style sheet cache so any changes to RCSS files will be applied
	Rocket::Core::Factory::ClearStyleSheetCache();
	
	Rocket::Core::ElementDocument* pDocument = m_pRocketContext->LoadDocument("/GUI/console.rml");
	if (pDocument != NULL)
	{
		pDocument->SetId( "Console" );
		pDocument->SetOffset(m_pDocument->GetRelativeOffset(), NULL);
		pDocument->RemoveReference();
		m_pRocketContext->UnloadDocument(m_pDocument);

		InitGUIReferences();
		InitGUIEvents();
		ApplyCurrentGUIState();	
	}
}

void Console::InitGUIReferences()
{
	m_pDocument = m_pRocketContext->GetDocument("Console");
	AU_ASSERT(m_pDocument);

	m_pViewButton = static_cast<Rocket::Controls::ElementFormControl*>(m_pDocument->GetElementById("ViewButton"));
	m_pClearButton = static_cast<Rocket::Controls::ElementFormControl*>(m_pDocument->GetElementById("ClearButton"));
	m_pBackButton = static_cast<Rocket::Controls::ElementFormControl*>(m_pDocument->GetElementById("BackButton"));
	m_pForwardButton = static_cast<Rocket::Controls::ElementFormControl*>(m_pDocument->GetElementById("ForwardButton"));
	m_pCloseButton = static_cast<Rocket::Controls::ElementFormControl*>(m_pDocument->GetElementById("CloseButton"));
	m_pExecuteButton = static_cast<Rocket::Controls::ElementFormControl*>(m_pDocument->GetElementById("ExecuteButton"));
	m_pSingleLineArea = static_cast<Rocket::Controls::ElementFormControl*>(m_pDocument->GetElementById("SingleLineArea"));
	m_pMultiLineArea = static_cast<Rocket::Controls::ElementFormControl*>(m_pDocument->GetElementById("MultiLineArea"));
	AU_ASSERT(m_pViewButton && m_pBackButton && m_pForwardButton && m_pCloseButton && m_pExecuteButton && m_pSingleLineArea && m_pMultiLineArea);

	m_textAreaParams[0].pElement = m_pSingleLineArea;
	m_textAreaParams[1].pElement = m_pMultiLineArea;
}

void Console::InitGUIEvents()
{
	m_pViewButton->AddEventListener("click", this);
	m_pClearButton->AddEventListener("click", this);
	m_pBackButton->AddEventListener("click", this);
	m_pForwardButton->AddEventListener("click", this);
	m_pCloseButton->AddEventListener("click", this);
	m_pExecuteButton->AddEventListener("click", this);
	m_pSingleLineArea->AddEventListener("textinput", this);
}

void Console::ProcessEvent(Rocket::Core::Event& event)
{
	Rocket::Core::Element* pElement = event.GetTargetElement();
	std::string target = pElement->GetId().CString();

	if (!stricmp(target.c_str(), "SingleLineArea"))
	{
		// Character added to single line area - execute contents if the enter key was pressed
		Rocket::Core::word character = event.GetParameter< Rocket::Core::word >("data", 0);
		if (character == '\n')
		{
			STextAreaParams& params = m_textAreaParams[m_bGUIViewMulti ? ETAT_MULTI : ETAT_SINGLE];
			params.position = (int)params.history.size() - 1;

			StoreGUITextInHistory();
			ApplyGUIExecute();
		}
	}
	else if (!stricmp(target.c_str(), "ExecuteButton"))
	{
		// Execute contents of multi line area
		if (!m_pExecuteButton->IsDisabled())
		{
			STextAreaParams& params = m_textAreaParams[m_bGUIViewMulti ? ETAT_MULTI : ETAT_SINGLE];
			params.position = (int)params.history.size() - 1;

			StoreGUITextInHistory();
			ApplyGUIExecute();
		}
		else
		{
			m_pExecuteButton->Blur();
		}
	}
	else if (!stricmp(target.c_str(), "ViewButton"))
	{
		// Toggle between single and multi line views
		m_bGUIViewMulti = !m_bGUIViewMulti;

		STextAreaParams& params = m_textAreaParams[m_bGUIViewMulti ? ETAT_MULTI : ETAT_SINGLE];
		if (params.position == (int)params.history.size() - 1)
		{
			StoreGUITextInHistory(); // save text in latest history before changing views so we don't lose it
		}

		ApplyGUIViewType();
		ApplyGUIHistoryPosition(); // do this to set history buttons appropriately
		FocusOnTextArea();
	}
	else if (!stricmp(target.c_str(), "ClearButton"))
	{
		// Clear current text area
		ApplyGUIClear();
	}
	else if (!stricmp(target.c_str(), "CloseButton"))
	{
		// Close console
		m_bGUIVisible = false;
		ApplyGUIView();
	}
	else if (!stricmp(target.c_str(), "BackButton"))
	{
		// Go one step back in command history
		if (!m_pBackButton->IsDisabled())
		{
			STextAreaParams& params = m_textAreaParams[m_bGUIViewMulti ? ETAT_MULTI : ETAT_SINGLE];
			if (params.position == params.history.size() - 1)
			{
				StoreGUITextInHistory(); // save text in latest history before starting to go back so we don't lose it
			}

			params.position--;
			params.position = std::max(0, params.position);

			ApplyGUIHistoryPosition();
			FocusOnTextArea();
		}	
		else 
		{
			m_pBackButton->Blur();
		}
	}
	else if (!stricmp(target.c_str(), "ForwardButton"))
	{
		// Go one step forward in command history
		if (!m_pForwardButton->IsDisabled())
		{
			STextAreaParams& params = m_textAreaParams[m_bGUIViewMulti ? ETAT_MULTI : ETAT_SINGLE];
			params.position++;
			params.position = std::min((int)params.history.size(), params.position);

			ApplyGUIHistoryPosition();
			FocusOnTextArea();
		}
		else 
		{
			m_pForwardButton->Blur();
		}
	} 
}

void Console::StoreGUITextInHistory()
{
	STextAreaParams& params = m_textAreaParams[m_bGUIViewMulti ? ETAT_MULTI : ETAT_SINGLE];
	params.history[params.position] = params.pElement->GetValue().CString();
}

void Console::ToggleGUI()
{
	m_bGUIVisible = !m_bGUIVisible;
	ApplyGUIView();
	FocusOnTextArea();
}

void Console::FocusOnTextArea()
{
	if (m_bGUIViewMulti)
	{
		m_pMultiLineArea->Focus();
	}
	else
	{
		m_pSingleLineArea->Focus();
	}
}

void Console::ApplyCurrentGUIState()
{
	ApplyGUIView();
	ApplyGUIViewType();
	ApplyGUIHistoryPosition();
	FocusOnTextArea();
}

void Console::ApplyGUIView()
{
	if (m_bGUIVisible)
	{
		m_pDocument->Show();
	}
	else
	{
		m_pDocument->Hide();
	}
}

void Console::ApplyGUIViewType()
{
	if (m_bGUIViewMulti)
	{
		m_pMultiLineArea->SetProperty("display", "block");
		m_pSingleLineArea->SetProperty("display", "none");
		m_pExecuteButton->SetProperty("display", "block");
		m_pViewButton->SetInnerRML("Single");

		m_pMultiLineArea->SetProperty("tab-index", "auto");
		m_pSingleLineArea->SetProperty("tab-index", "none");
		m_pExecuteButton->SetProperty("tab-index", "auto");
	}
	else
	{
		m_pMultiLineArea->SetProperty("display", "none");
		m_pSingleLineArea->SetProperty("display", "block");
		m_pExecuteButton->SetProperty("display", "none");
		m_pViewButton->SetInnerRML("Multi");

		m_pMultiLineArea->SetProperty("tab-index", "none");
		m_pSingleLineArea->SetProperty("tab-index", "auto");
		m_pExecuteButton->SetProperty("tab-index", "none");
	}
}

void Console::ApplyGUIClear()
{
	STextAreaParams& params = m_textAreaParams[m_bGUIViewMulti ? ETAT_MULTI : ETAT_SINGLE];
	params.pElement->SetValue("");
}

void Console::ApplyGUIHistoryPosition()
{
	STextAreaParams& params = m_textAreaParams[m_bGUIViewMulti ? ETAT_MULTI : ETAT_SINGLE];

	params.pElement->SetValue(params.history[params.position].c_str());	

	if (params.position == 0)
	{
		m_pBackButton->SetDisabled(true);
		m_pBackButton->SetProperty("tab-index", "none");
		m_pBackButton->SetPseudoClass("disabled", true);
	}
	else
	{
		m_pBackButton->SetDisabled(false);
		m_pBackButton->SetProperty("tab-index", "auto");
		m_pBackButton->SetPseudoClass("disabled", false);
	}
	
	if (params.position == params.history.size() - 1)
	{
		m_pForwardButton->SetDisabled(true);
		m_pForwardButton->SetProperty("tab-index", "none");
		m_pForwardButton->SetPseudoClass("disabled", true);
	}
	else
	{
		m_pForwardButton->SetDisabled(false);
		m_pForwardButton->SetProperty("tab-index", "auto");
		m_pForwardButton->SetPseudoClass("disabled", false);
	}
}

void Console::ApplyGUIExecute()
{
	if (!m_bWaitingForCompile)
	{
		Rocket::Core::String rcText = m_bGUIViewMulti ? m_pMultiLineArea->GetValue() : m_pSingleLineArea->GetValue();
		const std::string& text = rcText.Append(";").CString(); // Add ; to end to handle common error
		
		m_bWaitingForCompile = true;
		m_bCurrentContextFromGUI = true;
		WriteConsoleContext(text);

		// Disable Execute button and text area
		m_pExecuteButton->SetDisabled(true);
		m_pMultiLineArea->SetDisabled(true);
		m_pSingleLineArea->SetDisabled(true);
		m_pMultiLineArea->SetProperty("tab-index", "none");
		m_pSingleLineArea->SetProperty("tab-index", "none");
		m_pExecuteButton->SetProperty("tab-index", "none");
		m_pExecuteButton->SetPseudoClass("disabled", true);
	}
	else
	{
		m_pEnv->sys->pLogSystem->Log(eLV_WARNINGS, "Received console code from GUI while still waiting for last compile to complete\n");
	}
}

void Console::ApplyGUIFinishExecute()
{
	// Enable Execute button and text area
	m_pExecuteButton->SetDisabled(false);
	m_pMultiLineArea->SetDisabled(false);
	m_pSingleLineArea->SetDisabled(false);
	m_pMultiLineArea->SetProperty("tab-index", "auto");
	m_pSingleLineArea->SetProperty("tab-index", "auto");
	m_pExecuteButton->SetProperty("tab-index", "auto");
	m_pExecuteButton->SetPseudoClass("disabled", false);

	STextAreaParams& params = m_textAreaParams[m_bGUIViewMulti ? ETAT_MULTI : ETAT_SINGLE];
	params.pElement->Focus();
	params.history.push_back(""); // add new history element which corresponds to current input
	params.position = (int)params.history.size() - 1;

	ApplyGUIHistoryPosition();
}

bool Console::CreateConsoleContextFile()
{
	bool bRet = true;
	std::ofstream outFile;
	outFile.open(m_contextFile.c_str(), std::ios::out | std::ios::trunc);
	if (!outFile)
	{
		bRet = false;
		m_pEnv->sys->pLogSystem->Log(eLV_ERRORS, "Unable to create context file: %s\n", m_contextFile.c_str());
	}

	return bRet;
}

// Only needs to be called once, on first compile of console context (not at program start)
void Console::CreateConsoleContext()
{
	IObject *pObj = IObjectUtils::CreateObject( "ConsoleContext" );
	pObj->GetObjectId( m_contextId );
}

void Console::WriteConsoleContext()
{
	std::ifstream inFile;
	inFile.open(m_inputFile.c_str(), std::ios::in);
	if (!inFile)
	{
		m_pEnv->sys->pLogSystem->Log(eLV_ERRORS, "Unable to open console input file for reading: %s\n", m_inputFile.c_str());
		return;
	}

	std::ofstream outFile;
	outFile.open(m_contextFile.c_str(), std::ios::out | std::ios::trunc);
	if (!outFile)
	{
		m_pEnv->sys->pLogSystem->Log(eLV_ERRORS, "Unable to open context file for writing: %s\n", m_contextFile.c_str());
		return;
	}

	// Write header boilerplate
	outFile << CONTEXT_HEADER;

	// Write console code
	outFile << inFile.rdbuf();

	// Write footer boilerplate
	outFile << CONTEXT_FOOTER;
}

void Console::WriteConsoleContext(const std::string& text)
{
	std::ofstream outFile;
	outFile.open(m_contextFile.c_str(), std::ios::out | std::ios::trunc);
	if (!outFile)
	{
		m_pEnv->sys->pLogSystem->Log(eLV_ERRORS, "Unable to open context file for writing: %s\n", m_contextFile.c_str());
		return;
	}

	// Write header boilerplate
	outFile << CONTEXT_HEADER;

	// Write console code
	outFile << text;

	// Write footer boilerplate
	outFile << CONTEXT_FOOTER;
}

// local class for console execution
class ConsoleExecuteProtector : public RuntimeProtector
{
public:
 	IConsoleContext*	pContext;
    SystemTable*		pSys;
private:
    virtual void ProtectedFunc()
    {
		pContext->Execute( pSys );
    }

};
void Console::ExecuteConsoleContext()
{
	ILogSystem *pLog = m_pEnv->sys->pLogSystem;

	IConsoleContext* pContext;
	IObjectUtils::GetObject( &pContext, m_contextId );
	AU_ASSERT(pContext);

	if (pContext)
	{
		pLog->Log(eLV_COMMENTS, "Executing console context...\n");

		// Console should execute Safe-C, but for some things we really do want to return
		// null pointers on occaision. So lets deal with those simple cases cleanly.
		ConsoleExecuteProtector consoleProtectedExecutor;
		consoleProtectedExecutor.m_bHintAllowDebug = false;
		consoleProtectedExecutor.pContext = pContext;
		consoleProtectedExecutor.pSys = m_pEnv->sys;
        m_pEnv->sys->pRuntimeObjectSystem->TryProtectedFunction( &consoleProtectedExecutor );

		if( consoleProtectedExecutor.HasHadException() )
		{
			switch (consoleProtectedExecutor.ExceptionInfo.Type)
			{
			case RuntimeProtector::ESE_Unknown:
				pLog->Log(eLV_ERRORS, "Console command caused an unknown error\n");
				break;
			case RuntimeProtector::ESE_AccessViolation:
				// Note that in practice, writing to pointers it not something that should often be needed in a console command
				if (consoleProtectedExecutor.ExceptionInfo.Addr == 0)
					pLog->Log(eLV_ERRORS, "Console command tried to access a null pointer\n");
				else
					pLog->Log(eLV_ERRORS, "Console command tried to access an invalid pointer (address 0x%p)\n", consoleProtectedExecutor.ExceptionInfo.Addr);
				break;
			case RuntimeProtector::ESE_AccessViolationRead:
				if (consoleProtectedExecutor.ExceptionInfo.Addr == 0)
					pLog->Log(eLV_ERRORS, "Console command tried to read from a null pointer\n");
				else
					pLog->Log(eLV_ERRORS, "Console command tried to read from an invalid pointer (address 0x%p)\n", consoleProtectedExecutor.ExceptionInfo.Addr);
				break;
			case RuntimeProtector::ESE_AccessViolationWrite:
				// Note that in practice, writing to pointers it not something that should often be needed in a console command
				if (consoleProtectedExecutor.ExceptionInfo.Addr == 0)
					pLog->Log(eLV_ERRORS, "Console command tried to write to a null pointer\n");
				else
					pLog->Log(eLV_ERRORS, "Console command tried to write to an invalid pointer (address 0x%p)\n", consoleProtectedExecutor.ExceptionInfo.Addr);
				break;
			case RuntimeProtector::ESE_InvalidInstruction:
					pLog->Log(eLV_ERRORS, "Console command tried to execute an invalid instruction at (address 0x%p)\n", consoleProtectedExecutor.ExceptionInfo.Addr);
				break;
			default:
				AU_ASSERT(false);
				break;
			}
		}
	}
	else
	{
		pLog->Log(eLV_ERRORS, "Unable to execute console context - no context found\n");
	}
}
