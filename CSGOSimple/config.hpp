#pragma once
#include "helpers/math.hpp"
#include "helpers/utils.hpp"
#include "menu.hpp"
#include "helpers/input.hpp"
#include "helpers/jsonconfig.h"

/*template< typename T >
class ConfigItem
{
	std::string category, name;
	T* value;
public:
	ConfigItem(std::string category, std::string name, T* value)
	{
		this->category = category;
		this->name = name;
		this->value = value;
	}
};

template< typename T >
class ConfigValue
{
public:
	ConfigValue(std::string category_, std::string name_, T* value_, const T& def)
	{
		category = category_;
		name = name_;
		value = value_;
		default_value = def; 
	}

	std::string category, name;
	T* value;
	T default_value;
};*/

class CConfig
{
public:
	void load(std::string name);
	void save(std::string name);

};

extern CConfig* Config;
