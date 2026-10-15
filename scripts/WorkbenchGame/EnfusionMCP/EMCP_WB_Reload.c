/**
 * EMCP_WB_Reload.c - Script and plugin reload handler
 *
 * Triggers script compilation via ScriptEditor module.
 * Called via NET API TCP protocol: APIFunc = "EMCP_WB_Reload"
 */

class EMCP_WB_ReloadRequest : JsonApiStruct
{
	string target;

	void EMCP_WB_ReloadRequest()
	{
		RegV("target");
	}
}

class EMCP_WB_ReloadResponse : JsonApiStruct
{
	string status;
	string message;

	void EMCP_WB_ReloadResponse()
	{
		RegV("status");
		RegV("message");
	}
}

class EMCP_WB_Reload : NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new EMCP_WB_ReloadRequest();
	}

	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		EMCP_WB_ReloadRequest req = EMCP_WB_ReloadRequest.Cast(request);
		EMCP_WB_ReloadResponse resp = new EMCP_WB_ReloadResponse();

		string target = req.target;
		if (target == "")
			target = "scripts";

		if (target != "scripts" && target != "plugins" && target != "both")
		{
			resp.status = "error";
			resp.message = "Unknown target: " + target + ". Valid: scripts, plugins, both";
			return resp;
		}

		array<string> results = {};
		bool scriptsOk = true;
		bool pluginsOk = true;

		if (target == "scripts" || target == "both")
		{
			ScriptEditor scriptEditor = Workbench.GetModule(ScriptEditor);
			if (scriptEditor)
			{
				// WBModuleDef.ExecuteAction accepts menu labels as an array.
				// A true return accepts the action; compiler results must be read from logs.
				array<string> menuPath = {"Build", "Compile and Reload Scripts"};
				bool reloadRequested = scriptEditor.ExecuteAction(menuPath);
				scriptsOk = reloadRequested;
				if (reloadRequested)
					results.Insert("Scripts: ScriptEditor Build / Compile and Reload Scripts accepted; compilation and reload completion must be verified in logs");
				else
				{
					menuPath.Clear();
					menuPath.Insert("Build");
					menuPath.Insert("Validate Scripts");
					bool validationRequested = scriptEditor.ExecuteAction(menuPath);
					if (validationRequested)
						results.Insert("Scripts: ScriptEditor Build / Compile and Reload Scripts returned false; fallback Build / Validate Scripts accepted (validation only; scripts were NOT reloaded). Check logs for validation results");
					else
						results.Insert("Scripts: ScriptEditor ExecuteAction returned false for Build / Compile and Reload Scripts and fallback Build / Validate Scripts; no reload requested. Use these actions manually in ScriptEditor; do not try undocumented menu paths");
				}
			}
			else
			{
				scriptsOk = false;
				results.Insert("Scripts: ScriptEditor module not available");
			}
		}

		if (target == "plugins" || target == "both")
		{
			ResourceManager resMgr = Workbench.GetModule(ResourceManager);
			if (resMgr)
			{
				array<string> menuPath = {};
				menuPath.Insert("Plugins");
				menuPath.Insert("Settings");
				menuPath.Insert("Reload WB Scripts");
				bool result = resMgr.ExecuteAction(menuPath);
				pluginsOk = result;
				if (result)
					results.Insert("Plugins: ResourceManager Plugins / Settings / Reload WB Scripts accepted; reload completion is not verified");
				else
					results.Insert("Plugins: ResourceManager ExecuteAction(Plugins / Settings / Reload WB Scripts) returned false; use the documented action manually");
			}
			else
			{
				pluginsOk = false;
				results.Insert("Plugins: ResourceManager module not available");
			}
		}

		if (scriptsOk && pluginsOk)
			resp.status = "ok";
		else
			resp.status = "error";

		string msg = "";
		for (int i = 0; i < results.Count(); i++)
		{
			if (i > 0)
				msg = msg + " | ";
			msg = msg + results[i];
		}
		resp.message = msg;

		return resp;
	}
}
