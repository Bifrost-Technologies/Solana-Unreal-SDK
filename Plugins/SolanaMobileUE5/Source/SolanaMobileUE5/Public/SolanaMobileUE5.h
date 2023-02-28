#pragma once

#include "Modules/ModuleManager.h"

class FSolanaMobileUE5Module : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
