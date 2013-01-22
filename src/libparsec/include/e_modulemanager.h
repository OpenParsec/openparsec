#ifndef E_MODULEMANAGER_H_
#define E_MODULEMANAGER_H_

// ----------------------------------------------------------------------------
//
#define TheModuleManager (E_ModuleManager::GetModuleManager())

// forward decls --------------------------------------------------------------
//
class E_Module;

// ----------------------------------------------------------------------------
//
typedef UTL_listentry_s<E_Module*>	LE_Module;

// ----------------------------------------------------------------------------
//
class E_ModuleManager {
protected:
	UTL_List<E_Module*>	m_Modules;

protected:
	// register the console commands for managing modules
	void _RegisterConsoleCommand() const;

	E_ModuleManager() 
	{
		_RegisterConsoleCommand();
	};
	~E_ModuleManager() 
	{ 
		KillAllModules(); 
		m_Modules.RemoveAll();
	};

public:
	// SINGLETON access
	static E_ModuleManager* GetModuleManager()
	{
		static E_ModuleManager _TheModuleManager;
		return &_TheModuleManager;
	}

	// register a module
	void RegisterModule( E_Module* pModule );

	// unregister a module
	void UnregisterModule( E_Module* pModule );

	// init all registered modules ( constructor )
	void InitAllModules();

	// kill all registered modules ( destructor )
	void KillAllModules();

	// list all loaded modules in the console
	void ListAll() const;
};

// ----------------------------------------------------------------------------
//
class E_Module 
{
protected:
	char m_szName[ 256 ];
public:
	E_Module( const char* pszName );
	virtual void Init() = 0;
	virtual void Kill() = 0;
	const char* GetName() const { return m_szName; }
};

// module registration macros (automatic module-init/deinit on startup) -------
//
#define REGISTER_MODULE( f )			class E_Module_##f : public E_Module \
										{ public: E_Module_##f( const char* pszName ) : E_Module( pszName ) {}; void Init(); void Kill() {}; } \
										inst_##f( #f ); \
										void E_Module_##f::Init()
										
#define REGISTER_MODULE_INIT( f )		class E_Module_##f : public E_Module \
										{ public: E_Module_##f( const char* pszName ) : E_Module( pszName ) {}; void Init(); void Kill(); } \
										inst_##f( #f ); \
										void E_Module_##f::Init()

#define REGISTER_MODULE_KILL(f)			void E_Module_##f::Kill()

#endif // E_MODULEMANAGER_H_
