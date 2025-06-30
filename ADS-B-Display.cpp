//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
#include <tchar.h>
#include "Logger.h"
//---------------------------------------------------------------------------
USEFORM("AreaDialog.cpp", AreaConfirm);
USEFORM("DisplayGUI.cpp", Form1);
//---------------------------------------------------------------------------
static FILE* pCout = NULL;
static void SetStdOutToNewConsole(void);
//---------------------------------------------------------------------------
static void SetStdOutToNewConsole(void)
{
    // Allocate a console for this app
    AllocConsole();
    //AttachConsole(ATTACH_PARENT_PROCESS);
	freopen_s(&pCout, "CONOUT$", "w", stdout);
}

//---------------------------------------------------------------------------
int WINAPI _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int)
{
	try
	{
        SetStdOutToNewConsole();

		AnsiString logPath = "app.log";
		if (!Logger::getInstance()->initialize(logPath.c_str(), LogLevel::LOG_LEVEL_DEBUG)) {
			//ShowMessage("Failed to initialize logger");
		}
		Application->Initialize();
		Application->MainFormOnTaskBar = true;
		Application->CreateForm(__classid(TForm1), &Form1);
		Application->CreateForm(__classid(TAreaConfirm), &AreaConfirm);
		Application->Run();
	   if (pCout)
		{
		 fclose(pCout);
		 FreeConsole();
	    }
	}
	catch (Exception &exception)
	{
		Application->ShowException(&exception);
	}
	catch (...)
	{
		try
		{
			throw Exception("");
		}
		catch (Exception &exception)
		{
			Application->ShowException(&exception);
		}
	}
	return 0;
}
//---------------------------------------------------------------------------
